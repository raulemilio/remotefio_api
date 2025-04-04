#ifndef MESSAGE_VALIDATOR_H
#define MESSAGE_VALIDATOR_H

#include <cjson/cJSON.h>
//JSON
#define ERR_NULL_PTR                    "Error: Payload or data structure is NULL\n"
#define ERR_INVALID_JSON                "Error: Payload is not a valid JSON\n"
#define ERR_INVALID_FORMAT              "Error: The payload must start with '{' and end with '}'\n"

#define ERR_INVALID_TASK                "Error: 'task' field not found or incorrect\n"

//ADC
#define ERR_INVALID_SAMPLES             "Error: 'Samples' field not found or not an integer\n"
#define ERR_MODE_OUT_OF_RANGE           "Error: The value of 'mode' (%d) is out of the allowed range (0-2)\n"
#define ERR_SAMPLE_RATE_OUT_OF_RANGE    "Error: Sample rate out of range: %d\n"
#define ERR_BUFFER_SIZE_OUT_OF_RANGE    "Error: Buffer size out of range: %d\n"
#define ERR_NUM_SAMPLES_OUT_OF_RANGE    "Error: Number of samples out of range: %d\n"
#define ERR_INVALID_TRIGGER             "Error: Invalid enable_external_trigger value.\n"
#define ERR_INVALID_SAMPLE_PERIOD       "Error: Sample period must be an integer.\n"

//INPUT
#define ERR_INVALID_INPUT               "Error: The 'input' field does not exist or is not an array\n"
#define ERR_INPUT_OUT_OF_RANGE          "Error: The 'input' array must have between 1 and 4 elements\n"
#define ERR_INVALID_INPUT_VALUE         "Error: The value at position %d in the 'input' array is not valid\n"

#define ERR_INVALID_MODE                "Error: The 'mode' field does not exist or is not valid\n"

//OUTPUT
#define ERR_INVALID_OUTPUT              "Error: The 'output' field does not exist or is not an array\n"
#define ERR_INVALID_OUTPUT_SIZE         "Error: The 'output' array must have between 1 and 4 elements\n"
#define ERR_INVALID_OUTPUT_VALUE        "Error: The value at position %d in the 'output' array is not valid\n"
#define ERR_INVALID_STATE               "Error: The 'state' field must exist and have the same number of elements as 'output'\n"
#define ERR_INVALID_STATE_VALUE         "Error: The value at position %d in the 'state' array is not valid\n"

//MOTOR
#define ERROR_MOTOR_ARRAY               "Error: The 'motor' field does not exist or is not an array\n"
#define ERROR_MOTOR_COUNT               "Error: Invalid number of motors (%d). Must be between 1 and 4\n"
#define ERROR_MOTOR_VALUE               "Error: The value at position %d in the 'motor' array is not valid\n"
#define ERROR_MODE2_FIELDS              "Error: The fields 'dir', 'ena', and 'step_time' are required in mode 2\n"
#define ERROR_ARRAY_SIZE                "Error: The fields 'dir', 'ena', and 'step_time' must have the same number of elements as 'motor'\n"
#define ERROR_DIR_VALUE                 "Error: The value at position %d in the 'dir' array is not 0 or 1\n"
#define ERROR_ENA_VALUE                 "Error: The value at position %d in the 'ena' array is not 0 or 1\n"
#define ERROR_STEP_TIME_VALUE           "Error: The value at position %d in the 'step_time' array is out of the allowed range (1-1000000)\n"

//SYSTEM
#define ERR_INVALID_FUNCTION            "Error: 'function' field not found or not a string\n"
#define ERR_INVALID_FUNCTION_VALUE      "Error: Invalid 'function' value\n"
#define ERR_INVALID_ACTION              "Error: 'action' field not found or not a string\n"
#define ERR_INVALID_ACTION_VALUE        "Error: Invalid 'action' value for the specified function\n"

#define JSON_KEY_TASK                      "task"
#define JSON_KEY_MODE                      "mode"

