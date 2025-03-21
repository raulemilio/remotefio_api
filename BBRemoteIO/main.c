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

#include "pru_control.h"
#include "message_validator.h"// funciones auxiliares para procesar el payload json

#define PRU_PATH "/sys/class/remoteproc/remoteproc1"
#define PRU_FIRMWARE "am335x-pru1-rc-servo-fw"

#define MAP_SIZE 16384UL  // 16KB para cubrir 4096 valores de 4 bytes
#define DATA_SIZE sizeof(uint32_t)  // Tamanio de cada dato (4 bytes)

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

#define MESSAGE_CALLBACK_JSON_OK "Mensaje recibido ok"
#define MESSAGE_CALLBACK_JSON_ERROR " Hubo un error al recibir el mensaje"
#define TOPIC_LOGS "BBRemote/logs"

typedef struct {
    struct mosquitto *mosq;
    AdcData adc_data;
    GpioReadData gpio_read_data;
    GpioWriteData gpio_write_data;
    MotorData motor_data;
} ThreadArgs;

// gpio-read
int fd;
void *map_base;
uint32_t *shared;
off_t target = 0x4A310000; // Direccion base de la memoria
time_t start_time;
char json_payload[128];
pthread_mutex_t gpio_mutex = PTHREAD_MUTEX_INITIALIZER;
// gpio-read

GpioReadData gpio_data_send;
GpioWriteData gpio_write_data_send;
MotorData motor_data_send;

struct mosquitto *mosq = NULL;

// Flags para indicar si una tarea en ejecucion
int adc_in_progress = 0;
int gpio_read_in_progress = 0;
int gpio_write_in_progress = 0;
int motor_in_progress = 0;

void control_pru(int start) {
    char command[128];
    
    if (start) {
        // Cargar el firmware
        snprintf(command, sizeof(command), "echo '%s' > %s/firmware", PRU_FIRMWARE, PRU_PATH);
        system(command);
        
        // Iniciar la PRU
        snprintf(command, sizeof(command), "echo 'start' > %s/state", PRU_PATH);
        system(command);
        
        printf("PRU encendida.\n");
    } else {
        // Detener la PRU
        snprintf(command, sizeof(command), "echo 'stop' > %s/state", PRU_PATH);
        system(command);
        
        printf("PRU apagada.\n");
    }
}
//----------------------------------------------------------------------------------------------------------------------------------------
// Manejo de seniales para salir limpiamente
void handle_signal(int signo) {
    printf("Cerrando cliente MQTT...\n");
    if (mosq) {
        mosquitto_disconnect(mosq);
        mosquitto_destroy(mosq);
    }
    mosquitto_lib_cleanup();
    printf("Cerrando PRU...\n");
    exit(0);
}
//------------------------------------------------------------------------------------------------
void *adc_function(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;  // Casteo correcto del argumento
    struct mosquitto *mosq = args->mosq;

    printf("sample desde adc_function: %d\n", args->adc_data.samples);

    // Simulacion de la lectura ADC
    sleep(6);

    // Respuesta al topico
    const char *response = "Comando ADC recibido";
    mosquitto_publish(mosq,NULL, TOPIC_RSP_ADC, strlen(response), response, 0, false);
    printf("Enviando respuesta ADC: %s\n", response);

    // Liberar recursos
    adc_in_progress = 0;  // Marcar que el ADC ya no esta en progreso
    free(args);  // Importante: liberar la memoria del argumento

    pthread_exit(NULL);
}

