#ifndef MEM_MAP_H
#define MEM_MAP_H

#include <stdint.h>
#include <sys/types.h>

#define MAP_SIZE 4096

int map_memory(int *fd, void **map_base, uint32_t **shared, off_t target);
void unmap_memory(int fd, void *map_base);

#endif // MEM_MAP_H
