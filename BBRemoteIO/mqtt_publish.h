#ifndef MQTT_PUBLISH_H
#define MQTT_PUBLISH_H

#include <mosquitto.h>

// Estructura para pasar datos a traves de la cola
typedef struct {
    struct mosquitto *mosq;
    char *topic;
    char *message;
} MqttPublishArgs;

// Funcion para inicializar hilo y cola
void mqtt_publish_init(void);

// Funcion para encolar publicacion asincrona
void mqtt_publish_async(struct mosquitto *mosq, const char *topic, const char *message);

#endif
