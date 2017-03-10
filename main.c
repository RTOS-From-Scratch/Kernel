#include "src/nanokernel/nanokernel.h"

void task_1()
{

}

void task_2()
{

}

void task_3()
{

}

int main()
{
    nanokernel_init();

    nanokernel_Task_create(1000, PRIORITY_1, task_1);
    nanokernel_Task_create(1000, PRIORITY_2, task_2);
    nanokernel_Task_create(1000, PRIORITY_3, task_3);

    // it will never back
    nanokernel_bootup();

    return(0);
}
