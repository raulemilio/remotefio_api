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
#include <sys/queue.h>

#include "mqtt_callback.h"
#include "mem_map.h"
#include "mqtt_response.h"
#include "lcd_system_status.h"

#define OFFSET_STATE       		 4
#define MODE_COUNT         		 2  // modos 0 o  1

#define EXECUTION_ENABLED   		 1
#define EXECUTION_DISABLED 		 0

#define PRU_SHD_FLAGS_INDEX             20
#define PRU_SHD_ADC_DATARDY_INDEX       21
#define PRU_SHD_SAMPLE_PERIOD_INDEX     22
#define PRU_SHD_BUFFER_SIZE_INDEX       23

#define PRU_ADC_MODE0_FLAG    		 0
#define PRU_ADC_MODE1_FLAG    		 1

#define PRU_ADC_BUFFER0_DATARDY_FLAG     0
#define PRU_ADC_BUFFER1_DATARDY_FLAG     1
#define PRU_CIRCULAR_BUFFER_SIZE         2

#define MAX_QUEUE_SIZE 1000000  // Limite de almacenamiento de la cola
#define TOTAL_PRODUCED 32  // Cantidad de datos que generaro el productor

// Estructura de los elementos de la cola
typedef struct Node {
    uint16_t channels[4][512];  // Simulacion de datos
    TAILQ_ENTRY(Node) entries;
} Node;

// Definimos la cola FIFO
TAILQ_HEAD(FifoQueue, Node);
struct FifoQueue fifo_head;

// Variables de control
pthread_mutex_t fifo_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t fifo_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t fifo_not_full = PTHREAD_COND_INITIALIZER;  // Para controlar cuando la cola no esta llena

// No pueden existir dos instancias del mismo modo simultaneamente
static pthread_mutex_t mode_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t shared_mutex = PTHREAD_MUTEX_INITIALIZER;
static int execution_flags[MODE_COUNT] = {EXECUTION_ENABLED, EXECUTION_ENABLED};

int queue_size;  // Numero de elementos en la cola
int produced_count;  // Contador de datos generados
int consumed_count;   // Contador de datos consumidos

static int fd_ram0, fd_shared;
static void *map_base_ram0, *map_base_shared;
static uint16_t *ram0;  // RAM0 es de 16 bits por dato
static uint32_t *shared; // SHARED es de 32 bits por dato

static off_t target_ram0 = 0x4A300000;   // Direccion base de RAM0
static off_t target_shared = 0x4A310000; // Direccion base de SHARED
static volatile size_t ram0_size = 0x2000;        // 8KB de RAM0
static volatile size_t shared_size = 0x3000;      // 12KB de memoria compartida

