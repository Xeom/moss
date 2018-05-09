#if !defined(OS_CONF_H)
# define OS_CONF_H

//# define OS_DBG
//# define OS_MEM_CHKSUM
# define OS_COM

# define OS_PIPE_SIZE 32
# define OS_NUM_TASKS 32

# define OS_UART_ALL

# if defined(OS_UART_ALL)
#  define OS_UART_TX0
#  define OS_UART_TX1
#  define OS_UART_RX0
#  define OS_UART_RX1
# endif /* OS_UART_ALL */

# if defined(OS_DBG)
#  define OS_MEM_DBG
# endif /* OS_DBG */

#endif /* OS_CONF_H */
