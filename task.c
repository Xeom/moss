#include "task.h"

#include "conf.h"
#include "mem.h"

typedef enum
{
    f_alive = 0x01
} os_task_flags;

#define OS_ERR(a, b)

typedef struct os_task_s os_task;

struct os_task_s
{
    uint8_t flags;
    void *mem;
    uint8_t *sptr;
    void (*fptr)(void);
    size_t size;
};

int8_t os_task_curr = 0;

os_task os_tasks[OS_NUM_TASKS] = { [0] = { .flags = f_alive } };


void os_yield(void)
{
    static volatile int8_t prev, new;
    void *sptr;

    prev = os_task_curr;
    new  = prev;

    do
    {
        new += 1;
        if (new == OS_NUM_TASKS) new = 0;
    } while ((os_tasks[new].flags & f_alive) == 0);

    os_task_curr = new;

    asm volatile(
        "push r2 \n push r3 \n push r4 \n push r5 \n"
        "push r6 \n push r7 \n push r8 \n push r9 \n"
        "push r10\n push r11\n push r12\n push r13\n"
        "push r14\n push r15\n push r16\n push r17\n"

        "push r28\n push r29\n"

        "in r18, __SP_L__\n"
        "in r19, __SP_H__\n"

        "out __SP_L__, %A1\n"
        "out __SP_H__, %B1\n"

        "pop r29\n pop r28\n"

        "pop r17\n pop r16\n pop r15\n pop r14\n"
        "pop r13\n pop r12\n pop r11\n pop r10\n"
        "pop r9 \n pop r8 \n pop r7 \n pop r6 \n"
        "pop r5 \n pop r4 \n pop r3 \n pop r2 \n"

        "mov %A0, r18\n"
        "mov %B0, r19\n"
        : "=&r" (sptr)
        : "r"  (os_tasks[new].sptr)
        : "r18", "r19"
        );

    os_tasks[prev].sptr = sptr;
}

static int8_t os_alloc_taskn(void)
{
    int8_t rtn;

    for (rtn = 0; rtn < OS_NUM_TASKS; rtn++)
    {
        if (!(os_tasks[rtn].flags & f_alive))
            return rtn;
    }

    OS_ERR(tsk_mem, "alloc_taskn: too many tasks");
    
    return -1;
}

int8_t os_new_task(void (*fptr)(void), size_t size)
{  
    os_task *tsk;
    int8_t   tskn;
    void    *stack;

    tskn = os_alloc_taskn();

    if (tskn == -1) return -1;

    tsk = &os_tasks[tskn];

    stack = os_alloc(size);
    tsk->flags  = f_alive;
    tsk->mem    = stack;
    tsk->fptr   = fptr;
    tsk->size   = size;

    /* Reset stack pointer */
    tsk->sptr = tsk->mem + tsk->size - 1;

    /* Put function return addr */
    *(tsk->sptr--) = 0xff &  (int16_t)(tsk->fptr);
    *(tsk->sptr--) = 0xff & ((int16_t)(tsk->fptr) >> 8);

    /* Decrement for register space */
    tsk->sptr -= 18;

    return 0;
}