#include "Device.h"
#include "blemidi/blemidi.h"

#include <queue>
using std::queue;

#define TAG "BLE-MIDI"

#define BLE_MIDI_PORT_ID MIDI_PORT_BLUETOOTH

#define BLE_MIDI_PORT_STACK_SIZE 256
namespace Device
{
  namespace BLEMIDI
  {
    bool started = false;
    string name;
    MidiPort* midiPort;
    StackType_t bleMidiStack[BLE_MIDI_PORT_STACK_SIZE];
    StaticTask_t bleMidiTaskTCB;
    TaskHandle_t bleMidiTask;

    void Callback(uint8_t blemidi_port, uint16_t timestamp, uint8_t midi_status, uint8_t* remaining_message, size_t len,
                  size_t continued_sysex_pos) {
      // MatrixOS::Logging::LogDebug(TAG, "CALLBACK blemidi_port=%d, timestamp=%d, midi_status=0x%02x, len=%d,
      // continued_sysex_pos=%d, remaining_message:", blemidi_port, timestamp, midi_status, len, continued_sysex_pos);
      // MatrixOS::Logging::LogDebug(TAG, "Recived 0x%02x 0x%02x 0x%02x", midi_status, remaining_message[0],
      // remaining_message[1]);
      if (len == 2)
      {
        midiPort->Send(MidiPacket(midiPort->id, (EMidiStatus)(midi_status & 0xF0), midi_status, remaining_message[0],
                                  remaining_message[1]));
      }
    }

    void Toggle() {
      if (started == false)
      { Start(); }
      else
      { Stop(); }
    }

    void Init(string name) {
      BLEMIDI::name = name;
    }

    void portTask(void* param) {
      MidiPort port = MidiPort("Bluetooth", MIDI_PORT_BLUETOOTH);
      midiPort = &port;
      MidiPacket packet;
      while (true)
      {
        if (port.Get(&packet, portMAX_DELAY))
        { blemidi_send_message(port.id % 0x100, packet.data, 3); }
      }
    }

    void Start() {
      int status = blemidi_init((void*)Callback, name.c_str());
      if (status < 0)
      { ESP_LOGE(TAG, "BLE MIDI Driver returned status=%d", status); }
      else
      {
        ESP_LOGI(TAG, "BLE MIDI Driver initialized successfully");
        bleMidiTask = xTaskCreateStatic(portTask, "Bluetooth Midi Port", BLE_MIDI_PORT_STACK_SIZE, NULL, configMAX_PRIORITIES - 2, bleMidiStack, &bleMidiTaskTCB);
        started = true;
      }
    }

    void Stop() {
      int status = blemidi_deinit();
      if (status < 0)
      { ESP_LOGE(TAG, "BLE MIDI Driver returned status=%d", status); }
      else
      {
        ESP_LOGI(TAG, "BLE MIDI Driver deinitialized successfully");
        midiPort->Unregister();
        vTaskDelete(bleMidiTask);
        started = false;
      }
    }
  }
}