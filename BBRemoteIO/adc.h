#ifndef ADC_H
#define ADC_H

#include <mosquitto.h>
#include "shared_memory.h"
#include "task_queue.h"

#define ADC_TIMEOUT_MS                  50000

//ADC
#define ERR_INVALID_SAMPLES             "Err: samples    "
#define ERR_MODE_OUT_OF_RANGE           "Err: mode range "
#define ERR_SAMPLE_RATE_OUT_OF_RANGE    "Err: rate range "
#define ERR_BUFFER_SIZE_OUT_OF_RANGE    "Err: buffer size"
#define ERR_NUM_SAMPLES_OUT_OF_RANGE    "Err: samples num"
#define ERR_INVALID_TRIGGER             "Err: trigger val"
#define ERR_INVALID_SAMPLE_PERIOD       "Err: period val "


// 16 caracteres maximo
#define MSG_ADC_INVALID_MODE       	"Adc invalid mode"
#define MSG_ADC_DONE               	"Adc done        "
#define MSG_ADC_STOPPED            	"Adc stopped     "
#define MSG_ADC_TIMEOUT_EXPIRED    	"Expired timeout "
#define MSG_ADC_FINISH             	"Adc finish      "

// TASK
#define JSON_KEY_ADC_TASK_NAME          "adc"
#define JSON_KEY_ADC                    "adc"
#define JSON_KEY_SAMPLE_RATE            "sample_rate"
#define JSON_KEY_NUM_SAMPLES            "num_samples"
#define JSON_KEY_BUFFER_SIZE            "buffer_size"
#define JSON_KEY_ENABLE_TRIGGER         "enable_external_trigger"
#define JSON_KEY_SAMPLE_PERIOD          "sample_period"

#define JSON_KEY_STATE                  "state"
#define JSON_KEY_SAMPLES                "samples"
#define JSON_KEY_SAMPLE_RATE            "sample_rate"

#define ADC_MODE_MIN                    0
#define ADC_MODE_MAX                    1
#define ADC_SAMPLE_RATE_MIN             1
#define ADC_SAMPLE_RATE_MAX             40000
#define ADC_BUFFER_SIZE_MIN             8
#define ADC_BUFFER_SIZE_MAX             1024
#define ADC_NUM_SAMPLES_MIN             8
#define ADC_NUM_SAMPLES_MAX             5120
#define PRU_LINUX_PROCESS               0.02  // duracion en segundos del proceso Linux (~20ms)

extern volatile bool adc_running;

extern pthread_mutex_t adc_running_mutex;

extern TaskQueue adc_queue;
extern pthread_t adc_thread;

// Estructura para almacenar los datos del mensaje
typedef struct {
    int mode;
    int sample_rate;
    int sample_period;
    int buffer_size;
    int num_samples;
    int enable_external_trigger;
} AdcData;

// Argumentos enviados al hilo a traves de la cola
typedef struct {
    struct mosquitto *mosq;
    AdcData *adc_data;
} ThreadAdcDataArgs;

// Funcion que corre como hilo para manejar gpio_input
void *adc_thread_func(void *arg);

#endif
