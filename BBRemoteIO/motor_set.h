#ifndef MOTOR_SET_H
#define MOTOR_SET_H

#include <mosquitto.h>
#include "shared_memory.h"
#include "task_queue.h"
#include "motor.h"

#define MOTOR_SET_TIMEOUT_MS_MODE0   5000
#define MOTOR_SET_TIMEOUT_MS_MODE1   5000
#define MOTOR_SET_TIMEOUT_MS_MODE2   5000
#define MOTOR_SET_TIMEOUT_MS_MODE3   5000

#define MOTOR_SET_MODE_MIN           0
#define MOTOR_SET_MODE_MAX           1

// TASK
#define JSON_KEY_MOTOR_SET_TASK_NAME    "motor_set"

extern volatile bool motor_set_running;
extern pthread_mutex_t motor_set_running_mutex;

extern TaskQueue motor_set_queue;
extern pthread_t motor_set_thread;

// Estructura para almacenar los datos del mensaje
typedef struct {
    int num_motor;  // Numero de motores (1 a 4)
    int motor[MOTOR_NUM_MAX];   // Numero de motor (0 a 3) maximo 4
    int dir[MOTOR_NUM_MAX];     // Direccion de los motores, hasta 4 valores (0 o 1)
    int ena[MOTOR_NUM_MAX];     // Habilitacion de los motores, hasta 4 valores (0 o 1)
    int step_time[MOTOR_NUM_MAX];     // tiempo de medio ciclo de step
    int mode;       // Modo del motor
} MotorSetData;

// Argumentos enviados al hilo a traves de la cola
typedef struct {
    struct mosquitto *mosq;
    MotorSetData *motor_set_data;
} ThreadMotorSetDataArgs;

// Funcion que corre como hilo para manejar gpio_input
void *motor_set_thread_func(void *arg);

// Procesamiento del mensaje JSON (simulado ahora)
int handle_motor_set_message(struct mosquitto *mosq, const char *payload, MotorSetData *data);

#endif
