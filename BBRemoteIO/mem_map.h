#ifndef MEM_MAP_H
#define MEM_MAP_H

#include <stdint.h>
#include <sys/types.h>

// Funcion generica para mapear memoria (compatible con 16 y 32 bits)
int map_memory(int *fd, void **map_base, void **mem_ptr, off_t target, size_t map_size);

// Funcion para desmapear memoria
void unmap_memory(int fd, void *map_base, size_t map_size);

#endif // MEM_MAP_H
