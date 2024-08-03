#include "encoder_module.h"
#include "encoder_settings.h"
#include "../../global_storage.h"
#include <Arduino.h>
#include <HardwareSerial.h>

/* функция, которая будет вызываться диспетчером в цикле (периодически).
 * в этой функции опрос энкодера.
 */

static void encoder_module_t_task(task_t* task) {
  TRACE("encoder_module_t_task");

  encoder_module_t* encoder_module = (encoder_module_t* ) task;

  // обработка поворотов
  
  uint8_t clk = digitalRead(ENCODER_CLK_PIN);
  uint8_t dt = digitalRead(ENCODER_DT_PIN);
  uint8_t sw = digitalRead(ENCODER_SW_PIN);

  if (encoder_module->module_state == READY) {

    if (!clk) {
      // зафиксирован поворот вправо
      encoder_module->module_state = WAIT_TO_HIGH;

      // генерация события ENCODER_TURN_RIGHT
      event_t new_event = {
        .event_type = ENCODER_TURN_RIGHT,
        .event_data = NULL
      };

      DEBUG("[DEBUG]: In encoder_task: detected turn_right. Generating event\n");
      scheduler.emit_event(&scheduler, &new_event);
    }

    if (!dt) {
      // зафиксирован поворот влево
      encoder_module->module_state = WAIT_TO_HIGH;

      // генерация события ENCODER_TURN_LEFT
      
      event_t new_event = {
        .event_type = ENCODER_TURN_LEFT,
        .event_data = NULL
      };

      DEBUG("[DEBUG]: In encoder_task: detected turn_left. Generating event\n");
      scheduler.emit_event(&scheduler, &new_event);
    }
  }

  else if (encoder_module->module_state == WAIT_TO_HIGH) {
    if (clk && dt) {encoder_module->module_state = READY;}
  }

  // обработка нажатий
  if (encoder_module->prev_button_state && !sw) {
    
    // защита от дребезга
    DEBUG("[DEBUG]: In encoder_task: entering delay_ms\n");
    scheduler.delay_ms(&scheduler, task, 10);
    DEBUG("[DEBUG]: In encoder_task: delay is done. continuing..\n");

    encoder_module->prev_button_state = sw;
    // зафиксировано нажатие. генерация события ENCODER_PRESSED
    
    event_t new_event = {
        .event_type = ENCODER_PRESSED,
        .event_data = NULL
      };
    scheduler.emit_event(&scheduler, &new_event);
    
  } 
  else if (!encoder_module->prev_button_state && sw) {

    // защита от дребезга

    DEBUG("[DEBUG]: In encoder_task: entering delay_ms\n");
    scheduler.delay_ms(&scheduler, task, 10);
    DEBUG("[DEBUG]: In encoder_task: delay is done. continuing..\n");

    encoder_module->prev_button_state = sw;

    // зафиксировано отпускание. генерация события ENCODER_RELEASED
    event_t new_event = {
        .event_type = ENCODER_RELEASED,
        .event_data = NULL
      };
    scheduler.emit_event(&scheduler, &new_event);
  }
}

/* функция, которую необходимо вызвать для инициализации модуля. В ней настраиваются все необходимые пины и пр. */

static void encoder_module_t_module_enter(module_t* module) {
  TRACE("encoder_module_t_module_enter");

  pinMode(ENCODER_CLK_PIN, INPUT);
  pinMode(ENCODER_DT_PIN, INPUT);
  pinMode(ENCODER_SW_PIN, INPUT);
  
  scheduler.add_task(&scheduler, (task_t* ) module);

  return;
}

/* функция, которая вызывается при выходе из модуля. 
 * Вполне возможно, что использоваться не будет (например, если модуль никогда не завершается)
 */
static void encoder_module_t_module_exit(module_t* module) {
  return;
}

/* Создание экземпляра модуля. Он будет виден глобально */

/* функция для заполнения экземпляра */

void encoder_module_t_init(encoder_module_t* encoder_module) {
  TRACE("encoder_module_t_init");

  encoder_module->_module._task.func = encoder_module_t_task;
  encoder_module->_module.module_enter = encoder_module_t_module_enter;
  encoder_module->_module.module_exit= encoder_module_t_module_exit;
  encoder_module->module_state = WAIT_TO_HIGH;
  encoder_module->prev_button_state = 1;
}

encoder_module_t encoder_module;
