#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Para usar strcat()
#include "pru_control.h"
#include "log.h"

#define CMD_LOAD_FIRMWARE  "echo '%s' > %s/firmware"
#define CMD_START_PRU      "echo 'start' > %s/state"
#define CMD_STOP_PRU       "echo 'stop' > %s/state"


int control_pru(int start, const char *pru_path, const char *firmware) {
    char command[128];
    int status;

    if (start) {
        // Cargar el firmware
        snprintf(command, sizeof(command), CMD_LOAD_FIRMWARE, firmware, pru_path);
        status = system(command);
        if (status != 0) {
            LOG_ERROR(ERR_FIRMWARE_LOAD, pru_path);
            return -1;  // Error al cargar el firmware
        }

        // Iniciar la PRU
        snprintf(command, sizeof(command), CMD_START_PRU, pru_path);
        status = system(command);
        if (status != 0) {
            LOG_ERROR(ERR_PRU_START, pru_path);
            return -2;  // Error al iniciar la PRU
        }

        LOG_INFO(PRU_ON_SUCCESS, pru_path, firmware);
    } else {
        // Detener la PRU
        snprintf(command, sizeof(command),  CMD_STOP_PRU, pru_path);
        status = system(command);
        if (status != 0) {
            LOG_ERROR(ERR_PRU_STOP, pru_path);
            return -3;  // Error al detener la PRU
        }

        LOG_INFO(PRU_OFF_SUCCESS, pru_path);
    }

    return 0;  //exito
}
