#ifndef MESSAGE_VALIDATOR_H
#define MESSAGE_VALIDATOR_H

#include <cjson/cJSON.h>
#include "adc.h"
#include "gpio_input.h"
#include "gpio_output.h"
#include "motor_get.h"
#include "motor_set.h"
#include "system.h"

//JSON
#define ERR_NULL_PTR                    "Null pointer    "
#define ERR_INVALID_JSON                "Invalid JSON    "
#define ERR_INVALID_FORMAT              "Invalid format  "
#define ERR_INVALID_TASK                "Invalid task    "
#define ERR_INVALID_MODE                "Invalid mode    "

#define JSON_KEY_TASK                   "task"
#define JSON_KEY_MODE                   "mode"

int validate_adc_message(struct mosquitto *mosq, const char *payload, AdcData *adc_data);

int validate_gpio_input_message(struct mosquitto *mosq, const char *payload, GpioInputData *gpio_input_data);

int validate_gpio_output_message(struct mosquitto *mosq, const char *payload, GpioOutputData *gpio_output_data);

int validate_motor_get_message(struct mosquitto *mosq, const char *payload, MotorGetData *motor_get_data);

int validate_motor_set_message(struct mosquitto *mosq, const char *payload, MotorSetData *motor_set_data);

int validate_system_message(struct mosquitto *mosq, const char *payload, SystemData *system_data);

#endif // MESSAGE_VALIDATOR_H
