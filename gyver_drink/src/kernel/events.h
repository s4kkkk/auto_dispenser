#ifndef EVENTS_H
#define EVENTS_H

/* Типы событий */
typedef enum {
  ENCODER_TURN_LEFT = 1,
  ENCODER_TURN_RIGHT,
  ENCODER_PUSHED,
  BUTTON_PUSHED,
  BUTTON_RELEASED,
  EVENTS_COUNT
} event_type_t;

#endif // EVENTS_H
