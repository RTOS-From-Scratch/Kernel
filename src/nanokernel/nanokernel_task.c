#include "nanokernel_task.h"
#include "nanokernel_scheduler.h"
#include <stdlib.h>

// TODO: need to find a prober way
static int8_t id = 0;

//void nanokernel_Task_loadStack( int32_t* curr_stack_ptr );
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
//        "LDR     R0, =curr_task  @ 4) R0=pointer to RunPt, old thread\n\t"
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
    *(task->stack_start - 3)  = (int32_t)__nanokernel_SchedulerPreemptive_endCurrentTask;    // R14 - LR
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

nanokernel_Task_t* nanokernel_Task_create(uint32_t stack_size, int8_t priority, void (*run)())
{
    nanokernel_Task_t *task = malloc(sizeof(nanokernel_Task_t) + stack_size);

    // TODO: do something here
    if( task == NULL )
        return NULL;

    task->stack_end = task->stack;
    task->stack_size = stack_size;
    task->stack_ptr = (int32_t *)( (int8_t *)task->stack + stack_size );
    task->stack_start = task->stack_ptr;
    task->priority = priority;
    task->run = run;

    task->id = id++;

    // init the stack
    __nanokernel_Task_initStack(task);

    // add the created task to the scheduler
    __nanokernel_SchedulerPreemptive_addTask(task);

    return task;
}

void nanokernel_Task_terminate( nanokernel_Task_t *task )
{
    // TODO: need prober way
//    free(task);
}
