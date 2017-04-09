#include "nanokernel_task_idle.h"
#include "Drivers/src/ISR_ctrl.h"

void nanokernel_Task_idle()
{
    // suspend execution until an interrupt come
    // one of the saving mode
    ISR_WaitForInterrupt();
}
