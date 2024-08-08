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
#define SERVO_MIN_PULSE 400
#define SERVO_MAX_PULSE 2400

/* Параметры плавного пуска сервопривода.
 * Основная задумка состоит в снижении начального крутящего момента сервы.
 * Разница в мкс испульсов управляющего шима между начальным и целевым значением делится на шаги, размер которых
 * равен (MOVE_SPEED/POLLING_RATE). Каждый период опроса (POLLING) значение шима изменяется на величину шага.
 */

/* Скорость опроса в мкс. (dt ) Предполагается, что задача сервпоривода вызывается c периодом не более, чем POLLING_RATE */
#define SERVO_POLLING_RATE 10000


/* Максимальное ускорение сервопривода в мкШ/мкс^2 */

#define SERVO_ACCELERATION 0.00005

/* Начальное пололжение сервопривода */

#define SERVO_INIT_POS 0

/* в servo_module.h заполнить массив углов */


#endif //SERVO_MODULE_SETTINGS
