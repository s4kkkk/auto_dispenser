#include "kernel.h"
#include "events.h"
#include "../global_storage.h"

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include <Arduino.h>

/* EVENT_QUEUE_T */

static bool event_queue_t_is_empty(event_queue_t *queue) {
  if (queue->_current_size == 0) {
    return true;
  }
  else {
    return false;
  }
}

static uint8_t event_queue_t_enqueue(event_queue_t* queue, event_t* event) {

  queue->_events[queue->_queue_end] = *event;

  if (queue->_queue_end == (MAX_EVENT_QUEUE - 1) ) {
    queue->_queue_end = 0;
  }

  else {
    queue->_queue_end++;
  }

  if ( queue->_current_size < MAX_EVENT_QUEUE ) {
    queue->_current_size++;
  }

  return 0;
}

static event_t* event_queue_t_dequeue(event_queue_t* queue) {

  if (queue->is_empty(queue)) {
    return (event_t* ) NULL;
  }

  event_t* event_to_return = & (queue->_events[queue->_queue_start]);
  queue->_current_size--;

  if (queue->_queue_start == (MAX_EVENT_QUEUE - 1) ) {
    queue->_queue_start = 0;
  }

  else {
    queue->_queue_start++;
  }

  return event_to_return;
}

static uint8_t event_queue_t_get_events_number(event_queue_t* queue) {
  return queue->_current_size;
}


event_queue_t* event_queue_t_init(event_queue_t *queue) {

  /* Обнуление _events */

  uint8_t* _start = (uint8_t *) queue->_events;

  for (size_t i=0; i< (MAX_EVENT_QUEUE * sizeof(event_t)) ; i++) {
    *_start = 0;
    _start++;
  }

  queue->_queue_start = 0;
  queue->_queue_end = 0;

  queue->is_empty = event_queue_t_is_empty;
  queue->enqueue = event_queue_t_enqueue;
  queue->dequeue = event_queue_t_dequeue;
  queue->get_events_number = event_queue_t_get_events_number;
  queue->_current_size = 0;

  return queue;
}

/* EVENT_QUEUE_T  END*/

/* TASK_LIST_T */

static uint8_t task_list_t_add_task(task_list_t* task_list, task_t* task) {

  if (!task || !task_list) {return 2;}

  if (!task_list->have_freespace(task_list)) {return 1;}

  task_list->_tasks[task_list->_free_field_index] = *task;
  task_list->_current_tasks++;

  if (task_list->have_freespace(task_list)) {
    task_list->_free_field_index++;
  }

  return 0;
}

static task_t* task_list_t_get_task(task_list_t* task_list) {

  if (0 == task_list->_current_tasks) {
    return (task_t* ) NULL;
  }

  task_t* task_to_return = &task_list->_tasks[task_list->_selected_task];
  if (task_list->_selected_task < task_list->_current_tasks - 1) {
    task_list->_selected_task++;
  }
  else {
    task_list->_selected_task = 0;
  }

  return task_to_return;
}

static uint8_t task_list_t_delete_task(task_list_t* task_list, task_t* task) {

  uint8_t task_index = 0;

  for (; task_index < task_list->_current_tasks; task_index++) {
    if (task_list->_tasks[task_index].func == task->func) {
      // Задача найдена
      break;
    }
  }

  if (task_index == task_list->_current_tasks) {
    // Задача не найдена 
    return 1;
  }
  
  // проверка текущей выбранной задачи, которая вернется по get_task

  if ( (task_index < task_list->_selected_task) && (1 != task_list->_current_tasks) ) {
    task_list->_selected_task--;
  }

  task_list->_tasks[task_index].func = NULL;
  task_list->_current_tasks--;

  if (task_list->_current_tasks == 0) {
    task_list->_free_field_index = 0;
    return 0;
  }
  
  else {
    uint8_t old_tasks_num = task_list->_current_tasks + 1;
    for (uint8_t i=0; i<old_tasks_num; i++) {

      // поиск индекса первой активной задачи
      uint8_t first_ta_index = i;
    
      for (; first_ta_index < old_tasks_num; first_ta_index++) {
        if (task_list->_tasks[first_ta_index].func != NULL) {break;}
      }

      if (first_ta_index == i) {
        // first_ta_index == i. перезаписывать не надо => переход к следующей итерации
        continue;
      }

      if (first_ta_index == old_tasks_num) {
        // все задачи релоцированны. выход из цикла
        break;
      }

      // переноc задачи в начало массива

      task_list->_tasks[i] = task_list->_tasks[first_ta_index];
      task_list->_tasks[first_ta_index].func = 0;
    }
    task_list->_free_field_index = task_list->_current_tasks;
  }
  return 0;
}

