#include <os_TCB_list.h>
#include <TCB.h>
#include <kakaosstdint.h>
#include <myassert.h>
#include <myMicroLIB.h>

static struct TCB_list TCB_list[PRIO_MAX];

void __init_TCB_list(void)
{
	int i;
	for(i=0;i<PRIO_MAX;++i)
	{
		INIT_LIST_HEAD(&TCB_list[i].head);
		TCB_list[i].ready_num = 0;
		TCB_list[i].TCB_num = 0;
	}
}

//os must set the task_state before using this function
inline int register_in_TCB_list(TCB *TCB_ptr)
{
	ASSERT(NULL != TCB_ptr);
#if CONFIG_PARA_CHECK
	if(NULL == TCB_ptr)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(register_in_TCB_list,TCB_ptr);
		return -ERROR_NULL_INPUT_PTR;
	}
#endif
	list_add(&TCB_ptr->same_prio_list,&TCB_list[TCB_ptr->prio].head);
	++(TCB_list[TCB_ptr->prio].TCB_num);
	return FUN_EXECUTE_SUCCESSFULLY;
}

/**
 * This is a system function,if the TCB_ptr state is ready,os should decrease
 * the ready num with function decrease_ready_num() in another place
 * which means, this functin do not decrease the ready num
 * @Author      kaka
 * @DateTime    2018-10-01
 * @description :
 * @param       TCB_ptr    [description]
 * @return                 [description]
 */
int delete_from_TCB_list(TCB *TCB_ptr)
{
	ASSERT(NULL != TCB_ptr);
#if CONFIG_PARA_CHECK
	if(NULL == TCB_ptr)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(delete_from_TCB_list,TCB_ptr);
		return -ERROR_NULL_INPUT_PTR;
	}
#endif
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
	//should not go here
	ASSERT(0);
	return ERROR_VALUELESS_INPUT;
}

inline struct list_head *get_from_TCB_list(unsigned int index)
{
	ASSERT(index < PRIO_MAX);
#if CONFIG_PARA_CHECK
	if(index >= PRIO_MAX)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(get_from_TCB_list,index);
		return NULL;
	}
#endif
	return &TCB_list[index].head;
}

inline unsigned char get_ready_num_from_TCB_list(unsigned int index)
{
	ASSERT(index < PRIO_MAX);
#if CONFIG_PARA_CHECK
	if(index >= PRIO_MAX)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(get_ready_num_from_TCB_list,index);
		return -1;
	}
#endif
	return TCB_list[index].ready_num;
}

void decrease_ready_num(unsigned int index)
{
	ASSERT(index < PRIO_MAX);
	ASSERT(0 != TCB_list[index].ready_num);
	--(TCB_list[index].ready_num);
}

void increase_ready_num(unsigned int index)
{
	ASSERT(index < PRIO_MAX);
	++(TCB_list[index].ready_num);
}

void decrease_TCB_num(unsigned int index)
{
	ASSERT(index < PRIO_MAX);
	ASSERT(0 != TCB_list[index].TCB_num);
	--(TCB_list[index].TCB_num);
}

void increase_TCB_num(unsigned int index)
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
				ka_printf("stack: %x\n",TCB_ptr->stack);
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
				ka_printf("prio %d TCB_num error!\n",i);
			}
			if(ready_num != TCB_list[i].ready_num)
			{
				ka_printf("prio %d ready_num error!\n",i);
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
				ka_printf("R4 	= 	%x\n",*reg++);
				ka_printf("R5 	= 	%x\n",*reg++);
				ka_printf("R6 	= 	%x\n",*reg++);
				ka_printf("R7 	= 	%x\n",*reg++);
				ka_printf("R8 	= 	%x\n",*reg++);
				ka_printf("R9 	= 	%x\n",*reg++);
				ka_printf("R10 	= 	%x\n",*reg++);
				ka_printf("R11 	= 	%x\n",*reg++);
				ka_printf("xPSR	=	%x\n",*reg++);
				ka_printf("PC 	= 	%x\n",*reg++);
				ka_printf("R12 	= 	%x\n",*reg++);
				ka_printf("R3 	= 	%x\n",*reg++);
				ka_printf("R2 	= 	%x\n",*reg++);
				ka_printf("R1 	= 	%x\n",*reg++);
				ka_printf("R0 	= 	%x\n",*reg);
				ka_printf("=====================================================\n");
			}
		}
	}
}

#endif 
