#include "test_module.h"
#include "../../global_storage.h"
#include <stddef.h>

#include <Arduino.h>
#include <HardwareSerial.h>

extern HardwareSerial Serial;

static void test_module_t_task(task_t* task) {
  scheduler.delay_ms(&scheduler, task, 1000);
  Serial.println("[DEBUG]: Hello from test_module");
  scheduler.delay_ms(&scheduler, task, 1000);
  return;
}

static void test_module_t_module_enter(module_t* module) {
  scheduler.add_task(&scheduler, (task_t* ) module);
  return;
}

static void test_module_t_module_exit(module_t* module) {
  return;
}

static void test_module_t_handler1(module_t* module, event_t* event) {
  return;
}

void test_module_t_init(test_module_t* module) {

  ((module_t *) module)->_task.func = test_module_t_task;
  ((module_t *) module)->module_enter = test_module_t_module_enter;
  ((module_t *) module)->module_exit = test_module_t_module_exit;

}

/* Определение экземпляра */

test_module_t test_module;
