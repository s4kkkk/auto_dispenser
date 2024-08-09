#include "pump_module.h"
#include "pump_module_settings.h"
#include "../servo_module/servo_module_settings.h"
#include "../scales_module/scales_module.h"
#include "../../global_storage.h"

#include <Arduino.h>


#define pump_on() \
  digitalWrite(MX1508_PUMP_PIN1, HIGH)
#define pump_off() \
  digitalWrite(MX1508_PUMP_PIN1, LOW)

#define reverse_on() \
  digitalWrite(MX1508_PUMP_PIN2, HIGH)
#define reverse_off() \
  digitalWrite(MX1508_PUMP_PIN2, LOW);

/* Задача, которая должна вызываться в ходе наливания */
static void pump_module_t_task(task_t* task) {
  TRACE("pump_module_t_task");
  
  pump_module_t* pump_module = (pump_module_t* ) task;
  
  if (pump_module->_target_weight <= 0) {
    DEBUG("panic: Incorrect target_weight");
    while(1);
  }

  /* Проверка на окончания процедуры наливания */
  int32_t current_weight = scales_module.get_weight(&scales_module, pump_module->_current_pos);
  if (ERROR_WEIGHT == current_weight) {
    DEBUG("panic: error when getting weight");
    while(1);
  }

  int32_t current_pured = current_weight - pump_module->_current_tare_weight;

  if ( (current_weight >= pump_module->_target_weight) || (current_pured<0) ) {

    if (current_weight > pump_module->_target_weight) {
      current_weight = pump_module->_target_weight;
    }

    /* Процедура наливания завершена. Запуск реверса, удаление задачи, генерация события PUMP_DONE */
    pump_off();

    reverse_on();
    scheduler.delay_ms(&scheduler, task, PUMP_REVERSE_TIME);
    reverse_off();

    scheduler.delete_task(&scheduler, task);

    event_t new_event = {
      .event_type = PUMP_DONE,
      .event_data = NULL
    };
    scheduler.emit_event(&scheduler, &new_event);
    return;
  }

  /* Оценка прогресса */

  uint8_t percent = (current_pured/pump_module->_liquid_mass)*100;

  if (percent != pump_module->_progress) {
    /* обновление процента наливания, генерация PUMP_PROGRESS_CHANGED */
    pump_module->_progress = percent;

    event_t new_event = {
      .event_type = PUMP_PROGRESS_CHANGED,
      .event_data = & (pump_module->_progress)
    };
    scheduler.emit_event(&scheduler, &new_event);
  }

  /* Процедура наливания */
  pump_on();
  scheduler.delay_ms(&scheduler, task, PUMP_POURING_TIME);
  pump_off();

  return;
}

static int8_t pump_module_t_pour(pump_module_t* pump_module, uint8_t position) {
  TRACE("pump_module_t_pour");

  if ( position >= POSITIONS_NUM ) {
    DEBUG("Error: position is invalid");
    return -1;
  }
  pump_module->_current_pos = position;

  if (pump_module->_liquid_mass <= 0) {
    DEBUG("Error: liquid mass is invalid");
    return -1;
  }

  /*Установка текущего веса рюмки и целевого веса */

  pump_module->_current_tare_weight = scales_module.get_weight(&scales_module, position);
  if (ERROR_WEIGHT == pump_module->_current_tare_weight) {
    DEBUG("Error when getting tare weight");
    return -1;
  }

  pump_module->_target_weight = pump_module->_current_tare_weight + pump_module->_liquid_mass;

  /* Все параметры установлены, запуск задачи наливания */

  scheduler.add_task(&scheduler, (task_t* ) pump_module);

  return 0;
}

static int8_t pump_module_t_set_liquid_mass(pump_module_t* pump_module, int32_t mass) {
  TRACE("pump_module_t_set_liquid_mass");

  if (mass <= 0) {
    DEBUG("Error: required mass is invalid");
    return -1;
  }

  pump_module->_liquid_mass = mass;

  return 0;
}

static void pump_module_t_module_enter(module_t* module) {
  TRACE("pump_module_t_module_enter");
  
  pump_module_t* pump_module = (pump_module_t* ) module;

  pinMode(MX1508_PUMP_PIN1, OUTPUT);
  pinMode(MX1508_PUMP_PIN2, OUTPUT);
  pinMode(MX1508_PUMP_PIN2, LOW);

  return;
}

static void pump_module_t_module_exit(module_t* module) {
  TRACE("pump_module_t_module_exit");

  pump_module_t* pump_module = (pump_module_t* ) module;

  return;
}

void pump_module_t_init(pump_module_t* module) {
  TRACE(pump_module_t_init);


  ((module_t* ) module)->_task.func = pump_module_t_task;
  ((module_t* ) module)->module_enter = pump_module_t_module_enter;
  ((module_t* ) module)->module_exit = pump_module_t_module_exit;

  module->_current_pos = 0;
  module->_liquid_mass = 0;
  module->_current_tare_weight = 0;
  module->_target_weight = 0;

  return;
}

pump_module_t pump_module;
