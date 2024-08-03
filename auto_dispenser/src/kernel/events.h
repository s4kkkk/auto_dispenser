#ifndef EVENTS_H
#define EVENTS_H

/* Типы событий */
typedef enum {
  ENCODER_PRESSED,
  ENCODER_RELEASED,
  ENCODER_TURN_LEFT,
  ENCODER_TURN_RIGHT,
  BUTTON_PRESSED,
  EVENTS_COUNT
} event_type_t;

#endif // EVENTS_H
