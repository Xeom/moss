#include "mem.h"
#include <stdlib.h>
#include <stdio.h>

extern void *__heap_start;
void *heap_low = 1000;
typedef struct os_mem_blk_s os_mem_blk;

struct os_mem_blk_s
{
#if defined(OS_MEM_CHKSUM)
    uint8_t     chksum;
#endif /* OS_MEM_CHKSUM */
    uint8_t     task;
    size_t      size;
    os_mem_blk *next;
};

#define OS_ERR(a, b)
#define os_tsk_curr 1

static os_mem_blk *os_mem_first;
static os_mem_blk *os_mem_final;

static int8_t os_free_blk(os_mem_blk *blk, os_mem_blk *prev);

/**
 *
 * Heap structure
 * |os_mem_first|.........|blk|.....|blk|     ......|os_mem_final|
 *
 * Every os_mem_blk sits at the end of the allocated block of memory.
 * The first os_mem_blk has no associated memory.
 *
 **/

/* Get the start of the memory contained in a block */
#define BLK_START(blk) ((char *)(blk) + sizeof(os_mem_blk) - (blk)->size)
/* Get the final byte of a block */
#define BLK_END(blk)   ((char *)(blk) + sizeof(os_mem_blk))

/*******************
 * MEMORY CHECKING *
 *******************/

/**
 *
 * Since we have no memory protection at all, if OS_MEM_CHKSUM is defined,
 * then each memory block will have a simple checksum of the contents of the
 * structure. If the heap itself is corrupted, then hopefully the checksums
 * will be able to detect the problem for debugging.
 *
 **/

#if defined(OS_MEM_CHKSUM)
/* Define macros for memory checking */

/* BLK_CHK runs code if blk is not a valid mem_blk */
# define BLK_CHK(blk, code) if (os_mem_chk(blk)) {code;}

/* BLK_UPDATE updates the checksum of a block to match its contents */
# define BLK_UPDATE(blk) (blk)->chksum = os_mem_chksum(blk)

#else

/* If we're not going to use checksums, define them as blank */
# define BLK_CHK(blk, code)
# define BLK_UPDATE(blk)

#endif /* OS_MEM_CHKSUM */

#if defined(OS_MEM_CHKSUM)
/* Define memory checking functions */

/**
 * Generte a checksum for a memory block
 **/
static uint8_t os_mem_chksum(os_mem_blk *blk);

/**
 * Check whether a memory block is valid. Returns 0 if valid, -1 if not.
 **/
static int8_t os_mem_chk(os_mem_blk *blk);

static uint8_t os_mem_chksum(os_mem_blk *blk)
{
    uint8_t sum, ind;

    sum = 0xaa;
    ind = sizeof(os_mem_blk);

    /* Note that ind = 0 is not considered. This means that *
     * we don't sum the old checksum as well.               */
    while (--ind) sum ^= ((char *)blk)[ind];

    return sum;
}

static int8_t os_mem_chk(os_mem_blk *blk)
{
    /* Check it's in thr right place */
    if (blk < os_mem_first || blk > os_mem_final)
    {
        OS_ERR(inv_blk, "mem_chk: out of range");
        return -1;
    }

    /* Check it has a valid checksum */
    if (os_mem_chksum(blk) != blk->chksum)
    {
        OS_ERR(inv_blk, "mem_chk: inv sum");
        return -1;
    }

    return 0;
}
#endif /* OS_MEM_CHKSUM */

/********
 * INIT *
 ********/

void os_mem_init(void)
{
    /* Initialize the first memory block */
    os_mem_first = heap_low;
    os_mem_first->task  = -1;
    os_mem_first->next = NULL;
    os_mem_first->size = 0;
    BLK_UPDATE(os_mem_first);

    os_mem_final = os_mem_first;
}

/*********************
 * ALLOCATING MEMORY *
 *********************/

void *os_alloc(size_t bytes)
{
    os_alloc_task(bytes, os_tsk_curr);
}

void *os_alloc_task(size_t bytes, uint8_t tskn)
{
    os_mem_blk *blk, *prev, *new;
    size_t spare;

    bytes += sizeof(os_mem_blk);
    blk    = os_mem_first;

    for (;;)
    {
        prev = blk;
        blk = blk->next;

        if (blk == NULL) break;

        BLK_CHK(blk, OS_ERR(inv_blk, "alloc: inv blk"); return NULL);

        spare = BLK_START(blk) - BLK_END(prev);

        if (spare >= bytes) break;
    }

    new = (os_mem_blk *)((char *)prev + bytes);

    if (blk == NULL) os_mem_final = new;

    new->size = bytes;
    new->next = blk;
    new->task = tskn;

    BLK_UPDATE(new);

    prev->next = new;
    BLK_UPDATE(prev);

    return BLK_START(new);
}

/******************
 * FREEING MEMORY *
 ******************/

int8_t os_free_tsk(uint8_t tskn)
{
    os_mem_blk *blk, *next;

    blk = os_mem_first->next;
    BLK_CHK(blk, OS_ERR(inv_blk, "free: inv blk"); return -1);

    while (blk)
    {
        next = blk->next;
        BLK_CHK(blk, OS_ERR(inv_blk, "free: inv blk"); return -1);

        if (blk->task == tskn)
            os_free(blk);

        blk = next;
    }

    return 0;
}

int8_t os_free(void *mem)
{
    os_mem_blk *prev, *blk;

    prev = (os_mem_blk *)((char *)mem - sizeof(os_mem_blk));
    BLK_CHK(prev, OS_ERR(inv_blk, "free: inv prev blk"); return -1);

    blk = prev->next;
    BLK_CHK(blk,  OS_ERR(inv_blk, "free: inv blk");      return -1);

    return os_free_blk(blk, prev);
}

static int8_t os_free_blk(os_mem_blk *blk, os_mem_blk *prev)
{
    os_mem_blk *next;

    next = blk->next;
    if (next)
        BLK_CHK(next, OS_ERR(inv_blk, "free: inv next blk"); return -1);

    prev->next   = next;
    BLK_UPDATE(prev);

#if defined(OS_MEM_CHKSUM)
    /* If we enabled checksums, reset the block to an invalid state */
    blk->chksum = 0;
    blk->task   = 0;
    blk->size   = 0;
    blk->next   = 0;
#endif /* OS_MEM_CHKSUM */

    return 0;
}

/*********
 * DEBUG *
 *********/

#if defined(OS_DBG)

int8_t os_mem_print(os_pipe *p)
{
    os_mem_blk *blk, *prev;
    char out[100];
    size_t used, heap;

    heap = BLK_END(os_mem_final) - BLK_START(os_mem_first);
    used = 0;

    if (os_write_str(p, "|_______Memory_______|Bytes|Tsk|\r\n"))
        return -1;

    for (prev = os_mem_first; prev->next; prev = prev->next)
    {
        blk = prev->next;

        BLK_CHK(
            blk,
            snprintf(out, sizeof(out), "INV BLK %p\r\n", (void *)blk);
            os_write_str(p, out);
            return -1;
        );

        used += blk->size;

        snprintf(
            out, sizeof(out), " 0x%04x 0x%04x - %04x %5d %3d\r\n",
            (uintptr_t)blk,
            (uintptr_t)BLK_START(blk),
            (uintptr_t)BLK_END(blk),
            blk->size - sizeof(os_mem_blk),
            blk->task
        );

        if (os_write_str(p, out)) return -1;
    }

    snprintf(out, sizeof(out), "\r\nUsed:%5d, Heap:%5d\r\n", used, heap);

    if (os_write_str(p, out)) return -1;

    return 0;
}

#endif /* OS_DBG */
