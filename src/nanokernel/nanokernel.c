#include "nanokernel.h"
#include "nanokernel_scheduler.h"
#include "Misc/src/assert.h"
#include "inner/__nanokernel_context_switch.h"

// TODO: for sure this need to be handled
#define MAX_PROCESSES_NUM 10


void nanokernel_init()
{
    // TODO: need to be called only once

    // TODO: save old stack first and load the new one
    //    nanokernel_Task_loadStack(task->stack_ptr);

    // TODO: better way ?
    __nanokernel_Scheduler_Preemptive_init(MAX_PROCESSES_NUM);
    // initiate the vector table
    __nanokernel_ISR_vectorTable_init();
    // put __nanokernel_Task_contextSwitch in vector table
    __nanokernel_ISR_register( ISR_PEND_SV, __nanokernel_Task_contextSwitch );

#ifdef PC_COMMUNICATION
    __SYS_UART_init();
#endif
    // init assert system
#if !defined(NDEBUG) && defined(PC_COMMUNICATION)
    ASSERT_init(SYS_UART_writeLine);
#endif

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
