#if !defined(OS_CONF_H)
# define OS_CONF_H

# define OS_DBG
# define OS_MEM_CHKSUM
# define OS_COM

# define OS_PIPE_SIZE 32
# define OS_NUM_TASKS 32

# if defined(OS_DBG)
#  define OS_MEM_DBG
# endif /* OS_DBG */

#endif /* OS_CONF_H */
