#include "src/kernel/kernel.h"
#include "src/global_storage.h"

void setup() {

  Serial.begin(9600);
  // инициализация диспетчера
  scheduler_t_init(&scheduler);

  // подключение модуля энкодера
  encoder_module_t_init(&encoder_module);
  ((module_t* ) &encoder_module)->module_enter( (module_t* ) &encoder_module );

  // подключение тестового модуля
  scheduler_test_module_init(&scheduler_test_module);
  scheduler_test_module.module_enter(&scheduler_test_module);
}

// луп
void loop() {
  // главный цикл
  ((task_t* ) &scheduler)->func( (task_t* ) &scheduler );
}
