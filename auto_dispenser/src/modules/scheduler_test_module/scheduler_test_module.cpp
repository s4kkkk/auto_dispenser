#include "scheduler_test_module.h"
#include "../../kernel/kernel.h"
#include "../../global_storage.h"

#include <Arduino.h>
#include <HardwareSerial.h>

extern HardwareSerial Serial;

static void scheduler_test_module_handler(module_t* module, event_t* pending_event) {
  
  Serial.println("[DEBUG]: from event_handler");

  event_type_t event_type = pending_event->event_type;

  switch (event_type) {

    case ENCODER_TURN_LEFT: 
      Serial.println("ENCODER_TURN_LEFT");
      break;

    case ENCODER_TURN_RIGHT: 
      Serial.println("ENCODER_TURN_RIGHT");
      break;

    case ENCODER_PRESSED: 
      Serial.println("ENCODER_PRESSED");
      break;

    case ENCODER_RELEASED: 
      Serial.println("ENCODER_RELEASED");
      break;

    default:
      break;
  }
  return;
}

static void scheduler_test_module_module_enter(module_t* module) {
  // подключение событий энкодера
  scheduler.register_event(&scheduler, ENCODER_TURN_LEFT, &scheduler_test_module_handler, module);
  scheduler.register_event(&scheduler, ENCODER_TURN_RIGHT, &scheduler_test_module_handler, module);
  scheduler.register_event(&scheduler, ENCODER_PRESSED, &scheduler_test_module_handler, module);
  scheduler.register_event(&scheduler, ENCODER_RELEASED, &scheduler_test_module_handler, module);
  return;
}

/* Создание экземпляра модуля */

void scheduler_test_module_init(module_t* scheduler_test_module) {
  scheduler_test_module->module_enter = scheduler_test_module_module_enter;
}

module_t scheduler_test_module;
