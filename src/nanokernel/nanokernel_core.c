#include "nanokernel_core.h"
#include <stdint.h>
#include <stdlib.h>
#include "nanokernel.h"

nanokernel_core_TCB *core_TCB;

struct nanokernel_core_TCB {
    int32_t* stack_ptr;
    size_t stack_size;
    void (*scheduler)();
};

//static void nanokernel_init()
//{
//    core_TCB = malloc(sizeof(nanokernel_core_TCB));
////    ASM( ""
////         :
////         :
////         : );

//}
