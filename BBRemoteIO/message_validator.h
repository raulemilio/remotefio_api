#ifndef MESSAGE_VALIDATOR_H
#define MESSAGE_VALIDATOR_H

#include "adc.h"
#include "gpio_input.h"
#include "gpio_output_get.h"
#include "gpio_output_set.h"
#include "motor_get.h"
#include "motor_set.h"
#include "system.h"

#define MQTT_REPORT_JSON_FORMAT    "{\"Log\":\"%s\"}"
#define MQTT_REPORT_JSON_SIZE      256

//JSON
#define ERR_NULL_PTR               "Err: null ptr   "
#define ERR_INVALID_JSON           "Err: inv JSON   "
#define ERR_INVALID_FORMAT         "Err: inv format "

#define ERR_INVALID_TASK           "Err: inv task   "
#define ERR_INVALID_MODE           "Err: inv mode   "

#define JSON_KEY_TASK              "task"
#define JSON_KEY_MODE              "mode"

int validate_adc_message(struct mosquitto *mosq, const char *payload, AdcData *adc_data);

int validate_gpio_input_message(struct mosquitto *mosq, const char *payload, GpioInputData *gpio_input_data);

int validate_gpio_output_get_message(struct mosquitto *mosq, const char *payload, GpioOutputGetData *gpio_output_get_data);

int validate_gpio_output_set_message(struct mosquitto *mosq, const char *payload, GpioOutputSetData *gpio_output_set_data);

int validate_motor_get_message(struct mosquitto *mosq, const char *payload, MotorGetData *motor_get_data);

int validate_motor_set_message(struct mosquitto *mosq, const char *payload, MotorSetData *motor_set_data);

int validate_system_message(struct mosquitto *mosq, const char *payload, SystemData *system_data);

void mqtt_report(struct mosquitto *mosq, const char *topic, const char *message);

#endif // MESSAGE_VALIDATOR_H
