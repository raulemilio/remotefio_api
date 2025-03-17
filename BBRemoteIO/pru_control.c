#include "pru_control.h"
#include <stdio.h>
#include <stdlib.h>

#define REMOTEPROC_PATH "/sys/class/remoteproc/remoteproc1/"

void load_firmware(const char *firmware_name) {
    FILE *file = fopen(REMOTEPROC_PATH "firmware", "w");
    if (file == NULL) {
        perror("Error al abrir firmware");
        return;
    }
    fprintf(file, "%s", firmware_name);
    fclose(file);
    printf("Firmware '%s' cargado correctamente.\n", firmware_name);
}

void start_pru() {
    FILE *file = fopen(REMOTEPROC_PATH "state", "w");
    if (file == NULL) {
        perror("Error al iniciar PRU");
        return;
    }
    fprintf(file, "start");
    fclose(file);
    printf("PRU iniciado.\n");
}

void stop_pru() {
    FILE *file = fopen(REMOTEPROC_PATH "state", "w");
    if (file == NULL) {
        perror("Error al detener PRU");
        return;
    }
    fprintf(file, "stop");
    fclose(file);
    printf("PRU detenido.\n");
}
