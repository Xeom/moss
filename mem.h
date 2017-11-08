#if !defined(OS_MEM_H)
# define OS_MEM_H
# include <stddef.h>
# include <stdint.h>

void *os_alloc(size_t bytes);

void *os_alloc_task(size_t bytes, uint8_t tsk);

uint8_t os_free_tsk(uint8_t tskn);

uint8_t os_free(void *mem);

# if defined(OS_DBG)
uint8_t os_mem_print(pipe *p);
# endif /* OS_DBG */

#endif /* OS_MEM_H */
