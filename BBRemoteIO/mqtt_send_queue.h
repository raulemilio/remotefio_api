#ifndef MQTT_SEND_QUEUE_H
#define MQTT_SEND_QUEUE_H

#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>

#define SEND_QUEUE_MAX_ITEMS 128

typedef struct {
    void *data;
    size_t size;
} SendItem;

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond_nonempty;
    pthread_cond_t cond_nonfull;
    SendItem items[SEND_QUEUE_MAX_ITEMS];
    int head;
    int tail;
    int count;
} SendQueue;

int send_queue_init(SendQueue *queue);
int send_queue_enqueue(SendQueue *queue, void *data, size_t size);
int send_queue_dequeue(SendQueue *queue, void *buffer, size_t size);
bool send_queue_is_full(SendQueue *queue);
bool send_queue_is_empty(SendQueue *queue);

#endif // SEND_QUEUE_H
