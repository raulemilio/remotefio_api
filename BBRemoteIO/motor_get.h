#ifndef MOTOR_GET_H
#define MOTOR_GET_H

#include <mosquitto.h>
#include "shared_memory.h"
#include "task_queue.h"
#include "motor.h"

#define MOTOR_GET_TIMEOUT_MS_MODE0   5000
#define MOTOR_GET_TIMEOUT_MS_MODE1   5000
#define MOTOR_GET_TIMEOUT_MS_MODE2   5000

#define MOTOR_GET_MODE_MIN           0
#define MOTOR_GET_MODE_MAX           2

// TASK
#define JSON_KEY_MOTOR_GET_TASK_NAME  "motor_get"

extern volatile bool motor_get_running;
extern pthread_mutex_t motor_get_running_mutex;

extern TaskQueue motor_get_queue;
extern pthread_t motor_get_thread;

// Estructura para almacenar los datos del mensaje
typedef struct {
    int num_motor;  // Numero de motores (1 a 4)
    int motor[MOTOR_NUM_MAX];   // Numero de motor (0 a 3) maximo 4
    int dir[MOTOR_NUM_MAX];     // Direccion de los motores, hasta 4 valores (0 o 1)
    int ena[MOTOR_NUM_MAX];     // Habilitacion de los motores, hasta 4 valores (0 o 1)
    int step_time[MOTOR_NUM_MAX];     // tiempo de medio ciclo de step
    int mode;       // Modo del motor
    uint64_t ts;
} MotorGetData;

// Argumentos enviados al hilo a traves de la cola
typedef struct {
    struct mosquitto *mosq;
    MotorGetData *motor_get_data;
} ThreadMotorGetDataArgs;

// Funcion que corre como hilo para manejar gpio_input
void *motor_get_thread_func(void *arg);

// Procesamiento del mensaje JSON (simulado ahora)
int handle_motor_get_message(struct mosquitto *mosq, const char *payload, MotorGetData *data);

#endif
