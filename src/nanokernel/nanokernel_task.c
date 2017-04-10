#include "nanokernel_task.h"
#include "nanokernel_scheduler.h"
#include "Drivers/src/Timer.h"
#include "Drivers/src/inner/__IO.h"
#include <stdlib.h>
#include "inner/__nanokernel_task.h"
#include "Misc/src/definitions.h"
#include "Drivers/src/inner/__driver.h"

static int8_t id = 0;
static nanokernel_Task_t* __nanokernel_Tasks[NUM_OF_TASKS];

extern void nanokernel_Task_enablePSP();
static void __nanokernel_Task_initStack( nanokernel_Task_t* task );

/* By default GNU ARM compiler will store and restore a Frame Pointer
 * using "r7" and do stack alignment when entering into and exiting functions.
 * To avoid such optimisations we need to declare our handlers as "naked".
 */
//__NAKED_ISR void  SysTickHandler()
//{
//    ASM (
//        "CPSID   I                  @ 2) Prevent interrupt during switch\n\t"
//        "PUSH    {R4-R11}           @ 3) Save remaining regs r4-11\n\t"
//        "LDR     R0, =curr_task     @ 4) R0=pointer to RunPt, old thread\n\t"
//        "LDR     R1, [R0]           @    R1 = RunPt\n\t"
//        "STR     SP, [R1]           @ 5) Save SP into TCB\n\t"
//        "LDR     R1, [R1,#24]       @ 6) R1 = RunPt->next\n\t"
//        "STR     R1, [R0]           @    RunPt = R1\n\t"
//        "LDR     SP, [R1]           @ 7) new thread SP@ SP = RunPt->sp\n\t"
//        "POP     {R4-R11}           @ 8) restore regs r4-11\n\t"
//        "CPSIE   I                  @ 9) tasks run with interrupts enabled\n\t"
//        "BX      LR                 @ 10) restore R0-R3,R12,LR,PC,PSR"
//        );
//}

void __nanokernel_Task_initStack( nanokernel_Task_t* task )
{
    // The stack intialized by this way for debugging purpose
    // R4-R11 -> R0-R3 -> R12 -> LR -> PC -> xPSR (Push)
    // This is simillar to the interrupt arrangement
    *(task->stack_start - 1)  = 0x01000000;    // Thumb bit - xPSR
    *(task->stack_start - 2)  = (int32_t)task->run;    // PC
    *(task->stack_start - 3)  = (int32_t)__nanokernel_Scheduler_Preemptive_endCurrentTask;    // R14 - LR
    *(task->stack_start - 4)  = 0x12121212;    // R12
    *(task->stack_start - 5)  = 0x03030303;    // R3
    *(task->stack_start - 6)  = 0x02020202;    // R2
    *(task->stack_start - 7)  = 0x01010101;    // R1
    *(task->stack_start - 8)  = 0x00000000;    // R0
    *(task->stack_start - 9)  = 0x11111111;    // R11
    *(task->stack_start - 10) = 0x10101010;    // R10
    *(task->stack_start - 11) = 0x09090909;    // R9
    *(task->stack_start - 12) = 0x08080808;    // R8
    *(task->stack_start - 13) = 0x07070707;    // R7
    *(task->stack_start - 14) = 0x06060606;    // R6
    *(task->stack_start - 15) = 0x05050505;    // R5
    *(task->stack_ptr = task->stack_start - 16) = 0x04040404;    // R4
}

