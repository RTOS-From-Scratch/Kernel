#ifndef INNER_NANOKERNEL_TASK_H_
#define INNER_NANOKERNEL_TASK_H_

#include <stdint.h>
#include <stddef.h>
#include "Misc/src/definitions.h"
#include "Drivers/src/driver.h"

typedef struct __nanokernel_Task_Driver {
    DriverName driverName;
    Driver* driver;
} __nanokernel_Task_Driver;

struct __nanokernel_Task_t {
    intptr_t* stack_ptr;
    intptr_t* stack_start;
    intptr_t* stack_end;
    size_t stack_size;
    int8_t id;
    long priority;
    void (*nanokernel_Task_entry)();
    intptr_t* stack;
    // inner struct contains data about a groups of tasks has the same priority
    struct __EqualPriQueue {
        struct __nanokernel_Task_t* next;
        // this variable is not NULL only in the `head` Task
        struct __nanokernel_Task_t* tail;
    } __EqualPriQueue;
    // inner struct contains data about the drivers current task hols
    struct __HoldedDrivers {
        int8_t len;
        int8_t currentIndex;
        __nanokernel_Task_Driver* list;
    } __HoldedDrivers;
    intptr_t chunkOfMemory[];
};

#endif // INNER_NANOKERNEL_TASK_H_
