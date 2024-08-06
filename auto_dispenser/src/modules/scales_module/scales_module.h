#ifndef SCALES_MODULE_H
#define SCALES_MODULE_H

#include "../../kernel/kernel.h"
#include "scales_module_settings.h"
#include "stdint.h"

#define TARE_ALL 255

/* Модуль весов. При превышении веса на датчике выше (GLASS_THRESHOLD + HYSTERESIS_WIDTH) генерирует событие
 * GLASS_AVAILABLE_CHANGE, передавая в данных указатель на uint8_t переменную, в которой содержится информация о состоянии
 * датчиков. 0 в k-м бите - рюмки на k-m датчике нет, 1 - есть.
 * При снижении веса на датчике ниже (GLASS_THRESHOLD - HYSTERESIS_WIDTH) (отри)
 */
typedef struct scales_module scales_module_t;
struct scales_module {
  module_t _module;
  
  /* функция для получения веса по номеру датчика */
  int32_t (*get_weight) (scales_module_t* module, uint8_t sensor_num);

  /* функция тарирования. 
   * sensor_num:
   * (0-254) - тарировать датчик с номером (0-254);
   * TARE_ALL - тарировать все датчики.
   * 
   * Сопоставление номера датчика с каналом мултиплексора во внутренней функции get_channel_num (файл scales_module.cpp)
   * 
   * НЕ ТАРИРОВАТЬ ПРИ НАЛИЧИИ РЮМОК НА ДАТЧИКАХ!! Это гарантированно приведет к ошибке распознавания рюмок!
   */
  void (*tare) (scales_module_t* module, uint8_t sensor_num);

  /* Таблица коэффициентов для перевода "попугаев" АЦП в граммы.
   * Значение равно попугаям на грамм массы по каждому каналу
   */ 
  const int32_t adc_to_grams[CHANNEL_COUNT] = {800, 800, 800, 800, 800, 800, 800, 800};

  uint8_t last_glass_posistions;

  int32_t callibration_koefs[CHANNEL_COUNT];
  
};

void scales_module_t_init(scales_module_t* module);

#endif // SCALES_MODULE_H
