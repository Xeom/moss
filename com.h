#if !defined(OS_COM_H)
# define OS_COM_H
# include "pipe.h"

typedef struct os_com_s os_com;

int8_t os_com_recieve(os_pipe *p);

int8_t os_com_transmit(os_pipe *p);

int16_t os_com_rx_blk(os_pipe *p);

int8_t os_com_tx_blk(os_pipe *p, char c);

int8_t os_com_tx_blk_str(os_pipe *p, char *str);

/* Initialize the communication systems */
void os_com_init(void);

#endif