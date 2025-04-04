#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include <mosquitto.h>
#include <sys/types.h> // Necesario para off_t

#include "message_validator.h"// funciones auxiliares para procesar el payload json

#define BROKER_ADDRESS 			"192.168.7.1"
#define BROKER_PORT    			1883
#define CLIENT_ID      			"BeagleBoneClient"

#define TOPIC_CMDS_ADC 	        	"BBRemote/cmds/adc"
#define TOPIC_CMDS_GPIO_INPUT   	"BBRemote/cmds/gpio_input"
#define TOPIC_CMDS_GPIO_OUTPUT  	"BBRemote/cmds/gpio_output"
#define TOPIC_CMDS_MOTOR        	"BBRemote/cmds/motor"
#define TOPIC_CMDS_SYSTEM       	"BBRemote/cmds/system"

#define TOPIC_RSP_ADC 			"BBRemote/rsp/adc"
#define TOPIC_RSP_GPIO_INPUT 		"BBRemote/rsp/gpio_input"
#define TOPIC_RSP_GPIO_OUTPUT 		"BBRemote/rsp/gpio_output"
#define TOPIC_RSP_MOTOR 		"BBRemote/rsp/motor"
#define TOPIC_RSP_SYSTEM        	"BBRemote/rsp/system"
#define TOPIC_LOGS 			"BBRemote/logs"

#define MQTT_CALLBACK_RECEIVED_OK    	"BBRemote Message received format: OK"
#define MQTT_CALLBACK_RECEIVED_ERROR 	"Error: BBRemote Message received format"

// Definir paths y firmwares para PRU0 y PRU1
#define PRU0_PATH      			"/sys/class/remoteproc/remoteproc1"
#define PRU1_PATH      			"/sys/class/remoteproc/remoteproc2"

#define PRU0_FIRMWARE  			"am335x-pru0-analog-fw"
#define PRU1_FIRMWARE  			"am335x-pru1-digital-fw"

#define PRU_ERASE_MEM                   0

// SHARED INDEX
#define PRU_SHD_FLAGS_INDEX           	0 // shd0 flags function
#define PRU_SHD_GPIO_INPUT_MODE0_INDEX  1 // shd1 gpio_input data
#define PRU_SHD_GPIO_INPUT_MODE1_INDEX  2 // shd2 gpio_input data
#define PRU_SHD_GPIO_INPUT_MODE2_INDEX  3 // shD3 gpio_input data
#define PRU_SHD_GPIO_OUTPUT_MODE0_INDEX 4 // shd gpio_output data
#define PRU_SHD_GPIO_OUTPUT_MODE1_INDEX 5 // shd gpio_output data
#define PRU_SHD_MOTOR_MODE0_INDEX       6 // shd motor data
#define PRU_SHD_MOTOR_MODE1_INDEX       7 // shd motor data
#define PRU_SHD_MOTOR_MODE2_INDEX       8 // shd motor data
#define PRU_SHD_MOTOR_MODE3_INDEX       9 // shd motor data

// SHARED[0] BITS FLAGS
#define PRU_GPIO_INPUT_MODE0_FLAG     	0 //flag pru shd[0]
#define PRU_GPIO_INPUT_MODE1_FLAG     	1 //flag pru shd[0]
#define PRU_GPIO_INPUT_MODE2_FLAG       2 //flag pru shd[0]
#define PRU_GPIO_OUTPUT_MODE0_FLAG    	3 //flag pru shd[0]
#define PRU_GPIO_OUTPUT_MODE1_FLAG    	4 //flag pru shd[0]
#define PRU_MOTOR_MODE0_FLAG          	5 //flag pru shd[0]
#define PRU_MOTOR_MODE1_FLAG          	6 //flag pru shd[0]
#define PRU_MOTOR_MODE2_FLAG          	7 //flag pru shd[0]
#define PRU_MOTOR_MODE3_FLAG            8 //flag pru shd[0]

#define PRU_GPIO_INPUT_MODE0_DATARDY_FLAG     12  // flag complete shd[1] bit12
#define PRU_GPIO_INPUT_MODE1_DATARDY_FLAG     12  // flag complete shd[1] bit12
#define PRU_GPIO_INPUT_MODE2_DATARDY_FLAG     12  // flag complete shd[1] bit12

#define PRU_GPIO_OUTPUT_MODE0_DATARDY_FLAG    12  // flag complete shd[2] bit12
#define PRU_GPIO_OUTPUT_MODE1_DATARDY_FLAG    12  // flag complete shd[2] bit12
#define PRU_MOTOR_MODE0_DATARDY_FLAG          12  // flag complete shd[3] bit12
#define PRU_MOTOR_MODE1_DATARDY_FLAG          12  // flag complete shd[3] bit12
#define PRU_MOTOR_MODE2_DATARDY_FLAG          12  // flag complete shd[3] bit12
#define PRU_MOTOR_MODE3_DATARDY_FLAG          12  // flag complete shd[3] bit12

