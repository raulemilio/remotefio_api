#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "mqtt_publish.h"
#include "mqtt_send_queue.h"
#include "log.h"

SendQueue mqtt_publish_queue;
pthread_t mqtt_publish_thread;

// Hilo que se encarga de publicar los mensajes
void *mqtt_publish_thread_func(void *arg) {
    MqttPublishArgs args;

    while (1) {
        if (send_queue_dequeue(&mqtt_publish_queue, &args, sizeof(MqttPublishArgs)) != 0) {
            break;  // Cola cerrada o error: terminar hilo
        }

        mosquitto_publish(args.mosq, NULL, args.topic, strlen(args.message), args.message, 0, false);
        free(args.topic);
        free(args.message);
    }

    return NULL;
}

// Funcion publica para encolar una publicacion
void mqtt_publish_async(struct mosquitto *mosq, const char *topic, const char *message) {
    if (!mosq || !topic || !message) return;

    MqttPublishArgs args = {
        .mosq = mosq,
        .topic = strdup(topic),
        .message = strdup(message)
    };

    if (!args.topic || !args.message) {
        LOG_ERROR("Error duplicating strings for MQTT publish");
        return;
    }

    if (send_queue_enqueue(&mqtt_publish_queue, &args, sizeof(MqttPublishArgs)) != 0) {
        LOG_ERROR("Error enqueuing MQTT message");
        free(args.topic);
        free(args.message);
    }
}
