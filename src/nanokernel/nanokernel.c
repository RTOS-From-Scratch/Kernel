#include "nanokernel.h"
#include "nanokernel_scheduler.h"
#include "Misc/src/assert.h"
#include "Misc/src/definitions.h"
#include "inner/__nanokernel_context_switch.h"
#include "Drivers/src/UART.h"

void nanokernel_init()
{
    // TODO: need to be called only once

    // TODO: save old stack first and load the new one
    //    nanokernel_Task_loadStack(task->stack_ptr);

    // TODO: better way ?
    __nanokernel_Scheduler_Preemptive_init(NUM_OF_TASKS);
    // initiate the vector table
    __ISR_vectorTable_init();
    // put __nanokernel_Task_contextSwitch in vector table
    __ISR_register( ISR_PEND_SV, __nanokernel_Task_contextSwitch );

#ifdef PC_COMMUNICATION
    // init System UART
    SYS_UART_init();

    #if !defined(NDEBUG)
        // init assert system
        ASSERT_init(SYS_UART_print);
    #endif
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
    __nanokernel_Scheduler_exec();
}
