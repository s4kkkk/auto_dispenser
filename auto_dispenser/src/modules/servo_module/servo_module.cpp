#include "servo_module.h"
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

  scheduler.delay_ms(&scheduler, task, 300);

  /*
  if (servo_module->servo.tickManual()) {
    // Сервопривод пришел в установленное положение. Удаление задачи, выключение питания сервопривода
    // Генерация события SERVO_DONE
    // Serial.println("Servo ready");
    scheduler.delete_task(&scheduler, task);
    servo_off();

    event_t new_event = {
      .event_type = SERVO_DONE,
      .event_data = NULL
    };
    scheduler.emit_event(&scheduler, &new_event);
  }
  */

  scheduler.delete_task(&scheduler, task);
  servo_off();

  event_t new_event = {
    .event_type = SERVO_DONE,
    .event_data = NULL
  };
  scheduler.emit_event(&scheduler, &new_event);

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

  servo_module->servo.write(servo_module->glass_pos_degs[pos]);

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
  servo_module->servo.attach(SERVO_CTRL_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
  // servo_module->servo.setSpeed(SERVO_SPEED);
  // servo_module->servo.setAccel(SERVO_ACCELERATION);

  servo_module->go_to(servo_module, SERVO_INIT_POS);

  return;
}

static void servo_module_t_module_exit(module_t* module) {
  TRACE("servo_module_t_module_exit");

  servo_module_t* servo_module = (servo_module_t* ) module;

  scheduler.delete_task(&scheduler, (task_t *) module);
  servo_module->servo.detach();
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
