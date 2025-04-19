#ifndef GPIO_OUTPUT_H
#define GPIO_OUTPUT_H

#include <mosquitto.h>
#include "shared_memory.h"
#include "task_queue.h"


#define GPIO_OUTPUT_TIMEOUT_MS_MODE0   5000
#define GPIO_OUTPUT_TIMEOUT_MS_MODE1   5000
#define GPIO_OUTPUT_TIMEOUT_MS_MODE2   5000

//OUTPUT
#define ERR_INVALID_OUTPUT                 "Err: output fld "
#define ERR_INVALID_OUTPUT_SIZE            "Err: output size"
#define ERR_INVALID_OUTPUT_VALUE           "Err: output val "
#define ERR_INVALID_STATE                  "Err: state field"
#define ERR_INVALID_STATE_VALUE            "Err: state value"

#define MSG_GPIO_OUTPUT_INVALID_MODE       "Gpio out inv mod"
#define MSG_GPIO_OUTPUT_DONE               "Gpio out done   "
#define MSG_GPIO_OUTPUT_STOPPED            "Gpio out stopped"
#define MSG_GPIO_OUTPUT_TIMEOUT_EXPIRED    "Expired timeout "
#define MSG_GPIO_OUTPUT_FINISH             "Gpio out finish"

// TASK
#define JSON_KEY_GPIO_OUTPUT_TASK_NAME     "gpio_output"
#define JSON_KEY_OUTPUT                    "output"

#define GPIO_OUTPUT_CH_MIN              0
#define GPIO_OUTPUT_CH_MAX              3
#define GPIO_OUTPUT_NUM_MIN             1
#define GPIO_OUTPUT_NUM_MAX             4
#define GPIO_OUTPUT_LOW_LEVEL           0
#define GPIO_OUTPUT_HIGH_LEVEL          1
#define GPIO_OUTPUT_MODE_MIN            0
#define GPIO_OUTPUT_MODE1               1 // valida campo state
#define GPIO_OUTPUT_MODE_MAX            2

extern volatile bool gpio_output_running;
extern pthread_mutex_t gpio_output_running_mutex;

extern TaskQueue gpio_output_queue;
extern pthread_t gpio_output_thread;

typedef struct {
    int output[GPIO_OUTPUT_NUM_MAX];  // Arreglo para almacenar los valores de las entradas (maximo 4 pines)
    int state[GPIO_OUTPUT_NUM_MAX];    // Arreglo para almacenar los estados de las entradas (maximo 4 estados)
    int num_output; // Numero de entradas leidas
    int mode;     // Modo de operacion (0, 1 o 2)
} GpioOutputData;

// Argumentos enviados al hilo a traves de la cola
typedef struct {
    struct mosquitto *mosq;
    GpioOutputData *gpio_output_data;
} ThreadGpioOutputDataArgs;

// Funcion que corre como hilo para manejar gpio_output
void *gpio_output_thread_func(void *arg);

// Procesamiento del mensaje JSON (simulado ahora)
int handle_gpio_output_message(struct mosquitto *mosq, const char *payload, GpioOutputData *data);

#endif
