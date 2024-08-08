#include "servo_module.h"
#include "Servo.h"
#include "servo_module_settings.h"
#include "../../global_storage.h"
#include "Arduino.h"


#define servo_on() \
  digitalWrite(MX1508_SERVO_PIN1, HIGH)
#define servo_off() \
  digitalWrite(MX1508_SERVO_PIN1, LOW)

static void servo_module_t_task(task_t* task) {
  TRACE("servo_module_t_task");

  servo_module_t* servo_module = (servo_module_t* ) task;
  
  /* Ждем POLLING_RATE мкс*/
  scheduler.delay_us(&scheduler, task, SERVO_POLLING_RATE);


  /* Вычисление оставшегося расстояния */

  int32_t delta = servo_module->_target_position - servo_module->_current_position;
  if (delta < 0) {delta = -delta;}

  int32_t step_size_abs = servo_module->_step_size;
  if (step_size_abs < 0) {step_size_abs = -step_size_abs;}

  /* Проверка на достижение */

  if ( delta <= step_size_abs ) {

    /* Привод дошел до нужной позиции */
    scheduler.delete_task(&scheduler, task);

    servo_off();
  
    event_t new_event = {
      .event_type = SERVO_DONE,
      .event_data = NULL
    };

    scheduler.emit_event(&scheduler, &new_event);
  }
  
  else {

    /* Выполенение шага*/

    /* Обработка мертвой зоны */
    if (servo_module->_dead_zone_process) {

      /* Проверка на окончание обработки мертвой зоны */
      int32_t dead_zone_dist = servo_module->_dead_zone_end - servo_module->_current_position;
      if (dead_zone_dist < 0) {dead_zone_dist = -dead_zone_dist;}

      if (dead_zone_dist <= DEAD_ZONE_STEP_SIZE) {
        /* Мертвая зона пройдена */
        servo_module->_dead_zone_process = false;
        DEBUG("Dead zone passed");
      }
      else {
        /* Шаг в мертвой зоне */
        DEBUG("Step in deadzone");
        servo_module->_current_position += servo_module->_dead_zone_step;
        servo_module->_servo.writeMicroseconds(servo_module->_current_position);
      }

    }

    else {
      DEBUG("STEP.. Size is: ");
      DEBUG(servo_module->_step_size);
      servo_module->_current_position += servo_module->_step_size;
      servo_module->_servo.writeMicroseconds(servo_module->_current_position);
    }


  }

  return;
}

/* Метод для установки целевой позиции для сервопривода.
 * Сопоставление целевой позиции по массиву glass_pos_degs
 */
static uint8_t servo_module_t_go_to(servo_module_t* servo_module, uint8_t pos) {
  TRACE("servo_module_t_go_to");
  
  if (pos >= POSITIONS_NUM) {
    DEBUG("ERROR in servo_module_t_go_to: incorrect pos parameter!");
    return 1;
  }

  servo_on();

  /* Расчет целевого значения в мкс ШИМа */

  uint8_t degrees = servo_module->glass_pos_degs[pos];

  float mult_koef = ( (float) ((SERVO_MAX_PULSE - SERVO_MIN_PULSE)) ) / (float) 180;

  int32_t target_pos = SERVO_MIN_PULSE + degrees*mult_koef;

  if ( (SERVO_MAX_PULSE - target_pos) < 10) {target_pos = SERVO_MAX_PULSE;}
  if ( (target_pos - SERVO_MIN_PULSE) < 10) {target_pos = MIN_PULSE_WIDTH;}

  servo_module->_target_position = target_pos;
  
  bool turn_left = (target_pos < servo_module->_current_position)? true: false;

  /* Расчет мертвой зоны */
  int32_t dead_zone_end;

  if (turn_left) {
    dead_zone_end = servo_module->_current_position - DEAD_ZONE;
  } else {
    dead_zone_end = servo_module->_current_position + DEAD_ZONE;
  }

  /* Расчёт размера шага */

  if ( ( (dead_zone_end < servo_module->_target_position) && !turn_left ) || 
       ( (dead_zone_end > servo_module->_target_position) && turn_left )
      ) {

    /* Расчет размера шага */
    int32_t step_size = (servo_module->_target_position - dead_zone_end) / SERVO_MOVE_STEPS;
  
    servo_module->_step_size = step_size;
    servo_module->_dead_zone_process = true;
    servo_module->_dead_zone_end = dead_zone_end;
    if (turn_left) {
      servo_module->_dead_zone_step = -DEAD_ZONE_STEP_SIZE;
    } 
    else {
      servo_module->_dead_zone_step = DEAD_ZONE_STEP_SIZE;
    }

  }
  else {

    /* Расчет размера шага */
    int32_t step_size = (servo_module->_target_position - servo_module->_current_position) / SERVO_MOVE_STEPS;
  
    servo_module->_step_size = step_size;
    servo_module->_dead_zone_process = false;

  }

  /* Добавление задачи в очередь */
  scheduler.add_task(&scheduler, (task_t* ) servo_module);

  return 0;
}

static void servo_module_t_module_enter(module_t* module) {
  TRACE("servo_module_t_enter");

  servo_module_t* servo_module = (servo_module_t* ) module;
  
  pinMode(SERVO_CTRL_PIN, OUTPUT);
  pinMode(MX1508_SERVO_PIN1, OUTPUT);
  pinMode(MX1508_SERVO_PIN2, OUTPUT);
  pinMode(MX1508_SERVO_PIN2, LOW);

  servo_on();

  servo_module->_servo.attach(SERVO_CTRL_PIN);


  servo_module->go_to(servo_module, SERVO_INIT_POS);

  return;
}

static void servo_module_t_module_exit(module_t* module) {
  TRACE("servo_module_t_module_exit");

  servo_module_t* servo_module = (servo_module_t* ) module;

  scheduler.delete_task(&scheduler, (task_t *) module);
  servo_module->_servo.detach();
  return;
}

void servo_module_t_init(servo_module_t* module) {
  TRACE("servo_module_t_init");

  ((module_t* ) module)->_task.func = servo_module_t_task;
  ((module_t* ) module)->module_enter = servo_module_t_module_enter;
  ((module_t* ) module)->module_exit = servo_module_t_module_exit;

  module->go_to = servo_module_t_go_to;

  return;
}

#undef servo_on
#undef servo_off

servo_module_t servo_module;
