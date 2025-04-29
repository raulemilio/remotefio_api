#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdarg.h>
#include "mqtt_publish.h"
#include "mqtt_response.h"
#include "log.h"

#define HEADER_MESSAGE_GPIO_INPUT             "{\"task\":\"gpio_input\",\"status\":\"%s\",\"input\":["
#define HEADER_MESSAGE_GPIO_OUTPUT_GET        "{\"task\":\"gpio_output_get\",\"status\":\"%s\",\"output\":["
#define HEADER_MESSAGE_GPIO_OUTPUT_SET        "{\"task\":\"gpio_output_set\",\"status\":\"%s\",\"output\":["
#define HEADER_MESSAGE_MOTOR_GET              "{\"task\":\"motor_get\",\"status\":\"%s\",\"motor\":["
#define HEADER_MESSAGE_MOTOR_SET              "{\"task\":\"motor_set\",\"status\":\"%s\",\"motor\":["

#define HEADER_MESSAGE_ADC_STATUS             "{\"task\":\"adc\",\"status\":\"%s\"}"
#define HEADER_MESSAGE_GPIO_INPUT_STATUS      "{\"task\":\"gpio_input\",\"status\":\"%s\"}"
#define HEADER_MESSAGE_GPIO_OUTPUT_GET_STATUS "{\"task\":\"gpio_output_get\",\"status\":\"%s\"}"
#define HEADER_MESSAGE_GPIO_OUTPUT_SET_STATUS "{\"task\":\"gpio_output_set\",\"status\":\"%s\"}"
#define HEADER_MESSAGE_MOTOR_GET_STATUS       "{\"task\":\"motor_get\",\"status\":\"%s\"}"
#define HEADER_MESSAGE_MOTOR_SET_STATUS       "{\"task\":\"motor_set\",\"status\":\"%s\"}"

#define HEADER_MESSAGE_SYSTEM_STATUS          "{\"task\":\"system\",\"status\":\"%s\"}"

#define RESPONDING_ADC                        "Sending response adc: %s"
#define RESPONDING_GPIO_INPUT     	      "Sending response gpio_input: %s"
#define RESPONDING_GPIO_OUTPUT_GET    	      "Sending response gpio_output_set: %s"
#define RESPONDING_GPIO_OUTPUT_SET            "Sending response gpio_output_set: %s"
#define RESPONDING_MOTOR_GET          	      "Sending response motor_get: %s"
#define RESPONDING_MOTOR_SET                  "Sending response motor_set: %s"
#define RESPONDING_SYSTEM                     "Sending response system: %s"

#define PAYLOAD_DATA_SIZE   512
#define PAYLOAD_STATUS_SIZE 256

static int safe_append(char *buffer, size_t buffer_size, int offset, const char *format, ...);

// Funcioon auxiliar segura para agregar cadenas al buffer
static int safe_append(char *buffer, size_t buffer_size, int offset, const char *format, ...) {
    if (offset >= (int)buffer_size) return offset;

    va_list args;
    va_start(args, format);
    int written = vsnprintf(buffer + offset, buffer_size - offset, format, args);
    va_end(args);

    if (written < 0) return offset; // error
    if (offset + written >= (int)buffer_size) return (int)buffer_size; // overflow

    return offset + written;
}

// ----------- FUNCIONES DE PUBLICACION UNIFICADAS ------------

void publish_gpio_input_response(struct mosquitto *mosq, GpioInputData gpio_input_data_send, const char *status) {
    char json_payload[PAYLOAD_DATA_SIZE];
    int offset = 0;

    offset = safe_append(json_payload, sizeof(json_payload), offset, HEADER_MESSAGE_GPIO_INPUT, status);

    for (int i = 0; i < gpio_input_data_send.num_input; i++) {
        if (i > 0) offset = safe_append(json_payload, sizeof(json_payload), offset, ",");
        offset = safe_append(json_payload, sizeof(json_payload), offset, "%d", gpio_input_data_send.input[i]);
    }

    offset = safe_append(json_payload, sizeof(json_payload), offset, "],\"state\":[");
    for (int i = 0; i < gpio_input_data_send.num_input; i++) {
        if (i > 0) offset = safe_append(json_payload, sizeof(json_payload), offset, ",");
        offset = safe_append(json_payload, sizeof(json_payload), offset, "%d", gpio_input_data_send.state[i]);
    }

    offset = safe_append(json_payload, sizeof(json_payload), offset, "],\"ts\":%" PRIu64 "}", gpio_input_data_send.ts);

    mqtt_publish_async(mosq, TOPIC_RSP_GPIO_INPUT, json_payload);
    LOG_DEBUG(RESPONDING_GPIO_INPUT, json_payload);
}

