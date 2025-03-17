#include "send_msg_mqtt.h"
#include <stdio.h>
#include <string.h>

void send_gpio_data_mqtt(struct mosquitto *mosq, GpioReadData *gpio_data_send) {
    char json_payload[128];
    snprintf(json_payload, sizeof(json_payload),
             "{\"task\":\"gpio_read\",\"status\":\"ok\",\"pins\":[%d,%d,%d,%d]}",
             gpio_data_send->pins[0],
             gpio_data_send->pins[1],
             gpio_data_send->pins[2],
             gpio_data_send->pins[3]);

    mosquitto_publish(mosq, NULL, TOPIC_RSP_GPIO_READ, strlen(json_payload), json_payload, 0, false);
    printf("Enviando respuesta GPIO read: %s\n", json_payload);
}
