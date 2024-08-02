#ifndef ENCODER_MODULE_H
#define ENCODER_MODULE_H

#include "../../kernel/kernel.h"

typedef enum {
  READY,
  WAIT_TO_HIGH
} enc_module_state_t;

typedef struct encoder_module encoder_module_t;
struct encoder_module {
  module_t _module;

  uint8_t prev_button_state;
  enc_module_state_t module_state;
};

void encoder_module_t_init(encoder_module_t* encoder_module);

#endif // ENCODER_MODULE_H
