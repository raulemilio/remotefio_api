#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <mosquitto.h>
#include <cjson/cJSON.h>

#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>
#include <time.h>

#include "mqtt_callback.h"
#include "mem_map.h"
#include "mqtt_response.h"
#include "lcd_system_status.h"

#define OFFSET_STATE       4
#define MODE_COUNT         3  // modos 0, 1 y 2
#define EXECUTION_ENABLED  1
#define EXECUTION_DISABLED 0

// No pueden existir dos instancias del mismo modo simultaneamente
static pthread_mutex_t mode_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t shared_mutex = PTHREAD_MUTEX_INITIALIZER;
static int execution_flags[MODE_COUNT] = {EXECUTION_ENABLED, EXECUTION_ENABLED, EXECUTION_ENABLED};

static int fd_shared;
static void *map_base_shared;
static uint32_t *shared; // SHARED es de 32 bits por dato

static off_t target_shared = 0x4A310000; // Direccion base de SHARED
static size_t shared_size = 0x3000;      // 12KB de memoria compartida


// MODE 0 get state
void handle_gpio_output_mode_0(ThreadArgs *args) {
    printf("gpio_output -> mode 0\n");
    GpioOutputData gpio_output_data_send;

    pthread_mutex_lock(&shared_mutex);
    shared[PRU_SHD_FLAGS_INDEX] |= (1 << PRU_GPIO_OUTPUT_MODE0_FLAG);
    while (!(shared[PRU_SHD_GPIO_OUTPUT_MODE0_INDEX] & (1 << PRU_GPIO_OUTPUT_MODE0_DATARDY_FLAG))) {
        usleep(1000);
    }
    for (int i = 0; i < args->gpio_output_data->num_output; i++) {
        int pin = args->gpio_output_data->output[i];
        gpio_output_data_send.state[i] = (shared[PRU_SHD_GPIO_OUTPUT_MODE0_INDEX] >> (pin + OFFSET_STATE)) & 1;
	gpio_output_data_send.output[i] = pin;
    }
    gpio_output_data_send.num_output = args->gpio_output_data->num_output;
    publish_gpio_output_response(args->mosq, gpio_output_data_send, FUNCTION_STATUS_OK);
    lcd_system_status(LCD_GPIO_OUTPUT_STATUS_OK);
    pthread_mutex_unlock(&shared_mutex);

    // Liberar el flag del modo
    pthread_mutex_lock(&mode_mutex);
    execution_flags[0] = EXECUTION_ENABLED;
    pthread_mutex_unlock(&mode_mutex);
    pthread_exit(NULL);
}

// MODE 1 set state
void handle_gpio_output_mode_1(ThreadArgs *args) {
    printf("gpio_output -> mode 1\n");

    pthread_mutex_lock(&shared_mutex);
    // cargamos los datos en la memoria shd
    for (int i = 0; i < args->gpio_output_data->num_output; i++) {
        int pin = args->gpio_output_data->output[i];
        int state = args->gpio_output_data->state[i] & 1;

        if (pin >= 0 && pin < 4) shared[PRU_SHD_GPIO_OUTPUT_MODE1_INDEX] |= (1 << pin);
        if (i < 4) shared[PRU_SHD_GPIO_OUTPUT_MODE1_INDEX] |= (state << (OFFSET_STATE + pin));
    }
    shared[PRU_SHD_FLAGS_INDEX] |= (1 << PRU_GPIO_OUTPUT_MODE1_FLAG);
    while (!(shared[PRU_SHD_GPIO_OUTPUT_MODE1_INDEX] & (1 << PRU_GPIO_OUTPUT_MODE1_DATARDY_FLAG))) {
      usleep(1000);
    }
    // send data
    publish_gpio_output_response_status(args->mosq, FUNCTION_STATUS_OK);
    lcd_system_status(LCD_GPIO_OUTPUT_STATUS_OK);
    pthread_mutex_unlock(&shared_mutex);

    // Liberar el flag del modo
    pthread_mutex_lock(&mode_mutex);
    execution_flags[1] = EXECUTION_ENABLED;
    pthread_mutex_unlock(&mode_mutex);
    pthread_exit(NULL);
}

// MODE 2 set state
void handle_gpio_output_mode_2(ThreadArgs *args) {
    printf("gpio_output -> mode 2\n");
    // Implementaciones futuras
}

// **FUNCION PRINCIPAL**
void *gpio_output_function(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;

    int mode = args->gpio_output_data->mode;

    // Bloquear el mutex antes de modificar execution_flags
    pthread_mutex_lock(&mode_mutex);

    if (mode < 0 || mode >= MODE_COUNT) {
        printf(FUNCTION_INVALID_MODE);
        pthread_mutex_unlock(&mode_mutex);  // Desbloquear el mutex antes de salir
        pthread_exit(NULL);
    }
    // si ya hay en ejecucion una instancia de este modo se sale
    if (execution_flags[mode] == EXECUTION_DISABLED) {
        printf(MODE_ALREADY_RUNNING, mode);
        lcd_system_status(LCD_MQTT_MSG_RECEIVED_GPIO_OUTPUT_BUSY);
        pthread_mutex_unlock(&mode_mutex);
        pthread_exit(NULL);
    }

    // Marcar el modo como en ejecucion
    execution_flags[mode] = EXECUTION_DISABLED;
    pthread_mutex_unlock(&mode_mutex);

    // Mapear SHARED (32 bits por dato)
    if (map_memory(&fd_shared, &map_base_shared, (void **)&shared, target_shared, shared_size) == -1) {
        // Si falla el mapeo, liberar el flag y el mutex
        pthread_mutex_lock(&mode_mutex);
        execution_flags[mode] = EXECUTION_ENABLED;
        pthread_mutex_unlock(&mode_mutex);
        pthread_exit(NULL);  // Abortamos el hilo
        return -1;
    }

    switch (args->gpio_output_data->mode) {
        case 0: handle_gpio_output_mode_0(args); break;
        case 1: handle_gpio_output_mode_1(args); break;
        case 2: handle_gpio_output_mode_2(args); break;
        default: printf(GPIO_OUTPUT_INVALID_OPTION); break;
    }

    // Liberar memoria mapeada
    unmap_memory(fd_shared, map_base_shared, shared_size);

}

