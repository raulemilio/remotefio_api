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
#include "motor_get.h"
#include "task_queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mqtt_publish.h"
#include "lcd_display.h"
#include "log.h"

volatile bool motor_get_running;
pthread_mutex_t motor_get_running_mutex = PTHREAD_MUTEX_INITIALIZER;

TaskQueue motor_get_queue;
pthread_t motor_get_thread;

// mode 0
static void motor_get_on_demand_get(ThreadMotorGetDataArgs args,
                                int flag_index,
                                int trigger_flag,
                                int data_index,
                                int datardy_flag,
                                int timeout_ms);
// mode 1 y 2
static void motor_get_trigger_get(ThreadMotorGetDataArgs args,
                              int flag_index,
                              int trigger_flag,
                              int data_index,
                              int datardy_flag,
                              int timeout_ms);
// aux 1
static int wait_for_motor_get_data_and_process_get(ThreadMotorGetDataArgs args,
                                          int data_index,
                                          int datardy_flag,
                                          int timeout_ms,
                                          int clear_after_read);
// Aux2
static void process_motor_get_data_get(ThreadMotorGetDataArgs args, MotorGetData *motor_get_data_send, int data_shd_index);

// Aux 3
static void notify_motor_get_status_message(struct mosquitto *mosq, const char *msg);


void *motor_get_thread_func(void *arg) {
    ThreadMotorGetDataArgs args;
    char mensaje[128];
    int timeout_ms;

    while (1) {
        if (task_queue_dequeue(&motor_get_queue, &args, sizeof(ThreadMotorGetDataArgs)) == 0) {
            if (!args.motor_get_data) continue;

            LOG_INFO("Function: motor_get -> mode %d running", args.motor_get_data->mode);
            snprintf(mensaje, sizeof(mensaje), "Motor_get mode %d", args.motor_get_data->mode);
            lcd_show_message(mensaje);

            int flag_index, data_index, trigger_flag, datardy_flag;

            switch (args.motor_get_data->mode) {
                case 0:
                    // logica de modo 0
                    flag_index = PRU_SHD_MOTOR_GET_MODE0_FLAG_INDEX;
                    data_index = PRU_SHD_MOTOR_GET_MODE0_DATA_INDEX;
                    trigger_flag = PRU_MOTOR_GET_MODE0_FLAG;
                    datardy_flag = PRU_MOTOR_GET_MODE0_DATARDY_FLAG;
                    timeout_ms = MOTOR_GET_TIMEOUT_MS_MODE0;
                    motor_get_on_demand_get(args, flag_index, trigger_flag, data_index, datardy_flag, timeout_ms);
                    break;
                case 1:
                    // logica de modo 1
                    flag_index = PRU_SHD_MOTOR_GET_MODE1_FLAG_INDEX;
                    data_index = PRU_SHD_MOTOR_GET_MODE1_DATA_INDEX;
                    trigger_flag = PRU_MOTOR_GET_MODE1_FLAG;
                    datardy_flag = PRU_MOTOR_GET_MODE1_DATARDY_FLAG;
                    timeout_ms = MOTOR_GET_TIMEOUT_MS_MODE1;
                    motor_get_trigger_get(args, flag_index,trigger_flag,data_index,datardy_flag,timeout_ms);
                    break;
                case 2:
                    // logica de modo 2
                    flag_index = PRU_SHD_MOTOR_GET_MODE2_FLAG_INDEX;
                    data_index = PRU_SHD_MOTOR_GET_MODE2_DATA_INDEX;
                    trigger_flag = PRU_MOTOR_GET_MODE2_FLAG;
                    datardy_flag = PRU_MOTOR_GET_MODE2_DATARDY_FLAG;
                    timeout_ms = MOTOR_GET_TIMEOUT_MS_MODE2;
                    motor_get_trigger_get(args,flag_index,trigger_flag,data_index,datardy_flag,timeout_ms);
                    break;
                default:
                    notify_motor_get_status_message(args.mosq, MSG_MOTOR_INVALID_MODE);
                    break;
            }

            pthread_mutex_lock(&motor_get_running_mutex);
            motor_get_running = TASK_STOPPED;
            pthread_mutex_unlock(&motor_get_running_mutex);

	    notify_motor_get_status_message(args.mosq, MSG_MOTOR_FINISH);
            free(args.motor_get_data);
            //Limpieza del struct para evitar basura en la proxima iteracion
            memset(&args, 0, sizeof(args));
        }
    }
    return NULL;
}

