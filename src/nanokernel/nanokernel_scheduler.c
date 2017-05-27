#include "nanokernel_scheduler.h"
#include "nanokernel_task.h"
#include "DataStructures/src/sorted_linkedlist.h"
#include "nanokernel_task_idle.h"
#include "Drivers/src/inner/__systick.h"

// PS -> Preemptive Scheduler
#define CONTEXT_SWITCH      NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PEND_SV

// current scheduler used
typedef void(*Scheduler)();

// This design keep private data more safe
struct nanokernel_Scheduler_t
{
    SortedLinkedList_t ready_tasks_queue;
    SortedLinkedList_Node_t ready_tasks[NUM_OF_TASKS];

    nanokernel_Task_t* curr_task;
    nanokernel_Task_t* curr_equalPriority_task;

    Scheduler scheduler;
} __nanokernel_scheduler;

// TODO: remove these
//extern void __nanokernel_enableMSP();
//extern void __nanokernel_enablePSP();
static void __nanokernel_Scheduler_Preemptive_onEqualPriorityTasksExit();
static void __nanokernel_Task_addEqualPriTask( nanokernel_Task_t *task );
static void __nanokernel_Task_removeEqualPriTask( nanokernel_Task_t *task );

void __nanokernel_Scheduler_Preemptive_init(int8_t max_processes_num)
{
    // TODO: This need to be called once
//    scheduler = malloc(sizeof(__nanokernel_Scheduler_Preemptive_t));
    __nanokernel_scheduler.curr_task = NULL;
    __nanokernel_scheduler.curr_equalPriority_task = NULL;
    __nanokernel_scheduler.scheduler = NULL;

    SortedLinkedList_init( __nanokernel_scheduler.ready_tasks,
                           NUM_OF_TASKS,
                           &__nanokernel_scheduler.ready_tasks_queue );
}

__Scheduler __nanokernel_getScheduler()
{
    return __nanokernel_scheduler.scheduler;
}

void __nanokernel_Scheduler_Preemptive_onEqualPriorityTasksExit()
{
    __SysTick_stop();
    __nanokernel_scheduler.curr_equalPriority_task = NULL;
}

void __nanokernel_Scheduler_Preemptive()
{
    // check if the `onEqualPriorityTasks` premptive scheduler is exited
    if( __SysTick_isInit() )
        if( __nanokernel_scheduler.curr_task->__EqualPriQueue.next is NULL )
            __nanokernel_Scheduler_Preemptive_onEqualPriorityTasksExit();

    // if there is no other tasks
    if( SortedLinkedList_isEmpty(&__nanokernel_scheduler.ready_tasks_queue) )
        nanokernel_Task_idle();

    else
    {
        // freshly booted or return from terminated task
        if( __nanokernel_scheduler.curr_task is NULL )
        {
            __nanokernel_scheduler.curr_task = __nanokernel_Scheduler_Preemptive_getNextTask();

            // if equal priority tasks exits with the same priority as the current task
            if( __nanokernel_scheduler.curr_task->__EqualPriQueue.next is_not NULL )
            {
                // get next task in the queue of equal priorty
                __nanokernel_scheduler.curr_equalPriority_task = __nanokernel_scheduler.curr_task;

                // initiate the timer if it's not initiated
                if( __SysTick_isInit() is false )
                {
                    __SysTick_init( __SysTick_getTicks(1000)/*EQUAL_PRIRITY_TIME_SLICE*/,
                                    __nanokernel_Scheduler_exec,
                                    __nanokernel_scheduler.curr_task->priority );
                    __SysTick_start(ST_MODE_CONTINOUS);
                }
            }
        }

        else
        {
            nanokernel_Task_t * hightestReadyTask =
                    SortedLinkedList_getHeadData( &__nanokernel_scheduler.ready_tasks_queue);

            // task which has lower priority value means that it has higher priority
            if( __nanokernel_scheduler.curr_task->priority > hightestReadyTask->priority )
            {
                // get the highest priority task
                __nanokernel_scheduler.curr_task = hightestReadyTask;
            }

            // Equal priority Tasks
            else if( __nanokernel_scheduler.curr_task->__EqualPriQueue.next is_not NULL )
            {
                __nanokernel_scheduler.curr_task = __nanokernel_Scheduler_Preemptive_getNextTask();
            }
        }
    }
}

void __nanokernel_Scheduler_exec()
{
    // Preemptive scheduler
    // TODO: critical section
    // TODO: msp - psp
    // FIXME: what if not initiated ?!
    if( __nanokernel_getState() > __NOT_INITIATED )
    {
        __nanokernel_scheduler.scheduler = __nanokernel_Scheduler_Preemptive;

        CONTEXT_SWITCH;
    }
}

