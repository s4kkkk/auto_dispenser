#include "test_module.h"
#include "../../global_storage.h"
#include <stddef.h>

#include <Arduino.h>
#include <HardwareSerial.h>

static void test_module_t_task(task_t* task) {
  scheduler.delay_ms(&scheduler, task, 1000);
  Serial.println("[DEBUG]: Hello from test_module");
  scheduler.delay_ms(&scheduler, task, 1000);
  return;
}

static void test_module_t_handler(module_t* module, event_t* pending_event) {
  TRACE("test_module_t_handler");

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

static void test_module_t_button_handler(module_t* module, event_t* pending_event) {
  TRACE("test_module_t_button_handler");
  
  event_type_t event_type = pending_event->event_type;

  switch (event_type) {

    case BUTTON_PRESSED:
      Serial.println("BUTTON_PRESSED");
      break;

    default:
      break;
  }
  return;
}

static void test_module_t_module_enter(module_t* module) {
  TRACE("test_module_t_module_enter");
  // scheduler.add_task(&scheduler, (task_t* ) module);
  //
  scheduler.register_event(&scheduler, ENCODER_TURN_LEFT, &test_module_t_handler, (module_t* ) module);
  scheduler.register_event(&scheduler, ENCODER_TURN_RIGHT, &test_module_t_handler, (module_t* ) module);
  scheduler.register_event(&scheduler, ENCODER_PRESSED, &test_module_t_handler, (module_t* ) module);
  scheduler.register_event(&scheduler, ENCODER_RELEASED, &test_module_t_handler, (module_t*) module);
  scheduler.register_event(&scheduler, BUTTON_PRESSED, &test_module_t_button_handler, (module_t* ) module);

  return;
}

static void test_module_t_module_exit(module_t* module) {
  return;
}

void test_module_t_init(test_module_t* module) {
  TRACE("test_module_t_init");

  ((module_t *) module)->_task.func = test_module_t_task;
  ((module_t *) module)->module_enter = test_module_t_module_enter;
  ((module_t *) module)->module_exit = test_module_t_module_exit;
  module->handler1 = test_module_t_handler;
  module->button_handler = test_module_t_button_handler;

}

/* Определение экземпляра */

test_module_t test_module;
