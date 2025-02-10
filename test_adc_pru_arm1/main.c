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
#include <time.h>

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
    double voltage;
    unsigned int numberOutputSamples = 1;
    off_t target = 0x4a300000; // PRU0-> RAM0 ram0[0]
    struct timespec start, end;

    // Set signal handler so the loop can exit cleanly
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

    FILE *file = fopen("voltage_data.txt", "a");
    if (file == NULL) {
        printf("Failed to open file for writing\n");
        return -1;
    }

    while (running) {
        clock_gettime(CLOCK_MONOTONIC, &start);// medimos el tiempo que tarda cada iteracion

        for (i = 0; i < numberOutputSamples; i++) {
            virt_addr = map_base + (target & MAP_MASK);
            read_result = *((unsigned long *) virt_addr);
            voltage = ((double)read_result / 4095.0) * 1.8;
            fprintf(file, "%.4f\n", voltage);
            fflush(file);
        }
	rc_usleep(250);
        clock_gettime(CLOCK_MONOTONIC, &end);
        double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
        printf("Value is: %.4f V, Cycle Time: %.6f seconds\n", voltage, elapsed_time);
    }

    fclose(file);

    if (munmap(map_base, MAP_SIZE) == -1) {
        printf("Failed to unmap memory\n");
        return -1;
    }
    close(fd);
    return 0;
}
