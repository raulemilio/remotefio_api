#ifndef PRU_CONTROL_H
#define PRU_CONTROL_H

#define ERR_FIRMWARE_LOAD          "Error: Failed to load firmware %s on %s.\n"
#define ERR_PRU_START              "Error: Failed to start PRU on %s.\n"
#define PRU_ON_SUCCESS             "PRU on %s turned on with firmware %s.\n"
#define ERR_PRU_STOP               "Error: Failed to stop PRU on %s.\n"
#define PRU_OFF_SUCCESS            "PRU on %s turned off.\n"

// Declaracion de funciones para controlar PRUs
int control_pru(int start, const char *pru_path, const char *firmware);

#endif // PRU_CONTROL_H
