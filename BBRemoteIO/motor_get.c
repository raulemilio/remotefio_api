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
#include <sys/time.h>

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

#define MOTOR_GET_SLEEP_TIME_MS 5

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
    char message[128];
    int timeout_ms;

    while (1) {
        if (task_queue_dequeue(&motor_get_queue, &args, sizeof(ThreadMotorGetDataArgs)) == 0) {
            if (!args.motor_get_data) continue;

            snprintf(message, sizeof(message), "Motor get mode %d", args.motor_get_data->mode);
            notify_motor_get_status_message(args.mosq, message);

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

    pthread_mutex_lock(&shm->shared_mutex[MUTEX_MOTOR_GET]);
    shm->shared[flag_index] &= ~(1 << trigger_flag);  // Detener pru function mode
    pthread_mutex_unlock(&shm->shared_mutex[MUTEX_MOTOR_GET]);

}

// aux 1
static int wait_for_motor_get_data_and_process_get(ThreadMotorGetDataArgs args,
                                          int data_index,
                                          int datardy_flag,
                                          int timeout_ms,
                                          int clear_after_read){

    int elapsed = 0;
    int sleep_time_ms = MOTOR_GET_SLEEP_TIME_MS;
    bool is_motor_get_running = false;
    MotorGetData motor_get_data_send;

    // variables internas que no se almacenan en pru
    // Cargamos con los valores globales los datos de salida, luego segun corresponda se modificaran
    pthread_mutex_lock(&motor_data_mutex);
    for (int motor = 0; motor <= MOTOR_CH_MAX; motor++) {
        motor_get_data_send.rpm[motor] = rpm[motor];
        motor_get_data_send.step_per_rev[motor] = step_per_rev[motor];
        motor_get_data_send.micro_step[motor] = micro_step[motor];
    }
    pthread_mutex_unlock(&motor_data_mutex);

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
            LOG_DEBUG(MSG_MOTOR_DONE);
            //lcd_show_message(MSG_MOTOR_DONE);
            return 0; //exito
        }

        if (!is_motor_get_running) {
            notify_motor_get_status_message(args.mosq, MSG_MOTOR_STOPPED);
            return -1;
        }

        usleep(sleep_time_ms * 1000); // usleep recibe microsegundos (1 ms = 1000 us)
        elapsed += sleep_time_ms;

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
        motor_get_data_send->factor_step_time[i] = shm->shared[(MOTOR_OFFSET_STEPTIME_SHD_INDEX + motor)];

        // variables internas que no se almacenan en pru
        pthread_mutex_lock(&motor_data_mutex);
        motor_get_data_send->rpm[i] = rpm[motor];
        motor_get_data_send->step_per_rev[i] = step_per_rev[motor];
        motor_get_data_send->micro_step[i] = micro_step[motor];
        pthread_mutex_unlock(&motor_data_mutex);
    }
    motor_get_data_send->num_motor = args.motor_get_data->num_motor;
    // Cargar timestamp en milisegundos
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint64_t ts_ms = (uint64_t)tv.tv_sec * 1000 + (tv.tv_usec / 1000);
    motor_get_data_send->ts = ts_ms;
}
// Aux 3
static void notify_motor_get_status_message(struct mosquitto *mosq, const char *msg) {
    publish_motor_get_response_status(mosq, msg);
    LOG_INFO("%s", msg);
    lcd_show_message(msg);
}