static bool task_list_t_have_freespace(task_list_t* task_list) {
  if (task_list->_current_tasks < MAX_TASKS) {return true;}
  else {return false;}
}

static uint8_t task_list_t_get_tasks_number(task_list_t* task_list) {
  return task_list->_current_tasks;
}

task_list_t* task_list_t_init(task_list_t* task_list) {

  task_list->add_task = task_list_t_add_task;
  task_list->delete_task = task_list_t_delete_task;
  task_list->get_task = task_list_t_get_task;
  task_list->have_freespace = task_list_t_have_freespace;
  task_list->get_tasks_number = task_list_t_get_tasks_number;

  task_list->_current_tasks = 0;
  task_list->_selected_task = 0;
  task_list->_free_field_index = 0;

  // зануление задач
  
  for (uint8_t i=0; i<MAX_TASKS; i++) {
    task_list->_tasks[i].func = NULL;
  }

  return task_list;
}

/* TASK_LIST_T END */

/* SCHEDULER_T */


static void scheduler_t_func(task_t* task) {
  TRACE("scheduler_t_func");

  scheduler_t* scheduler = (scheduler_t* )task;

  // запуск активных задач

  uint8_t current_tasks_num = scheduler->_active_tasks.get_tasks_number(&scheduler->_active_tasks);
  
  // delay(1000);
  
  DEBUG("[DEBUG]: In Scheulder: Current tasks num: ");
  DEBUG(current_tasks_num);
  DEBUG("\n");

  for (uint8_t i=0; i<current_tasks_num; i++) {
    DEBUG("[DEBUG]: In Scheulder: Entering task\n");
    task_t* task = scheduler->_active_tasks.get_task(&scheduler->_active_tasks);
    task->func(task);
  }

  // обработка возникших событий

  uint8_t current_events = scheduler->_events.get_events_number(&scheduler->_events);
  DEBUG("[DEBUG]: In scheduler. Current events: ");
  DEBUG(current_events); DEBUG("\n");

  for (uint8_t i=0; i<current_events; i++) {
    DEBUG("[DEBUG]: In scheduler. Processing event\n");
    event_t* event = scheduler->_events.dequeue(&scheduler->_events);

    void (*handler) (module_t*, event_t*) = scheduler->_event_bindings[event->event_type].handler;
    module_t* handler_module = scheduler->_event_bindings[event->event_type].module;

    handler(handler_module, event);
    DEBUG("[DEBUG]: In scheduler. Processing event done\n");
  }

  return;
}

static uint8_t scheduler_t_add_task(scheduler_t* scheduler, task_t* task) {
  TRACE("scheduler_t_add_task");

  if (!scheduler || !task) {return 2;}

  if (!scheduler->_active_tasks.have_freespace(&scheduler->_active_tasks)) {
    return 1;
  }

  DEBUG("[DEBUG]: In add_task: Adding task\n");
  return scheduler->_active_tasks.add_task(&scheduler->_active_tasks, task);
}

