#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <mosquitto.h>
#include <cjson/cJSON.h>

#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>
#include <time.h>
#include <rc/i2c.h>

#include "i2c_lcd.h"
#include "mqtt_callback.h"
#include "pru_control.h"
#include "lcd_system_status.h"

extern struct mosquitto* mosq = NULL;  // Define mosq only once

// Manejo de seniales para salir limpiamente
void handle_signal(int signo) {
    printf("Cliente MQTT...closed\n");
    if (mosq) {
        mosquitto_disconnect(mosq);
        mosquitto_destroy(mosq);
    }
    mosquitto_lib_cleanup();
    // Apagar PRU0
    if (control_pru(0, PRU0_PATH, PRU0_FIRMWARE) == 0) {
        printf(PRU0_OFF_SUCCESS);
    } else {
        printf(PRU0_OFF_ERROR);
    }

    // Apagar PRU0
    if (control_pru(0, PRU1_PATH, PRU1_FIRMWARE) == 0) {
        printf(PRU1_OFF_SUCCESS);
    } else {
        printf(PRU1_OFF_ERROR);
    }

    exit(0);
}

int main() {
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    int ret = lcd_system_init();// LCD init

    if(ret == -1){
    lcd_system_end();
    printf("lcd init() error");
    }

    // Encender PRU0
    if (control_pru(1, PRU0_PATH, PRU0_FIRMWARE) == 0) {
        printf(PRU0_ON_SUCCESS);
        lcd_system_status(LCD_PRU0_START);
    } else {
        printf(PRU0_ON_ERROR);
        lcd_system_status(LCD_PRU0_ERROR);
    }
    sleep(LCD_FUNCTION_TIMEOUT); // demoramos para ver la carga del PRU

    // Encender PRU1
    if (control_pru(1, PRU1_PATH, PRU1_FIRMWARE) == 0) {
        printf(PRU1_ON_SUCCESS);
        lcd_system_status(LCD_PRU1_START);
    } else {
        printf(PRU1_ON_ERROR);
        lcd_system_status(LCD_PRU1_ERROR);
    }

    sleep(LCD_FUNCTION_TIMEOUT); // demoramos para ver la carga del PRU

    mosquitto_lib_init();
    mosq = mosquitto_new(CLIENT_ID, true, NULL);

    if (!mosq) {
        fprintf(stderr, "Cliente MQTT error\n");
        mosquitto_lib_cleanup();
        lcd_system_status(LCD_MQTT_CLIENT_ERROR);
        return EXIT_FAILURE;
    }

    mosquitto_message_callback_set(mosq, message_callback);

    if (mosquitto_connect(mosq, BROKER_ADDRESS, BROKER_PORT, 60)) {
        fprintf(stderr, "Failed to connect to the MQTT broker.\n");
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        lcd_system_status(LCD_MQTT_CONNECT_ERROR);
        return EXIT_FAILURE;
    }

    // Suscribirse a los cuatro tpicos
    if (mosquitto_subscribe(mosq, NULL, TOPIC_CMDS_ADC, 0) ||
        mosquitto_subscribe(mosq, NULL, TOPIC_CMDS_GPIO_INPUT, 0) ||
        mosquitto_subscribe(mosq, NULL, TOPIC_CMDS_GPIO_OUTPUT, 0) ||
        mosquitto_subscribe(mosq, NULL, TOPIC_CMDS_MOTOR, 0) ||
        mosquitto_subscribe(mosq, NULL, TOPIC_CMDS_SYSTEM, 0)) {
        fprintf(stderr, "Error subscribing to one or more topics.\n");
        mosquitto_disconnect(mosq);
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        lcd_system_status(LCD_MQTT_TOPIC_ERROR);
        return EXIT_FAILURE;
    }

    printf("Connected to the MQTT broker and subscribed to the topics:\n");
    printf("  %s\n", TOPIC_CMDS_ADC);
    printf("  %s\n", TOPIC_CMDS_GPIO_INPUT);
    printf("  %s\n", TOPIC_CMDS_GPIO_OUTPUT);
    printf("  %s\n", TOPIC_CMDS_MOTOR);
    printf("  %s\n", TOPIC_CMDS_SYSTEM);

    lcd_system_status(LCD_SYSTEM_RUN);
    mosquitto_loop_forever(mosq, -1, 1);

    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    // Apagar PRU0
    if (control_pru(0, PRU0_PATH, PRU0_FIRMWARE) == 0) {
        printf(PRU0_OFF_SUCCESS);
    } else {
        printf(PRU0_OFF_ERROR);
    }
/*
    // Apagar PRU0
    if (control_pru(0, PRU1_PATH, PRU1_FIRMWARE) == 0) {
        printf(PRU1_OFF_SUCCESS);
    } else {
        printf(PRU1_OFF_ERROR);
    }
*/
    return 0;
}


