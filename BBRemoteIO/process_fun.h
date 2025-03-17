#ifndef GPIO_READ_H
#define GPIO_READ_H

#include <stdint.h>
#include "message_validator.h"
// Estructura para pasar los argumentos al hilo
typedef struct {
    struct mosquitto *mosq;
    AdcData adc_data;
    GpioReadData gpio_read_data;
    GpioWriteData gpio_write_data;
    MotorData motor_data;
} ThreadArgs;

// Declaracion de la funcion para leer datos GPIO
int read_gpio_data(ThreadArgs *thread_args, GpioReadData *gpio_data_send);

#endif // GPIO_READ_H