// mode 0
static void motor_get_on_demand_get(ThreadMotorGetDataArgs args,
                                int flag_index,
                                int trigger_flag,
                                int data_index,
                                int datardy_flag,
                                int timeout_ms){
    // Avisamos a pru
    pthread_mutex_lock(&shm->shared_mutex[MUTEX_MOTOR_GET]);
    shm->shared[flag_index] |= (1 << trigger_flag);
    pthread_mutex_unlock(&shm->shared_mutex[MUTEX_MOTOR_GET]);
    // esperamos dato
    wait_for_motor_get_data_and_process_get(args, data_index, datardy_flag, timeout_ms, 0);
}
// mode 1 y 2
static void motor_get_trigger_get(ThreadMotorGetDataArgs args,
                              int flag_index,
                              int trigger_flag,
                              int data_index,
                              int datardy_flag,
                              int timeout_ms){
    // Avisamos a pru
    pthread_mutex_lock(&shm->shared_mutex[MUTEX_MOTOR_GET]);
    shm->shared[flag_index] |= (1 << trigger_flag);
    pthread_mutex_unlock(&shm->shared_mutex[MUTEX_MOTOR_GET]);

    bool is_motor_get_running = false;

    while (1) {

        pthread_mutex_lock(&motor_get_running_mutex);
        is_motor_get_running = motor_get_running;
        pthread_mutex_unlock(&motor_get_running_mutex);

        pthread_mutex_lock(&shm->shared_mutex[MUTEX_MOTOR_GET]);
        int trigger_active = shm->shared[flag_index] & (1 << trigger_flag);
        pthread_mutex_unlock(&shm->shared_mutex[MUTEX_MOTOR_GET]);

        if (!is_motor_get_running || !trigger_active) {
            notify_motor_get_status_message(args.mosq, MSG_MOTOR_STOPPED);
            break;
        }
        // entramos a un loop de medicion
        int result = wait_for_motor_get_data_and_process_get(args, data_index, datardy_flag, timeout_ms, 1);
        if (result != 0) break; // salir si hubo timeout, parada o error
    }
}

// aux 1
static int wait_for_motor_get_data_and_process_get(ThreadMotorGetDataArgs args,
                                          int data_index,
                                          int datardy_flag,
                                          int timeout_ms,
                                          int clear_after_read){

    int elapsed = 0;
    bool is_motor_get_running = false;
    MotorGetData motor_get_data_send;

    while (timeout_ms < 0 || elapsed < timeout_ms) {

        pthread_mutex_lock(&motor_get_running_mutex);
        is_motor_get_running = motor_get_running;
        pthread_mutex_unlock(&motor_get_running_mutex);

        pthread_mutex_lock(&shm->shared_mutex[MUTEX_MOTOR_GET]);
        int ready = shm->shared[data_index] & (1 << datardy_flag);
        pthread_mutex_unlock(&shm->shared_mutex[MUTEX_MOTOR_GET]);

        if (ready) {
            pthread_mutex_lock(&shm->shared_mutex[MUTEX_MOTOR_GET]);
            process_motor_get_data_get(args, &motor_get_data_send, data_index);
            if (clear_after_read)
                shm->shared[data_index] = PRU_ERASE_MEM;
            pthread_mutex_unlock(&shm->shared_mutex[MUTEX_MOTOR_GET]);

            publish_motor_get_response(args.mosq, motor_get_data_send, MSG_MOTOR_DONE);
            LOG_INFO(MSG_MOTOR_DONE);
            lcd_show_message(MSG_MOTOR_DONE);
            return 0; //exito
        }

        if (!is_motor_get_running) {
            notify_motor_get_status_message(args.mosq, MSG_MOTOR_STOPPED);
            return -1;
        }

        usleep(1000);  // 1 ms
        elapsed += 1;
    }

    if (timeout_ms > 0 && elapsed >= timeout_ms) {
        notify_motor_get_status_message(args.mosq, MSG_MOTOR_TIMEOUT_EXPIRED);
        return -2;
    }

    return -3; // sin exito pero sin timeout
}

// Aux 2
static void process_motor_get_data_get(ThreadMotorGetDataArgs args, MotorGetData *motor_get_data_send, int data_shd_index){

    for (int i = 0; i < args.motor_get_data->num_motor; i++) {
        int motor = args.motor_get_data->motor[i];
        motor_get_data_send->ena[i] = (shm->shared[data_shd_index] >> ((2*motor) + MOTOR_OFFSET_ENABLE)) & 1; // <-shm
        motor_get_data_send->motor[i] = motor;
        motor_get_data_send->dir[i] = (shm->shared[data_shd_index] >> ((2*motor) + MOTOR_OFFSET_DIRECTION)) & 1;
        motor_get_data_send->step_time[i] = shm->shared[(MOTOR_OFFSET_STEPTIME_SHD_INDEX + motor)];
    }
    motor_get_data_send->num_motor = args.motor_get_data->num_motor;
}
// Aux 3
static void notify_motor_get_status_message(struct mosquitto *mosq, const char *msg) {
    publish_motor_get_response_status(mosq, msg);
    LOG_INFO("%s", msg);
    lcd_show_message(msg);
}
