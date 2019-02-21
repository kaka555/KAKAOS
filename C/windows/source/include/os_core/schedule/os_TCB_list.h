#ifndef _OS_TCB_LIST_H
#define _OS_TCB_LIST_H

#include <double_linked_list.h>
#include <TCB.h>

struct TCB_list{
	struct list_head head;
	unsigned char ready_num;
	unsigned char TCB_num;
};

void __init_TCB_list(void);
int register_in_TCB_list(TCB *TCB_ptr);
int delete_from_TCB_list(TCB *TCB_ptr);
struct list_head *get_from_TCB_list(unsigned int index);
unsigned char get_ready_num_from_TCB_list(unsigned int index);
void decrease_ready_num(unsigned int index);
void increase_ready_num(unsigned int index);
void decrease_TCB_num(unsigned int index);
void increase_TCB_num(unsigned int index);
void shell_check_TCB_list(void);
void shell_stack_check(int argc, char const *argv[]);
void shell_show_tasks_registers(int argc, char const *argv[]);

#endif
