#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "shared_memory.h"
#include "log.h"

SharedMemoryContext *shm = NULL;

int map_memory(int *fd, void **map_base, void **virtual_addr, off_t target, size_t size) {
    *fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (*fd == -1) {
        LOG_ERROR("Error /dev/mem");
        return -1;
    }

    *map_base = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, *fd, target);
    if (*map_base == MAP_FAILED) {
        LOG_ERROR("Error mmap");
        close(*fd);
        return -1;
    }

    *virtual_addr = *map_base;
    return 0;
}

void unmap_memory(int fd, void *map_base, size_t size) {
    munmap(map_base, size);
    close(fd);
}
