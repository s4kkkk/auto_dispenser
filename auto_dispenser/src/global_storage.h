#ifndef GLOBAL_STORAGE_H
#define GLOBAL_STORAGE_H

#include <HardwareSerial.h>
extern HardwareSerial Serial;

#include "modules/encoder_module/encoder_module.h"
extern encoder_module_t encoder_module;

#include "modules/button_module/button_module.h"
extern button_module_t button_module;

#include "modules/test_module/test_module.h"
extern test_module_t test_module;

#include "modules/scales_module/scales_module.h"
extern scales_module_t scales_module;

extern scheduler_t scheduler;

#endif // GLOBAL_STORAGE_H
