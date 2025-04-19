#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <mosquitto.h>
#include <cjson/cJSON.h>

#include <stdint.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>
#include <time.h>

#include "mqtt_callback.h"
#include "mqtt_response.h"
#include "pru_control.h"
#include "system.h"
#include "task_queue.h"
#include "mqtt_publish.h"

volatile bool system_running;
pthread_mutex_t system_running_mutex = PTHREAD_MUTEX_INITIALIZER;

TaskQueue system_queue;
pthread_t system_thread;

static void handle_lcd_function(ThreadSystemDataArgs args);
static void handle_pru0_function(ThreadSystemDataArgs args);
static void handle_pru1_function(ThreadSystemDataArgs args);
static void handle_adc_function(ThreadSystemDataArgs args);
static void handle_gpio_input_function(ThreadSystemDataArgs args);
static void handle_gpio_output_function(ThreadSystemDataArgs args);
static void handle_motor_get_function(ThreadSystemDataArgs args);
static void handle_motor_set_function(ThreadSystemDataArgs args);
static void handle_all_functions_function(ThreadSystemDataArgs args);
static void handle_pru_action(struct mosquitto *mosq, int action, const char *pru_path, const char *pru_firmware);
static void handle_generic_system_function(struct mosquitto *mosq, const char *name, pthread_mutex_t *mutex, volatile bool *flag, int action);
static void set_running(volatile bool *flag, pthread_mutex_t *mutex, bool value);

void *system_thread_func(void *arg) {
    ThreadSystemDataArgs args;
    //char mensaje[128];

    while (1) {
        if (task_queue_dequeue(&system_queue, &args, sizeof(ThreadSystemDataArgs)) == 0) {
            if (!args.system_data) continue;

            LOG_INFO("Function: system running");

	    FunctionType function_type = args.system_data->function;

            switch (function_type) {
        	case FUNC_LCD:
            		handle_lcd_function(args);
            		break;
        	case FUNC_PRU0:
            		handle_pru0_function(args);
            		break;
        	case FUNC_PRU1:
            		handle_pru1_function(args);
            		break;
        	case FUNC_ADC:
           		handle_adc_function(args);
            		break;
        	case FUNC_GPIO_INPUT:
            		handle_gpio_input_function(args);
            		break;
        	case FUNC_GPIO_OUTPUT:
            		handle_gpio_output_function(args);
            		break;
        	case FUNC_MOTOR_GET:
            		handle_motor_get_function(args);
            		break;
                case FUNC_MOTOR_SET:
                        handle_motor_set_function(args);
                        break;
        	case FUNC_ALL_FUNCTIONS:
            		handle_all_functions_function(args);
	    		break;
        	default:
            		LOG_ERROR(MSG_SYSTEM_INVALID_OPTION);
            		break;
    		}

            pthread_mutex_lock(&system_running_mutex);
            system_running = TASK_STOPPED;
            pthread_mutex_unlock(&system_running_mutex);

            publish_system_response_status(args.mosq, MSG_SYSTEM_FINISH);
            //lcd_show_message(MSG_SYSTEM_FINNISH);
            free(args.system_data);
            //Limpieza del struct para evitar basura en la proxima iteracion
            memset(&args, 0, sizeof(args));
        }
    }
    return NULL;
}
// Auxilar functions
// LCD
static void handle_lcd_function(ThreadSystemDataArgs args) {
    LOG_DEBUG("system lcd function");
    publish_system_response_status(args.mosq, MSG_SYSTEM_LCD_CLEAN);
    lcd_show_message(MSG_LCD_ERASE_SCREEN);
}

