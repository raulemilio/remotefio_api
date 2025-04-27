#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <mosquitto.h>
#include <cjson/cJSON.h>

#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <errno.h>
#include <time.h>

#include "mqtt_callback.h"
#include "mqtt_response.h"
#include "pru_control.h"

#include "gpio_input.h"
#include "task_queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mqtt_publish.h"
#include "lcd_display.h"
#include "log.h"

#define GPIO_INPUT_SLEEP_TIME_MS 5

volatile bool gpio_input_running;
pthread_mutex_t gpio_input_running_mutex = PTHREAD_MUTEX_INITIALIZER;

// mode 0
static void gpio_input_on_demand_get(ThreadGpioInputDataArgs args,
                                int flag_index,
                                int trigger_flag,
                                int data_index,
                                int datardy_flag,
                                int timeout_ms);
// mode 1 y 2
 static void gpio_input_trigger_get(ThreadGpioInputDataArgs args,
                              int flag_index,
                              int trigger_flag,
                              int data_index,
                              int datardy_flag,
                              int timeout_ms);

// aux 1
static int wait_for_gpio_input_data_and_process_get(ThreadGpioInputDataArgs args,
                                          int data_index,
                                          int datardy_flag,
                                          int timeout_ms,
                                          int clear_after_read);

// Aux 2
static void process_gpio_input_data_get(ThreadGpioInputDataArgs args, GpioInputData *gpio_input_data_send, int data_shd_index);
// Aux 3
static void notify_gpio_input_status_message(struct mosquitto *mosq, const char *msg);

TaskQueue gpio_input_queue;
pthread_t gpio_input_thread;


void *gpio_input_thread_func(void *arg) {
    ThreadGpioInputDataArgs args;
    char message[128];
    int timeout_ms;

    while (1) {
        if (task_queue_dequeue(&gpio_input_queue, &args, sizeof(ThreadGpioInputDataArgs)) == 0) {
            if (!args.gpio_input_data) continue;

            snprintf(message, sizeof(message), "Gpio in mode %d", args.gpio_input_data->mode);
            notify_gpio_input_status_message(args.mosq, message);

            int flag_index, data_index, trigger_flag, datardy_flag;

            switch (args.gpio_input_data->mode) {
                case 0:
                    // logica de modo 0
                    flag_index = PRU_SHD_GPIO_INPUT_MODE0_FLAG_INDEX;
                    data_index = PRU_SHD_GPIO_INPUT_MODE0_DATA_INDEX;
                    trigger_flag = PRU_GPIO_INPUT_MODE0_FLAG;
                    datardy_flag = PRU_GPIO_INPUT_MODE0_DATARDY_FLAG;
		    timeout_ms = GPIO_INPUT_TIMEOUT_MS_MODE0;
		    gpio_input_on_demand_get(args, flag_index, trigger_flag, data_index, datardy_flag, timeout_ms);
                    break;
                case 1:
                    // logica de modo 1
                    flag_index = PRU_SHD_GPIO_INPUT_MODE1_FLAG_INDEX;
                    data_index = PRU_SHD_GPIO_INPUT_MODE1_DATA_INDEX;
                    trigger_flag = PRU_GPIO_INPUT_MODE1_FLAG;
                    datardy_flag = PRU_GPIO_INPUT_MODE1_DATARDY_FLAG;
                    timeout_ms = GPIO_INPUT_TIMEOUT_MS_MODE1;
                    gpio_input_trigger_get(args, flag_index,trigger_flag,data_index,datardy_flag,timeout_ms);
                    break;
                case 2:
                    // logica de modo 2
                    flag_index = PRU_SHD_GPIO_INPUT_MODE2_FLAG_INDEX;
                    data_index = PRU_SHD_GPIO_INPUT_MODE2_DATA_INDEX;
                    trigger_flag = PRU_GPIO_INPUT_MODE2_FLAG;
                    datardy_flag = PRU_GPIO_INPUT_MODE2_DATARDY_FLAG;
                    timeout_ms = GPIO_INPUT_TIMEOUT_MS_MODE2;
		    gpio_input_trigger_get(args,flag_index,trigger_flag,data_index,datardy_flag,timeout_ms);
                    break;
                default:
		    notify_gpio_input_status_message(args.mosq, MSG_GPIO_INPUT_INVALID_MODE);
                    break;
            }

            pthread_mutex_lock(&gpio_input_running_mutex);
            gpio_input_running = TASK_STOPPED;
            pthread_mutex_unlock(&gpio_input_running_mutex);

            notify_gpio_input_status_message(args.mosq, MSG_GPIO_INPUT_FINISH);
            free(args.gpio_input_data);
            //Limpieza del struct para evitar basura en la proxima iteracion
            memset(&args, 0, sizeof(args));
        }
    }
    return NULL;
}

