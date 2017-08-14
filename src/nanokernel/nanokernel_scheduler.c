#include "nanokernel_scheduler.h"
#include "nanokernel_task.h"
#include "DataStructures/src/sorted_linkedlist_with_id.h"
#include "nanokernel_task_idle.h"
#include "Drivers/src/inner/__systick.h"
#include <stdlib.h>
#include <string.h>

// PS -> Preemptive Scheduler
#define CONTEXT_SWITCH      NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PEND_SV

typedef struct __TaskQueue_t {
    nanokernel_Task_t* head;
    nanokernel_Task_t* tail;
    byte length;
} __TaskQueue_t;

// This design keep private data more safe
static struct nanokernel_Scheduler_t
{
//    SortedLinkedListWithID_t ready_tasks_queue;
//    SortedLinkedListWithID_t blocked_tasks_queue;
//    SortedLinkedListWithID_t suspended_tasks_queue;
//    SortedLinkedListWithID_Node_t *tasks;

//    __TaskQueue_t readyTasksQueue;
//    __TaskQueue_t blockedTasksQueue;
//    __TaskQueue_t suspendedTasksQueue;

    // 3 here for the three task states
    // ready - blocked - suspended
    __TaskQueue_t taskStateQueues[3];

    nanokernel_Task_t* curr_task;
//    nanokernel_Task_t* curr_equalPriority_task;

    __Scheduler scheduler;

    byte tasks_length;
    nanokernel_Task_t** tasks;
} __nanokernel_scheduler;

// TODO: remove these
//extern void __nanokernel_enableMSP();
//extern void __nanokernel_enablePSP();
//static void __nanokernel_Scheduler_Preemptive_onEqualPriorityTasksExit();
//static void __nanokernel_Scheduler_EquPriTask_add( nanokernel_Task_t *task,
//                                                   __nanokernel_Task_State state );
//static void __nanokernel_Scheduler_EquPriTask_remove( nanokernel_Task_t *task,
//                                                      __nanokernel_Task_State state );
//static void __nanokernel_Scheduler_EquPriTask_setState( nanokernel_Task_t *task,
//                                                        __nanokernel_Task_State from,
//                                                        __nanokernel_Task_State to );
//static bool __nanokernel_Scheduler_Preemptive_shouldExec();
//static void __nanokernel_Scheduler_Preemptive_changeTaskState( nanokernel_Task_t* task,
//                                                               SortedLinkedListWithID_t *from,
//                                                               SortedLinkedListWithID_t *to );

/*****************************************************************************/
static void __nanokernel_Scheduler_changeTaskState( nanokernel_Task_t* task,
                                                    __nanokernel_Task_State to );
/******************************* TaskQ manager *******************************/
static void __TaskQManager_insert( nanokernel_Task_t* task );
static bool __TaskQManager_insertEqualPri( nanokernel_Task_t* task );
static void __TaskQManager_remove( nanokernel_Task_t* task );
static nanokernel_Task_t* __TaskQManager_getNext( nanokernel_Task_t* task );
static nanokernel_Task_t* __TaskQManager_findPrev( nanokernel_Task_t* task );
static bool __TaskQManager_isEmpty( __nanokernel_Task_State state );
static nanokernel_Task_t* __TaskQManager_getHead( __nanokernel_Task_State state );
//static nanokernel_Task_t* __TaskQManager_getTail( __nanokernel_Task_State state );
/*****************************************************************************/

