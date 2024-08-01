#include <cstdint>
#include <stdint.h>
#include <stdbool.h>

#ifndef KERNEL_H
#define KERNEL_H
#include "events.h"
/*--------------------ОБЩИЕ НАСТРОЙКИ----------------------*/

/* Максимальное число элементов в очереди событий. Максимально возможное - 255 */
#define MAX_EVENT_QUEUE 4

/* Максимальное число активных задач */
#define MAX_TASKS 10

/*-------------------------КОНЕЦ---------------------------*/


typedef void* event_data_t;

/* Общая структура событий */

typedef struct {
  event_type_t event_type;
  event_data_t event_data;
} event_t;



/* Очередь событий */

typedef struct event_queue event_queue_t;

event_queue_t* event_queue_t_init(event_queue_t* queue);

struct event_queue {
  
  /* Очередь пуста? */
  bool (*is_empty)(event_queue_t* queue);

  /* Внести событие в очередь. Возвращает:
   * 0 - успешное выполнение
   */
  uint8_t (*enqueue)(event_queue_t* queue, event_t* event);

  /* Вытащить событие из очереди. Возвращает:
   * event_t* - успешное выполнение
   * NULL - очередь пуста
   */
  event_t* (*dequeue)(event_queue_t* queue);
  
  /*Сколько ожидающих событий? */

  uint8_t (*get_events_number)(event_queue_t* queue);
  
  // ПРИВАТНЫЕ ПОЛЯ //
  event_t _events[MAX_EVENT_QUEUE];

  uint8_t _queue_start;

  uint8_t _queue_end;

  uint8_t _current_size;
};


/* Интерфейс задачи. Задача - базовая абстракция. Предполагается, что задача - просто периодически выполняемая
 * функция со своими данными. При определении конкретной задачи (например, задачи опроса энкодера) создается 
 * структура, наследуемая от task_t, в которой перечисляются необходимые для работы задачи данные. Предполагается, что
 * перед каждым вызовом func() на вход передается указатель на задачу. Внутри func() этот указатель приводится к типу
 * структуры конкретной задачи, таким образом в конкретной задаче можно взаимодействовать со статическими данными задачи
 */

typedef struct task task_t;

struct task {
  void (*func)(task_t* task);
};


/* Модули - как-бы "драйверы", которые могут реагировать на внешние события и отсылать свои события.
 * при запуске модуля необходимо вызвать функцию metod_enter() в которой произвести инициализацию модуля
 * и привязку необходимых событий к модулю. При выходе из модуля, соответственно, вызвать module_exit().
 * Модули являются расширением обычных задач в том смысле, что реакция на события доступна только модулям
 * (классам - наследникам модулей, которые определяют обработчики)
 */

typedef struct module module_t;

struct module {
  task_t _task;

  void (*module_enter)(module_t* module);

  void (*module_exit) (module_t* module);

};

/* Список задач. Используется в диспетчере */
typedef struct task_list task_list_t;

task_list_t* task_list_t_init(task_list_t* task_list);

struct task_list {

  /* Метод для добавления задачи. Возвращает:
   * 0 - успешное выполнение
   * 1 - исчерпан максимальный объем очереди задач
   * 2 - критическая ошибка
   */
  uint8_t (*add_task) (task_list_t* task_list, task_t* task);

  /* Метод для удаления задачи. Возвращает:
   * 0 - успешное выполнение
   * 1 - задача не найдена
   * 2 - критическая ошибка
   */
  uint8_t (*delete_task) (task_list_t* task_list, task_t* task);

  /* Метод для получения задачи. Возвращает:
   * task_t* - успешное выполнение
   * NULL - в случае ошибки (внутренняя ошибка, пустой списо и т.д.)
   */
  task_t* (*get_task) (task_list_t* task_list);
  
  /* Можно ли добавить задачу? */
  bool (*have_freespace) (task_list_t* task_list);

  /* Сколько активных задач? */

  uint8_t (*get_tasks_number) (task_list_t* task_list);

  // ПРИВАТНЫЕ ПОЛЯ //
  
  uint8_t _current_tasks;
  uint8_t _selected_task;

  uint8_t _free_field_index;

  task_t _tasks[MAX_TASKS];
};


/* Структура, хранящая связь обработчика события с событием */

typedef struct event_to_handler_binding event_to_handler_binding_t;

struct event_to_handler_binding {
  void (*handler)(module_t*, event_t* event);
  module_t* module;
};

/* Диспетчер задач, выполняющий периодический запуск задач, вызывающий обработчики событий. */

typedef struct scheduler scheduler_t;

scheduler_t* scheduler_t_init(scheduler_t* scheduler);

struct scheduler {

  task_t _task;

  /* Метод для добавления задачи. Возвращает:
   * 0 - успешное выполнение
   * 1 - исчерпан максимальный объем очереди задач
   * 2 - критическая ошибка
   */
  uint8_t (*add_task) (scheduler_t* scheduler, task_t* task);

  /* Метод для удаления задачи. Возвращает:
   * 0 - успешное выполнение
   * 1 - задача не найдена
   * 2 - критическая ошибка
   */
  uint8_t (*delete_task) (scheduler_t* scheduler, task_t* task);
  
  /* Метод для регистрации события. Связывает событие с обработчиком.Возвращает:
   * 0 - успешное выполнение
   * 1 - критическая ошибка
   */
  uint8_t (*register_event) (scheduler_t* scheduler, event_type_t event_type, void (*handler)(module_t*, event_t* ), module_t* module);  

  /* Метод для вызова события. Возвращает:
   * 0 - успешное выполнение
   * 1 - критическая ошибка
   */
  uint8_t (*emit_event) (scheduler_t* scheduler, event_t* event);



  // ПРИВАТНЫЕ ПОЛЯ //
  
  /* Отображение, связывающее event с обработчиком */

  event_to_handler_binding_t _event_bindings [EVENTS_COUNT];

  /* Очередь событий */
  event_queue_t _events;

  /* Массив, хранящий активные задачи */

  task_list_t _active_tasks;

};

#endif // KERNEL_H
