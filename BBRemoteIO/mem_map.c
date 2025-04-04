#include "mem_map.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

// Funcion generica para mapear memoria (compatible con 16 y 32 bits)
int map_memory(int *fd, void **map_base, void **mem_ptr, off_t target, size_t map_size) {
    *fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (*fd == -1) {
        perror("Failed to open /dev/mem");
        return -1;
    }

    *map_base = mmap(0, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, *fd, target & ~0xFFF);
    if (*map_base == MAP_FAILED) {
        perror("Failed to map memory");
        close(*fd);
        return -1;
    }

    *mem_ptr = *map_base;  // Se asigna la direccion base del mapa
    return 0;
}

// Funcion para desmapear memoria
void unmap_memory(int fd, void *map_base, size_t map_size) {
    if (map_base && munmap(map_base, map_size) == -1) {
        perror("Failed to unmap memory");
    }
    if (fd != -1) {
        close(fd);
    }
}
