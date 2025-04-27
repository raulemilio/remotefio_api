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

#include "mqtt_callback.h"
#include "mqtt_response.h"
#include "pru_control.h"
#include "gpio_output.h"
#include "task_queue.h"
#include "mqtt_publish.h"
#include "lcd_display.h"
#include "log.h"

#define GPIO_OUTPUT_OFFSET_STATE       4
#define GPIO_OUTPUT_SLEEP_TIME_MS      10

volatile bool gpio_output_running;
pthread_mutex_t gpio_output_running_mutex = PTHREAD_MUTEX_INITIALIZER;

TaskQueue gpio_output_queue;
pthread_t gpio_output_thread;

// mode 0
static void gpio_output_on_demand_get(ThreadGpioOutputDataArgs args,
                                int flag_index,
                                int trigger_flag,
                                int data_index,
                                int datardy_flag,
                                int timeout_ms);
// mode 1
static void gpio_output_on_demand_set(ThreadGpioOutputDataArgs args,
                              int flag_index,
                              int trigger_flag,
                              int data_index,
                              int datardy_flag,
                              int timeout_ms);
// aux 1
static int wait_for_gpio_output_data_and_process_get(ThreadGpioOutputDataArgs args,
                                          int data_index,
                                          int datardy_flag,
                                          int timeout_ms,
                                          int clear_after_read);
// aux 2
static int wait_for_gpio_output_data_and_process_done(ThreadGpioOutputDataArgs args,
                                                          int data_index,
                                                          int datardy_flag,
                                                          int timeout_ms);
// Aux 3
static void process_gpio_output_data_get(ThreadGpioOutputDataArgs args, GpioOutputData *gpio_output_data_send, int data_shd_index);
// Aux 4
static void process_gpio_output_data_set(ThreadGpioOutputDataArgs args, int data_shd_index);
// Aux 5
static void notify_gpio_output_status_message(struct mosquitto *mosq, const char *msg);