nanokernel_Task_t* nanokernel_Task_create(size_t stack_len, Priority_t priority,
                                          void (*run)(), uint8_t maxNumberOfDrivers)
{
    nanokernel_Task_t *task = malloc(sizeof(nanokernel_Task_t) +
                                     stack_len +
                                     (maxNumberOfDrivers * sizeof(Driver)));

    // TODO: do something here
    if( task == NULL )
        return NULL;

    task->__HoldedDrivers.list = (Driver **)task->chunkOfMemory;
    task->__HoldedDrivers.len = maxNumberOfDrivers;
    task->__HoldedDrivers.currentIndex = -1;

    task->stack = (intptr_t*)((int8_t *)task->__HoldedDrivers.list + (maxNumberOfDrivers * sizeof(Driver)));
    task->stack_end = task->stack;
    task->stack_size = stack_len;
    task->stack_ptr = (intptr_t *)( (int8_t *)task->stack + stack_len );
    task->stack_start = task->stack_ptr;
    task->priority = priority;
    task->run = run;
    task->id = id;

    // init the stack
    __nanokernel_Task_initStack(task);

    // add the new task to the array of tasks
    __nanokernel_Tasks[id++] = task;

    // add the created task to the scheduler
    __nanokernel_Scheduler_Preemptive_addTask(task);

    return task;
}

Driver* nanokernel_Task_requestDriver( DriverName driverName, Module module )
{
    // FIXME: do something if the driver is not available
    if( Driver_isAvailable( driverName, module ) is false )
        return NULL;

    Driver *driver;

    if( id is_not TASKLESS )
    {
         nanokernel_Task_t* task = __nanokernel_Scheduler_getCurrentTask();

        // reaches maximum number of holded drivers
        // TODO: do something here
        if(task->__HoldedDrivers.currentIndex EQUAL (task->__HoldedDrivers.len - 1))
           return NULL;

        driver = Driver_construct( driverName, module );
        task->__HoldedDrivers.list[++task->__HoldedDrivers.currentIndex] = driver;
    }

    else
    {
        driver = Driver_construct( driverName, module );
    }

    return driver;
}

void   nanokernel_Task_releaseDriver( Driver* driver )
{
    // if id is not equal zero
    if( id is_not TASKLESS )
    {
        nanokernel_Task_t* task = __nanokernel_Tasks[id];
        Driver **list = task->__HoldedDrivers.list;

        // TODO: need better way to the search for the needed driver to be removed
        for(uint8_t driver_index = 0; driver_index < task->__HoldedDrivers.len; ++driver_index)
        {
            if( (list[driver_index]->driverName is driver->driverName) and
                    (list[driver_index]->config is driver->config) )
            {
                // move the last element in the place of the removed driver
                Driver_deinit( list[driver_index] );
                list[task->__HoldedDrivers.currentIndex--] = list[task->__HoldedDrivers.len - 1];
                break;
            }
        }
    }

    else
        Driver_deinit( driver );
}

TaskID nanokernel_Task_getID()
{
    //FIXME: critical section
    // return ID of the current task
    return __nanokernel_Scheduler_getCurrentTask()->id;
}

void nanokernel_Task_terminate( nanokernel_Task_t *task )
{
    // TODO: critical section
    Driver **list = task->__HoldedDrivers.list;

    // release the drivers
    for(uint8_t driver_index = 0;
        driver_index <= task->__HoldedDrivers.currentIndex;
        driver_index++)
    {
        // call the deinit function
        Driver_deinit(list[driver_index]);
    }

    task->__HoldedDrivers.currentIndex = -1;
    // TODO: need prober way
//    free(task);
}

void nanokernel_Task_delayedStart(void(*task)(void), uint32_t value)
{
    //TODO: START CRITICAL
    //store I bit and disable interrupts

    //user function
    PeriodicTask = task;

    Timer_init(TIMER0_A_PORTB, __periodic_timer, __DOWN, value);
//    IO_REG(NVIC_R, PRI4) |= (IO_REG(NVIC_R, PRI4)&0x00FFFFFF)|0x80000000;
    IO_REG(NVIC_R, EN0) |= (1<<19);
    IO_REG(__Timer_Addr[0], TIMER_CTL_R) |= (1<< 0);

    //TODO: END CRITICAL
    //restore I bit and enable interrupts
}
