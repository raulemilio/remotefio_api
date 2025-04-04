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

#define OFFSET_ENABLE       4
#define OFFSET_DIRECTION    5
#define OFFSET_SHD_STEPTIME 10 //Shared[10]

#define MODE_COUNT          4  // modos 0, 1 y 2
#define EXECUTION_ENABLED   1
#define EXECUTION_DISABLED  0

// No pueden existir dos instancias del mismo modo simultaneamente
static pthread_mutex_t mode_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t shared_mutex = PTHREAD_MUTEX_INITIALIZER;

static int execution_flags[MODE_COUNT] = {EXECUTION_ENABLED, EXECUTION_ENABLED, EXECUTION_ENABLED,EXECUTION_ENABLED};

static int fd_shared;
static void *map_base_shared;
static uint32_t *shared; // SHARED es de 32 bits por dato

static off_t target_shared = 0x4A310000; // Direccion base de SHARED
static size_t shared_size = 0x3000;      // 12KB de memoria compartida

//MODE 0
void handle_motor_mode_0(ThreadArgs *args) {
   printf("motor -> mode 0\n");
   MotorData motor_data_send;

   pthread_mutex_lock(&shared_mutex);
   shared[PRU_SHD_FLAGS_INDEX] |= (1 << PRU_MOTOR_MODE0_FLAG);
   while (!(shared[PRU_SHD_MOTOR_MODE0_INDEX] & (1 << PRU_MOTOR_MODE0_DATARDY_FLAG))) {
        usleep(1000);
   }
   // send response
   process_motor_data(args, &motor_data_send,PRU_SHD_MOTOR_MODE0_INDEX);
   publish_motor_response(args->mosq, motor_data_send, FUNCTION_STATUS_OK);
   lcd_system_status(LCD_MOTOR_STATUS_MODE1_OK);
   pthread_mutex_unlock(&shared_mutex);

   // Liberar el flag del modo
   pthread_mutex_lock(&mode_mutex);
   execution_flags[0] = EXECUTION_ENABLED;
   pthread_mutex_unlock(&mode_mutex);
   pthread_exit(NULL);
}
//MODE 1
void handle_motor_mode_1(ThreadArgs *args) {
   printf("motor -> mode 1\n");
   MotorData motor_data_send;

   pthread_mutex_lock(&shared_mutex);
   shared[PRU_SHD_FLAGS_INDEX] |= (1 << PRU_MOTOR_MODE1_FLAG);
   while (shared[PRU_SHD_FLAGS_INDEX] & (1 << PRU_MOTOR_MODE1_FLAG)){
     while (!(shared[PRU_SHD_MOTOR_MODE1_INDEX] & (1 << PRU_MOTOR_MODE1_DATARDY_FLAG))) {
     usleep(1000);
     }
     shared[PRU_SHD_MOTOR_MODE1_INDEX] = PRU_ERASE_MEM;
     // send response
     process_motor_data(args, &motor_data_send,PRU_SHD_MOTOR_MODE1_INDEX);
     publish_motor_response(args->mosq, motor_data_send, FUNCTION_STATUS_OK);
     lcd_system_status(LCD_MOTOR_STATUS_MODE1_OK);
     pthread_mutex_unlock(&shared_mutex);
   }

   pthread_mutex_lock(&mode_mutex);
   execution_flags[1] = EXECUTION_ENABLED;
   pthread_mutex_unlock(&mode_mutex);
   pthread_exit(NULL);
}

//MODE 2
void handle_motor_mode_2(ThreadArgs *args) {
   printf("motor -> mode 2\n");
   MotorData motor_data_send;

   pthread_mutex_lock(&shared_mutex);
   shared[PRU_SHD_FLAGS_INDEX] |= (1 << PRU_MOTOR_MODE2_FLAG);
   while (shared[PRU_SHD_FLAGS_INDEX] & (1 << PRU_MOTOR_MODE2_FLAG)){
     while (!(shared[PRU_SHD_MOTOR_MODE2_INDEX] & (1 << PRU_MOTOR_MODE2_DATARDY_FLAG))) {
     usleep(1000);
     }
     shared[PRU_SHD_MOTOR_MODE2_INDEX] = PRU_ERASE_MEM;
     // send response
     process_motor_data(args, &motor_data_send,PRU_SHD_MOTOR_MODE2_INDEX);
     publish_motor_response(args->mosq, motor_data_send, FUNCTION_STATUS_OK);
     lcd_system_status(LCD_MOTOR_STATUS_MODE1_OK);
     pthread_mutex_unlock(&shared_mutex);
   }

   pthread_mutex_lock(&mode_mutex);
   execution_flags[2] = EXECUTION_ENABLED;
   pthread_mutex_unlock(&mode_mutex);
   pthread_exit(NULL);

}

