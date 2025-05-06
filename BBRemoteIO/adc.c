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
#include "mqtt_response.h"
#include "pru_control.h"

#include "adc.h"
#include <sys/queue.h>
#include "task_queue.h"
#include <unistd.h>
#include "mqtt_publish.h"
#include "lcd_display.h"
#include "log.h"

#define PRU_SHD_FLAGS_INDEX             40
#define PRU_SHD_ADC_DATARDY_INDEX       41
#define PRU_SHD_SAMPLE_PERIOD_INDEX     42
#define PRU_SHD_BUFFER_SIZE_INDEX       43

#define PRU_ADC_MODE0_FLAG    		 0
#define PRU_ADC_MODE1_FLAG    		 1

#define PRU_RAM0_INDEX_START             0
#define PRU_RAM0_INDEX_MID	         2048
#define PRU_ADC_BUFFER0_DATARDY_FLAG     0
#define PRU_ADC_BUFFER1_DATARDY_FLAG     1
#define PRU_CIRCULAR_BUFFER_SIZE         2

#define MAX_QUEUE_SIZE 			 10240  // Limite de almacenamiento de la cola
#define MODE1_SAMPLES_MAX		 1000

#define ADC_VREF			 1.8
#define ADC_RESOLUTION 			 4096
#define ADC_SLEEP_TIME_1MS               1    // 1 milisegundo
#define ADC_SLEEP_TIME_10MS              10   // 10 milisegundos
#define BUFFER_DURATION_THRESHOLD_SEC    1.0f  // 1 segundo

#define FILE_DATA_NAME			"adc_data.bin"

volatile bool adc_running;
static volatile bool producer_running;

pthread_mutex_t adc_running_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t producer_running_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t args_mutex = PTHREAD_MUTEX_INITIALIZER; // los datos se pasan por puntero a producer y consumer

TaskQueue adc_queue;
pthread_t adc_thread;

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

int queue_size;      // Numero de elementos en la cola
int produced_count;  // Contador de datos generados

void *producer(void *arg);
void *consumer(void *arg);
void fifo_reset(void);
static void notify_adc_status_message(struct mosquitto *mosq, const char *msg);

void *adc_thread_func(void *arg) {
    ThreadAdcDataArgs args;
    char message[128];

    while (1) {
        if (task_queue_dequeue(&adc_queue, &args, sizeof(ThreadAdcDataArgs)) == 0) {
            if (!args.adc_data) continue;

            snprintf(message, sizeof(message), "Adc mode %d run", args.adc_data->mode);
            notify_adc_status_message(args.mosq, message);

            pthread_mutex_lock(&producer_running_mutex);
            producer_running = TASK_RUNNING;
            pthread_mutex_unlock(&producer_running_mutex);

            fifo_reset();
    	    produced_count = 0;  // Contador de datos generados

            pthread_t producer_thread, consumer_thread;

            // Crear hilos para productor y consumidor
            if (pthread_create(&producer_thread, NULL, producer, &args) != 0) {
                LOG_ERROR("Error al crear hilo productor");
                return NULL;
            }

            if (pthread_create(&consumer_thread, NULL, consumer, &args) != 0) {
                LOG_ERROR("Error al crear hilo consumidor");
                return NULL;
            }

    	    pthread_join(producer_thread, NULL);
    	    pthread_join(consumer_thread, NULL);

            pthread_mutex_lock(&adc_running_mutex);
            adc_running = TASK_STOPPED;
            pthread_mutex_unlock(&adc_running_mutex);

            notify_adc_status_message(args.mosq, MSG_ADC_FINISH);
            free(args.adc_data);
            //Limpieza del struct para evitar basura en la proxima iteracion
            memset(&args, 0, sizeof(args));
            // memset(args, 0 , sizeof(ThreadAdcDataArgs)); //mas claridad
        }
    }
    return NULL;
}

