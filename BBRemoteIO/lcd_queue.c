#include "lcd_queue.h"
#include <string.h>

void lcd_queue_init(LcdQueue *queue) {
    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->not_empty, NULL);
}

int lcd_queue_enqueue(LcdQueue *queue, const char *msg) {
    pthread_mutex_lock(&queue->mutex);
    if (queue->count == LCD_QUEUE_CAPACITY) {
        pthread_mutex_unlock(&queue->mutex);
        return -1; // Cola llena
    }

    strncpy(queue->buffer[queue->tail].text, msg, LCD_MSG_MAX_LEN - 1);
    queue->buffer[queue->tail].text[LCD_MSG_MAX_LEN - 1] = '\0';

    queue->tail = (queue->tail + 1) % LCD_QUEUE_CAPACITY;
    queue->count++;

    pthread_cond_signal(&queue->not_empty);
    pthread_mutex_unlock(&queue->mutex);
    return 0;
}

int lcd_queue_dequeue(LcdQueue *queue, LcdMessage *msg) {
    pthread_mutex_lock(&queue->mutex);
    while (queue->count == 0) {
        pthread_cond_wait(&queue->not_empty, &queue->mutex);
    }

    *msg = queue->buffer[queue->head];
    queue->head = (queue->head + 1) % LCD_QUEUE_CAPACITY;
    queue->count--;

    pthread_mutex_unlock(&queue->mutex);
    return 0;
}
