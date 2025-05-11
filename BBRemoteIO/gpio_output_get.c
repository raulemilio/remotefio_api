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
#include "gpio_output_get.h"
#include "task_queue.h"
#include "mqtt_publish.h"
#include "lcd_display.h"
#include "log.h"

#define GPIO_OUTPUT_GET_SLEEP_TIME_MS      10

volatile bool gpio_output_get_running;
pthread_mutex_t gpio_output_get_running_mutex = PTHREAD_MUTEX_INITIALIZER;

TaskQueue gpio_output_get_queue;
pthread_t gpio_output_get_thread;

// mode 0
static void gpio_output_get_on_demand_get(ThreadGpioOutputGetDataArgs args,
                                int flag_index,
                                int trigger_flag,
                                int data_index,
                                int datardy_flag,
                                int timeout_ms);
// aux 1
static int wait_for_gpio_output_get_data_and_process_get(ThreadGpioOutputGetDataArgs args,
                                          int data_index,
                                          int datardy_flag,
                                          int timeout_ms,
                                          int clear_after_read);
// Aux 3
static void process_gpio_output_get_data_get(ThreadGpioOutputGetDataArgs args, GpioOutputGetData *gpio_output_get_data_send, int data_shd_index);

// Aux 5
static void notify_gpio_output_get_status_message(struct mosquitto *mosq, const char *msg);


void *gpio_output_get_thread_func(void *arg) {
    ThreadGpioOutputGetDataArgs args;
    char message[128];
    int timeout_ms;

    while (1) {
        int res = task_queue_dequeue(&gpio_output_get_queue, &args, sizeof(ThreadGpioOutputGetDataArgs));
        if (res == -1) {
            LOG_DEBUG("gpio_output_get_thread finish");
            break;
        }

        if (!args.gpio_output_get_data) continue;

        snprintf(message, sizeof(message), "G Out get mode %d", args.gpio_output_get_data->mode);
        notify_gpio_output_get_status_message(args.mosq, message);

        pthread_mutex_lock(&gpio_output_get_running_mutex);
	gpio_output_get_running = true;
        pthread_mutex_unlock(&gpio_output_get_running_mutex);

        int flag_index, data_index, trigger_flag, datardy_flag;

        flag_index = PRU_SHD_GPIO_OUTPUT_GET_FLAG_INDEX;
        data_index = PRU_SHD_GPIO_OUTPUT_GET_DATA_INDEX;
        datardy_flag = PRU_GPIO_OUTPUT_GET_DATARDY_FLAG;

        switch (args.gpio_output_get_data->mode) {
             case 0:
                  // logica de modo 0
                  trigger_flag = PRU_GPIO_OUTPUT_GET_MODE0_FLAG;
                  timeout_ms = GPIO_OUTPUT_GET_TIMEOUT_MS_MODE0;
                  gpio_output_get_on_demand_get(args, flag_index, trigger_flag, data_index, datardy_flag, timeout_ms);
                  break;
             default:
		  notify_gpio_output_get_status_message(args.mosq, MSG_GPIO_OUTPUT_INVALID_MODE);
                  break;
        }

        pthread_mutex_lock(&gpio_output_get_running_mutex);
        gpio_output_get_running = TASK_STOPPED;
        pthread_mutex_unlock(&gpio_output_get_running_mutex);

        notify_gpio_output_get_status_message(args.mosq, MSG_GPIO_OUTPUT_FINISH);
        free(args.gpio_output_get_data);
        //Limpieza del struct para evitar basura en la proxima iteracion
        memset(&args, 0, sizeof(args));
    }
    return NULL;
}
// mode 0
static void gpio_output_get_on_demand_get(ThreadGpioOutputGetDataArgs args,
                                int flag_index,
                                int trigger_flag,
                                int data_index,
                                int datardy_flag,
                                int timeout_ms)
{
    pthread_mutex_lock(&shm->shared_mutex[MUTEX_GPIO_OUTPUT_GET]);
    shm->shared[flag_index] |= (1 << trigger_flag);
    pthread_mutex_unlock(&shm->shared_mutex[MUTEX_GPIO_OUTPUT_GET]);

    wait_for_gpio_output_get_data_and_process_get(args, data_index, datardy_flag, timeout_ms, 0);
}