#define JSON_KEY_ADC_TASK_NAME             "adc"
#define JSON_KEY_ADC			   "adc"
#define JSON_KEY_SAMPLE_RATE 		   "sample_rate"
#define JSON_KEY_NUM_SAMPLES 		   "num_samples"
#define JSON_KEY_BUFFER_SIZE 		   "buffer_size"
#define JSON_KEY_ENABLE_TRIGGER 	   "enable_external_trigger"
#define JSON_KEY_SAMPLE_PERIOD             "sample_period"

#define JSON_KEY_GPIO_INPUT_TASK_NAME      "gpio_input"
#define JSON_KEY_INPUT                     "input"

#define JSON_KEY_GPIO_OUTPUT_TASK_NAME     "gpio_output"
#define JSON_KEY_OUTPUT                    "output"
#define JSON_KEY_STATE                     "state"
#define JSON_KEY_SAMPLES                   "samples"
#define JSON_KEY_SAMPLE_RATE               "sample_rate"

#define JSON_KEY_MOTOR_TASK_NAME           "motor"
#define JSON_KEY_MOTOR			   "motor"
#define JSON_KEY_MOTOR_DIR                 "dir"
#define JSON_KEY_MOTOR_ENA                 "ena"
#define JSON_KEY_MOTOR_STEP_TIME           "step_time"

#define JSON_KEY_SYSTEM_TASK_MANE          "system"
#define JSON_KEY_FUNCTION                  "function"
#define JSON_KEY_ACTION                    "action"

#define JSON_KEY_SYSTEM_LCD                "lcd"
#define JSON_KEY_SYSTEM_PRU0               "pru0"
#define JSON_KEY_SYSTEM_PRU1               "pru1"
#define JSON_KEY_SYSTEM_ADC		   "adc"
#define JSON_KEY_SYSTEM_GPIO_INPUT_MODE0   "gpio_input_m0"
#define JSON_KEY_SYSTEM_GPIO_INPUT_MODE1   "gpio_input_m1"
#define JSON_KEY_SYSTEM_GPIO_INPUT_MODE2   "gpio_input_m2"
#define JSON_KEY_SYSTEM_GPIO_OUTPUT_MODE0  "gpio_out_m0"
#define JSON_KEY_SYSTEM_GPIO_OUTPUT_MODE1  "gpio_out_m1"
#define JSON_KEY_SYSTEM_MOTOR_MODE0        "motor_m0"
#define JSON_KEY_SYSTEM_MOTOR_MODE1        "motor_m1"
#define JSON_KEY_SYSTEM_MOTOR_MODE2        "motor_m2"
#define JSON_KEY_SYSTEM_MOTOR_MODE3        "motor_m3"
#define JSON_KEY_SYSTEM_ALL_STOP_FUNCTIONS "all_functions"

#define JSON_KEY_SYSTEM_CLEAN              "clean"
#define JSON_KEY_SYSTEM_START              "start"
#define JSON_KEY_SYSTEM_STOP               "stop"
#define JSON_KEY_SYSTEM_PAUSE              "pause"

#define ADC_MODE_MIN 			0
#define ADC_MODE_MAX 			1
#define ADC_SAMPLE_RATE_MIN 		1
#define ADC_SAMPLE_RATE_MAX 		10000
#define ADC_BUFFER_SIZE_MIN 		4
#define ADC_BUFFER_SIZE_MAX 		512
#define ADC_NUM_SAMPLES_MIN 		4
#define ADC_NUM_SAMPLES_MAX 		5120
#define ADC_SAMPLE_PERIOD_BASE          96000000

#define GPIO_INPUT_CH_MIN 		0
#define GPIO_INPUT_CH_MAX 		3
#define GPIO_INPUT_NUM_MIN 		1
#define GPIO_INPUT_NUM_MAX 		4
#define GPIO_INPUT_MODE_MIN   		0
#define GPIO_INPUT_MODE_MAX   		2

