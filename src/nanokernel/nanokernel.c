#include "nanokernel.h"
#include "nanokernel_scheduler.h"
#include "Misc/src/assert.h"
#include "Misc/src/definitions.h"
#include "inner/__nanokernel_context_switch.h"
#include "Drivers/src/UART.h"
#include "Drivers/src/PLL.h"

void nanokernel_init( byte numberOfTasks )
{
    __nanokernel_States nanokernel_currentState = __nanokernel_getState();
    if( (nanokernel_currentState is __NOT_BOOTED) or
        (nanokernel_currentState is __BOOTED) )
        return;

    // TODO: save old stack first and load the new one
    //    nanokernel_Task_loadStack(task->stack_ptr);

    // initiate the vector table
    __ISR_vectorTable_init();
    // put __nanokernel_Task_contextSwitch in vector table
    __ISR_register( ISR_PEND_SV, __nanokernel_Task_contextSwitch );

    // enable PLL `system clock`
    PLL_setClockSpeed(SYS_CLK_SPEED_IN_MHZ);

#ifdef PC_COMMUNICATION
    // init System UART
    SYS_UART_init();

    #if !defined(NDEBUG)
        // init assert system
        ASSERT_init(SYS_UART_print);
    #endif
#endif

    if(numberOfTasks > 0)
    {
        // initiate the Premptive scheduler
        __nanokernel_Scheduler_Preemptive_init(numberOfTasks);
        // change state from `not initiated` to `not booted`
        __nanokernel_setState(__NOT_BOOTED);
    }

    else
    {
        // that means This is Taskless state
        // just the drivers is working
        __nanokernel_setState(__TASKLESS);
    }
}

void nanokernel_bootup()
{
    if( __nanokernel_getState() is __NOT_INITIATED )
        return;
    else if( __nanokernel_getState() is_not __TASKLESS )
        __nanokernel_setState(__BOOTED);

    // run the scheduler
    __nanokernel_Scheduler_exec();
}
