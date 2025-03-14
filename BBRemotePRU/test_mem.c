// read shared[0]
/*
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>

#define MAP_SIZE 16384UL  // 16KB para cubrir 4096 valores de 4 bytes
#define DATA_SIZE sizeof(uint32_t)  // Tamaño de cada dato (4 bytes)

int main(int argc, char **argv) {
    int fd;
    void *map_base, *virt_addr;
    uint32_t read_result;
    off_t target = 0x4A310000; // Dirección base de la memoria

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

    // Leer solo la primera posición (shared[0])
    virt_addr = map_base;  // Primera posición
    read_result = *((uint32_t *) virt_addr);
    printf("Valor en shared[0]: %d\n", read_result);

    // Desmontar la memoria
    if (munmap(map_base, MAP_SIZE) == -1) {
        perror("Failed to unmap memory");
    }

    // Cerrar el descriptor de archivo
    close(fd);
    return 0;
}
*/
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