void __nanokernel_Scheduler_init( byte max_tasks_num )
{
    // TODO: This need to be called once
//    __nanokernel_scheduler.tasks = malloc( max_tasks_num *
//                                           sizeof(SortedLinkedListWithID_Node_t) );
//    memset( __nanokernel_scheduler.tasks, 0, max_tasks_num *
//                                             sizeof(SortedLinkedListWithID_Node_t) );
    __nanokernel_scheduler.tasks = malloc( max_tasks_num * sizeof(nanokernel_Task_t **) );
    memset( __nanokernel_scheduler.tasks, 0, max_tasks_num * sizeof(nanokernel_Task_t **) );

    __nanokernel_scheduler.tasks_length = max_tasks_num;

    __nanokernel_scheduler.curr_task = NULL;
//    __nanokernel_scheduler.curr_equalPriority_task = NULL;
    __nanokernel_scheduler.scheduler = NULL;

    // clear the states Queue
    memset( __nanokernel_scheduler.taskStateQueues,
            0,
            3 * sizeof(__TaskQueue_t) );

//    // ready Queue initiation
//    __nanokernel_scheduler.readyTasksQueue.head   = NULL;
//    __nanokernel_scheduler.readyTasksQueue.tail   = NULL;
//    __nanokernel_scheduler.readyTasksQueue.length = 0;

//    // blocked Queue initiation
//    __nanokernel_scheduler.blockedTasksQueue.head   = NULL;
//    __nanokernel_scheduler.blockedTasksQueue.tail   = NULL;
//    __nanokernel_scheduler.blockedTasksQueue.length = 0;

//    // suspended Queue initiation
//    __nanokernel_scheduler.suspendedTasksQueue.head   = NULL;
//    __nanokernel_scheduler.suspendedTasksQueue.tail   = NULL;
//    __nanokernel_scheduler.suspendedTasksQueue.length = 0;

    // these queues operate on the same array `tasks`
    // but the are independant on each other ( different heads/tails )
//    SortedLinkedListWithID_init( __nanokernel_scheduler.tasks,
//                                 max_tasks_num,
//                                 __READY,
//                                 &__nanokernel_scheduler.ready_tasks_queue );

//    SortedLinkedListWithID_init( __nanokernel_scheduler.tasks,
//                                 max_tasks_num,
//                                 __BLOCKED,
//                                 &__nanokernel_scheduler.blocked_tasks_queue );

//    SortedLinkedListWithID_init( __nanokernel_scheduler.tasks,
//                                 max_tasks_num,
//                                 __SUSPENDED,
//                                 &__nanokernel_scheduler.suspended_tasks_queue );
}

void __nanokernel_Scheduler_addTask( nanokernel_Task_t* task )
{
    // TODO: critical section
    // if return false that means that there is already task with the same priority
    // so attack it to the `already task with the same priority`.
//    if (SortedLinkedListWithID_insert(&__nanokernel_scheduler.ready_tasks_queue,
//                                (void*)task, task->priority) is false)
//        __nanokernel_Task_addEqualPriTask( task,
//                                           &__nanokernel_scheduler.ready_tasks_queue );
    __TaskQManager_insert(task);

    // FIXME: you can't just return if not initiated !!
    if( __nanokernel_getState() <= __NOT_BOOTED ) return;

    // check if the added one has higher priority (lower priority value)
    __nanokernel_Scheduler_update();
}

nanokernel_Task_t* __nanokernel_Scheduler_getNextTask()
{
    return __TaskQManager_getNext(__nanokernel_scheduler.curr_task);
}

nanokernel_Task_t *__nanokernel_Scheduler_getCurrentTask()
{
    return __nanokernel_scheduler.curr_task;
}

void __nanokernel_Scheduler_endCurrentTask()
{
//    __nanokernel_enableMSP();

    // FIXME: equal priority Q
//    if( __nanokernel_scheduler.curr_task->__EqualPriQueue.next is_not NULL )
//        __nanokernel_Task_removeEqualPriTask( __nanokernel_scheduler.curr_task,
//                                              &__nanokernel_scheduler.ready_tasks_queue );

//    else
//        SortedLinkedListWithID_popAt( &__nanokernel_scheduler.ready_tasks_queue,
//                                      __nanokernel_scheduler.curr_task->priority );

    __TaskQManager_remove(__nanokernel_scheduler.curr_task);

    // terminate current task
    nanokernel_Task_terminate(__nanokernel_scheduler.curr_task);

    // to get the next higher priority task and run it
    __nanokernel_scheduler.curr_task = NULL;

    __nanokernel_Scheduler_update();
}

