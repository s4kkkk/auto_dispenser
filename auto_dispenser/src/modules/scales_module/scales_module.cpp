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
  scheduler.delay_ms(&scheduler, (task_t* ) module, 70);

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

static void adc_calibrate(scales_module_t* module, uint8_t channel_num){
  TRACE("adc_calibrate");

  if (255 == channel_num) {
    /* тарирование всех каналов */

    /* Два считывания для улучшения дальнейших показаний */
    for (uint8_t i=0; i<2; i++) {
      for (uint8_t j=0; j<CHANNEL_COUNT; j++) {
        adc_read(module, j);
      }
    }
  
    for (uint8_t i=0; i<CHANNEL_COUNT; i++) {
      int64_t cal_samples_sum = 0;
      module->callibration_koefs[i] = 0;
      for (uint8_t j=0; j<5; j++) {
        cal_samples_sum += adc_read(module, i);
      }
      cal_samples_sum = cal_samples_sum / 5;
      module->callibration_koefs[i] = cal_samples_sum;
    }
  }
  
  else {
    /* тарирование конкретного канала */

    /* Два считывания для улучшения дальнейших показаний */
    for (uint8_t i=0; i<2; i++) {
      adc_read(module, channel_num);
    }

    int64_t cal_samples_sum = 0;
    module->callibration_koefs[channel_num] = 0;
    for (uint8_t i=0; i<5; i++) {
      cal_samples_sum += adc_read(module, channel_num);
    }
    cal_samples_sum = cal_samples_sum / 5;
    module->callibration_koefs[channel_num] = cal_samples_sum;
  }

  return;
}

static uint8_t get_channel_num (uint8_t sensor_num) {

  switch(sensor_num) {
    case 5:
      {
        return 6;
        break;
      }
    default:
      {
        return sensor_num;
        break;
      }
  }
}

static int32_t scales_module_t_get_weight (scales_module_t* module, uint8_t sensor_num) {
  TRACE("scales_module_t_get_weight");

  if (sensor_num > 7) {
    DEBUG("error: sensor_num is invalid.");
    return ERROR_WEIGHT;
  }
  
  uint8_t channel_num = 0;

  /* Получение номера канала в зависимости от номера датчика */

  channel_num = get_channel_num(sensor_num);
  
  return adc_read(module, channel_num) / module->adc_to_grams[channel_num];
}

static void scales_module_t_tare (scales_module_t* module, uint8_t sensor_num) {
  TRACE("scales_module_t_tare");
  
  uint8_t channel_num = get_channel_num(sensor_num);
  adc_calibrate(module, channel_num);
  return;
}

static void scales_module_t_task(task_t* task) {
  TRACE("scales_module_t_task");
  
  scales_module_t* module = (scales_module_t* ) task;

  /*
  while(1) {
    Serial.println(module->get_weight(module, 1));
  }
  */

  uint8_t current_glass_positions = module->last_glass_posistions;

  for (uint8_t i=0; i<SENSORS_COUNT; i++) {

    int32_t current_weight = module->get_weight(module, i);

    if (current_weight > GLASS_THRESHOLD + HYSTERESIS_WIDTH ) {
      current_glass_positions |= (0b1<<i);
    }
    else if (current_weight < GLASS_THRESHOLD - HYSTERESIS_WIDTH ) {
      current_glass_positions &= ~(0b1<<i);
    }

  }

  if (current_glass_positions != module->last_glass_posistions) {

    /* Обнаружено изменение состояния рюмок. Генерация события GLASS_AVAILABLE_CHANGE */
    module->last_glass_posistions = current_glass_positions;

    event_t new_event = {
      .event_type = GLASS_AVAILABLE_CHANGE,
      .event_data = &(module->last_glass_posistions)
    };

    scheduler.emit_event(&scheduler, &new_event);
  }
   
  return;
}

static void scales_module_t_module_enter(module_t* module) {
  TRACE("scales_module_t_module_enter");

  pinMode(MUX_S0_PIN, OUTPUT);
  pinMode(MUX_S1_PIN, OUTPUT);
  pinMode(MUX_S2_PIN, OUTPUT);

  pinMode(ADC_DT_PIN, INPUT);
  pinMode(ADC_SCK_PIN, OUTPUT);

  scheduler.add_task(&scheduler, (task_t* ) module);

  adc_calibrate((scales_module_t* ) module, TARE_ALL);
  return;
}

static void scales_module_t_module_exit(module_t* module) {
  scheduler.delete_task(&scheduler, (task_t *) module);
  return;
}

void scales_module_t_init(scales_module_t* module) {
  TRACE("scales_module_t_init");

  ((module_t* ) module)->_task.func = scales_module_t_task;
  ((module_t* ) module)->module_enter = scales_module_t_module_enter;
  ((module_t* ) module)->module_exit = scales_module_t_module_exit;
  module->get_weight = scales_module_t_get_weight;
  module->tare = scales_module_t_tare;

  return;
}

scales_module_t scales_module;
