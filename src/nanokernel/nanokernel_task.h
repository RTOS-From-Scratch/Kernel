#ifndef NANOKERNEL_TASK_H_
#define NANOKERNEL_TASK_H_

#include "nanokernel.h"
#include "inner/__nanokernel_task.h"
#include <stddef.h>
#include <stdint.h>

typedef struct __nanokernel_Task_t nanokernel_Task_t;

// This is an example of the priorties used
// you can use any positive number in the range of an `int`
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
nanokernel_Task_t* nanokernel_Task_create( size_t stack_len,
                                           Priority_t priority,
                                           void (*run)(),
                                           uint8_t maxNumberOfDrivers );
void nanokernel_Task_terminate( nanokernel_Task_t *task );
// return ID of the current task
TaskID nanokernel_Task_getID();

#endif // NANOKERNEL_TASK_H_
