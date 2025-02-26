#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <mosquitto.h>

#define BROKER_ADDRESS "192.168.7.1"
#define BROKER_PORT 1883
#define CLIENT_ID "BeagleBoneClient"
#define TOPIC_CMDS_ADC "BBRemote/cmds/adc"
#define TOPIC_CMDS_GPIO_READ "BBRemote/cmds/gpio_read"
#define TOPIC_CMDS_GPIO_WRITE "BBRemote/cmds/gpio_write"
#define TOPIC_CMDS_MOTOR "BBRemote/cmds/motor"

#define TOPIC_RSP_ADC "BBRemote/rsp/adc"
#define TOPIC_RSP_GPIO_READ "BBRemote/rsp/gpio_read"
#define TOPIC_RSP_GPIO_WRITE "BBRemote/rsp/gpio_write"
#define TOPIC_RSP_MOTOR "BBRemote/rsp/motor"

struct mosquitto *mosq = NULL;

// Flags para indicar si una tarea en ejecucion
int adc_in_progress = 0;
int gpio_in_in_progress = 0;
int gpio_out_in_progress = 0;
int motor_in_progress = 0;

// Estructura para pasar los argumentos al hilo
typedef struct {
    struct mosquitto *mosq;
    char payload[256];  // Tamanio seguro para el payload
} ThreadArgs;

// Manejo de seniales para salir limpiamente
void handle_signal(int signo) {
    printf("Cerrando cliente MQTT...\n");
    if (mosq) {
        mosquitto_disconnect(mosq);
        mosquitto_destroy(mosq);
    }
    mosquitto_lib_cleanup();
    exit(0);
}

void *adc_function(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;  // Casteo correcto del argumento
    struct mosquitto *mosq = args->mosq;
    char *payload = args->payload;

    printf("ADC iniciado con payload: %s\n", payload);

    // Simulacion de la lectura ADC
    sleep(6);

    // Respuesta al topico
    const char *response = "Comando ADC recibido";
    mosquitto_publish(mosq, NULL, TOPIC_RSP_ADC, strlen(response), response, 0, false);
    printf("Enviando respuesta ADC: %s\n", response);

    // Liberar recursos
    adc_in_progress = 0;  // Marcar que el ADC ya no esta en progreso
    free(args);  // Importante: liberar la memoria del argumento

    pthread_exit(NULL);
}

void *gpio_read_function(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;  // Casteo correcto del argumento
    struct mosquitto *mosq = args->mosq;
    char *payload = args->payload;

    // Logica para GPIO READ
    sleep(5);
    const char *response = "Comando GPIO read recibido";
    mosquitto_publish(mosq, NULL, TOPIC_RSP_GPIO_READ, strlen(response), response, 0, false);
    printf("Enviando respuesta GPIO read: %s\n", response);

    // Liberar recursos antes de terminar
    gpio_in_in_progress = 0; // Liberar el flag
    pthread_exit(NULL);
}

void *gpio_write_function(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;  // Casteo correcto del argumento
    struct mosquitto *mosq = args->mosq;
    char *payload = args->payload;

    // Logica para GPIO WRITE
    sleep(6);
    const char *response = "Comando GPIO write recibido";
    mosquitto_publish(mosq, NULL, TOPIC_RSP_GPIO_WRITE, strlen(response), response, 0, false);
    printf("Enviando respuesta GPIO Out: %s\n", response);

    // Liberar recursos antes de terminar
    gpio_out_in_progress = 0; // Liberar el flag
    pthread_exit(NULL);
}

void *motor_function(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;  // Casteo correcto del argumento
    struct mosquitto *mosq = args->mosq;
    char *payload = args->payload;

    // Logica para Motor
    sleep(5);
    const char *response = "Comando Motor recibido";
    mosquitto_publish(mosq, NULL, TOPIC_RSP_MOTOR, strlen(response), response, 0, false);
    printf("Enviando respuesta Motor: %s\n", response);

    // Liberar recursos antes de terminar
    motor_in_progress = 0; // Liberar el flag
    pthread_exit(NULL);
}

