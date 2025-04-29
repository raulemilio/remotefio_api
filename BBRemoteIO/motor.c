#include <pthread.h>
#include "motor.h"

#define RPM_DEF          60
#define STEP_PER_REV_DEF 200
#define MICRO_STEP_DEF   1 // 1, 2, 4, 8, 16

// Definicion de variables
int rpm[MOTOR_NUM_MAX] = {RPM_DEF,
			  RPM_DEF,
			  RPM_DEF,
			  RPM_DEF};

int step_per_rev[MOTOR_NUM_MAX] = {STEP_PER_REV_DEF,
				   STEP_PER_REV_DEF,
				   STEP_PER_REV_DEF,
				   STEP_PER_REV_DEF};

int micro_step[MOTOR_NUM_MAX] =   {MICRO_STEP_DEF,
				   MICRO_STEP_DEF,
				   MICRO_STEP_DEF,
				   MICRO_STEP_DEF};

pthread_mutex_t motor_data_mutex = PTHREAD_MUTEX_INITIALIZER;
