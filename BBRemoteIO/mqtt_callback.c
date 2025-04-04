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
#include "lcd_system_status.h"

char json_payload[128];

pthread_mutex_t gpio_mutex = PTHREAD_MUTEX_INITIALIZER;

void message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message) {

    // Imprimir el mensaje recibido
    printf(MSG_RECEIVED, message->topic, (char *)message->payload);
    // Verificacion de mensaje nulo o vacio
    if (message->payload == NULL || *((char *)message->payload) == '\0') {
        printf(ERR_NULL_OR_EMPTY_MSG);
        const char *response = ERR_MSG_FORMAT;
        mosquitto_publish(mosq, NULL, TOPIC_LOGS, strlen(response), response, 0, false);
        lcd_system_status(LCD_MQTT_MSG_RECEIVED_ERROR);
	return;
    }

    // reserva de memoria para un posible hilo de tarea
    pthread_t thread;
    void *(*function)(void *);

    // Crear y llenar estructura de argumentos
    ThreadArgs *args = malloc(sizeof(ThreadArgs));
    if (!args) {
        fprintf(stderr, ERR_THREAD_MEM_ALLOC);
        return;
    }
    // Inicializamos a NULL todos los punteros
    args->adc_data = NULL;
    args->gpio_input_data = NULL;
    args->gpio_output_data = NULL;
    args->motor_data = NULL;
    args->system_data = NULL;

    // Verificar si ya hay una tarea en ejecuciOn
    if (strcmp(message->topic, TOPIC_CMDS_ADC) == 0) {
        // Verificamos el mensaje JSON y cargamos los datos en la estructura correspondiente
        AdcData *adc_data = malloc(sizeof(AdcData)); // Asignamos memoria solo si es necesario
        if (!adc_data) {
           fprintf(stderr, ERR_ADC_DATA_MEM_ALLOC);
           free(args); // Liberamos la memoria antes de salir
           return;
        }

	int result = handle_adc_message(mosq, (char *)message->payload, adc_data);
	if (result != 0) {
           lcd_system_status(LCD_MQTT_MSG_RECEIVED_ERROR);
    	   return;  // Terminar en caso de error
	}
        lcd_system_status(LCD_MQTT_MSG_RECEIVED_OK);
        args->adc_data=adc_data;
        function = adc_function;

    } else if (strcmp(message->topic, TOPIC_CMDS_GPIO_INPUT) == 0) {
        // Verificamos el mensaje JSON y cargamos los datos en la estructura correspondiente
        GpioInputData *gpio_input_data = malloc(sizeof(GpioInputData)); // Asignamos memoria solo si es necesario
        if (!gpio_input_data) {
           fprintf(stderr, ERR_GPIO_INPUT_DATA_MEM_ALLOC);
           free(args); // Liberamos la memoria antes de salir
           return;
        }

        int result = handle_gpio_input_message(mosq, (char *)message->payload, gpio_input_data);
        if (result != 0) {
            lcd_system_status(LCD_MQTT_MSG_RECEIVED_ERROR);
            free(gpio_input_data);
            return;  // Terminar en caso de error
        }
        lcd_system_status(LCD_MQTT_MSG_RECEIVED_OK);
        args->gpio_input_data=gpio_input_data;
        function = gpio_input_function;

    } else if (strcmp(message->topic, TOPIC_CMDS_GPIO_OUTPUT) == 0) {
        // Verificamos el mensaje JSON y cargamos los datos en la estructura correspondiente
        GpioOutputData *gpio_output_data = malloc(sizeof(GpioOutputData)); // Asignamos memoria solo si es necesario
        if (!gpio_output_data) {
           fprintf(stderr, ERR_GPIO_OUTPUT_DATA_MEM_ALLOC);
           free(args); // Liberamos la memoria antes de salir
           return;
        }

        int result = handle_gpio_output_message(mosq, (char *)message->payload, gpio_output_data);
        if (result != 0) {
            lcd_system_status(LCD_MQTT_MSG_RECEIVED_ERROR);
	    free(gpio_output_data);
            return;  // Terminar en caso de error
        }
        lcd_system_status(LCD_MQTT_MSG_RECEIVED_OK);
        args->gpio_output_data=gpio_output_data;
        function = gpio_output_function;

    } else if (strcmp(message->topic, TOPIC_CMDS_MOTOR) == 0) {
        // Verificamos el mensaje JSON y cargamos los datos en la estructura correspondiente
        MotorData *motor_data = malloc(sizeof(MotorData)); // Asignamos memoria solo si es necesario
        if (!motor_data) {
           fprintf(stderr, ERR_MOTOR_DATA_MEM_ALLOC);
           free(args); // Liberamos la memoria antes de salir
           return;
        }

        int result = handle_motor_message(mosq, (char *)message->payload, motor_data);
        if (result != 0) {
            lcd_system_status(LCD_MQTT_MSG_RECEIVED_ERROR);
            return;  // Terminar en caso de error
        }
        lcd_system_status(LCD_MQTT_MSG_RECEIVED_OK);
        args->motor_data=motor_data;
        function = motor_function;

    } else if (strcmp(message->topic, TOPIC_CMDS_SYSTEM) == 0) {
        // Verificamos el mensaje JSON y cargamos los datos en la estructura correspondiente
        SystemData *system_data = malloc(sizeof(SystemData)); // Asignamos memoria solo si es necesario
        if (!system_data) {
           fprintf(stderr, ERR_SYSTEM_DATA_MEM_ALLOC);
           free(args); // Liberamos la memoria antes de salir
           return;
        }

        int result = handle_system_message(mosq, (char *)message->payload, system_data);
        if (result != 0) {
            lcd_system_status(LCD_MQTT_MSG_RECEIVED_ERROR);
            return;  // Terminar en caso de error
        }

        lcd_system_status(LCD_MQTT_MSG_RECEIVED_OK);
        args->system_data=system_data;
        function = system_function;

    } else {
        printf(ERR_UNKNOWN_TOPIC);
        return;
    }

    args->mosq = mosq;

    if (pthread_create(&thread, NULL, function, args) == 0) {
        pthread_detach(thread); // El hilo se libera automaticamente
    } else {
       fprintf(stderr, "Error creando hilo \n");
       free(args);
    }
}

