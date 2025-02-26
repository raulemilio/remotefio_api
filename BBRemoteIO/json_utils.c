#include "json_utils.h"
#include <stdio.h>
#include <stdlib.h>

// ADC {"samples":10}  de 1 a 1000
int process_json_message_adc(const char *payload, AdcData *adc_data) {
    if (payload == NULL || adc_data == NULL) {
        printf("Error: Payload o estructura data es NULL\n");
        return -1;
    }
    // Parsear el JSON
    cJSON *root = cJSON_Parse(payload);
    if (!root) {
        printf("Error: Payload no es un JSON valido\n");
        return -1;
    }
    // Verificar si el campo "samples" existe
    cJSON *samples = cJSON_GetObjectItemCaseSensitive(root, "samples");
    if (!cJSON_IsNumber(samples)) {
        printf("Error: Campo 'samples' no encontrado o no es un numero entero\n");
        cJSON_Delete(root);
        return -1;
    }
    // Verificar si el ultimo caracter es '}'
    size_t len = strlen(payload);
    if (payload[len - 1] != '}') {
        printf("Error: El ultimo caracter del payload no es '}'\n");
        return -1;
    }
    // Obtener el valor de "samples"
    int sample_value = samples->valueint;
    if (sample_value < 1 || sample_value > 1000) {
        printf("Error: El valor de 'samples' (%d) esta fuera del rango permitido (1-1000)\n", sample_value);
        cJSON_Delete(root);
        return -1;
    }
    // Guardar el valor en la estructura
    adc_data->samples = sample_value;
    // JSON valido y valor en rango
    printf("Mensaje valido. samples = %d\n", sample_value);
    cJSON_Delete(root);
    return 0; //exito
}

