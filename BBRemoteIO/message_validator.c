#include "message_validator.h"
#include <stdio.h>
#include <stdlib.h>

// ADC {"task":"adc","samples":10, "mode":1, "sample_rate":5000}
int validate_adc_message(const char *payload, AdcData *adc_data) {
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
    // Verificar si el campo "task" existe y es "adc"
    cJSON *task = cJSON_GetObjectItemCaseSensitive(root, "task");
    if (!cJSON_IsString(task) || strcmp(task->valuestring, "adc") != 0) {
        printf("Error: Campo 'task' no encontrado o no es 'adc'\n");
        cJSON_Delete(root);
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
        printf("Error: El ultimo caracter del payload no es '}\n");
        return -1;
    }
    // Obtener el valor de "samples"
    int sample_value = samples->valueint;
    if (sample_value < 1 || sample_value > 1000) {
        printf("Error: El valor de 'samples' (%d) esta fuera del rango permitido (1-1000)\n", sample_value);
        cJSON_Delete(root);
        return -1;
    }
    adc_data->samples = sample_value;

    // Verificar si el campo "mode" existe y es valido (0, 1 o 2)
    cJSON *mode_item = cJSON_GetObjectItemCaseSensitive(root, "mode");
    if (!cJSON_IsNumber(mode_item)) {
        printf("Error: El campo 'mode' no existe o no es un numero\n");
        cJSON_Delete(root);
        return -1;
    }
    int mode_value = mode_item->valueint;
    if (mode_value < 0 || mode_value > 2) {
        printf("Error: El valor de 'mode' (%d) esta fuera del rango permitido (0-2)\n", mode_value);
        cJSON_Delete(root);
        return -1;
    }
    adc_data->mode = mode_value;

    // Verificar si el campo "sample_rate" existe y es valido (0-10000)
    cJSON *sample_rate_item = cJSON_GetObjectItemCaseSensitive(root, "sample_rate");
    if (!cJSON_IsNumber(sample_rate_item)) {
        printf("Error: El campo 'sample_rate' no existe o no es un numero\n");
        cJSON_Delete(root);
        return -1;
    }
    int sample_rate_value = sample_rate_item->valueint;
    if (sample_rate_value < 0 || sample_rate_value > 10000) {
        printf("Error: El valor de 'sample_rate' (%d) esta fuera del rango permitido (0-10000)\n", sample_rate_value);
        cJSON_Delete(root);
        return -1;
    }
    adc_data->sample_rate = sample_rate_value;

    // JSON valido y valores en rango
    printf("Mensaje valido. samples = %d, mode = %d, sample_rate = %d\n", adc_data->samples, adc_data->mode, adc_data->sample_rate);

    cJSON_Delete(root);
    return 0; //exito
}

// GPIO READ {"task":"gpio_read","pins":[0,1,2,3], "mode":1} pueden ser de 1 a 4
int validate_gpio_read_message(const char *payload, GpioReadData *gpio_read_data) {
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
        printf("Error: El ultimo caracter del payload no es '}\n");
        cJSON_Delete(root);
        return -1;
    }
    // Verificar si el campo "task" existe y es "gpio_read"
    cJSON *task = cJSON_GetObjectItemCaseSensitive(root, "task");
    if (!cJSON_IsString(task) || strcmp(task->valuestring, "gpio_read") != 0) {
        printf("Error: Campo 'task' no encontrado o no es 'gpio_read'\n");
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

    // Verificar si el campo "mode" existe y es valido (0, 1 o 2)
    cJSON *mode_item = cJSON_GetObjectItemCaseSensitive(root, "mode");
    if (!cJSON_IsNumber(mode_item)) {
        printf("Error: El campo 'mode' no existe o no es un numero\n");
        cJSON_Delete(root);
        return -1;
    }
    int mode_value = mode_item->valueint;
    if (mode_value < 0 || mode_value > 2) {
        printf("Error: El valor de 'mode' (%d) esta fuera del rango permitido (0-2)\n", mode_value);
        cJSON_Delete(root);
        return -1;
    }
    gpio_read_data->mode = mode_value;

    // JSON valido y valores en rango
    printf("Mensaje valido. pins = [");
    for (int i = 0; i < num_elements; i++) {
        printf("%d", gpio_read_data->pins[i]);
        if (i < num_elements - 1) {
            printf(", ");
        }
    }
    printf("], mode = %d\n", gpio_read_data->mode);

    cJSON_Delete(root);
    return 0; //exito
}

