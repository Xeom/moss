#if !defined(OS_PIPE_H)
#define OS_PIPE_H
#include <stdint.h>
#include <stddef.h>

#define OS_PIPE_SIZE 10

typedef struct os_pipe_s os_pipe;

extern os_pipe usart0;
extern os_pipe usart1;

int8_t os_write_str(os_pipe *p, char *str);

int8_t os_write(os_pipe *p, char c);

char os_read(os_pipe *p);

#endif /* OS_PIPE_H */
