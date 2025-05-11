#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <mosquitto.h>
#include <cjson/cJSON.h>

#include <stdint.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>

#include "mqtt_callback.h"
#include "mqtt_response.h"
#include "pru_control.h"
#include "gpio_output_set.h"
#include "task_queue.h"
#include "mqtt_publish.h"
#include "lcd_display.h"
#include "log.h"

#define GPIO_OUTPUT_SET_SLEEP_TIME_MS      10

volatile bool gpio_output_set_running;
pthread_mutex_t gpio_output_set_running_mutex = PTHREAD_MUTEX_INITIALIZER;

TaskQueue gpio_output_set_queue;
pthread_t gpio_output_set_thread;

// mode 0
static void gpio_output_set_on_demand_set(ThreadGpioOutputSetDataArgs args,
                              int flag_index,
                              int trigger_flag,
                              int data_index,
                              int datardy_flag,
                              int timeout_ms);
// aux 1
static int wait_for_gpio_output_set_data_and_process_done(ThreadGpioOutputSetDataArgs args,
                                                          int data_index,
                                                          int datardy_flag,
                                                          int timeout_ms);
// Aux 2
static void process_gpio_output_set_data_set(ThreadGpioOutputSetDataArgs args, int data_shd_index);
// Aux 3
static void notify_gpio_output_set_status_message(struct mosquitto *mosq, const char *msg);

void *gpio_output_set_thread_func(void *arg) {
    ThreadGpioOutputSetDataArgs args;
    char message[128];
    int timeout_ms;

    while (1) {
        int res = task_queue_dequeue(&gpio_output_set_queue, &args, sizeof(ThreadGpioOutputSetDataArgs));
        if (res == -1) {
            LOG_DEBUG("gpio_output_set_thread finish");
            break;
        }

        if (!args.gpio_output_set_data) continue;

        snprintf(message, sizeof(message), "G Out set mode %d", args.gpio_output_set_data->mode);
        notify_gpio_output_set_status_message(args.mosq, message);

        pthread_mutex_lock(&gpio_output_set_running_mutex);
	gpio_output_set_running = true;
        pthread_mutex_unlock(&gpio_output_set_running_mutex);

        int flag_index, data_index, trigger_flag, datardy_flag;

        flag_index = PRU_SHD_GPIO_OUTPUT_SET_FLAG_INDEX;
        data_index = PRU_SHD_GPIO_OUTPUT_SET_DATA_INDEX;
        datardy_flag = PRU_GPIO_OUTPUT_SET_DATARDY_FLAG;

        switch (args.gpio_output_set_data->mode) {
            case 0:
                 // logica de modo 0
                 trigger_flag = PRU_GPIO_OUTPUT_SET_MODE0_FLAG;
                 timeout_ms = GPIO_OUTPUT_SET_TIMEOUT_MS_MODE0;
                 gpio_output_set_on_demand_set(args, flag_index,trigger_flag,data_index,datardy_flag,timeout_ms);
                 break;
            default:
		 notify_gpio_output_set_status_message(args.mosq, MSG_GPIO_OUTPUT_INVALID_MODE);
                 break;
        }

        pthread_mutex_lock(&gpio_output_set_running_mutex);
        gpio_output_set_running = TASK_STOPPED;
        pthread_mutex_unlock(&gpio_output_set_running_mutex);

        notify_gpio_output_set_status_message(args.mosq, MSG_GPIO_OUTPUT_FINISH);
        free(args.gpio_output_set_data);
        //Limpieza del struct para evitar basura en la proxima iteracion
        memset(&args, 0, sizeof(args));
    }
    return NULL;
}
// mode 0
static void gpio_output_set_on_demand_set(ThreadGpioOutputSetDataArgs args,
                              int flag_index,
                              int trigger_flag,
                              int data_index,
                              int datardy_flag,
                              int timeout_ms)
{
    // write data into shared
    pthread_mutex_lock(&shm->shared_mutex[MUTEX_GPIO_OUTPUT_SET]);
    process_gpio_output_set_data_set(args, data_index);
    shm->shared[flag_index] |= (1 << trigger_flag); // Aviso al pru
    pthread_mutex_unlock(&shm->shared_mutex[MUTEX_GPIO_OUTPUT_SET]);

    // Esperamos que cumpla la tarea
    wait_for_gpio_output_set_data_and_process_done(args,data_index, datardy_flag, timeout_ms);
}

// aux 1
static int wait_for_gpio_output_set_data_and_process_done(ThreadGpioOutputSetDataArgs args,
							  int data_index,
                                          	          int datardy_flag,
                                          		  int timeout_ms){

    int elapsed = 0;
    int sleep_time_ms = GPIO_OUTPUT_SET_SLEEP_TIME_MS;
    bool is_gpio_output_set_running = false;

    while (timeout_ms < 0 || elapsed < timeout_ms) {

        pthread_mutex_lock(&gpio_output_set_running_mutex);
        is_gpio_output_set_running = gpio_output_set_running;
        pthread_mutex_unlock(&gpio_output_set_running_mutex);

        pthread_mutex_lock(&shm->shared_mutex[MUTEX_GPIO_OUTPUT_SET]);
        int ready = shm->shared[data_index] & (1 << datardy_flag);
        pthread_mutex_unlock(&shm->shared_mutex[MUTEX_GPIO_OUTPUT_SET]);

        if (ready) {
            notify_gpio_output_set_status_message(args.mosq, MSG_GPIO_OUTPUT_DONE);
            return 0; //exito
        }

        if (!is_gpio_output_set_running) {
            notify_gpio_output_set_status_message(args.mosq, MSG_GPIO_OUTPUT_STOPPED);
            return -1;
        }

        usleep(sleep_time_ms * 1000); // usleep recibe microsegundos (1 ms = 1000 us)
        elapsed += sleep_time_ms;
    }

    if (timeout_ms > 0 && elapsed >= timeout_ms) {
        notify_gpio_output_set_status_message(args.mosq, MSG_GPIO_OUTPUT_TIMEOUT_EXPIRED);
        return -2;
    }

    return -3; // sin exito pero sin timeout
}

// Aux 2
static void process_gpio_output_set_data_set(ThreadGpioOutputSetDataArgs args, int data_shd_index){

    for (int i = 0; i < args.gpio_output_set_data->num_output; i++) {
        int pin = args.gpio_output_set_data->output[i];
        int state = args.gpio_output_set_data->state[i] & 1;
        if (pin >= 0 && pin < 4){
	    shm->shared[data_shd_index] |= (1 << pin);
        }
        if (i < 4){
	    shm->shared[data_shd_index] |= (state << (GPIO_OUTPUT_OFFSET_STATE + pin));
        }
    }
}
// Aux 3
static void notify_gpio_output_set_status_message(struct mosquitto *mosq, const char *msg) {
    publish_gpio_output_set_response_status(mosq, msg);
    LOG_INFO("%s", msg);
    lcd_show_message(msg);
}
