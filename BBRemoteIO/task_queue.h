#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include <pthread.h>
#include <stdbool.h>

typedef struct {
    void *data;
    size_t size;
} TaskItem;

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    bool full;
    TaskItem item;
} TaskQueue;

int task_queue_init(TaskQueue *queue);
int task_queue_enqueue(TaskQueue *queue, void *data, size_t size);
int task_queue_dequeue(TaskQueue *queue, void *buffer, size_t size);
bool task_queue_is_full(TaskQueue *queue);

#endif // TASK_QUEUE_H
