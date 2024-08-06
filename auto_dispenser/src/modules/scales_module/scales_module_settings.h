#ifndef SCALES_MODULE_SETTING_H
#define SCALES_MODULE_SETTING_H

/* пины мультиплексора */
#define MUX_S0_PIN 6
#define MUX_S1_PIN 5
#define MUX_S2_PIN 4

/* пины АЦП */
#define ADC_DT_PIN 2
#define ADC_SCK_PIN 3

/* кол-во каналов мультиплексора */
#define CHANNEL_COUNT 8

/* кол-во датчиков*/
#define SENSORS_COUNT 6

/* минимальный порог массы рюмки в граммах */
#define GLASS_THRESHOLD 15

/* ширина петли гистерезиса в граммах для детектирования наличия рюмки */
#define HYSTERESIS_WIDTH 3

#endif // SCALES_MODULE_SETTING_H
