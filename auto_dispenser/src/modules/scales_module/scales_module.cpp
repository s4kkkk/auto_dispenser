#include "scales_module.h"
#include "scales_module_settings.h"
#include <Arduino.h>
#include "../../global_storage.h"

/* установить используемый канал. Возвращает:
 * 0 - успех;
 * 1 - ошибка
 */
static uint8_t mux_setup_channel(uint8_t pos) {
  TRACE("mux_setup_channel");

  if (pos > 7) {
    DEBUG("error: pos is invalid");
    return 1;
  }

  digitalWrite(MUX_S0_PIN, (pos & (1<<0) )? HIGH:LOW );
  digitalWrite(MUX_S1_PIN, (pos & (1<<1) )? HIGH:LOW );
  digitalWrite(MUX_S2_PIN, (pos & (1<<2) )? HIGH:LOW );
  return 0;
}

/* метод для проверки доступности данных */
static bool is_available(void* data) {
  TRACE("is_avaliable");
  DEBUG("In is_avaliable. Readed:");
  DEBUG(!digitalRead(ADC_DT_PIN));
  return !digitalRead(ADC_DT_PIN);
}

/* считать данные */
static int32_t adc_read(scales_module_t* module, uint8_t channel) {
  int32_t count = 0;

  digitalWrite(ADC_SCK_PIN, LOW);

  /* установка рабочего канала на мультиплексоре */
  mux_setup_channel(channel);
  scheduler.delay_ms(&scheduler, (task_t* ) module, 350);

  /* ожидание готовности данных */
  scheduler.wait(&scheduler, (task_t* ) module, is_available, NULL);

  /* считывание данных */
  for (uint8_t i=0; i<24; i++) {
    digitalWrite(ADC_SCK_PIN, HIGH);
    scheduler.delay_us(&scheduler, (task_t* ) module, 1);
    
    if (digitalRead(ADC_DT_PIN)) {count++;}

    if (i<23) {
      // для считывания 24 битов нужно только 23 сдвига
      count = count<<1;
    }
    digitalWrite(ADC_SCK_PIN, LOW);
  }
  // послеждний 25-й импульс
  digitalWrite(ADC_SCK_PIN, HIGH);
  scheduler.delay_us(&scheduler, (task_t* ) module, 1);
  digitalWrite(ADC_SCK_PIN, LOW);

  if ( (count & ( (uint32_t) 0x800000)) ) {
    // старший бит установлен, результат отрицательный. установка всех старших битов
    count |= 0xFF000000;
  }
  return count - module->callibration_koefs[channel];
}

static void adc_calibrate(scales_module_t* module){
  TRACE("adc_calibrate");

  for (uint8_t i=0; i<CHANNEL_COUNT; i++) {
    module->callibration_koefs[i] = adc_read(module, i);
  }

  return;
}


static void scales_module_t_task(task_t* task) {
  TRACE("scales_module_t_task");

  // пока просто выводим данные
  for (uint8_t i=0; i<SENSORS_COUNT; i++){
    //scheduler.delay_ms(&scheduler, task, 50);
    //i = 0;
    if (5 == i) {
      Serial.print(6);
      Serial.print(": ");
      Serial.println(adc_read((scales_module_t* ) task, 6));
    }
    else {
      Serial.print(i);
      Serial.print(": ");
      Serial.println(adc_read((scales_module_t* ) task, i));
    }
    
  }
  Serial.println("---------------------------------");

  return;
}

static void scales_module_t_module_enter(module_t* module) {
  pinMode(MUX_S0_PIN, OUTPUT);
  pinMode(MUX_S1_PIN, OUTPUT);
  pinMode(MUX_S2_PIN, OUTPUT);

  pinMode(ADC_DT_PIN, INPUT);
  pinMode(ADC_SCK_PIN, OUTPUT);

  scheduler.add_task(&scheduler, (task_t* ) module);

  adc_calibrate((scales_module_t* ) module);
  return;
}

static void scales_module_t_module_exit(module_t* module) {
  scheduler.delete_task(&scheduler, (task_t *) module);
  return;
}

void scales_module_t_init(scales_module_t* module) {

  ((module_t* ) module)->_task.func = scales_module_t_task;
  ((module_t* ) module)->module_enter = scales_module_t_module_enter;
  ((module_t* ) module)->module_exit = scales_module_t_module_exit;

  for (uint8_t i=0; i<SENSORS_COUNT; i++) {
    module->callibration_koefs[i] = 0;
  }

  return;
}

scales_module_t scales_module;
