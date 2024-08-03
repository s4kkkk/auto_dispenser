#include "button_module.h"
#include "button_module_settings.h"
#include "../../global_storage.h"
#include <Arduino.h>

/* функция, опрашивающая кнопку */

static void button_module_t_task(task_t* task) {
  TRACE("button_module_t_task");

  button_module_t* button_module = (button_module_t* ) task;

  // Считывание текущего значение
  uint8_t btn_state = digitalRead(BUTTON_PIN);

  if (button_module->_prev_button_state && !btn_state) {
    // защита от дребезга
    scheduler.delay_ms(&scheduler, task, 100);


    // генерация события BUTTON_PRESSED
    event_t new_event = {
      .event_type = BUTTON_PRESSED,
      .event_data = NULL
    };
    scheduler.emit_event(&scheduler, &new_event);
  }

  button_module->_prev_button_state = btn_state;
  return;
}

static void button_module_t_module_enter(module_t* module) {
  TRACE("button_module_t_enter_module");

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  scheduler.add_task(&scheduler, (task_t* ) module);

  return;
}

static void button_module_t_module_exit(module_t* module) {
  return;
}

void button_module_t_init(button_module_t* button_module) {

  button_module->_module._task.func = button_module_t_task;
  button_module->_module.module_enter = button_module_t_module_enter;
  button_module->_module.module_exit = button_module_t_module_exit;
  
  button_module->_prev_button_state = 1;

  return;
}

button_module_t button_module;
