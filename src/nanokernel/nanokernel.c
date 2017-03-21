#include "nanokernel.h"
#include "nanokernel_scheduler.h"

// TODO: for sure this need to be handled
#define MAX_PROCESSES_NUM 10

// This is declared in `nanokernel_context.S`
extern void __nanokernel_Task_contextSwitch();

void nanokernel_init()
{
    // TODO: need to be called only once

    // TODO: save old stack first and load the new one
    //    nanokernel_Task_loadStack(task->stack_ptr);

    // TODO: better way ?
    // initiate the vector table
    __nanokernel_ISR_vectorTable_init();
    // put __nanokernel_Task_contextSwitch in vector table
    __nanokernel_ISR_register( ISR_PEND_SV, __nanokernel_Task_contextSwitch );

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
    __nanokernel_Scheduler_Preemptive_run();
}