void message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message) {
    printf("Mensaje recibido en %s: %s\n", message->topic, (char *)message->payload);

    pthread_t thread;
    void *(*function)(void *);

    // Verificar si ya hay una tarea en ejecuciOn
    if (strcmp(message->topic, TOPIC_CMDS_ADC) == 0) {
        if (adc_in_progress) {
            printf("Ya hay una tarea ADC en progreso. No se ejecutara otra.\n");
            return;
        }
        adc_in_progress = 1;
        function = adc_function;
    } else if (strcmp(message->topic, TOPIC_CMDS_GPIO_READ) == 0) {
        if (gpio_in_in_progress) {
            printf("Ya hay una tarea GPIO Read en progreso. No se ejecutara otra.\n");
            return;
        }
        gpio_in_in_progress = 1;
        function = gpio_read_function;
    } else if (strcmp(message->topic, TOPIC_CMDS_GPIO_WRITE) == 0) {
        if (gpio_out_in_progress) {
            printf("Ya hay una tarea GPIO Write en progreso. No se ejecutara otra.\n");
            return;
        }
        gpio_out_in_progress = 1;
        function = gpio_write_function;
    } else if (strcmp(message->topic, TOPIC_CMDS_MOTOR) == 0) {
        if (motor_in_progress) {
            printf("Ya hay una tarea Motor en progreso. No se ejecutara otra.\n");
            return;
        }
        motor_in_progress = 1;
        function = motor_function;
    } else {
        printf("No es un topico conocido\n");
        return;
    }

    // Crear y llenar estructura de argumentos
    ThreadArgs *args = malloc(sizeof(ThreadArgs));
    if (!args) {
        fprintf(stderr, "Error al asignar memoria para los argumentos del hilo\n");
        return;
    }

    args->mosq = mosq;
    strncpy(args->payload, (char *)message->payload, sizeof(args->payload) - 1);
    args->payload[sizeof(args->payload) - 1] = '\0';  // Asegurar terminacion de cadena

    // Crear el hilo para ejecutar la funcion correspondiente
    if (pthread_create(&thread, NULL, function, args) != 0) {
        fprintf(stderr, "Error al crear el hilo\n");
        free(args); // Liberar memoria en caso de fallo
    } else {
        printf("Hilo creado para el topico %s\n", message->topic);
        pthread_detach(thread); // Permite que el sistema libere el hilo al terminar
    }
}

int main() {
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    mosquitto_lib_init();
    mosq = mosquitto_new(CLIENT_ID, true, NULL);
    if (!mosq) {
        fprintf(stderr, "Error al crear el cliente MQTT\n");
        mosquitto_lib_cleanup();
        return EXIT_FAILURE;
    }

    mosquitto_message_callback_set(mosq, message_callback);

    if (mosquitto_connect(mosq, BROKER_ADDRESS, BROKER_PORT, 60)) {
        fprintf(stderr, "No se pudo conectar al broker MQTT\n");
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        return EXIT_FAILURE;
    }

    // Suscribirse a los cuatro tpicos
    if (mosquitto_subscribe(mosq, NULL, TOPIC_CMDS_ADC, 0) ||
        mosquitto_subscribe(mosq, NULL, TOPIC_CMDS_GPIO_READ, 0) ||
        mosquitto_subscribe(mosq, NULL, TOPIC_CMDS_GPIO_WRITE, 0) ||
        mosquitto_subscribe(mosq, NULL, TOPIC_CMDS_MOTOR, 0)) {
        fprintf(stderr, "Error al suscribirse a uno o mas topicos\n");
        mosquitto_disconnect(mosq);
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        return EXIT_FAILURE;
    }

    printf("Conectado al broker MQTT y suscrito a los topicos:\n");
    printf("  %s\n", TOPIC_CMDS_ADC);
    printf("  %s\n", TOPIC_CMDS_GPIO_READ);
    printf("  %s\n", TOPIC_CMDS_GPIO_WRITE);
    printf("  %s\n", TOPIC_CMDS_MOTOR);

    mosquitto_loop_forever(mosq, -1, 1);

    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    return 0;
}


