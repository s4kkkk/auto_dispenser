#ifndef BUTTON_MODULE_H
#define BUTTON_MODULE_H

#include "../../kernel/kernel.h"
#include "stdint.h"

typedef struct button_module button_module_t;
struct button_module {
  module_t _module;

  uint8_t _prev_button_state;
};

void button_module_t_init(button_module_t* button_module);

#endif // BUTTON_MODULE_H
