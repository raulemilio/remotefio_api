#ifndef MQTT_PUBLISH_H
#define MQTT_PUBLISH_H

#include <mosquitto.h>
#include "mqtt_send_queue.h"

// Estructura para pasar datos a traves de la cola
typedef struct {
    struct mosquitto *mosq;
    char *topic;
    char *message;
} MqttPublishArgs;

extern SendQueue mqtt_publish_queue;
extern pthread_t mqtt_publish_thread;

// Funcion del hilo principal
void *mqtt_publish_thread_func(void *arg);

// Funcion para encolar publicacion asincrona
void mqtt_publish_async(struct mosquitto *mosq, const char *topic, const char *message);

#endif
