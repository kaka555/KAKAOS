#include <TCB.h>
#include <myassert.h>
#include <myMicroLIB.h>
#include <task_state.h>
#include <os_TCB_list.h>
#include <os_error.h>
#include <os_cpu_stm32.h>
#include <os_ready.h>
#include <os_suspend.h>
#include <os_delay.h>
#include <os_schedule.h>
#include <module.h>

volatile TCB *OSTCBCurPtr;
volatile TCB *OSTCBHighRdyPtr;
volatile int g_interrupt_count = 0;

static void delete_myself(void)
{
	task_delete((TCB *)OSTCBCurPtr);
}

int _must_check task_init(
	TCB *TCB_ptr,
	unsigned int stack_size,
	TASK_PRIO_TYPE prio,
	unsigned int timeslice_hope_time,
	const char *name,
	functionptr function,
	void *para,
	TASK_STATE state)
{
	ASSERT(prio < PRIO_MAX);
	ASSERT(NULL != name);
	ASSERT(NULL != TCB_ptr);
#if CONFIG_PARA_CHECK
	if((prio >= PRIO_MAX) || (NULL == name) || (NULL == TCB_ptr))
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(task_init,prio);
		OS_ERROR_PARA_MESSAGE_DISPLAY(task_init,name);
		OS_ERROR_PARA_MESSAGE_DISPLAY(task_init,TCB_ptr);
		return -ERROR_NULL_INPUT_PTR;
	}
#endif
	stack_size &= (~0x03);// according to CPU : 32bit?64bit
	TCB_ptr->stack_end = (STACK_TYPE *)ka_malloc(stack_size);
	if(NULL == TCB_ptr->stack_end)
	{
		return -ERROR_ALLOCATE_STACK;
	}
	TCB_ptr->stack_size = stack_size;
	TCB_ptr->stack = (STACK_TYPE *)TCB_ptr->stack_end + stack_size/4 - 1;
	ASSERT((char *)(TCB_ptr->stack) == (char *)(TCB_ptr->stack_end) + stack_size - 4);
	ka_memset(TCB_ptr->stack_end,0,stack_size);
	set_register((void **)&TCB_ptr->stack,function,delete_myself,para);
	TCB_ptr->prio = prio;
	TCB_ptr->reserve_prio = RESERVED_PRIO;
	TCB_ptr->task_state = state;
	TCB_ptr->attribution = DEFAULT_ATTRIBUTION | TCB_ATTRIBUTION_INIT;
	TCB_ptr->delay_reach_time = 0;
	TCB_ptr->delay_heap_position = 0;
	INIT_LIST_HEAD(&TCB_ptr->same_prio_list);
	INIT_LIST_HEAD(&TCB_ptr->suspend_list);
	TCB_ptr->name = (char *)name;
	TCB_ptr->timeslice_hope_time = timeslice_hope_time;
	TCB_ptr->timeslice_rest_time = timeslice_hope_time;
	TCB_ptr->dynamic_module_ptr = NULL;
	register_in_TCB_list(TCB_ptr);
	return FUN_EXECUTE_SUCCESSFULLY;
}

TCB *_must_check task_creat(
	unsigned int stack_size,
	TASK_PRIO_TYPE prio,
	unsigned int timeslice_hope_time,
	const char *name,
	functionptr function,
	void *para,
	TASK_STATE state)
{
	ASSERT((prio < PRIO_MAX));
	ASSERT(NULL != name);
#if CONFIG_PARA_CHECK
	if((prio >= PRIO_MAX) || (NULL == name))
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(task_creat,prio);
		OS_ERROR_PARA_MESSAGE_DISPLAY(task_creat,name);
		return NULL;
	}
#endif
	TCB *TCB_ptr = ka_malloc(sizeof(TCB));
	if(NULL == TCB_ptr)
	{
		return NULL;
	}
	if(FUN_EXECUTE_SUCCESSFULLY != 
		task_init(	TCB_ptr,
					stack_size,
					prio,
					timeslice_hope_time,
					name,
					function,
					para,
					state)
	  )
	{
		ka_free(TCB_ptr);
		return NULL;
	}
	TCB_ptr->attribution = DEFAULT_ATTRIBUTION | TCB_ATTRIBUTION_CREATE;
	return TCB_ptr;
}



//this function shoul
int task_change_prio(TCB *TCB_ptr,TASK_PRIO_TYPE prio)
{
	ASSERT(prio < PRIO_MAX);
	ASSERT(NULL != TCB_ptr);
#if CONFIG_PARA_CHECK
	if((prio >= PRIO_MAX) || (NULL == TCB_ptr))
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(task_change_prio,prio);
		OS_ERROR_PARA_MESSAGE_DISPLAY(task_change_prio,TCB_ptr);
		return -ERROR_NULL_INPUT_PTR;
	}
#endif
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	//1 step: delete it from TCB_list
	delete_from_TCB_list(TCB_ptr);
	//2 step: according to the task_state,do some changes
	switch(TCB_ptr->task_state)
	{
		case STATE_READY:
			delete_TCB_from_ready(TCB_ptr);break;
		//add other state's handling code here
		/*
		 *
		 *
		 * 
		 */
		default: break;// do nothing
	}
	//3 step: change the prio
	TCB_ptr->prio = prio;
	//4 step: register into TCB_list
	register_in_TCB_list(TCB_ptr);
	//5 step: according to the task_state,do some changes
	//this place should add different code according to the different task_state
	switch(TCB_ptr->task_state)
	{
		case STATE_READY:
			insert_ready_TCB(TCB_ptr);break;
		//add other state's handling code here
		/*
		 *
		 *
		 * 
		 */
		default: break;// do nothing
	}
	CPU_CRITICAL_EXIT();
	schedule();
	return FUN_EXECUTE_SUCCESSFULLY;
}

int task_delete(TCB *TCB_ptr)
{
	ASSERT(NULL != TCB_ptr);
#if CONFIG_PARA_CHECK
	if(NULL == TCB_ptr)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(task_delete,TCB_ptr);
		return -ERROR_NULL_INPUT_PTR;
	}
#endif
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	delete_from_TCB_list(TCB_ptr);
	ka_free(TCB_ptr->stack_end);
	switch(TCB_ptr->task_state)
	{
		case STATE_SUSPEND_NORMAL:
			remove_from_suspend_list(TCB_ptr);break;
		case STATE_DELAY:
			remove_from_delay_heap(TCB_ptr);break;
		case STATE_READY:
			delete_TCB_from_ready(TCB_ptr);break;
		//add other state's handling code here
		/*
		 *
		 *
		 * 
		 */
		default:
			break;
	}
	if(TCB_IS_CREATED(TCB_ptr))
	{
		ka_free(TCB_ptr);
	}
	if(is_module(TCB_ptr))
	{
		ASSERT(TCB_ptr->dynamic_module_ptr);
		set_module_state(TCB_ptr->dynamic_module_ptr,MODULE_STATE_LOADED);
	}
	CPU_CRITICAL_EXIT();
	schedule();
	return FUN_EXECUTE_SUCCESSFULLY;
}

