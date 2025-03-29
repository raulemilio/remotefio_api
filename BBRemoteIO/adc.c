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

void *adc_function(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    struct mosquitto *mosq = args->mosq;

    //printf(ERR_ADC_TASK_IN_PROGRESS);
    //lcd_system_status(LCD_MQTT_MSG_RECEIVED_ADC_BUSY);

    // Liberar la memoria de los argumentos si fue asignada dinámicamente
    free(args);
    pthread_exit(NULL);
}
