#include "Device.h"

TIM_HandleTypeDef htim8;
DMA_HandleTypeDef hdma_tim8_ch2;

namespace Device
{
  namespace LED
  {
    void Init() {
      MX_DMA_Init();
      MX_TIM8_Init();
      WS2812::Init(&htim8, TIM_CHANNEL_2, numsOfLED);
    }

    void Start() {}

    void Update(Color* frameBuffer, uint8_t brightness)  // Render LED
    {
      WS2812::Show(frameBuffer, brightness);
    }

    uint16_t XY2Index(Point xy) {
      if (xy.x >= 0 && xy.x < 8 && xy.y >= 0 && xy.y < 8)  // Main grid
      { return xy.x + xy.y * 8; }
      return UINT16_MAX;
    }

    // Matrix use the following ID Struct
    //  CCCC IIIIIIIIIIII
    //  C as class (4 bits), I as index (12 bits). I could be spilted by the class defination, for example, class 0
    //  (grid), it's spilted to XXXXXXX YYYYYYY. Class List: Class 0 - System - IIIIIIIIIIII Class 1 - Grid - XXXXXX
    //  YYYYYY Class 2 - TouchBar - IIIIIIIIIIII Class 3 - Underglow - IIIIIIIIIIII

    uint16_t ID2Index(uint16_t ledID) {
      uint8_t ledClass = ledID >> 12;
      switch (ledClass)
      {
        case 1:  // Main Grid
        {
          uint16_t index = ledID & (0b0000111111111111);
          if (index < 64)
            return index;
          break;
        }
        case 3:   // Underglow
          break;  // TODO: Underglow
      }
      return UINT16_MAX;
    }
  }
}

extern "C" {
void MX_DMA_Init(void) {
    /* DMA controller clock enable */
    __HAL_RCC_DMA2_CLK_ENABLE();

    /* DMA interrupt init */
    /* DMA2_Channel4_5_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA2_Channel4_5_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA2_Channel4_5_IRQn);
  }

  void MX_TIM8_Init(void) {
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};
    TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

    htim8.Instance = TIM8;
    htim8.Init.Prescaler = 0;
    htim8.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim8.Init.Period = WS2812::GetTimerPeriod();
    htim8.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim8.Init.RepetitionCounter = 0;
    htim8.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_PWM_Init(&htim8) != HAL_OK)
    { Device::ErrorHandler(); }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim8, &sMasterConfig) != HAL_OK)
    { Device::ErrorHandler(); }
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    if (HAL_TIM_PWM_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
    { Device::ErrorHandler(); }
    sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
    sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
    sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
    sBreakDeadTimeConfig.DeadTime = 0;
    sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
    sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
    sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
    if (HAL_TIMEx_ConfigBreakDeadTime(&htim8, &sBreakDeadTimeConfig) != HAL_OK)
    { Device::ErrorHandler(); }

    HAL_TIM_MspPostInit(&htim8);
  }

void DMA2_Channel4_5_IRQHandler(void) {
  HAL_DMA_IRQHandler(&hdma_tim8_ch2);
}
/**
 * Initializes the Global MSP.
 */

// /**
// * @brief TIM_PWM MSP Initialization
// * This function configures the hardware resources used in this example
// * @param htim_pwm: TIM_PWM handle pointer
// * @retval None
// */
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef* htim_pwm) {
  if (htim_pwm->Instance == TIM8)
  {
    /* USER CODE BEGIN TIM8_MspInit 0 */

    /* USER CODE END TIM8_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_TIM8_CLK_ENABLE();

    /* TIM8 DMA Init */
    /* TIM8_CH2 Init */
    hdma_tim8_ch2.Instance = DMA2_Channel5;
    hdma_tim8_ch2.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_tim8_ch2.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_tim8_ch2.Init.MemInc = DMA_MINC_ENABLE;
    hdma_tim8_ch2.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_tim8_ch2.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_tim8_ch2.Init.Mode = DMA_NORMAL;
    hdma_tim8_ch2.Init.Priority = DMA_PRIORITY_HIGH;
    if (HAL_DMA_Init(&hdma_tim8_ch2) != HAL_OK)
    { Device::ErrorHandler(); }

    __HAL_LINKDMA(htim_pwm, hdma[TIM_DMA_ID_CC2], hdma_tim8_ch2);

    /* USER CODE BEGIN TIM8_MspInit 1 */

    /* USER CODE END TIM8_MspInit 1 */
  }
}

void HAL_TIM_MspPostInit(TIM_HandleTypeDef* htim) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if (htim->Instance == TIM8)
  {
    /* USER CODE BEGIN TIM8_MspPostInit 0 */

    /* USER CODE END TIM8_MspPostInit 0 */

    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**TIM8 GPIO Configuration
    PC7     ------> TIM8_CH2
    */
    GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* USER CODE BEGIN TIM8_MspPostInit 1 */

    /* USER CODE END TIM8_MspPostInit 1 */
  }
}
//     /**
//     * @brief TIM_PWM MSP De-Initialization
//     * This function freeze the hardware resources used in this example
//     * @param htim_pwm: TIM_PWM handle pointer
//     * @retval None
//     */
void HAL_TIM_PWM_MspDeInit(TIM_HandleTypeDef* htim_pwm) {
  if (htim_pwm->Instance == TIM8)
  {
    /* USER CODE BEGIN TIM8_MspDeInit 0 */

    /* USER CODE END TIM8_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM8_CLK_DISABLE();

    /* TIM8 DMA DeInit */
    HAL_DMA_DeInit(htim_pwm->hdma[TIM_DMA_ID_CC2]);
    /* USER CODE BEGIN TIM8_MspDeInit 1 */

    /* USER CODE END TIM8_MspDeInit 1 */
  }
}
}