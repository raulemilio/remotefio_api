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

#define PRU_RAM0_INDEX_START             0
#define PRU_RAM0_INDEX_MID	         2048
#define PRU_ADC_BUFFER0_DATARDY_FLAG     0
#define PRU_ADC_BUFFER1_DATARDY_FLAG     1
#define PRU_CIRCULAR_BUFFER_SIZE         2

#define MAX_QUEUE_SIZE 			 1000000  // Limite de almacenamiento de la cola
#define MODE1_SAMPLES_MAX		 1000

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
    //printf("adc -> producer running \n");
    ThreadArgs *args = (ThreadArgs *)arg;
    int idx;
    produced_count = 0;
    int iteration = 0;
    int trigger_flag = 0;
    int mode = 0;
    int num_iteration = 0;

    int pru_buffer_size = args->adc_data->buffer_size / PRU_CIRCULAR_BUFFER_SIZE;

    if(args->adc_data->mode == 0){
       num_iteration = args->adc_data->num_samples / args->adc_data->buffer_size;
    }else{
       num_iteration = MODE1_SAMPLES_MAX;
    }

    trigger_flag = (args->adc_data->enable_external_trigger == 0) ?
                 PRU_ADC_MODE0_FLAG : PRU_ADC_MODE1_FLAG;

    // Configurar parametros compartidos con PRU
    shared[PRU_SHD_SAMPLE_PERIOD_INDEX] = args->adc_data->sample_period;
    shared[PRU_SHD_BUFFER_SIZE_INDEX] = pru_buffer_size;
    shared[PRU_SHD_ADC_DATARDY_INDEX] = PRU_ERASE_MEM;

    shared[PRU_SHD_FLAGS_INDEX] |= (1 << trigger_flag);  // Iniciar adquisicion

    while ((shared[PRU_SHD_FLAGS_INDEX] & (1 << trigger_flag)) && (iteration < num_iteration)) {

        bool buffer_processed = false;

        pthread_mutex_lock(&fifo_mutex);

        if (queue_size < MAX_QUEUE_SIZE) {
            uint32_t flags = shared[PRU_SHD_ADC_DATARDY_INDEX];

            if (flags & (1 << PRU_ADC_BUFFER0_DATARDY_FLAG)) {
                idx = PRU_RAM0_INDEX_START;
                buffer_processed = true;
                shared[PRU_SHD_ADC_DATARDY_INDEX] &= ~(1 << PRU_ADC_BUFFER0_DATARDY_FLAG);
                //printf("buffer0\n");
            } else if (flags & (1 << PRU_ADC_BUFFER1_DATARDY_FLAG)) {
                idx = PRU_RAM0_INDEX_MID;
                buffer_processed = true;
                shared[PRU_SHD_ADC_DATARDY_INDEX] &= ~(1 << PRU_ADC_BUFFER1_DATARDY_FLAG);
                //printf("buffer1 \n");
                iteration++;
            }
        }

        if (buffer_processed) {
            Node *new_node = malloc(sizeof(Node));
            if (!new_node) {
                perror("malloc failed");
                exit(EXIT_FAILURE);
            }

            // Copiar datos por canal
            for (int i = 0; i < pru_buffer_size; ++i) {
                new_node->channels[0][i] = ram0[idx];
                new_node->channels[1][i] = ram0[idx + 1];
                new_node->channels[2][i] = ram0[idx + 2];
                new_node->channels[3][i] = ram0[idx + 3];
                idx += 4;
            }

            TAILQ_INSERT_TAIL(&fifo_head, new_node, entries);
            queue_size++;
            produced_count++;
            pthread_cond_signal(&fifo_cond);  // Notifica al consumidor
        }

        pthread_mutex_unlock(&fifo_mutex);

        if (!buffer_processed) {
            sched_yield();  // Cede CPU si no hay buffer disponible
        }
    }

    shared[PRU_SHD_FLAGS_INDEX] &= ~(1 << trigger_flag);  // Detener adquisicion
    return NULL;
}

