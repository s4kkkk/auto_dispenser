#ifndef SERVO_MODULE_H
#define SERVO_MODULE_H

#include "../../kernel/kernel.h"
#include "servo_module_settings.h"

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

};

#endif // SERVO_MODULE_H
