#ifndef GPIO_OUTPUT_GET_H
#define GPIO_OUTPUT_GET_H

#include <mosquitto.h>
#include "shared_memory.h"
#include "task_queue.h"
#include "gpio_output.h"

#define GPIO_OUTPUT_GET_TIMEOUT_MS_MODE0   5000

// TASK
#define JSON_KEY_GPIO_OUTPUT_GET_TASK_NAME     "gpio_output_get"

#define GPIO_OUTPUT_GET_MODE_MIN        0
#define GPIO_OUTPUT_GET_MODE_MAX        1

extern volatile bool gpio_output_get_running;
extern pthread_mutex_t gpio_output_get_running_mutex;

extern TaskQueue gpio_output_get_queue;
extern pthread_t gpio_output_get_thread;

typedef struct {
    int output[GPIO_OUTPUT_NUM_MAX];  // Arreglo para almacenar los valores de las entradas (maximo 4 pines)
    int state[GPIO_OUTPUT_NUM_MAX];    // Arreglo para almacenar los estados de las entradas (maximo 4 estados)
    int num_output; // Numero de entradas leidas
    int mode;     // Modo de operacion (0, 1 o 2)
    uint64_t ts;
} GpioOutputGetData;

// Argumentos enviados al hilo a traves de la cola
typedef struct {
    struct mosquitto *mosq;
    GpioOutputGetData *gpio_output_get_data;
} ThreadGpioOutputGetDataArgs;

// Funcion que corre como hilo para manejar gpio_output
void *gpio_output_get_thread_func(void *arg);

// Procesamiento del mensaje JSON (simulado ahora)
int handle_gpio_output_get_message(struct mosquitto *mosq, const char *payload, GpioOutputGetData *data);

#endif
