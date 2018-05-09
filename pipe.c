#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/io.h>
#include "pipe.h"
#include "com.h"
#include "task.h"

#define INCR_IND(i) i = ((i + 1 == OS_PIPE_SIZE)? 0 : (i + 1)) % OS_PIPE_SIZE

#define PIPE_EMPTY(p) ((p)->inp_ind == (p)->out_ind && \
                          !((p)->flags & f_pipe_full))


typedef struct os_com_s  os_com;
typedef struct os_pipe_s os_pipe;

struct os_pipe_s
{
    volatile unsigned char buf[OS_PIPE_SIZE];
    volatile uint8_t out_ind, inp_ind;
    volatile uint8_t flags;
    uint8_t interface;
};

os_pipe uart0_tx = {.flags = f_pipe_com_tx,                  .interface = 0};
os_pipe uart1_tx = {.flags = f_pipe_com_tx,                  .interface = 1};
os_pipe uart0_rx = {.flags = f_pipe_com_rx | f_pipe_noblk_w, .interface = 0};
os_pipe uart1_rx = {.flags = f_pipe_com_rx | f_pipe_noblk_w, .interface = 1};

int8_t os_write_str(os_pipe *p, char *str)
{
    if (!str) return -1;

    while (*str) os_write(p, *(str++));

    return 0;
}

int8_t os_write(os_pipe *p, char c)
{
    int8_t rtn = 0;

    while (p->flags & f_pipe_full)
    {
        os_yield();
    }

    p->buf[p->inp_ind] = c;
    INCR_IND(p->inp_ind);

    if (p->inp_ind == p->out_ind)
    {
        if (p->flags & f_pipe_noblk_w)
            INCR_IND(p->out_ind);
        else
            p->flags |= f_pipe_full;
    }

    else if (p->flags & f_pipe_com_tx)
        rtn = os_com_transmit(p);

    return rtn;
}

int16_t os_read(os_pipe *p)
{
    int16_t c;

    cli();

    if (PIPE_EMPTY(p))
        return -1;

    c = p->buf[p->out_ind];
    INCR_IND(p->out_ind);

    p->flags &= ~f_pipe_full;

    sei();

    return c;
}

int16_t os_peek(os_pipe *p)
{
    if (PIPE_EMPTY(p))
        return -1;

    return p->buf[p->out_ind];
}

int16_t os_pflags(os_pipe *p)
{
    return p->flags;
}

int8_t os_pinterface(os_pipe *p)
{
    if (!(p->flags & f_pipe_com_tx) &&
        !(p->flags & f_pipe_com_rx))
        return -1;

    return p->interface;
}