void *gpio_read_function(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;  // Casteo correcto del argumento
    struct mosquitto *mosq = args->mosq;
    printf("num_pins desde gpio_read_function: %d\n", args->gpio_read_data.num_pins);

    int mode= args->gpio_read_data.mode;

    // Abrir /dev/mem
    if ((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) {
        perror("Failed to open /dev/mem");
        return -1;
    }

    // Mapeo de la memoria en espacio de usuario
    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target & ~0xFFF);
    if (map_base == (void *) -1) {
        perror("Failed to map memory");
        close(fd);
        return -1;
    }
    shared = (uint32_t *)map_base;

    switch (mode) {
        case 0:
            printf("gpio_read mode 0\n");
            // Borrar shared[1]
            shared[1] = 0;
            __sync_synchronize(); // Asegurar que la escritura se propague
            pthread_mutex_lock(&gpio_mutex);  // Bloquear acceso a shared[0]
            // Poner en 1 el bit 0 de shared[0]
            shared[0] |= 1 << 0;
            __sync_synchronize();
            pthread_mutex_unlock(&gpio_mutex); // Desbloquear tras escritura en shared[0]

            // Esperar el bit 12 de shared[1] hasta 10 segundos
            start_time = time(NULL);
            while (!(shared[1] & (1 << 12))) {
        	if (time(NULL) - start_time > 10) {
            	printf("Tiempo de espera agotado. Terminando programa.\n");
           	break;
        	}
        	usleep(1000); // Pequenia espera para evitar alto consumo de CPU
    	     }
    		// Si la condicion se cumplio, mostrar los primeros 4 bits de shared[1]
    		if (shared[1] & (1 << 12)) {
        	printf("Primeros 4 bits de shared[1]: %x\n", shared[1] & 0xF);
           		for (int i = 0; i < args->gpio_read_data.num_pins; i++) {
               		int pin = args->gpio_read_data.pins[i]; // Obtener el numero de pin solicitado
               		gpio_data_send.state[i] = (shared[1] >> pin) & 1; // Extraer el bit correspondiente
			gpio_data_send.pins[i] = pin;
           		}
    		}
            break;
        case 1:
            printf("gpio_read mode 1\n");
            break;
        case 2:
            printf("gpio_read mode 2\n");
            break;
        default:
            printf("Opcion no valida\n");
            break;
    }

    // Desmontar la memoria
    if (munmap(map_base, MAP_SIZE) == -1) {
        perror("Failed to unmap memory");
    }
    // Cerrar el descriptor de archivo
    close(fd);
char json_payload[256]; // Ajustar el tama      o seg      n sea necesario
int offset = snprintf(json_payload, sizeof(json_payload), "{\"task\":\"gpio_read\",\"status\":\"ok\",\"pins\":[");

// Agregar din      micamente los valores de los pines
for (int i = 0; i < args->gpio_read_data.num_pins; i++) {
    if (i > 0) {
        offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, ",");
    }
    offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, "%d", gpio_data_send.pins[i]);
}

// Agregar los estados de los pines
offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, "],\"states\":[");

for (int i = 0; i < args->gpio_read_data.num_pins; i++) {
    if (i > 0) {
        offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, ",");
    }
    offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, "%d", gpio_data_send.state[i]);
}

// Cerrar el JSON
snprintf(json_payload + offset, sizeof(json_payload) - offset, "]}");
    // Publicar el mensaje
    mosquitto_publish(mosq, NULL, TOPIC_RSP_GPIO_READ, strlen(json_payload), json_payload, 0, false);
    printf("Enviando respuesta GPIO read: %s\n", json_payload);

    // Liberar recursos antes de terminar
    gpio_read_in_progress = 0; // Liberar el flag
    pthread_exit(NULL);
}

