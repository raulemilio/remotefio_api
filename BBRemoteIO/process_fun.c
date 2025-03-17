#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include "process_fun.h"  // Incluir el encabezado

#define MAP_SIZE 16384UL  // 16KB para cubrir 4096 valores de 4 bytes
#define DATA_SIZE sizeof(uint32_t)  // Tamanio de cada dato (4 bytes)

pthread_mutex_t gpio_mutex = PTHREAD_MUTEX_INITIALIZER;

// Implementacion de la funcion read_gpio_data
int read_gpio_data(ThreadArgs *thread_args, GpioReadData *gpio_data_send) {
    if (thread_args == NULL || gpio_data_send == NULL) {
        fprintf(stderr, "Error: Estructura ThreadArgs o GpioReadData es NULL\n");
        return -1;
    }

    GpioReadData *gpio_data = &thread_args->gpio_read_data;
    int fd;
    void *map_base;
    uint32_t *shared;
    off_t target = 0x4A310000; // Direccion base de la memoria
    time_t start_time;

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

    pthread_mutex_lock(&gpio_mutex);  // Bloquear acceso a shared[0]

    if (gpio_data->mode == 0) {
        // Reset shared[1]
        shared[1] = 0;
        __sync_synchronize();

        // Activar bit 0 de shared[0]
        shared[0] |= 1;
        __sync_synchronize();

        pthread_mutex_unlock(&gpio_mutex); // Desbloquear tras escritura en shared[0]

        // Esperar el bit 12 de shared[1] hasta 10 segundos
        start_time = time(NULL);
        while (!(shared[1] & (1 << 12))) {
            if (time(NULL) - start_time > 10) {
                printf("Tiempo de espera agotado. Terminando funcion.\n");
                munmap(map_base, MAP_SIZE);
                close(fd);
                return -1;
            }
            usleep(1000);
        }

        // Cargar los valores leidos en la estructura de salida
        if (shared[1] & (1 << 12)) {
            for (int i = 0; i < gpio_data->num_pins; i++) {
                gpio_data_send->pins[i] = (shared[1] >> i) & 1;  // Se llena la estructura de salida
            }
        }
    } else if (gpio_data->mode == 1) {
        printf("Modo 1: Funcion no implementada aun.\n");
    } else if (gpio_data->mode == 2) {
        printf("Modo 2: Funcion no implementada aun.\n");
    }

    // Desmontar la memoria y cerrar el descriptor de archivo
    munmap(map_base, MAP_SIZE);
    close(fd);

    return 0;
}
