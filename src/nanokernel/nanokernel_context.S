.section .text
.ALIGN 0
.thumb
.syntax unified							@ _Force thumb syntax_ `https://sourceware.org/binutils/docs/as/ARM_002dInstruction_002dSet.html#ARM_002dInstruction_002dSet`
.global nanokernel_Task_context
.global nanokernel_Task_loadStack
.global SysTickHandler
.global PendSVHandler
.global nanokernel_Task_enablePSP
.extern nanokernel_scheduler


.thumb_func								@ -- Thumb encoded function
SysTickHandler:							@ 1) Saves R0-R3,R12,LR,PC,PSR
	CPSID   I							@ 2) Prevent interrupt during switch

	MRS		R0, PSP						@ -- load PSP into MSP
	MSR		MSP, R0

	PUSH    {R4-R11}					@ 3) Save remaining regs r4-11
	LDR     R0, =nanokernel_currTask	@ 4) R0=pointer to RunPt, old thread
	LDR     R1, [R0]					@    R1 = current_task
	STR     SP, [R1]					@ 5) Save SP into TCB
	push	{R0, LR}
	BL		nanokernel_scheduler		@ 6) call the scheduler
	pop		{R0, LR}
	LDR     R1, [R0]					@ -- load the the new task
	LDR     SP, [R1]					@ 7) new thread SP@ SP = RunPt->sp
	POP     {R4-R11}					@ 8) restore regs r4-11

	MRS		R0, MSP						@ in exception mode MSP is used
	MSR		PSP, R0						@ -- load MSP into PSP

	CPSIE   I							@ 9) tasks run with interrupts enabled
	BX      LR							@ 10) restore R0-R3,R12,LR,PC,PSR


nanokernel_Task_loadStack:
	MOV		SP, R0						@ load the stack pointer
	POP		{R4-R11}					@ pop R4 -> R11
	POP		{R0-R3}						@ pop R0 -> R3
	POP		{R12}						@ pop R12
	ADD		SP, 4						@ discard LR data
	POP		{LR}						@ pop PC into LR
	ADD		SP, 4						@ discard xPSR data

	push	{R0}						@ -- load MSP into PSP
	MRS		R0, MSP
	MSR		PSP, R0
	MOV		R0, 0x2						@ -- Use PSP for user task
	MSR		CONTROL, R0
	ISB
	pop		{R0}

	BX		LR


nanokernel_Task_enablePSP:
    push	{R0}						@ -- load MSP into PSP
    MRS		R0, MSP
    MSR		PSP, R0
    MOV		R0, 0x2						@ -- Use PSP for user task
    MSR		CONTROL, R0
    ISB
    pop		{R0}

    BX		LR


.thumb_func								@ -- Thumb encoded function
PendSVHandler:
    CPSID   I							@ 2) Prevent interrupt during switch

    MRS		R0, PSP						@ -- load PSP into MSP
    MSR		MSP, R0

	PUSH    {R4-R11}					@ 3) Save remaining regs r4-11


	LDR     R0, =core_TCB				@ 4) R0=pointer to core_TCB
	LDR     R1, [R0]					@    R1 = current_task
	STR     SP, [R1]					@ 5) Save SP into TCB


	LDR		R0, =nanokernel_nextTask
	LDR     R1, [R0]					@ -- load the the new task
	LDR     SP, [R1]					@ 7) new thread SP@ SP = curr_process->sp


	POP     {R4-R11}					@ 8) restore regs r4-11

    MRS		R0, MSP						@ in exception mode MSP is used
    MSR		PSP, R0						@ -- load MSP into PSP

	CPSIE   I							@ 9) tasks run with interrupts enabled
    BX      LR							@ 10) restore R0-R3,R12,LR,PC,PSR
/*    MOV		SP, R0						@ load the stack pointer
    POP		{R4-R11}					@ pop R4 -> R11
    POP		{R0-R3}						@ pop R0 -> R3
    POP		{R12}						@ pop R12
    ADD		SP, 4						@ discard LR data
    POP		{LR}						@ pop PC into LR
    ADD		SP, 4						@ discard xPSR data

    push	{R0}						@ -- load MSP into PSP
    MRS		R0, MSP
    MSR		PSP, R0
    MOV		R0, 0x2						@ -- Use PSP for user task
    MSR		CONTROL, R0
    ISB
    pop		{R0}

    BX		LR*/

.ALIGN
.END

