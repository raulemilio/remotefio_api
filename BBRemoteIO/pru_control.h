#ifndef PRU_CONTROL_H
#define PRU_CONTROL_H

// Declaracion de funciones para controlar PRUs
void load_firmware(const char *firmware_name, int pru_number);
void start_pru(int pru_number);
void stop_pru(int pru_number);

#endif // PRU_CONTROL_H
