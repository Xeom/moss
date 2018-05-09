#include "mem.h"
#include "pipe.h"
#include "com.h"
#include "task.h"
#include <util/delay.h>


void print_hellos(void *p)
{
    for (;;)
    {
//        os_mem_print(&uart0_tx);
        os_yield();
    }
}

void print_str(void *str)
{
    for (;;)
    {
        _delay_ms(50);
        os_write_str(&uart0_tx, str);
        os_yield();
    }
}

char *wow = "Wowzers\r\n";
char *woah = "Woah\r\n";

int main(void)
{
    os_mem_init();
    os_com_init();

    os_new_task(print_str, 500, wow);
    os_new_task(print_str, 500, woah);

    os_kill_task(os_task_curr);

    return 0;
}