void *gpio_write_function(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;  // Casteo correcto del argumento
    struct mosquitto *mosq = args->mosq;
    printf("num_pins desde gpio_write_function: %d\n", args->gpio_write_data.num_pins);

   int mode = args->gpio_write_data.mode;

   // Abrir /dev/mem
    if ((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) {
        perror("Failed to open /dev/mem");
        return -1;
    }
    // Mapeo de la memoria en espacio de usuario
    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target & ~0xFFF);
    if (map_base == (void *) -1) {
        perror("Failed to map memory");
        close(fd);
        return -1;
    }
    shared = (uint32_t *)map_base;

    // Borrar shared[3] write set function PRU
    shared[3] = 0;

    switch (mode) {
       case 0:
            pthread_mutex_lock(&gpio_mutex);  // Bloquear acceso a shared[0]
            // Poner en 1 el bit 3 de shared[0]
            shared[0] |= (1 << 3);
            __sync_synchronize();
            pthread_mutex_unlock(&gpio_mutex); // Desbloquear tras escritura en shared[0]

            // Esperar el bit 12 de shared[1] hasta 10 segundos
            start_time = time(NULL);
            while (!(shared[3] & (1 << 12))) {
                if (time(NULL) - start_time > 10) {
                printf("Tiempo de espera agotado. Terminando programa.\n");
                break;
                }
                usleep(1000); // Pequenia espera para evitar alto consumo de CPU
             }
	        for (int i = 0; i < args->gpio_write_data.num_pins; i++) {
                        int pin = args->gpio_write_data.pins[i]; // Obtener el numero de pin solicitado
                        gpio_write_data_send.state[i] = (shared[3] >> (pin+4)) & 1; // Extraer el bit correspondiente
			gpio_write_data_send.pins[i] = pin;
                         }

char json_payload[256]; // Ajustar el tamaño según sea necesario
int offset = snprintf(json_payload, sizeof(json_payload), "{\"task\":\"gpio_write\",\"status\":\"ok\",\"pins\":[");

// Agregar dinámicamente los valores de los pines
for (int i = 0; i < args->gpio_write_data.num_pins; i++) {
    if (i > 0) {
        offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, ",");
    }
    offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, "%d", gpio_write_data_send.pins[i]);
}

// Agregar los estados de los pines
offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, "],\"states\":[");

for (int i = 0; i < args->gpio_write_data.num_pins; i++) {
    if (i > 0) {
        offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, ",");
    }
    offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, "%d", gpio_write_data_send.state[i]);
}

// Cerrar el JSON
snprintf(json_payload + offset, sizeof(json_payload) - offset, "]}");

// Publicar el mensaje
mosquitto_publish(mosq, NULL, TOPIC_RSP_GPIO_WRITE, strlen(json_payload), json_payload, 0, false);
printf("Enviando respuesta GPIO write: %s\n", json_payload);

        break;
       case 1:
            printf("gpio_write -> mode 1 get gpio set state\n");
	   // cargamos los datos en shared[3]
        	for (int i = 0; i < args->gpio_write_data.num_pins; i++) {
                	int pin = args->gpio_write_data.pins[i]; // Obtener el numero de pin
                	int state = args->gpio_write_data.state[i] & 1; // Asegurar que el estado es 0 o 1

                	// Establecer el bit correspondiente en los bits 0-3
                	if (pin >= 0 && pin < 4) {
                	shared[3] |= (1 << pin);
                	}
                	// Establecer el estado en los bits 4-7
                	if (i < 4) {
                	shared[3] |= (state << (4 + pin));
                	}
        	}
    		__sync_synchronize(); // Asegurar que la escritura se propague

    		printf("Primeros 8 bits de shared[3]: %x\n", shared[3] & 0xFF);

    		pthread_mutex_lock(&gpio_mutex);  // Bloquear acceso a shared[0]
    		// Poner en 1 el bit 2 de shared[0]
    		shared[0] |= (1 << 2);
    		__sync_synchronize();
    		pthread_mutex_unlock(&gpio_mutex); // Desbloquear tras escritura en shared[0]

    		// Esperar el bit 12 de shared[3] hasta 10 segundos
    		start_time = time(NULL);
    		while (!(shared[3] & (1 << 12))) {
        		if (time(NULL) - start_time > 10) {
            		printf("Tiempo de espera agotado. Terminando programa.\n");
            		break;
        		}
        		usleep(1000); // Pequenia espera para evitar alto consumo de CPU
    		}

    		printf("Primeros 8 bits de shared[3] despues PRU: %x\n", shared[3] & 0xFF);
    		shared[3] = 0;
            break;
       case 2:
            printf("gpio_write -> mode 2\n");
            break;
       default:
            printf("Opcion no valida\n");
            break;
    }

    // Desmontar la memoria
    if (munmap(map_base, MAP_SIZE) == -1) {
        perror("Failed to unmap memory");
    }
    // Cerrar el descriptor de archivo
    close(fd);

//    mosquitto_publish(mosq, NULL, TOPIC_RSP_GPIO_WRITE, strlen(json_payload), json_payload, 0, false);
//    printf("Enviando respuesta GPIO WRITE: %s\n", json_payload);

    // Liberar recursos antes de terminar
    gpio_write_in_progress = 0; // Liberar el flag
    pthread_exit(NULL);
}

