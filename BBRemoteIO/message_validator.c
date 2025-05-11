#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>

#include "mqtt_callback.h"
#include "message_validator.h"
#include "mqtt_publish.h"

#include "log.h"

#define ADC_SAMPLING_TABLE_SIZE         (sizeof(adc_sampling_table) / sizeof(adc_sampling_table[0]))

typedef struct {
    int frequency;
    int cycles; // representa el periodo de muestreo en ciclos de ejecucion del pru
} AdcSamplingEntry;

static const AdcSamplingEntry adc_sampling_table[] = {
    {40000, 1981}, {20000, 4480}, {10000, 9480}, {8000, 11982},
    {6000, 16150}, {4000, 24486}, {2000, 49480}, {1000, 99450},
    {800, 124482}, {600, 166150}, {400, 249480}, {200, 499580},
    {100, 999500}, {80, 1249500}, {60, 1666200}, {40, 2499900},
    {20, 4999000}, {10, 9995100}, {8, 12500000}, {6, 16667000},
    {4, 25000000}, {2, 49990000}, {1, 99990000}
};

//ADC
int validate_adc_message(struct mosquitto *mosq, const char *payload, AdcData *adc_data) {
    if (payload == NULL || adc_data == NULL) {
        mqtt_report(mosq, TOPIC_LOGS, ERR_NULL_PTR);
        return -1;
    }

    size_t len = strlen(payload);
    if (len < 2 || payload[0] != '{' || payload[len - 1] != '}') {
        mqtt_report(mosq, TOPIC_LOGS, ERR_INVALID_FORMAT);
        return -1;
    }

    cJSON *root = cJSON_Parse(payload);
    if (!root) {
        mqtt_report(mosq, TOPIC_LOGS, ERR_INVALID_JSON);
        return -1;
    }

    cJSON *task = cJSON_GetObjectItemCaseSensitive(root, JSON_KEY_TASK);
    if (!cJSON_IsString(task) || strcmp(task->valuestring, JSON_KEY_ADC_TASK_NAME) != 0) {
        mqtt_report(mosq, TOPIC_LOGS, ERR_INVALID_TASK);
        cJSON_Delete(root);
        return -1;
    }

    cJSON *sample_rate_item = cJSON_GetObjectItemCaseSensitive(root, JSON_KEY_ADC_SAMPLE_RATE);
    if (!cJSON_IsNumber(sample_rate_item)) {
        mqtt_report(mosq, TOPIC_LOGS, ERR_ADC_SAMPLE_RATE_OUT_OF_RANGE);
        cJSON_Delete(root);
        return -1;
    }

    int sample_rate = sample_rate_item->valueint;
    int found = 0;
    int cycles = 0;

    for (size_t i = 0; i < ADC_SAMPLING_TABLE_SIZE; ++i) {
        if (adc_sampling_table[i].frequency == sample_rate) {
            cycles = adc_sampling_table[i].cycles;
            found = 1;
            break;
        }
    }

    if (!found) {
        mqtt_publish_async(mosq, TOPIC_LOGS, ERR_ADC_SAMPLE_RATE_OUT_OF_RANGE);
        mqtt_report(mosq, TOPIC_LOGS, ERR_ADC_SAMPLE_RATE_OUT_OF_RANGE);
        cJSON_Delete(root);
        return -1;
    }

    adc_data->sample_rate = sample_rate;
    adc_data->sample_period = cycles; // recordar que sample_period no es una variable en ms sino en ciclos de pru

    // Calcular tamano minimo de buffer en base a la tasa de muestreo y duracion del proceso Linux
    int min_buffer_size = (int)(adc_data->sample_rate * PRU_LINUX_PROCESS);

    cJSON *buffer_size_item = cJSON_GetObjectItemCaseSensitive(root, JSON_KEY_ADC_BUFFER_SIZE);
    if (!cJSON_IsNumber(buffer_size_item) || buffer_size_item->valueint < ADC_BUFFER_SIZE_MIN ||
        buffer_size_item->valueint > ADC_BUFFER_SIZE_MAX || buffer_size_item->valueint % 4 != 0 ||
        buffer_size_item->valueint < min_buffer_size) {
        mqtt_report(mosq, TOPIC_LOGS, ERR_ADC_BUFFER_SIZE_OUT_OF_RANGE);
        cJSON_Delete(root);
        return -1;
    }
    adc_data->buffer_size = buffer_size_item->valueint;

    cJSON *num_samples_item = cJSON_GetObjectItemCaseSensitive(root, JSON_KEY_ADC_NUM_SAMPLES);
    int num_samples = num_samples_item ? num_samples_item->valueint : 0;  // toma valor entero
    int max_samples = ((ADC_NUM_SAMPLES_MAX / adc_data->buffer_size) * adc_data->buffer_size);

    if (!cJSON_IsNumber(num_samples_item) ||
        num_samples < adc_data->buffer_size ||
        num_samples > max_samples ||
        num_samples % adc_data->buffer_size != 0) {
        mqtt_report(mosq, TOPIC_LOGS, ERR_ADC_NUM_SAMPLES_OUT_OF_RANGE);
        cJSON_Delete(root);
        return -1;
    }

    adc_data->num_samples = num_samples;

    cJSON *trigger_item = cJSON_GetObjectItemCaseSensitive(root, JSON_KEY_ADC_ENABLE_TRIGGER);
    if (!cJSON_IsNumber(trigger_item) || (trigger_item->valueint != 0 && trigger_item->valueint != 1)) {
        mqtt_report(mosq, TOPIC_LOGS, ERR_ADC_INVALID_TRIGGER);
        cJSON_Delete(root);
        return -1;
    }
    adc_data->enable_external_trigger = trigger_item->valueint;

    cJSON *mode_item = cJSON_GetObjectItemCaseSensitive(root, JSON_KEY_MODE);
    if (!cJSON_IsNumber(mode_item) || (mode_item->valueint < ADC_MODE_MIN || mode_item->valueint > ADC_MODE_MAX)) {
        mqtt_report(mosq, TOPIC_LOGS, ERR_INVALID_MODE);
        cJSON_Delete(root);
        return -1;
    }

    adc_data->mode = mode_item->valueint;

    cJSON_Delete(root);
    return 0;
}


