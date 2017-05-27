#include "nanokernel_task.h"
#include "nanokernel_scheduler.h"
#include "Drivers/src/Timer.h"
#include "Drivers/src/inner/__IO.h"
#include <stdlib.h>
#include "inner/__nanokernel_task.h"
#include "Misc/src/definitions.h"
#include "Drivers/src/UART.h"

static byte id = 0;

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
    // register `s` is for floating-point registers
    // This is simillar to the interrupt arrangement
    *(task->stack_start - 1)  = 0x00000000;    // empty gap
    *(task->stack_start - 2)  = 0x00000000;    // FPSCR
    *(task->stack_start - 3)  = 0x15151515;    // s15
    *(task->stack_start - 4)  = 0x14141414;    // s14
    *(task->stack_start - 5)  = 0x13131313;    // s13
    *(task->stack_start - 6)  = 0x12121212;    // s12
    *(task->stack_start - 7)  = 0x11111111;    // s11
    *(task->stack_start - 8)  = 0x10101010;    // s10
    *(task->stack_start - 0)  = 0x09090909;    // s9
    *(task->stack_start - 10) = 0x08080808;    // s8
    *(task->stack_start - 11) = 0x07070707;    // s7
    *(task->stack_start - 12) = 0x06060606;    // s6
    *(task->stack_start - 13) = 0x05050505;    // s5
    *(task->stack_start - 14) = 0x04040404;    // s4
    *(task->stack_start - 15) = 0x03030303;    // s3
    *(task->stack_start - 16) = 0x02020202;    // s2
    *(task->stack_start - 17) = 0x01010101;    // s1
    *(task->stack_start - 18) = 0x00000000;    // s0
    *(task->stack_start - 19) = 0x01000000;    // Thumb bit - xPSR
    *(task->stack_start - 20) = (intptr_t)task->nanokernel_Task_entry;    // PC
    *(task->stack_start - 21) = (intptr_t)__nanokernel_Scheduler_Preemptive_endCurrentTask;    // R14 - LR
    *(task->stack_start - 22) = 0x12121212;    // R12
    *(task->stack_start - 23) = 0x03030303;    // R3
    *(task->stack_start - 24) = 0x02020202;    // R2
    *(task->stack_start - 25) = 0x01010101;    // R1
    // the task paramter go through R0
    *(task->stack_start - 26) = (uintptr_t)task->parameter;    // R0
    *(task->stack_start - 27) = 0x11111111;    // R11
    *(task->stack_start - 28) = 0x10101010;    // R10
    *(task->stack_start - 29) = 0x09090909;    // R9
    *(task->stack_start - 30) = 0x08080808;    // R8
    *(task->stack_start - 31) = 0x07070707;    // R7
    *(task->stack_start - 32) = 0x06060606;    // R6
    *(task->stack_start - 33) = 0x05050505;    // R5
    *(task->stack_start - 34) = 0x04040404;    // R4
    *(task->stack_start - 35) = 0x31313131;    // s31
    *(task->stack_start - 36) = 0x30303030;    // s30
    *(task->stack_start - 37) = 0x29292929;    // s29
    *(task->stack_start - 38) = 0x28282828;    // s28
    *(task->stack_start - 39) = 0x27272727;    // s27
    *(task->stack_start - 40) = 0x26262626;    // s26
    *(task->stack_start - 41) = 0x25252525;    // s25
    *(task->stack_start - 42) = 0x24242424;    // s24
    *(task->stack_start - 43) = 0x23232323;    // s23
    *(task->stack_start - 44) = 0x22222222;    // s22
    *(task->stack_start - 45) = 0x21212121;    // s21
    *(task->stack_start - 46) = 0x20202020;    // s20
    *(task->stack_start - 47) = 0x19191919;    // s19
    *(task->stack_start - 48) = 0x18181818;    // s18
    *(task->stack_start - 49) = 0x17171717;    // s17
    *(task->stack_ptr = task->stack_start - 50) = 0x16161616;    // s16
}