// GPIO READ {"pins":[0,1,2,3]} pueden ser de 1 a 4
int process_json_message_gpio_read(const char *payload, GpioReadData *gpio_read_data) {
    if (payload == NULL || gpio_read_data == NULL) {
        printf("Error: Payload o estructura de datos es NULL\n");
        return -1;
    }
    // Parsear el JSON
    cJSON *root = cJSON_Parse(payload);
    if (!root) {
        printf("Error: Payload no es un JSON valido\n");
        return -1;
    }
    // Verificar si el ultimo caracter es '}'
    size_t len = strlen(payload);
    if (payload[len - 1] != '}') {
        printf("Error: El ultimo caracter del payload no es '}'\n");
        cJSON_Delete(root);
        return -1;
    }
    // Verificar si el campo "pins" existe
    cJSON *pins_array = cJSON_GetObjectItemCaseSensitive(root, "pins");
    if (!cJSON_IsArray(pins_array)) {
        printf("Error: El campo 'pins' no existe o no es un arreglo\n");
        cJSON_Delete(root);
        return -1;
    }
    // Verificar que el arreglo tenga entre 1 y 4 elementos
    int num_elements = cJSON_GetArraySize(pins_array);
    if (num_elements < 1 || num_elements > 4) {
        printf("Error: El arreglo 'pins' debe tener entre 1 y 4 elementos, pero tiene %d\n", num_elements);
        cJSON_Delete(root);
        return -1;
    }
    // Verificar que cada valor dentro del arreglo esta entre 0 y 3
    for (int i = 0; i < num_elements; i++) {
        cJSON *pin_item = cJSON_GetArrayItem(pins_array, i);
        if (!cJSON_IsNumber(pin_item)) {
            printf("Error: El valor en la posicion %d del arreglo 'pins' no es un numero\n", i);
            cJSON_Delete(root);
            return -1;
        }
        int pin_value = pin_item->valueint;
        if (pin_value < 0 || pin_value > 3) {
            printf("Error: El valor en la posicion %d del arreglo 'pins' (%d) esta fuera del rango permitido (0-3)\n", i, pin_value);
            cJSON_Delete(root);
            return -1;
        }
        // Guardar el valor en la estructura gpio_read_data
        gpio_read_data->pins[i] = pin_value;
    }

    // Guardar el numero de elementos en el arreglo
    gpio_read_data->num_pins = num_elements;

    // JSON valido y valores en rango
    printf("Mensaje valido. pins = [");
    for (int i = 0; i < num_elements; i++) {
        printf("%d", gpio_read_data->pins[i]);
        if (i < num_elements - 1) {
            printf(", ");
        }
    }
    printf("]\n");

    cJSON_Delete(root);
    return 0; //exito
}
// GPIO WRITE {"pins":[0,1,2,3], "values": [0,0,0,0]} puede ser de 1 a 4
int process_json_message_gpio_write(const char *payload, GpioWriteData *gpio_write_data) {
    if (payload == NULL || gpio_write_data == NULL) {
        printf("Error: Payload o estructura de datos es NULL\n");
        return -1;
    }

    // Parsear el JSON
    cJSON *root = cJSON_Parse(payload);
    if (!root) {
        printf("Error: Payload no es un JSON valido\n");
        return -1;
    }

    // Verificar si el ultimo caracter es '}'
    size_t len = strlen(payload);
    if (payload[len - 1] != '}') {
        printf("Error: El ultimo caracter del payload no es '}'\n");
        cJSON_Delete(root);
        return -1;
    }

    // Verificar si el campo "pins" existe
    cJSON *pins_array = cJSON_GetObjectItemCaseSensitive(root, "pins");
    if (!cJSON_IsArray(pins_array)) {
        printf("Error: El campo 'pins' no existe o no es un arreglo\n");
        cJSON_Delete(root);
        return -1;
    }

    // Verificar si el campo "state" existe
    cJSON *state_array = cJSON_GetObjectItemCaseSensitive(root, "state");
    if (!cJSON_IsArray(state_array)) {
        printf("Error: El campo 'state' no existe o no es un arreglo\n");
        cJSON_Delete(root);
        return -1;
    }

    // Verificar que ambos arreglos tengan la misma cantidad de elementos (deben estar relacionados 1 a 1)
    int num_pins = cJSON_GetArraySize(pins_array);
    int num_state = cJSON_GetArraySize(state_array);
    if (num_pins < 1 || num_pins > 4 || num_pins != num_state) {
        printf("Error: Los arreglos 'pins' y 'state' deben tener el mismo numero de elementos entre 1 y 4\n");
        cJSON_Delete(root);
        return -1;
    }

    // Verificar que los valores en el arreglo 'pins' estan entre 0 y 3
    for (int i = 0; i < num_pins; i++) {
        cJSON *pin_item = cJSON_GetArrayItem(pins_array, i);
        if (!cJSON_IsNumber(pin_item)) {
            printf("Error: El valor en la posicion %d del arreglo 'pins' no es un numero\n", i);
            cJSON_Delete(root);
            return -1;
        }

        int pin_value = pin_item->valueint;
        if (pin_value < 0 || pin_value > 3) {
            printf("Error: El valor en la posicion %d del arreglo 'pins' (%d) esta fuera del rango permitido (0-3)\n", i, pin_value);
            cJSON_Delete(root);
            return -1;
        }

        // Guardar el valor en la estructura gpio_write_data
        gpio_write_data->pins[i] = pin_value;
    }

    // Verificar que los valores en el arreglo 'state' sean solo 0 o 1
    for (int i = 0; i < num_pins; i++) {
        cJSON *state_item = cJSON_GetArrayItem(state_array, i);
        if (!cJSON_IsNumber(state_item)) {
            printf("Error: El valor en la posicion %d del arreglo 'state' no es un numero\n", i);
            cJSON_Delete(root);
            return -1;
        }

        int state_value = state_item->valueint;
        if (state_value != 0 && state_value != 1) {
            printf("Error: El valor en la posicion %d del arreglo 'state' (%d) no es 0 ni 1\n", i, state_value);
            cJSON_Delete(root);
            return -1;
        }

        // Guardar el valor en la estructura gpio_write_data
        gpio_write_data->state[i] = state_value;
    }

    // Guardar el numero de pines procesados
    gpio_write_data->num_pins = num_pins;

    // JSON valido y valores en rango
    printf("Mensaje valido. Pins y State:\n");
    for (int i = 0; i < num_pins; i++) {
        printf("Pin %d -> State %d\n", gpio_write_data->pins[i], gpio_write_data->state[i]);
    }

    cJSON_Delete(root);
    return 0; //exito
}

