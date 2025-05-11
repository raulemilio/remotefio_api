#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "task_queue.h"
#include "log.h"

int task_queue_init(TaskQueue *queue) {
    if (pthread_mutex_init(&queue->mutex, NULL) != 0) return -1;
    if (pthread_cond_init(&queue->cond, NULL) != 0) return -1;
    queue->full = false;
    queue->shutdown = false;
    queue->item.data = NULL;
    queue->item.size = 0;
    return 0;
}

int task_queue_enqueue(TaskQueue *queue, void *data, size_t size) {
    pthread_mutex_lock(&queue->mutex);
    if (queue->full) {
        pthread_mutex_unlock(&queue->mutex);
        return -1; // Ya hay un elemento
    }

    queue->item.data = malloc(size);
    if (!queue->item.data) {
        pthread_mutex_unlock(&queue->mutex);
        return -1;
    }
    memcpy(queue->item.data, data, size);
    queue->item.size = size;
    queue->full = true;

    pthread_cond_signal(&queue->cond);
    pthread_mutex_unlock(&queue->mutex);
    return 0;
}

int task_queue_dequeue(TaskQueue *queue, void *buffer, size_t size) {
    pthread_mutex_lock(&queue->mutex);

    while (!queue->full && !queue->shutdown) {
        pthread_cond_wait(&queue->cond, &queue->mutex);
    }

    if (queue->shutdown) {
        pthread_mutex_unlock(&queue->mutex);
        return -1; // Señal para que el hilo termine
    }

    //Verificar si el tamanio solicitado es mayor al almacenado
    if (size > queue->item.size) {
        //LOG_ERROR("Error: tamanio solicitado (%zu) mayor que el tamanio almacenado en cola (%zu)\n", size, queue->item.size);
        pthread_mutex_unlock(&queue->mutex);
        return -1;
    }

    memcpy(buffer, queue->item.data, size);
    free(queue->item.data);
    queue->item.data = NULL;
    queue->item.size = 0;
    queue->full = false;

    pthread_mutex_unlock(&queue->mutex);
    return 0;
}

bool task_queue_is_full(TaskQueue *queue) {
    pthread_mutex_lock(&queue->mutex);
    bool result = queue->full;
    pthread_mutex_unlock(&queue->mutex);
    return result;
}

void task_queue_shutdown(TaskQueue *queue) {
    pthread_mutex_lock(&queue->mutex);
    queue->shutdown = true;
    pthread_cond_broadcast(&queue->cond); // Despierta a todos los hilos esperando
    pthread_mutex_unlock(&queue->mutex);
}

void task_queue_destroy(TaskQueue *queue) {
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->cond);

    // Por si acaso quedó algo sin liberar (en caso de error)
    if (queue->item.data) {
        free(queue->item.data);
        queue->item.data = NULL;
        queue->item.size = 0;
    }

    queue->full = false;
}
