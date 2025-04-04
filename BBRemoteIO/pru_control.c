#include "pru_control.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Para usar strcat()

int control_pru(int start, const char *pru_path, const char *firmware) {
    char command[128];
    int status;

    if (start) {
        // Cargar el firmware
        snprintf(command, sizeof(command), "echo '%s' > %s/firmware", firmware, pru_path);
        status = system(command);
        if (status != 0) {
            printf(ERR_FIRMWARE_LOAD, firmware, pru_path);
            return -1;  // Error al cargar el firmware
        }

        // Iniciar la PRU
        snprintf(command, sizeof(command), "echo 'start' > %s/state", pru_path);
        status = system(command);
        if (status != 0) {
            printf(ERR_PRU_START, pru_path);
            return -2;  // Error al iniciar la PRU
        }

        printf(PRU_ON_SUCCESS, pru_path, firmware);
    } else {
        // Detener la PRU
        snprintf(command, sizeof(command), "echo 'stop' > %s/state", pru_path);
        status = system(command);
        if (status != 0) {
            printf(ERR_PRU_STOP, pru_path);
            return -3;  // Error al detener la PRU
        }

        printf(PRU_OFF_SUCCESS, pru_path);
    }

    return 0;  //exito
}
