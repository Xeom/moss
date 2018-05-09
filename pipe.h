#if !defined(OS_PIPE_H)
# define OS_PIPE_H
# include <stdint.h>
# include <stddef.h>
# include "conf.h"

typedef struct os_pipe_s os_pipe;

enum os_pipe_flags
{
    f_pipe_full      = 0x01,
    f_pipe_com_tx    = 0x02,
    f_pipe_com_rx    = 0x04,
    f_pipe_noblk_w   = 0x08,
    f_pipe_noblk_r   = 0x10,
    f_pipe_empty     = 0x20     
};

extern os_pipe uart0_rx;
extern os_pipe uart1_rx;

extern os_pipe uart0_tx;
extern os_pipe uart1_tx;

/**
 * Write a null-terminated string to a pipe.
 *
 * This function repeatedly calls os_write for each charater in the
 * pipe.
 *
 * @param p   The pipe to write the string to.
 * @param str A pointer to a null-terminated character string.
 *
 * @return 0 on success, -1 on error.
 *
 **/
int8_t os_write_str(os_pipe *p, char *str);

/**
 * Writes a character to a pipe.
 *
 * A character is added to the pipe. If the pipe is full, the OS
 * yields to other processes until the pipe is emptied.
 *
 * If the pipe is configured with the f_pipe_noblk_w flag, then if the
 * pipe is filled, the first character is overwritten.
 *
 * If the pipe is a com pipe, the function automatically tries to
 * transmit it if needed.
 *
 * @param p The pipe to write the character to.
 * @param c The character to write.
 *
 * @return 0 on success, -1 on error.
 *
 **/
int8_t os_write(os_pipe *p, char c);

/**
 * Reads a character from a pipe, nonblockingly.
 *
 * A character is removed from the pipe and returned. If the
 * pipe is empty, -1 is returned.
 *
 * @param p The pipe to read from.
 *
 * @return The character on success, -1 on error.
 *
 **/
int16_t os_read(os_pipe *p);

/**
 * Views the next charater in a pipe.
 *
 * The next character in the pipe is returned but not removed. If
 * the pipe is empty, -1 is returned.
 *
 * This function can be a useful way to check if a pipe is empty, along
 * with (os_pflags(p) & f_pipe_empty).
 *
 * @param p The pipe to peek at.
 *
 * @return The next character in the pipe on success, -1 on error.
 *
 **/
int16_t os_peek(os_pipe *p);

int16_t os_pflags(os_pipe *p);

int8_t os_pinterface(os_pipe *p);

#endif /* OS_PIPE_H */
