#ifndef LCD_QUEUE_H
#define LCD_QUEUE_H

#include <pthread.h>
#include <stdbool.h>

#define LCD_MSG_MAX_LEN    32
#define LCD_QUEUE_CAPACITY 8  // Tamanio de la cola circular

typedef struct {
    char text[LCD_MSG_MAX_LEN];
} LcdMessage;

typedef struct {
    LcdMessage buffer[LCD_QUEUE_CAPACITY];
    int head;
    int tail;
    int count;
    bool shutdown;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
} LcdQueue;

void lcd_queue_init(LcdQueue *queue);
int lcd_queue_enqueue(LcdQueue *queue, const char *msg);
int lcd_queue_dequeue(LcdQueue *queue, LcdMessage *msg);
void lcd_queue_shutdown(LcdQueue *queue);
void lcd_queue_destroy(LcdQueue *queue);

#endif // LCD_QUEUE_H
