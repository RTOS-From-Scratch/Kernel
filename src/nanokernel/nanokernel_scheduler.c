#include "nanokernel_scheduler.h"
#include "nanokernel_task.h"
#include "nanokernel.h"
#include "inner/inner_nanokernel.h"
#include "../../../DataStructures/src/inverted_priority_queue.h"
#include "nanokernel_task_idle.h"

#define CONTEXT_SWITCH      NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PEND_SV

// TODO: make sure that these functions will not re-called
static IPQueue *ready_processes;
static nanokernel_Task_t* curr_task;

void __nanokernel_SchedulerPreemptive_init(int8_t max_processes_num)
{
    // TODO: This need to be called once
//    scheduler = malloc(sizeof(__nanokernel_SchedulerPreemptive_t));
    curr_task = NULL;
    ready_processes = IPQueue_new(max_processes_num);
}

void __nanokernel_SchedulerPreemptive_run()
{
    // Preemptive scheduler
    // TODO: critical section
    // TODO: msp - psp
    // FIXME: what if not initiated ?!
    if( __nanokernel_getState() > __NOT_INITIATED )
    {
        // freshly booted
        if( curr_task == NULL )
        {
            // if there is no other tasks
            if( IPQueue_isEmpty(ready_processes) )
                nanokernel_Task_idle();
            // TODO: no tasks in the queue
            curr_task = __nanokernel_SchedulerPreemptive_getNextTask();
            // context switch
            // TODO: MSP - PSP
            CONTEXT_SWITCH;
        }

        // task which has lower priority value means that it has higher priority
        else if( curr_task->priority > IPQueue_getHeadPriority(ready_processes) )
        {
            // save the current task
            IPQueue_push(ready_processes, curr_task->priority, curr_task);
            // get highest priority task
            curr_task = __nanokernel_SchedulerPreemptive_getNextTask();
            // context switch
            // TODO: MSP - PSP
            CONTEXT_SWITCH;
        }
    }
}

void __nanokernel_SchedulerPreemptive_addTask(nanokernel_Task_t* task)
{
    // TODO: critical section
    IPQueue_push(ready_processes, task->priority, task );

    // FIXME: you can't just return if not initiated !!
    if( __nanokernel_getState() <= __NOT_BOOTED ) return;

    // check if the added one has higher priority (lower priority value)
    __nanokernel_SchedulerPreemptive_run();
}

nanokernel_Task_t* __nanokernel_SchedulerPreemptive_getNextTask()
{
    return IPQueue_popHead(ready_processes);
}

nanokernel_Task_t *__nanokernel_getCurrentTask()
{
    return curr_task;
}

void __nanokernel_SchedulerPreemptive_endCurrentTask()
{
    // terminate current task
    nanokernel_Task_terminate(curr_task);

    // to get the next higher priority task and run it
    curr_task = NULL;

    __nanokernel_SchedulerPreemptive_run();
}

void __nanokernel_SchedulerPreemptive_clean()
{
    IPQueue_clean(ready_processes);
}
