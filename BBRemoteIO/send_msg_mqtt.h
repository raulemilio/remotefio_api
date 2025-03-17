#ifndef SEND_MSG_MQTT_H
#define SEND_MSG_MQTT_H

#include "process_fun.h"
#include <mosquitto.h>

#define TOPIC_RSP_GPIO_READ "bbremoteio/gpio_read/response"

void send_gpio_data_mqtt(struct mosquitto *mosq, GpioReadData *gpio_data_send);

#endif // SEND_MSG_MQTT_H
