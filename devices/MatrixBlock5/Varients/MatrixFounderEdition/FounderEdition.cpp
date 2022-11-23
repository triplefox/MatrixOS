// Define Device Specific Function
#include "Device.h"

namespace Device
{


  void TouchBar_Init() {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Configure TouchBar GPIOs */
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(TouchClock_GPIO_Port, TouchClock_Pin, GPIO_PIN_RESET);

    /*Configure GPIO pins : TouchClock_Pin */
    GPIO_InitStruct.Pin = TouchClock_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(TouchClock_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pin : TouchData_Pin */
    GPIO_InitStruct.Pin = TouchData_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(TouchData_GPIO_Port, &GPIO_InitStruct);
  }
}

