#ifndef PRU_CONTROL_H
#define PRU_CONTROL_H

// ==========================
// PRU Paths y Firmwares
// ==========================
#define PRU0_PATH                                  "/sys/class/remoteproc/remoteproc1"
#define PRU1_PATH                                  "/sys/class/remoteproc/remoteproc2"

#define PRU0_FIRMWARE                              "am335x-pru0-analog-fw"
#define PRU1_FIRMWARE                              "am335x-pru1-digital-fw"

// ==========================
// Constantes Generales
// ==========================
#define PRU_ERASE_MEM                              0

// ==========================
// SHARED Memory Indexes
// ==========================
// GPIO Input
#define PRU_SHD_GPIO_INPUT_MODE0_FLAG_INDEX        0
#define PRU_SHD_GPIO_INPUT_MODE0_DATA_INDEX        1
#define PRU_SHD_GPIO_INPUT_MODE1_FLAG_INDEX        2
#define PRU_SHD_GPIO_INPUT_MODE1_DATA_INDEX        3
#define PRU_SHD_GPIO_INPUT_MODE2_FLAG_INDEX        4
#define PRU_SHD_GPIO_INPUT_MODE2_DATA_INDEX        5

// GPIO Output get
#define PRU_SHD_GPIO_OUTPUT_GET_MODE0_FLAG_INDEX   6
#define PRU_SHD_GPIO_OUTPUT_GET_MODE0_DATA_INDEX   7
// GPIO Output set
#define PRU_SHD_GPIO_OUTPUT_SET_MODE0_FLAG_INDEX   8
#define PRU_SHD_GPIO_OUTPUT_SET_MODE0_DATA_INDEX   9

// Motor get
#define PRU_SHD_MOTOR_GET_MODE0_FLAG_INDEX         10
#define PRU_SHD_MOTOR_GET_MODE0_DATA_INDEX         11
#define PRU_SHD_MOTOR_GET_MODE1_FLAG_INDEX         12
#define PRU_SHD_MOTOR_GET_MODE1_DATA_INDEX         13
#define PRU_SHD_MOTOR_GET_MODE2_FLAG_INDEX         14
#define PRU_SHD_MOTOR_GET_MODE2_DATA_INDEX         15
// Motor set
#define PRU_SHD_MOTOR_SET_MODE0_FLAG_INDEX         16
#define PRU_SHD_MOTOR_SET_MODE0_DATA_INDEX         17

// ==========================
// SHARED[flag_index] Bits
// ==========================
// GPIO Input Flags
#define PRU_GPIO_INPUT_MODE0_FLAG                  0
#define PRU_GPIO_INPUT_MODE1_FLAG                  0
#define PRU_GPIO_INPUT_MODE2_FLAG                  0

// GPIO Output Flags
#define PRU_GPIO_OUTPUT_GET_MODE0_FLAG             0
#define PRU_GPIO_OUTPUT_SET_MODE0_FLAG             0

// Motor Flags
#define PRU_MOTOR_GET_MODE0_FLAG                   0
#define PRU_MOTOR_GET_MODE1_FLAG                   0
#define PRU_MOTOR_GET_MODE2_FLAG                   0

#define PRU_MOTOR_SET_MODE0_FLAG                   0

// ==========================
// DATARDY Flags (Bit 12)
// ==========================
// GPIO Input
#define PRU_GPIO_INPUT_MODE0_DATARDY_FLAG          12
#define PRU_GPIO_INPUT_MODE1_DATARDY_FLAG          12
#define PRU_GPIO_INPUT_MODE2_DATARDY_FLAG          12

// GPIO Output get
#define PRU_GPIO_OUTPUT_GET_MODE0_DATARDY_FLAG     12
// GPIO Output set
#define PRU_GPIO_OUTPUT_SET_MODE0_DATARDY_FLAG     12

// Motor
#define PRU_MOTOR_GET_MODE0_DATARDY_FLAG           12
#define PRU_MOTOR_GET_MODE1_DATARDY_FLAG           12
#define PRU_MOTOR_GET_MODE2_DATARDY_FLAG           12

#define PRU_MOTOR_SET_MODE0_DATARDY_FLAG           12

// ==========================
// Mensajes Genericos (sin cambiar nombres)
// ==========================
#define ERR_FIRMWARE_LOAD   "FW load fail: %s"
#define ERR_PRU_START       "PRU start fail: %s"
#define PRU_ON_SUCCESS      "PRU %s ON w/FW %s"
#define ERR_PRU_STOP        "PRU stop fail: %s"
#define PRU_OFF_SUCCESS     "PRU %s OFF"

// ===========================================================
// Declaracion de funcion para controlar PRUs
// -----------------------------------------------------------
// Parametros:
// - start: 1 para iniciar la PRU, 0 para detenerla
// - pru_path: ruta al remoteproc correspondiente (PRU0 o PRU1)
// - firmware: nombre del firmware a cargar si se inicia la PRU
// ===========================================================
int control_pru(int start, const char *pru_path, const char *firmware);

#endif // PRU_CONTROL_H
