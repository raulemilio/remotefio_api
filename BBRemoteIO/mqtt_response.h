#ifndef MQTT_RESPONSE_H
#define MQTT_RESPONSE_H

#include <mosquitto.h>
#include "mqtt_callback.h"

#define FUNCTION_STATUS_ERROR             "Error: Function not completed"
#define FUNCTION_STATUS_OK                "Function completed"

// send function status
void publish_system_response_status(struct mosquitto *mosq, const char *status);

void publish_gpio_input_response_status(struct mosquitto *mosq, const char *status);

void publish_gpio_output_response_status(struct mosquitto *mosq, const char *status);

void publish_motor_response_status(struct mosquitto *mosq, const char* status);

// send data
void publish_gpio_input_response(struct mosquitto *mosq, GpioInputData gpio_input_data_send, const char *status);

void publish_gpio_output_response(struct mosquitto *mosq, GpioOutputData gpio_output_data_send, const char *status);

void publish_motor_response(struct mosquitto *mosq, MotorData motordata_send, const char *status);

#endif // MQTT_RESPONSE_H
