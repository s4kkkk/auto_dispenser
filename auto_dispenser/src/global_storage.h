#ifndef GLOBAL_STORAGE_H
#define GLOBAL_STORAGE_H

#include "kernel/kernel.h"
extern scheduler_t scheduler;

#include "modules/encoder_module/encoder_module.h"
extern encoder_module_t encoder_module;

#include "modules/scheduler_test_module/scheduler_test_module.h"
extern module_t scheduler_test_module;

#endif // GLOBAL_STORAGE_H