// MOTOR
int process_json_message_motor(const char *payload, MotorData *motor_data) {
    if (payload == NULL || motor_data == NULL) {
        printf("Error: Payload o estructura de datos es NULL\n");
        return -1;
    }

    // Parsear el JSON
    cJSON *root = cJSON_Parse(payload);
    if (!root) {
        printf("Error: Payload no es un JSON valido\n");
        return -1;
    }

    // Verificar que el ultimo caracter sea '}'
    size_t len = strlen(payload);
    if (payload[len - 1] != '}') {
        printf("Error: El ultimo caracter del payload no es '}'\n");
        cJSON_Delete(root);
        return -1;
    }

    // Verificar si el campo "num" existe
    cJSON *num_item = cJSON_GetObjectItemCaseSensitive(root, "num");
    if (!cJSON_IsNumber(num_item)) {
        printf("Error: El campo 'num' no existe o no es un numero\n");
        cJSON_Delete(root);
        return -1;
    }

    int num_value = num_item->valueint;
    if (num_value < 0 || num_value > 3) {
        printf("Error: El valor de 'num' (%d) esta fuera del rango permitido (0-3)\n", num_value);
        cJSON_Delete(root);
        return -1;
    }
    motor_data->num = num_value;

    // Verificar si el campo "cfg" existe y es un arreglo de 2 elementos
    cJSON *cfg_array = cJSON_GetObjectItemCaseSensitive(root, "cfg");
    if (!cJSON_IsArray(cfg_array) || cJSON_GetArraySize(cfg_array) != 2) {
        printf("Error: El campo 'cfg' no existe o no tiene exactamente 2 elementos\n");
        cJSON_Delete(root);
        return -1;
    }

    // Verificar que los valores en el arreglo "cfg" sean 0 o 1
    for (int i = 0; i < 2; i++) {
        cJSON *cfg_item = cJSON_GetArrayItem(cfg_array, i);
        if (!cJSON_IsNumber(cfg_item)) {
            printf("Error: El valor en la posicion %d del arreglo 'cfg' no es un numero\n", i);
            cJSON_Delete(root);
            return -1;
        }

        int cfg_value = cfg_item->valueint;
        if (cfg_value != 0 && cfg_value != 1) {
            printf("Error: El valor en la posicion %d del arreglo 'cfg' (%d) no es 0 ni 1\n", i, cfg_value);
            cJSON_Delete(root);
            return -1;
        }

        motor_data->cfg[i] = cfg_value;
    }

    // Verificar si el campo "spd" existe y es un numero valido
    cJSON *spd_item = cJSON_GetObjectItemCaseSensitive(root, "spd");
    if (!cJSON_IsNumber(spd_item)) {
        printf("Error: El campo 'spd' no existe o no es un numero\n");
        cJSON_Delete(root);
        return -1;
    }

    int spd_value = spd_item->valueint;
    if (spd_value < 0 || spd_value > 9) {
        printf("Error: El valor de 'spd' (%d) esta fuera del rango permitido (0-9)\n", spd_value);
        cJSON_Delete(root);
        return -1;
    }
    motor_data->spd = spd_value;

    // JSON valido y valores en rango
    printf("Mensaje valido. num = %d, cfg = [%d, %d], spd = %d\n",
           motor_data->num, motor_data->cfg[0], motor_data->cfg[1], motor_data->spd);

    cJSON_Delete(root);
    return 0; //exito
}
