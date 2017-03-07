#include "nanokernel.h"
#include "nanokernel_scheduler.h"

// TODO: for sure this need to be handled
#define MAX_PROCESSES_NUM 10

void nanokernel_init()
{
    // TODO: need to be called only once

    // TODO: save old stack first and load the new one
    //    nanokernel_Task_loadStack(task->stack_ptr);

    // TODO: better way ?
    __nanokernel_SchedulerPreemptive_init(MAX_PROCESSES_NUM);

    // change state from `not initiated` to `not booted`
    __nanokernel_setState(__NOT_BOOTED);
}

void nanokernel_bootup()
{
    // FIXME: you can't just return !!
    if( __nanokernel_getState() == __NOT_INITIATED )
        return;

    __nanokernel_setState(__BOOTED);

    // run the scheduler
    __nanokernel_SchedulerPreemptive_run();
}