void *motor_function(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;  // Casteo correcto del argumento
    struct mosquitto *mosq = args->mosq;
    printf("num_motor desde motor_function: %d\n", args->motor_data.num);

   // Abrir /dev/mem
    if ((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) {
        perror("Failed to open /dev/mem");
        return -1;
    }
    // Mapeo de la memoria en espacio de usuario
    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target & ~0xFFF);
    if (map_base == (void *) -1) {
        perror("Failed to map memory");
        close(fd);
        return -1;
    }
    shared = (uint32_t *)map_base;

    int mode = args->motor_data.mode;

    switch (mode) {
        case 0:
            printf("motor-> mode 0\n");
            pthread_mutex_lock(&gpio_mutex);  // Bloquear acceso a shared[0]
            // Poner en 1 el bit 3 de shared[0]
            shared[0] |= (1 << 5);
            __sync_synchronize();
            pthread_mutex_unlock(&gpio_mutex); // Desbloquear tras escritura en shared[0]

            // Esperar el bit 12 de shared[1] hasta 10 segundos
            start_time = time(NULL);
            while (!(shared[4] & (1 << 12))) {
                if (time(NULL) - start_time > 10) {
                printf("Tiempo de espera agotado. Terminando programa.\n");
                break;
                }
                usleep(1000); // Pequenia espera para evitar alto consumo de CPU
             }
                for (int i = 0; i < args->motor_data.num; i++) {
                        int motor = args->motor_data.motor[i]; // Obtener el numero de pin solicitado
                        motor_data_send.ena[i] = (shared[4] >> ((2*motor)+4)) & 1; // Extraer el bit correspondiente
                        motor_data_send.motor[i] = motor;
			motor_data_send.dir[i] = (shared[4] >> ((2*motor)+5)) & 1;
			motor_data_send.spd[i] = shared[(5+motor)];
                         }
 char json_payload[256]; // Ajustar el tamaño según sea necesario
    int offset = snprintf(json_payload, sizeof(json_payload), "{\"task\":\"motor_control\",\"status\":\"ok\",\"motor\":[");

    // Agregar dinámicamente los valores de motor
    for (int i = 0; i < args->motor_data.num; i++) {
        if (i > 0) {
            offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, ",");
        }
        offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, "%d", motor_data_send.motor[i]);
    }

    // Agregar los valores de ena
    offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, "],\"ena\":[");
    for (int i = 0; i < args->motor_data.num; i++) {
        if (i > 0) {
            offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, ",");
        }
        offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, "%d", motor_data_send.ena[i]);
    }

    // Agregar los valores de dir
    offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, "],\"dir\":[");
    for (int i = 0; i < args->motor_data.num; i++) {
        if (i > 0) {
            offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, ",");
        }
        offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, "%d", motor_data_send.dir[i]);
    }

    // Agregar los valores de spd
    offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, "],\"spd\":[");
    for (int i = 0; i < args->motor_data.num; i++) {
        if (i > 0) {
            offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, ",");
        }
        offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, "%d", motor_data_send.spd[i]);
    }

    // Cerrar el JSON
    snprintf(json_payload + offset, sizeof(json_payload) - offset, "]}");

