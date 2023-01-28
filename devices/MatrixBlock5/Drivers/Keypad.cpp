#include "Device.h"
#include "timers.h"

namespace Device::KeyPad
{
  StaticTimer_t keypad_timer_def;
  TimerHandle_t keypad_timer;

  void ScanTimerCallback(TimerHandle_t xTimer) {
    if (ScanFN())
      return;
    if (ScanKeyPad())
      return;
  }

  void InitKeyPad() {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOC, Key8_Pin | Key7_Pin | Key6_Pin | Key5_Pin, GPIO_PIN_RESET);

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOB, Key4_Pin | Key3_Pin | Key2_Pin | Key1_Pin, GPIO_PIN_RESET);

    /*Configure GPIO pins : Key8_Pin Key7_Pin Key6_Pin Key5_Pin */
    GPIO_InitStruct.Pin = Key8_Pin | Key7_Pin | Key6_Pin | Key5_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /*Configure GPIO pins : KeyRead5_Pin KeyRead8_Pin KeyRead7_Pin */
    GPIO_InitStruct.Pin = KeyRead5_Pin | KeyRead8_Pin | KeyRead7_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /*Configure GPIO pins : FN_Pin KeyRead4_Pin KeyRead3_Pin KeyRead6_Pin */
    GPIO_InitStruct.Pin = FN_Pin | KeyRead4_Pin | KeyRead3_Pin | KeyRead6_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /*Configure GPIO pins : KeyRead2_Pin KeyRead1_Pin */
    GPIO_InitStruct.Pin = KeyRead2_Pin | KeyRead1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /*Configure GPIO pins : Key4_Pin Key3_Pin Key2_Pin Key1_Pin */
    GPIO_InitStruct.Pin = Key4_Pin | Key3_Pin | Key2_Pin | Key1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    fnState.setConfig(&keyinfo_config);

    for (uint8_t x = 0; x < x_size; x++)
    {
      for (uint8_t y = 0; y < y_size; y++)
      { keypadState[x][y].setConfig(&keyinfo_config); }
    }
  }

  void Init() {
    InitKeyPad();
    // InitTouchBar();
  }


  void StartKeyPad() {
    keypad_timer = xTimerCreateStatic(NULL, configTICK_RATE_HZ / Device::keypad_scanrate, true, NULL,
                                      ScanTimerCallback, &keypad_timer_def);
    xTimerStart(keypad_timer, 0);
  }

  void Start() {
    StartKeyPad();
    // StartTouchBar();
  }

  void Clear() {
    fnState.Clear();

    for (uint8_t x = 0; x < x_size; x++)
    {
      for (uint8_t y = 0; y < y_size; y++)
      { keypadState[x][y].Clear(); }
    }

    for (uint8_t i = 0; i < touchbar_size; i++)
    { touchbarState[i].Clear(); }
  }

  KeyInfo* GetKey(uint16_t keyID) {
    uint8_t keyClass = keyID >> 12;
    switch (keyClass)
    {
      case 0:  // System
      {
        uint16_t index = keyID & (0b0000111111111111);
        switch (index)
        {
          case 0:
            return &fnState;
        }
        break;
      }
      case 1:  // Main Grid
      {
        int16_t x = (keyID & (0b0000111111000000)) >> 6;
        int16_t y = keyID & (0b0000000000111111);
        if (x < x_size && y < y_size)
          return &keypadState[x][y];
        break;
      }
      case 2:  // Touch Bar
      {
        uint16_t index = keyID & (0b0000111111111111);
        // MatrixOS::Logging::LogDebug("Keypad", "Read Touch %d", index);
        if (index < touchbar_size)
          return &touchbarState[index];
        break;
      }
    }
    return nullptr;  // Return an empty KeyInfo
  }


bool ScanFN() {
    Fract16 read = HAL_GPIO_ReadPin(FN_GPIO_Port, FN_Pin) * UINT16_MAX;
    if (fnState.update(read, false))
    {
      if (NotifyOS(0, &fnState))
      { return true; }
    }
    return false;
  }

  bool ScanKeyPad() {
    for (uint8_t x = 0; x < Device::x_size; x++)
    {
      HAL_GPIO_WritePin(keypad_write_pins[x].port, keypad_write_pins[x].pin, GPIO_PIN_SET);
      for (uint8_t y = 0; y < Device::y_size; y++)
      {
        Fract16 read = HAL_GPIO_ReadPin(keypad_read_pins[y].port, keypad_read_pins[y].pin) * UINT16_MAX;
       bool updated = keypadState[x][y].update(read, true);
        if (updated)
        {
          uint16_t keyID = (1 << 12) + (x << 6) + y;
          if (NotifyOS(keyID, &keypadState[x][y]))
          { return true; }
        }
      }
      HAL_GPIO_WritePin(keypad_write_pins[x].port, keypad_write_pins[x].pin, GPIO_PIN_RESET);
      volatile int i;
      for (i = 0; i < 5; ++i)
      {}  // Add small delay
    }
    return false;
  }

  bool NotifyOS(uint16_t keyID, KeyInfo* keyInfo) {
    KeyEvent keyEvent;
    keyEvent.id = keyID;
    keyEvent.info = *keyInfo;
    return MatrixOS::KEYPAD::NewEvent(&keyEvent);
  }


  // Matrix use the following ID Struct
  // CCCC IIIIIIIIIIII
  // C as class (4 bits), I as index (12 bits). I could be spilted by the class defination, for example, class 0 (grid),
  // it's spilted to XXXXXXX YYYYYYY. Class List: Class 0 - System - IIIIIIIIIIII Class 1 - Grid - XXXXXX YYYYYY Class 2
  // - TouchBar - IIIIIIIIIIII Class 3 - Underglow - IIIIIIIIIIII

  uint16_t XY2ID(Point xy) {
    if (xy.x >= 0 && xy.x < 8 && xy.y >= 0 && xy.y < 8)  // Main grid
    { return (1 << 12) + (xy.x << 6) + xy.y; }
    else if ((xy.x >= 0 && xy.x < 8) && xy.y == 8)  // Touch Bar
    { return (2 << 12) + xy.x; }
    // MatrixOS::Logging::LogError("Keypad", "Failed XY2ID %d %d", xy.x, xy.y);
    return UINT16_MAX;
  }


  Point ID2XY(uint16_t keyID) {
    uint8_t keyClass = keyID >> 12;
    switch (keyClass)
    {
      case 1:  // Main Grid
      {
        int16_t x = (keyID & 0b0000111111000000) >> 6;
        int16_t y = keyID & (0b0000000000111111);
        if (x < Device::x_size && y < Device::y_size)
          return Point(x, y);
        break;
      }
      case 2:  // TouchBar
      {
        uint16_t index = keyID & (0b0000111111111111);
        if (index < Device::touchbar_size)
        {
          return Point(index % 8, Device::y_size) ;
        }
        break;
      }
    }
    return Point(INT16_MIN, INT16_MIN);
  }
}