#include "nanokernel_task_idle.h"
#include "Drivers/src/ISR_ctrl.h"
#include <stdbool.h>

void nanokernel_Task_idle()
{
    // suspend execution until an interrupt come
    // one of the saving mode
    while(true)
    {
        ISR_WaitForInterrupt();
    }
}