// Publicar el mensaje
mosquitto_publish(mosq, NULL, TOPIC_RSP_MOTOR, strlen(json_payload), json_payload, 0, false);
printf("Enviando respuesta GPIO write: %s\n", json_payload);





            break;
        case 1:
            printf("motor-> mode 1\n");
            // Borrar shared[4] write set function PRU
            shared[4] = 0;
            __sync_synchronize(); // Asegurar que la escritura se propague
            // Recorrer cada motor definido en motor_data
            for (int i = 0; i < args->motor_data.num; i++) {
        	int motor_id = args->motor_data.motor[i];
        	if (motor_id < 0 || motor_id > 3) {
            	continue; // Ignorar valores fuera de rango
        	}
        	// Cargar bit correspondiente al motor en shared[4]
        	shared[4] |= (1 << motor_id);
        	// Asegurarse de que solo se usen 0 o 1 para ena y dir
        	shared[4] |= ((args->motor_data.ena[i] & 1) << (4 + (2 * motor_id)));  // Asegura que 'ena' sea 0 o 1
        	shared[4] |= ((args->motor_data.dir[i] & 1) << (5 + (2 * motor_id)));  // Asegura que 'dir' sea 0 o 1
        	// Cargar velocidades en los registros correspondientes de forma dinamica
        	shared[5 + i] = args->motor_data.spd[i];
    		}
    		__sync_synchronize(); // Asegurar que la escritura se propague
    		printf("Primeros 12 bits de shared[4]: %x\n", shared[4] & 0xFFF);


    		pthread_mutex_lock(&gpio_mutex);  // Bloquear acceso a shared[0]
    		// Poner en 1 el bit 4 de shared[0]
    		shared[0] |= (1 << 4);
    		__sync_synchronize();
    		pthread_mutex_unlock(&gpio_mutex); // Desbloquear tras escritura en shared[0]

    		// Esperar el bit 12 de shared[4] hasta 10 segundos
    		start_time = time(NULL);
    		while (!(shared[4] & (1 << 12))) {
        		if (time(NULL) - start_time > 10) {
            		printf("Tiempo de espera agotado. Terminando programa.\n");
            		break;
        		}
        		usleep(1000); // Pequenia espera para evitar alto consumo de CPU
    		}
    		printf("Primeros 12 bits de shared[4] despues PRU: %x\n", shared[4] & 0xFFF);
            break;
        case 2:
            printf("motor-> mode 2\n");
            break;
        default:
            printf("Opcion no valida\n");
            break;
    }

    // Desmontar la memoria
    if (munmap(map_base, MAP_SIZE) == -1) {
        perror("Failed to unmap memory");
    }
    // Cerrar el descriptor de archivo
    close(fd);

    // Liberar recursos antes de terminar
    motor_in_progress = 0; // Liberar el flag
    pthread_exit(NULL);
}

