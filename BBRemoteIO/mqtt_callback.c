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
#include "task_queue.h"
#include "log.h"

char json_payload[128];

static void handle_gpio_input_command(struct mosquitto *mosq, const char *payload);
static void handle_gpio_output_command(struct mosquitto *mosq, const char *payload);
static void handle_motor_get_command(struct mosquitto *mosq, const char *payload);
static void handle_motor_set_command(struct mosquitto *mosq, const char *payload);
static void handle_adc_command(struct mosquitto *mosq, const char *payload);
static void handle_system_command(struct mosquitto *mosq, const char *payload);

static bool is_running(volatile bool *flag, pthread_mutex_t *mutex);
static void set_running(volatile bool *flag, pthread_mutex_t *mutex, bool value);

void message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message) {

    const char *payload = (const char *)message->payload;
    LOG_DEBUG(MSG_RECEIVED, message->topic);
    LOG_DEBUG(MSG_RECEIVED_ON_TOPIC, (char *)message->payload);

    if (payload == NULL || payload[0] == '\0') {
        mqtt_publish_async(mosq, TOPIC_LOGS, MSG_INVALID_FORMAT_RESPONSE);
        LOG_ERROR(MSG_INVALID_FORMAT_RESPONSE);
        return;
    }


    if (strcmp(message->topic, TOPIC_CMDS_GPIO_INPUT) == 0) {

        handle_gpio_input_command(mosq, payload);

    } else if (strcmp(message->topic, TOPIC_CMDS_GPIO_OUTPUT) == 0) {

          handle_gpio_output_command(mosq, payload);

    } else if (strcmp(message->topic, TOPIC_CMDS_MOTOR_GET) == 0) {

        handle_motor_get_command(mosq, payload);

    } else if (strcmp(message->topic, TOPIC_CMDS_MOTOR_SET) == 0) {

        handle_motor_set_command(mosq, payload);

    } else if (strcmp(message->topic, TOPIC_CMDS_ADC) == 0) {

          handle_adc_command(mosq, payload);

    } else if (strcmp(message->topic, TOPIC_CMDS_SYSTEM) == 0) {

           handle_system_command(mosq, payload);

    } else {
        LOG_WARN(MSG_UNKNOWN_TOPIC);
    }
}

// HANDLERS

