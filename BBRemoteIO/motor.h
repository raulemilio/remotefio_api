#ifndef MOTOR_H
#define MOTOR_H

//MOTOR
#define ERROR_MOTOR_ARRAY               "Err: motor field"
#define ERROR_MOTOR_COUNT               "Err: motor count"
#define ERROR_MOTOR_VALUE               "Err: motor value"
#define ERROR_FIELDS                    "Err: field      "
#define ERROR_ARRAY_SIZE                "Err: array size "
#define ERROR_DIR_VALUE                 "Err: dir value  "
#define ERROR_ENA_VALUE                 "Err: ena value  "
#define ERROR_STEP_TIME_VALUE           "Err: step value "

// 16 caracteres maximo
#define MSG_MOTOR_INVALID_MODE          "Motor inv mod   "
#define MSG_MOTOR_DONE                  "Motor done      "
#define MSG_MOTOR_STOPPED               "Motor in stopped"
#define MSG_MOTOR_TIMEOUT_EXPIRED       "Motor exp TO    "
#define MSG_MOTOR_FINISH                "Motor finish    "

#define JSON_KEY_MOTOR                  "motor"
#define JSON_KEY_MOTOR_DIR              "dir"
#define JSON_KEY_MOTOR_ENA              "ena"
#define JSON_KEY_MOTOR_STEP_TIME        "step_time"

#define MOTOR_CH_MIN                    0
#define MOTOR_CH_MAX                    3
#define MOTOR_NUM_MIN                   1
#define MOTOR_NUM_MAX                   4

#define MOTOR_DIR0                      0
#define MOTOR_DIR1                      1
#define MOTOR_ENA_DISABLE               0
#define MOTOR_ENA_ENABLE                1
#define MOTOR_STEP_TIME_MIN             10
#define MOTOR_STEP_TIME_MAX             1000000 // aprox 1,34 s de semiperiodo

#define MOTOR_OFFSET_ENABLE             4
#define MOTOR_OFFSET_DIRECTION          5
#define MOTOR_OFFSET_STEPTIME_SHD_INDEX 18 //Shared[18]

#endif