void publish_gpio_output_get_response(struct mosquitto *mosq, GpioOutputGetData gpio_output_get_data_send, const char *status) {
    char json_payload[PAYLOAD_DATA_SIZE];
    int offset = 0;

    offset = safe_append(json_payload, sizeof(json_payload), offset, HEADER_MESSAGE_GPIO_OUTPUT_GET, status);

    for (int i = 0; i < gpio_output_get_data_send.num_output; i++) {
        if (i > 0) offset = safe_append(json_payload, sizeof(json_payload), offset, ",");
        offset = safe_append(json_payload, sizeof(json_payload), offset, "%d", gpio_output_get_data_send.output[i]);
    }

    offset = safe_append(json_payload, sizeof(json_payload), offset, "],\"state\":[");
    for (int i = 0; i < gpio_output_get_data_send.num_output; i++) {
        if (i > 0) offset = safe_append(json_payload, sizeof(json_payload), offset, ",");
        offset = safe_append(json_payload, sizeof(json_payload), offset, "%d", gpio_output_get_data_send.state[i]);
    }

    offset = safe_append(json_payload, sizeof(json_payload), offset, "],\"ts\":%" PRIu64 "}", gpio_output_get_data_send.ts);

    mqtt_publish_async(mosq, TOPIC_RSP_GPIO_OUTPUT_GET, json_payload);
    LOG_DEBUG(RESPONDING_GPIO_OUTPUT_GET, json_payload);
}

void publish_motor_get_response(struct mosquitto *mosq, MotorGetData motor_get_data_send, const char *status) {
    char json_payload[PAYLOAD_DATA_SIZE];
    int offset = 0;

    offset = safe_append(json_payload, sizeof(json_payload), offset, HEADER_MESSAGE_MOTOR_GET, status);

    for (int i = 0; i < motor_get_data_send.num_motor; i++) {
        if (i > 0) offset = safe_append(json_payload, sizeof(json_payload), offset, ",");
        offset = safe_append(json_payload, sizeof(json_payload), offset, "%d", motor_get_data_send.motor[i]);
    }

    offset = safe_append(json_payload, sizeof(json_payload), offset, "],\"ena\":[");
    for (int i = 0; i < motor_get_data_send.num_motor; i++) {
        if (i > 0) offset = safe_append(json_payload, sizeof(json_payload), offset, ",");
        offset = safe_append(json_payload, sizeof(json_payload), offset, "%d", motor_get_data_send.ena[i]);
    }

    offset = safe_append(json_payload, sizeof(json_payload), offset, "],\"dir\":[");
    for (int i = 0; i < motor_get_data_send.num_motor; i++) {
        if (i > 0) offset = safe_append(json_payload, sizeof(json_payload), offset, ",");
        offset = safe_append(json_payload, sizeof(json_payload), offset, "%d", motor_get_data_send.dir[i]);
    }

    offset = safe_append(json_payload, sizeof(json_payload), offset, "],\"rpm\":[");
    for (int i = 0; i < motor_get_data_send.num_motor; i++) {
        if (i > 0) offset = safe_append(json_payload, sizeof(json_payload), offset, ",");
        offset = safe_append(json_payload, sizeof(json_payload), offset, "%d", motor_get_data_send.rpm[i]);
    }

    offset = safe_append(json_payload, sizeof(json_payload), offset, "],\"step_per_rev\":[");
    for (int i = 0; i < motor_get_data_send.num_motor; i++) {
        if (i > 0) offset = safe_append(json_payload, sizeof(json_payload), offset, ",");
        offset = safe_append(json_payload, sizeof(json_payload), offset, "%d", motor_get_data_send.step_per_rev[i]);
    }

    offset = safe_append(json_payload, sizeof(json_payload), offset, "],\"micro_step\":[");
    for (int i = 0; i < motor_get_data_send.num_motor; i++) {
        if (i > 0) offset = safe_append(json_payload, sizeof(json_payload), offset, ",");
        offset = safe_append(json_payload, sizeof(json_payload), offset, "%d", motor_get_data_send.micro_step[i]);
    }

    offset = safe_append(json_payload, sizeof(json_payload), offset, "],\"ts\":%" PRIu64 "}", motor_get_data_send.ts);

    mqtt_publish_async(mosq, TOPIC_RSP_MOTOR_GET, json_payload);
    LOG_DEBUG(RESPONDING_MOTOR_GET, json_payload);
}

