#ifndef GPIO_OUTPUT_H
#define GPIO_OUTPUT_H

//OUTPUT
#define ERR_GPIO_OUTPUT_INVALID_OUTPUT               "Err: output fld "
#define ERR_GPIO_OUTPUT_INVALID_OUTPUT_SIZE          "Err: output size"
#define ERR_GPIO_OUTPUT_INVALID_OUTPUT_VALUE         "Err: output val "

// aqui no pude poner "Err: state field" mqtt da Decoded SparkplugB
#define ERR_GPIO_OUTPUT_INVALID_OUTPUT_STATE         "Err: state field"
#define ERR_GPIO_OUTPUT_INVALID_OUTPUT_STATE_VALUE   "Err: state value"

#define MSG_GPIO_OUTPUT_INVALID_MODE                 "Gpio out inv mod"
#define MSG_GPIO_OUTPUT_DONE                         "Gpio out done   "
#define MSG_GPIO_OUTPUT_STOPPED                      "Gpio out stopped"
#define MSG_GPIO_OUTPUT_TIMEOUT_EXPIRED              "Gpio out exp TO "
#define MSG_GPIO_OUTPUT_FINISH                       "Gpio out finish"

#define JSON_KEY_GPIO_OUTPUT_OUTPUT                  "output"
#define JSON_KEY_GPIO_OUTPUT_STATE                   "state"

#define GPIO_OUTPUT_CH_MIN              0
#define GPIO_OUTPUT_CH_MAX              3
#define GPIO_OUTPUT_NUM_MIN             1
#define GPIO_OUTPUT_NUM_MAX             4
#define GPIO_OUTPUT_LOW_LEVEL           0
#define GPIO_OUTPUT_HIGH_LEVEL          1

#define GPIO_OUTPUT_OFFSET_STATE        4

#endif
