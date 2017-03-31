#ifndef INNER_NANOKERNEL_H_
#define INNER_NANOKERNEL_H_

#ifdef __GNUC__
    #define ASM __asm__ __volatile__
    #define __NAKED __attribute__((naked))
    #define __NAKED_ISR __attribute__((naked, isr))
    #define NOP ASM("NOP")
#else
    #error "You should use `arm-none-eabi` toolchain"
#endif // __GNUC__

enum __nanokernel_States {
    __NOT_INITIATED = -2,
    __NOT_BOOTED = -1,
    __BOOTED,
    __IDLE,
};

typedef enum __nanokernel_States __nanokernel_States;

__nanokernel_States __nanokernel_getState();
void __nanokernel_setState(__nanokernel_States state);

#endif // INNER_NANOKERNEL_TASK_H_
