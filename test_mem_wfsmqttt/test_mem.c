// Este codigo basico envia cada dato uno por vez lo que da un tiempo de envio de 7 min
/*
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include <MQTTClient.h>

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)
#define NUM_SAMPLES 4092

// Configuración de MQTT
#define ADDRESS     "tcp://192.168.0.171:1883"
#define CLIENTID    "BeagleBoneClient"
#define TOPIC       "pru/data"
#define QOS         1
#define TIMEOUT     1000L

static int running = 1;

// Manejador de señal para salir con Ctrl+C
static void __signal_handler(int dummy) {
    running = 0;
}

int main() {
    int fd;
    void *map_base, *virt_addr;
    unsigned long read_result;
    off_t target = 0x4a300000; // PRU0 -> RAM0 (shared memory)

    struct timespec start_total, end_total, start_process, end_process;

    // Iniciar medición de tiempo total
    clock_gettime(CLOCK_MONOTONIC, &start_total);

    // Configurar la señal para Ctrl+C
    signal(SIGINT, __signal_handler);

    // Abrir la memoria compartida
    if ((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) {
        perror("Error al abrir /dev/mem");
        return -1;
    }

    // Mapear la memoria compartida
    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target & ~MAP_MASK);
    if (map_base == (void *) -1) {
        perror("Error al mapear la memoria");
        close(fd);
        return -1;
    }

    // Inicializar cliente MQTT
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    int rc;

    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Error conectando al broker MQTT: %d\n", rc);
        munmap(map_base, MAP_SIZE);
        close(fd);
        return rc;
    }
    printf("Conectado al broker MQTT en %s\n", ADDRESS);

    // Llenar la memoria con valores conocidos
    printf("Inicializando memoria con valores conocidos...\n");
    for (int i = 0; i < NUM_SAMPLES; i++) {
        virt_addr = map_base + (target & MAP_MASK);
        *((unsigned long *) virt_addr) = 0xAAAA0000 + i; // Patrón de prueba
        target += sizeof(unsigned long);
    }

    // Restaurar la dirección base
    target = 0x4a300000;

    // Iniciar medición del tiempo de lectura y envío
    clock_gettime(CLOCK_MONOTONIC, &start_process);

    // Crear buffer JSON
    char payload[NUM_SAMPLES * 12 + 50]; // Suficiente espacio para almacenar el JSON
    char temp[12];
    strcpy(payload, "{dev:\"1\", array:[");

    // Leer y almacenar datos en el JSON
    for (int i = 0; i < NUM_SAMPLES; i++) {
        virt_addr = map_base + (target & MAP_MASK);
        read_result = *((unsigned long *) virt_addr);

        // Convertir dato a string en hexadecimal
        snprintf(temp, sizeof(temp), "0x%08lX", read_result);
        strcat(payload, temp);

        // Agregar coma si no es el último elemento
        if (i < NUM_SAMPLES - 1) {
            strcat(payload, ",");
        }

        target += sizeof(unsigned long);
    }

    strcat(payload, "]}");

    // Enviar el mensaje JSON por MQTT
    pubmsg.payload = payload;
    pubmsg.payloadlen = strlen(payload);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;

    MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token);
    MQTTClient_waitForCompletion(client, token, TIMEOUT);

    // Medir el tiempo entre lectura y envío
    clock_gettime(CLOCK_MONOTONIC, &end_process);

    // Medir el tiempo total de ejecución
    clock_gettime(CLOCK_MONOTONIC, &end_total);

    // Calcular los tiempos en milisegundos
    double total_time = (end_total.tv_sec - start_total.tv_sec) * 1000.0 +
                        (end_total.tv_nsec - start_total.tv_nsec) / 1000000.0;

    double process_time = (end_process.tv_sec - start_process.tv_sec) * 1000.0 +
                          (end_process.tv_nsec - start_process.tv_nsec) / 1000000.0;

    printf("Tiempo total de ejecución: %.3f ms\n", total_time);
    printf("Tiempo de lectura y envío: %.3f ms\n", process_time);

    // Desconectar y limpiar MQTT
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);

    // Liberar recursos
    munmap(map_base, MAP_SIZE);
    close(fd);

    return 0;
}
*/
// Primer optimizacion: se envian todos los datos de una sola vez
// tiempo total de ejecucion 210 ms
// tiempo de guardado en archivo y envio: 100 ms
/*
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include <MQTTClient.h>

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)
#define NUM_SAMPLES 4092

// Configuración de MQTT
#define ADDRESS     "tcp://192.168.0.171:1883"
#define CLIENTID    "BeagleBoneClient"
#define TOPIC       "pru/data"
#define QOS         1
#define TIMEOUT     1000L

static int running = 1;

// Manejador de señal para salir con Ctrl+C
static void __signal_handler(int dummy) {
    running = 0;
}

int main() {
    int fd;
    void *map_base, *virt_addr;
    unsigned long read_result;
    off_t target = 0x4a300000; // PRU0 -> RAM0 (shared memory)

    struct timespec start_total, end_total, start_process, end_process;

    // Iniciar medición de tiempo total
    clock_gettime(CLOCK_MONOTONIC, &start_total);

    // Configurar la señal para Ctrl+C
    signal(SIGINT, __signal_handler);

    // Abrir la memoria compartida
    if ((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) {
        perror("Error al abrir /dev/mem");
        return -1;
    }

    // Mapear la memoria compartida
    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target & ~MAP_MASK);
    if (map_base == (void *) -1) {
        perror("Error al mapear la memoria");
        close(fd);
        return -1;
    }

    // Inicializar cliente MQTT
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    int rc;

    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Error conectando al broker MQTT: %d\n", rc);
        munmap(map_base, MAP_SIZE);
        close(fd);
        return rc;
    }
    printf("Conectado al broker MQTT en %s\n", ADDRESS);

    // Llenar la memoria con valores conocidos
    printf("Inicializando memoria con valores conocidos...\n");
    for (int i = 0; i < NUM_SAMPLES; i++) {
        virt_addr = map_base + (target & MAP_MASK);
        *((unsigned long *) virt_addr) = 0xAAAA0000 + i; // Patrón de prueba
        target += sizeof(unsigned long);
    }

    // Restaurar la dirección base
    target = 0x4a300000;

    // Iniciar medición del tiempo de lectura y envío
    clock_gettime(CLOCK_MONOTONIC, &start_process);

    // Crear buffer JSON (sin uso de strcat, lo llenamos en bloque)
    char payload[NUM_SAMPLES * 12 + 50]; // Suficiente espacio para almacenar el JSON
    char *ptr = payload;
    ptr += sprintf(ptr, "{dev:\"1\", array:[");

    // Leer y almacenar datos en el JSON
    for (int i = 0; i < NUM_SAMPLES; i++) {
        virt_addr = map_base + (target & MAP_MASK);
        read_result = *((unsigned long *) virt_addr);

        // Convertir dato a string en hexadecimal y agregarlo al JSON
        ptr += sprintf(ptr, "0x%08lX", read_result);

        // Agregar coma si no es el último elemento
        if (i < NUM_SAMPLES - 1) {
            *ptr++ = ',';
        }

        target += sizeof(unsigned long);
    }

    // Finalizar el JSON
    strcpy(ptr, "]}");

    // Enviar el mensaje JSON por MQTT
    pubmsg.payload = payload;
    pubmsg.payloadlen = strlen(payload);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;

    MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token);
    MQTTClient_waitForCompletion(client, token, TIMEOUT);

    // Medir el tiempo entre lectura y envío
    clock_gettime(CLOCK_MONOTONIC, &end_process);

    // Medir el tiempo total de ejecución
    clock_gettime(CLOCK_MONOTONIC, &end_total);

    // Calcular los tiempos en milisegundos
    double total_time = (end_total.tv_sec - start_total.tv_sec) * 1000.0 +
                        (end_total.tv_nsec - start_total.tv_nsec) / 1000000.0;

    double process_time = (end_process.tv_sec - start_process.tv_sec) * 1000.0 +
                          (end_process.tv_nsec - start_process.tv_nsec) / 1000000.0;

    printf("Tiempo total de ejecución: %.3f ms\n", total_time);
    printf("Tiempo de lectura y envío: %.3f ms\n", process_time);

    // Desconectar y limpiar MQTT
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);

    // Liberar recursos
    munmap(map_base, MAP_SIZE);
    close(fd);

    return 0;
}
*/
/*
// Tercera optimizacion no se guardan los datos en archivos
// tiempo total 215 ms
// tiempo de envio 100 ms
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include <MQTTClient.h>

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)
#define NUM_SAMPLES 4092

// Configuración de MQTT
#define ADDRESS     "tcp://192.168.0.171:1883"
#define CLIENTID    "BeagleBoneClient"
#define TOPIC       "pru/data"
#define QOS         1
#define TIMEOUT     1000L

static int running = 1;

// Manejador de señal para salir con Ctrl+C
static void __signal_handler(int dummy) {
    running = 0;
}

int main() {
    int fd;
    void *map_base, *virt_addr;
    unsigned long read_result;
    off_t target = 0x4a300000; // PRU0 -> RAM0 (shared memory)

    struct timespec start_total, end_total, start_process, end_process;

    // Iniciar medición de tiempo total
    clock_gettime(CLOCK_MONOTONIC, &start_total);

    // Configurar la señal para Ctrl+C
    signal(SIGINT, __signal_handler);

    // Abrir la memoria compartida
    if ((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) {
        perror("Error al abrir /dev/mem");
        return -1;
    }

    // Mapear la memoria compartida
    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target & ~MAP_MASK);
    if (map_base == (void *) -1) {
        perror("Error al mapear la memoria");
        close(fd);
        return -1;
    }

    // Inicializar cliente MQTT
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    int rc;

    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Error conectando al broker MQTT: %d\n", rc);
        munmap(map_base, MAP_SIZE);
        close(fd);
        return rc;
    }
    printf("Conectado al broker MQTT en %s\n", ADDRESS);

    // Llenar la memoria con valores conocidos
    printf("Inicializando memoria con valores conocidos...\n");
    for (int i = 0; i < NUM_SAMPLES; i++) {
        virt_addr = map_base + (target & MAP_MASK);
        *((unsigned long *) virt_addr) = 0xAAAA0000 + i; // Patrón de prueba
        target += sizeof(unsigned long);
    }

    // Restaurar la dirección base
    target = 0x4a300000;

    // Iniciar medición del tiempo de lectura y envío
    clock_gettime(CLOCK_MONOTONIC, &start_process);

    // Crear buffer JSON (sin uso de strcat, lo llenamos en bloque)
    char payload[NUM_SAMPLES * 12 + 50]; // Suficiente espacio para almacenar el JSON
    char *ptr = payload;
    ptr += sprintf(ptr, "{dev:\"1\", array:[");

    // Leer y almacenar datos en el JSON
    for (int i = 0; i < NUM_SAMPLES; i++) {
        virt_addr = map_base + (target & MAP_MASK);
        read_result = *((unsigned long *) virt_addr);

        // Convertir dato a string en hexadecimal y agregarlo al JSON
        ptr += sprintf(ptr, "0x%08lX", read_result);

        // Agregar coma si no es el último elemento
        if (i < NUM_SAMPLES - 1) {
            *ptr++ = ',';
        }

        target += sizeof(unsigned long);
    }

    // Finalizar el JSON
    strcpy(ptr, "]}");

    // Enviar el mensaje JSON por MQTT
    pubmsg.payload = payload;
    pubmsg.payloadlen = strlen(payload);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;

    MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token);
    MQTTClient_waitForCompletion(client, token, TIMEOUT);

    // Medir el tiempo entre lectura y envío
    clock_gettime(CLOCK_MONOTONIC, &end_process);

    // Medir el tiempo total de ejecución
    clock_gettime(CLOCK_MONOTONIC, &end_total);

    // Calcular los tiempos en milisegundos
    double total_time = (end_total.tv_sec - start_total.tv_sec) * 1000.0 +
                        (end_total.tv_nsec - start_total.tv_nsec) / 1000000.0;

    double process_time = (end_process.tv_sec - start_process.tv_sec) * 1000.0 +
                          (end_process.tv_nsec - start_process.tv_nsec) / 1000000.0;

    printf("Tiempo total de ejecución: %.3f ms\n", total_time);
    printf("Tiempo de lectura y envío: %.3f ms\n", process_time);

    // Desconectar y limpiar MQTT
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);

    // Liberar recursos
    munmap(map_base, MAP_SIZE);
    close(fd);

    return 0;
}
*/