void *producer(void *arg) {
    LOG_DEBUG("adc -> producer running");

    ThreadAdcDataArgs *args = (ThreadAdcDataArgs *)arg;
    // acceso protegido a args
    pthread_mutex_lock(&args_mutex);
    int pru_buffer_size = args->adc_data->buffer_size / PRU_CIRCULAR_BUFFER_SIZE;
    int mode = args->adc_data->mode;
    int num_samples = args->adc_data->num_samples;
    int buffer_size = args->adc_data->buffer_size;
    float samples_rate = (float)args->adc_data->sample_rate;
    int enable_external_trigger = args->adc_data->enable_external_trigger;
    int sample_period = args->adc_data->sample_period;
    pthread_mutex_unlock(&args_mutex);

    int idx;
    produced_count = 0;
    int iteration = 0;
    int trigger_flag = 0;
    int num_iteration = 0;

    bool buffer_processed = false;
    bool can_enqueue = false;
    bool is_adc_running = false;

    bool finished_due_to_timeout = false;
    bool finished_due_to_adc_stopped = false;
    bool finished_normally = false;

    const char *msg;

    int timeout_ms = ADC_TIMEOUT_MS;
    long elapsed_ms;
    int sleep_time_ms = 1; // 1 ms default

    if(mode == 0){
       num_iteration = num_samples / buffer_size;
    } else{
       num_iteration = MODE1_SAMPLES_MAX;
    }

    float buffer_duration_sec = (float)pru_buffer_size / samples_rate; // dinamic sleep thread time

    if (buffer_duration_sec >= BUFFER_DURATION_THRESHOLD_SEC) {
        sleep_time_ms = ADC_SLEEP_TIME_10MS;
    } else {
        sleep_time_ms = ADC_SLEEP_TIME_1MS;
    }

    // recordar que en pru las funciones son distintas si es por extarnal flag o no
    trigger_flag = (enable_external_trigger == 0) ?
                    PRU_ADC_MODE0_FLAG : PRU_ADC_MODE1_FLAG;

    // Configurar parametros compartidos con PRU
    pthread_mutex_lock(&shm->shared_mutex[MUTEX_ADC]);
    shm->shared[PRU_SHD_SAMPLE_PERIOD_INDEX] = sample_period; // cargamos periodo de muestreo
    shm->shared[PRU_SHD_BUFFER_SIZE_INDEX] = pru_buffer_size; // cargamos tamanio de buffer
    shm->shared[PRU_SHD_ADC_DATARDY_INDEX] = PRU_ERASE_MEM;   // flags data rdy
    shm->shared[PRU_SHD_FLAGS_INDEX] |= (1 << trigger_flag);  // Iniciar adquisicion
    pthread_mutex_unlock(&shm->shared_mutex[MUTEX_ADC]);

    // Guardamos el tiempo de inicio
    struct timespec start_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    while (1) {

        buffer_processed = false;
        can_enqueue = false;

        // Verificamos si se alcanzo el timeout global
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);

        elapsed_ms = (now.tv_sec - start_time.tv_sec) * 1000 +
                     (now.tv_nsec - start_time.tv_nsec) / 1000000;

        pthread_mutex_lock(&shm->shared_mutex[MUTEX_ADC]);
        int trigger_active = shm->shared[PRU_SHD_FLAGS_INDEX] & (1 << trigger_flag);
        pthread_mutex_unlock(&shm->shared_mutex[MUTEX_ADC]);

        pthread_mutex_lock(&adc_running_mutex);
        is_adc_running = adc_running;
        pthread_mutex_unlock(&adc_running_mutex);

        if (elapsed_ms >= timeout_ms) {
            finished_due_to_timeout = true;
            break;
        }
        if (!trigger_active || iteration >= num_iteration) {
            finished_normally = true;
            break;
        }
        if (!is_adc_running) {
            finished_due_to_adc_stopped = true;
            break;
        }
	// Verificamos si hay espacio en la cola
	pthread_mutex_lock(&fifo_mutex);
	if (queue_size < MAX_QUEUE_SIZE) {
    	    can_enqueue = true;
	}

	pthread_mutex_unlock(&fifo_mutex);

        if (can_enqueue) {

            pthread_mutex_lock(&shm->shared_mutex[MUTEX_ADC]);
            uint32_t flags = shm->shared[PRU_SHD_ADC_DATARDY_INDEX];

            if (flags & (1 << PRU_ADC_BUFFER0_DATARDY_FLAG)) {
                idx = PRU_RAM0_INDEX_START;
                buffer_processed = true;
                shm->shared[PRU_SHD_ADC_DATARDY_INDEX] &= ~(1 << PRU_ADC_BUFFER0_DATARDY_FLAG);
                //LOG_DEBUG("buffer0");

            } else if (flags & (1 << PRU_ADC_BUFFER1_DATARDY_FLAG)) {
                idx = PRU_RAM0_INDEX_MID;
                buffer_processed = true;
                shm->shared[PRU_SHD_ADC_DATARDY_INDEX] &= ~(1 << PRU_ADC_BUFFER1_DATARDY_FLAG);
                //LOG_DEBUG("buffer1");

                iteration++;
            }
                pthread_mutex_unlock(&shm->shared_mutex[MUTEX_ADC]);
        }

        if (buffer_processed) {
            Node *new_node = malloc(sizeof(Node));
            if (!new_node) {
                LOG_ERROR("malloc failed");
                exit(EXIT_FAILURE);
            }

            pthread_mutex_lock(&shm->ram0_mutex);
            // Copiar datos por canal
            for (int i = 0; i < pru_buffer_size; ++i) {
                new_node->channels[0][i] = shm->ram0[idx];
                new_node->channels[1][i] = shm->ram0[idx + 1];
                new_node->channels[2][i] = shm->ram0[idx + 2];
                new_node->channels[3][i] = shm->ram0[idx + 3];
                idx += 4;
            }
            pthread_mutex_unlock(&shm->ram0_mutex);

            pthread_mutex_lock(&fifo_mutex);
            TAILQ_INSERT_TAIL(&fifo_head, new_node, entries);
            queue_size++;
            produced_count++;
            pthread_cond_signal(&fifo_cond);  // Notifica al consumidor
            pthread_mutex_unlock(&fifo_mutex);
        } else {
            struct timespec ts;
            ts.tv_sec = sleep_time_ms / 1000;
            ts.tv_nsec = (sleep_time_ms % 1000) * 1000000; // convierte ms a ns
            nanosleep(&ts, NULL);
        }
    }

    // Avisamos a consumer
    pthread_mutex_lock(&producer_running_mutex);
    producer_running = TASK_STOPPED;
    pthread_mutex_unlock(&producer_running_mutex);
    // Desbloqueamos a consumer para que pueda leer la variable adc_running
    pthread_mutex_lock(&fifo_mutex);
    pthread_cond_broadcast(&fifo_cond);
    pthread_mutex_unlock(&fifo_mutex);
    //LOG_DEBUG("[Producer] Finalizo, aviso enviado al consumidor");

    pthread_mutex_lock(&shm->shared_mutex[MUTEX_ADC]);
    shm->shared[PRU_SHD_FLAGS_INDEX] &= ~(1 << trigger_flag);  // Detener adquisicion
    pthread_mutex_unlock(&shm->shared_mutex[MUTEX_ADC]);

    if (finished_due_to_timeout) {
        msg = MSG_ADC_TIMEOUT_EXPIRED;
    } else if (finished_due_to_adc_stopped) {
        msg = MSG_ADC_STOPPED;
    } else if (finished_normally) {
        msg = MSG_ADC_DONE;
    }

    pthread_mutex_lock(&args_mutex);
    notify_adc_status_message(args->mosq, msg);
    pthread_mutex_unlock(&args_mutex);

    return NULL;
}

