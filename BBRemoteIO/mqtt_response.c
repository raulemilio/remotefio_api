#include "mqtt_response.h"
#include <stdio.h>
#include <string.h>

#define HEADER_MESSAGE_GPIO_INPUT         "{\"task\":\"gpio_input\",\"status\":\"%s\",\"output\":["
#define HEADER_MESSAGE_GPIO_INPUT_ERROR   "{\"task\":\"gpio_input\",\"status\":\"%s\"}"

#define HEADER_MESSAGE_GPIO_OUTPUT_MODE1  "{\"task\":\"gpio_output\",\"status\":\"%s\"}"
#define HEADER_MESSAGE_GPIO_OUTPUT        "{\"task\":\"gpio_output\",\"status\":\"%s\",\"output\":["

#define HEADER_MESSAGE_MOTOR_MODE2        "{\"task\":\"motor\",\"status\":\"%s\"}"
#define HEADER_MESSAGE_MOTOR              "{\"task\":\"motor\",\"status\":\"%s\",\"motor\":["

#define HEADER_MESSAGE_SYSTEM             "{\"task\":\"system\",\"status\":\"%s\"}"

#define RESPONDING_SYSTEM         	  "Sending response system: %s\n"
#define RESPONDING_GPIO_INPUT     	  "Sending response gpio_input: %s\n"
#define RESPONDING_GPIO_OUTPUT    	  "Sending response gpio_output: %s\n"
#define RESPONDING_MOTOR          	  "Sending response motor: %s\n"

void publish_system_response_status(struct mosquitto *mosq, const char *status) {

    char json_payload[256]; // Ajustar el tamanio segun sea necesario
    snprintf(json_payload, sizeof(json_payload), HEADER_MESSAGE_SYSTEM, status);
    mosquitto_publish(mosq, NULL, TOPIC_RSP_SYSTEM, strlen(json_payload), json_payload, 0, false);
    printf(RESPONDING_SYSTEM, json_payload);
}

void publish_gpio_input_response_status(struct mosquitto *mosq, const char *status) {

    char json_payload[256]; // Ajustar el tamanio segun sea necesario
    snprintf(json_payload, sizeof(json_payload), HEADER_MESSAGE_GPIO_INPUT_ERROR, status);
    mosquitto_publish(mosq, NULL, TOPIC_RSP_GPIO_INPUT, strlen(json_payload), json_payload, 0, false);
    printf(RESPONDING_GPIO_INPUT, json_payload);
}

void publish_gpio_output_response_status(struct mosquitto *mosq, const char *status) {

    char json_payload[256];
    snprintf(json_payload, sizeof(json_payload), HEADER_MESSAGE_GPIO_OUTPUT_MODE1, status);
    mosquitto_publish(mosq, NULL, TOPIC_RSP_GPIO_OUTPUT, strlen(json_payload), json_payload, 0, false);
    printf(RESPONDING_GPIO_OUTPUT, json_payload);
}

void publish_motor_response_status(struct mosquitto *mosq, const char *status) {

    char json_payload[256]; // Ajustar el tamanio segun sea necesario
    snprintf(json_payload, sizeof(json_payload), HEADER_MESSAGE_MOTOR_MODE2, status);
    mosquitto_publish(mosq, NULL, TOPIC_RSP_MOTOR, strlen(json_payload), json_payload, 0, false);
    printf(RESPONDING_MOTOR, json_payload);
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
    mosquitto_publish(mosq, NULL, TOPIC_RSP_GPIO_INPUT, strlen(json_payload), json_payload, 0, false);
    printf(RESPONDING_GPIO_INPUT, json_payload);
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

    mosquitto_publish(mosq, NULL, TOPIC_RSP_GPIO_OUTPUT, strlen(json_payload), json_payload, 0, false);
    printf(RESPONDING_GPIO_OUTPUT, json_payload);
}

void publish_motor_response(struct mosquitto *mosq, MotorData motor_data_send, const char *status) {

    char json_payload[256]; // Ajustar el tamanio segun sea necesario

    int offset = snprintf(json_payload, sizeof(json_payload), HEADER_MESSAGE_MOTOR, status);

    // Agregar dinamicamente los valores de motor
    for (int i = 0; i < motor_data_send.num_motor; i++) {
        if (i > 0) {
            offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, ",");
        }
        offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, "%d", motor_data_send.motor[i]);
    }

    // Agregar los valores de ena
    offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, "],\"ena\":[");
    for (int i = 0; i < motor_data_send.num_motor; i++) {
        if (i > 0) {
            offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, ",");
        }
        offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, "%d", motor_data_send.ena[i]);
    }

    // Agregar los valores de dir
    offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, "],\"dir\":[");
    for (int i = 0; i < motor_data_send.num_motor; i++) {
        if (i > 0) {
            offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, ",");
        }
        offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, "%d", motor_data_send.dir[i]);
    }

    // Agregar los valores de step_time
    offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, "],\"step_time\":[");
    for (int i = 0; i < motor_data_send.num_motor; i++) {
        if (i > 0) {
            offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, ",");
        }
        offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, "%d", motor_data_send.step_time[i]);
    }

    // Cerrar el JSON
    snprintf(json_payload + offset, sizeof(json_payload) - offset, "]}");

    // Publicar el mensaje
    mosquitto_publish(mosq, NULL, TOPIC_RSP_MOTOR, strlen(json_payload), json_payload, 0, false);
    printf(RESPONDING_MOTOR, json_payload);
}

