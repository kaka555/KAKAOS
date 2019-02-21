#ifndef _COMMAND_PROCESSOR_H
#define _COMMAND_PROCESSOR_H

#include <ka_configuration.h>
#include <singly_linked_list.h>

#if CONFIG_SHELL_EN

#define DEBUG_SHELL 0  

#define ARRAY_SIZE 	10
#define ARGV_SIZE	10	

struct command{
	const char *command_name;
	void (* const f)(int argc, char const *argv[]);
	struct singly_list_head list;
};

/**************************
*insert into corresponding command_processer
**************************/
void insert_struct_command_1(struct command *ptr);
void insert_struct_command_2(struct command *ptr);
void insert_struct_command_3(struct command *ptr);
void insert_struct_command_4(struct command *ptr);
void insert_struct_command_5(struct command *ptr);
void insert_struct_command_6(struct command *ptr);
void insert_struct_command_7(struct command *ptr);
void insert_struct_command_8(struct command *ptr);

struct command_processer{
	const unsigned int command_length;
	struct singly_list_head *command_list_address;
};
int match_and_execute_command(
	int num,
	char const *argv[],
	struct command_processer * const command_processer_ptr);

struct command_processer *get_command_processer(unsigned int num);

#endif	
	
#endif
