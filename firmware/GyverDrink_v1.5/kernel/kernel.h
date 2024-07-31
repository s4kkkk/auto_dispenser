#include <stdint.h>
#include <stdbool.h>

#ifndef KERNEL_H
#define KERNEL_H
#include "events.h"
/*--------------------ОБЩИЕ НАСТРОЙКИ----------------------*/

/* Максимальное число элементов в очереди событий. Максимально возможное - 255 */
#define MAX_EVENT_QUEUE 10

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


void event_queue_t_init(event_queue_t* queue);

bool event_queue_t_is_empty(event_queue_t* queue);

void event_queue_t_enqueue(event_queue_t* queue, event_t* event);

event_t* event_queue_t_dequeue(event_queue_t* queue);

struct event_queue {
  
  /* Очередь пустая? */
  bool (*is_empty)(event_queue_t* queue);

  /* Внести событие в очередь */
  event_t* (*enqueue)(event_queue_t* queue, event_t* event);

  /* Вытащить событие из очереди */
  event_t* (*dequeue)(event_queue_t* queue);
  
  
  // ПРИВАТНЫЕ ПОЛЯ //
  event_t _events[MAX_EVENT_QUEUE];

  uint8_t _queue_start;

  uint8_t _queue_end;
};


/* Тип задачи */

typedef struct {
  void (*func)(void *);
} task_t;

/* Модули - как-бы "драйверы", которые могут реагировать на внешние события и отсылать свои события.
 * при запуске модуля необходимо вызвать функцию metod_enter() в которой произвести инициализацию модуля
 * и привязку необходимых событий к модулю. При выходе из модуля, соответственно, вызвать module_enter().
 * Модули являются расширением обычных задач, но в отличие от задачи модули необязательно могут быть добавлены
 * в список активных задач. Модули могут только реагировать на события
 */

typedef struct module module_t;

struct module {
  task_t _task;

  module_t* parent;

  void (*module_enter)(void *resource);

  void (*module_exit) (void *resource);

};

/* Диспетчер задач, выполняющий периодический запуск задач, вызывающий обработчики событий. */

typedef struct scheduler scheduler_t;

void scheduler_t_init(scheduler_t* scheduler);

uint8_t scheduler_t_add_task(scheduler_t* scheduler, task_t* task);

uint8_t scheduler_t_delete_task(scheduler_t* scheduler, task_t* task);

uint8_t scheduler_t_emit_event(scheduler_t* scheduler, event_t* event);


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

  /* Метод для вызова события. Возвращает:
   * 0 - успешное выполнение
   * 1 - критическая ошибка
   */
  uint8_t (*emit_event) (scheduler_t* scheduler, event_t* event);


  // ПРИВАТНЫЕ ПОЛЯ //
  
  /* Отображение, связывающее event с обработчиком */

  void (*_handler[EVENTS_COUNT])(void);

  /* Очередь событий */
  event_queue_t events;

  /* Массив, хранящий активные задачи */

  task_t* _active_tasks[MAX_TASKS];

};

#endif // KERNEL_H