//GPIO INPUT
int validate_gpio_input_message(struct mosquitto *mosq, const char *payload, GpioInputData *gpio_input_data) {

    if (payload == NULL || gpio_input_data == NULL) {
        mqtt_report(mosq, TOPIC_LOGS, ERR_NULL_PTR);
        return -1;
    }

    size_t len = strlen(payload);
    if (len < 2 || payload[0] != '{' || payload[len - 1] != '}') {
        mqtt_report(mosq, TOPIC_LOGS, ERR_INVALID_FORMAT);
        return -1;
    }
    cJSON *root = cJSON_Parse(payload);
    if (!root) {
        mqtt_report(mosq, TOPIC_LOGS, ERR_INVALID_JSON);
        return -1;
    }

    cJSON *task = cJSON_GetObjectItemCaseSensitive(root, JSON_KEY_TASK);
    if (!cJSON_IsString(task) || strcmp(task->valuestring, JSON_KEY_GPIO_INPUT_TASK_NAME) != 0) {
        mqtt_report(mosq, TOPIC_LOGS, ERR_INVALID_TASK);
        cJSON_Delete(root);
        return -1;
    }

    cJSON *input_array = cJSON_GetObjectItemCaseSensitive(root, JSON_KEY_GPIO_INPUT_INPUT);
    if (!cJSON_IsArray(input_array)) {
        mqtt_report(mosq, TOPIC_LOGS, ERR_GPIO_INPUT_INVALID_INPUT);
        cJSON_Delete(root);
        return -1;
    }

    int num_input = cJSON_GetArraySize(input_array);
    if (num_input < GPIO_INPUT_NUM_MIN || num_input > GPIO_INPUT_NUM_MAX) {
        mqtt_report(mosq, TOPIC_LOGS, ERR_GPIO_INPUT_INPUT_OUT_OF_RANGE);
        cJSON_Delete(root);
        return -1;
    }

    for (int i = 0; i < num_input; i++) {
        cJSON *pin_item = cJSON_GetArrayItem(input_array, i);
        if (!cJSON_IsNumber(pin_item) || pin_item->valueint < GPIO_INPUT_CH_MIN || pin_item->valueint > GPIO_INPUT_CH_MAX) {
            mqtt_report(mosq, TOPIC_LOGS, ERR_GPIO_INPUT_INVALID_INPUT_VALUE);
            cJSON_Delete(root);
            return -1;
        }
        gpio_input_data->input[i] = pin_item->valueint;
    }

    gpio_input_data->num_input = num_input;

    cJSON *mode_item = cJSON_GetObjectItemCaseSensitive(root, JSON_KEY_MODE);
    if (!cJSON_IsNumber(mode_item) || mode_item->valueint < GPIO_INPUT_MODE_MIN || mode_item->valueint > GPIO_INPUT_MODE_MAX) {
         mqtt_report(mosq, TOPIC_LOGS, ERR_INVALID_MODE);
         cJSON_Delete(root);
         return -1;
    }

    gpio_input_data->mode = mode_item->valueint;

    cJSON_Delete(root);
    return 0;
}

