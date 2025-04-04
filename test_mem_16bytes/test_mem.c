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
//	   voltage = ((double)read_result / 4095.0)*1.8;
//	   printf("Value is: %.3f V\n", voltage);
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
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>

#define RAM0_BASE_ADDR 0x4A300000  // Dirección base de RAM0 (8 KB en PRU)
#define RAM0_SIZE      0x2000      // 8 KB (8192 valores de 1 byte)
#define NUM_VALUES     (RAM0_SIZE / sizeof(uint16_t))  // 4096 valores de 2 bytes

int main() {
    int fd;
    void *map_base;
    volatile uint16_t *ram0;
    FILE *output_file;

    // Abrir /dev/mem
    fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd == -1) {
        perror("Failed to open /dev/mem");
        return -1;
    }

    // Mapear RAM0
    map_base = mmap(0, RAM0_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, RAM0_BASE_ADDR);
    if (map_base == MAP_FAILED) {
        perror("Failed to map RAM0");
        close(fd);
        return -1;
    }

    ram0 = (volatile uint16_t *)map_base;

    // Abrir archivo de salida
    output_file = fopen("ram0_output.txt", "w");
    if (output_file == NULL) {
        perror("Failed to open output file");
        munmap(map_base, RAM0_SIZE);
        close(fd);
        return -1;
    }

    printf("Leyendo %d valores de RAM0 (2 bytes cada uno)...\n", NUM_VALUES);

    // Leer y guardar los valores de RAM0
    for (int i = 0; i < NUM_VALUES; i++) {
        uint16_t value = ram0[i];
        fprintf(output_file, "%u\n", value);  // Escribir en archivo
        printf("ram0[%d] = %u\n", i, value);  // Mostrar en pantalla
    }

    printf("Lectura completada. Datos guardados en 'ram0_output.txt'\n");

    // Cerrar recursos
    fclose(output_file);
    munmap(map_base, RAM0_SIZE);
    close(fd);

    return 0;
}
