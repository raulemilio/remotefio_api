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
#include "lcd_queue.h"
#include "pru_control.h"
#include "gpio_input.h"
#include "gpio_output_get.h"
#include "gpio_output_set.h"
#include "motor_get.h"
#include "motor_set.h"
#include "task_queue.h"
#include "lcd_display.h"
#include "mqtt_publish.h"
#include "mqtt_send_queue.h"
#include "shared_memory.h"
#include "pru_control.h"

#define RAM0_BASE     0x4A300000
#define RAM0_SIZE     0x2000 // 8KB
#define SHARED_BASE   0x4A310000
#define SHARED_SIZE   0x3000 // 12KB

#define LCD_TIME      2 // segundos

struct mosquitto* mosq = NULL;  // Define mosq only once
volatile sig_atomic_t keep_running = 1;

void handle_signal(int signo) {
    keep_running = 0;
}

int main() {

    signal(SIGINT, handle_signal);

    SharedMemoryContext shm_ctx;
    shm = &shm_ctx;  // Apunta el puntero global a la instancia local

    int fd_ram0, fd_shared;
    void *map_base_ram0, *map_base_shared;

    // descriptores de memoria
    if (map_memory(&fd_ram0, &map_base_ram0, (void **)&shm_ctx.ram0, RAM0_BASE, RAM0_SIZE) == -1) {
        exit(EXIT_FAILURE);
    }
    if (map_memory(&fd_shared, &map_base_shared, (void **)&shm_ctx.shared, SHARED_BASE, SHARED_SIZE) == -1) {
        unmap_memory(fd_ram0, map_base_ram0, RAM0_SIZE);
        exit(EXIT_FAILURE);
    }
    // inicializacion de mutex ram0
    pthread_mutex_init(&shm_ctx.ram0_mutex, NULL);
    // Inicializa todos los mutexes de shared_mutex
    for (int i = 0; i < SHARED_MUTEX_COUNT; i++) {
         pthread_mutex_init(&shm_ctx.shared_mutex[i], NULL);
    }
    //LCD
    lcd_queue_init(&lcd_display_queue);
    pthread_create(&lcd_display_thread, NULL, lcd_display_thread_func, NULL);

    // Encender PRU0
    if (control_pru(1, PRU0_PATH, PRU0_FIRMWARE) == 0) {
	lcd_show_message(MSG_LCD_PRU0_ON_SUCCESS);
    } else {
        lcd_show_message(MSG_LCD_PRU0_ON_ERROR);
    }

    sleep(LCD_TIME);

    // Encender PRU1
    if (control_pru(1, PRU1_PATH, PRU1_FIRMWARE) == 0) {
        lcd_show_message(MSG_LCD_PRU1_ON_SUCCESS);
    } else {
        lcd_show_message(MSG_LCD_PRU1_ON_ERROR);
    }

    sleep(LCD_TIME); // demoramos para ver la carga del PRU

    mosquitto_lib_init();
    mosq = mosquitto_new(CLIENT_ID, true, NULL);

    if (!mosq) {
        LOG_ERROR("Cliente MQTT error");
        mosquitto_lib_cleanup();
        return EXIT_FAILURE;
    }

    mosquitto_message_callback_set(mosq, message_callback);

    //MQTT PRUBLISH
    send_queue_init(&mqtt_publish_queue);
    pthread_create(&mqtt_publish_thread, NULL, mqtt_publish_thread_func, NULL);

    // Iniciamos las colas de funciones
    task_queue_init(&adc_queue);
    task_queue_init(&gpio_input_queue);
    task_queue_init(&gpio_output_get_queue);
    task_queue_init(&gpio_output_set_queue);
    task_queue_init(&motor_get_queue);
    task_queue_init(&motor_set_queue);
    task_queue_init(&system_queue);

    // Iniciamos hilo
    pthread_create(&adc_thread, NULL, adc_thread_func, NULL);
    pthread_create(&gpio_input_thread, NULL, gpio_input_thread_func, NULL);
    pthread_create(&gpio_output_get_thread, NULL, gpio_output_get_thread_func, NULL);
    pthread_create(&gpio_output_set_thread, NULL, gpio_output_set_thread_func, NULL);
    pthread_create(&motor_get_thread, NULL, motor_get_thread_func, NULL);
    pthread_create(&motor_set_thread, NULL, motor_set_thread_func, NULL);
    pthread_create(&system_thread, NULL, system_thread_func, NULL);

    if (mosquitto_connect(mosq, BROKER_ADDRESS, BROKER_PORT, 60)) {
        LOG_ERROR("Failed to connect to the MQTT broker");
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        lcd_show_message("Failed MQTT   ");
        return EXIT_FAILURE;
    }

    // Suscribirse a los cuatro tpicos
    if (mosquitto_subscribe(mosq, NULL, TOPIC_CMDS_ADC, 0) ||
        mosquitto_subscribe(mosq, NULL, TOPIC_CMDS_GPIO_INPUT, 0) ||
        mosquitto_subscribe(mosq, NULL, TOPIC_CMDS_GPIO_OUTPUT_GET, 0) ||
        mosquitto_subscribe(mosq, NULL, TOPIC_CMDS_GPIO_OUTPUT_SET, 0) ||
        mosquitto_subscribe(mosq, NULL, TOPIC_CMDS_MOTOR_GET, 0) ||
        mosquitto_subscribe(mosq, NULL, TOPIC_CMDS_MOTOR_SET, 0) ||
        mosquitto_subscribe(mosq, NULL, TOPIC_CMDS_SYSTEM, 0)) {
        LOG_ERROR("Error subscribing to one or more topics");
        mosquitto_disconnect(mosq);
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        lcd_show_message("Failed Topic");
        return EXIT_FAILURE;
    }
    LOG_INFO("Connected to the MQTT broker and subscribed to the topics:");
    LOG_INFO("%s", TOPIC_CMDS_ADC);
    LOG_INFO("%s", TOPIC_CMDS_GPIO_INPUT);
    LOG_INFO("%s", TOPIC_CMDS_GPIO_OUTPUT_GET);
    LOG_INFO("%s", TOPIC_CMDS_GPIO_OUTPUT_SET);
    LOG_INFO("%s", TOPIC_CMDS_MOTOR_GET);
    LOG_INFO("%s", TOPIC_CMDS_MOTOR_SET);
    LOG_INFO("%s", TOPIC_CMDS_SYSTEM);

    //mosquitto_loop(mosq, -1, 1);
    if (mosquitto_loop_start(mosq) != MOSQ_ERR_SUCCESS) {
       LOG_ERROR("Error Mosquitto loop");
       exit(EXIT_FAILURE);
    }

    mqtt_publish_async(mosq, TOPIC_LOGS, "Ready...");
    lcd_show_message("Ready...         ");

    // Espera mientras keep_running sea 1
    while (keep_running) {
        pause();  // Más eficiente que sleep(1), espera señales
    }

    mqtt_publish_async(mosq, TOPIC_LOGS, "Shutting down...");
    lcd_show_message("Shutting down...");

    sleep(LCD_TIME);

    pthread_mutex_lock(&adc_running_mutex);
    adc_running = TASK_STOPPED;
    pthread_mutex_unlock(&adc_running_mutex);

    pthread_mutex_lock(&gpio_input_running_mutex);
    gpio_input_running = TASK_STOPPED;
    pthread_mutex_unlock(&gpio_input_running_mutex);

    pthread_mutex_lock(&gpio_output_get_running_mutex);
    gpio_output_get_running = TASK_STOPPED;
    pthread_mutex_unlock(&gpio_output_get_running_mutex);

    pthread_mutex_lock(&gpio_output_set_running_mutex);
    gpio_output_set_running = TASK_STOPPED;
    pthread_mutex_unlock(&gpio_output_set_running_mutex);

    pthread_mutex_lock(&motor_get_running_mutex);
    motor_get_running = TASK_STOPPED;
    pthread_mutex_unlock(&motor_get_running_mutex);

    pthread_mutex_lock(&motor_set_running_mutex);
    motor_set_running = TASK_STOPPED;
    pthread_mutex_unlock(&motor_set_running_mutex);

    pthread_mutex_lock(&system_running_mutex);
    system_running = TASK_STOPPED;
    pthread_mutex_unlock(&system_running_mutex);

    // Apagar MQTT y recursos
    mosquitto_disconnect(mosq);
    mosquitto_loop_stop(mosq, false);  // false: no forzar parada abrupta
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    sleep(LCD_TIME);

    // Apagar PRU0
    if (control_pru(0, PRU0_PATH, PRU0_FIRMWARE) == 0) {
        lcd_show_message(MSG_LCD_PRU0_OFF_SUCCESS);
    } else {
        lcd_show_message(MSG_LCD_PRU0_OFF_ERROR);
    }
    // Apagar PRU1
    if (control_pru(0, PRU1_PATH, PRU1_FIRMWARE) == 0) {
        lcd_show_message(MSG_LCD_PRU1_OFF_SUCCESS);
    } else {
        lcd_show_message(MSG_LCD_PRU1_OFF_ERROR);
    }

    unmap_memory(fd_ram0, map_base_ram0, RAM0_SIZE);
    unmap_memory(fd_shared, map_base_shared, SHARED_SIZE);

    lcd_show_message("Finished        ");
    sleep(LCD_TIME);

    // Function queue
    task_queue_shutdown(&adc_queue);
    task_queue_shutdown(&gpio_input_queue);
    task_queue_shutdown(&gpio_output_get_queue);
    task_queue_shutdown(&gpio_output_set_queue);
    task_queue_shutdown(&motor_get_queue);
    task_queue_shutdown(&motor_set_queue);
    task_queue_shutdown(&system_queue);
    send_queue_shutdown(&mqtt_publish_queue);
    lcd_queue_shutdown(&lcd_display_queue);

    // Enviá señal de salida a hilos si corresponde (por ejemplo, con colas vacías especiales)
    pthread_join(adc_thread, NULL);
    pthread_join(gpio_input_thread, NULL);
    pthread_join(gpio_output_get_thread, NULL);
    pthread_join(gpio_output_set_thread, NULL);
    pthread_join(motor_get_thread, NULL);
    pthread_join(motor_set_thread, NULL);
    pthread_join(system_thread, NULL);
    pthread_join(mqtt_publish_thread, NULL);
    pthread_join(lcd_display_thread, NULL);

    task_queue_destroy(&adc_queue);
    task_queue_destroy(&gpio_input_queue);
    task_queue_destroy(&gpio_output_get_queue);
    task_queue_destroy(&gpio_output_set_queue);
    task_queue_destroy(&motor_get_queue);
    task_queue_destroy(&motor_set_queue);
    task_queue_destroy(&system_queue);

    lcd_queue_destroy(&lcd_display_queue);
    send_queue_destroy(&mqtt_publish_queue);

    pthread_mutex_destroy(&shm_ctx.ram0_mutex);
    // Destruir todos los mutexes de memoria compartida
    for (int i = 0; i < SHARED_MUTEX_COUNT; i++) {
        pthread_mutex_destroy(&shm_ctx.shared_mutex[i]);
    }
    pthread_mutex_destroy(&adc_running_mutex);
    pthread_mutex_destroy(&gpio_input_running_mutex);
    pthread_mutex_destroy(&gpio_output_get_running_mutex);
    pthread_mutex_destroy(&gpio_output_set_running_mutex);
    pthread_mutex_destroy(&motor_get_running_mutex);
    pthread_mutex_destroy(&motor_set_running_mutex);
    pthread_mutex_destroy(&system_running_mutex);

    LOG_INFO("System stopped");

    return 0;
}


