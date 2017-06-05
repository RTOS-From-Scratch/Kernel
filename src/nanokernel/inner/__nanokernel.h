#ifndef INNER_NANOKERNEL_H_
#define INNER_NANOKERNEL_H_

enum __nanokernel_States {
    __NOT_INITIATED = -2,
    __NOT_BOOTED = -1,
    __BOOTED,
    __TASKLESS,
};

typedef enum __nanokernel_States __nanokernel_States;

__nanokernel_States __nanokernel_getState();
void __nanokernel_setState(__nanokernel_States state);

#endif // INNER_NANOKERNEL_TASK_H_