static void handle_gpio_input_command(struct mosquitto *mosq, const char *payload){

    if (is_running(&gpio_input_running, &gpio_input_running_mutex)) {
        LOG_INFO(INFO_FUNCTION_BUSY);
        return;
    }
    set_running(&gpio_input_running, &gpio_input_running_mutex, TASK_RUNNING);

    GpioInputData *gpio_input_data = malloc(sizeof(GpioInputData));
    if (!gpio_input_data) {
        LOG_ERROR(ERR_MALLOC_FUNCTION_DATA);
        set_running(&gpio_input_running, &gpio_input_running_mutex, TASK_STOPPED);
        return;
    }

    if (handle_gpio_input_message(mosq, payload, gpio_input_data) != 0) {
        free(gpio_input_data);
        set_running(&gpio_input_running, &gpio_input_running_mutex, TASK_STOPPED);
        return;
    }

    ThreadGpioInputDataArgs *args = malloc(sizeof(ThreadGpioInputDataArgs));
    if (!args) {
        LOG_ERROR(ERR_MALLOC_THREAD_ARGS);
        free(gpio_input_data);
        set_running(&gpio_input_running, &gpio_input_running_mutex, TASK_STOPPED);
        return;
    }

    args->mosq = mosq;
    args->gpio_input_data = gpio_input_data;

    if (task_queue_enqueue(&gpio_input_queue, args, sizeof(ThreadGpioInputDataArgs)) != 0) {
        LOG_INFO(INFO_FUNCTION_BUSY);
        free(gpio_input_data);
        free(args);
        set_running(&gpio_input_running, &gpio_input_running_mutex, TASK_STOPPED);
    } else {
        LOG_DEBUG(INFO_FUNCTION_ENQUEUED);
    }
}
static void handle_gpio_output_command(struct mosquitto *mosq, const char *payload){

    if (is_running(&gpio_output_running, &gpio_output_running_mutex)) {
        LOG_INFO(INFO_FUNCTION_BUSY);
        return;
    }
    set_running(&gpio_output_running, &gpio_output_running_mutex, TASK_RUNNING);

    GpioOutputData *gpio_output_data = malloc(sizeof(GpioOutputData));
    if (!gpio_output_data) {
        LOG_ERROR(ERR_MALLOC_FUNCTION_DATA);
        set_running(&gpio_output_running, &gpio_output_running_mutex, TASK_STOPPED);
        return;
    }
    int result = handle_gpio_output_message(mosq, payload, gpio_output_data);
    if (result != 0) {
        free(gpio_output_data);
        set_running(&gpio_output_running, &gpio_output_running_mutex, TASK_STOPPED);
        return;
    }
    ThreadGpioOutputDataArgs *args = malloc(sizeof(ThreadGpioOutputDataArgs));
    if (!args) {
        LOG_ERROR(ERR_MALLOC_THREAD_ARGS);
        free(gpio_output_data);
        set_running(&gpio_output_running, &gpio_output_running_mutex, TASK_STOPPED);
        return;
    }
    args->mosq = mosq;
    args->gpio_output_data = gpio_output_data;
    if (task_queue_enqueue(&gpio_output_queue, args, sizeof(ThreadGpioOutputDataArgs)) != 0) {
        LOG_INFO(INFO_FUNCTION_BUSY);
        free(gpio_output_data);
        free(args);
        set_running(&gpio_output_running, &gpio_output_running_mutex, TASK_STOPPED);
    } else {
        LOG_DEBUG(INFO_FUNCTION_ENQUEUED);
    }
}
static void handle_motor_get_command(struct mosquitto *mosq, const char *payload){

    if (is_running(&motor_get_running, &motor_get_running_mutex)) {
        LOG_INFO(INFO_FUNCTION_BUSY);
        return;
    }
    set_running(&motor_get_running, &motor_get_running_mutex, TASK_RUNNING);

    MotorGetData *motor_get_data = malloc(sizeof(MotorGetData));
    if (!motor_get_data) {
        LOG_ERROR(ERR_MALLOC_FUNCTION_DATA);
        set_running(&motor_get_running, &motor_get_running_mutex, TASK_STOPPED);
        return;
    }
    int result = handle_motor_get_message(mosq, payload, motor_get_data);
    if (result != 0) {
        free(motor_get_data);
        set_running(&motor_get_running, &motor_get_running_mutex, TASK_STOPPED);
        return;
    }
    ThreadMotorGetDataArgs *args = malloc(sizeof(ThreadMotorGetDataArgs));
    if (!args) {
        LOG_ERROR(ERR_MALLOC_THREAD_ARGS);
        free(motor_get_data);
        set_running(&motor_get_running, &motor_get_running_mutex, TASK_STOPPED);
        return;
    }
    args->mosq = mosq;
    args->motor_get_data = motor_get_data;
    if (task_queue_enqueue(&motor_get_queue, args, sizeof(ThreadMotorGetDataArgs)) != 0) {
        LOG_INFO(INFO_FUNCTION_BUSY);
        free(motor_get_data);
        free(args);
        set_running(&motor_get_running, &motor_get_running_mutex, TASK_STOPPED);
    } else {
        LOG_DEBUG(INFO_FUNCTION_ENQUEUED);
    }
}
static void handle_motor_set_command(struct mosquitto *mosq, const char *payload){

    if (is_running(&motor_set_running, &motor_set_running_mutex)) {
        LOG_INFO(INFO_FUNCTION_BUSY);
        return;
    }
    set_running(&motor_set_running, &motor_set_running_mutex, TASK_RUNNING);

    MotorSetData *motor_set_data = malloc(sizeof(MotorSetData));
    if (!motor_set_data) {
        LOG_ERROR(ERR_MALLOC_FUNCTION_DATA);
        set_running(&motor_set_running, &motor_set_running_mutex, TASK_STOPPED);
        return;
    }
    int result = handle_motor_set_message(mosq, payload, motor_set_data);
    if (result != 0) {
        free(motor_set_data);
        set_running(&motor_set_running, &motor_set_running_mutex, TASK_STOPPED);
        return;
    }
    ThreadMotorSetDataArgs *args = malloc(sizeof(ThreadMotorSetDataArgs));
    if (!args) {
        LOG_ERROR(ERR_MALLOC_THREAD_ARGS);
        free(motor_set_data);
        set_running(&motor_set_running, &motor_set_running_mutex, TASK_STOPPED);
        return;
    }
    args->mosq = mosq;
    args->motor_set_data = motor_set_data;
    if (task_queue_enqueue(&motor_set_queue, args, sizeof(ThreadMotorSetDataArgs)) != 0) {
        LOG_INFO(INFO_FUNCTION_BUSY);
        free(motor_set_data);
        free(args);
        set_running(&motor_set_running, &motor_set_running_mutex, TASK_STOPPED);
   } else {
       LOG_DEBUG(INFO_FUNCTION_ENQUEUED);
   }
}
static void handle_adc_command(struct mosquitto *mosq, const char *payload){

   if (is_running(&adc_running, &adc_running_mutex)) {
       LOG_INFO(INFO_FUNCTION_BUSY);
       return;
   }
   set_running(&adc_running, &adc_running_mutex, TASK_RUNNING);

   AdcData *adc_data = malloc(sizeof(AdcData));
   if (!adc_data) {
       LOG_ERROR(ERR_MALLOC_FUNCTION_DATA);
       set_running(&adc_running, &adc_running_mutex, TASK_STOPPED);
       return;
   }
   int result = handle_adc_message(mosq, payload, adc_data);
   if (result != 0) {
       free(adc_data);
       set_running(&adc_running, &adc_running_mutex, TASK_STOPPED);
       return;
   }
   ThreadAdcDataArgs *args = malloc(sizeof(ThreadAdcDataArgs));
   if (!args) {
       LOG_ERROR(ERR_MALLOC_THREAD_ARGS);
       free(adc_data);
       set_running(&adc_running, &adc_running_mutex, TASK_STOPPED);
       return;
   }
   args->mosq = mosq;
   args->adc_data = adc_data;
   if (task_queue_enqueue(&adc_queue, args, sizeof(ThreadAdcDataArgs)) != 0) {
       LOG_INFO(INFO_FUNCTION_BUSY);
       free(adc_data);
       free(args);
       set_running(&adc_running, &adc_running_mutex, TASK_STOPPED);
    } else {
        LOG_DEBUG(INFO_FUNCTION_ENQUEUED);
    }
}
static void handle_system_command(struct mosquitto *mosq, const char *payload){

    if (is_running(&system_running, &system_running_mutex)) {
        LOG_INFO(INFO_FUNCTION_BUSY);
        return;
    }
    set_running(&system_running, &system_running_mutex, TASK_RUNNING);

    SystemData *system_data = malloc(sizeof(SystemData));
    if (!system_data) {
        LOG_ERROR(ERR_MALLOC_FUNCTION_DATA);
        set_running(&system_running, &system_running_mutex, TASK_STOPPED);
        return;
    }
    int result = handle_system_message(mosq, payload, system_data);
    if (result != 0) {
        free(system_data);
        set_running(&system_running, &system_running_mutex, TASK_STOPPED);
        return;
    }
    ThreadSystemDataArgs *args = malloc(sizeof(ThreadSystemDataArgs));
    if (!args) {
        LOG_ERROR(ERR_MALLOC_THREAD_ARGS);
        free(system_data);
        set_running(&system_running, &system_running_mutex, TASK_STOPPED);
        return;
    }
    args->mosq = mosq;
    args->system_data = system_data;
    if (task_queue_enqueue(&system_queue, args, sizeof(ThreadSystemDataArgs)) != 0) {
        LOG_INFO(INFO_FUNCTION_BUSY);
        free(system_data);
        free(args);
        set_running(&system_running, &system_running_mutex, TASK_STOPPED);
    } else {
        LOG_DEBUG(INFO_FUNCTION_ENQUEUED);
    }
}

