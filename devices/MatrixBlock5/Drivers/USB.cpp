#include "Device.h"

namespace Device::USB
{
  void Init() {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Configure USB FS GPIOs */
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* Configure USB D+ D- Pins */
    GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // USB Clock enable
    __HAL_RCC_USB_CLK_ENABLE();
  }
}