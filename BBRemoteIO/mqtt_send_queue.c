#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "mqtt_send_queue.h"
#include "log.h"


int send_queue_init(SendQueue *queue) {
    if (pthread_mutex_init(&queue->mutex, NULL) != 0) return -1;
    if (pthread_cond_init(&queue->cond_nonempty, NULL) != 0) return -1;
    if (pthread_cond_init(&queue->cond_nonfull, NULL) != 0) return -1;

    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
    queue->shutdown = false;

    for (int i = 0; i < SEND_QUEUE_MAX_ITEMS; i++) {
        queue->items[i].data = NULL;
        queue->items[i].size = 0;
    }
    return 0;
}

int send_queue_enqueue(SendQueue *queue, void *data, size_t size) {
    pthread_mutex_lock(&queue->mutex);

    while (queue->count == SEND_QUEUE_MAX_ITEMS && !queue->shutdown) {
        pthread_cond_wait(&queue->cond_nonfull, &queue->mutex);
    }

    if (queue->shutdown) {
        pthread_mutex_unlock(&queue->mutex);
        return -1;
    }

    queue->items[queue->tail].data = malloc(size);
    if (!queue->items[queue->tail].data) {
        pthread_mutex_unlock(&queue->mutex);
        return -1;
    }
    memcpy(queue->items[queue->tail].data, data, size);
    queue->items[queue->tail].size = size;

    queue->tail = (queue->tail + 1) % SEND_QUEUE_MAX_ITEMS;
    queue->count++;

    pthread_cond_signal(&queue->cond_nonempty);
    pthread_mutex_unlock(&queue->mutex);
    return 0;
}

int send_queue_dequeue(SendQueue *queue, void *buffer, size_t size) {
    pthread_mutex_lock(&queue->mutex);

    while (queue->count == 0 && !queue->shutdown) {
        pthread_cond_wait(&queue->cond_nonempty, &queue->mutex);
    }

    if (queue->count == 0 && queue->shutdown) {
        pthread_mutex_unlock(&queue->mutex);
        return -1;
    }

    if (size > queue->items[queue->head].size) {
        //LOG_ERROR("Error: tamanio solicitado (%zu) mayor que tamanio almacenado (%zu)\n", size, queue->items[queue->head].size);
        pthread_mutex_unlock(&queue->mutex);
        return -1;
    }

    memcpy(buffer, queue->items[queue->head].data, size);
    free(queue->items[queue->head].data);
    queue->items[queue->head].data = NULL;
    queue->items[queue->head].size = 0;

    queue->head = (queue->head + 1) % SEND_QUEUE_MAX_ITEMS;
    queue->count--;

    pthread_cond_signal(&queue->cond_nonfull);
    pthread_mutex_unlock(&queue->mutex);
    return 0;
}

bool send_queue_is_full(SendQueue *queue) {
    pthread_mutex_lock(&queue->mutex);
    bool result = (queue->count == SEND_QUEUE_MAX_ITEMS);
    pthread_mutex_unlock(&queue->mutex);
    return result;
}

bool send_queue_is_empty(SendQueue *queue) {
    pthread_mutex_lock(&queue->mutex);
    bool result = (queue->count == 0);
    pthread_mutex_unlock(&queue->mutex);
    return result;
}

void send_queue_shutdown(SendQueue *queue) {
    pthread_mutex_lock(&queue->mutex);
    queue->shutdown = true;
    pthread_cond_broadcast(&queue->cond_nonempty);
    pthread_cond_broadcast(&queue->cond_nonfull);
    pthread_mutex_unlock(&queue->mutex);
}

void send_queue_destroy(SendQueue *queue) {
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->cond_nonempty);
    pthread_cond_destroy(&queue->cond_nonfull);

    // Liberar datos que hayan quedado en la cola
    for (int i = 0; i < SEND_QUEUE_MAX_ITEMS; i++) {
        if (queue->items[i].data != NULL) {
            free(queue->items[i].data);
            queue->items[i].data = NULL;
            queue->items[i].size = 0;
        }
    }

    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
    queue->shutdown = false;
}
