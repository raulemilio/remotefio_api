#ifndef MOTOR_H
#define MOTOR_H

//MOTOR
#define ERR_MOTOR_ARRAY                  "Err: motor field"
#define ERR_MOTOR_COUNT                  "Err: motor count"
#define ERR_MOTOR_VALUE                  "Err: motor value"
#define ERR_MOTOR_FIELDS                 "Err: motor field"
#define ERR_MOTOR_ARRAY_SIZE             "Err: motor array"
#define ERR_MOTOR_DIR_VALUE              "Err: motor dir  "
#define ERR_MOTOR_ENA_VALUE              "Err: motor ena  "
#define ERR_MOTOR_RPM_VALUE              "Err: motor rpm  "
#define ERR_MOTOR_INVALID_STEP_PER_REV   "Err: motor rev  "
#define ERR_MOTOR_INVALID_MICRO_STEP     "Err: motor step "
#define ERR_MOTOR_STEPS_DIV_ZERO         "Err: motor zero "
#define ERR_MOTOR_INVALID_FIELD_FACTOR   "Err: motor conf "

// 16 caracteres maximo
#define MSG_MOTOR_INVALID_MODE           "Motor inv mod   "
#define MSG_MOTOR_DONE                   "Motor done      "
#define MSG_MOTOR_STOPPED                "Motor in stopped"
#define MSG_MOTOR_TIMEOUT_EXPIRED        "Motor exp TO    "
#define MSG_MOTOR_FINISH                 "Motor finish    "

#define JSON_KEY_MOTOR                   "motor"
#define JSON_KEY_MOTOR_DIR               "dir"
#define JSON_KEY_MOTOR_ENA               "ena"
#define JSON_KEY_MOTOR_RPM               "rpm"
#define JSON_KEY_MOTOR_STEP_PER_REV      "step_per_rev"
#define JSON_KEY_MOTOR_MICRO_STEP        "micro_step"

#define MOTOR_MICRO_STEP_FULL_NAME       "full"
#define MOTOR_MICRO_STEP_HALF_NAME       "half"
#define MOTOR_MICRO_STEP_QUARTER_NAME    "quarter"
#define MOTOR_MICRO_STEP_EIGHTH_NAME     "eighth"
#define MOTOR_MICRO_STEP_SIXTEENTH_NAME  "sixteenth"

#define MOTOR_MICRO_STEP_FULL_VALUE       1
#define MOTOR_MICRO_STEP_HALF_VALUE       2
#define MOTOR_MICRO_STEP_QUARTER_VALUE    4
#define MOTOR_MICRO_STEP_EIGHTH_VALUE     8
#define MOTOR_MICRO_STEP_SIXTEENTH_VALUE  16

#define MOTOR_CH_MIN                      0
#define MOTOR_CH_MAX                      3
#define MOTOR_NUM_MIN                     1
#define MOTOR_NUM_MAX                     4

#define MOTOR_DIR0                        0
#define MOTOR_DIR1                        1
#define MOTOR_ENA_DISABLE                 0
#define MOTOR_ENA_ENABLE                  1
#define MOTOR_RPM_MIN                     1
#define MOTOR_RPM_MAX                     1000 // para un NEMA 17 es el maximo
#define MOTOR_STEP_PER_REV_MIN            1
#define MOTOR_STEP_PER_REV_MAX            400 // 200 para NEMA 17
// Esta constante sale de multiplicar rpm = 420 x step_per_rev = 200 x micro_step_sixteenth = 16
// este valor de 1344000 da un factor_step_time de 10.6292 que representa un periodo de 42, 6 us.
// El periodo minimo que el pru puede procesar son 4.7 us que corresponde a un factor_step_time de aprox 1
// por lo tanto se usa un factor de seguridad de 10 para evitar trabajar al maximo
#define MOTOR_MAX_FIELD_SIZE              1344000
#define MOTOR_FACTOR_STEP_TIME            0.0042f // 10000 -> 42 ms | 1000 -> 4.2ms lineal valor minimo 1 -> 4,7 us


#define MOTOR_OFFSET_ENABLE               4
#define MOTOR_OFFSET_DIRECTION            5
#define MOTOR_OFFSET_STEPTIME_SHD_INDEX   18 //Shared[18]

#define MOTOR_STEPS_PER_SECOND_DEBUG      "steps_per_second: %.2f"

// Variables compartidas
extern int rpm[MOTOR_NUM_MAX];
extern int step_per_rev[MOTOR_NUM_MAX];
extern int micro_step[MOTOR_NUM_MAX];

// Mutex para proteger el acceso a estas variables
extern pthread_mutex_t motor_data_mutex;

#endif

