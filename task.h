#if !defined(TASK_H)
# define TASK_H
# include <stdint.h>
# include <stddef.h>

#define OS_ERR(a, b)

extern int8_t os_task_curr;

void os_yield(void);

int8_t os_new_task(void (*fptr)(void *), size_t size, void *param);

int8_t os_kill_task(int8_t tskn);

#endif