void *gpio_output_thread_func(void *arg) {
    ThreadGpioOutputDataArgs args;
    char mensaje[128];
    int timeout_ms;

    while (1) {
        if (task_queue_dequeue(&gpio_output_queue, &args, sizeof(ThreadGpioOutputDataArgs)) == 0) {
            if (!args.gpio_output_data) continue;

	    LOG_INFO("Function: gpio_output -> mode %d running", args.gpio_output_data->mode);
            snprintf(mensaje, sizeof(mensaje), "Gpio out mode %d", args.gpio_output_data->mode);
            lcd_show_message(mensaje);

            pthread_mutex_lock(&gpio_output_running_mutex);
	    gpio_output_running = true;
            pthread_mutex_unlock(&gpio_output_running_mutex);

            int flag_index, data_index, trigger_flag, datardy_flag;

            switch (args.gpio_output_data->mode) {
                case 0:
                    // logica de modo 0
                    flag_index = PRU_SHD_GPIO_OUTPUT_MODE0_FLAG_INDEX;
                    data_index = PRU_SHD_GPIO_OUTPUT_MODE0_DATA_INDEX;
                    trigger_flag = PRU_GPIO_OUTPUT_MODE0_FLAG;
                    datardy_flag = PRU_GPIO_OUTPUT_MODE0_DATARDY_FLAG;
                    timeout_ms = GPIO_OUTPUT_TIMEOUT_MS_MODE0;
                    gpio_output_on_demand_get(args, flag_index, trigger_flag, data_index, datardy_flag, timeout_ms);
                    break;
                case 1:
                    // logica de modo 1
                    flag_index = PRU_SHD_GPIO_OUTPUT_MODE1_FLAG_INDEX;
                    data_index = PRU_SHD_GPIO_OUTPUT_MODE1_DATA_INDEX;
                    trigger_flag = PRU_GPIO_OUTPUT_MODE1_FLAG;
                    datardy_flag = PRU_GPIO_OUTPUT_MODE1_DATARDY_FLAG;
                    timeout_ms = GPIO_OUTPUT_TIMEOUT_MS_MODE1;
                    gpio_output_on_demand_set(args, flag_index,trigger_flag,data_index,datardy_flag,timeout_ms);
                    break;
                default:
		    notify_gpio_output_status_message(args.mosq, MSG_GPIO_OUTPUT_INVALID_MODE);
                    break;
            }

            pthread_mutex_lock(&gpio_output_running_mutex);
            gpio_output_running = TASK_STOPPED;
            pthread_mutex_unlock(&gpio_output_running_mutex);

            notify_gpio_output_status_message(args.mosq, MSG_GPIO_OUTPUT_FINISH);
            free(args.gpio_output_data);
            //Limpieza del struct para evitar basura en la proxima iteracion
            memset(&args, 0, sizeof(args));
        }
    }
    return NULL;
}
// mode 0
static void gpio_output_on_demand_get(ThreadGpioOutputDataArgs args,
                                int flag_index,
                                int trigger_flag,
                                int data_index,
                                int datardy_flag,
                                int timeout_ms)
{
    pthread_mutex_lock(&shm->shared_mutex[MUTEX_GPIO_OUTPUT]);
    shm->shared[flag_index] |= (1 << trigger_flag);
    pthread_mutex_unlock(&shm->shared_mutex[MUTEX_GPIO_OUTPUT]);

    wait_for_gpio_output_data_and_process_get(args, data_index, datardy_flag, timeout_ms, 0);
}
// mode 1
static void gpio_output_on_demand_set(ThreadGpioOutputDataArgs args,
                              int flag_index,
                              int trigger_flag,
                              int data_index,
                              int datardy_flag,
                              int timeout_ms)
{
    // write data into shared
    pthread_mutex_lock(&shm->shared_mutex[MUTEX_GPIO_OUTPUT]);
    process_gpio_output_data_set(args, data_index);
    shm->shared[flag_index] |= (1 << trigger_flag); // Aviso al pru
    pthread_mutex_unlock(&shm->shared_mutex[MUTEX_GPIO_OUTPUT]);

    // Esperamos que cumpla la tarea
    wait_for_gpio_output_data_and_process_done(args,data_index, datardy_flag, timeout_ms);
}

// aux 1
static int wait_for_gpio_output_data_and_process_get(ThreadGpioOutputDataArgs args,
                                          int data_index,
                                          int datardy_flag,
                                          int timeout_ms,
                                          int clear_after_read)
{
    int elapsed = 0;
    int sleep_time_ms = GPIO_OUTPUT_SLEEP_TIME_MS;
    GpioOutputData gpio_output_data_send;
    bool is_gpio_output_running = false;

    while (timeout_ms < 0 || elapsed < timeout_ms) {

        pthread_mutex_lock(&gpio_output_running_mutex);
        is_gpio_output_running = gpio_output_running;
        pthread_mutex_unlock(&gpio_output_running_mutex);

        pthread_mutex_lock(&shm->shared_mutex[MUTEX_GPIO_OUTPUT]);
        int ready = shm->shared[data_index] & (1 << datardy_flag);
        pthread_mutex_unlock(&shm->shared_mutex[MUTEX_GPIO_OUTPUT]);

        if (ready) {
            pthread_mutex_lock(&shm->shared_mutex[MUTEX_GPIO_OUTPUT]);
            process_gpio_output_data_get(args, &gpio_output_data_send, data_index);
            if (clear_after_read)
                shm->shared[data_index] = PRU_ERASE_MEM;
            pthread_mutex_unlock(&shm->shared_mutex[MUTEX_GPIO_OUTPUT]);

            publish_gpio_output_response(args.mosq, gpio_output_data_send, MSG_GPIO_OUTPUT_DONE);
            LOG_DEBUG(MSG_GPIO_OUTPUT_DONE);
            //lcd_show_message(MSG_GPIO_OUTPUT_DONE);
            return 0; //exito
        }

        if (!is_gpio_output_running) {
	    notify_gpio_output_status_message(args.mosq, MSG_GPIO_OUTPUT_STOPPED);
            return -1;
        }

        usleep(sleep_time_ms * 1000); // usleep recibe microsegundos (1 ms = 1000 us)
        elapsed += sleep_time_ms;
    }

    if (timeout_ms > 0 && elapsed >= timeout_ms) {
        notify_gpio_output_status_message(args.mosq, MSG_GPIO_OUTPUT_TIMEOUT_EXPIRED);
        return -2;
    }

    return -3; // sin exito pero sin timeout
}

