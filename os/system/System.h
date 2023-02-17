#pragma once

// #define XSTR(x) STR(x)
// #define STR(x) #x
// #pragma message "app index" XSTR(APPLICATION_INDEX)
const inline uint16_t app_count = APPLICATION_INDEX;
Application_Info* applications[app_count];

namespace MatrixOS::SYS
{
  StackType_t application_stack[APPLICATION_STACK_SIZE];
  StaticTask_t application_taskdef;

  #define SUPERVISOR_STACK_SIZE 64
  StackType_t supervisor_stack[SUPERVISOR_STACK_SIZE];
  StaticTask_t supervisor_taskdef;

  // #define TASKLOGGING_STACK_SIZE 192
  // StackType_t tasklogging_stack[TASKLOGGING_STACK_SIZE];
  // StaticTask_t tasklogging_taskdef;

  inline TaskHandle_t active_app_task = NULL;
  inline uint32_t active_app_id = 0;
  inline uint32_t next_app = 0;

  void ExecuteAPP(uint32_t app_id);
  uint32_t GenerateAPPID(string author, string app_name);

  uint16_t GetApplicationCount();
}