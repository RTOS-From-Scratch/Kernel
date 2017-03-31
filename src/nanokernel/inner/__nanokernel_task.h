#ifndef INNER_NANOKERNEL_TASK_H_
#define INNER_NANOKERNEL_TASK_H_

#include <stdint.h>
#include <stddef.h>
#include "../Misc/src/definitions.h"

typedef struct __Driver {
    __Driver_deinit_func deinit_func;
    int module_number;
} __Driver;

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
    struct Drivers {
        uint8_t len;
        int8_t currentIndex;
        __Driver* list;
    } Drivers;
    intptr_t chunkOfMemory[];
};

// if id == ID_TASKLESS, treat it as there is no current task
// this technique is used if the developer initiate the driver in the `main` function
void __nanokernel_Task_holdDriver(TaskID id, __Driver_deinit_func deinit_func, int module_number);
void __nanokernel_Task_releaseDriver(TaskID id, __Driver_deinit_func deinit_func, int module_number);
#endif // INNER_NANOKERNEL_TASK_H_
