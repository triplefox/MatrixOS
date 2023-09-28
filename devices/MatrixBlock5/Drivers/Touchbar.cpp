#include "Device.h"
#include "timers.h"

namespace Device::KeyPad
{
  StaticTimer_t touchbar_timer_def;
  TimerHandle_t touchbar_timer;

  void TouchBarScanTimerCallback(TimerHandle_t xTimer) {
    if(touchbar_enable) ScanTouchBar();
  }

  void InitTouchbar() {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(TouchClock_GPIO_Port, TouchClock_Pin, GPIO_PIN_RESET);

    /*Configure GPIO pins : TouchData_Pin */
    GPIO_InitStruct.Pin = TouchData_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(TouchClock_GPIO_Port, &GPIO_InitStruct);

    for (uint8_t x = 0; x < x_size; x++)
    {
      touchbarState[x].setConfig(&keyinfo_config);
    }
  }

  void StartTouchBar() {
    touchbar_timer = xTimerCreateStatic(NULL, configTICK_RATE_HZ / Device::touchbar_scanrate, true, NULL, TouchBarScanTimerCallback, &touchbar_timer_def);
    xTimerStart(touchbar_timer, 0);
  }

  bool ScanTouchBar() {
    for (uint8_t x = 0; x < 8; x++)
    {
      bool reading = false;
      for (uint8_t i = 0; i < 2; i++)
      {
        volatile int a;

        HAL_GPIO_WritePin(TouchClock_GPIO_Port, TouchClock_Pin, GPIO_PIN_SET);

        for (a = 0; a < 5; ++a)
        {}  // Add small delay
        reading = reading | HAL_GPIO_ReadPin(TouchData_GPIO_Port, TouchData_Pin);

        HAL_GPIO_WritePin(TouchClock_GPIO_Port, TouchClock_Pin, GPIO_PIN_RESET);

        for (a = 0; a < 5; ++a)
        {}  // Add small delay
      }

      Fract16 reading16 = reading * UINT16_MAX;
      bool updated = touchbarState[x].update(reading16, false);
      if (updated)
      {
        uint16_t keyID = (2 << 12) + touchbar_map[x * 2] / 2;
        if (NotifyOS(keyID, &touchbarState[x]))
        {
          return true;
        }
      }
    }
    return false;
  }
}