#ifndef NANOKERNEL_SCHEDULER_H_
#define NANOKERNEL_SCHEDULER_H_

#include "nanokernel_task.h"

void nanokernel_scheduler();

void __nanokernel_SchedulerPreemptive_init(int8_t max_processes_num);
void __nanokernel_SchedulerPreemptive_addTask(nanokernel_Task_t* task);
nanokernel_Task_t* __nanokernel_SchedulerPreemptive_getNextTask();
// TODO: change name `run`
void __nanokernel_SchedulerPreemptive_run();
void __nanokernel_SchedulerPreemptive_endCurrentTask();
nanokernel_Task_t* __nanokernel_getCurrentTask();
void __nanokernel_SchedulerPreemptive_clean();

#endif // NANOKERNEL_SCHEDULER_H_