// ADC
void publish_adc_response_status(struct mosquitto *mosq, const char *status) {

    char json_payload[PAYLOAD_STATUS_SIZE]; // Ajustar el tamanio segun sea necesario
    snprintf(json_payload, sizeof(json_payload), HEADER_MESSAGE_ADC_STATUS, status);
    mqtt_publish_async(mosq, TOPIC_LOGS, json_payload);
    LOG_DEBUG(RESPONDING_ADC, json_payload);
}
//GPIO_INPUT
void publish_gpio_input_response_status(struct mosquitto *mosq, const char *status) {

    char json_payload[PAYLOAD_STATUS_SIZE]; // Ajustar el tamanio segun sea necesario
    snprintf(json_payload, sizeof(json_payload), HEADER_MESSAGE_GPIO_INPUT_STATUS, status);
    mqtt_publish_async(mosq, TOPIC_LOGS, json_payload);
    LOG_DEBUG(RESPONDING_GPIO_INPUT, json_payload);
}
// GPIO OUTPUT GET
void publish_gpio_output_get_response_status(struct mosquitto *mosq, const char *status) {

    char json_payload[PAYLOAD_STATUS_SIZE];
    snprintf(json_payload, sizeof(json_payload), HEADER_MESSAGE_GPIO_OUTPUT_GET_STATUS, status);
    mqtt_publish_async(mosq, TOPIC_LOGS, json_payload);
    LOG_DEBUG(RESPONDING_GPIO_OUTPUT_GET, json_payload);
}
// GPIO OUTPUT SET
void publish_gpio_output_set_response_status(struct mosquitto *mosq, const char *status) {

    char json_payload[PAYLOAD_STATUS_SIZE];
    snprintf(json_payload, sizeof(json_payload), HEADER_MESSAGE_GPIO_OUTPUT_SET_STATUS, status);
    mqtt_publish_async(mosq, TOPIC_LOGS, json_payload);
    LOG_DEBUG(RESPONDING_GPIO_OUTPUT_SET, json_payload);
}
// MOTOR GET
void publish_motor_get_response_status(struct mosquitto *mosq, const char *status) {

    char json_payload[PAYLOAD_STATUS_SIZE]; // Ajustar el tamanio segun sea necesario
    snprintf(json_payload, sizeof(json_payload), HEADER_MESSAGE_MOTOR_GET_STATUS, status);
    mqtt_publish_async(mosq, TOPIC_LOGS, json_payload);
    LOG_DEBUG(RESPONDING_MOTOR_GET, json_payload);
}
// MOTOR SET
void publish_motor_set_response_status(struct mosquitto *mosq, const char *status) {

    char json_payload[PAYLOAD_STATUS_SIZE]; // Ajustar el tamanio segun sea necesario
    snprintf(json_payload, sizeof(json_payload), HEADER_MESSAGE_MOTOR_SET_STATUS, status);
    mqtt_publish_async(mosq, TOPIC_LOGS, json_payload);
    LOG_DEBUG(RESPONDING_MOTOR_SET, json_payload);
}
// SYSTEM
void publish_system_response_status(struct mosquitto *mosq, const char *status) {

    char json_payload[PAYLOAD_STATUS_SIZE]; // Ajustar el tamanio segun sea necesario
    snprintf(json_payload, sizeof(json_payload), HEADER_MESSAGE_SYSTEM_STATUS, status);
    mqtt_publish_async(mosq, TOPIC_LOGS, json_payload);
    LOG_DEBUG(RESPONDING_SYSTEM, json_payload);
}