void __nanokernel_Scheduler_update()
{
    // Preemptive scheduler
    // TODO: critical section
    // TODO: msp - psp
    // FIXME: what if not initiated ?!

    __nanokernel_States curr_sys_state = __nanokernel_getState();

    if( curr_sys_state is __BOOTED )
    {
        // NOTE: this line will be using when adding cooperative scheduler
        __nanokernel_scheduler.scheduler = __nanokernel_Scheduler_exec;

        // if there is no other tasks
//        if( SortedLinkedListWithID_isEmpty(&__nanokernel_scheduler.ready_tasks_queue) )
        if( __TaskQManager_isEmpty(__READY) )
            nanokernel_Task_idle();

        // this make sure that it's not useless to context switch
        // as may be the current task has already the highest priority
//        else if( __nanokernel_Scheduler_Preemptive_shouldExec() )
        else if( __nanokernel_scheduler.curr_task->priority >= __TaskQManager_getHead(__READY)->priority )
            CONTEXT_SWITCH;
    }

//    else if( curr_sys_state is __TASKLESS )
//        nanokernel_Task_idle();
}

void __nanokernel_Scheduler_exec()
{
    // check if the `onEqualPriorityTasks` premptive scheduler is exited
//    if( __SysTick_isInit() )
//        if( __nanokernel_scheduler.curr_task->__EqualPriQueue.next is NULL )
//            __nanokernel_Scheduler_Preemptive_onEqualPriorityTasksExit();

    // freshly booted or return from terminated task
    if( __nanokernel_scheduler.curr_task is NULL )
    {
        __nanokernel_scheduler.curr_task = __TaskQManager_getHead(__READY);

        // if equal priority tasks exits with the same priority as the current task
//        if( __nanokernel_scheduler.curr_task->__EqualPriQueue.next is_not NULL )
//        {
//            // get next task in the queue of equal priorty
//            __nanokernel_scheduler.curr_equalPriority_task = __nanokernel_scheduler.curr_task;

//            // initiate the timer if it's not initiated
//            if( __SysTick_isInit() is false )
//            {
//                __SysTick_init( __SysTick_getTicks(EQUAL_PRIRITY_TIME_SLICE),
//                                __nanokernel_Scheduler_exec,
//                                __nanokernel_scheduler.curr_task->priority );
//                __SysTick_start(ST_MODE_CONTINOUS);
//            }
//        }
    }

    else
    {
        nanokernel_Task_t * hightestReadyTask = __TaskQManager_getHead(__READY);
//                SortedLinkedListWithID_getHeadData( &__nanokernel_scheduler.ready_tasks_queue);

        // There is a higher priority new task
        if( __nanokernel_scheduler.curr_task->priority > hightestReadyTask->priority )
            __nanokernel_scheduler.curr_task = hightestReadyTask;

        // current task is in EqualPriorityTasksGroup/Suspended/Blocked state
        else if( __nanokernel_scheduler.curr_task->priority EQUAL hightestReadyTask->priority )
            __nanokernel_scheduler.curr_task = __TaskQManager_getNext(
                        __nanokernel_scheduler.curr_task);

        // task which has lower priority value means that it has higher priority
//        if( __nanokernel_scheduler.curr_task->priority > hightestReadyTask->priority )
//        {
//            // get the highest priority task
//            __nanokernel_scheduler.curr_task = hightestReadyTask;
//        }

//        // Equal priority Tasks
//        else if( __nanokernel_scheduler.curr_task->__EqualPriQueue.next is_not NULL )
//        {
//            __nanokernel_scheduler.curr_task = __nanokernel_Scheduler_Preemptive_getNextTask();
//        }

//        else if( __nanokernel_scheduler.curr_task->state is __BLOCKED )
//        {
//            __nanokernel_scheduler.curr_task = __nanokernel_Scheduler_Preemptive_getNextTask();
//        }
    }
}

void __nanokernel_Scheduler_blockTask( nanokernel_Task_t* task )
{
    // remove task from ready state into blocked state
//    __nanokernel_Scheduler_Preemptive_changeTaskState( task,
//                                                       &__nanokernel_scheduler.ready_tasks_queue,
//                                                       &__nanokernel_scheduler.blocked_tasks_queue );
    __nanokernel_Scheduler_changeTaskState( task, __BLOCKED );

    // check if we blocked the current task
    if( __nanokernel_scheduler.curr_task is task )
    {
        __nanokernel_scheduler.curr_task = NULL;
        // to get the next higher priority task and run it
        __nanokernel_Scheduler_update();
    }
}

