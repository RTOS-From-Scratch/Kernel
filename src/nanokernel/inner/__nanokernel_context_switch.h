#ifndef NANOKERNEL_CONTEXT_SWITCH
#define NANOKERNEL_CONTEXTSWITCH

#include <stdint.h>

// These are declared in `nanokernel_context_switch.S`
void __nanokernel_Task_loadStack(intptr_t* stack);
void __nanokernel_Task_contextSwitch();
void __nanokernel_enablePSP();
void __nanokernel_enableMSP();

#endif // NANOKERNEL_CONTEXTSWITCH

