#include <os_TCB_list.h>
#include <TCB.h>
#include <kakaosstdint.h>
#include <myassert.h>
#include <myMicroLIB.h>
#include <sys_init_fun.h>

static struct TCB_list TCB_list[PRIO_MAX];

static void __INIT __init_TCB_list(void)
{
	int i;
	for(i=0;i<PRIO_MAX;++i)
	{
		INIT_LIST_HEAD(&TCB_list[i].head);
		TCB_list[i].ready_num = 0;
		TCB_list[i].TCB_num = 0;
	}
}
INIT_FUN(__init_TCB_list);

/*os must set the task_state before using this function*/
void _register_in_TCB_list(TCB *TCB_ptr)
{
	ASSERT(NULL != TCB_ptr);
	list_add(&TCB_ptr->same_prio_list,&TCB_list[TCB_ptr->prio].head);
	++(TCB_list[TCB_ptr->prio].TCB_num);
}

/**
 * This is a system function,if the TCB_ptr state is ready,os should decrease
 * the ready num with function _decrease_ready_num() in another place
 * which means, this functin do not decrease the ready num
 * @Author      kaka
 * @DateTime    2018-10-01
 * @description :
 * @param       TCB_ptr    [description]
 * @return                 [description]
 */
int _delete_from_TCB_list(TCB *TCB_ptr)
{
	ASSERT(NULL != TCB_ptr);
	struct list_head *pos;
	list_for_each(pos,&TCB_list[TCB_ptr->prio].head)
	{
		if(&TCB_ptr->same_prio_list == pos)
		{
			list_del(pos);
			ASSERT(0 != TCB_list[TCB_ptr->prio].TCB_num);
			--(TCB_list[TCB_ptr->prio].TCB_num);
			return FUN_EXECUTE_SUCCESSFULLY;
		}
	}
	/*should not go here*/
	ASSERT(0);
	return ERROR_VALUELESS_INPUT;
}

struct list_head *_get_from_TCB_list(unsigned int index)
{
	ASSERT(index < PRIO_MAX);
	return &TCB_list[index].head;
}

unsigned char _get_ready_num_from_TCB_list(unsigned int index)
{
	ASSERT(index < PRIO_MAX);
	return TCB_list[index].ready_num;
}

void _decrease_ready_num(unsigned int index)
{
	ASSERT(index < PRIO_MAX);
	ASSERT(0 != TCB_list[index].ready_num);
	--(TCB_list[index].ready_num);
}

void _increase_ready_num(unsigned int index)
{
	ASSERT(index < PRIO_MAX);
	++(TCB_list[index].ready_num);
}

void _decrease_TCB_num(unsigned int index)
{
	ASSERT(index < PRIO_MAX);
	ASSERT(0 != TCB_list[index].TCB_num);
	--(TCB_list[index].TCB_num);
}

void _increase_TCB_num(unsigned int index)
{
	ASSERT(index < PRIO_MAX);
	++(TCB_list[index].TCB_num);
}

#if CONFIG_SHELL_EN
void shell_check_TCB_list(void)
{
	unsigned int i;
	unsigned int ready_num = 0,TCB_num = 0;
	struct list_head *pos;
	TCB *TCB_ptr;
	for(i=0;i<PRIO_MAX;++i)
	{
		if(!list_empty(&TCB_list[i].head))
		{
			ka_printf("prio %u has task:\n",i);
			list_for_each(pos,&TCB_list[i].head)
			{
				TCB_ptr = list_entry(pos,TCB,same_prio_list);
				++TCB_num;
				if(STATE_READY == TCB_ptr->task_state)
				{
					++ready_num;
				}
				ka_printf("name: %s\n",TCB_ptr->name);
				ka_printf("timeslice_hope_time: %d\n",TCB_ptr->timeslice_hope_time);
				ka_printf("stack: %p\n",(void *)TCB_ptr->stack);
				ka_printf("stack_size: %d\n",TCB_ptr->stack_size);
				ka_printf("task_state: ");
				switch(TCB_ptr->task_state)
				{
					case STATE_READY:
						ka_printf("STATE_READY\n");break;
					case STATE_DELAY:
					case STATE_WAIT_MCB_TIMEOUT:
					case STATE_WAIT_MESSAGE_QUEUE_TIMEOUT:
					case STATE_PUT_MESSAGE_QUEUE_TIMEOUT:
						ka_printf("STATE_DELAY\n");break;
					case STATE_SUSPEND_NORMAL:
					case STATE_WAIT_MCB_FOREVER:
					case STATE_WAIT_MESSAGE_QUEUE_FOREVER:
					case STATE_PUT_MESSAGE_QUEUE_FOREVER:
					case STATE_WAIT_MUTEX_FOREVER:
						ka_printf("STATE_SUSPEND\n");break;
				}
				ka_printf("/*****************************************************/\n");
			}
			if(TCB_num != TCB_list[i].TCB_num)
			{
				ka_printf("prio %u TCB_num error!\n",i);
			}
			if(ready_num != TCB_list[i].ready_num)
			{
				ka_printf("prio %u ready_num error!\n",i);
			}
			ready_num = 0;
			TCB_num = 0;
		}
	}
}
#endif

