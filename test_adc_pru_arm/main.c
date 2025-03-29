#include <signal.h>
#include <rc/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

static int running = 0;

static void __signal_handler(__attribute__ ((unused)) int dummy)
{
    running = 0;
    return;
}

int main(int argc, char **argv) {

    int fd, i;
    void *map_base, *virt_addr;
    unsigned long read_result;
    unsigned int shared[2];  // Memoria compartida para shared[0] y shared[1]
    unsigned int data_array[1];  // Array para los bits 0-3 de shared[1]

    off_t target = 0x4a310000;  // Dirección de la memoria compartida (ajustar según sea necesario)

    // Establecer el controlador de señal para manejar CTRL+C y terminar el programa
    signal(SIGINT, __signal_handler);
    running = 1;

    // Abrir el dispositivo /dev/mem
    if((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1){
        printf("Failed to open memory!\n");
        return -1;
    }

    // Mapear la memoria
    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target & ~MAP_MASK);
    if(map_base == (void *) -1) {
        printf("Failed to map base address\n");
        return -1;
    }

    // Iteración principal
    while(running){
        // Acceso a la memoria compartida
        virt_addr = map_base + (target & MAP_MASK);
        memcpy(shared, virt_addr, sizeof(shared));  // Copiar la memoria compartida a nuestra variable

        // Paso 1: Borrar shared[1]
        shared[1] = 0;

        // Paso 2: Escribir un 1 en el bit 0 de shared[0]
        shared[0] |= 1;  // Establecer bit 0 en shared[0]

        // Copiar la memoria actualizada a la memoria mapeada
        memcpy(virt_addr, shared, sizeof(shared));

        // Paso 3: Esperar hasta que el bit 12 de shared[1] sea 1
        while ((shared[1] & (1 << 12)) == 0) {
            // Leer de nuevo si ha cambiado
            memcpy(shared, virt_addr, sizeof(shared));
            usleep(1000);  // Esperar un poco antes de leer de nuevo
        }

        // Paso 4: Cargar los bits 0-3 de shared[1] en el array
        data_array[0] = shared[1] & 0xF;  // Extraer los bits 0-3 de shared[1]

        printf("Datos cargados: %u\n", data_array[0]);
        fflush(stdout);

        // Finalizar la ejecución (puedes agregar más lógica si es necesario)
        break;
    }

    // Desmapear la memoria y cerrar el archivo
    if(munmap(map_base, MAP_SIZE) == -1) {
        printf("Failed to unmap memory");
        return -1;
    }
    close(fd);
    return 0;
}