void __nanokernel_Scheduler_unblockTask( nanokernel_Task_t* task )
{
    // remove task from blocked state into ready state
//    __nanokernel_Scheduler_Preemptive_changeTaskState( task,
//                                                       &__nanokernel_scheduler.blocked_tasks_queue,
//                                                       &__nanokernel_scheduler.ready_tasks_queue );

//    task->state= __READY;

    __nanokernel_Scheduler_changeTaskState( task, __READY );

    // run the scheduler
    __nanokernel_Scheduler_update();
}

void __nanokernel_Scheduler_changeTaskState( nanokernel_Task_t* task,
                                             __nanokernel_Task_State to )
{
    // pop task from readyQueue
    // NOTE: be careful from `equal-priority tasks`

    __TaskQManager_remove(task);
    task->state = to;
    __TaskQManager_insert(task);

//    if( task->__EqualPriQueue.next is_not NULL )
//        __nanokernel_Task_removeEqualPriTask( task, from );

//    else
//        SortedLinkedListWithID_popAt( from, task->priority );

    // push it in the blocked queue
    // if it's failed to push the task
    // then there is a another task with the same priority
//    if (SortedLinkedListWithID_insert(to, (void*)task, task->priority) is false)
//        __nanokernel_Task_addEqualPriTask( task, to );
}

__Scheduler __nanokernel_getScheduler()
{
    return __nanokernel_scheduler.scheduler;
}
/*****************************************************************************/

/********************** Scheduler Task Queue Management **********************/
void __TaskQManager_insert( nanokernel_Task_t* task )
{
    // FIXME: equal priority Q
    __TaskQueue_t *queue = &__nanokernel_scheduler.taskStateQueues[task->state];

    if( queue->head is NULL )
        queue->head = queue->tail = task;

    else if( queue->head->priority > task->priority )
    {
        task->taskManagmenet.nextTask = queue->head;
        queue->head = task;
    }

    else if( queue->tail->priority < task->priority )
    {
        queue->tail->taskManagmenet.nextTask = task;
        queue->tail = task;
    }

    else
    {
        nanokernel_Task_t* prev = __TaskQManager_findPrev( task );
        task->taskManagmenet.nextTask = prev->taskManagmenet.nextTask;
        prev->taskManagmenet.nextTask = task;
    }

    // indexing the task
    // if equal priority group of tasks, adopt it
    if( __nanokernel_scheduler.tasks[task->priority] is NULL )
        __nanokernel_scheduler.tasks[task->priority] = task;

    else
        __TaskQManager_insertEqualPri(task);

    queue->length++;
}

bool __TaskQManager_insertEqualPri( nanokernel_Task_t* task )
{
    __nanokernel_HybridTask_t* hybridTask;
    __HybridTaskManagment* hTaskManagment;

    if( __nanokernel_scheduler.tasks[task->priority] is NULL )
        return false;
    // if this is freshly equal-priority group of tasks
    // then create the hybrid task
    else if( __nanokernel_scheduler.tasks[task->priority]->state is_not __HYBRID )
    {
        hybridTask = __nanokernel_HybridTask_create(task->priority);
        nanokernel_Task_t *head = __nanokernel_scheduler.tasks[task->priority];
        hTaskManagment = &hybridTask->tasksStatesManagement[head->state];

        hTaskManagment->head = hTaskManagment->tail = head;
        hTaskManagment->len++;
    }

    else
    {
        hybridTask = (__nanokernel_HybridTask_t*)__nanokernel_scheduler.tasks[task->priority];
        hTaskManagment = &hybridTask->tasksStatesManagement[task->state];
    }

    hTaskManagment->tail->taskManagmenet.nextEqualPriTask = task;
    task->taskManagmenet.nextEqualPriTask = hTaskManagment->head;

    return true;
}