static uint8_t scheduler_t_delete_task(scheduler_t* scheduler, task_t* task) {
  TRACE("scheduler_t_delete_task");

  if (!scheduler || !task) {return 2;}

  DEBUG("[DEBUG]: In delete_task: Deleting task\n");
  return scheduler->_active_tasks.delete_task(&scheduler->_active_tasks, task);
}

static uint8_t scheduler_t_register_event(scheduler_t* scheduler, event_type_t event_type, void (*handler)(module_t*, event_t* ), module_t* module) {
  TRACE("scheduler_t_register_event");

  if (!scheduler || !handler || !module) {return 1;}

  // привязка события к обработчику
  scheduler->_event_bindings[event_type].handler = handler;
  scheduler->_event_bindings[event_type].module = module;

  return 0;
}

static uint8_t scheduler_t_emit_event(scheduler_t* scheduler, event_t* event) {
  TRACE("scheduler_t_emit_event");
  if (!scheduler || !event) {return 1;}
  
  return scheduler->_events.enqueue(&scheduler->_events, event);
}


#ifndef UINT32_MAX
#define UINT32_MAX ( (uint32_t) (-1) )
#endif

static void scheduler_t_delay_us(scheduler_t* scheduler, task_t* task, uint32_t useconds) {
  TRACE("scheduler_t_delay_us");

  // получение текущего времени  
  uint32_t saved_time = micros();

  // удаление текущей задачи из списка активных
  if (task) {
    scheduler->delete_task(scheduler, task);
  }
  
  uint32_t delta = 0;

  while (delta < useconds) {
    scheduler->_task.func((task_t* ) scheduler);

    // вычисление прошедшего времени
    uint32_t current_time = micros();

    if (current_time >= saved_time) {
      delta = current_time - saved_time;
    }
    else {
      // произошло переполнение
      delta = current_time + (UINT32_MAX - saved_time + 1);
    }

  }

  if (task) {
    scheduler->add_task(scheduler, task);
  }

  return;
}

static void scheduler_t_delay_ms(scheduler_t* scheduler, task_t* task, uint32_t mseconds) {
  TRACE("scheduler_t_delay_ms");

  // получение текущего времени  
  uint32_t saved_time = millis();

  // удаление текущей задачи из списка активных
  if (task) {
    DEBUG("[DEBUG]: In delay_ms: deleting task\n");

    scheduler->delete_task(scheduler, task);
  }
  
  uint32_t delta = 0;

  while (delta < mseconds) {

    DEBUG("[DEBUG]: In delay_ms. Entering scheduler\n");

    scheduler->_task.func((task_t* ) scheduler);

    // вычисление прошедшего времени
    uint32_t current_time = millis();

    if (current_time >= saved_time) {
      delta = current_time - saved_time;
    }
    else {
      // произошло переполнение
      DEBUG("[DEBUG]: In delay_ms: triggered overflow!\n");
      delta = current_time + (UINT32_MAX - saved_time + 1);
    }

  }

  if (task) {
    DEBUG("[DEBUG]: In delay_ms: adding task\n");
    scheduler->add_task(scheduler, task);
  }

  return;
}

scheduler_t* scheduler_t_init(scheduler_t* scheduler) {
  TRACE("scheduler_t_init");

  // Инициализация _active_tasks

  task_list_t_init(&scheduler->_active_tasks);

  // Инициализация _events

  event_queue_t_init(&scheduler->_events);
  
  // Основная инициализация планировщика 
  scheduler->add_task = scheduler_t_add_task;
  scheduler->delete_task = scheduler_t_delete_task;
  scheduler->register_event = scheduler_t_register_event;
  scheduler->emit_event = scheduler_t_emit_event;
  scheduler->delay_us = scheduler_t_delay_us;
  scheduler->delay_ms = scheduler_t_delay_ms;

  ((task_t* ) scheduler)->func = scheduler_t_func;

  return scheduler;
}

/* Определение планировщика */

scheduler_t scheduler;

/* SCHEDULER_T END */