void message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message) {

    // Imprimir el mensaje recibido
    printf("Mensaje recibido en %s: %s\n", message->topic, (char *)message->payload);

    // Verificacion de mensaje nulo o vacio
    if (message->payload == NULL || *((char *)message->payload) == '\0') {
        printf("Error: El mensaje recibido es NULL o vacio\n");
        const char *response = "Error: Message received format";
        mosquitto_publish(mosq, NULL, TOPIC_LOGS, strlen(response), response, 0, false);
        return;
    }
    // reserva de memoria para un posible hilo de tarea
    pthread_t thread;
    void *(*function)(void *);
    // Crear y llenar estructura de argumentos
    ThreadArgs *args = malloc(sizeof(ThreadArgs));
    if (!args) {
        fprintf(stderr, "Error al asignar memoria para los argumentos del hilo\n");
        return;
    }

    // Verificar si ya hay una tarea en ejecuciOn
    if (strcmp(message->topic, TOPIC_CMDS_ADC) == 0) {
        if (adc_in_progress) {
            printf("Ya hay una tarea ADC en progreso. No se ejecutara otra.\n");
            return;
        }
        // Verificamos el mensaje JSON y cargamos los datos en la estructura correspondiente
        AdcData adc_data; // aqui ya se cargan los datos de entrada
        int result = validate_adc_message((char *)message->payload, &adc_data);
           if (result == 0) {
             const char *response = "BBRemote Message received format: OK";
             mosquitto_publish(mosq, NULL, TOPIC_LOGS, strlen(response), response, 0, false);
             // Si el procesamiento es exitoso
             printf("%s\n", MESSAGE_CALLBACK_JSON_OK);
           } else {
             const char *response = "BBRemote Message received format: Error";
             mosquitto_publish(mosq, NULL, TOPIC_LOGS, strlen(response), response, 0, false);
             // Si ocurrio un error al procesar el mensaje JSON
             printf("%s\n",MESSAGE_CALLBACK_JSON_ERROR);
             return;
           }
        // cargamos los datos en la variable que se enviara al hilo
        args->adc_data=adc_data;
        // Activamos el flag
        adc_in_progress = 1;
        // Cargamos en el hilo la funcion correspondente
        function = adc_function;

    } else if (strcmp(message->topic, TOPIC_CMDS_GPIO_READ) == 0) {
        if (gpio_read_in_progress) {
            printf("Ya hay una tarea GPIO Read en progreso. No se ejecutara otra.\n");
            return;
        }
         // Verificamos el mensaje JSON y cargamos los datos en la estructura correspondiente
         GpioReadData gpio_read_data; // aqui ya se cargan los datos de entrada
         int result = validate_gpio_read_message((char *)message->payload, &gpio_read_data);
           if (result == 0) {
             const char *response = "BBRemote Message received format: Ok";
             mosquitto_publish(mosq, NULL, TOPIC_LOGS, strlen(response), response, 0, false);
             // Si el procesamiento es exitoso
             printf("%s\n", MESSAGE_CALLBACK_JSON_OK);
           } else {
             const char *response = "BBRemote Message received format: Error";
             mosquitto_publish(mosq, NULL, TOPIC_LOGS, strlen(response), response, 0, false);
             // Si ocurrio un error al procesar el mensaje JSON
             printf("%s\n", MESSAGE_CALLBACK_JSON_ERROR);
             return;
	   }
        // cargamos los datos en la variable que se enviara al hilo
        args->gpio_read_data=gpio_read_data;
        // Activamos el flag
        gpio_read_in_progress = 1;
        function = gpio_read_function;

    } else if (strcmp(message->topic, TOPIC_CMDS_GPIO_WRITE) == 0) {
        if (gpio_write_in_progress) {
            printf("Ya hay una tarea GPIO Write en progreso. No se ejecutara otra.\n");
            return;
        }
          // Verificamos el mensaje JSON y cargamos los datos en la estructura correspondiente
          GpioWriteData gpio_write_data; // aqui ya se cargan los datos de entrada
          int result = validate_gpio_write_message((char *)message->payload, &gpio_write_data);
           if (result == 0) {
             const char *response = "BBRemote Message received format: Ok";
             mosquitto_publish(mosq, NULL, TOPIC_LOGS, strlen(response), response, 0, false);
             // Si el procesamiento es exitoso
             printf("%s\n", MESSAGE_CALLBACK_JSON_OK);
           } else {
             const char *response = "BBRemote Message received format: Error";
             mosquitto_publish(mosq, NULL, TOPIC_LOGS, strlen(response), response, 0, false);
             // Si ocurrio un error al procesar el mensaje JSON
             printf("%s\n",MESSAGE_CALLBACK_JSON_ERROR);
             return;
           }
        // cargamos los datos en la variable que se enviara al hilo
        args->gpio_write_data=gpio_write_data;
        // Activamos el flag
        gpio_write_in_progress = 1;
        function = gpio_write_function;
    } else if (strcmp(message->topic, TOPIC_CMDS_MOTOR) == 0) {
        if (motor_in_progress) {
            printf("Ya hay una tarea Motor en progreso. No se ejecutara otra.\n");
            return;
        }
        // Verificamos el mensaje JSON y cargamos los datos en la estructura correspondiente
        MotorData motor_data; // aqui ya se cargan los datos de entrada
        int result = validate_motor_message((char *)message->payload, &motor_data);
           if (result == 0) {
             const char *response = "BBRemote Message received format: Ok";
             mosquitto_publish(mosq, NULL, TOPIC_LOGS, strlen(response), response, 0, false);
             // Si el procesamiento es exitoso
             printf("%s\n", MESSAGE_CALLBACK_JSON_OK);
           } else {
             // Si ocurrio un error al procesar el mensaje JSON
             const char *response = "BBRemote Message received format: Error";
             mosquitto_publish(mosq, NULL, TOPIC_LOGS, strlen(response), response, 0, false);
             printf("%s\n",MESSAGE_CALLBACK_JSON_ERROR);
             return;
           }
        // cargamos los datos en la variable que se enviara al hilo
        args->motor_data=motor_data;

        motor_in_progress = 1;
        function = motor_function;
    } else {
        printf("No es un topico conocido\n");
        return;
    }

    args->mosq = mosq;

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

    //load_firmware(firmware_name,0);
    control_pru(1); // Encender PRU

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


