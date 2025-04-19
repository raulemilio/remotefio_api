#ifndef GPIO_INPUT_H
#define GPIO_INPUT_H

#include <mosquitto.h>
#include "shared_memory.h"
#include "task_queue.h"

#define GPIO_INPUT_TIMEOUT_MS_MODE0   5000
#define GPIO_INPUT_TIMEOUT_MS_MODE1   5000
#define GPIO_INPUT_TIMEOUT_MS_MODE2   5000

//INPUT
#define ERR_INVALID_INPUT                 "Err: input field"
#define ERR_INPUT_OUT_OF_RANGE            "Err: input size "
#define ERR_INVALID_INPUT_VALUE           "Err: input value"

// 16 caracteres maximo
#define MSG_GPIO_INPUT_INVALID_MODE       "Gpio in inv mod "
#define MSG_GPIO_INPUT_DONE               "Gpio in done"
#define MSG_GPIO_INPUT_STOPPED            "Gpio in stopped "
#define MSG_GPIO_INPUT_TIMEOUT_EXPIRED    "Expired timeout "
#define MSG_GPIO_INPUT_FINISH             "Gpio in finish  "

// TASK
#define JSON_KEY_GPIO_INPUT_TASK_NAME      "gpio_input"
#define JSON_KEY_INPUT                     "input"

#define GPIO_INPUT_NUM_MAX              4
#define GPIO_INPUT_CH_MIN               0
#define GPIO_INPUT_CH_MAX               3
#define GPIO_INPUT_NUM_MIN              1
#define GPIO_INPUT_MODE_MIN             0
#define GPIO_INPUT_MODE_MAX             2

extern volatile bool gpio_input_running;
extern pthread_mutex_t gpio_input_running_mutex;

extern TaskQueue gpio_input_queue;
extern pthread_t gpio_input_thread;

typedef struct {
    int input[GPIO_INPUT_NUM_MAX];  // Arreglo para almacenar los valores de las entradas (maximo 4 pines)
    int state[GPIO_INPUT_NUM_MAX];    // Arreglo para almacenar los estados de las entradas (maximo 4 estados)
    int num_input; // Numero de entradas leidas
    int mode;     // Modo de operacion (0, 1 o 2)
} GpioInputData;

// Argumentos enviados al hilo a traves de la cola
typedef struct {
    struct mosquitto *mosq;
    GpioInputData *gpio_input_data;
} ThreadGpioInputDataArgs;

// Funcion que corre como hilo para manejar gpio_input
void *gpio_input_thread_func(void *arg);

// Procesamiento del mensaje JSON (simulado ahora)
int handle_gpio_input_message(struct mosquitto *mosq, const char *payload, GpioInputData *data);

#endif
