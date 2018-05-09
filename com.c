#include "pipe.h"
#include "com.h"
#include <avr/io.h>
#include <avr/interrupt.h>

struct os_com_s
{
    volatile uint8_t *data_reg;
    volatile uint8_t *cntrl_reg;
    uint8_t           tx_ready_bit : 3;
    volatile uint8_t  tx_ready     : 1;
    uint8_t           rx_ready_bit : 3;
    volatile uint8_t  rx_ready     : 1;
    uint8_t  timeout;
    uint16_t baud;
};

#define RX_READY(com) (*((com)->cntrl_reg) & _BV((com)->rx_ready_bit))
#define TX_READY(com) (*((com)->cntrl_reg) & _BV((com)->tx_ready_bit))

#define OS_ERR(a, b)

/* Gets the com pointer for a pipe */
#define GET_COM(fname)                             \
    int interface;                                 \
    interface = os_pinterface(p);                  \
    if (interface == -1)                           \
    {                                              \
        OS_ERR(inv_pipe, #fname ": no interface"); \
        return -1;                                 \
    }                                              \
    com = &os_coms[interface];

os_com os_coms[] =
{
    {
        .cntrl_reg = &UCSR0A,
        .tx_ready_bit = UDRE0, .tx_ready = 0,
        .rx_ready_bit = RXC0,
        .data_reg     = &UDR0,
        .timeout      = 100,
        .baud         = 9600
    },
    {
        .cntrl_reg = &UCSR1A,
        .tx_ready_bit = UDRE1, .tx_ready = 0,
        .rx_ready_bit = RXC1,
        .data_reg     = &UDR1,
        .timeout      = 100,
        .baud         = 9600
    }
};

ISR(USART0_TX_vect)
{
    os_com_transmit(&uart0_tx);
}

ISR(USART1_TX_vect)
{
    os_com_transmit(&uart1_tx);
}

ISR(USART0_RX_vect)
{
    os_com_recieve(&uart0_rx);
#if defined(OS_DBG)
//    os_debug_read(&uart0_rx);
#endif
}

ISR(USART1_RX_vect)
{
    os_com_recieve(&uart1_rx);
}

int8_t os_com_recieve(os_pipe *p)
{
    os_com *com;

    if (!(os_pflags(p) & f_pipe_com_rx))
        return -1;

    GET_COM(com_recieve);

    if (!RX_READY(com))
        return -1;

    os_write(p, *(com->data_reg));

    return 0;
}

int8_t os_com_transmit(os_pipe *p)
{
    char c;
    os_com *com;

    if (!(os_pflags(p) & f_pipe_com_tx))
        return -1;

    GET_COM(com_transmit);

    if (os_peek(p) == -1)
        return -1;

    if (!TX_READY(com))
        return 0;

    c = os_read(p);
    
    *(com->data_reg) = c;
    
    return 0;
}

int16_t os_com_rx_blk(os_pipe *p)
{
    os_com *com;

    if (!(os_pflags(p) & f_pipe_com_rx))
    {
        OS_ERR(inv_pipe, "rx_blk: pipe not rx");
        return -1;
    }

    GET_COM(com_rx_blk);

    while (!RX_READY(com));

    return *(com->data_reg);
}

int8_t os_com_tx_blk(os_pipe *p, char c)
{
    os_com *com;

    if (!(os_pflags(p) & f_pipe_com_tx))
    {
        OS_ERR(inv_pipe, "tx_blk: pipe not tx");
        return -1;
    }

    GET_COM(com_tx_blk);
    
    while (!TX_READY(com));

    *(com->data_reg) = c;

    return 0;
}

int8_t os_com_tx_blk_str(os_pipe *p, char *str)
{
    while (*str)
    {
        if (os_com_tx_blk(p, *(str++)) == -1)
            return -1;
    }

    return 0;
}

void os_com_init(void)
{
    cli();

    UBRR0H = (F_CPU / os_coms[0].baud / 16 - 1) >> 8;
    UBRR0L = (F_CPU / os_coms[0].baud / 16 - 1)  & 0xff;

    UCSR0B = _BV(RXEN0)  | _BV(TXEN0)  | _BV(RXCIE0) | _BV(TXCIE0);
    UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);

    UBRR1H = (F_CPU / os_coms[1].baud / 16 - 1) >> 8;
    UBRR1L = (F_CPU / os_coms[1].baud / 16 - 1)  & 0xff;

    UCSR1B = _BV(RXEN1)  | _BV(TXEN1);
    UCSR1C = _BV(UCSZ10) | _BV(UCSZ11);

    sei();
}