#define GPIO_OUTPUT_CH_MIN 		0
#define GPIO_OUTPUT_CH_MAX 		3
#define GPIO_OUTPUT_NUM_MIN 		1
#define GPIO_OUTPUT_NUM_MAX 		4
#define GPIO_OUTPUT_LOW_LEVEL 		0
#define GPIO_OUTPUT_HIGH_LEVEL 		1
#define GPIO_OUTPUT_MODE_MIN   		0
#define GPIO_OUTPUT_MODE1		1 // valida campo state
#define GPIO_OUTPUT_MODE_MAX   		2

#define MOTOR_CH_MIN 			0
#define MOTOR_CH_MAX 			3
#define MOTOR_NUM_MIN 			1
#define MOTOR_NUM_MAX	 		4

#define MOTOR_DIR0 			0
#define MOTOR_DIR1 			1
#define MOTOR_ENA_DISABLE 		0
#define MOTOR_ENA_ENABLE  		1
#define MOTOR_STEP_TIME_MIN 		10
#define MOTOR_STEP_TIME_MAX 		1000000 // aprox 1,34 s de semiperiodo
#define MOTOR_MODE_MIN    		0
#define MOTOR_MODE_MAX 			3
#define MOTOR_MODE3			3

// Enum para las funciones
typedef enum {
    FUNC_LCD,
    FUNC_PRU0,
    FUNC_PRU1,
    FUNC_ADC,
    FUNC_GPIO_INPUT_MODE0,
    FUNC_GPIO_INPUT_MODE1,
    FUNC_GPIO_INPUT_MODE2,
    FUNC_GPIO_OUTPUT_MODE0,
    FUNC_GPIO_OUTPUT_MODE1,
    FUNC_MOTOR_MODE0,
    FUNC_MOTOR_MODE1,
    FUNC_MOTOR_MODE2,
    FUNC_MOTOR_MODE3,
    FUNC_ALL_STOP_FUNCTIONS,
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

// Estructura para almacenar los datos del mensaje

typedef struct {
    int mode;
    int sample_rate;
    int sample_period;
    int buffer_size;
    int num_samples;
    int enable_external_trigger;
} AdcData;

typedef struct {
    int input[GPIO_INPUT_NUM_MAX];  // Arreglo para almacenar los valores de las entradas (maximo 4 pines)
    int state[GPIO_INPUT_NUM_MAX];    // Arreglo para almacenar los estados de las entradas (maximo 4 estados)
    int num_input; // Numero de entradas leidas
    int mode;     // Modo de operacion (0, 1 o 2)
} GpioInputData;

typedef struct {
    int output[GPIO_OUTPUT_NUM_MAX];     // Arreglo para almacenar las salidas (maximo 4 pines)
    int state[GPIO_OUTPUT_NUM_MAX];    // Arreglo para almacenar los estados de las salidas (maximo 4 estados)
    int num_output;    // Numero de salidas procesados
    int mode;     // Modo de operacion (0 o 1)
} GpioOutputData;

// Estructura para almacenar los datos del mensaje
typedef struct {
    int num_motor;  // Numero de motores (1 a 4)
    int motor[MOTOR_NUM_MAX];   // Numero de motor (0 a 3) maximo 4
    int dir[MOTOR_NUM_MAX];     // Direccion de los motores, hasta 4 valores (0 o 1)
    int ena[MOTOR_NUM_MAX];     // Habilitacion de los motores, hasta 4 valores (0 o 1)
    int step_time[MOTOR_NUM_MAX];     // tiempo de medio ciclo de step
    int mode;       // Modo del motor
} MotorData;

typedef struct {
    FunctionType function; // Almacena el tipo de funcion (lcd, pru0, adc, etc.)
    ActionType action;     // Almacena la accion (clean, start, stop, etc.)
} SystemData;

int validate_adc_message(const char *payload, AdcData *adc_data);

int validate_gpio_input_message(const char *payload, GpioInputData *gpio_input_data);

int validate_gpio_output_message(const char *payload, GpioOutputData *gpio_output_data);

int validate_motor_message(const char *payload, MotorData *motor_data);

int validate_system_message(const char *payload, SystemData *system_data);

#endif // MESSAGE_VALIDATOR_H
