#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <stdint.h>
#include <pthread.h>
#include <sys/types.h>
#include <stddef.h>

// Mutex index assignment for shared memory regions
#define MUTEX_GPIO_INPUT       0
#define MUTEX_GPIO_OUTPUT_GET  1
#define MUTEX_GPIO_OUTPUT_SET  2
#define MUTEX_MOTOR_GET        3
#define MUTEX_MOTOR_SET        4
#define MUTEX_ADC              5
#define SHARED_MUTEX_COUNT     6

typedef struct {
    volatile uint16_t *ram0;
    volatile uint32_t *shared;
    pthread_mutex_t ram0_mutex;
    pthread_mutex_t shared_mutex[SHARED_MUTEX_COUNT];
} SharedMemoryContext;

extern SharedMemoryContext *shm;  //

int map_memory(int *fd, void **map_base, void **virtual_addr, off_t target, size_t size);
void unmap_memory(int fd, void *map_base, size_t size);

#endif
