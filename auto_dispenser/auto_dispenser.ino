#include "src/kernel/kernel.h"
#include "src/global_storage.h"
#include "src/modules/button_module/button_module.h"
#include "src/modules/test_module/test_module.h"

void setup() {

  Serial.begin(9600);

  scheduler_t_init(&scheduler);
   
  // подключение модуля энкодера
  encoder_module_t_init(&encoder_module);
  ((module_t* ) &encoder_module)->module_enter( (module_t* ) &encoder_module ); 
  
  // подключение модуля кнопки
  button_module_t_init(&button_module);
  ((module_t* ) &button_module)->module_enter( (module_t *) &button_module );

  // тестовый модуль
  test_module_t_init(&test_module);
  ((module_t *) &test_module)->module_enter((module_t* ) &test_module);
}


void loop() {
  ((task_t*) (&scheduler))->func((task_t* ) &scheduler);
}
