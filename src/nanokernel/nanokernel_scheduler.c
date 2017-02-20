#include "nanokernel_scheduler.h"
#include "nanokernel_task.h"
#include "nanokernel.h"

void nanokernel_scheduler()
{
    // Round robin scheduler
    nanokernel_currTask = nanokernel_currTask->next;
}