void __TaskQManager_remove( nanokernel_Task_t* task )
{
    // FIXME: equal priority Q
    __TaskQueue_t *queue = &__nanokernel_scheduler.taskStateQueues[task->state];

//    if( task is NULL )
//        queue->head = queue->tail = task;

//    else if( queue->head is task )
//    {
//        queue->head = task->next;
//    }

    if( queue->head is task )
    {
        nanokernel_Task_t* old_head = queue->head;
        queue->head = queue->head->taskManagmenet.nextTask;
        old_head->taskManagmenet.nextTask = NULL;
    }

    else
    {
        nanokernel_Task_t* prev = __TaskQManager_findPrev( task );

        prev->taskManagmenet.nextTask = task->taskManagmenet.nextTask;
        task->taskManagmenet.nextTask = NULL;

        if( queue->tail is task )
            queue->tail = prev;
    }

    // insert the task
    if( __nanokernel_scheduler.tasks[task->priority]->state is_not __HYBRID )
    {
        __nanokernel_scheduler.tasks[task->priority] = NULL;
    }

    else
    {
        // TODO:
    }

    queue->length--;
}

nanokernel_Task_t* __TaskQManager_getNext( nanokernel_Task_t* task )
{
    if( task->state is_not __HYBRID )
        return task->taskManagmenet.nextTask;
    else
        return task->taskManagmenet.nextEqualPriTask;
}

nanokernel_Task_t* __TaskQManager_findPrev( nanokernel_Task_t* task )
{
    // FIXME: equal priority queue
    byte curr_index = task->priority;
    __TaskQueue_t *queue = &__nanokernel_scheduler.taskStateQueues[task->state];

    if( task is queue->tail )
        return NULL;

    // find the nearest next task that has same state as `task`
    do {
        if(__nanokernel_scheduler.tasks[--curr_index] is_not NULL)
            if( __nanokernel_scheduler.tasks[curr_index]->state is task->state )
                break;
    } while( __nanokernel_scheduler.tasks[curr_index] is_not queue->tail );

    return __nanokernel_scheduler.tasks[curr_index];
}

bool __TaskQManager_isEmpty( __nanokernel_Task_State state )
{
    if( __nanokernel_scheduler.taskStateQueues[state].length > 0 )
        return false;
    else
        return true;
}

nanokernel_Task_t* __TaskQManager_getHead( __nanokernel_Task_State state )
{
    return __nanokernel_scheduler.taskStateQueues[state].head;
}

//nanokernel_Task_t* __TaskQManager_getTail( __nanokernel_Task_State state )
//{
//    return __nanokernel_scheduler.taskStateQueues[state].tail;
//}
/*****************************************************************************/

//void __nanokernel_Scheduler_Preemptive_init()
//{
//    // TODO:
//}

//void __nanokernel_Scheduler_Preemptive_addTask( nanokernel_Task_t* task )
//{

//}

//void __nanokernel_Scheduler_Preemptive_onEqualPriorityTasksExit()
//{
//    __SysTick_stop();
//    __nanokernel_scheduler.curr_equalPriority_task = NULL;
//}

//bool __nanokernel_Scheduler_Preemptive_shouldExec()
//{
//    if( __nanokernel_scheduler.curr_task is_not NULL )
//    {
//        if( __TaskQManager_getNext(__nanokernel_scheduler.curr_task)->priority <=
//                __nanokernel_scheduler.curr_task->priority )
//            return true;

//        else if( __nanokernel_scheduler.curr_task->state is __BLOCKED )
//            return true;

//        else if( __nanokernel_scheduler.curr_task->state is __SUSPENDED )
//            return true;

//        else
//            return false;
//    }

//    return true;
//}

//void __nanokernel_Scheduler_Preemptive()
//{
//    // check if the `onEqualPriorityTasks` premptive scheduler is exited
//    if( __SysTick_isInit() )
//        if( __nanokernel_scheduler.curr_task->__EqualPriQueue.next is NULL )
//            __nanokernel_Scheduler_Preemptive_onEqualPriorityTasksExit();

//    // freshly booted or return from terminated task
//    if( __nanokernel_scheduler.curr_task is NULL )
//    {
//        __nanokernel_scheduler.curr_task = __nanokernel_Scheduler_Preemptive_getNextTask();

