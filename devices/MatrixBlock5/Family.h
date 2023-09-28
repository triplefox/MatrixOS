// Declear Family specific function
#pragma once

#include "stm32f1xx_hal.h"

#undef USB  // CMSIS defined the USB, undef so we can use USB as MatrixOS namespace

#include "WS2812/WS2812.h"
#include "Drivers/NVS.h"

#define FUNCTION_KEY 0  // Keypad Code for main function key
#define DEVICE_APPLICATIONS
#define DEVICE_SETTING

#define DEVICE_SAVED_VAR_SCOPE "Device"

#define GRID_8x8

namespace Device
{  
  // Device Variable
  inline CreateSavedVar(DEVICE_SAVED_VAR_SCOPE, touchbar_enable, bool, true);

  const string name = "Matrix Founder Edition";
  const string model = "MXFE1";

  const string manufaturer_name = "203 Electronics";
  const string product_name = "Matrix";
  const uint16_t usb_vid = 0x0203;
  const uint16_t usb_pid = 0x1040;  //(Device Class)0001 (Device Code)000001 (Reserved for Device ID (0~63))000000

  const uint8_t sysex_mfg_id[3] = {0x00, 0x02, 0x03};
  const uint8_t sysex_family_id[3] = {0x4D, 0x58}; // {'M', 'X'}
  const uint8_t sysex_model_id[3] = {0x00, 0x00};

  const uint16_t numsOfLED = 64;
  const uint8_t x_size = 8;
  const uint8_t y_size = 8;

#define MAX_LED_LAYERS 5
  inline uint16_t keypad_scanrate = 120;
  inline uint16_t touchbar_scanrate = 60;
  inline uint16_t fps = 120;  // Depends on the FreeRTOS tick speed

  inline uint8_t brightness_level[8] = {8, 12, 24, 40, 64, 90, 128, 168};

  const uint8_t touchbar_size = 8;  // Not required by the API, private use. 16 Physical but 8 virtualized key.

  const uint16_t page_size = 2048;
  const uint8_t nums_of_page = 32;  // Total size has to smaller than 64kb because address constrain
  const uint32_t nvs_address = 0x8070000;

  void SystemClock_Config();

  inline UART_HandleTypeDef huart4;
  void MX_UART4_Init(void);

  namespace KeyPad
  {
    inline KeyConfig keyinfo_config = {
      .velocity_sensitive = false,
      .low_threshold = 0,
      .high_threshold = 65535,
      .debounce = 0,
    };
    
    inline KeyInfo fnState;
    inline KeyInfo keypadState[x_size][y_size];
    inline KeyInfo touchbarState[x_size];

    void Init();
    void InitKeyPad();
    void InitTouchbar();

    void Start();
    void StartKeyPad();
    void StartTouchBar();

    bool Scan();
    bool ScanFN();
    bool ScanKeyPad();
    bool ScanTouchBar();
    
    void Clear();
    bool NotifyOS(uint16_t keyID, KeyInfo* keyInfo);
  }

  namespace USB
  {
    void Init();
  }

  namespace LED
  {
    void Init();
    void Start();
  }
}

extern "C" {
void MX_DMA_Init();
void MX_TIM8_Init();
void HAL_TIM_MspPostInit(TIM_HandleTypeDef* htim);
void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);
void DMA2_Channel4_5_IRQHandler(void);
}