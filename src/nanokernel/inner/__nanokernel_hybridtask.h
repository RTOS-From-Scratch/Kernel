#ifndef __NANOKERNEL_HYBRIDTASK_H_
#define __NANOKERNEL_HYBRIDTASK_H_

#include "__nanokernel_task.h"
#include "Misc/src/definitions.h"

typedef struct __HybridTaskManagment {
    struct __nanokernel_Task_t* head;
    struct __nanokernel_Task_t* tail;
    byte len;
} __HybridTaskManagment;

typedef struct __nanokernel_HybridTask_t {
    struct __nanokernel_Task_t task;
    __HybridTaskManagment tasksStatesManagement[3];
} __nanokernel_HybridTask_t;

#endif // __NANOKERNEL_HYBRIDTASK_H_
