#ifndef MQTT_RESPONSE_H
#define MQTT_RESPONSE_H

#include <mosquitto.h>
#include "mqtt_callback.h"
#include "adc.h"
#include "gpio_input.h"
#include "gpio_output.h"
#include "motor.h"
#include "motor_get.h"
#include "motor_set.h"
#include "system.h"

// send function status
void publish_adc_response_status(struct mosquitto *mosq, const char *status);
void publish_gpio_input_response_status(struct mosquitto *mosq, const char *status);
void publish_gpio_output_response_status(struct mosquitto *mosq, const char *status);
void publish_motor_get_response_status(struct mosquitto *mosq, const char* status);
void publish_motor_set_response_status(struct mosquitto *mosq, const char* status);
void publish_system_response_status(struct mosquitto *mosq, const char *status);

// send data
void publish_gpio_input_response(struct mosquitto *mosq, GpioInputData gpio_input_data_send, const char *status);
void publish_gpio_output_response(struct mosquitto *mosq, GpioOutputData gpio_output_data_send, const char *status);
void publish_motor_get_response(struct mosquitto *mosq, MotorGetData motor_get_data_send, const char *status);
void publish_motor_set_response(struct mosquitto *mosq, MotorSetData motor_set_data_send, const char *status);

#endif // MQTT_RESPONSE_H
