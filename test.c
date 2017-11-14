#include "mem.h"
#include "pipe.h"
#include <util/delay.h>
#include <avr/interrupt.h>
int8_t os_com_transmit(os_pipe *p);
typedef struct os_com_s os_com;
struct os_com_s
{
    volatile uint8_t *data_reg;
    volatile uint8_t *ready_reg;
    uint8_t  ready_bit : 3;
    uint8_t  ready     : 1;
    uint8_t  timeout;
    uint16_t baud;
};
extern os_com coms[];

int main(void)
{
    char *ptr;
    os_mem_init();
    os_com_init();
    sei();
    coms[0].ready = 1;
    os_alloc(100);
    ptr = os_alloc(100);
    os_alloc(100);
    os_free(ptr);
    os_alloc(50);
 os_alloc(10);
    for(;;){
    os_mem_print(&usart0);
    _delay_ms(1000);}
}
