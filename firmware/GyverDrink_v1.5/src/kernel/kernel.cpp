#include "kernel.h"
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

bool event_queue_t_is_empty(event_queue_t *queue) {
  if (queue->_current_size == 0) {
    return true;
  }
  else {
    return false;
  }
}

uint8_t event_queue_t_enqueue(event_queue_t* queue, event_t* event) {

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

event_t* event_queue_t_dequeue(event_queue_t* queue) {

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


event_queue_t* event_queue_t_init(event_queue_t *queue) {

  /* Обнуление _events */

  uint8_t* _start = (uint8_t *) queue->_events;

  for (size_t i=0; i< (MAX_EVENT_QUEUE * sizeof(event_t)) ; i++) {
    *_start = 0;
    _start++;
  }

  queue->_queue_start = 0;
  queue->_queue_end = 0;

  queue->enqueue = event_queue_t_enqueue;
  queue->dequeue = event_queue_t_dequeue;
  queue->is_empty = event_queue_t_is_empty;
  queue->_current_size = 0;

  return queue;
}