// mode 0
static void gpio_input_on_demand_get(ThreadGpioInputDataArgs args,
				int flag_index,
				int trigger_flag,
				int data_index,
				int datardy_flag,
				int timeout_ms){
    // Avisamos a pru
    pthread_mutex_lock(&shm->shared_mutex[MUTEX_GPIO_INPUT]);
    shm->shared[flag_index] |= (1 << trigger_flag);
    pthread_mutex_unlock(&shm->shared_mutex[MUTEX_GPIO_INPUT]);
    // esperamos dato
    wait_for_gpio_input_data_and_process_get(args, data_index, datardy_flag, timeout_ms, 0);
}
// mode 1 y 2
static void gpio_input_trigger_get(ThreadGpioInputDataArgs args,
			      int flag_index,
			      int trigger_flag,
			      int data_index,
			      int datardy_flag,
			      int timeout_ms){
    // Avisamos a pru
    pthread_mutex_lock(&shm->shared_mutex[MUTEX_GPIO_INPUT]);
    shm->shared[flag_index] |= (1 << trigger_flag);
    pthread_mutex_unlock(&shm->shared_mutex[MUTEX_GPIO_INPUT]);

    bool is_gpio_input_running = false;

    /*DEBUG
    struct timespec start, end;
    double elapsed_ms;
    DEBUG*/

    while (1) {

	/*DEBUG
        clock_gettime(CLOCK_MONOTONIC, &start); // Tomamos tiempo antes de la vuelta
	DEBUG*/

        // Bloqueamos el mutex para leer gpio_input_running de forma segura

        pthread_mutex_lock(&gpio_input_running_mutex);
        is_gpio_input_running = gpio_input_running;
        pthread_mutex_unlock(&gpio_input_running_mutex);

        pthread_mutex_lock(&shm->shared_mutex[MUTEX_GPIO_INPUT]);
        int trigger_active = shm->shared[flag_index] & (1 << trigger_flag);
        pthread_mutex_unlock(&shm->shared_mutex[MUTEX_GPIO_INPUT]);

        if (!is_gpio_input_running || !trigger_active) {
	    notify_gpio_input_status_message(args.mosq, MSG_GPIO_INPUT_STOPPED);
            break;
        }

        // entramos a un loop de medicion
        int result = wait_for_gpio_input_data_and_process_get(args, data_index, datardy_flag, timeout_ms, 1);
        if (result != 0) break; // salir si hubo timeout, parada o error

        /*DEBUG
        clock_gettime(CLOCK_MONOTONIC, &end); // Tomamos tiempo después de la vuelta
        elapsed_ms = (end.tv_sec - start.tv_sec) * 1000.0 +
                 (end.tv_nsec - start.tv_nsec) / 1000000.0;
        printf("Tiempo de vuelta: %.3f ms\n", elapsed_ms);
        DEBUG*/

    }

    pthread_mutex_lock(&shm->shared_mutex[MUTEX_GPIO_INPUT]);
    shm->shared[flag_index] &= ~(1 << trigger_flag);  // Detener pru function mode
    pthread_mutex_unlock(&shm->shared_mutex[MUTEX_GPIO_INPUT]);

}

// aux 1
static int wait_for_gpio_input_data_and_process_get(ThreadGpioInputDataArgs args,
				          int data_index,
				          int datardy_flag,
					  int timeout_ms,
					  int clear_after_read){

    int elapsed = 0;
    int sleep_time_ms = GPIO_INPUT_SLEEP_TIME_MS;

    GpioInputData gpio_input_data_send;
    bool is_gpio_input_running = false;

    while (timeout_ms < 0 || elapsed < timeout_ms) {

        pthread_mutex_lock(&gpio_input_running_mutex);
        is_gpio_input_running = gpio_input_running;
        pthread_mutex_unlock(&gpio_input_running_mutex);

        pthread_mutex_lock(&shm->shared_mutex[MUTEX_GPIO_INPUT]);
        int ready = shm->shared[data_index] & (1 << datardy_flag);
        pthread_mutex_unlock(&shm->shared_mutex[MUTEX_GPIO_INPUT]);

        if (ready) {
            pthread_mutex_lock(&shm->shared_mutex[MUTEX_GPIO_INPUT]);
            process_gpio_input_data_get(args, &gpio_input_data_send, data_index);
            if (clear_after_read)
                shm->shared[data_index] = PRU_ERASE_MEM;
            pthread_mutex_unlock(&shm->shared_mutex[MUTEX_GPIO_INPUT]);

            publish_gpio_input_response(args.mosq, gpio_input_data_send, MSG_GPIO_INPUT_DONE);
            LOG_DEBUG(MSG_GPIO_INPUT_DONE);
            //lcd_show_message(MSG_GPIO_INPUT_DONE);
            return 0; //exito
        }

        if (!is_gpio_input_running) {
	    notify_gpio_input_status_message(args.mosq, MSG_GPIO_INPUT_STOPPED);
            return -1;
        }

        usleep(sleep_time_ms * 1000); // usleep recibe microsegundos (1 ms = 1000 us)
        elapsed += sleep_time_ms;     // sumamos correctamente segun cuanto dormimos
    }

    if (timeout_ms > 0 && elapsed >= timeout_ms) {
        notify_gpio_input_status_message(args.mosq, MSG_GPIO_INPUT_TIMEOUT_EXPIRED);
        return -2;
    }

    return -3; // sin exito pero sin timeout
}

// Aux 2
static void process_gpio_input_data_get(ThreadGpioInputDataArgs args, GpioInputData *gpio_input_data_send, int data_shd_index){

    for (int i = 0; i < args.gpio_input_data->num_input; i++) {
        int pin = args.gpio_input_data->input[i];
        gpio_input_data_send->state[i] = (shm->shared[data_shd_index] >> pin) & 1; // <-shm
        gpio_input_data_send->input[i] = pin;
        //LOG_DEBUG("state [i] %d",gpio_input_data_send->state[i]);
    }
    gpio_input_data_send->num_input = args.gpio_input_data->num_input;
    // Cargar timestamp en milisegundos
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint64_t ts_ms = (uint64_t)tv.tv_sec * 1000 + (tv.tv_usec / 1000);
    gpio_input_data_send->ts = ts_ms;
}
// Aux 3
static void notify_gpio_input_status_message(struct mosquitto *mosq, const char *msg) {
    publish_gpio_input_response_status(mosq, msg);
    LOG_INFO("%s", msg);
    lcd_show_message(msg);
}