// AUXILIARES INTERNAS

int handle_adc_message(struct mosquitto *mosq, const char *message_payload, AdcData *adc_data) {
    // Validar el mensaje ADC
    int result = validate_adc_message(message_payload, adc_data);
    msg_received_rsp(mosq, result); // Error de formato
    // Retornar el resultado de la operacion
    return (result == 0) ? 0 : -1;
}

int handle_gpio_input_message(struct mosquitto *mosq, const char *message_payload, GpioInputData *gpio_input_data) {
    // Verificar el mensaje JSON y cargar los datos en la estructura correspondiente
    int result = validate_gpio_input_message(message_payload, gpio_input_data);
    msg_received_rsp(mosq, result); // Error de formato
    // Retornar el resultado de la operacion
    return (result == 0) ? 0 : -1;
}

int handle_gpio_output_message(struct mosquitto *mosq, const char *message_payload, GpioOutputData *gpio_output_data) {
    // Verificar el mensaje JSON y cargar los datos en la estructura correspondiente
    int result = validate_gpio_output_message(message_payload, gpio_output_data);
    msg_received_rsp(mosq, result); // Error de formato
    // Retornar el resultado de la operacion
    return (result == 0) ? 0 : -1;
}

int handle_motor_message(struct mosquitto *mosq, const char *message_payload, MotorData *motor_data) {
    // Verificar el mensaje JSON y cargar los datos en la estructura correspondiente
    int result = validate_motor_message(message_payload, motor_data);
    msg_received_rsp(mosq, result); // Error de formato
    // Retornar el resultado de la operacion
    return (result == 0) ? 0 : -1;
}

int handle_system_message(struct mosquitto *mosq, const char *message_payload, SystemData *system_data) {
    // Verificar el mensaje JSON y cargar los datos en la estructura correspondiente
    int result = validate_system_message(message_payload, system_data);
    msg_received_rsp(mosq, result); // Error de formato
    // Retornar el resultado de la operacion
    return (result == 0) ? 0 : -1;
}

void msg_received_rsp(struct mosquitto *mosq, int result){
    // Seleccionar la respuesta segun el resultado
    const char *response = (result == 0) ? MQTT_CALLBACK_RECEIVED_OK : MQTT_CALLBACK_RECEIVED_ERROR;

    // Publicar la respuesta
    mosquitto_publish(mosq, NULL, TOPIC_LOGS, strlen(response), response, 0, false);

    // Imprimir el mensaje de callback
    printf("%s\n", response);

}
