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
#include <sys/mman.h>
#include <errno.h>
#include <time.h>

#include "mqtt_callback.h"
#include "mqtt_response.h"
#include "pru_control.h"

#include "motor.h"
#include "motor_set.h"
#include "task_queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mqtt_publish.h"
#include "lcd_display.h"
#include "log.h"

#define MOTOR_SET_SLEEP_TIME_MS 10

volatile bool motor_set_running;
pthread_mutex_t motor_set_running_mutex = PTHREAD_MUTEX_INITIALIZER;

TaskQueue motor_set_queue;
pthread_t motor_set_thread;

// mode 0
static void motor_set_on_demand_set(ThreadMotorSetDataArgs args,
                              int flag_index,
                              int trigger_flag,
                              int data_index,
                              int datardy_flag,
                              int timeout_ms);
// aux 1
static int wait_for_motor_set_data_and_process_done(ThreadMotorSetDataArgs args,
                                                          int data_index,
                                                          int datardy_flag,
                                                          int timeout_ms);
// Aux 2
static void process_motor_set_data_set(ThreadMotorSetDataArgs args, int data_shd_index);
// Aux 3
static void notify_motor_set_status_message(struct mosquitto *mosq, const char *msg);


void *motor_set_thread_func(void *arg) {
    ThreadMotorSetDataArgs args;
    char message[128];
    int timeout_ms;

    while (1) {
        if (task_queue_dequeue(&motor_set_queue, &args, sizeof(ThreadMotorSetDataArgs)) == 0) {
            if (!args.motor_set_data) continue;

            snprintf(message, sizeof(message), "Motor set mode %d", args.motor_set_data->mode);
            notify_motor_set_status_message(args.mosq, message);

            int flag_index, data_index, trigger_flag, datardy_flag;

            switch (args.motor_set_data->mode) {
                case 0:
                    // logica de modo 0
                    flag_index = PRU_SHD_MOTOR_SET_MODE0_FLAG_INDEX;
                    data_index = PRU_SHD_MOTOR_SET_MODE0_DATA_INDEX;
                    trigger_flag = PRU_MOTOR_SET_MODE0_FLAG;
                    datardy_flag = PRU_MOTOR_SET_MODE0_DATARDY_FLAG;
                    timeout_ms = MOTOR_SET_TIMEOUT_MS_MODE0;
                    motor_set_on_demand_set(args, flag_index,trigger_flag,data_index,datardy_flag,timeout_ms);
                    break;
                default:
                    notify_motor_set_status_message(args.mosq, MSG_MOTOR_INVALID_MODE);
                    break;
            }

            pthread_mutex_lock(&motor_set_running_mutex);
            motor_set_running = TASK_STOPPED;
            pthread_mutex_unlock(&motor_set_running_mutex);

	    notify_motor_set_status_message(args.mosq, MSG_MOTOR_FINISH);
            free(args.motor_set_data);
            //Limpieza del struct para evitar basura en la proxima iteracion
            memset(&args, 0, sizeof(args));
        }
    }
    return NULL;
}

// mode 0
static void motor_set_on_demand_set(ThreadMotorSetDataArgs args,
                              int flag_index,
                              int trigger_flag,
                              int data_index,
                              int datardy_flag,
                              int timeout_ms)
{
    // write data into shared
    pthread_mutex_lock(&shm->shared_mutex[MUTEX_MOTOR_SET]);
    process_motor_set_data_set(args, data_index);
    shm->shared[flag_index] |= (1 << trigger_flag); // aviso al pru
    pthread_mutex_unlock(&shm->shared_mutex[MUTEX_MOTOR_SET]);

    // Esperamos que cumpla la tarea
    wait_for_motor_set_data_and_process_done(args,data_index, datardy_flag, timeout_ms);
}

// aux 1
static int wait_for_motor_set_data_and_process_done(ThreadMotorSetDataArgs args,
                                                          int data_index,
                                                          int datardy_flag,
                                                          int timeout_ms){

    int elapsed = 0;
    int sleep_time_ms = MOTOR_SET_SLEEP_TIME_MS;
    bool is_motor_set_running = false;

    while (timeout_ms < 0 || elapsed < timeout_ms) {

        pthread_mutex_lock(&motor_set_running_mutex);
        is_motor_set_running = motor_set_running;
        pthread_mutex_unlock(&motor_set_running_mutex);

        pthread_mutex_lock(&shm->shared_mutex[MUTEX_MOTOR_SET]);
        int ready = shm->shared[data_index] & (1 << datardy_flag);
        pthread_mutex_unlock(&shm->shared_mutex[MUTEX_MOTOR_SET]);

        if (ready) {
            notify_motor_set_status_message(args.mosq, MSG_MOTOR_DONE);
            return 0; //exito
        }

        if (!is_motor_set_running) {
            notify_motor_set_status_message(args.mosq, MSG_MOTOR_STOPPED);
            return -1;
        }

        usleep(sleep_time_ms * 1000); // usleep recibe microsegundos (1 ms = 1000 us)
        elapsed += sleep_time_ms;

    }

    if (timeout_ms > 0 && elapsed >= timeout_ms) {
        notify_motor_set_status_message(args.mosq, MSG_MOTOR_TIMEOUT_EXPIRED);
        return -2;
    }

    return -3; // sin exito pero sin timeout
}

// Aux 2
static void process_motor_set_data_set(ThreadMotorSetDataArgs args, int data_shd_index){

    for (int i = 0; i < args.motor_set_data->num_motor; i++) {
        int motor = args.motor_set_data->motor[i];
        if (motor >= 0 && motor < 4){
            shm->shared[data_shd_index] |= (1 << motor);
        }
        if (i < 4){
            shm->shared[data_shd_index] |= ((args.motor_set_data->ena[i] & 1) << (MOTOR_OFFSET_ENABLE + (2 * motor)));
            shm->shared[data_shd_index] |= ((args.motor_set_data->dir[i] & 1) << (MOTOR_OFFSET_DIRECTION + (2 * motor)));
            shm->shared[MOTOR_OFFSET_STEPTIME_SHD_INDEX + i] = args.motor_set_data->step_time[i];
        }
    }
}

// Aux 3
static void notify_motor_set_status_message(struct mosquitto *mosq, const char *msg) {
    publish_motor_set_response_status(mosq, msg);
    LOG_INFO("%s", msg);
    lcd_show_message(msg);
}
