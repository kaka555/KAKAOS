#ifndef _SHELL_H
#define _SHELL_H

#if CONFIG_SHELL_EN

#define BUFFER_SIZE 100

void shell(void *para);
void put_in_shell_buffer(char c);

#endif

#endif
