#include "pru_control.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Para usar strcat()

#define REMOTEPROC_PATH_0 "/sys/class/remoteproc/remoteproc0/"
#define REMOTEPROC_PATH_1 "/sys/class/remoteproc/remoteproc1/"

// Función que carga el firmware en un PRU específico
void load_firmware(const char *firmware_name, int pru_number) {
    const char *path = pru_number == 0 ? REMOTEPROC_PATH_0 : REMOTEPROC_PATH_1;
    char full_path[256]; // Asegúrate de que la longitud del path sea suficiente
    snprintf(full_path, sizeof(full_path), "%sfirmware", path); // Concatenar la ruta correctamente

    FILE *file = fopen(full_path, "w");
    if (file == NULL) {
        perror("Error al abrir firmware");
        return;
    }
    fprintf(file, "%s", firmware_name);
    fclose(file);
    printf("Firmware '%s' cargado correctamente en PRU%d.\n", firmware_name, pru_number);
}

// Función que inicia un PRU específico
void start_pru(int pru_number) {
    const char *path = pru_number == 0 ? REMOTEPROC_PATH_0 : REMOTEPROC_PATH_1;
    char full_path[256]; // Asegúrate de que la longitud del path sea suficiente
    snprintf(full_path, sizeof(full_path), "%sstate", path); // Concatenar la ruta correctamente

    FILE *file = fopen(full_path, "w");
    if (file == NULL) {
        perror("Error al iniciar PRU");
        return;
    }
    fprintf(file, "start");
    fclose(file);
    printf("PRU%d iniciado.\n", pru_number);
}

// Función que detiene un PRU específico
void stop_pru(int pru_number) {
    const char *path = pru_number == 0 ? REMOTEPROC_PATH_0 : REMOTEPROC_PATH_1;
    char full_path[256]; // Asegúrate de que la longitud del path sea suficiente
    snprintf(full_path, sizeof(full_path), "%sstate", path); // Concatenar la ruta correctamente

    FILE *file = fopen(full_path, "w");
    if (file == NULL) {
        perror("Error al detener PRU");
        return;
    }
    fprintf(file, "stop");
    fclose(file);
    printf("PRU%d detenido.\n", pru_number);
}
