#if !defined(OS_MEM_H)
# define OS_MEM_H
# include <stddef.h>
# include <stdint.h>
# include "conf.h"
# include "pipe.h"

void os_mem_init(void);

/**
 * os_alloc_task() a block of memory on behalf of the current process.
 *
 * @argument bytes The number of bytes of memory to allocate.
 * @return         A pointer to the memory on success, NULL on error.
 **/
void *os_alloc(size_t bytes);

/**
 * Allocate memory on behalf of a specific program.
 *
 * If successful, the function returns a pointer to the start of a continuous
 * block of memory, marked as allocated by a specific task number.
 *
 * @argument bytes The number of bytes of memory to allocate.
 * @argument tskn  The task number to allocate the memory as.
 * @return         A pointer to the memory on success, NULL on error.
 **/
void *os_alloc_task(size_t bytes, uint8_t tskn);

/**
 * os_free() all the memory that was allocated by a specific task.
 *
 * @argument tskn The task number of the process to free memory from.
 * @return        0 on success, -1 on error.
 **/
int8_t os_free_tsk(uint8_t tskn);

/**
 * Free a pointer to allocated memory.
 *
 * @argument mem A pointer to the memory to free. This must be the same as the
 *               pointer returned by os_alloc.
 * @return       0 on success, -1 on error.
 **/
int8_t os_free(void *mem);

# if defined(OS_DBG)

/**
 * Print out the locations and owners of blocks in the heap.
 *
 * Prints three columns:
 *   The first has three numbers in - a pointer to the os_mem_blk structure
 * towards the end of the block, a pointer to the first byte in the block, and
 * a pointer to the last byte past the end of the block.
 *   The second is simply the number of bytes allocated to a program in that
 * block (the size of the block minus the size of the os_mem_blk structure)
 *   The third is the task number of the task that allocated the memory
 *
 * After the columns, it prints the number of bytes in the heap being used, and
 * the total number of bytes in the heap, giving an idea of how fragmented the
 * heap is.
 *
 * @argument p A pointer to a pipe to write the heap details to.
 * @return     0 on success, -1 on error.
 **/
int8_t os_mem_print(os_pipe *p);

# endif /* OS_DBG */

#endif /* OS_MEM_H */
