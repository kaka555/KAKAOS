#ifndef _SHELL_H
#define _SHELL_H

#if CONFIG_SHELL_EN

#define ARGV_SIZE			10	
#define BUFFER_SIZE 		100

struct shell_buffer //use for thread shell to get the input
{
	char *argv[ARGV_SIZE];
	char *argv_reserve[ARGV_SIZE];
	char *buffer;
	char *buffer_reserve;
	unsigned int index;
	unsigned int index_reserve;
};

int __init_shell_buffer(struct shell_buffer *shell_buffer_ptr,char *buffer_ptr,char *buffer_reserve_ptr);
struct shell_buffer *change_shell_buffer(struct shell_buffer *shell_buffer_ptr); //use new buffer to replace the old one,return the old one
void shell_buffer_wait_str(const char *str_ptr); // thread will going to sleep
void shell(void *para);
void put_in_shell_buffer(char c);
char *get_para_add(int argc, char const *argv[], const char *pre_name);

#endif

#endif