// GPIO OUTPUT GET
int validate_gpio_output_get_message(struct mosquitto *mosq, const char *payload, GpioOutputGetData *gpio_output_get_data) {

    if (payload == NULL || gpio_output_get_data == NULL) {
        mqtt_report(mosq, TOPIC_LOGS, ERR_NULL_PTR);
        return -1;
    }

    size_t len = strlen(payload);
    if (len < 2 || payload[0] != '{' || payload[len - 1] != '}') {
        mqtt_report(mosq, TOPIC_LOGS, ERR_INVALID_FORMAT);
        return -1;
    }
    cJSON *root = cJSON_Parse(payload);
    if (!root) {
        mqtt_report(mosq, TOPIC_LOGS, ERR_INVALID_JSON);
        return -1;
    }

  // Verificar el campo "task" Y que valga gpio_output_get
  cJSON *task = cJSON_GetObjectItemCaseSensitive(root, JSON_KEY_TASK);
  if (!cJSON_IsString(task) || strcmp(task->valuestring, JSON_KEY_GPIO_OUTPUT_GET_TASK_NAME) != 0) {
    mqtt_report(mosq, TOPIC_LOGS, ERR_INVALID_TASK);
    cJSON_Delete(root);
    return -1;
  }

  // Verificar el campo "output"
  cJSON *output_array = cJSON_GetObjectItemCaseSensitive(root, JSON_KEY_GPIO_OUTPUT_OUTPUT);
  if (!cJSON_IsArray(output_array)) {
    mqtt_report(mosq, TOPIC_LOGS, ERR_GPIO_OUTPUT_INVALID_OUTPUT);
    cJSON_Delete(root);
    return -1;
  }

  int num_output = cJSON_GetArraySize(output_array);
  if (num_output < GPIO_OUTPUT_NUM_MIN || num_output > GPIO_OUTPUT_NUM_MAX) {
    mqtt_report(mosq, TOPIC_LOGS, ERR_GPIO_OUTPUT_INVALID_OUTPUT_SIZE);
    cJSON_Delete(root);
    return -1;
  }

  for (int i = 0; i < num_output; i++) {
    cJSON *pin_item = cJSON_GetArrayItem(output_array, i);
    if (!cJSON_IsNumber(pin_item) || pin_item->valueint < GPIO_OUTPUT_CH_MIN || pin_item->valueint > GPIO_OUTPUT_CH_MAX) {
        mqtt_report(mosq, TOPIC_LOGS, ERR_GPIO_OUTPUT_INVALID_OUTPUT_VALUE);
        cJSON_Delete(root);
        return -1;
    }
    gpio_output_get_data->output[i] = pin_item->valueint;
  }

  gpio_output_get_data->num_output = num_output;

  // Verificar el campo "mode"
  cJSON *mode_item = cJSON_GetObjectItemCaseSensitive(root, JSON_KEY_MODE);
  if (!cJSON_IsNumber(mode_item) || mode_item->valueint < GPIO_OUTPUT_GET_MODE_MIN || mode_item->valueint > GPIO_OUTPUT_GET_MODE_MAX) {
       mqtt_report(mosq, TOPIC_LOGS, ERR_INVALID_MODE);
       cJSON_Delete(root);
       return -1;
  }

  gpio_output_get_data->mode = mode_item->valueint;

  cJSON_Delete(root);
  return 0; //exito
}

