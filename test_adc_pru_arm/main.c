/*
 contador in| PRU IN| P8_16| pr1_pru0_pru_r31_14 |config-pin P8_16 pruin  |
 */

#include <stdio.h>
#include <signal.h>
#include <rc/time.h>
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

	double sum_voltage = 0.0;

    while(running){
   // for(j=0; j<1000; j++){
       for(i=0; i<numberOutputSamples; i++){
           virt_addr = map_base + (target & MAP_MASK);
           read_result = *((unsigned long *) virt_addr);
           voltage = ((double)read_result / 4095.0)*1.8;
           printf("Value is: %.3f V\n", voltage);
		//sum_voltage+=voltage;
	}
       fflush(stdout);
	//printf("Value is: %.4f V\n", (sum_voltage/numberOutputSamples));
	sum_voltage =0.0;
	rc_usleep(200000);
    }

    if(munmap(map_base, MAP_SIZE) == -1) {
       printf("Failed to unmap memory");
       return -1;
    }
    close(fd);
    return 0;
}
