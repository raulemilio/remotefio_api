#ifndef MQTT_CALLBACK_H
#define MQTT_CALLBACK_H

#include <mosquitto.h>
#include <sys/types.h> // Necesario para off_t

#include "message_validator.h"// funciones auxiliares para procesar el payload json
#include "mqtt_publish.h"
#include "adc.h"
#include "gpio_input.h"
#include "gpio_output.h"
#include "motor_get.h"
#include "motor_set.h"
#include "system.h"

#define BROKER_ADDRESS 			"192.168.7.1"
#define BROKER_PORT    			1883
#define CLIENT_ID      			"BeagleBoneClient"

#define TOPIC_CMDS_ADC 	        	"BBRemote/cmds/adc"
#define TOPIC_CMDS_GPIO_INPUT   	"BBRemote/cmds/gpio/gpio_input"
#define TOPIC_CMDS_GPIO_OUTPUT  	"BBRemote/cmds/gpio/gpio_output"
#define TOPIC_CMDS_MOTOR_GET        	"BBRemote/cmds/motor/motor_get"
#define TOPIC_CMDS_MOTOR_SET            "BBRemote/cmds/motor/motor_set"
#define TOPIC_CMDS_SYSTEM       	"BBRemote/cmds/system"
#define TOPIC_LOGS                      "BBRemote/logs"

#define TOPIC_RSP_ADC 			"BBRemote/rsp/adc"
#define TOPIC_RSP_GPIO_INPUT 		"BBRemote/rsp/gpio/gpio_input"
#define TOPIC_RSP_GPIO_OUTPUT 		"BBRemote/rsp/gpio/gpio_output"
#define TOPIC_RSP_MOTOR_GET 		"BBRemote/rsp/motor/motor_get"
#define TOPIC_RSP_MOTOR_SET             "BBRemote/rsp/motor/motor_set"

#define TOPIC_RSP_SYSTEM        	"BBRemote/rsp/system"

#define MSG_RECEIVED                    "Msg recv: %s    "
#define MSG_RECEIVED_ON_TOPIC           "Message topic %s"
#define MSG_EMPTY_OR_NULL               "Empty or null   "
#define MSG_INVALID_FORMAT_RESPONSE     "Invalid format  "
#define MSG_UNKNOWN_TOPIC               "Unknown topic   "

#define MQTT_CALLBACK_RECEIVED_OK       "Message OK      "
#define MQTT_CALLBACK_RECEIVED_ERROR    "Message error   "

#define ERR_MALLOC_FUNCTION_DATA        "Malloc fail data"
#define ERR_MALLOC_THREAD_ARGS          "Malloc fail args"
#define INFO_FUNCTION_BUSY              "Busy, ignoring  "
#define INFO_FUNCTION_ENQUEUED          "Enqueued OK     "


#define TASK_RUNNING  true
#define TASK_STOPPED  false

// Declaracion de la variable mosq como extern (sin definicion)
extern struct mosquitto *mosq;

// Declaracion de la funcion callback
void message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message);

// Funciones auxiliares
int handle_adc_message(struct mosquitto *mosq, const char *message_payload, AdcData *adc_data);
int handle_gpio_input_message(struct mosquitto *mosq, const char *message_payload, GpioInputData *gpio_input_data);
int handle_gpio_output_message(struct mosquitto *mosq, const char *message_payload, GpioOutputData *gpio_output_data);
int handle_motor_get_message(struct mosquitto *mosq, const char *message_payload, MotorGetData *motor_get_data);
int handle_motor_set_message(struct mosquitto *mosq, const char *message_payload, MotorSetData *motor_set_data);
int handle_system_message(struct mosquitto *mosq, const char *message_payload, SystemData *system_data);

#endif // MQTT_CALBACK_H
