#ifndef NANOKERNEL_H_
#define NANOKERNEL_H_

#include "tm4c123gh6pm.h"
#include "inner/__nanokernel.h"
#include "nanokernel_ISR_vectortable.h"
#include "nanokernel_task.h"

void nanokernel_init();
// boot up the system ( using the nano kernel )
// it will never return
void nanokernel_bootup();


#endif // NANO_KERNEL_H_