// PRU0
static void handle_pru0_function(ThreadSystemDataArgs args) {
    LOG_DEBUG("system pru0 function");
    handle_pru_action(args.mosq, args.system_data->action, PRU0_PATH, PRU0_FIRMWARE);
}
// PRU1
static void handle_pru1_function(ThreadSystemDataArgs args) {
    LOG_DEBUG("system pru1 function");
    handle_pru_action(args.mosq, args.system_data->action, PRU1_PATH, PRU1_FIRMWARE);
}
// ADC
static void handle_adc_function(ThreadSystemDataArgs args) {
    handle_generic_system_function(args.mosq, "adc", &adc_running_mutex, &adc_running, args.system_data->action);
}
// GPIO_INPUT
static void handle_gpio_input_function(ThreadSystemDataArgs args) {
    handle_generic_system_function(args.mosq, "gpio_input", &gpio_input_running_mutex, &gpio_input_running, args.system_data->action);
}
// GPIO_OUTPUT
static void handle_gpio_output_function(ThreadSystemDataArgs args) {
    handle_generic_system_function(args.mosq, "gpio_output", &gpio_output_running_mutex, &gpio_output_running, args.system_data->action);
}
// MOTOR GET
static void handle_motor_get_function(ThreadSystemDataArgs args) {
    handle_generic_system_function(args.mosq, "motor_get", &motor_get_running_mutex, &motor_get_running, args.system_data->action);
}
// MOTOR SET
static void handle_motor_set_function(ThreadSystemDataArgs args) {
    handle_generic_system_function(args.mosq, "motor_set", &motor_set_running_mutex, &motor_set_running, args.system_data->action);
}
// ALL_FUNCTIONS
static void handle_all_functions_function(ThreadSystemDataArgs args) {
    LOG_INFO("system all_functions function");
    bool action;

    switch (args.system_data->action) {
        case ACTION_START:
            action = true;
            set_running(&adc_running, &adc_running_mutex, action);
            set_running(&gpio_input_running, &gpio_input_running_mutex, action);
            set_running(&gpio_output_running, &gpio_output_running_mutex, action);
            set_running(&motor_get_running, &motor_get_running_mutex, action);
            set_running(&motor_set_running, &motor_set_running_mutex, action);
            publish_system_response_status(args.mosq, MSG_SYSTEM_ALL_FUNCTION_ON);
            lcd_show_message(MSG_SYSTEM_ALL_FUNCTION_ON);
            break;
        case ACTION_STOP:
            action = false;
            set_running(&adc_running, &adc_running_mutex, action);
            set_running(&gpio_input_running, &gpio_input_running_mutex, action);
            set_running(&gpio_output_running, &gpio_output_running_mutex, action);
            set_running(&motor_get_running, &motor_get_running_mutex, action);
            set_running(&motor_set_running, &motor_set_running_mutex, action);
            publish_system_response_status(args.mosq, MSG_SYSTEM_ALL_FUNCTION_OFF);
            lcd_show_message(MSG_SYSTEM_ALL_FUNCTION_OFF);
            break;
        default:
            publish_system_response_status(args.mosq, MSG_SYSTEM_INVALID_OPTION);
            lcd_show_message(MSG_SYSTEM_INVALID_OPTION);
            printf(MSG_SYSTEM_INVALID_OPTION);
            break;
    }
}
// Funciones auxiliares
static void handle_pru_action(struct mosquitto *mosq, int action, const char *pru_path, const char *pru_firmware) {
    const char *on_success_msg;
    const char *on_error_msg;
    const char *off_success_msg;
    const char *off_error_msg;

    // Determinar mensajes segun pru_path
    if (strcmp(pru_path, PRU0_PATH) == 0) {
        on_success_msg = MSG_LCD_PRU0_ON_SUCCESS;
        on_error_msg = MSG_LCD_PRU0_ON_ERROR;
        off_success_msg = MSG_LCD_PRU0_OFF_SUCCESS;
        off_error_msg = MSG_LCD_PRU0_OFF_ERROR;
    } else {
        on_success_msg = MSG_LCD_PRU1_ON_SUCCESS;
        on_error_msg = MSG_LCD_PRU1_ON_ERROR;
        off_success_msg = MSG_LCD_PRU1_OFF_SUCCESS;
        off_error_msg = MSG_LCD_PRU1_OFF_ERROR;
    }

    switch (action) {
        case ACTION_START:
            // Encender PRU
            if (control_pru(1, pru_path, pru_firmware) == 0) {
                LOG_INFO("%s", on_success_msg);
                publish_system_response_status(mosq, on_success_msg);
                lcd_show_message(on_success_msg);
            } else {
                LOG_ERROR("%s", on_error_msg);
                publish_system_response_status(mosq, on_error_msg);
                lcd_show_message(on_error_msg);
            }
            break;
        case ACTION_STOP:
            // Apagar PRU
            if (control_pru(0, pru_path, pru_firmware) == 0) {
                LOG_INFO("%s", off_success_msg);
                publish_system_response_status(mosq, off_success_msg);
                lcd_show_message(off_success_msg);
            } else {
                LOG_ERROR("%s", off_error_msg);
                publish_system_response_status(mosq, off_error_msg);
                lcd_show_message(off_error_msg);
            }
            break;
        default:
            publish_system_response_status(mosq, MSG_SYSTEM_INVALID_OPTION);
            lcd_show_message(MSG_SYSTEM_INVALID_OPTION);
            LOG_ERROR(MSG_SYSTEM_INVALID_OPTION);
            break;
    }
}
static void handle_generic_system_function(struct mosquitto *mosq, const char *name, pthread_mutex_t *mutex, volatile bool *flag, int action) {
    LOG_INFO("system %s function", name);

    char lcd_msg[16];  // 15 + 1 para null terminator
    char pub_msg[64];  // mensaje mas largo para publicacion si se desea mas detalle

    // Truncar el nombre si es muy largo
    char short_name[8];  // Hasta 8 caracteres del nombre (para dejar espacio al estado)
    snprintf(short_name, sizeof(short_name), "%.7s", name);

    switch (action) {
        case ACTION_START:
            pthread_mutex_lock(mutex);
            *flag = true;
            pthread_mutex_unlock(mutex);

            snprintf(pub_msg, sizeof(pub_msg), "%s ON", short_name);
            publish_system_response_status(mosq, pub_msg);
            LOG_INFO("%s", pub_msg);

            snprintf(lcd_msg, sizeof(lcd_msg), "%s ON", short_name);
            lcd_show_message(lcd_msg);
            break;
        case ACTION_STOP:
            pthread_mutex_lock(mutex);
            *flag = false;
            pthread_mutex_unlock(mutex);

            snprintf(pub_msg, sizeof(pub_msg), "%s OFF", short_name);
            publish_system_response_status(mosq, pub_msg);
            LOG_INFO("%s", pub_msg);

            snprintf(lcd_msg, sizeof(lcd_msg), "%s OFF", short_name);
            lcd_show_message(lcd_msg);
            break;
        default:
            snprintf(pub_msg, sizeof(pub_msg), "%s INVALID", short_name);
            publish_system_response_status(mosq, pub_msg);
            LOG_ERROR("%s", pub_msg);

            snprintf(lcd_msg, sizeof(lcd_msg), "%s INVALID", short_name);
            lcd_show_message(lcd_msg);
            LOG_ERROR(MSG_SYSTEM_INVALID_OPTION);
            break;
    }
}

static void set_running(volatile bool *flag, pthread_mutex_t *mutex, bool value) {
    pthread_mutex_lock(mutex);
    *flag = value;
    pthread_mutex_unlock(mutex);
}
