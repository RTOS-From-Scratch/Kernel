#ifndef NANOKERNEL_ISR_VECTORTABLE_H_
#define NANOKERNEL_ISR_VECTORTABLE_H_

#include <stdint.h>

void nanokernel_ISR_vectorTable_init();
void nanokernel_ISR_register(uint8_t exception_num, void(*handler)(void));

#endif // NANOKERNEL_ISR_VECTORTABLE_H_