// esta modificacion envia en formato decimal y 2bytes 
// tiempo total de ejecucion 220 ms
// tiempo de envio 100 ms

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include <MQTTClient.h>

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)
#define NUM_SAMPLES 4092

// Configuracion de MQTT
#define ADDRESS     "tcp://192.168.0.171:1883"
#define CLIENTID    "BeagleBoneClient"
#define TOPIC       "pru/data"
#define QOS         1
#define TIMEOUT     1000L

static int running = 1;

// Manejador de señal para salir con Ctrl+C
static void __signal_handler(int dummy) {
    running = 0;
}

int main() {
    int fd;
    void *map_base, *virt_addr;
    unsigned short read_result;
    off_t target = 0x4a300000; // PRU0 -> RAM0 (shared memory)

    struct timespec start_total, end_total, start_process, end_process;

    // Iniciar medicion de tiempo total
    clock_gettime(CLOCK_MONOTONIC, &start_total);

    // Configurar la señal para Ctrl+C
    signal(SIGINT, __signal_handler);

    // Abrir la memoria compartida
    if ((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) {
        perror("Error al abrir /dev/mem");
        return -1;
    }

    // Mapear la memoria compartida
    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target & ~MAP_MASK);
    if (map_base == (void *) -1) {
        perror("Error al mapear la memoria");
        close(fd);
        return -1;
    }

    // Inicializar cliente MQTT
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    int rc;

    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Error conectando al broker MQTT: %d\n", rc);
        munmap(map_base, MAP_SIZE);
        close(fd);
        return rc;
    }
    printf("Conectado al broker MQTT en %s\n", ADDRESS);

    // Llenar la memoria con valores conocidos
    printf("Inicializando memoria con valores conocidos...\n");
    for (int i = 0; i < NUM_SAMPLES; i++) {
        virt_addr = map_base + (target & MAP_MASK);
        *((unsigned short *) virt_addr) = (unsigned short)(0xAAAA + i); // Patron de prueba
        target += sizeof(unsigned short);
    }

    // Restaurar la direccion base
    target = 0x4a300000;

    // Iniciar medicion del tiempo de lectura y envio
    clock_gettime(CLOCK_MONOTONIC, &start_process);

    // Crear buffer JSON (sin uso de strcat, lo llenamos en bloque)
    char payload[NUM_SAMPLES * 6 + 50]; // Suficiente espacio para almacenar el JSON
    char *ptr = payload;
    ptr += sprintf(ptr, "{dev:\"1\", array:[");

    // Leer y almacenar datos en el JSON
    for (int i = 0; i < NUM_SAMPLES; i++) {
        virt_addr = map_base + (target & MAP_MASK);
        read_result = *((unsigned short *) virt_addr);

        // Convertir dato a string en decimal y agregarlo al JSON
        ptr += sprintf(ptr, "%u", read_result);

        // Agregar coma si no es el ultimo elemento
        if (i < NUM_SAMPLES - 1) {
            *ptr++ = ',';
        }

        target += sizeof(unsigned short);
    }

    // Finalizar el JSON
    strcpy(ptr, "]}");

    // Enviar el mensaje JSON por MQTT
    pubmsg.payload = payload;
    pubmsg.payloadlen = strlen(payload);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;

    MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token);
    MQTTClient_waitForCompletion(client, token, TIMEOUT);

    // Medir el tiempo entre lectura y envio
    clock_gettime(CLOCK_MONOTONIC, &end_process);

    // Medir el tiempo total de ejecucion
    clock_gettime(CLOCK_MONOTONIC, &end_total);

    // Calcular los tiempos en milisegundos
    double total_time = (end_total.tv_sec - start_total.tv_sec) * 1000.0 +
                         (end_total.tv_nsec - start_total.tv_nsec) / 1000000.0;

    double process_time = (end_process.tv_sec - start_process.tv_sec) * 1000.0 +
                           (end_process.tv_nsec - start_process.tv_nsec) / 1000000.0;

    printf("Tiempo total de ejecucion: %.3f ms\n", total_time);
    printf("Tiempo de lectura y envio: %.3f ms\n", process_time);

    // Desconectar y limpiar MQTT
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);

    // Liberar recursos
    munmap(map_base, MAP_SIZE);
    close(fd);

    return 0;
}
