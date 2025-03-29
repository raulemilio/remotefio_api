// gpio_read
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>
#include <time.h>

#define MAP_SIZE 16384UL  // 16KB para cubrir 4096 valores de 4 bytes
#define DATA_SIZE sizeof(uint32_t)  // Tamanio de cada dato (4 bytes)

int main(int argc, char **argv) {
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

    // Borrar shared[1]
    shared[1] = 0;
    __sync_synchronize(); // Asegurar que la escritura se propague

    // Poner en 1 el bit 0 de shared[0]
    shared[0] |= 1;
    __sync_synchronize();

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
    }

    // Desmontar la memoria
    if (munmap(map_base, MAP_SIZE) == -1) {
        perror("Failed to unmap memory");
    }

    // Cerrar el descriptor de archivo
    close(fd);
    return 0;
}
// gpio_write
// motor

/*
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>
#include <time.h>

#define MAP_SIZE 4096UL  // 4KB de mapeo
#define TARGET_ADDRESS 0x4A322000 // Direccion de memoria a escribir

int main() {
    int fd;
    void *map_base;
    uint32_t *mem;
    off_t target = TARGET_ADDRESS;

    // Abrir /dev/mem
    if ((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) {
        perror("Failed to open /dev/mem");
        return -1;
    }

    // Mapear la memoria en espacio de usuario
    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target & ~0xFFF);
    if (map_base == (void *) -1) {
        perror("Failed to map memory");
        close(fd);
        return -1;
    }

    mem = (uint32_t *)((char *)map_base + (target & 0xFFF));

    // Establecer siempre los bits 0 y 1 en 1
    uint32_t value = (*mem & ~(1 << 2)) | 0x03;

    while (1) {
        // Alternar el bit 2
        value ^= (1 << 1);
        *mem = value;
        __sync_synchronize();
        printf("Escrito 0x%X en 0x%X\n", value, TARGET_ADDRESS);
        sleep(10);
    }

    // Desmontar la memoria
    if (munmap(map_base, MAP_SIZE) == -1) {
        perror("Failed to unmap memory");
    }

    close(fd);
    return 0;
}
*/
/*
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>
#include <time.h>

#define MAP_SIZE 4096UL  // 4KB de mapeo
#define TARGET_ADDRESS 0x4A322000 // Dirección de memoria a escribir

int main() {
    int fd;
    void *map_base;
    uint32_t *mem;
    off_t target = TARGET_ADDRESS;

    // Abrir /dev/mem
    if ((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) {
        perror("Failed to open /dev/mem");
        return -1;
    }

    // Mapear la memoria en espacio de usuario
    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target & ~0xFFF);
    if (map_base == (void *) -1) {
        perror("Failed to map memory");
        close(fd);
        return -1;
    }

    mem = (uint32_t *)((char *)map_base + (target & 0xFFF));

    // Establecer siempre los bits 0 y 1 en 1
    uint32_t value = (*mem & ~(1 << 2)) | 0x03;

    while (1) {
        // Alternar el bit 2
        value ^= (1 << 1);
        *mem = value;
        __sync_synchronize();

        // Leer y mostrar el estado del bit 15
        uint32_t current_value = *mem;
        uint32_t bit_15 = (current_value >> 15) & 1;  // Obtener el bit 15
        printf("Escrito 0x%X en 0x%X, Bit 15: %u\n", value, TARGET_ADDRESS, bit_15);

        sleep(10);
    }

    // Desmontar la memoria
    if (munmap(map_base, MAP_SIZE) == -1) {
        perror("Failed to unmap memory");
    }

    close(fd);
    return 0;
}
*/
