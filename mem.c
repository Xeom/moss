struct os_mem_blk
{
#if defined(OS_MEM_CHKSUM)
    uint8_t chksum;
#endif /* OS_MEM_CHKSUM */
    uint8_t proc;
    size_t size;
    void  *next;
};

static os_mem_blk *os_mem_first;
static os_mem_blk *os_mem_final;

/**
 *
 * Heap structure
 * |os_mem_first|.........|blk|.....|blk|     ......|os_mem_final|
 *
 * Every os_mem_blk sits at the end of the allocated block of memory.
 * The first os_mem_blk has no associated memory.
 *
 **/

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
# define BLK_CHK(blk, code) if (os_mem_chk(blk)) {code}

/* BLK_UPDATE updates the checksum of a block to match its contents */
# define BLK_UPDATE(blk) (blk)->chksum = os_mem_chksum(blk)

#else

/* If we're not going to use checksums, define them as blank */
# define BLK_CHK(blk, code)
# define BLK_UPDATE(blk)

#endif /* OS_MEM_CHKSUM */

#if defined(OS_MEM_CHKSUM)
/* Define memory checking functions */

static uint8_t os_mem_chksum(os_mem_blk *blk);
static uint8_t os_mem_chk(os_mem_blk *blk);

static uint8_t os_mem_chksum(os_mem_blk *blk)
{
    uint8_t sum, ind;

    sum = 0xaa;
    ind = sizeof(os_mem_blk);

    while (--ind) sum ^= ((char *)blk)[ind];

    return sum;
}

static uint8_t os_mem_chk(os_mem_blk *blk)
{
    uint8_t ind, sum;

    if (blk < os_mem_first || blk > os_mem_final)
    {
        OS_ERR("mem_chk: out of range");
        return -1;
    }

    if (os_mem_chksum(blk) != blk->chksum)
    {
        OS_ERR("mem_chk: inv sum");
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
    os_mem_first = __heap_start;
    os_mem_first->proc = -1;
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
    os_alloc_proc(bytes, os_curr_proc);
}

void *os_alloc_proc(size_t bytes, uint8_t proc)
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

        spare = (char *)blk - (char *)prev - blk->size;

        if (spare >= bytes) break;
    }

    new = (char *)prev + byte;

    if (blk == NULL) os_mem_final = new;

    new->size = bytes;
    new->next = blk;
    new->proc = proc;

    BLK_UPDATE(new);

    prev->next = blk;
    BLK_UPDATE(prev);

    return (char *)new - blk->size + sizeof(os_mem_blk);
}

/******************
 * FREEING MEMORY *
 ******************/

uint8_t os_free_proc(uint8_t proc)
{
    os_mem_blk *blk, *next;

    blk = os_mem_first->next;

    while (blk)
    {
        next = blk->next;

        if (blk->proc = proc)
            os_free(blk);

        blk = next;
    }

    return;
}

uint8_t os_free(void *mem)
{
    os_mem_blk *prev, *blk, *next;

    prev = (char *)mem - sizeof(os_mem_blk);
    BLK_CHK(prev, OS_ERR(inv_blk, "free: inv prev blk"); return -1);

    blk  = prev->next;
    BLK_CHK(blk, OS_ERR(inv_blk, "free: inv blk"); return -1);

    next = blk->next;
    if (next)
        BLK_CHK(next, OS_ERR(inv_blk, "free: inv next blk"); return -1);

    prev->next   = next;
    BLK_UPDATE(prev);

    /* If we enabled checksums, reset the block to an invalid state */
#if defined(OS_MEM_CHKSUM)
    blk->chksum  = 0;
    blk->proc    = 0;
    blk->size    = 0;
    blk->next    = 0;
#endif /* OS_MEM_CHKSUM */

    return 0;
}
