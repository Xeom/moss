#include "mem.h"
#include "pipe.h"
#include "com.h"
#include "task.h"
#include <util/delay.h>

void print_hellos(void)
{
    for (;;)
    {
        os_write_str(&uart0_tx, "Hello I am also happy\r\n");
        os_yield();
    }
}

void print_goodbyes(void)
{
    for (;;)
    {
        os_write_str(&uart0_tx, "Goodbye, but I am happy\r\n");
        os_yield();
    }
}

int main(void)
{
    os_mem_init();
    os_com_init();

    os_new_task(print_hellos, 500);
    print_goodbyes();

    return 0;
}