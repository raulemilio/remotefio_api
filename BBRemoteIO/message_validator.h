#ifndef JSON_UTILS_H
#define JSON_UTILS_H

#include <cjson/cJSON.h>
// Estructura para almacenar los datos del mensaje

typedef struct {
    int samples;       // Numero de muestras (1-1000)
    int mode;          // Modo de operacion (0, 1 o 2)
    int sample_rate;   // Frecuencia de muestreo (0-10000)
} AdcData;

typedef struct {
    int pins[4];  // Arreglo para almacenar los valores de los pines (maximo 4 pines)
    int num_pins; // Numero de pines leidos
    int mode;     // Modo de operacion (0, 1 o 2)
} GpioReadData;

typedef struct {
    int pins[4];     // Arreglo para almacenar los pines (maximo 4 pines)
    int state[4];    // Arreglo para almacenar los estados de los pines (maximo 4 estados)
    int num_pins;    // Numero de pines procesados
    int mode;     // Modo de operacion (0, 1 o 2)
} GpioWriteData;

// Estructura para almacenar los datos del mensaje
typedef struct {
    int num; // numero de motor 0 a 3
    int cfg[2]; // Enable Direccion
    int spd; // velocidad 0 menor velocidad valores a definir
    int mode; // Modo de operacion (0, 1, o 2)
} MotorData;

// ADC {"task":"adc","samples":10, "mode":1, "sample_rate":5000}
int validate_adc_message(const char *payload, AdcData *adc_data);

// GPIO READ {"task":"gpio_read","pins":[0,1,2,3], "mode":1}
int validate_gpio_read_message(const char *payload, GpioReadData *gpio_read_data);

// GPIO WRITE {"task":"gpio_write","pins":[0,1,2,3], "values": [0,0,0,0], "mode":1}
int validate_gpio_write_message(const char *payload, GpioWriteData *gpio_write_data);

// MOTOR {"task":"motor","num":0,"cfg":[0,0],"spd": 0, "mode": 1}
int validate_motor_message(const char *payload, MotorData *motor_data);

#endif // JSON_UTILS_H
