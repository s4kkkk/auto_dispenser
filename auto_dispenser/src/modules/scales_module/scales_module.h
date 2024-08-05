#ifndef SCALES_MODULE_H
#define SCALES_MODULE_H

#include "../../kernel/kernel.h"
#include "scales_module_settings.h"
#include "stdint.h"

typedef struct scales_module scales_module_t;
struct scales_module {
  module_t _module;
  
  uint8_t last_state;

  int32_t callibration_koefs[CHANNEL_COUNT];

};

void scales_module_t_init(scales_module_t* module);

#endif // SCALES_MODULE_H
