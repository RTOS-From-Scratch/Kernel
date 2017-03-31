.section .text
.ALIGN 0
.thumb
.syntax unified                            @ Force thumb syntax - `https://sourceware.org/binutils/docs/as/ARM_002dInstruction_002dSet.html#ARM_002dInstruction_002dSet`
.global nanokernel_Task_loadStack
.global __nanokernel_Task_contextSwitch
.global nanokernel_enablePSP
.global nanokernel_enableMSP

nanokernel_Task_loadStack:
    MOV     SP, R0                          @ load the stack pointer
    POP     {R4-R11}                        @ pop R4 -> R11
    POP     {R0-R3}                         @ pop R0 -> R3
    POP     {R12}                           @ pop R12
    ADD     SP, 4                           @ discard LR data
    POP     {LR}                            @ pop PC into LR
    ADD     SP, 4                           @ discard xPSR data
    BX      LR

/************************* Process Stack Pointer *************************/
nanokernel_enablePSP:
/*    push    {R0-R1}                         @ -- load MSP into PSP*/
    MRS     R0, MSP
    MSR     PSP, R0
    MOV     R0, 0x2                         @ -- Use PSP for user task
    MRS     R1, CONTROL
    ORR     R1, R0
    MSR     CONTROL, R1
    ISB
/*    pop     {R0-R1}*/
    BX      LR
/*************************************************************************/

/*************************** Main Stack Pointer ***************************/
nanokernel_enableMSP:
/*    push    {R0-R1}                         @ -- load PSP into MSP*/
    MRS     R0, PSP
    MSR     MSP, R0
    MOV     R0, 0x2                         @ -- Use MSP for OS task
    MRS     R1, CONTROL
    ORN     R1, R0                          @ -- complement R0 and then or it with R1
    MSR     CONTROL, R1
    ISB
/*    pop     {R0-R1}*/
    BX      LR
/**************************************************************************/

/****************************** Context switch ****************************/
.thumb_func                                 @ -- Thumb encoded function
__nanokernel_Task_contextSwitch:
    CPSID   I                               @ 2) Prevent interrupt during switch

   /* MRS     R0, PSP                         @ -- update MSP
    MSR     MSP, R0*/

    PUSH    {R4-R11}                        @ 3) Save remaining regs r4-11

    @ TODO: use OS stack

    push    {LR}
    BL  __nanokernel_Scheduler_getCurrentTask @ -- get current task in R0
    pop     {LR}

    CBZ     R0, ON_NULL_CUR_TASK            @ branch if cuurent task is NULL
    STR     SP, [R0]                        @ -- Save SP into TCB

    ON_NULL_CUR_TASK:
    push    {LR}
    BL  __nanokernel_getScheduler
    /* go to the function dereferenced from the returned pointer of function */
    BLX     R0                              @ -- run the scheduler

    BL  __nanokernel_Scheduler_getCurrentTask @ -- get current task in R0
    pop     {LR}

    LDR     SP, [R0]

    POP     {R4-R11}                        @ 8) restore regs r4-11

    /*MRS     R0, MSP                         @ -- update MSP
    MSR     PSP, R0                         @ in exception mode MSP is used*/

    CPSIE   I                               @ 9) tasks run with interrupts enabled
    BX      LR                              @ 10) restore R0-R3,R12,LR,PC,PSR

            /******************************************************/

/*.thumb_func                                 @ -- Thumb encoded function
SysTickHandler:                             @ 1) Saves R0-R3,R12,LR,PC,PSR
    CPSID   I                               @ 2) Prevent interrupt during switch

    BL nanokernel_updateMSP

    PUSH    {R4-R11}                        @ 3) Save remaining regs r4-11
    LDR     R0, =nanokernel_currTask        @ 4) R0=pointer to curr_process, old thread
    LDR     R1, [R0]                        @    R1 = current_task
    STR     SP, [R1]                        @ 5) Save SP into TCB
    push    {R0, LR}
    BL      nanokernel_scheduler            @ 6) call the scheduler
    pop     {R0, LR}
    LDR     R1, [R0]                        @ -- load the the new task
    LDR     SP, [R1]                        @ 7) new thread SP@ SP = curr_process->sp
    POP     {R4-R11}                        @ 8) restore regs r4-11

    BL nanokernel_updatePSP               @ in exception mode MSP is used

    CPSIE   I                               @ 9) tasks run with interrupts enabled
    BX      LR                              @ 10) restore R0-R3,R12,LR,PC,PSR*/
/**************************************************************************/

.ALIGN
.END

