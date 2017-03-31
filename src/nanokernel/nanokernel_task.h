#ifndef NANOKERNEL_TASK_H_
#define NANOKERNEL_TASK_H_

#include "nanokernel.h"
#include "inner/__nanokernel_task.h"
#include <stddef.h>
#include <stdint.h>

typedef struct __nanokernel_Task_t nanokernel_Task_t;

typedef enum Priority_t {
    PRIORITY_0, // highest priority
    PRIORITY_1,
    PRIORITY_2,
    PRIORITY_3,
    PRIORITY_4,
    PRIORITY_5,
    PRIORITY_6,
    PRIORITY_7, // lowest priority
} Priority_t;

// functions
nanokernel_Task_t* nanokernel_Task_create(uint32_t stack_size, Priority_t priority, void (*run)());
void nanokernel_Task_terminate( nanokernel_Task_t *task );

#endif // NANOKERNEL_TASK_H_
