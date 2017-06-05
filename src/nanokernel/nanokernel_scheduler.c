#include "nanokernel_scheduler.h"
#include "nanokernel_task.h"
#include "DataStructures/src/sorted_linkedlist_with_id.h"
#include "nanokernel_task_idle.h"
#include "Drivers/src/inner/__systick.h"
#include <stdlib.h>

// PS -> Preemptive Scheduler
#define CONTEXT_SWITCH      NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PEND_SV

// This design keep private data more safe
struct nanokernel_Scheduler_t
{
    SortedLinkedListWithID_t ready_tasks_queue;
    SortedLinkedListWithID_t blocked_tasks_queue;
    SortedLinkedListWithID_t suspended_tasks_queue;
    SortedLinkedListWithID_Node_t *tasks;

    nanokernel_Task_t* curr_task;
    nanokernel_Task_t* curr_equalPriority_task;

    __Scheduler scheduler;
} __nanokernel_scheduler;

// TODO: remove these
//extern void __nanokernel_enableMSP();
//extern void __nanokernel_enablePSP();
static void __nanokernel_Scheduler_Preemptive_onEqualPriorityTasksExit();
static void __nanokernel_Task_addEqualPriTask( nanokernel_Task_t *task,
                                               SortedLinkedListWithID_t *queue );
static void __nanokernel_Task_removeEqualPriTask(nanokernel_Task_t *task ,
                                                  SortedLinkedListWithID_t *queue );
static bool __nanokernel_Scheduler_Preemptive_shouldExec();
static void __nanokernel_Scheduler_Preemptive_changeTaskState(nanokernel_Task_t* task,
                                                             SortedLinkedListWithID_t *from,
                                                             SortedLinkedListWithID_t *to );

void __nanokernel_Scheduler_Preemptive_init( byte max_tasks_num )
{
    // TODO: This need to be called once
//    scheduler = malloc(sizeof(__nanokernel_Scheduler_Preemptive_t));
    __nanokernel_scheduler.tasks = malloc(sizeof(SortedLinkedListWithID_Node_t));
    __nanokernel_scheduler.curr_task = NULL;
    __nanokernel_scheduler.curr_equalPriority_task = NULL;
    __nanokernel_scheduler.scheduler = NULL;

    // these queues operate on the same array `tasks`
    // but the are independant on each other ( different heads/tails )
    SortedLinkedListWithID_init( __nanokernel_scheduler.tasks,
                                 max_tasks_num,
                                 __READY,
                                 &__nanokernel_scheduler.ready_tasks_queue );

    SortedLinkedListWithID_init( __nanokernel_scheduler.tasks,
                                 max_tasks_num,
                                 __BLOCKED,
                                 &__nanokernel_scheduler.blocked_tasks_queue );

    SortedLinkedListWithID_init( __nanokernel_scheduler.tasks,
                                 max_tasks_num,
                                 __SUSPENDED,
                                 &__nanokernel_scheduler.suspended_tasks_queue );
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

bool __nanokernel_Scheduler_Preemptive_shouldExec()
{
    if( __nanokernel_scheduler.curr_task is_not NULL )
    {
        if( ( (nanokernel_Task_t *)SortedLinkedListWithID_getHeadData(
                  &__nanokernel_scheduler.ready_tasks_queue ) )->priority <=
                __nanokernel_scheduler.curr_task->priority )
            return true;

        else if( __nanokernel_scheduler.curr_task->state is __BLOCKED )
            return true;

        else if( __nanokernel_scheduler.curr_task->state is __SUSPENDED )
            return true;

        else
            return false;
    }

    return true;
}

void __nanokernel_Scheduler_Preemptive()
{
    // check if the `onEqualPriorityTasks` premptive scheduler is exited
    if( __SysTick_isInit() )
        if( __nanokernel_scheduler.curr_task->__EqualPriQueue.next is NULL )
            __nanokernel_Scheduler_Preemptive_onEqualPriorityTasksExit();

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
                __SysTick_init( __SysTick_getTicks(EQUAL_PRIRITY_TIME_SLICE),
                                __nanokernel_Scheduler_exec,
                                __nanokernel_scheduler.curr_task->priority );
                __SysTick_start(ST_MODE_CONTINOUS);
            }
        }
    }

    else
    {
        nanokernel_Task_t * hightestReadyTask =
                SortedLinkedListWithID_getHeadData( &__nanokernel_scheduler.ready_tasks_queue);

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

        else if( __nanokernel_scheduler.curr_task->state is __BLOCKED )
        {
            __nanokernel_scheduler.curr_task = __nanokernel_Scheduler_Preemptive_getNextTask();
        }
    }
}

