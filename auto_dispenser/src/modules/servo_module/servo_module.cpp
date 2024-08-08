#include "servo_module.h"
#include "Servo.h"
#include "servo_module_settings.h"
#include "../../global_storage.h"
#include "Arduino.h"


#define servo_on() \
  digitalWrite(MX1508_SERVO_PIN1, HIGH)
#define servo_off() \
  digitalWrite(MX1508_SERVO_PIN1, LOW)

static float tr (float x) {

  if ( (x>=0) && (x<0.5) ) {
    return x;
  }
  else if ( (x>=0.5) && (x<=1) ) {
    return 1 - x;
  }
  else {
    return 0;
  }
}

static void servo_module_t_task(task_t* task) {
  TRACE("servo_module_t_task");

  servo_module_t* servo_module = (servo_module_t* ) task;
  
  /* Ждем POLLING_RATE мкс*/
  scheduler.delay_us(&scheduler, task, SERVO_POLLING_RATE);

  /* Проверка на достижение цели */

  if ( servo_module->_steps_completed >= servo_module->_steps_count ) {

    /* Привод дошел до нужной позиции */
    servo_module->_servo.writeMicroseconds(servo_module->_target_position);
    servo_module->_current_position = servo_module->_target_position;
    scheduler.delay_us(&scheduler, task, SERVO_POLLING_RATE);

    scheduler.delete_task(&scheduler, task);

    servo_off();
  
    event_t new_event = {
      .event_type = SERVO_DONE,
      .event_data = NULL
    };

    scheduler.emit_event(&scheduler, &new_event);
    return;
  }

  /* Вычисление текущего размера шага */

  servo_module->_steps_completed++;

  float step_size = (((float) 4*servo_module->_distance)/(servo_module->_steps_count*SERVO_POLLING_RATE)) * tr( ((float) servo_module->_steps_completed) / (servo_module->_steps_count))*SERVO_POLLING_RATE;

  if (servo_module->_negative_distance) {step_size = -step_size;}

  /* Выполнение шага */
  servo_module->_current_position += step_size;

  servo_module->_servo.writeMicroseconds(servo_module->_current_position);
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
  
  servo_module->_negative_distance = (target_pos < servo_module->_current_position)? true: false;

  int32_t distance = (servo_module->_negative_distance) ? -(target_pos - servo_module->_current_position) : (target_pos - servo_module->_current_position);
  
  servo_module->_distance = distance;

  servo_module->_steps_count = (2*sqrt(distance / ( SERVO_ACCELERATION * SERVO_ACCELERATION ) ) / SERVO_POLLING_RATE);

  servo_module->_steps_completed = 0;

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
