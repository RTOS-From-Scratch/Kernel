#include "nanokernel_scheduler.h"
#include "nanokernel_task.h"
#include "DataStructures/src/inverted_priority_queue.h"
#include "nanokernel_task_idle.h"

// PS -> Preemptive Scheduler
#define CONTEXT_SWITCH      NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PEND_SV

// TODO: make sure that these functions will not re-called
static IPQueue *ready_processes;
static nanokernel_Task_t* curr_task;
// current scheduler used
static void(*nanokernel_scheduler)() = NULL;

extern void __nanokernel_enableMSP();
extern void __nanokernel_enablePSP();

void __nanokernel_Scheduler_Preemptive_init(int8_t max_processes_num)
{
    // TODO: This need to be called once
//    scheduler = malloc(sizeof(__nanokernel_Scheduler_Preemptive_t));
    curr_task = NULL;
    ready_processes = IPQueue_new(max_processes_num);
}

__Scheduler __nanokernel_getScheduler()
{
    return nanokernel_scheduler;
}

void __nanokernel_Scheduler_Preemptive_updateNullableCurrentTask()
{
    // if there is no other tasks
    if( IPQueue_isEmpty(ready_processes) )
        nanokernel_Task_idle();

    curr_task = __nanokernel_Scheduler_Preemptive_getNextTask();
}

void __nanokernel_Scheduler_Preemptive_updateLowerPriorityCurrentTask()
{
    // save the current task
    IPQueue_push(ready_processes, curr_task->priority, (void*)curr_task);
    // get highest priority task
    curr_task = __nanokernel_Scheduler_Preemptive_getNextTask();
}

void __nanokernel_Scheduler_Preemptive_run()
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
            nanokernel_scheduler =
                    __nanokernel_Scheduler_Preemptive_updateNullableCurrentTask;

            // context switch
            // TODO: MSP - PSP
//            __nanokernel_enablePSP();
            CONTEXT_SWITCH;
        }

        // task which has lower priority value means that it has higher priority
        else if( curr_task->priority > IPQueue_getHeadPriority(ready_processes) )
        {
            nanokernel_scheduler =
                    __nanokernel_Scheduler_Preemptive_updateLowerPriorityCurrentTask;

            // context switch
            // TODO: MSP - PSP
//            __nanokernel_enablePSP();
            CONTEXT_SWITCH;
        }
    }
}

void __nanokernel_Scheduler_Preemptive_addTask(nanokernel_Task_t* task)
{
    // TODO: critical section
    IPQueue_push(ready_processes, task->priority, task );

    // FIXME: you can't just return if not initiated !!
    if( __nanokernel_getState() <= __NOT_BOOTED ) return;

    // check if the added one has higher priority (lower priority value)
    __nanokernel_Scheduler_Preemptive_run();
}

nanokernel_Task_t* __nanokernel_Scheduler_Preemptive_getNextTask()
{
    return IPQueue_popHead(ready_processes);
}

nanokernel_Task_t *__nanokernel_Scheduler_getCurrentTask()
{
    return curr_task;
}

void __nanokernel_Scheduler_Preemptive_endCurrentTask()
{
//    __nanokernel_enableMSP();
    // terminate current task
    nanokernel_Task_terminate(curr_task);

    // to get the next higher priority task and run it
    curr_task = NULL;

    __nanokernel_Scheduler_Preemptive_run();
}

void __nanokernel_Scheduler_Preemptive_clean()
{
    IPQueue_clean(ready_processes);
}
