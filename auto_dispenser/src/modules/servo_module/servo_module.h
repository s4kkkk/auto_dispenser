#ifndef SERVO_MODULE_H
#define SERVO_MODULE_H

#include "../../kernel/kernel.h"
#include "servo_module_settings.h"
#include <Servo.h>

/* Модуль сервопривода. Метод позволяет отправить серво на нужную позицию. При успешном повороте 
 * этот модуль генерирует событие SERVO_DONE
 */

typedef struct servo_module servo_module_t;
struct servo_module {
  module_t _module;

  /* углы направления на рюмки */
  const int glass_pos_degs[POSITIONS_NUM] = {
    36, // 0
    72, // 1
    108, // 2
    144, // 3
    180 // 4
  };

  /* для перемещения. Возвращает
   * 0 - успешное выполнение. Серво начал движение на установленную позицию
   * 1 - критическая ошибка
   */
  uint8_t (*go_to) (servo_module_t* servo_module, uint8_t pos);


  /* ПРИВАТНЫЕ ПОЛЯ */

  Servo _servo;

  /* Текущая позиция сервопривода в мкс импульса ШИМа */
  int32_t _current_position;

  /* Целевая позиция сервопривода в мкс импульса ШИМа */
  int32_t _target_position;

  /* Размер шага в мкс импульса ШИМа */
  int32_t _step_size;

  /* Флаг обработки мертвой зоны */
  bool _dead_zone_process;

  /* Позиция конца мертвой зоны */
  int32_t _dead_zone_end;

  /* Размер шага в мертвой зоне */
  int32_t _dead_zone_step;

};

void servo_module_t_init(servo_module_t* module);

#endif // SERVO_MODULE_H
