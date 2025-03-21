#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include <mosquitto.h>

// Declaracion de la funcion callback
void message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message);

#endif // MQTT_HANDLER_H
