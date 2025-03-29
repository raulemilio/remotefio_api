#include "mem_map.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

int map_memory(int *fd, void **map_base, uint32_t **shared, off_t target) {
    *fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (*fd == -1) {
        perror("Failed to open /dev/mem");
        return -1;
    }

    *map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, *fd, target & ~0xFFF);
    if (*map_base == MAP_FAILED) {
        perror("Failed to map memory");
        close(*fd);
        return -1;
    }

    *shared = (uint32_t *)*map_base;
    return 0;
}

void unmap_memory(int fd, void *map_base) {
    if (map_base && munmap(map_base, MAP_SIZE) == -1) {
        perror("Failed to unmap memory");
    }
    if (fd != -1) {
        close(fd);
    }
}