// aux 1
static int wait_for_gpio_output_get_data_and_process_get(ThreadGpioOutputGetDataArgs args,
                                          int data_index,
                                          int datardy_flag,
                                          int timeout_ms,
                                          int clear_after_read)
{
    int elapsed = 0;
    int sleep_time_ms = GPIO_OUTPUT_GET_SLEEP_TIME_MS;
    GpioOutputGetData gpio_output_get_data_send;
    bool is_gpio_output_get_running = false;

    while (timeout_ms < 0 || elapsed < timeout_ms) {

        pthread_mutex_lock(&gpio_output_get_running_mutex);
        is_gpio_output_get_running = gpio_output_get_running;
        pthread_mutex_unlock(&gpio_output_get_running_mutex);

        pthread_mutex_lock(&shm->shared_mutex[MUTEX_GPIO_OUTPUT_GET]);
        int ready = shm->shared[data_index] & (1 << datardy_flag);
        pthread_mutex_unlock(&shm->shared_mutex[MUTEX_GPIO_OUTPUT_GET]);

        if (ready) {
            pthread_mutex_lock(&shm->shared_mutex[MUTEX_GPIO_OUTPUT_GET]);
            process_gpio_output_get_data_get(args, &gpio_output_get_data_send, data_index);
            if (clear_after_read)
                shm->shared[data_index] = PRU_ERASE_MEM;
            pthread_mutex_unlock(&shm->shared_mutex[MUTEX_GPIO_OUTPUT_GET]);

            publish_gpio_output_get_response(args.mosq, gpio_output_get_data_send, MSG_GPIO_OUTPUT_DONE);
            LOG_DEBUG(MSG_GPIO_OUTPUT_DONE);
            //lcd_show_message(MSG_GPIO_OUTPUT_DONE);
            return 0; //exito
        }

        if (!is_gpio_output_get_running) {
	    notify_gpio_output_get_status_message(args.mosq, MSG_GPIO_OUTPUT_STOPPED);
            return -1;
        }

        usleep(sleep_time_ms * 1000); // usleep recibe microsegundos (1 ms = 1000 us)
        elapsed += sleep_time_ms;
    }

    if (timeout_ms > 0 && elapsed >= timeout_ms) {
        notify_gpio_output_get_status_message(args.mosq, MSG_GPIO_OUTPUT_TIMEOUT_EXPIRED);
        return -2;
    }

    return -3; // sin exito pero sin timeout
}

// Aux 2
static void process_gpio_output_get_data_get(ThreadGpioOutputGetDataArgs args, GpioOutputGetData *gpio_output_get_data_send, int data_shd_index){

    for (int i = 0; i < args.gpio_output_get_data->num_output; i++) {
        int pin = args.gpio_output_get_data->output[i];
        gpio_output_get_data_send->state[i] = (shm->shared[data_shd_index] >> (pin + GPIO_OUTPUT_OFFSET_STATE)) & 1; // <-shm
        gpio_output_get_data_send->output[i] = pin;
        //LOG_DEBUG("state [i] %d",gpio_output_get_data_send->state[i]);
    }
    gpio_output_get_data_send->num_output = args.gpio_output_get_data->num_output;
    // Cargar timestamp en milisegundos
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint64_t ts_ms = (uint64_t)tv.tv_sec * 1000 + (tv.tv_usec / 1000);
    gpio_output_get_data_send->ts = ts_ms;

}

// Aux 3
static void notify_gpio_output_get_status_message(struct mosquitto *mosq, const char *msg) {
    publish_gpio_output_get_response_status(mosq, msg);
    LOG_INFO("%s", msg);
    lcd_show_message(msg);
}