void *producer(void *arg) {
printf("producer running \n");
ThreadArgs *args = (ThreadArgs *)arg;
    // Cargamos frecuencia de muestreo y cantidad de muestras

    shared[PRU_SHD_SAMPLE_PERIOD_INDEX] = args->adc_data->sample_period;
    shared[PRU_SHD_BUFFER_SIZE_INDEX] = args->adc_data->buffer_size;
    shared[PRU_SHD_ADC_DATARDY_INDEX] = 0;

    // Avisamos a PRU que inicie la conversion
    shared[PRU_SHD_FLAGS_INDEX] |= (1 << PRU_ADC_MODE0_FLAG);

    int idx = 0;
    int num_iteration = (args->adc_data->num_samples/args->adc_data->buffer_size)/PRU_CIRCULAR_BUFFER_SIZE;
    printf("num_iteration %d \n", num_iteration);
    printf("sample_period %d \n",args->adc_data->sample_period);
    int iteration = 0;

    // Continuamos la conversion mientras el flag no se baje desde Linux
    while ((shared[PRU_SHD_FLAGS_INDEX] & (1 << PRU_ADC_MODE0_FLAG)) && (iteration < num_iteration)) {
//        printf("num_iteration %d \n", iteration);
/*
      while (produced_count < TOTAL_PRODUCED) {
          pthread_mutex_lock(&fifo_mutex);

      }
         // Esperar si la cola esta llena
          while (queue_size >= MAX_QUEUE_SIZE) {
              pthread_cond_wait(&fifo_not_full, &fifo_mutex);
          }
*/
         // buffer 0
          if (shared[PRU_SHD_ADC_DATARDY_INDEX] & (1 << PRU_ADC_BUFFER0_DATARDY_FLAG)) {
          printf("num_iteration %d \n", iteration);

	  // Cargamos los datos del buffer 0 desde RAM1
          printf("buffer0 \n");
          // Agregar nuevo dato a la cola

          Node *new_node = malloc(sizeof(Node));
          if (!new_node) {
              perror("malloc failed");
              exit(EXIT_FAILURE);
          }
          idx = 0;
             // Organizar los datos segun el formato especificado
             for (int i = 0; i < args->adc_data->buffer_size; ++i) {
                  new_node->channels[0][i] = ram0[idx];     // ch0
                  new_node->channels[1][i] = ram0[idx + 1]; // ch1
                  new_node->channels[2][i] = ram0[idx + 2]; // ch2
                  new_node->channels[3][i] = ram0[idx + 3]; // ch3
    	          // Mostrar los datos extraidos en cada iteracion

    	          printf("i=%d | ram0[%d]=%d, ram0[%d]=%d, ram0[%d]=%d, ram0[%d]=%d\n",
                	idx,
                	idx, ram0[idx],         // ch0
        		idx+ 1, ram0[idx+ 1],   // ch1
        		idx + 2, ram0[idx + 2], // ch2
        		idx + 3, ram0[idx + 3]);// ch3

                  idx += 4;

             }

          TAILQ_INSERT_TAIL(&fifo_head, new_node, entries);
          queue_size++;

          // Notificar al consumidor que hay datos disponibles
          pthread_cond_signal(&fifo_cond);
          pthread_mutex_unlock(&fifo_mutex);

          // Borramos el flag de datos listos
          shared[PRU_SHD_ADC_DATARDY_INDEX] &= ~(1 << PRU_ADC_BUFFER0_DATARDY_FLAG);
          }

          // buffer1
          if (shared[PRU_SHD_ADC_DATARDY_INDEX] & (1 << PRU_ADC_BUFFER1_DATARDY_FLAG)) {
          // Cargamos los datos del buffer 0 desde RAM1
          iteration++;
          printf("buffer1 \n");

          // Agregar nuevo dato a la cola
          Node *new_node = malloc(sizeof(Node));
          if (!new_node) {
              perror("malloc failed");
              exit(EXIT_FAILURE);
          }
          idx = 2048;
          // Organizar los datos segun el formato especificado
             for (int i = 0; i < args->adc_data->buffer_size; ++i) {
                 new_node->channels[0][i] = ram0[idx];   // ch0
                 new_node->channels[1][i] = ram0[idx + 1]; // ch1
                 new_node->channels[2][i] = ram0[idx + 2]; // ch2
                 new_node->channels[3][i] = ram0[idx + 3]; // ch3
                 // Mostrar los datos extraidos en cada iteracion

                 printf("i=%d | ram0[%d]=%d, ram0[%d]=%d, ram0[%d]=%d, ram0[%d]=%d\n",
        		idx,
        		idx, ram0[idx],         // ch0
        		idx+ 1, ram0[idx+ 1],   // ch1
        		idx + 2, ram0[idx + 2], // ch2
        		idx + 3, ram0[idx + 3]);// ch3

    	      idx += 4;
              }

          TAILQ_INSERT_TAIL(&fifo_head, new_node, entries);
          queue_size++;

          // Notificar al consumidor que hay datos disponibles
          pthread_cond_signal(&fifo_cond);
          pthread_mutex_unlock(&fifo_mutex);

          // Borramos el flag de datos listos
          shared[PRU_SHD_ADC_DATARDY_INDEX] &= ~(1 << PRU_ADC_BUFFER1_DATARDY_FLAG);
          }

          usleep(1000); //
    }
    // Borramos el flag para apagar el adquisidor
    shared[PRU_SHD_FLAGS_INDEX] &= ~(1 << PRU_ADC_MODE0_FLAG);

    return NULL;
}

