#ifndef NANOKERNEL_TASK_H_
#define NANOKERNEL_TASK_H_

#include "nanokernel.h"
#include <stddef.h>
#include <stdint.h>

struct nanokernel_Task_t {
    int32_t* stack_ptr;
    int32_t* stack_start;
    int32_t* stack_end;
    size_t stack_size;
    int8_t id;
    priority_t priority;
    void (*run)();
    int32_t stack[];
};

typedef struct nanokernel_Task_t nanokernel_Task_t;

// functions
nanokernel_Task_t* nanokernel_Task_create(uint32_t stack_size, int8_t priority, void (*run)());
void nanokernel_Task_terminate( nanokernel_Task_t *task );

#endif // NANOKERNEL_TASK_H_
