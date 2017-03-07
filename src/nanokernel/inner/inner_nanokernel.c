#include "inner_nanokernel.h"

static __nanokernel_States nanokernel_state = __NOT_INITIATED;

__nanokernel_States __nanokernel_getState()
{
    return nanokernel_state;
}

void __nanokernel_setState(__nanokernel_States state)
{
    nanokernel_state = state;
}
