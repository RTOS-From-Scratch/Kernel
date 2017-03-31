#ifndef INNER_NANOKERNEL_TASK_H_
#define INNER_NANOKERNEL_TASK_H_

#include <stdint.h>
#include <stddef.h>
#include "../Misc/src/definitions.h"

struct __nanokernel_Task_t {
    int32_t* stack_ptr;
    int32_t* stack_start;
    int32_t* stack_end;
    size_t stack_size;
    int8_t id;
    long priority;
    void (*run)();
    int32_t stack[];
};

#endif // INNER_NANOKERNEL_TASK_H_