void __nanokernel_Scheduler_Preemptive_addTask( nanokernel_Task_t* task )
{
    // TODO: critical section

    // if return false that means that there is already task with the same priority
    // so attack it to the `already task with the same priority`.
    if (SortedLinkedList_insert(&__nanokernel_scheduler.ready_tasks_queue,
                                (void*)task, task->priority) is false)
        __nanokernel_Task_addEqualPriTask(task);

    // FIXME: you can't just return if not initiated !!
    if( __nanokernel_getState() <= __NOT_BOOTED ) return;

    // check if the added one has higher priority (lower priority value)
    __nanokernel_Scheduler_exec();
}

nanokernel_Task_t* __nanokernel_Scheduler_Preemptive_getNextTask()
{
    if( __nanokernel_scheduler.curr_equalPriority_task is_not NULL )
        return __nanokernel_scheduler.curr_equalPriority_task =
                __nanokernel_scheduler.curr_task =
                 __nanokernel_scheduler.curr_task->__EqualPriQueue.next;

    else
        return SortedLinkedList_getHeadData(&__nanokernel_scheduler.ready_tasks_queue);
}

nanokernel_Task_t *__nanokernel_Scheduler_getCurrentTask()
{
    return __nanokernel_scheduler.curr_task;
}

void __nanokernel_Scheduler_Preemptive_endCurrentTask()
{
//    __nanokernel_enableMSP();

    // TODO: remove from the queue of tasks
    // TODO: remove from blocked queue
    if( __nanokernel_scheduler.curr_task->__EqualPriQueue.next is_not NULL )
        __nanokernel_Task_removeEqualPriTask(__nanokernel_scheduler.curr_task);

    else
        SortedLinkedList_popAt( &__nanokernel_scheduler.ready_tasks_queue,
                                __nanokernel_scheduler.curr_task->priority );

    // terminate current task
    nanokernel_Task_terminate(__nanokernel_scheduler.curr_task);

    // to get the next higher priority task and run it
    __nanokernel_scheduler.curr_task = NULL;

    __nanokernel_Scheduler_exec();
}

void __nanokernel_Scheduler_Preemptive_clean()
{
//    IPQueue_clean(ready_tasks);
    SortedLinkedList_clean(&__nanokernel_scheduler.ready_tasks_queue);
}

// TODO: This is conflict in design
// TODO: These 2 functions should be part of `nanokernel_Task`
static void __nanokernel_Task_addEqualPriTask( nanokernel_Task_t *task )
{
    nanokernel_Task_t *same_pri_task =
            SortedLinkedList_getData( &__nanokernel_scheduler.ready_tasks_queue,
                                      task->priority );

    // this task is the first one that has same priority
    if( same_pri_task->__EqualPriQueue.next is NULL )
    {
        same_pri_task->__EqualPriQueue.next = task;
        same_pri_task->__EqualPriQueue.tail = task;
    }

    // there are more than one task with the same priority
    else
    {
        // update the tail `next_equal_pri` task in this queue
        same_pri_task->__EqualPriQueue.tail->__EqualPriQueue.next = task;
        // update `tail` in the `head` task
        same_pri_task->__EqualPriQueue.tail = task;
    }

    task->__EqualPriQueue.next = same_pri_task;
}

static void __nanokernel_Task_removeEqualPriTask( nanokernel_Task_t *task )
{
    nanokernel_Task_t *task_to_be_terminated = task;
    nanokernel_Task_t *prev = task;
    nanokernel_Task_t *head = SortedLinkedList_getData(&__nanokernel_scheduler.ready_tasks_queue,
                                                       task->priority);

    // search for the task id in this queue
    while( (task_to_be_terminated->id) is_not task->id )
    {
        prev = task_to_be_terminated;
        task_to_be_terminated = task_to_be_terminated->__EqualPriQueue.next;
    }

    // if we want to remove the first element in this queue
    if( task_to_be_terminated is head )
    {
        // just update the first element data in this queue,
        // to hold the `next_equal_pri` task
        SortedLinkedList_updateData(&__nanokernel_scheduler.ready_tasks_queue,
                                    head->priority,
                                    head->__EqualPriQueue.next);

        // get the new head
        head = head->__EqualPriQueue.next;

        // check if there is only 1 remaining task in the queue after updating
        if( head is task->__EqualPriQueue.tail )
        {
            head->__EqualPriQueue.next = NULL;
            head->__EqualPriQueue.tail = NULL;
        }

        else
        {
            // update the `tail` of the new `head` task
            head->__EqualPriQueue.tail = task->__EqualPriQueue.tail;
            head->__EqualPriQueue.tail->__EqualPriQueue.next = head;
        }
    }

    else
    {
        // this is the next task of the head is the tail task
        if( head->__EqualPriQueue.next is head->__EqualPriQueue.tail )
        {
            head->__EqualPriQueue.next = NULL;
            head->__EqualPriQueue.tail = NULL;
        }

        else
        {
            // find the wanted task and its previous task
//            while( task_to_be_terminated->id is_not task->id )
//            {
//                task_to_be_terminated = task_to_be_terminated->__EqualPriQueue.next;
//                prev = task_to_be_terminated;
//            }

            prev->__EqualPriQueue.next = task_to_be_terminated->__EqualPriQueue.next;
        }
    }
}