// AUXILIARES INTERNAS

int handle_adc_message(struct mosquitto *mosq, const char *message_payload, AdcData *adc_data) {
    // Validar el mensaje ADC
    int result = validate_adc_message(mosq, message_payload, adc_data);
    // Retornar el resultado de la operacion
    return (result == 0) ? 0 : -1;
}

int handle_gpio_input_message(struct mosquitto *mosq, const char *message_payload, GpioInputData *gpio_input_data) {
    // Verificar el mensaje JSON y cargar los datos en la estructura correspondiente
    int result = validate_gpio_input_message(mosq, message_payload, gpio_input_data);
    // Retornar el resultado de la operacion
    return (result == 0) ? 0 : -1;
}

int handle_gpio_output_message(struct mosquitto *mosq, const char *message_payload, GpioOutputData *gpio_output_data) {
    // Verificar el mensaje JSON y cargar los datos en la estructura correspondiente
    int result = validate_gpio_output_message(mosq, message_payload, gpio_output_data);
    // Retornar el resultado de la operacion
    return (result == 0) ? 0 : -1;
}

int handle_motor_get_message(struct mosquitto *mosq, const char *message_payload, MotorGetData *motor_get_data) {
    // Verificar el mensaje JSON y cargar los datos en la estructura correspondiente
    int result = validate_motor_get_message(mosq, message_payload, motor_get_data);
    // Retornar el resultado de la operacion
    return (result == 0) ? 0 : -1;
}

int handle_motor_set_message(struct mosquitto *mosq, const char *message_payload, MotorSetData *motor_set_data) {
    // Verificar el mensaje JSON y cargar los datos en la estructura correspondiente
    int result = validate_motor_set_message(mosq, message_payload, motor_set_data);
    // Retornar el resultado de la operacion
    return (result == 0) ? 0 : -1;
}

int handle_system_message(struct mosquitto *mosq, const char *message_payload, SystemData *system_data) {
    // Verificar el mensaje JSON y cargar los datos en la estructura correspondiente
    int result = validate_system_message(mosq, message_payload, system_data);
    // Retornar el resultado de la operacion
    return (result == 0) ? 0 : -1;
}

static bool is_running(volatile bool *flag, pthread_mutex_t *mutex) {
    pthread_mutex_lock(mutex);
    bool value = *flag;
    pthread_mutex_unlock(mutex);
    return value;
}
static void set_running(volatile bool *flag, pthread_mutex_t *mutex, bool value) {
    pthread_mutex_lock(mutex);
    *flag = value;
    pthread_mutex_unlock(mutex);
}
