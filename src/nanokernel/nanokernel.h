#ifndef NANOKERNEL_H_
#define NANOKERNEL_H_

#include "tm4c123gh6pm.h"
#include "inner/inner_nanokernel.h"
#include "inner/inner_nanokernel_task.h"

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

void nanokernel_init();
// boot up the system ( using the nano kernel )
// it will never return
void nanokernel_bootup();

#include "nanokernel_task.h"

#endif // NANO_KERNEL_H_
