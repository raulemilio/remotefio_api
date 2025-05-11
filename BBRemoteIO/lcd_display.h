#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include <pthread.h>
#include "lcd_queue.h"
#include "log.h"

// LCD display
#define MSG_LCD_SYSTEM_STATUS      "System status"
#define MSG_LCD_ERASE_SCREEN       "                "
#define MSG_LCD_ENQUEUE_OK         "LCD -> Enqueue ok"
#define MSG_LCD_ENQUEUE_ERROR      "LCD -> Error: queue full"
#define MSG_LCD_NOT_AVAILABLE      "LCD not available"
#define MSG_LCD_DISCONNECTED       "LCD disconnected"
#define MSG_LCD_RECONNECTED        "LCD reconnected"
#define MSG_LCD_MESSAGE_RECEIVED   "LCD -> '%s'"

#define MSG_LCD_PRU0_ON_SUCCESS    "PRU0 on         "
#define MSG_LCD_PRU0_ON_ERROR      "PRU0 on error   "
#define MSG_LCD_PRU0_OFF_SUCCESS   "PRU0 off        "
#define MSG_LCD_PRU0_OFF_ERROR     "PRU0 off error  "

#define MSG_LCD_PRU1_ON_SUCCESS    "PRU1 on         "
#define MSG_LCD_PRU1_ON_ERROR      "PRU1 on error   "
#define MSG_LCD_PRU1_OFF_SUCCESS   "PRU1 off        "
#define MSG_LCD_PRU1_OFF_ERROR     "PRU1 off error  "

extern LcdQueue lcd_display_queue;
extern pthread_t lcd_display_thread;

void lcd_show_message(const char *msg);
void *lcd_display_thread_func(void *arg);

#endif // LCD_DISPLAY_H