// GPIO OUTPUT SET
int validate_gpio_output_set_message(struct mosquitto *mosq, const char *payload, GpioOutputSetData *gpio_output_set_data) {

    if (payload == NULL || gpio_output_set_data == NULL) {
        mqtt_report(mosq, TOPIC_LOGS, ERR_NULL_PTR);
        return -1;
    }

    size_t len = strlen(payload);
    if (len < 2 || payload[0] != '{' || payload[len - 1] != '}') {
        mqtt_report(mosq, TOPIC_LOGS, ERR_INVALID_FORMAT);
        return -1;
    }
    cJSON *root = cJSON_Parse(payload);
    if (!root) {
        mqtt_report(mosq, TOPIC_LOGS, ERR_INVALID_JSON);
        return -1;
    }

  // Verificar el campo "task" Y que valga gpio_output_set
  cJSON *task = cJSON_GetObjectItemCaseSensitive(root, JSON_KEY_TASK);
  if (!cJSON_IsString(task) || strcmp(task->valuestring, JSON_KEY_GPIO_OUTPUT_SET_TASK_NAME) != 0) {
    mqtt_report(mosq, TOPIC_LOGS, ERR_INVALID_TASK);
    cJSON_Delete(root);
    return -1;
  }

  // Verificar el campo "output"
  cJSON *output_array = cJSON_GetObjectItemCaseSensitive(root, JSON_KEY_GPIO_OUTPUT_OUTPUT);
  if (!cJSON_IsArray(output_array)) {
    mqtt_report(mosq, TOPIC_LOGS, ERR_GPIO_OUTPUT_INVALID_OUTPUT);
    cJSON_Delete(root);
    return -1;
  }

  int num_output = cJSON_GetArraySize(output_array);
  if (num_output < GPIO_OUTPUT_NUM_MIN || num_output > GPIO_OUTPUT_NUM_MAX) {
    mqtt_report(mosq, TOPIC_LOGS, ERR_GPIO_OUTPUT_INVALID_OUTPUT_SIZE);
    cJSON_Delete(root);
    return -1;
  }

  for (int i = 0; i < num_output; i++) {
    cJSON *pin_item = cJSON_GetArrayItem(output_array, i);
    if (!cJSON_IsNumber(pin_item) || pin_item->valueint < GPIO_OUTPUT_CH_MIN || pin_item->valueint > GPIO_OUTPUT_CH_MAX) {
        mqtt_report(mosq, TOPIC_LOGS, ERR_GPIO_OUTPUT_INVALID_OUTPUT_VALUE);
        cJSON_Delete(root);
        return -1;
    }
    gpio_output_set_data->output[i] = pin_item->valueint;
  }

  gpio_output_set_data->num_output = num_output;

  // Verificar el campo "state"
  cJSON *state_array = cJSON_GetObjectItemCaseSensitive(root, JSON_KEY_GPIO_OUTPUT_STATE);
  if (!cJSON_IsArray(state_array) || cJSON_GetArraySize(state_array) != num_output) {
      mqtt_report(mosq, TOPIC_LOGS, ERR_GPIO_OUTPUT_INVALID_OUTPUT_STATE);
      cJSON_Delete(root);
      return -1;
  }

  for (int i = 0; i < num_output; i++) {
      cJSON *state_item = cJSON_GetArrayItem(state_array, i);
      if (!cJSON_IsNumber(state_item) || (state_item->valueint != GPIO_OUTPUT_LOW_LEVEL && state_item->valueint != GPIO_OUTPUT_HIGH_LEVEL)) {
          mqtt_report(mosq, TOPIC_LOGS, ERR_GPIO_OUTPUT_INVALID_OUTPUT_STATE_VALUE);
          cJSON_Delete(root);
          return -1;
      }
      gpio_output_set_data->state[i] = state_item->valueint;
  }

  // Verificar el campo "mode"
  cJSON *mode_item = cJSON_GetObjectItemCaseSensitive(root, JSON_KEY_MODE);
  if (!cJSON_IsNumber(mode_item) || mode_item->valueint < GPIO_OUTPUT_SET_MODE_MIN || mode_item->valueint > GPIO_OUTPUT_SET_MODE_MAX) {
       mqtt_report(mosq, TOPIC_LOGS, ERR_INVALID_MODE);
       cJSON_Delete(root);
       return -1;
  }

  gpio_output_set_data->mode = mode_item->valueint;

  cJSON_Delete(root);
  return 0; //exito
}