void __nanokernel_Scheduler_exec()
{
    // Preemptive scheduler
    // TODO: critical section
    // TODO: msp - psp
    // FIXME: what if not initiated ?!

    __nanokernel_States curr_sys_state = __nanokernel_getState();

    if( curr_sys_state is __BOOTED )
    {
        __nanokernel_scheduler.scheduler = __nanokernel_Scheduler_Preemptive;

        // if there is no other tasks
        if( SortedLinkedListWithID_isEmpty(&__nanokernel_scheduler.ready_tasks_queue) )
            nanokernel_Task_idle();

        // this make sure that it's not useless to context switch
        // as may be the current task has already the highest priority
        else if( __nanokernel_Scheduler_Preemptive_shouldExec() )
            CONTEXT_SWITCH;
    }

    else if( curr_sys_state is __TASKLESS )
        nanokernel_Task_idle();
}

void __nanokernel_Scheduler_Preemptive_addTask( nanokernel_Task_t* task )
{
    // TODO: critical section
    // if return false that means that there is already task with the same priority
    // so attack it to the `already task with the same priority`.
    if (SortedLinkedListWithID_insert(&__nanokernel_scheduler.ready_tasks_queue,
                                (void*)task, task->priority) is false)
        __nanokernel_Task_addEqualPriTask( task,
                                           &__nanokernel_scheduler.ready_tasks_queue );

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
        return SortedLinkedListWithID_getHeadData(&__nanokernel_scheduler.ready_tasks_queue);
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
        __nanokernel_Task_removeEqualPriTask( __nanokernel_scheduler.curr_task,
                                              &__nanokernel_scheduler.ready_tasks_queue );

    else
        SortedLinkedListWithID_popAt( &__nanokernel_scheduler.ready_tasks_queue,
                                      __nanokernel_scheduler.curr_task->priority );

    // terminate current task
    nanokernel_Task_terminate(__nanokernel_scheduler.curr_task);

    // to get the next higher priority task and run it
    __nanokernel_scheduler.curr_task = NULL;

    __nanokernel_Scheduler_exec();
}

void __nanokernel_Scheduler_Preemptive_changeTaskState( nanokernel_Task_t* task,
                                                        SortedLinkedListWithID_t* from,
                                                        SortedLinkedListWithID_t* to )
{
    // pop task from readyQueue
    // NOTE: be careful from `equal-priority tasks`
    if( task->__EqualPriQueue.next is_not NULL )
        __nanokernel_Task_removeEqualPriTask( task, from );

    else
        SortedLinkedListWithID_popAt( from, task->priority );

    // push it in the blocked queue
    // if it's failed to push the task
    // then there is a another task with the same priority
    if (SortedLinkedListWithID_insert(to, (void*)task, task->priority) is false)
        __nanokernel_Task_addEqualPriTask( task, to );
}

void __nanokernel_Scheduler_blockTask( nanokernel_Task_t* task )
{
    // remove task from ready state into blocked state
    __nanokernel_Scheduler_Preemptive_changeTaskState( task,
                                                       &__nanokernel_scheduler.ready_tasks_queue,
                                                       &__nanokernel_scheduler.blocked_tasks_queue );

    task->state = __BLOCKED;

    // check if we blocked the current task
    if( __nanokernel_scheduler.curr_task is task )
    {
        // to get the next higher priority task and run it
        __nanokernel_Scheduler_exec();
    }
}

void __nanokernel_Scheduler_unblockTask( nanokernel_Task_t* task )
{
    // remove task from blocked state into ready state
    __nanokernel_Scheduler_Preemptive_changeTaskState( task,
                                                       &__nanokernel_scheduler.blocked_tasks_queue,
                                                       &__nanokernel_scheduler.ready_tasks_queue );

    task->state= __READY;

    // run the scheduler
    __nanokernel_Scheduler_exec();
}

void __nanokernel_Scheduler_Preemptive_clean()
{
    SortedLinkedListWithID_clean(&__nanokernel_scheduler.ready_tasks_queue);
}

// TODO: This is conflict in design
// TODO: These 2 functions should be part of `nanokernel_Task`
static void __nanokernel_Task_addEqualPriTask( nanokernel_Task_t *task,
                                               SortedLinkedListWithID_t *queue )
{
    nanokernel_Task_t *same_pri_task =
            SortedLinkedListWithID_getData( queue, task->priority );

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

static void __nanokernel_Task_removeEqualPriTask( nanokernel_Task_t *task,
                                                  SortedLinkedListWithID_t *queue )
{
    nanokernel_Task_t *prev = task;
    nanokernel_Task_t *head = SortedLinkedListWithID_getData( queue,
                                                              task->priority );
    nanokernel_Task_t *task_to_be_terminated = head;

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
        SortedLinkedListWithID_updateData( queue,
                                           head->priority,
                                           head->__EqualPriQueue.next );

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
