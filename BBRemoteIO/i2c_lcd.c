#include <stdio.h>
#include <unistd.h>
#include <rc/i2c.h>
#include <string.h>
#include "i2c_lcd.h"
#include "log.h"

int lcd_available = 0;

int lcd_set_i2c() {
    static int last_status = -1;

    // Cerrar si estaba abierto previamente
    if (last_status == 0) {
        rc_i2c_close(BUS);
    }

    if (rc_i2c_init(BUS, I2C_ADDR) == 0) {
        last_status = 0;
        return 0;
    } else {
        last_status = -1;
        return -1;
    }
}

void lcd_send_nibble(const uint8_t nibble) {
    uint8_t buf;
    int ret;

    buf = nibble | EN;
    usleep(EN_CYCLE_DELAY);
    ret = rc_i2c_send_byte(BUS, buf);
    if (ret != 0) {
        lcd_available = 0;
        return;
    }

    usleep(EN_PULSE_WIDTH);

    buf = nibble & ~EN;
    ret = rc_i2c_send_byte(BUS, buf);
    if (ret != 0) {
        lcd_available = 0;
        return;
    }

    usleep(EN_CYCLE_DELAY);
}

void lcd_send_byte(const uint8_t byte, uint8_t mode) {
    if (!lcd_available) return;

    uint8_t high_nibble = mode | (byte & 0xF0) | (LCD_BACKLIGHT & (~RW));
    uint8_t low_nibble  = mode | ((byte << 4) & 0xF0) | (LCD_BACKLIGHT & (~RW));

    lcd_send_nibble(high_nibble);
    usleep(CMD_DELAY);
    lcd_send_nibble(low_nibble);
    usleep(CMD_DELAY);
}

void lcd_init(void) {
    lcd_send_byte(0b00110011, LCD_CMD);
    usleep(INIT_DELAY);
    lcd_send_byte(0b00110010, LCD_CMD);
    usleep(INIT_DELAY);
    lcd_send_byte(0b00000110, LCD_CMD);
    usleep(INIT_DELAY);
    lcd_send_byte(0b00001100, LCD_CMD);
    usleep(INIT_DELAY);
    lcd_send_byte(0b00101000, LCD_CMD);
    usleep(INIT_DELAY);
    lcd_send_byte(0b00000001, LCD_CMD);
    usleep(INIT_DELAY);
}

void lcd_end(void) {
    rc_i2c_close(BUS);
    lcd_available = 0;
}

void lcd_clear_screen(void) {
    lcd_send_byte(0b00000001, LCD_CMD);
    usleep(INIT_DELAY);
}

void lcd_cursor_pos(const uint8_t line, const uint8_t column) {
    uint8_t buf;
    if (line == 1) {
        buf = LCD_LINE_1 + column - 1;
        lcd_send_byte(buf, LCD_CMD);
    } else if (line == 2) {
        buf = LCD_LINE_2 + column - 1;
        lcd_send_byte(buf, LCD_CMD);
    } else {
        LOG_ERROR("LCD cursor line param greater than 2!!!");
    }
}

void lcd_put_char(const char ch) {
    lcd_send_byte(ch, LCD_DATA);
}

void lcd_put_str(const char *str) {
    if (strlen(str) <= LCD_WIDTH) {
        while (*str) {
            lcd_send_byte(*str, LCD_DATA);
            str++;
        }
    } else {
        LOG_WARN("Number of chars in string is greater than screen!!!");
    }
}
