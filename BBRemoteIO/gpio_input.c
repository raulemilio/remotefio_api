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

//MODE 0 get state
void handle_gpio_input_mode_0(ThreadArgs *args) {
   printf("gpio_input -> mode 0\n");
   GpioInputData gpio_input_data_send;

   pthread_mutex_lock(&shared_mutex);
   shared[PRU_SHD_FLAGS_INDEX] |= (1 << PRU_GPIO_INPUT_MODE0_FLAG);
   while (!(shared[PRU_SHD_GPIO_INPUT_MODE0_INDEX] & (1 << PRU_GPIO_INPUT_MODE0_DATARDY_FLAG))) {
        usleep(1000);
        printf("estamos aca?\n");
   }
   // send response
   process_gpio_input_data(args, &gpio_input_data_send,PRU_SHD_GPIO_INPUT_MODE0_INDEX);
   publish_gpio_input_response(args->mosq, gpio_input_data_send, FUNCTION_STATUS_OK);
   lcd_system_status(LCD_GPIO_INPUT_STATUS_MODE0_OK);
   pthread_mutex_unlock(&shared_mutex);
   // Liberar el flag del modo
   pthread_mutex_lock(&mode_mutex);
   execution_flags[0] = EXECUTION_ENABLED;
   pthread_mutex_unlock(&mode_mutex);
   pthread_exit(NULL);
}

//MODE 1 get state flag response
void handle_gpio_input_mode_1(ThreadArgs *args) {
   printf("gpio_input -> mode 1\n");
   lcd_system_status(LCD_GPIO_INPUT_MODE1_RUNNING);
   GpioInputData gpio_input_data_send;

   pthread_mutex_lock(&shared_mutex);
   shared[PRU_SHD_FLAGS_INDEX] |= (1 << PRU_GPIO_INPUT_MODE1_FLAG);
   while (shared[PRU_SHD_FLAGS_INDEX] & (1 << PRU_GPIO_INPUT_MODE1_FLAG)) {
     while (!(shared[PRU_SHD_GPIO_INPUT_MODE1_INDEX] & (1 << PRU_GPIO_INPUT_MODE1_DATARDY_FLAG))) {
     usleep(1000);
     }
     // send response
     process_gpio_input_data(args, &gpio_input_data_send,PRU_SHD_GPIO_INPUT_MODE1_INDEX);
     publish_gpio_input_response(args->mosq, gpio_input_data_send, FUNCTION_STATUS_OK);
     shared[PRU_SHD_GPIO_INPUT_MODE1_INDEX] = PRU_ERASE_MEM;
     pthread_mutex_unlock(&shared_mutex);
   }

   // Liberar el flag del modo
   pthread_mutex_lock(&mode_mutex);
   execution_flags[1] = EXECUTION_ENABLED;
   pthread_mutex_unlock(&mode_mutex);
   pthread_exit(NULL);
}
//MODE 2 get state flag response
void handle_gpio_input_mode_2(ThreadArgs *args) {
   printf("gpio_input -> mode 2\n");
   lcd_system_status(LCD_GPIO_INPUT_MODE1_RUNNING);
   GpioInputData gpio_input_data_send;

   pthread_mutex_lock(&shared_mutex);
   shared[PRU_SHD_FLAGS_INDEX] |= (1 << PRU_GPIO_INPUT_MODE2_FLAG);
   while (shared[PRU_SHD_FLAGS_INDEX] & (1 << PRU_GPIO_INPUT_MODE2_FLAG)) {
     while (!(shared[PRU_SHD_GPIO_INPUT_MODE2_INDEX] & (1 << PRU_GPIO_INPUT_MODE2_DATARDY_FLAG))) {
     usleep(1000);
     }
     // send response
     process_gpio_input_data(args, &gpio_input_data_send,PRU_SHD_GPIO_INPUT_MODE2_INDEX);
     publish_gpio_input_response(args->mosq, gpio_input_data_send, FUNCTION_STATUS_OK);
     shared[PRU_SHD_GPIO_INPUT_MODE2_INDEX] = PRU_ERASE_MEM;
     pthread_mutex_unlock(&shared_mutex);
   }

   // Liberar el flag del modo
   pthread_mutex_lock(&mode_mutex);
   execution_flags[2] = EXECUTION_ENABLED;
   pthread_mutex_unlock(&mode_mutex);
   pthread_exit(NULL);
}

/**
 * Procesa la entrada de GPIO y almacena los valores en gpio_input_data_send.
 */
void process_gpio_input_data(ThreadArgs *args, GpioInputData *gpio_input_data_send, int pru_shd_index) {
    for (int i = 0; i < args->gpio_input_data->num_input; i++) {
        int pin = args->gpio_input_data->input[i]; // Obtener el numero de canal solicitado
        gpio_input_data_send->state[i] = (shared[pru_shd_index] >> pin) & 1; // Extraer el bit correspondiente
        gpio_input_data_send->input[i] = pin;
    }
    // Cargar la cantidad de canales solicitados en la variable interna
    gpio_input_data_send->num_input = args->gpio_input_data->num_input;
}

// **FUNCION PRINCIPAL**
void *gpio_input_function(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;

    int mode = args->gpio_input_data->mode;

    // Bloquear el mutex antes de modificar execution_flags
    pthread_mutex_lock(&mode_mutex);
    if (mode < 0 || mode >= MODE_COUNT) {
        printf(FUNCTION_INVALID_MODE);
        pthread_mutex_unlock(&mode_mutex);  // Desbloquear el mutex antes de salir
        pthread_exit(NULL);
    }
    if (execution_flags[mode] == EXECUTION_DISABLED) {
        printf(MODE_ALREADY_RUNNING, mode);
        lcd_system_status(LCD_MQTT_MSG_RECEIVED_GPIO_INPUT_BUSY);
        pthread_mutex_unlock(&mode_mutex);
        pthread_exit(NULL);
    }

    // Marcar el modo como en ejecucion
    execution_flags[mode] = EXECUTION_DISABLED;
    pthread_mutex_unlock(&mode_mutex);  // Desbloquear el mutex despues de modificar el flag

    // Mapear SHARED (32 bits por dato)
    if (map_memory(&fd_shared, &map_base_shared, (void **)&shared, target_shared, shared_size) == -1) {
        // Si falla el mapeo, liberar el flag y el mutex
        pthread_mutex_lock(&mode_mutex);
        execution_flags[mode] = EXECUTION_ENABLED;
        pthread_mutex_unlock(&mode_mutex);
        pthread_exit(NULL);  // Abortamos el hilo
        return -1;
    }
    switch (args->gpio_input_data->mode) {
        case 0: handle_gpio_input_mode_0(args); break;
        case 1: handle_gpio_input_mode_1(args); break;
        case 2: handle_gpio_input_mode_2(args); break;
        default: printf(GPIO_INPUT_INVALID_OPTION); break;
    }

    // Liberar memoria mapeada
    unmap_memory(fd_shared, map_base_shared, shared_size);
}



