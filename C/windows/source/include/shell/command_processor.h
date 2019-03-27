#ifndef _COMMAND_PROCESSOR_H
#define _COMMAND_PROCESSOR_H

#include <ka_configuration.h>
#include <singly_linked_list.h>

#if CONFIG_SHELL_EN

#define DEBUG_SHELL 0  

#define ARRAY_SIZE 	10

struct command{
	const char *command_name;
	void (*f)(int argc, char const *argv[]);
	struct singly_list_head list;
	char **para_arv;  /* parament list,used for 'TAB' */
	unsigned int para_len;
};

/**************************
*insert into corresponding command_processer
**************************/
void _insert_struct_command_1(struct command *ptr);
void _insert_struct_command_2(struct command *ptr);
void _insert_struct_command_3(struct command *ptr);
void _insert_struct_command_4(struct command *ptr);
void _insert_struct_command_5(struct command *ptr);
void _insert_struct_command_6(struct command *ptr);
void _insert_struct_command_7(struct command *ptr);
void _insert_struct_command_8(struct command *ptr);

struct command_processer{
	const unsigned int command_length;
	struct singly_list_head *command_list_address;
};
int _match_and_execute_command(
	int num,
	char const *argv[],
	struct command_processer *command_processer_ptr);

struct command_processer *_get_command_processer(unsigned int num);

static inline void _set_command_para_list(struct command *command_ptr,char **para_arv,unsigned int para_len)
{
	if(para_arv)
	{
		command_ptr->para_arv = para_arv;
	}
	command_ptr->para_len = para_len;
}

#endif	
	
#endif