void *consumer(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    //printf("adc -> consumer running \n");

    int total_produced = (args->adc_data->num_samples / args->adc_data->buffer_size) * PRU_CIRCULAR_BUFFER_SIZE;

    // Abrir archivo binario
    FILE *adc_file = fopen("adc_data.bin", "wb");
    if (!adc_file) {
        perror("Error al abrir archivo de datos");
        return NULL;
    }

    while (1) {
        pthread_mutex_lock(&fifo_mutex);

        while (TAILQ_EMPTY(&fifo_head) && produced_count < total_produced) {
            pthread_cond_wait(&fifo_cond, &fifo_mutex);
        }

        if ((TAILQ_EMPTY(&fifo_head) && produced_count >= total_produced) || consumed_count >= total_produced) {
            pthread_mutex_unlock(&fifo_mutex);
            break;
        }

        // Sacar el nodo de la cola lo mas rapido posible
        Node *node = TAILQ_FIRST(&fifo_head);
        if (node) {
            TAILQ_REMOVE(&fifo_head, node, entries);
            queue_size--;
            consumed_count++;
            pthread_cond_signal(&fifo_not_full);
        }

        pthread_mutex_unlock(&fifo_mutex);

        // Procesar nodo FUERA del mutex
        if (node) {
            int samples_per_node = args->adc_data->buffer_size / PRU_CIRCULAR_BUFFER_SIZE;

            // Escribir en archivo
            for (int j = 0; j < samples_per_node; j++) {
                uint16_t sample[4];
                for (int i = 0; i < 4; i++) {
                    sample[i] = (uint16_t)node->channels[i][j];
                }
                fwrite(sample, sizeof(uint16_t), 4, adc_file);
                //printf("Nodo %d - Muestra %d: ch0=%u ch1=%u ch2=%u ch3=%u\n", consumed_count, j, sample[0], sample[1], sample[2], sample[3]);
            }

            // Crear JSON
            cJSON *json_msg = cJSON_CreateObject();
            cJSON *json_channels = cJSON_CreateObject();
            for (int i = 0; i < 4; i++) {
                cJSON *json_channel = cJSON_CreateArray();
                for (int j = 0; j < samples_per_node; j++) {
                    cJSON_AddItemToArray(json_channel, cJSON_CreateNumber(node->channels[i][j]));
                }
                char channel_name[10];
                snprintf(channel_name, sizeof(channel_name), "ch%d", i);
                cJSON_AddItemToObject(json_channels, channel_name, json_channel);
            }
            cJSON_AddStringToObject(json_msg, "task", "adc");
            cJSON_AddItemToObject(json_msg, "channels", json_channels);

            // Publicar
            char *json_str = cJSON_PrintUnformatted(json_msg);
            mosquitto_publish(args->mosq, NULL, TOPIC_RSP_ADC, strlen(json_str), json_str, 0, false);

            cJSON_Delete(json_msg);
            free(json_str);
            free(node);
        }
    }

    fclose(adc_file);
    return NULL;
}

// **FUNCION PRINCIPAL**
void *adc_function(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    printf("Adc function running\n");
    lcd_system_status(LCD_ADC_STATUS_RUNNING);

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

    // Liberar memoria mapeada
    unmap_memory(fd_ram0, map_base_ram0, ram0_size);
    unmap_memory(fd_shared, map_base_shared, shared_size);

    // Liberar el flag del modo
    pthread_mutex_lock(&mode_mutex);
    execution_flags[0] = EXECUTION_ENABLED;
    pthread_mutex_unlock(&mode_mutex);

    printf("Adc function finnish\n");
    lcd_system_status(LCD_ADC_STATUS_FINNISH);
    free(args);  // Liberamos la memoria si `ThreadArgs` se aloca dinamicamente
    pthread_exit(NULL);
}
