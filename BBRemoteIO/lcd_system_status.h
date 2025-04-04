#ifndef LCD_SYSTEM_STATUS_H
#define LCD_SYSTEM_STATUS_H

#include "i2c_lcd.h"
// completar simpre los 16 caracteres

#define LINE2                  2
#define LCD_OFFSET             1

#define LCD_FUNCTION_TIMEOUT                   1

#define LCD_PRU0_ERROR			       "PRU0 firmware E"
#define LCD_PRU0_START                         "PRU0 online    "
#define LCD_PRU0_STOP                          "PRU0 offline   "
#define LCD_PRU1_ERROR                         "PRU1 firmware E"
#define LCD_PRU1_START                         "PRU1 online    "
#define LCD_PRU1_STOP                          "PRU1 offline   "

#define LCD_SYSTEM_RUN			       "Running        "
#define LCD_SYSTEM_STOP			       "Stopped        "

#define LCD_MQTT_CLIENT_ERROR		       "Mqtt client E  "
#define LCD_MQTT_CONNECT_ERROR		       "Mqtt conn E    "
#define LCD_MQTT_TOPIC_ERROR		       "Mqtt topic E   "

#define LCD_MQTT_MSG_RECEIVED_ERROR	       "Mqtt msg E     "
#define LCD_MQTT_MSG_RECEIVED_OK	       "Mqtt msg OK    "
#define LCD_MQTT_MSG_RECEIVED_ADC_BUSY         "Adc busy       "
#define LCD_MQTT_MSG_RECEIVED_GPIO_INPUT_BUSY  "Input busy     "
#define LCD_MQTT_MSG_RECEIVED_GPIO_OUTPUT_BUSY "Output busy    "
#define LCD_MQTT_MSG_RECEIVED_MOTOR_BUSY       "Motor busy     "
#define LCD_MQTT_MSG_RECEIVED_SYSTEM_BUSY      "System busy    "

#define LCD_ADC_RUNNING 		       "Adc running    "
#define LCD_ADC_STATUS_ERROR  	               "Adc Err        "
#define LCD_ADC_STATUS_OK                      "Adc done       "
#define LCD_ADC_STATUS_FINNISH                 "Adc In stop    "

#define LCD_GPIO_INPUT_MODE1_RUNNING	       "Gpio In running"
#define LCD_GPIO_INPUT_STATUS_ERROR            "Gpio In Err    "
#define LCD_GPIO_INPUT_STATUS_MODE0_OK         "Gpio In done   "
#define LCD_GPIO_INPUT_STATUS_MODE1_OK         "Gpio In sending"
#define LCD_GPIO_INPUT_STATUS_FINNISH	       "Gpio In stop   "

#define LCD_GPIO_OUTPUT_RUNNING		       "Gpio Out run   "
#define LCD_GPIO_OUTPUT_STATUS_ERROR           "Gpio Out Err   "
#define LCD_GPIO_OUTPUT_STATUS_OK              "Gpio Out done  "
#define LCD_GPIO_OUTPUT_STATUS_FINNISH         "Gpio Out stop  "

#define LCD_MOTOR_MODE1_RUNNING		       "Motor running  "
#define LCD_MOTOR_STATUS_ERROR                 "Motor Err      "
#define LCD_MOTOR_STATUS_OK                    "Motor done     "
#define LCD_MOTOR_STATUS_MODE1_OK              "Motor sending  "
#define LCD_MOTOR_STATUS_FINNISH               "Motor stop     "

#define LCD_SYSTEM_CLEAN		       "Sys lcd clean  "
#define LCD_SYSTEM_RUNNING   		       "System running "
#define LCD_SYSTEM_STATUS_ERROR                "System Err     "
#define LCD_SYSTEM_STATUS_OK                   "System done    "
#define LCD_FUNCTION_ALL_STATUS_FINNISH        "Functions stop "

int lcd_system_init(void);

void lcd_system_end(void);

void lcd_system_clear_screen(void);

// Declaracion de la funcion lcd_systemstatus
void lcd_system_status(const char* status_message);

#endif //LCD_SYSTEM_STATUS_H
