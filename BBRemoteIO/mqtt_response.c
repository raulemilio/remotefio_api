#include <stdio.h>
#include <string.h>
#include "mqtt_publish.h"
#include "mqtt_response.h"
#include "log.h"

#define HEADER_MESSAGE_GPIO_INPUT         "{\"task\":\"gpio_input\",\"status\":\"%s\",\"input\":["
#define HEADER_MESSAGE_GPIO_OUTPUT        "{\"task\":\"gpio_output\",\"status\":\"%s\",\"output\":["
#define HEADER_MESSAGE_MOTOR_GET          "{\"task\":\"motor_get\",\"status\":\"%s\",\"motor\":["
#define HEADER_MESSAGE_MOTOR_SET          "{\"task\":\"motor_set\",\"status\":\"%s\",\"motor\":["

#define HEADER_MESSAGE_ADC_STATUS         "{\"task\":\"adc\",\"status\":\"%s\"}"
#define HEADER_MESSAGE_GPIO_INPUT_STATUS  "{\"task\":\"gpio_input\",\"status\":\"%s\"}"
#define HEADER_MESSAGE_GPIO_OUTPUT_STATUS "{\"task\":\"gpio_output\",\"status\":\"%s\"}"
#define HEADER_MESSAGE_MOTOR_GET_STATUS   "{\"task\":\"motor_get\",\"status\":\"%s\"}"
#define HEADER_MESSAGE_MOTOR_SET_STATUS   "{\"task\":\"motor_set\",\"status\":\"%s\"}"

#define HEADER_MESSAGE_SYSTEM_STATUS      "{\"task\":\"system\",\"status\":\"%s\"}"

#define RESPONDING_ADC                    "Sending response adc: %s"
#define RESPONDING_GPIO_INPUT     	  "Sending response gpio_input: %s"
#define RESPONDING_GPIO_OUTPUT    	  "Sending response gpio_output: %s"
#define RESPONDING_MOTOR_GET          	  "Sending response motor_get: %s"
#define RESPONDING_MOTOR_SET              "Sending response motor_set: %s"
#define RESPONDING_SYSTEM                 "Sending response system: %s"

void publish_adc_response_status(struct mosquitto *mosq, const char *status) {

    char json_payload[256]; // Ajustar el tamanio segun sea necesario
    snprintf(json_payload, sizeof(json_payload), HEADER_MESSAGE_ADC_STATUS, status);
    mqtt_publish_async(mosq, TOPIC_RSP_ADC, json_payload);
    LOG_DEBUG(RESPONDING_ADC, json_payload);
}

void publish_gpio_input_response_status(struct mosquitto *mosq, const char *status) {

    char json_payload[256]; // Ajustar el tamanio segun sea necesario
    snprintf(json_payload, sizeof(json_payload), HEADER_MESSAGE_GPIO_INPUT_STATUS, status);
    mqtt_publish_async(mosq, TOPIC_RSP_GPIO_INPUT, json_payload);
    LOG_DEBUG(RESPONDING_GPIO_INPUT, json_payload);
}
// GPIO OUTPUT
void publish_gpio_output_response_status(struct mosquitto *mosq, const char *status) {

    char json_payload[256];
    snprintf(json_payload, sizeof(json_payload), HEADER_MESSAGE_GPIO_OUTPUT_STATUS, status);
    mqtt_publish_async(mosq, TOPIC_RSP_GPIO_OUTPUT, json_payload);
    LOG_DEBUG(RESPONDING_GPIO_OUTPUT, json_payload);
}
// MOTOR GET
void publish_motor_get_response_status(struct mosquitto *mosq, const char *status) {

    char json_payload[256]; // Ajustar el tamanio segun sea necesario
    snprintf(json_payload, sizeof(json_payload), HEADER_MESSAGE_MOTOR_GET_STATUS, status);
    mqtt_publish_async(mosq, TOPIC_RSP_MOTOR_GET, json_payload);
    LOG_DEBUG(RESPONDING_MOTOR_GET, json_payload);
}
// MOTOR SET
void publish_motor_set_response_status(struct mosquitto *mosq, const char *status) {

    char json_payload[256]; // Ajustar el tamanio segun sea necesario
    snprintf(json_payload, sizeof(json_payload), HEADER_MESSAGE_MOTOR_SET_STATUS, status);
    mqtt_publish_async(mosq, TOPIC_RSP_MOTOR_SET, json_payload);
    LOG_DEBUG(RESPONDING_MOTOR_SET, json_payload);
}

void publish_system_response_status(struct mosquitto *mosq, const char *status) {

    char json_payload[256]; // Ajustar el tamanio segun sea necesario
    snprintf(json_payload, sizeof(json_payload), HEADER_MESSAGE_SYSTEM_STATUS, status);
    mqtt_publish_async(mosq, TOPIC_RSP_SYSTEM, json_payload);
    LOG_DEBUG(RESPONDING_SYSTEM, json_payload);
}