//        // if equal priority tasks exits with the same priority as the current task
//        if( __nanokernel_scheduler.curr_task->__EqualPriQueue.next is_not NULL )
//        {
//            // get next task in the queue of equal priorty
//            __nanokernel_scheduler.curr_equalPriority_task = __nanokernel_scheduler.curr_task;

//            // initiate the timer if it's not initiated
//            if( __SysTick_isInit() is false )
//            {
//                __SysTick_init( __SysTick_getTicks(EQUAL_PRIRITY_TIME_SLICE),
//                                __nanokernel_Scheduler_exec,
//                                __nanokernel_scheduler.curr_task->priority );
//                __SysTick_start(ST_MODE_CONTINOUS);
//            }
//        }
//    }

//    else
//    {
//        nanokernel_Task_t * hightestReadyTask =
//                SortedLinkedListWithID_getHeadData( &__nanokernel_scheduler.ready_tasks_queue);

//        // task which has lower priority value means that it has higher priority
//        if( __nanokernel_scheduler.curr_task->priority > hightestReadyTask->priority )
//        {
//            // get the highest priority task
//            __nanokernel_scheduler.curr_task = hightestReadyTask;
//        }

//        // Equal priority Tasks
//        else if( __nanokernel_scheduler.curr_task->__EqualPriQueue.next is_not NULL )
//        {
//            __nanokernel_scheduler.curr_task = __nanokernel_Scheduler_Preemptive_getNextTask();
//        }

//        else if( __nanokernel_scheduler.curr_task->state is __BLOCKED )
//        {
//            __nanokernel_scheduler.curr_task = __nanokernel_Scheduler_Preemptive_getNextTask();
//        }
//    }
//}

//nanokernel_Task_t* __nanokernel_Scheduler_Preemptive_getNextTask()
//{
//    if( __nanokernel_scheduler.curr_equalPriority_task is_not NULL )
//        return __nanokernel_scheduler.curr_equalPriority_task =
//                __nanokernel_scheduler.curr_task =
//                 __nanokernel_scheduler.curr_task->__EqualPriQueue.next;

//    else
//        return SortedLinkedListWithID_getHeadData(&__nanokernel_scheduler.ready_tasks_queue);
//}

//void __nanokernel_Scheduler_Preemptive_endCurrentTask()
//{
////    __nanokernel_enableMSP();

//    // TODO: remove from the queue of tasks
//    // TODO: remove from blocked queue
//    if( __nanokernel_scheduler.curr_task->__EqualPriQueue.next is_not NULL )
//        __nanokernel_Task_removeEqualPriTask( __nanokernel_scheduler.curr_task,
//                                              &__nanokernel_scheduler.ready_tasks_queue );

//    else
//        SortedLinkedListWithID_popAt( &__nanokernel_scheduler.ready_tasks_queue,
//                                      __nanokernel_scheduler.curr_task->priority );

//    // terminate current task
//    nanokernel_Task_terminate(__nanokernel_scheduler.curr_task);

//    // to get the next higher priority task and run it
//    __nanokernel_scheduler.curr_task = NULL;

//    __nanokernel_Scheduler_exec();
//}

//void __nanokernel_Scheduler_Preemptive_changeTaskState( nanokernel_Task_t* task,
//                                                        SortedLinkedListWithID_t* from,
//                                                        SortedLinkedListWithID_t* to )
//{
//    // pop task from readyQueue
//    // NOTE: be careful from `equal-priority tasks`
//    if( task->__EqualPriQueue.next is_not NULL )
//        __nanokernel_Task_removeEqualPriTask( task, from );

//    else
//        SortedLinkedListWithID_popAt( from, task->priority );

//    // push it in the blocked queue
//    // if it's failed to push the task
//    // then there is a another task with the same priority
//    if (SortedLinkedListWithID_insert(to, (void*)task, task->priority) is false)
//        __nanokernel_Task_addEqualPriTask( task, to );
//}

//void __nanokernel_Scheduler_Preemptive_clean()
//{
//    SortedLinkedListWithID_clean(&__nanokernel_scheduler.ready_tasks_queue);
//}

///************************* Equal Priority Tasks Scheduler *************************/