//MODE 3 set state
void handle_motor_mode_3(ThreadArgs *args) {
   printf("motor -> mode 3\n");
    // Borrar shared[4] write set function PRU
   pthread_mutex_lock(&shared_mutex);
   shared[PRU_SHD_MOTOR_MODE3_INDEX] = PRU_ERASE_MEM;
   // Recorrer cada motor definido en motor_data
   for (int i = 0; i < args->motor_data->num_motor; i++) {
      int motor_id = args->motor_data->motor[i];
      if (motor_id < 0 || motor_id > 3) {
         continue; // Ignorar valores fuera de rango
      }
     // Cargar bit correspondiente al motor en shared[4]
     shared[PRU_SHD_MOTOR_MODE3_INDEX] |= (1 << motor_id);
     // Asegurarse de que solo se usen 0 o 1 para ena y dir
     shared[PRU_SHD_MOTOR_MODE3_INDEX] |= ((args->motor_data->ena[i] & 1) << (OFFSET_ENABLE    + (2 * motor_id)));  // Asegura que 'ena' sea 0 o 1
     shared[PRU_SHD_MOTOR_MODE3_INDEX] |= ((args->motor_data->dir[i] & 1) << (OFFSET_DIRECTION + (2 * motor_id)));  // Asegura que 'dir' sea 0 o 1
     // Cargar velocidades en los registros correspondientes de forma dinamica
     shared[OFFSET_SHD_STEPTIME + i] = args->motor_data->step_time[i];
    }
    // Poner en 1 el bit 4 de shared[0]
    shared[PRU_SHD_FLAGS_INDEX] |= (1 << PRU_MOTOR_MODE3_FLAG);
    while (!(shared[PRU_SHD_MOTOR_MODE3_INDEX] & (1 << PRU_MOTOR_MODE3_DATARDY_FLAG))) {
        usleep(1000);
    }
    // send data
    publish_motor_response_status(args->mosq, FUNCTION_STATUS_OK);
    lcd_system_status(LCD_MOTOR_STATUS_OK);
    // Liberamos el mutex al final
    pthread_mutex_unlock(&shared_mutex);

    pthread_mutex_lock(&mode_mutex);
    execution_flags[3] = EXECUTION_ENABLED;
    pthread_mutex_unlock(&mode_mutex);
    pthread_exit(NULL);
}

// Funciones auxiliares
/**
 * Procesa la entrada de GPIO y almacena los valores en gpio_input_data_send.
 */
void process_motor_data(ThreadArgs *args, MotorData *motor_data_send, int pru_shd_index) {
    for (int i = 0; i < args->motor_data->num_motor; i++) {
        int motor = args->motor_data->motor[i]; // Obtener el numero de pin solicitado
        motor_data_send->ena[i] = (shared[pru_shd_index] >> ((2*motor) + OFFSET_ENABLE)) & 1; // Extraer el bit correspondiente
        motor_data_send->motor[i] = motor;
        motor_data_send->dir[i] = (shared[pru_shd_index] >> ((2*motor) + OFFSET_DIRECTION)) & 1;
        motor_data_send->step_time[i] = shared[(OFFSET_SHD_STEPTIME + motor)];
    }
    // cargamos en la estructura de salida la cantidad de motores a consultar
    motor_data_send->num_motor = args->motor_data->num_motor;
}

// **FUNCION PRINCIPAL**
void *motor_function(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    int mode = args->motor_data->mode;

    // Bloquear el mutex antes de modificar execution_flags
    pthread_mutex_lock(&mode_mutex);
    if (mode < 0 || mode >= MODE_COUNT) {
        printf(FUNCTION_INVALID_MODE);
        pthread_mutex_unlock(&mode_mutex);  // Desbloquear el mutex antes de salir
        pthread_exit(NULL);
    }

    if (execution_flags[mode] == EXECUTION_DISABLED) {
        printf(MODE_ALREADY_RUNNING, mode);
        lcd_system_status(LCD_MQTT_MSG_RECEIVED_MOTOR_BUSY);
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

    switch (args->motor_data->mode) {
        case 0: handle_motor_mode_0(args); break;
        case 1: handle_motor_mode_1(args); break;
        case 2: handle_motor_mode_2(args); break;
        case 3: handle_motor_mode_3(args); break;
        default: printf(GPIO_MOTOR_INVALID_OPTION); break;
    }

    // Liberar memoria mapeada
    unmap_memory(fd_shared, map_base_shared, shared_size);

}
