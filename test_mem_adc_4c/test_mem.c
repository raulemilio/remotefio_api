/*
 contador in| PRU IN| P8_16| pr1_pru0_pru_r31_14 |config-pin P8_16 pruin  |
 */

/*
#include <stdio.h>
#include <signal.h>
//#include <rc/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/mman.h>

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

static int running = 0;

// interrupt handler to catch ctrl-c
static void __signal_handler(__attribute__ ((unused)) int dummy)
{
        running=0;
        return;
}
int main(int argc, char **argv) {

    int fd, i, j;
    void *map_base, *virt_addr;

    unsigned long read_result, writeval;
    double voltage;
    unsigned int numberOutputSamples = 1;

    off_t target = 0x4a300000;//PRU0-> RAM0 ram0[0]
//    off_t target = 0x4a302000;//PRU0-> RAM1 ram1[0]
//    off_t target = 0x4a310000;//PRU0-> RAM0 shared[0]

    // set signal handler so the loop can exit cleanly
    signal(SIGINT, __signal_handler);
    running=1;

    if((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1){
        printf("Failed to open memory!\n");
        return -1;
    }
    fflush(stdout);
    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target & ~MAP_MASK);
    if(map_base == (void *) -1) {
       printf("Failed to map base address\n");
       return -1;
    }
    fflush(stdout);

    while(running){
   // for(j=0; j<1000; j++){
       for(i=0; i<numberOutputSamples; i++){
           virt_addr = map_base + (target & MAP_MASK);
           read_result = *((unsigned long *) virt_addr);
//         voltage = ((double)read_result / 4095.0)*1.8;
//         printf("Value is: %.3f V\n", voltage);
//           printf("Value at address 0x%X is: %d\n", target, read_result);
           printf("Value at address 0x%X is: 0x%X\n", target, read_result);
  //         target+=2;                   // 2 bytes per sample
       }
       fflush(stdout);
    }

    if(munmap(map_base, MAP_SIZE) == -1) {
       printf("Failed to unmap memory");
       return -1;
    }
    close(fd);
    return 0;
}
*/

/*
#include <stdio.h>
#include <signal.h>
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

// interrupt handler to catch ctrl-c
static void __signal_handler(__attribute__ ((unused)) int dummy)
{
        running = 0;
        return;
}

int main(int argc, char **argv) {

    int fd, i;
    void *map_base, *virt_addr;
    unsigned long read_result;
    off_t target = 0x4a300000; // PRU0-> RAM0 ram0[0]

    // set signal handler so the loop can exit cleanly
    signal(SIGINT, __signal_handler);
    running = 1;

    if ((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) {
        printf("Failed to open memory!\n");
        return -1;
    }
    fflush(stdout);
    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target & ~MAP_MASK);
    if (map_base == (void *) -1) {
        printf("Failed to map base address\n");
        return -1;
    }
    fflush(stdout);

    while (running) {
        // Leer las 4 primeras posiciones de RAM0 del PRU0
        for (i = 0; i < 4; i++) {
            virt_addr = map_base + (target & MAP_MASK) + (i * sizeof(unsigned long));
            read_result = *((unsigned long *) virt_addr);
            printf("Value at address 0x%X is: 0x%lX\n", target + (i * sizeof(unsigned long)), read_result);
        }
        fflush(stdout);
    }

    if (munmap(map_base, MAP_SIZE) == -1) {
        printf("Failed to unmap memory");
        return -1;
    }
    close(fd);
    return 0;
}
*/