// MOTOR GET
int validate_motor_get_message(struct mosquitto *mosq, const char *payload, MotorGetData *motor_get_data) {

    if (payload == NULL || motor_get_data == NULL) {
        mqtt_report(mosq, TOPIC_LOGS, ERR_NULL_PTR);
        return -1;
    }

    size_t len = strlen(payload);
    if (len < 2 || payload[0] != '{' || payload[len - 1] != '}') {
        mqtt_report(mosq, TOPIC_LOGS, ERR_INVALID_FORMAT);
        return -1;
    }
    cJSON *root = cJSON_Parse(payload);
    if (!root) {
        mqtt_report(mosq, TOPIC_LOGS, ERR_INVALID_JSON);
        return -1;
    }

  // Verificar si el campo "task" existe y es "motor_get"
  cJSON *task = cJSON_GetObjectItemCaseSensitive(root, JSON_KEY_TASK);
  if (!cJSON_IsString(task) || strcmp(task->valuestring, JSON_KEY_MOTOR_GET_TASK_NAME) != 0) {
    mqtt_report(mosq, TOPIC_LOGS, ERR_INVALID_TASK);
    cJSON_Delete(root);
    return -1;
  }

  // Obtener el arreglo "motor"
  cJSON *motor_array = cJSON_GetObjectItemCaseSensitive(root, JSON_KEY_MOTOR);
  if (!cJSON_IsArray(motor_array)) {
    mqtt_report(mosq, TOPIC_LOGS, ERR_MOTOR_ARRAY);
    cJSON_Delete(root);
    return -1;
  }

  int num_motor = cJSON_GetArraySize(motor_array);
  if (num_motor < MOTOR_NUM_MIN || num_motor > MOTOR_NUM_MAX) {
    mqtt_report(mosq, TOPIC_LOGS, ERR_MOTOR_COUNT);
    cJSON_Delete(root);
    return -1;
  }

  motor_get_data->num_motor = num_motor;

  // Leer valores del arreglo "motor"
  for (int i = 0; i < num_motor; i++) {
    cJSON *motor_item = cJSON_GetArrayItem(motor_array, i);
    if (!cJSON_IsNumber(motor_item) || motor_item->valueint < MOTOR_CH_MIN || motor_item->valueint > MOTOR_CH_MAX) {
        mqtt_report(mosq, TOPIC_LOGS, ERR_MOTOR_VALUE);
        cJSON_Delete(root);
        return -1;
    }
    motor_get_data->motor[i] = motor_item->valueint;
  }

  // Verificar el campo "mode"
  cJSON *mode_item = cJSON_GetObjectItemCaseSensitive(root, JSON_KEY_MODE);
  if (!cJSON_IsNumber(mode_item) || mode_item->valueint < MOTOR_GET_MODE_MIN || mode_item->valueint > MOTOR_GET_MODE_MAX) {
       mqtt_report(mosq, TOPIC_LOGS, ERR_INVALID_MODE);
       cJSON_Delete(root);
       return -1;
  }

  motor_get_data->mode = mode_item->valueint;

  cJSON_Delete(root);
  return 0;
}

