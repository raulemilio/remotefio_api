#include <stdio.h>
#include <rc/i2c.h>
#include "lcd_system_status.h"
#include "i2c_lcd.h"

int lcd_system_init(void){

    if (lcd_set_i2c()){
        printf("lcd_set_i2c() failed!!!\n");
        return -1;
    }
    printf("Running LCD...\n");
    lcd_init();

    // Limpiar la pantalla antes de actualizar
    lcd_clear_screen();

    // Primera linea del LCD
    lcd_cursor_pos(1, 1);

    lcd_put_str("System status");
    return 0;
}

void lcd_system_end(void){
    // Limpiar la pantalla antes de actualizar
    lcd_clear_screen();
    // cierra LCD
    lcd_end();
    // libera puerto
    rc_i2c_close(0);
}

// Funcion para limpiar la pantalla del LCD
void system_lcd_clear_screen(void) {
    // Limpiar la pantalla antes de actualizar
    lcd_clear_screen();
}

// Funcion para mostrar el mensaje de estado en el LCD
void lcd_system_status(const char *status_message) {

    // Segunda linea del LCD
    lcd_cursor_pos(LINE2, LCD_OFFSET);

    // Mostrar el mensaje en la segunda linea
    lcd_put_str(status_message);
}
