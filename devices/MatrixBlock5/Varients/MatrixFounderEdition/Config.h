// Define Device Specific Macro, Value and private function
#pragma once

#include "Family.h"

#define MODEL MXFE1


struct GPIO {
  GPIO_TypeDef* port;
  uint16_t pin;

  GPIO(GPIO_TypeDef* port, uint16_t pin) {
    this->port = port;
    this->pin = pin;
  }
};

#define FN_Pin GPIO_PIN_0
#define FN_GPIO_Port GPIOA

#define Key1_Pin GPIO_PIN_15
#define Key1_GPIO_Port GPIOB
#define Key2_Pin GPIO_PIN_14
#define Key2_GPIO_Port GPIOB
#define Key3_Pin GPIO_PIN_13
#define Key3_GPIO_Port GPIOB
#define Key4_Pin GPIO_PIN_12
#define Key4_GPIO_Port GPIOB
#define Key5_Pin GPIO_PIN_6
#define Key5_GPIO_Port GPIOC
#define Key6_Pin GPIO_PIN_15
#define Key6_GPIO_Port GPIOC
#define Key7_Pin GPIO_PIN_14
#define Key7_GPIO_Port GPIOC
#define Key8_Pin GPIO_PIN_13
#define Key8_GPIO_Port GPIOC

#define KeyRead1_Pin GPIO_PIN_1
#define KeyRead1_GPIO_Port GPIOB
#define KeyRead2_Pin GPIO_PIN_0
#define KeyRead2_GPIO_Port GPIOB
#define KeyRead3_Pin GPIO_PIN_2
#define KeyRead3_GPIO_Port GPIOA
#define KeyRead4_Pin GPIO_PIN_1
#define KeyRead4_GPIO_Port GPIOA
#define KeyRead5_Pin GPIO_PIN_3
#define KeyRead5_GPIO_Port GPIOC
#define KeyRead6_Pin GPIO_PIN_3
#define KeyRead6_GPIO_Port GPIOA
#define KeyRead7_Pin GPIO_PIN_5
#define KeyRead7_GPIO_Port GPIOC
#define KeyRead8_Pin GPIO_PIN_4
#define KeyRead8_GPIO_Port GPIOC

#define TouchData_Pin GPIO_PIN_6
#define TouchData_GPIO_Port GPIOA
#define TouchClock_Pin GPIO_PIN_7
#define TouchClock_GPIO_Port GPIOA

inline GPIO keypad_write_pins[] = {
    GPIO(Key1_GPIO_Port, Key1_Pin), GPIO(Key2_GPIO_Port, Key2_Pin), GPIO(Key3_GPIO_Port, Key3_Pin),
    GPIO(Key4_GPIO_Port, Key4_Pin), GPIO(Key5_GPIO_Port, Key5_Pin), GPIO(Key6_GPIO_Port, Key6_Pin),
    GPIO(Key7_GPIO_Port, Key7_Pin), GPIO(Key8_GPIO_Port, Key8_Pin),
};

inline GPIO keypad_read_pins[] = {
    GPIO(KeyRead1_GPIO_Port, KeyRead1_Pin), GPIO(KeyRead2_GPIO_Port, KeyRead2_Pin),
    GPIO(KeyRead3_GPIO_Port, KeyRead3_Pin), GPIO(KeyRead4_GPIO_Port, KeyRead4_Pin),
    GPIO(KeyRead5_GPIO_Port, KeyRead5_Pin), GPIO(KeyRead6_GPIO_Port, KeyRead6_Pin),
    GPIO(KeyRead7_GPIO_Port, KeyRead7_Pin), GPIO(KeyRead8_GPIO_Port, KeyRead8_Pin),
};

inline GPIO fn_pin = GPIO(FN_GPIO_Port, FN_Pin);

inline GPIO touch_clock_pin = GPIO(TouchClock_GPIO_Port, TouchClock_Pin);
inline GPIO touch_data_pin = GPIO(TouchData_GPIO_Port, TouchData_Pin);

inline uint8_t touchbar_map[16] = {4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 3};