// MOTOR SET
int validate_motor_set_message(struct mosquitto *mosq, const char *payload, MotorSetData *motor_set_data) {
    if (payload == NULL || motor_set_data == NULL) {
        mqtt_report(mosq, TOPIC_LOGS, ERR_NULL_PTR);
        return -1;
    }

    size_t len = strlen(payload);
    if (len < 2 || payload[0] != '{' || payload[len - 1] != '}') {
        mqtt_report(mosq, TOPIC_LOGS, ERR_INVALID_FORMAT);
        return -1;
    }
    cJSON *root = cJSON_Parse(payload);
    if (!root) {
        mqtt_report(mosq, TOPIC_LOGS, ERR_INVALID_JSON);
        return -1;
    }

    // Verificar si el campo "task" existe y es "motor_set"
    cJSON *task = cJSON_GetObjectItemCaseSensitive(root, JSON_KEY_TASK);
    if (!cJSON_IsString(task) || strcmp(task->valuestring, JSON_KEY_MOTOR_SET_TASK_NAME) != 0) {
        mqtt_report(mosq, TOPIC_LOGS, ERR_INVALID_TASK);
        cJSON_Delete(root);
        return -1;
    }

    cJSON *motor_array = cJSON_GetObjectItemCaseSensitive(root, JSON_KEY_MOTOR);
    cJSON *ena_array = cJSON_GetObjectItemCaseSensitive(root, JSON_KEY_MOTOR_ENA);
    cJSON *dir_array = cJSON_GetObjectItemCaseSensitive(root, JSON_KEY_MOTOR_DIR);
    cJSON *rpm_array = cJSON_GetObjectItemCaseSensitive(root, JSON_KEY_MOTOR_RPM);
    cJSON *steps_array = cJSON_GetObjectItemCaseSensitive(root, JSON_KEY_MOTOR_STEP_PER_REV);
    cJSON *micro_step_array = cJSON_GetObjectItemCaseSensitive(root, JSON_KEY_MOTOR_MICRO_STEP);
    cJSON *mode_item = cJSON_GetObjectItemCaseSensitive(root, JSON_KEY_MODE);

    if (!cJSON_IsArray(motor_array) || !cJSON_IsArray(ena_array) || !cJSON_IsArray(dir_array) ||
        !cJSON_IsArray(rpm_array) || !cJSON_IsArray(steps_array) || !cJSON_IsArray(micro_step_array)) {
        mqtt_report(mosq, TOPIC_LOGS, ERR_MOTOR_FIELDS);
        cJSON_Delete(root);
        return -1;
    }

    int num_motor = cJSON_GetArraySize(motor_array);
    if (num_motor < MOTOR_NUM_MIN || num_motor > MOTOR_NUM_MAX ||
        num_motor != cJSON_GetArraySize(ena_array) ||
        num_motor != cJSON_GetArraySize(dir_array) ||
        num_motor != cJSON_GetArraySize(rpm_array) ||
        num_motor != cJSON_GetArraySize(steps_array) ||
        num_motor != cJSON_GetArraySize(micro_step_array)) {
        mqtt_report(mosq, TOPIC_LOGS, ERR_MOTOR_ARRAY_SIZE);
        cJSON_Delete(root);
        return -1;
    }

    motor_set_data->num_motor = num_motor;

    for (int i = 0; i < num_motor; i++) {
        cJSON *motor_item = cJSON_GetArrayItem(motor_array, i);
        cJSON *ena_item = cJSON_GetArrayItem(ena_array, i);
        cJSON *dir_item = cJSON_GetArrayItem(dir_array, i);
        cJSON *rpm_item = cJSON_GetArrayItem(rpm_array, i);
        cJSON *steps_item = cJSON_GetArrayItem(steps_array, i);
        cJSON *micro_step_item = cJSON_GetArrayItem(micro_step_array, i);

        if (!cJSON_IsNumber(motor_item) || motor_item->valueint < MOTOR_CH_MIN || motor_item->valueint > MOTOR_CH_MAX ||
            !cJSON_IsNumber(ena_item) || (ena_item->valueint != MOTOR_ENA_DISABLE && ena_item->valueint != MOTOR_ENA_ENABLE) ||
            !cJSON_IsNumber(dir_item) || (dir_item->valueint != MOTOR_DIR0 && dir_item->valueint != MOTOR_DIR1) ||
            !cJSON_IsNumber(rpm_item) || rpm_item->valueint < MOTOR_RPM_MIN || rpm_item->valueint > MOTOR_RPM_MAX ||
            !cJSON_IsNumber(steps_item) || steps_item->valueint < MOTOR_STEP_PER_REV_MIN || steps_item->valueint > MOTOR_STEP_PER_REV_MAX ||
            !cJSON_IsNumber(micro_step_item)) {
            mqtt_report(mosq, TOPIC_LOGS,ERR_MOTOR_FIELDS);
            cJSON_Delete(root);
            return -1;
        }

	// Validar micro_step permitido
	int micro_step_value = micro_step_item->valueint;
	if (micro_step_value != 1 && micro_step_value != 2 &&
    	    micro_step_value != 4 && micro_step_value != 8 &&
            micro_step_value != 16) {
            mqtt_report(mosq, TOPIC_LOGS, ERR_MOTOR_INVALID_MICRO_STEP);
            cJSON_Delete(root);
            return -1;
         }

        int id = motor_item->valueint;
        motor_set_data->motor[i] = id;
        motor_set_data->ena[i] = ena_item->valueint;
        motor_set_data->dir[i] = dir_item->valueint;
        motor_set_data->rpm[i] = rpm_item->valueint;
        motor_set_data->step_per_rev[i] = steps_item->valueint;
        motor_set_data->micro_step[i] = micro_step_value;

        // Calculo de factor_step_time para cada motor
        int rpm = motor_set_data->rpm[i];
        int steps_per_rev = motor_set_data->step_per_rev[i];
        int microstep = motor_set_data->micro_step[i];

        int step_product = rpm * steps_per_rev * microstep;

        // Validar que el producto no supere el limite
        if (step_product > MOTOR_MAX_FIELD_SIZE) {
            mqtt_report(mosq, TOPIC_LOGS, ERR_MOTOR_INVALID_FIELD_FACTOR);
            cJSON_Delete(root);
            return -1;
        }

        float total_steps_per_minute = (float)(rpm * steps_per_rev * microstep);
        float steps_per_second = total_steps_per_minute / 60.0f; // Hz

        if (steps_per_second < 1.0f) {
            steps_per_second = 1.0f;  // evitar division por cero
            mqtt_report(mosq, TOPIC_LOGS, ERR_MOTOR_STEPS_DIV_ZERO);
        }

        //LOG_DEBUG(MOTOR_STEPS_PER_SECOND_DEBUG, steps_per_second);

        float steps_per_second_ms = 1000.0f / steps_per_second;
        float factor_step_time_f = steps_per_second_ms / MOTOR_FACTOR_STEP_TIME;

        // Redondeo a entero
        int factor_step_time = (int)(factor_step_time_f + 0.5f);
        motor_set_data->factor_step_time[i] = factor_step_time;

    }

    if (!cJSON_IsNumber(mode_item) || mode_item->valueint < MOTOR_SET_MODE_MIN || mode_item->valueint > MOTOR_SET_MODE_MAX) {
        mqtt_report(mosq, TOPIC_LOGS, ERR_INVALID_MODE);
        cJSON_Delete(root);
        return -1;
    }

    motor_set_data->mode = mode_item->valueint;

    cJSON_Delete(root);
    return 0;
}