// aux 2
static int wait_for_gpio_output_data_and_process_done(ThreadGpioOutputDataArgs args,
							  int data_index,
                                          	          int datardy_flag,
                                          		  int timeout_ms){

    int elapsed = 0;
    int sleep_time_ms = GPIO_OUTPUT_SLEEP_TIME_MS;
    bool is_gpio_output_running = false;

    while (timeout_ms < 0 || elapsed < timeout_ms) {

        pthread_mutex_lock(&gpio_output_running_mutex);
        is_gpio_output_running = gpio_output_running;
        pthread_mutex_unlock(&gpio_output_running_mutex);

        pthread_mutex_lock(&shm->shared_mutex[MUTEX_GPIO_OUTPUT]);
        int ready = shm->shared[data_index] & (1 << datardy_flag);
        pthread_mutex_unlock(&shm->shared_mutex[MUTEX_GPIO_OUTPUT]);

        if (ready) {
            notify_gpio_output_status_message(args.mosq, MSG_GPIO_OUTPUT_DONE);
            return 0; //exito
        }

        if (!is_gpio_output_running) {
            notify_gpio_output_status_message(args.mosq, MSG_GPIO_OUTPUT_STOPPED);
            return -1;
        }

        usleep(sleep_time_ms * 1000); // usleep recibe microsegundos (1 ms = 1000 us)
        elapsed += sleep_time_ms;
    }

    if (timeout_ms > 0 && elapsed >= timeout_ms) {
        notify_gpio_output_status_message(args.mosq, MSG_GPIO_OUTPUT_TIMEOUT_EXPIRED);
        return -2;
    }

    return -3; // sin exito pero sin timeout
}

// Aux 3
static void process_gpio_output_data_get(ThreadGpioOutputDataArgs args, GpioOutputData *gpio_output_data_send, int data_shd_index){

    for (int i = 0; i < args.gpio_output_data->num_output; i++) {
        int pin = args.gpio_output_data->output[i];
        gpio_output_data_send->state[i] = (shm->shared[data_shd_index] >> (pin + GPIO_OUTPUT_OFFSET_STATE)) & 1; // <-shm
        gpio_output_data_send->output[i] = pin;
        //LOG_DEBUG("state [i] %d",gpio_output_data_send->state[i]);
    }
    gpio_output_data_send->num_output = args.gpio_output_data->num_output;
}

// Aux 4
static void process_gpio_output_data_set(ThreadGpioOutputDataArgs args, int data_shd_index){

    for (int i = 0; i < args.gpio_output_data->num_output; i++) {
        int pin = args.gpio_output_data->output[i];
        int state = args.gpio_output_data->state[i] & 1;
        if (pin >= 0 && pin < 4){
	    shm->shared[data_shd_index] |= (1 << pin);
        }
        if (i < 4){
	    shm->shared[data_shd_index] |= (state << (GPIO_OUTPUT_OFFSET_STATE + pin));
        }
    }
}
// Aux 5
static void notify_gpio_output_status_message(struct mosquitto *mosq, const char *msg) {
    publish_gpio_output_response_status(mosq, msg);
    LOG_INFO("%s", msg);
    lcd_show_message(msg);
}
