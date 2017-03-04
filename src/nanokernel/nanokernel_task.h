#ifndef NANOKERNEL_TASK_H_
#define NANOKERNEL_TASK_H_

#include "nanokernel.h"
#include "stddef.h"

typedef struct nanokernel_Task nanokernel_Task;
struct nanokernel_Task {
    int32_t* stack_ptr;
    int32_t* stack_start;
    int32_t* stack_end;
    size_t stack_size;
    uint32_t id;
    priority_t priority;
    void (*run)();
    nanokernel_Task* next;
    int32_t stack[];
};

// functions
void nanokernel_init( nanokernel_Task* task );
void nanokernel_Task_initStack( nanokernel_Task* task );
nanokernel_Task* nanokernel_Task_create(uint32_t stack_size, void (*run)());
void nanokernel_Task_terminate( nanokernel_Task *task );

#endif // NANOKERNEL_TASK_H_
