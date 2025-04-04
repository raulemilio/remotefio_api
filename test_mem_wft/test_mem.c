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

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)
#define NUM_SAMPLES 4092

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

    // Configurar la senial Ctrl+C
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

    // Llenar la memoria con valores conocidos
    printf("Inicializando memoria con valores conocidos...\n");
    for (int i = 0; i < NUM_SAMPLES; i++) {
        virt_addr = map_base + (target & MAP_MASK);
        *((unsigned long *) virt_addr) = 0xAAAA0000 + i; // en decimal 2863270912 +1
        target += sizeof(unsigned long);
    }

    // Restaurar la dirección base
    target = 0x4a300000;

    // Abrir archivo para guardar los datos
    FILE *file = fopen("data.bin", "wb");
    if (!file) {
        perror("Error al abrir el archivo");
        munmap(map_base, MAP_SIZE);
        close(fd);
        return -1;
    }

    // Obtener el tiempo de inicio
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Leer y almacenar 4092 muestras
    printf("Leyendo y guardando datos...\n");
    for (int i = 0; i < NUM_SAMPLES; i++) {
        virt_addr = map_base + (target & MAP_MASK);
        read_result = *((unsigned long *) virt_addr);
        //fwrite(&read_result, sizeof(unsigned long), 1, file);
	fprintf(file, "%lu\n", read_result);
        target += sizeof(unsigned long);
    }

    // Obtener el tiempo de finalización
    clock_gettime(CLOCK_MONOTONIC, &end);

    // Calcular el tiempo transcurrido en milisegundos
    double elapsed_time = (end.tv_sec - start.tv_sec) * 1000.0 +
                          (end.tv_nsec - start.tv_nsec) / 1000000.0;

    printf("Operacion completada en %.3f ms\n", elapsed_time);

    // Liberar recursos
    fclose(file);
    munmap(map_base, MAP_SIZE);
    close(fd);

    return 0;
}
