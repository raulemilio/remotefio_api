#ifndef SYSTEM_H
#define SYSTEM_H

#include <mosquitto.h>
#include "shared_memory.h"
#include "task_queue.h"
#include "adc.h"
#include "lcd_display.h"
#include "gpio_input.h"
#include "gpio_output.h"
#include "motor.h"

//SYSTEM
#define ERR_SYSTEM_INVALID_FUNCTION        "Err: function fld"
#define ERR_SYSTEM_INVALID_FUNCTION_VALUE  "Err: function val"
#define ERR_SYSTEM_INVALID_ACTION          "Err: action field"
#define ERR_SYSTEM_INVALID_ACTION_VALUE    "Err: action value"


#define MSG_SYSTEM_LCD_CLEAN		   "System lcd clr  "
#define MSG_SYSTEM_ALL_FUNCTION_ON	   "System all on   "
#define MSG_SYSTEM_ALL_FUNCTION_OFF	   "System all off  "
#define MSG_SYSTEM_FUNCTION_ON		   "System func on  "
#define MSG_SYSTEM_FUNCTION_OFF		   "System func off "

// 16 caracteres maximo
#define MSG_SYSTEM_INVALID_OPTION          "System inv opt  "
#define MSG_SYSTEM_DONE                    "System done     "
#define MSG_SYSTEM_STOPPED                 "System stopped  "
#define MSG_SYSTEM_TIMEOUT_EXPIRED         "System exp TO   "
#define MSG_SYSTEM_FINISH                  "System finish   "

// TASK
#define JSON_KEY_SYSTEM_TASK_MANE          "system"
#define JSON_KEY_SYSTEM_FUNCTION           "function"
#define JSON_KEY_SYSTEM_ACTION             "action"

#define JSON_KEY_SYSTEM_LCD                "lcd"
#define JSON_KEY_SYSTEM_PRU0               "pru0"
#define JSON_KEY_SYSTEM_PRU1               "pru1"
#define JSON_KEY_SYSTEM_ADC                "adc"
#define JSON_KEY_SYSTEM_GPIO_INPUT         "gpio_input"
#define JSON_KEY_SYSTEM_GPIO_OUTPUT_GET    "gpio_out_get"
#define JSON_KEY_SYSTEM_GPIO_OUTPUT_SET    "gpio_out_set"
#define JSON_KEY_SYSTEM_MOTOR_GET          "motor_get"
#define JSON_KEY_SYSTEM_MOTOR_SET          "motor_set"
#define JSON_KEY_SYSTEM_ALL_FUNCTIONS      "all_functions"

#define JSON_KEY_SYSTEM_CLEAN              "clean"
#define JSON_KEY_SYSTEM_START              "start"
#define JSON_KEY_SYSTEM_STOP               "stop"
#define JSON_KEY_SYSTEM_PAUSE              "pause"

extern volatile bool system_running;
extern pthread_mutex_t system_running_mutex;


extern TaskQueue system_queue;
extern pthread_t system_thread;

// Enum para las funciones
typedef enum {
    FUNC_LCD,
    FUNC_PRU0,
    FUNC_PRU1,
    FUNC_ADC,
    FUNC_GPIO_INPUT,
    FUNC_GPIO_OUTPUT_GET,
    FUNC_GPIO_OUTPUT_SET,
    FUNC_MOTOR_GET,
    FUNC_MOTOR_SET,
    FUNC_ALL_FUNCTIONS,
    FUNC_UNKNOWN // Para valores no validos
} FunctionType;

// Enum para las acciones
typedef enum {
    ACTION_CLEAN,
    ACTION_START,
    ACTION_STOP,
    ACTION_PAUSE,
    ACTION_UNKNOWN // Para acciones no validas
} ActionType;

typedef struct {
    FunctionType function; // Almacena el tipo de funcion (lcd, pru0, adc, etc.)
    ActionType action;     // Almacena la accion (clean, start, stop, etc.)
} SystemData;

// Argumentos enviados al hilo a traves de la cola
typedef struct {
    struct mosquitto *mosq;
    SystemData *system_data;
} ThreadSystemDataArgs;

// Funcion que corre como hilo para manejar gpio_input
void *system_thread_func(void *arg);

// Procesamiento del mensaje JSON (simulado ahora)
int handle_system_message(struct mosquitto *mosq, const char *payload, SystemData *data);

#endif
