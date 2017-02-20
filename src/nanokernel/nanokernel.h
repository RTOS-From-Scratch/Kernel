#ifndef NANOKERNEL_H_
#define NANOKERNEL_H_

#include "tm4c123gh6pm.h"
#include <stdint.h>

#ifdef __GNUC__
    #define ASM __asm__ __volatile__
    #define __NAKED __attribute__((naked))
    #define __NAKED_ISR __attribute__((naked, isr))
    #define NOP ASM("NOP")
#else
    #error "You should use `arm-none-eabi` toolchain"
#endif // __GNUC__

typedef enum priority_t {
    priority_0, // highest priority
    priority_1,
    priority_2,
    priority_3,
    priority_4,
    priority_5,
    priority_6,
    priority_7, // lowest priority
} priority_t;

#endif // NANO_KERNEL_H_