//PRUs
#define PRU0_ON_SUCCESS                         "PRU0 turned on successfully.\n"
#define PRU0_ON_ERROR                           "Error turning on PRU0.\n"
#define PRU0_OFF_SUCCESS                        "PRU0 turned off successfully.\n"
#define PRU0_OFF_ERROR                          "Error turning off PRU0.\n"

#define PRU1_ON_SUCCESS                         "PRU1 turned on successfully.\n"
#define PRU1_ON_ERROR                           "Error turning on PRU1.\n"
#define PRU1_OFF_SUCCESS                        "PRU1 turned off successfully.\n"
#define PRU1_OFF_ERROR                          "Error turning off PRU1.\n"

//MQTT
#define MSG_RECEIVED               		"Message received on %s: %s\n"
#define ERR_NULL_OR_EMPTY_MSG      		"Error: Received message is NULL or empty\n"
#define ERR_MSG_FORMAT             		"Error: Message received format"

#define ERR_THREAD_MEM_ALLOC       		"Error: Failed to allocate memory for thread arguments\n"
#define ERR_ADC_TASK_IN_PROGRESS   		"Error: Adc task already in progress. Another will not be executed.\n"
#define ERR_GPIO_INPUT_TASK_IN_PROGRESS 	"Error: Gpio_input task already in progress. Another will not be executed.\n"
#define ERR_GPIO_OUTPUT_TASK_IN_PROGRESS 	"Error: Gpio_output task already in progress. Another will not be executed.\n"
#define ERR_MOTOR_TASK_IN_PROGRESS 		"Error: Motor task already in progress. Another will not be executed.\n"
#define ERR_SYSTEM_TASK_IN_PROGRESS 		"Error: System task already in progress. Another will not be executed.\n"

#define ERR_ADC_DATA_MEM_ALLOC     		"Error: Adc failed to allocate memory for data\n"
#define ERR_GPIO_INPUT_DATA_MEM_ALLOC 		"Error: Gpio_input failed to allocate memory for data\n"
#define ERR_GPIO_OUTPUT_DATA_MEM_ALLOC 		"Error: Gpio_output failed to allocate memory for data\n"
#define ERR_MOTOR_DATA_MEM_ALLOC   		"Error: Motor failed to allocate memory for data\n"
#define ERR_SYSTEM_DATA_MEM_ALLOC  		"Error: System failed to allocate memory for data\n"

#define ERR_UNKNOWN_TOPIC          		"Error: Not a known topic\n"
#define ERR_THREAD_CREATION        		"Error: Failed to create thread\n"

// FUNCTION
#define FUNCTION_TIMEOUT                        10 // segundos
#define EXECUTION_ENABLED 			0  // Permite que otro sistema pueda generar una nueva instancia
#define EXECUTION_DISABLED 			1  // Bloquea la creacion de nuevas instancias
#define FUNCTION_INVALID_MODE                   "Invalid mode\n"
#define MODE_ALREADY_RUNNING 			"Mode %d is already running. Aborting new instance.\n"

//ADC
#define ADC_INVALID_OPTION       		"Error: Adc invalid option\n"
#define GPIO_INPUT_INVALID_OPTION               "Error: Gpio_input invalid option\n"
#define GPIO_OUTPUT_INVALID_OPTION              "Error: Gpio_output invalid option\n"
#define GPIO_MOTOR_INVALID_OPTION               "Error: Motor invalid option\n"
#define GPIO_SYSTEM_INVALID_OPTION              "Error: System invalid option\n"

#define ADC0_INPUT_TIMEOUT_EXPIRED              "Error: Adc expired time.\n"
#define GPIO_INPUT_TIMEOUT_EXPIRED      	"Error: Gpio_input expired time.\n"
#define GPIO_OUTPUT_TIMEOUT_EXPIRED             "Error: Gpio_output expired time.\n"
#define MOTOR_TIMEOUT_EXPIRED                   "Error: Motor expired time.\n"
#define SYSTEM_INVALID_OPTION                   "Error: System invalid option\n"

#define MAP_SIZE 16384UL  // 16KB para cubrir 4096 valores de 4 bytes
#define DATA_SIZE sizeof(uint32_t)  // Tamanio de cada dato (4 bytes)

typedef struct {
    struct mosquitto *mosq;
    AdcData *adc_data;
    GpioInputData *gpio_input_data;
    GpioOutputData *gpio_output_data;
    MotorData *motor_data;
    SystemData *system_data;
} ThreadArgs;

// Variables globales y compartidas de unica instancia (extern)
extern pthread_mutex_t gpio_mutex;

// Declaracion de la funcion callback
void message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message);

void *adc_function(void *arg);
void *gpio_input_function(void *arg);
void *gpio_output_function(void *arg);
void *motor_function(void *arg);
void *system_function(void *arg);

#endif // MQTT_HANDLER_H
