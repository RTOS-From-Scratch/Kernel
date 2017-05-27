#ifndef NANOKERNEL_SCHEDULER_H_
#define NANOKERNEL_SCHEDULER_H_

#include "nanokernel_task.h"

PRIVATE
    typedef void (*__Scheduler)();

    // Premptive scheduler
    void __nanokernel_Scheduler_Preemptive_init(int8_t max_processes_num);
    void __nanokernel_Scheduler_Preemptive_addTask(nanokernel_Task_t* task);
    nanokernel_Task_t* __nanokernel_Scheduler_Preemptive_getNextTask();
    void __nanokernel_Scheduler_exec();
    void __nanokernel_Scheduler_Preemptive_endCurrentTask();
    nanokernel_Task_t *__nanokernel_Scheduler_getCurrentTask();
    void __nanokernel_Scheduler_Preemptive_clean();

    __Scheduler __nanokernel_getScheduler();

#endif // NANOKERNEL_SCHEDULER_H_