nanokernel_Task_t* nanokernel_Task_create( size_t stack_len,
                                           Priority_t priority,
                                           void (*nanokernel_Task_entry)(void*),
                                           void *task_paramter,
                                           byte maxNumberOfDrivers )
{
    // stack_len should be aligned by the length of pointer
    size_t stack_size = stack_len * sizeof(intptr_t *);

    nanokernel_Task_t *task = malloc( sizeof(nanokernel_Task_t) +
                                      stack_size +
                                      (maxNumberOfDrivers * sizeof(__nanokernel_Task_Driver)) );

    // TODO: do something here
    if( task == NULL )
        return NULL;

    if( maxNumberOfDrivers EQUAL 0 )
        task->__HoldedDrivers.list = NULL;
    else
        task->__HoldedDrivers.list = (__nanokernel_Task_Driver *)task->chunkOfMemory;

    task->__HoldedDrivers.len = maxNumberOfDrivers;
    task->__HoldedDrivers.currentIndex = -1;

    task->stack = (intptr_t*)( (int8_t *)task->chunkOfMemory +
                               (maxNumberOfDrivers * sizeof(__nanokernel_Task_Driver)) );
    task->stack_end = task->stack;
    task->stack_size = stack_size;
    task->stack_ptr = (intptr_t *)( (int8_t *)task->stack + stack_size );
    task->stack_start = task->stack_ptr;
    task->priority = priority;
    task->nanokernel_Task_entry = nanokernel_Task_entry;
    task->parameter = task_paramter;
    task->id = id++;

    // this will hold the next equal priority task if exists
    // they will act like a closed circle, each task point to the next
    task->__EqualPriQueue.next   = NULL;
    // only the head task has the data about the tail
    task->__EqualPriQueue.tail = NULL;

    // init the stack
    __nanokernel_Task_initStack(task);

    // add the created task to the scheduler
    __nanokernel_Scheduler_Preemptive_addTask(task);

    return task;
}

Driver* nanokernel_Task_requestDriver( DriverName driverName, Module module )
{
    int current_task_id = nanokernel_Task_getID();
    Driver *driver;
    __nanokernel_Task_Driver *task_driver;

    // FIXME: do something if the driver is not available
    if( Driver_isAvailable( driverName, module ) is false )
        return NULL;

    if( current_task_id is_not TASKLESS )
    {
         nanokernel_Task_t* task = __nanokernel_Scheduler_getCurrentTask();

        // reaches maximum number of holded drivers
        // TODO: do something here
        if(task->__HoldedDrivers.currentIndex EQUAL (task->__HoldedDrivers.len - 1))
           return NULL;

        driver = Driver_construct( driverName, module );

        // get the next empty slot for new driver
        task_driver = &task->__HoldedDrivers.list[++(task->__HoldedDrivers.currentIndex)];
        // set the task driver data
        task_driver->driverName = driverName;
        task_driver->driver = Driver_construct( driverName, module );
    }

    else
    {
        driver = Driver_construct( driverName, module );
    }

    return driver;
}

void   nanokernel_Task_releaseDriver( DriverName driverName, Driver* driver )
{
    /* This function called only explicitly by the user at any time
     * before the task is finished */
    int current_task_id = nanokernel_Task_getID();
    nanokernel_Task_t* task;

    // if id is not equal TASKLESS
    if( current_task_id is_not TASKLESS )
    {
        task = __nanokernel_Scheduler_getCurrentTask();

        __nanokernel_Task_Driver *list = task->__HoldedDrivers.list;

        // TODO: need better way to the search for the needed driver to be removed
        for(uint8_t driver_index = 0; driver_index < task->__HoldedDrivers.len; ++driver_index)
        {
            if( list[driver_index].driverName is driverName )
            {
                Driver_deinit( driverName, list[driver_index].driver );
                // move the last element in the place of the removed driver
                list[task->__HoldedDrivers.currentIndex--] = list[task->__HoldedDrivers.len - 1];
                break;
            }
        }
    }

    else
        Driver_deinit( driverName, driver );
}

TaskID nanokernel_Task_getID()
{
    //FIXME: critical section
    // return ID of the current task
    nanokernel_Task_t *current_task = __nanokernel_Scheduler_getCurrentTask();
    if( current_task is NULL )
        return TASKLESS;
    else
        return __nanokernel_Scheduler_getCurrentTask()->id;
}

void nanokernel_Task_terminate( nanokernel_Task_t *task )
{
    // TODO: critical section
    __nanokernel_Task_Driver *list = task->__HoldedDrivers.list;

    // release the drivers
    for(uint8_t driver_index = 0;
        driver_index <= task->__HoldedDrivers.currentIndex;
        driver_index++)
    {
        // call the deinit function
        Driver_deinit( list[driver_index].driverName,
                       list[driver_index].driver );
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
//    PeriodicTask = task;

//    Timer_init(TIMER0_A_PORTB, __periodic_timer, __DOWN, value);
//    IO_REG(NVIC_R, PRI4) |= (IO_REG(NVIC_R, PRI4)&0x00FFFFFF)|0x80000000;
//    IO_REG(NVIC_R, EN0) |= (1<<19);
//    IO_REG(__Timer_Addr[0], TIMER_CTL_R) |= (1<< 0);

    //TODO: END CRITICAL
    //restore I bit and enable interrupts
}
