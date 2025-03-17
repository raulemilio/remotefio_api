#ifndef PRU_CONTROL_H
#define PRU_CONTROL_H

void load_firmware(const char *firmware_name);
void start_pru();
void stop_pru();

#endif // PRU_CONTROL_H