//// TODO: This is conflict in design
//// TODO: These 2 functions should be part of `nanokernel_Task`
//void __nanokernel_Scheduler_EquPriTask_add( nanokernel_Task_t *task,
//                                            __nanokernel_Task_State state )
//{
////    nanokernel_Task_t *head =
////            SortedLinkedListWithID_getData( state, task->priority );

////    // this task is the first one that has same priority
////    if( head->__EqualPriQueue.next is NULL )
////        head->__EqualPriQueue.next = task;

////    // there are more than one task with the same priority
////    else
////        // update the tail `next_equal_pri` task in this queue
////        head->__EqualPriQueue.tail->__EqualPriQueue.next = task;

////    // update `tail` in the `head` task
////    head->__EqualPriQueue.tail = task;
////    task->__EqualPriQueue.next = head;
//    nanokernel_Task_t *head = SortedLinkedListWithID_getData(
//                &__nanokernel_scheduler.ready_tasks_queue, task->priority);

//    if( head->state is_not __HYBRID )
//    {
//        __nanokernel_HybridTask_t *hTask = __nanokernel_HybridTask_create(task->priority);

//        if(head->state is __READY)
//        {
//            head = SortedLinkedListWithID_popAt( &__nanokernel_scheduler.ready_tasks_queue,
//                                                 head->priority );
//            SortedLinkedListWithID_push()
//        }

//        else if(head->state is __BLOCKED)
//        {
//            head = SortedLinkedListWithID_popAt( &__nanokernel_scheduler.blocked_tasks_queue,
//                                                 head->priority );
//        }

//        else if(head->state is __SUSPENDED)
//        {
//            head = SortedLinkedListWithID_popAt( &__nanokernel_scheduler.suspended_tasks_queue,
//                                                 head->priority );
//        }
//    }

//    if( state is __READY )
//    {

//    }

//    else if( state is __BLOCKED )
//    {

//    }

//    else if( state is __SUSPENDED )
//    {

//    }
//}

//void __nanokernel_Scheduler_EquPriTask_remove(nanokernel_Task_t *task,
//                                               __nanokernel_Task_State state )
//{
//    nanokernel_Task_t *prev = task;
//    nanokernel_Task_t *head = SortedLinkedListWithID_getData( state,
//                                                              task->priority );
//    nanokernel_Task_t *task_to_be_terminated = head;

//    // search for the task id in this queue
//    while( (task_to_be_terminated->id) is_not task->id )
//    {
//        prev = task_to_be_terminated;
//        task_to_be_terminated = task_to_be_terminated->__EqualPriQueue.next;
//    }

//    // if we want to remove the first element in this queue
//    if( task_to_be_terminated is head )
//    {
//        // just update the first element data in this queue,
//        // to hold the `next_equal_pri` task
//        SortedLinkedListWithID_updateData( state,
//                                           head->priority,
//                                           head->__EqualPriQueue.next );

//        // get the new head
//        head = head->__EqualPriQueue.next;

//        // check if there is only 1 remaining task in the queue after updating
//        if( head is task->__EqualPriQueue.tail )
//        {
//            head->__EqualPriQueue.next = NULL;
//            head->__EqualPriQueue.tail = NULL;
//        }

//        else
//        {
//            // update the `tail` of the new `head` task
//            head->__EqualPriQueue.tail = task->__EqualPriQueue.tail;
//            head->__EqualPriQueue.tail->__EqualPriQueue.next = head;
//        }
//    }

//    else
//    {
//        // this is the next task of the head is the tail task
//        if( head->__EqualPriQueue.next is head->__EqualPriQueue.tail )
//        {
//            head->__EqualPriQueue.next = NULL;
//            head->__EqualPriQueue.tail = NULL;
//        }

//        else
//        {
//            // find the wanted task and its previous task
////            while( task_to_be_terminated->id is_not task->id )
////            {
////                task_to_be_terminated = task_to_be_terminated->__EqualPriQueue.next;
////                prev = task_to_be_terminated;
////            }

//            prev->__EqualPriQueue.next = task_to_be_terminated->__EqualPriQueue.next;
//        }
//    }
//}
