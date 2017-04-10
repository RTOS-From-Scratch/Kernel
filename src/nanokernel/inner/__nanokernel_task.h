#ifndef INNER_NANOKERNEL_TASK_H_
#define INNER_NANOKERNEL_TASK_H_

#include <stdint.h>
#include <stddef.h>
#include "Misc/src/definitions.h"
#include "Drivers/src/driver.h"

struct __nanokernel_Task_t {
    intptr_t* stack_ptr;
    intptr_t* stack_start;
    intptr_t* stack_end;
    size_t stack_size;
    int8_t id;
    long priority;
    void (*run)();
    intptr_t* stack;
    // inner struct contains data about the drivers current task hols
    struct __HoldedDrivers {
        uint8_t len;
        int8_t currentIndex;
        Driver** list;
    } __HoldedDrivers;
    intptr_t chunkOfMemory[];
};

#endif // INNER_NANOKERNEL_TASK_H_
