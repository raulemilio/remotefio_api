#include "lcd_display.h"
#include "i2c_lcd.h"
#include <stdio.h>
#include <unistd.h>
#include "log.h"

#define LCD_SLEEP      100000 // 100 ms
#define LCD_MSG_SLEEP  500000 // 500 ms en prueba originalmente esta linea no la puse

LcdQueue lcd_display_queue;
pthread_t lcd_display_thread;

void lcd_show_message(const char *msg) {
    if (lcd_queue_enqueue(&lcd_display_queue, msg) == 0) {
        LOG_DEBUG(MSG_LCD_ENQUEUE_OK, msg);
    } else {
        LOG_ERROR(MSG_LCD_ENQUEUE_ERROR);
    }
}

void *lcd_display_thread_func(void *arg) {
    (void)arg;

    LcdMessage msg;

    if (lcd_set_i2c() == 0) {
        lcd_available = 1;
        lcd_init();
        lcd_clear_screen();
        lcd_cursor_pos(1, 1);
        lcd_put_str(MSG_LCD_SYSTEM_STATUS);
    } else {
        lcd_available = 0;
        LOG_ERROR(MSG_LCD_NOT_AVAILABLE);
    }

    while (1) {
        if (lcd_set_i2c() != 0) {
            if (lcd_available) {
                lcd_available = 0;
                LOG_WARN(MSG_LCD_DISCONNECTED);
            }
        } else if (!lcd_available) {
            lcd_available = 1;
            LOG_WARN(MSG_LCD_RECONNECTED);
            lcd_init();
            lcd_clear_screen();
            lcd_cursor_pos(1, 1);
            lcd_put_str(MSG_LCD_SYSTEM_STATUS);

            // Opcional: limpiar cola tras reconexion
            pthread_mutex_lock(&lcd_display_queue.mutex);
            lcd_display_queue.head = 0;
            lcd_display_queue.tail = 0;
            lcd_display_queue.count = 0;
            pthread_mutex_unlock(&lcd_display_queue.mutex);
        }

        if (lcd_available && lcd_queue_dequeue(&lcd_display_queue, &msg) == 0) {
            LOG_DEBUG(MSG_LCD_MESSAGE_RECEIVED, msg.text);
            lcd_cursor_pos(2, 1);
            lcd_put_str(MSG_LCD_ERASE_SCREEN); // limpiar linea
            lcd_cursor_pos(2, 1);
            lcd_put_str(msg.text);
            usleep(LCD_MSG_SLEEP);
        }

        usleep(LCD_SLEEP);
    }

    return NULL;
}
