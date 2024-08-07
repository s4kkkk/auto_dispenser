#ifndef TEST_MODULE_H
#define TEST_MODULE_H

#include "../../kernel/kernel.h"

typedef struct test_module test_module_t;
struct test_module {
  module_t _module;
  
  void (*handler1) (module_t* module, event_t* event);
  void (*button_handler) (module_t* module, event_t* event);
  void (*scales_handler) (module_t* module, event_t* event);
  void (*servo_handler) (module_t* module, event_t* event);

  uint8_t last_glass_states;

};

void test_module_t_init(test_module_t* module);

#endif // TEST_MODULE_H
