#include "nanokernel_ISR_vectortable.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "tm4c123gh6pm.h"

#define VT_LEN      155
#define __VT_SECTION__ __attribute__((section("vtable")))
#define __VT_align__ __attribute__((aligned(1024)))

extern void (* const g_pfnVectors[])(void);
typedef void (*VT_handler)(void);
static __VT_SECTION__ VT_handler VectorTable[VT_LEN] __VT_align__;

void nanokernel_ISR_vectorTable_init()
{
    memcpy(VectorTable, g_pfnVectors, sizeof(VT_handler) * VT_LEN);

    // update VTABLE reg to get the new offset
    NVIC_VTABLE_R = (uintptr_t)VectorTable;
}

void nanokernel_ISR_register(uint8_t exception_num, void(*handler)(void))
{
    VectorTable[exception_num] = (VT_handler)handler;
}