void *consumer(void *arg) {

    ThreadAdcDataArgs *args = (ThreadAdcDataArgs *)arg;
    LOG_DEBUG("adc -> consumer running");

    pthread_mutex_lock(&args_mutex);
    int sample_rate = args->adc_data->sample_rate;
    int samples_per_node = args->adc_data->buffer_size / PRU_CIRCULAR_BUFFER_SIZE;
    pthread_mutex_unlock(&args_mutex);

    /*DEBUG
    // Abrir archivo binario
    FILE *adc_file = fopen(FILE_DATA_NAME, "wb");
    if (!adc_file) {
        LOG_ERROR("Error al abrir archivo de datos");
        return NULL;
    }
    DEBUG*/

    while (1) {
        pthread_mutex_lock(&fifo_mutex);

        while (TAILQ_EMPTY(&fifo_head)) {
            pthread_mutex_lock(&producer_running_mutex);
            bool still_running = producer_running;
            pthread_mutex_unlock(&producer_running_mutex);

            if (!still_running) {
                // Si no hay datos y ya no corre, terminamos
                pthread_mutex_unlock(&fifo_mutex);
		/*DEBUG
                fclose(adc_file);
		DEBUG*/
                LOG_DEBUG("adc consumer finish");
                return NULL;
            }

            // Si todavia deberia correr, esperamos
            pthread_cond_wait(&fifo_cond, &fifo_mutex);
        }

        // Sacar el nodo de la cola lo mas rapido posible
        Node *node = TAILQ_FIRST(&fifo_head);
        if (node) {
            TAILQ_REMOVE(&fifo_head, node, entries);
            queue_size--;
            pthread_cond_signal(&fifo_not_full);
        }

        pthread_mutex_unlock(&fifo_mutex);

        // Procesar nodo FUERA del mutex
        if (node) {

	    /*DEBUG
            // Escribir en archivo
            for (int j = 0; j < samples_per_node; j++) {
                uint16_t sample[4];
                for (int i = 0; i < 4; i++) {
                    sample[i] = (uint16_t)node->channels[i][j];
                }
                fwrite(sample, sizeof(uint16_t), 4, adc_file);
                //LOG_DEBUG("Muestra %d: ch0=%u ch1=%u ch2=%u ch3=%u", j, sample[0], sample[1], sample[2], sample[3]);
            }
	    DEBUG*/

            // Crear JSON
            cJSON *json_msg = cJSON_CreateObject();
            cJSON *json_channels = cJSON_CreateObject();
	    // Agregar task
	    cJSON_AddStringToObject(json_msg, "task", "adc");

	    // Agregar metadata: vref, resolution y fsample
	    cJSON_AddNumberToObject(json_msg, "vref", 	ADC_VREF);         // 1.8V
	    cJSON_AddNumberToObject(json_msg, "resolution", ADC_RESOLUTION);  // 4096 niveles
	    cJSON_AddNumberToObject(json_msg, "sample_rate", sample_rate);   // 100 kHz

            for (int i = 0; i < 4; i++) {
                cJSON *json_channel = cJSON_CreateArray();
                for (int j = 0; j < samples_per_node; j++) {
                    cJSON_AddItemToArray(json_channel, cJSON_CreateNumber(node->channels[i][j]));
                }
                char channel_name[10];
                snprintf(channel_name, sizeof(channel_name), "ch%d", i);
                cJSON_AddItemToObject(json_channels, channel_name, json_channel);
            }

            cJSON_AddItemToObject(json_msg, "channels", json_channels);

            // Publicar
            char *json_str = cJSON_PrintUnformatted(json_msg);
            mqtt_publish_async(mosq, TOPIC_RSP_ADC, json_str);

            cJSON_Delete(json_msg);
            free(json_str);
            free(node);
        }
    }
    LOG_DEBUG("adc consumer finish");
    /*DEBUG
    fclose(adc_file);
    DEBUG*/

    return NULL;
}

void fifo_reset(void) {
    Node *node;
    while (!TAILQ_EMPTY(&fifo_head)) {
        node = TAILQ_FIRST(&fifo_head);
        TAILQ_REMOVE(&fifo_head, node, entries);
        free(node);
    }
    TAILQ_INIT(&fifo_head);
    queue_size = 0;
}

static void notify_adc_status_message(struct mosquitto *mosq, const char *msg) {
    publish_adc_response_status(mosq, msg);
    LOG_INFO("%s", msg);
    lcd_show_message(msg);
}

