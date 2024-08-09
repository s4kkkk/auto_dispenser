#ifndef PUMP_MODULE_H
#define PUMP_MODULE_H

#include "../../kernel/kernel.h"
#include "pump_module_settings.h"

/* Модуль помпы. Наливает установленное кол-во грамм жидкости в рюмку */

typedef struct pump_module pump_module_t;
struct pump_module {
  module_t _module;

  /* Метод для запуска наливания жидкости */
  int8_t (*pour) (pump_module_t* pump_module);

  /* Метод для установки массы наливаемой жидкости */
  int8_t (*set_liquid_mass) (pump_module_t* pump_module, int32_t mass);


  /* ПРИВАТНЫЕ ПОЛЯ */

  /* Позиция наливаемой рюмки */
  uint8_t _current_pos;

  /* Установленная масса жидкости */
  int32_t _liquid_mass;
  
  /* Текущая масса рюмки */
  int32_t _current_tare_weight;

  /* Целевая масса рюмки */
  int32_t _target_weight;

};


#endif // PUMP_MODULE_H
