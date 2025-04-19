#ifndef I2C_LCD_H
#define I2C_LCD_H

#include <stdint.h>

#define I2C_ADDR        0x27
#define BUS             2

#define LCD_BACKLIGHT   0x08
#define ENABLE          0x04

#define LCD_CMD         0
#define LCD_DATA        1

#define EN              0b00000100
#define RW              0b00000010
#define RS              0b00000001

#define LCD_LINE_1      0x80
#define LCD_LINE_2      0xC0
#define LCD_WIDTH       16

#define INIT_DELAY      5000
#define CMD_DELAY       100
#define EN_CYCLE_DELAY  100
#define EN_PULSE_WIDTH  100

extern int lcd_available;

int  lcd_set_i2c(void);
void lcd_send_nibble(const uint8_t nibble);
void lcd_send_byte(const uint8_t byte, uint8_t mode);
void lcd_init(void);
void lcd_end(void);
void lcd_clear_screen(void);
void lcd_cursor_pos(const uint8_t line, const uint8_t column);
void lcd_put_char(const char ch);
void lcd_put_str(const char *str);

#endif