// GPIO WRITE {"task":"gpio_write","pins":[0,1,2,3], "state": [0,0,0,0],, "mode":1} puede ser de 1 a 4
int validate_gpio_write_message(const char *payload, GpioWriteData *gpio_write_data) {
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
    // Verificar si el campo "task" existe y es "gpio_write"
    cJSON *task = cJSON_GetObjectItemCaseSensitive(root, "task");
    if (!cJSON_IsString(task) || strcmp(task->valuestring, "gpio_write") != 0) {
        printf("Error: Campo 'task' no encontrado o no es 'gpio_write'\n");
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

    // Verificar si el campo "mode" existe y es valido (0, 1 o 2)
    cJSON *mode_item = cJSON_GetObjectItemCaseSensitive(root, "mode");
    if (!cJSON_IsNumber(mode_item)) {
        printf("Error: El campo 'mode' no existe o no es un numero\n");
        cJSON_Delete(root);
        return -1;
    }
    int mode_value = mode_item->valueint;
    if (mode_value < 0 || mode_value > 2) {
        printf("Error: El valor de 'mode' (%d) esta fuera del rango permitido (0-2)\n", mode_value);
        cJSON_Delete(root);
        return -1;
    }
    gpio_write_data->mode = mode_value;

    // JSON valido y valores en rango
    printf("Mensaje valido. Pins y State:\n");
    for (int i = 0; i < num_pins; i++) {
        printf("Pin %d -> State %d\n", gpio_write_data->pins[i], gpio_write_data->state[i]);
    }

    cJSON_Delete(root);
    return 0; //exito
}
// MOTOR {"task":"motor","motor":[0,1,2,3],"dir":[0,1,0,1],"ena":[1,1,0,1],"spd":[1000,1500,1200,1800],"mode":1}
int validate_motor_message(const char *payload, MotorData *motor_data) {
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

    // Verificar si el campo "task" existe y es "motor"
    cJSON *task = cJSON_GetObjectItemCaseSensitive(root, "task");
    if (!cJSON_IsString(task) || strcmp(task->valuestring, "motor") != 0) {
        printf("Error: Campo 'task' no encontrado o no es 'motor'\n");
        cJSON_Delete(root);
        return -1;
    }

    // Obtener el arreglo "motor" para identificar los motores
    cJSON *motor_array = cJSON_GetObjectItemCaseSensitive(root, "motor");
    if (!cJSON_IsArray(motor_array)) {
        printf("Error: El campo 'motor' no existe o no es un arreglo\n");
        cJSON_Delete(root);
        return -1;
    }

    int num_motor = cJSON_GetArraySize(motor_array);
    if (num_motor < 1 || num_motor > 4) {
        printf("Error: Numero de motores invalido (%d). Debe estar entre 1 y 4\n", num_motor);
        cJSON_Delete(root);
        return -1;
    }
    motor_data->num = num_motor;

    // Verificar los otros campos
    cJSON *dir_array = cJSON_GetObjectItemCaseSensitive(root, "dir");
    cJSON *ena_array = cJSON_GetObjectItemCaseSensitive(root, "ena");
    cJSON *spd_array = cJSON_GetObjectItemCaseSensitive(root, "spd");

    if (!cJSON_IsArray(dir_array) || cJSON_GetArraySize(dir_array) != num_motor ||
        !cJSON_IsArray(ena_array) || cJSON_GetArraySize(ena_array) != num_motor ||
        !cJSON_IsArray(spd_array) || cJSON_GetArraySize(spd_array) != num_motor) {
        printf("Error: Los campos 'dir', 'ena' o 'spd' no tienen el numero correcto de elementos\n");
        cJSON_Delete(root);
        return -1;
    }

    // Leer valores del arreglo "motor"
    for (int i = 0; i < num_motor; i++) {
        cJSON *motor_item = cJSON_GetArrayItem(motor_array, i);
        if (!cJSON_IsNumber(motor_item) || motor_item->valueint < 0 || motor_item->valueint > 3) {
            printf("Error: El valor en la posicion %d del arreglo 'motor' no es valido\n", i);
            cJSON_Delete(root);
            return -1;
        }
        motor_data->motor[i] = motor_item->valueint;
    }

    // Verificar valores de "dir"
    for (int i = 0; i < num_motor; i++) {
        cJSON *dir_item = cJSON_GetArrayItem(dir_array, i);
        if (!cJSON_IsNumber(dir_item) || (dir_item->valueint != 0 && dir_item->valueint != 1)) {
            printf("Error: El valor en la posicion %d del arreglo 'dir' no es 0 ni 1\n", i);
            cJSON_Delete(root);
            return -1;
        }
        motor_data->dir[i] = dir_item->valueint;
    }

    // Verificar valores de "ena"
    for (int i = 0; i < num_motor; i++) {
        cJSON *ena_item = cJSON_GetArrayItem(ena_array, i);
        if (!cJSON_IsNumber(ena_item) || (ena_item->valueint != 0 && ena_item->valueint != 1)) {
            printf("Error: El valor en la posicion %d del arreglo 'ena' no es 0 ni 1\n", i);
            cJSON_Delete(root);
            return -1;
        }
        motor_data->ena[i] = ena_item->valueint;
    }

    // Verificar valores de "spd"
    for (int i = 0; i < num_motor; i++) {
        cJSON *spd_item = cJSON_GetArrayItem(spd_array, i);
        if (!cJSON_IsNumber(spd_item) || spd_item->valueint < 0 || spd_item->valueint > 2000) {
            printf("Error: El valor en la posicion %d del arreglo 'spd' esta fuera del rango permitido (0-2000)\n", i);
            cJSON_Delete(root);
            return -1;
        }
        motor_data->spd[i] = spd_item->valueint;
    }

    // Verificar si el campo "mode" existe y es valido (0, 1 o 2)
    cJSON *mode_item = cJSON_GetObjectItemCaseSensitive(root, "mode");
    if (!cJSON_IsNumber(mode_item) || mode_item->valueint < 0 || mode_item->valueint > 2) {
        printf("Error: El valor de 'mode' (%d) esta fuera del rango permitido (0-2)\n", mode_item->valueint);
        cJSON_Delete(root);
        return -1;
    }
    motor_data->mode = mode_item->valueint;

    printf("Mensaje valido. num_motor = %d\n", motor_data->num);
    cJSON_Delete(root);
    return 0; // exito
}