//SYSTEM
int validate_system_message(struct mosquitto *mosq, const char *payload, SystemData *system_data) {

    if (payload == NULL || system_data == NULL) {
        mqtt_report(mosq, TOPIC_LOGS, ERR_NULL_PTR);
        return -1;
    }

    size_t len = strlen(payload);
    if (len < 2 || payload[0] != '{' || payload[len - 1] != '}') {
        mqtt_report(mosq, TOPIC_LOGS, ERR_INVALID_FORMAT);
        return -1;
    }
    cJSON *root = cJSON_Parse(payload);
    if (!root) {
        mqtt_report(mosq, TOPIC_LOGS, ERR_INVALID_JSON);
        return -1;
    }

    cJSON *task = cJSON_GetObjectItemCaseSensitive(root, JSON_KEY_TASK);
    if (!cJSON_IsString(task) || strcmp(task->valuestring, JSON_KEY_SYSTEM_TASK_MANE) != 0) {
        mqtt_report(mosq, TOPIC_LOGS, ERR_INVALID_TASK);
        cJSON_Delete(root);
        return -1;
    }

    cJSON *function = cJSON_GetObjectItemCaseSensitive(root, JSON_KEY_SYSTEM_FUNCTION);
    if (!cJSON_IsString(function)) {
        mqtt_report(mosq, TOPIC_LOGS, ERR_SYSTEM_INVALID_FUNCTION);
        cJSON_Delete(root);
        return -1;
    }
    const char *func_value = function->valuestring;

    // Mapeo de funciones del sistema
    if      (strcmp(func_value, JSON_KEY_SYSTEM_LCD)                == 0) { system_data->function = FUNC_LCD; }
    else if (strcmp(func_value, JSON_KEY_SYSTEM_PRU0)               == 0) { system_data->function = FUNC_PRU0; }
    else if (strcmp(func_value, JSON_KEY_SYSTEM_PRU1)               == 0) { system_data->function = FUNC_PRU1; }
    else if (strcmp(func_value, JSON_KEY_SYSTEM_ADC)                == 0) { system_data->function = FUNC_ADC; }
    else if (strcmp(func_value, JSON_KEY_SYSTEM_GPIO_INPUT)         == 0) { system_data->function = FUNC_GPIO_INPUT; }
    else if (strcmp(func_value, JSON_KEY_SYSTEM_GPIO_OUTPUT_GET)    == 0) { system_data->function = FUNC_GPIO_OUTPUT_GET; }
    else if (strcmp(func_value, JSON_KEY_SYSTEM_GPIO_OUTPUT_SET)    == 0) { system_data->function = FUNC_GPIO_OUTPUT_SET; }
    else if (strcmp(func_value, JSON_KEY_SYSTEM_MOTOR_GET)          == 0) { system_data->function = FUNC_MOTOR_GET; }
    else if (strcmp(func_value, JSON_KEY_SYSTEM_MOTOR_SET)          == 0) { system_data->function = FUNC_MOTOR_SET; }
    else if (strcmp(func_value, JSON_KEY_SYSTEM_ALL_FUNCTIONS)      == 0) { system_data->function = FUNC_ALL_FUNCTIONS; }
    else {
        mqtt_report(mosq, TOPIC_LOGS, ERR_SYSTEM_INVALID_FUNCTION_VALUE);
        system_data->function = FUNC_UNKNOWN; // Asigna FUNC_UNKNOWN a valores no validos
        cJSON_Delete(root);
        return -1;
    }

    cJSON *action = cJSON_GetObjectItemCaseSensitive(root, JSON_KEY_SYSTEM_ACTION);
    if (!cJSON_IsString(action)) {
       mqtt_report(mosq, TOPIC_LOGS, ERR_SYSTEM_INVALID_ACTION);
       cJSON_Delete(root);
       return -1;
    }

    const char *action_value = action->valuestring;

    if (system_data->function == FUNC_LCD && strcmp(action_value, JSON_KEY_SYSTEM_CLEAN) == 0) {
        system_data->action = ACTION_CLEAN;
    }
    else if ((system_data->function == FUNC_PRU0 || system_data->function == FUNC_PRU1) &&
         (strcmp(action_value, JSON_KEY_SYSTEM_START) == 0 ||
          strcmp(action_value, JSON_KEY_SYSTEM_STOP) == 0  ||
          strcmp(action_value, JSON_KEY_SYSTEM_PAUSE) == 0)) {

      if (strcmp(action_value, JSON_KEY_SYSTEM_START) == 0) {
        system_data->action = ACTION_START;
      } else if (strcmp(action_value, JSON_KEY_SYSTEM_STOP) == 0) {
        system_data->action = ACTION_STOP;
      } else {
        system_data->action = ACTION_PAUSE;
      }
    }
    else if ((system_data->function == FUNC_ADC         ||
          system_data->function == FUNC_GPIO_INPUT      ||
          system_data->function == FUNC_GPIO_OUTPUT_GET ||
          system_data->function == FUNC_GPIO_OUTPUT_SET ||
          system_data->function == FUNC_MOTOR_GET       ||
          system_data->function == FUNC_MOTOR_SET       ||
          system_data->function == FUNC_ALL_FUNCTIONS) &&
         strcmp(action_value, JSON_KEY_SYSTEM_STOP) == 0) {
      system_data->action = ACTION_STOP;
    }
    else {
        mqtt_report(mosq, TOPIC_LOGS, ERR_SYSTEM_INVALID_ACTION_VALUE);
        cJSON_Delete(root);
        return -1;
    }

    cJSON_Delete(root);
    return 0;
}

void mqtt_report(struct mosquitto *mosq, const char *topic, const char *message) {
    char mqtt_message[MQTT_REPORT_JSON_SIZE];
    snprintf(mqtt_message, sizeof(mqtt_message), MQTT_REPORT_JSON_FORMAT, message);
    mqtt_publish_async(mosq, topic, mqtt_message);

    lcd_show_message(message);
    LOG_ERROR("%s", message);
}