void *consumer(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    printf("consumer running \n");
/*
    while (1) {
        pthread_mutex_lock(&fifo_mutex);
        printf("consumed_count %d \n", consumed_count);

        // Esperar si la cola esta vacia
        while (TAILQ_EMPTY(&fifo_head) && produced_count < TOTAL_PRODUCED) {
            pthread_cond_wait(&fifo_cond, &fifo_mutex);
        }

        // Si ya se produjeron todos los datos y la cola esta vacia, terminar el hilo
        if (TAILQ_EMPTY(&fifo_head) && produced_count >= TOTAL_PRODUCED) {
            pthread_mutex_unlock(&fifo_mutex);
            break;
        }

        // Si se han procesado todos los datos, terminar el hilo
        if (consumed_count >= TOTAL_PRODUCED) {
            pthread_mutex_unlock(&fifo_mutex);
            break;
        }

        // Procesar un elemento de la cola
        Node *node = TAILQ_FIRST(&fifo_head);
        if (node) {
            // Crear un objeto JSON para el mensaje
            cJSON *json_msg = cJSON_CreateObject();
            cJSON *json_channels = cJSON_CreateObject();

            // Llenar los canales con los datos
            for (int i = 0; i < 4; i++) {
                cJSON *json_channel = cJSON_CreateArray();

                // Llenar el array de datos para cada canal
                for (int j = 0; j < 16; j++) {
                    cJSON_AddItemToArray(json_channel, cJSON_CreateNumber(node->channels[i][j]));
                    //printf("muestra agregada %d \n",node->channels[i][j]);
                }

                // Anadir el canal al objeto JSON de canales
                char channel_name[10];
                snprintf(channel_name, sizeof(channel_name), "ch%d", i);
                cJSON_AddItemToObject(json_channels, channel_name, json_channel);
            }

            // Anadir la tarea y los canales al mensaje
            cJSON_AddStringToObject(json_msg, "task", "adc");
            cJSON_AddItemToObject(json_msg, "channels", json_channels);

            // Convertir el mensaje JSON a cadena
            char *json_str = cJSON_PrintUnformatted(json_msg);

            // Publicar los datos en MQTT
            //printf("Consumer: Enviando datos: %s (Queue size: %d)\n", json_str, queue_size - 1);
            mosquitto_publish(args->mosq, NULL, TOPIC_RSP_ADC, strlen(json_str), json_str, 0, false);

            // Limpiar recursos de JSON
            cJSON_Delete(json_msg);
            free(json_str);

            // Eliminar el nodo de la cola
            TAILQ_REMOVE(&fifo_head, node, entries);
            free(node);
            queue_size--;

            // Notificar al productor que hay espacio disponible
            pthread_cond_signal(&fifo_not_full);
        }

        usleep(100);  // Pausa para no sobrecargar el sistema
        pthread_mutex_unlock(&fifo_mutex);
    }
*/
    return NULL;
}

// **FUNCION PRINCIPAL**
void *adc_function(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    printf("adc function\n");
    int mode = args->adc_data->mode;
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

    // Mapear RAM0 (16 bits por dato)
    if (map_memory(&fd_ram0, &map_base_ram0, (void **)&ram0, target_ram0, ram0_size) == -1) {
        // Si falla el mapeo, liberar el flag y el mutex
        pthread_mutex_lock(&mode_mutex);
        execution_flags[mode] = EXECUTION_ENABLED;
        pthread_mutex_unlock(&mode_mutex);
        pthread_exit(NULL);  // Abortamos el hilo
        return -1;
    }

    // Mapear SHARED (32 bits por dato)
    if (map_memory(&fd_shared, &map_base_shared, (void **)&shared, target_shared, shared_size) == -1) {
        unmap_memory(fd_ram0, map_base_ram0, ram0_size);
       // Si falla el mapeo, liberar el flag y el mutex
        pthread_mutex_lock(&mode_mutex);
        execution_flags[mode] = EXECUTION_ENABLED;
        pthread_mutex_unlock(&mode_mutex);
        pthread_exit(NULL);  // Abortamos el hilo
        return -1;
    }

    TAILQ_INIT(&fifo_head);
    queue_size = 0;
    produced_count = 0;  // Contador de datos generados
    consumed_count = 0;   // Contador de datos consumidos

    pthread_t producer_thread, consumer_thread;


    // Crear hilos para productor y consumidor
    if (pthread_create(&producer_thread, NULL, producer, args) != 0) {
        fprintf(stderr, "Error al crear hilo productor\n");
        free(args);  // Liberar memoria si falla la creacion
        return NULL;
    }

    if (pthread_create(&consumer_thread, NULL, consumer, args) != 0) {
        fprintf(stderr, "Error al crear hilo consumidor\n");
        free(args);  // Liberar memoria si falla la creacion
        return NULL;
    }

    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    printf("ADC funcion completada\n");

    // Liberar memoria mapeada
    unmap_memory(fd_ram0, map_base_ram0, ram0_size);
    unmap_memory(fd_shared, map_base_shared, shared_size);

    // Liberar el flag del modo
    pthread_mutex_lock(&mode_mutex);
    execution_flags[0] = EXECUTION_ENABLED;
    pthread_mutex_unlock(&mode_mutex);
    pthread_exit(NULL);

    free(args);  // Liberamos la memoria si `ThreadArgs` se aloca dinamicamente
    pthread_exit(NULL);
}