/*
// Lee los primeros 4096 datos de ram0
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>

#define MAP_SIZE 16384UL  // 16KB para cubrir 4096 valores de 4 bytes
#define NUM_VALUES 1024   // Cantidad de valores a leer
#define DATA_SIZE sizeof(uint32_t)  // Tamaño de cada dato (4 bytes)

int main(int argc, char **argv) {
    int fd, i;
    void *map_base, *virt_addr;
    uint32_t read_result;
    off_t target = 0x4A300000; // Dirección base de la memoria

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

    // Leer los primeros 4096 valores (cada uno de 4 bytes)
    for (i = 0; i < NUM_VALUES; i++) {
        virt_addr = map_base + (i * DATA_SIZE);  // Desplazamiento correcto para leer cada valor
        read_result = *((uint32_t *) virt_addr);
        printf("Value at address 0x%lX is: 0x%X\n", (unsigned long)(target + (i * DATA_SIZE)), read_result);
    }

    // Desmontar la memoria
    if (munmap(map_base, MAP_SIZE) == -1) {
        perror("Failed to unmap memory");
    }

    // Cerrar el descriptor de archivo
    close(fd);
    return 0;
}
*/
/*
// lee los primeros 4096 datos de la ram0 y los almacena en  en formato decimal en un archivo
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>

#define MAP_SIZE 16384UL  // 16KB para cubrir 4096 valores de 4 bytes (más allá de 0x4A300FFC)
#define NUM_VALUES 1023   // Solo leer hasta 0x4A300FFC, es decir, 1023 valores
#define DATA_SIZE sizeof(uint32_t)  // Tamaño de cada dato (4 bytes)

int main(int argc, char **argv) {
    int fd, i;
    void *map_base, *virt_addr;
    uint32_t read_result;
    off_t target = 0x4A300000; // Dirección base de la memoria
    FILE *output_file;

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

    // Abrir archivo de salida
    output_file = fopen("data_output.txt", "w");
    if (output_file == NULL) {
        perror("Failed to open output file");
        munmap(map_base, MAP_SIZE);
        close(fd);
        return -1;
    }

    // Leer solo los primeros 1023 valores, hasta la dirección 0x4A300FFC
    for (i = 0; i < NUM_VALUES; i++) {
        virt_addr = map_base + (i * DATA_SIZE);  // Desplazamiento correcto para leer cada valor
        read_result = *((uint32_t *) virt_addr);
        fprintf(output_file, "%d\n", read_result);  // Escribir el valor en formato decimal
    }

    // Cerrar el archivo de salida
    fclose(output_file);

    // Desmontar la memoria
    if (munmap(map_base, MAP_SIZE) == -1) {
        perror("Failed to unmap memory");
    }

    // Cerrar el descriptor de archivo
    close(fd);
    return 0;
}
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>

#define MAP_SIZE 16384UL  // 16KB para cubrir 4096 valores de 4 bytes
#define NUM_VALUES 4096   //
#define DATA_SIZE sizeof(uint32_t)  // Tamaño de cada dato (4 bytes)

int main(int argc, char **argv) {
    int fd, i;
    void *map_base, *virt_addr;
    uint32_t read_result;
    off_t target = 0x4A300000; // Dirección base de la memoria
    FILE *output_file;

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

    // Abrir archivo de salida
    output_file = fopen("data_output.txt", "w");
    if (output_file == NULL) {
        perror("Failed to open output file");
        munmap(map_base, MAP_SIZE);
        close(fd);
        return -1;
    }

    printf("Leyendo %d valores de la memoria RAM0:\n", NUM_VALUES);

    // Leer solo los primeros 1023 valores, hasta la dirección 0x4A300FFC
    for (i = 0; i < NUM_VALUES; i++) {
        virt_addr = map_base + (i * DATA_SIZE);  // Desplazamiento correcto para leer cada valor
        read_result = *((uint32_t *) virt_addr);
        fprintf(output_file, "%d\n", read_result);  // Escribir el valor en formato decimal
        printf("Dato %d: %d\n", i + 1, read_result);  // Mostrar en pantalla
    }

    printf("Lectura completada. Datos guardados en data_output.txt\n");

    // Cerrar el archivo de salida
    fclose(output_file);

    // Desmontar la memoria
    if (munmap(map_base, MAP_SIZE) == -1) {
        perror("Failed to unmap memory");
    }

    // Cerrar el descriptor de archivo
    close(fd);
    return 0;
}
