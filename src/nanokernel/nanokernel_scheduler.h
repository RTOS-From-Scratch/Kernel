#ifndef NANOKERNEL_SCHEDULER_H_
#define NANOKERNEL_SCHEDULER_H_

#include "nanokernel_task.h"

void nanokernel_scheduler();

void __nanokernel_Scheduler_Preemptive_init(int8_t max_processes_num);
void __nanokernel_Scheduler_Preemptive_addTask(nanokernel_Task_t* task);
nanokernel_Task_t* __nanokernel_Scheduler_Preemptive_getNextTask();
// TODO: change name `run`
void __nanokernel_Scheduler_Preemptive_run();
void __nanokernel_Scheduler_Preemptive_endCurrentTask();
nanokernel_Task_t* __nanokernel_getCurrentTask();
void __nanokernel_SchedulerPreemptive_clean();

#endif // NANOKERNEL_SCHEDULER_H_