void publish_gpio_input_response(struct mosquitto *mosq, GpioInputData gpio_input_data_send, const char *status) {

    char json_payload[256]; // Ajustar el tamanio segun sea necesario

    int offset = snprintf(json_payload, sizeof(json_payload), HEADER_MESSAGE_GPIO_INPUT, status);

    // Agregar dinamicamente los valores de los pines
    for (int i = 0; i < gpio_input_data_send.num_input; i++) {
       if (i > 0) {
        offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, ",");
       }
    offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, "%d", gpio_input_data_send.input[i]);
    }

    // Agregar los estados de los pines
    offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, "],\"states\":[");

    for (int i = 0; i < gpio_input_data_send.num_input; i++) {
       if (i > 0) {
        offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, ",");
    }
    offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, "%d", gpio_input_data_send.state[i]);
    }

    // Cerrar el JSON
    snprintf(json_payload + offset, sizeof(json_payload) - offset, "]}");

    // Publicar el mensaje
    mqtt_publish_async(mosq, TOPIC_RSP_GPIO_INPUT, json_payload);
    // Publicar el mensaje
    LOG_DEBUG(RESPONDING_GPIO_INPUT, json_payload);
}


void publish_gpio_output_response(struct mosquitto *mosq, GpioOutputData gpio_output_data_send, const char *status) {

    char json_payload[256];

    int offset = snprintf(json_payload, sizeof(json_payload), HEADER_MESSAGE_GPIO_OUTPUT, status);

    for (int i = 0; i < gpio_output_data_send.num_output; i++) {
        if (i > 0) offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, ",");
        offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, "%d", gpio_output_data_send.output[i]);
    }

    offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, "],\"states\":[");
    for (int i = 0; i < gpio_output_data_send.num_output; i++) {
        if (i > 0) offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, ",");
        offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, "%d", gpio_output_data_send.state[i]);
    }

    snprintf(json_payload + offset, sizeof(json_payload) - offset, "]}");

    mqtt_publish_async(mosq, TOPIC_RSP_GPIO_OUTPUT, json_payload);
    LOG_DEBUG(RESPONDING_GPIO_OUTPUT, json_payload);
}

void publish_motor_get_response(struct mosquitto *mosq, MotorGetData motor_get_data_send, const char *status) {

    char json_payload[256]; // Ajustar el tamanio segun sea necesario

    int offset = snprintf(json_payload, sizeof(json_payload), HEADER_MESSAGE_MOTOR_GET, status);

    // Agregar dinamicamente los valores de motor
    for (int i = 0; i < motor_get_data_send.num_motor; i++) {
        if (i > 0) {
            offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, ",");
        }
        offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, "%d", motor_get_data_send.motor[i]);
    }

    // Agregar los valores de ena
    offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, "],\"ena\":[");
    for (int i = 0; i < motor_get_data_send.num_motor; i++) {
        if (i > 0) {
            offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, ",");
        }
        offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, "%d", motor_get_data_send.ena[i]);
    }

    // Agregar los valores de dir
    offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, "],\"dir\":[");
    for (int i = 0; i < motor_get_data_send.num_motor; i++) {
        if (i > 0) {
            offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, ",");
        }
        offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, "%d", motor_get_data_send.dir[i]);
    }

    // Agregar los valores de step_time
    offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, "],\"step_time\":[");
    for (int i = 0; i < motor_get_data_send.num_motor; i++) {
        if (i > 0) {
            offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, ",");
        }
        offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, "%d", motor_get_data_send.step_time[i]);
    }

    // Cerrar el JSON
    snprintf(json_payload + offset, sizeof(json_payload) - offset, "]}");

    // Publicar el mensaje
    mqtt_publish_async(mosq, TOPIC_RSP_MOTOR_GET, json_payload);
    LOG_DEBUG(RESPONDING_MOTOR_GET, json_payload);
}

void publish_motor_set_response(struct mosquitto *mosq, MotorSetData motor_set_data_send, const char *status) {

    char json_payload[256]; // Ajustar el tamanio segun sea necesario

    int offset = snprintf(json_payload, sizeof(json_payload), HEADER_MESSAGE_MOTOR_SET, status);

    // Agregar dinamicamente los valores de motor
    for (int i = 0; i < motor_set_data_send.num_motor; i++) {
        if (i > 0) {
            offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, ",");
        }
        offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, "%d", motor_set_data_send.motor[i]);
    }

    // Agregar los valores de ena
    offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, "],\"ena\":[");
    for (int i = 0; i < motor_set_data_send.num_motor; i++) {
        if (i > 0) {
            offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, ",");
        }
        offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, "%d", motor_set_data_send.ena[i]);
    }

    // Agregar los valores de dir
    offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, "],\"dir\":[");
    for (int i = 0; i < motor_set_data_send.num_motor; i++) {
        if (i > 0) {
            offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, ",");
        }
        offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, "%d", motor_set_data_send.dir[i]);
    }

    // Agregar los valores de step_time
    offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, "],\"step_time\":[");
    for (int i = 0; i < motor_set_data_send.num_motor; i++) {
        if (i > 0) {
            offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, ",");
        }
        offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, "%d", motor_set_data_send.step_time[i]);
    }

    // Cerrar el JSON
    snprintf(json_payload + offset, sizeof(json_payload) - offset, "]}");

    // Publicar el mensaje
    mqtt_publish_async(mosq, TOPIC_RSP_MOTOR_SET, json_payload);
    LOG_DEBUG(RESPONDING_MOTOR_SET, json_payload);
}