#if CONFIG_SHELL_EN
void shell_stack_check(int argc, char const *argv[])
{
	(void)argc;
	(void)argv;
	unsigned int i;
	struct list_head *pos;
	TCB *TCB_ptr;
	for(i=0;i<PRIO_MAX;++i)
	{
		if(!list_empty(&TCB_list[i].head))
		{
			list_for_each(pos,&TCB_list[i].head)
			{
				unsigned int num = 0;
				TCB_ptr = list_entry(pos,TCB,same_prio_list);
				unsigned int *ptr = (unsigned int *)(TCB_ptr->stack_end);
				ka_printf("=====================================================\n");
				ka_printf("task name : %s\n",TCB_ptr->name);
				ka_printf("The stack space is from %p to %p\n",TCB_ptr->stack_end,(STACK_TYPE *)TCB_ptr->stack_end + TCB_ptr->stack_size/4);
				if(0 != *(unsigned int *)(TCB_ptr->stack_end))
				{
					ka_printf("stack full!!!!\n");
					ASSERT(0);
				}
				while(0 == *ptr)
				{
					++num;
					++ptr;
					if((char *)ptr == (char *)(TCB_ptr->stack_end) + TCB_ptr->stack_size)
					{
						ka_printf("stack empty!!!!\n");
						break;
					}
				}
				ka_printf("It's stack used rate is %d%%\n",100-400*num / TCB_ptr->stack_size);
			}
		}
	}
}
#endif

#if CONFIG_SHELL_EN

void shell_show_tasks_registers(int argc, char const *argv[])
{
	(void)argc;
	(void)argv;
	unsigned int i;
	//struct list_head *pos;
	TCB *TCB_ptr;
	for(i=0;i<PRIO_MAX;++i)
	{
		if(!list_empty(&TCB_list[i].head))
		{
			list_for_each_entry(TCB_ptr,&TCB_list[i].head,same_prio_list)
			{
				const UINT32 *reg = (unsigned int *)(TCB_ptr->stack);
				ka_printf("=====================================================\n");
				ka_printf("task name : %s\n",TCB_ptr->name);
				if(0 == ka_strcmp(TCB_ptr->name,"shell"))
				{
					ka_printf("now show task shell's register is volatile\n");
					continue;
				}
				ka_printf("R4 	= 	0x%x\n",*reg++);
				ka_printf("R5 	= 	0x%x\n",*reg++);
				ka_printf("R6 	= 	0x%x\n",*reg++);
				ka_printf("R7 	= 	0x%x\n",*reg++);
				ka_printf("R8 	= 	0x%x\n",*reg++);
				ka_printf("R9 	= 	0x%x\n",*reg++);
				ka_printf("R10 	= 	0x%x\n",*reg++);
				ka_printf("R11 	= 	0x%x\n",*reg++);
				ka_printf("xPSR	=	0x%x\n",*reg++);
				ka_printf("PC 	= 	0x%x\n",*reg++);
				ka_printf("R12 	= 	0x%x\n",*reg++);
				ka_printf("R3 	= 	0x%x\n",*reg++);
				ka_printf("R2 	= 	0x%x\n",*reg++);
				ka_printf("R1 	= 	0x%x\n",*reg++);
				ka_printf("R0 	= 	0x%x\n",*reg);
				ka_printf("=====================================================\n");
			}
		}
	}
}

#endif 
