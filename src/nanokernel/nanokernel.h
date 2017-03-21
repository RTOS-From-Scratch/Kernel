#ifndef NANOKERNEL_H_
#define NANOKERNEL_H_

#include "tm4c123gh6pm.h"
#include "inner/inner_nanokernel.h"
#include "inner/inner_nanokernel_task.h"
#include "nanokernel_ISR_vectortable.h"

typedef enum Priority_t {
    PRIORITY_0, // highest priority
    PRIORITY_1,
    PRIORITY_2,
    PRIORITY_3,
    PRIORITY_4,
    PRIORITY_5,
    PRIORITY_6,
    PRIORITY_7, // lowest priority
} Priority_t;

void nanokernel_init();
// boot up the system ( using the nano kernel )
// it will never return
void nanokernel_bootup();

#include "nanokernel_task.h"

#endif // NANO_KERNEL_H_
