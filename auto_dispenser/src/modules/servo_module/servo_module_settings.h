#ifndef SERVO_MODULE_SETTINGS
#define SERVO_MODULE_SETTINGS


/* Кол-во позиций (рюмок) */
#define POSITIONS_NUM 5

/* Управляющий пин сервопривода */
#define SERVO_CTRL_PIN 9

/* Управляющие питанием серво пины MX1508 */
#define MX1508_SERVO_PIN1 5
#define MX1508_SERVO_PIN2 6

/* Ширина импульсов управления (в мкс) */
#define SERVO_MIN_PULSE 500
#define SERVO_MAX_PULSE 2400

/* Параметры из кода AlexGyver. Максимальная скорость и ускорение */
#define SERVO_SPEED 180
#define SERVO_ACCELERATION 0.6

/* Начальное пололжение сервопривода */

#define SERVO_INIT_POS 0

/* в servo_module.h заполнить массив углов */


#endif //SERVO_MODULE_SETTINGS
