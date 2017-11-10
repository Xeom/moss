#include <avr/interrupt.h>
#include "pipe.h"

#define OS_PIPE_SIZE 10


enum os_pipe_flags
{
    f_pipe_full    = 0b00000001,
    f_pipe_proc    = 0b00000010,
    f_pipe_com     = 0b00000100
};

#define PIPE_EMPTY(p) ((p)->inp_ind == (p)->out_ind && \
                          !((p)->flags & f_pipe_full))

#define os_yield()
#define os_enter()

typedef struct os_com_s  os_com;
typedef struct os_pipe_s os_pipe;

struct os_pipe_s
{
    char buf[OS_PIPE_SIZE];
    uint8_t out_ind, inp_ind;
    uint8_t flags;
    uint8_t interface;
};


struct os_com_s
{
    volatile uint8_t *data_reg;
    volatile uint8_t *ready_reg;
    uint8_t  ready_bit : 3;
    uint8_t  ready     : 1;
    uint8_t  timeout;
    uint16_t baud;
};

static int8_t os_com_transmit(os_pipe *p);


int8_t os_write_str(os_pipe *p, char *str)
{
    if (!str) return -1;

    while (*str) os_write(p, *(str++));

    return 0;
}

int8_t os_write(os_pipe *p, char c)
{
    while (p->flags & f_pipe_full)
        os_yield();

    cli();

    p->buf[p->inp_ind] = c;
    p->inp_ind = (p->inp_ind + 1) % OS_PIPE_SIZE;

    if (p->inp_ind == p->out_ind)
        p->flags |= f_pipe_full;

    else if (p->flags & f_pipe_com)
        os_com_transmit(p);

    sei();

    return 0;
}

char os_read(os_pipe *p)
{
    char c;

    sei();

    c = p->buf[p->out_ind];
    p->out_ind = (p->out_ind + 1) % OS_PIPE_SIZE;
    p->flags -= p->flags & f_pipe_full;

    cli();

    return c;
}

static os_com coms[] =
{
    {
        .ready_reg = &UCSR0A, .ready_bit= UDRE0, .ready  = 0,
        .data_reg     = &UDR0,
        .timeout      = 100,
        .baud         = 9600
    },
    {
        .ready_reg = &UCSR1A, .ready_bit = UDRE1, .ready = 0,
        .data_reg     = &UDR1,
        .timeout      = 100,
        .baud         = 9600
    }
};

os_pipe usart0 =
{
    .flags     = f_pipe_com,
    .interface = 0
};

os_pipe usart1 =
{
    .flags     = f_pipe_com,
    .interface = 1
};

ISR(USART0_TX_vect)
{
    coms[usart0.interface].ready = 1;
    os_com_transmit(&usart0);
}

ISR(USART1_TX_vect)
{
    coms[usart1.interface].ready = 1;
    os_com_transmit(&usart1);
}

static int8_t os_com_transmit(os_pipe *p)
{
    char c;
    os_com *com;

    com = coms + p->interface;

    if (PIPE_EMPTY(p))
    {
        com->ready = 1;
        return -1;
    }

    if (!com->ready)
        return -1;

    c = os_read(p);

    *(com->data_reg) = c;
    com->ready       = 0;

    return 0;
}

void os_com_init(void)
{
    UBRR0H = (F_CPU / coms[0].baud / 16 - 1) >> 8;
    UBRR0L = (F_CPU / coms[0].baud / 16 - 1)  & 0xff;

    UCSR0B = _BV(RXEN0)  | _BV(TXEN0);
    UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);

    coms[0].ready = *(coms[0].ready_reg) & _BV(coms[0].ready_bit);

    UBRR1H = (F_CPU / coms[1].baud / 16 - 1) >> 8;
    UBRR1L = (F_CPU / coms[1].baud / 16 - 1)  & 0xff;

    UCSR1B = _BV(RXEN1)  | _BV(TXEN1);
    UCSR1C = _BV(UCSZ10) | _BV(UCSZ11);

    coms[1].ready = *(coms[1].ready_reg) & _BV(coms[1].ready_bit);
}
