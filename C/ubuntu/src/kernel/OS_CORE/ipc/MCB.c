#include <MCB.h>
#include <os_cpu.h>
#include <os_time.h>
#include <myassert.h>
#include <myMicroLIB.h>
#include <task_state.h>
#include <os_schedule.h>
#include <os_suspend.h>
#include <os_delay.h>
#include <export.h>


extern TCB * OSTCBCurPtr;

static int compare(struct insert_sort_data *data1,struct insert_sort_data *data2)
{
	return ((TCB*)(data1->data_ptr))->prio - ((TCB*)(data2->data_ptr))->prio;
}

static int value_cmp(void *data1,void *data2)
{
	return ((TCB*)data1)->prio - ((TCB*)data2)->prio;
}

int
_init_MCB(
	MCB *MCB_ptr,
	int num,
	unsigned int flag
	)
{
	ASSERT(NULL != MCB_ptr);
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	MCB_ptr->resource_num = num;
	init_insert_sort_entity(
		&MCB_ptr->MCB_insert_sort_list,
		compare,
		NULL,
		value_cmp
	);
	MCB_ptr->flag = flag;
	if(MCB_type_is_binary(MCB_ptr))
	{
		if(num)
		{
			MCB_ptr->resource_num = 1;
		}
	}
	CPU_CRITICAL_EXIT();
	return FUN_EXECUTE_SUCCESSFULLY;
}

int
init_MCB(
	MCB *MCB_ptr,
	int num,
	unsigned int flag
	)
{
	if(NULL == MCB_ptr)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(init_MCB,MCB_ptr);
		return -ERROR_NULL_INPUT_PTR;
	}
	if (g_interrupt_count > 0)
	{
		return -ERROR_FUN_USE_IN_INTER;
	}
	return _init_MCB(MCB_ptr,num,flag);
}
EXPORT_SYMBOL(init_MCB);

int _delete_MCB(MCB *MCB_ptr)
{
	ASSERT(NULL != MCB_ptr);
	TCB *TCB_ptr;
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();/*enter critical*/
	struct insert_sort_data *insert_sort_data_ptr = insert_sort_delete_head(&MCB_ptr->MCB_insert_sort_list);
	while(NULL != insert_sort_data_ptr)
	{
		TCB_ptr = (TCB *)(insert_sort_data_ptr->data_ptr);
		if(STATE_WAIT_MCB_FOREVER == TCB_ptr->task_state)
		{
			_remove_from_suspend_list(TCB_ptr);
		}
		else
		{
			ASSERT(STATE_WAIT_MCB_TIMEOUT == TCB_ptr->task_state);
			_remove_from_delay_heap(TCB_ptr);
		}
		set_bad_state(TCB_ptr);
		insert_sort_data_ptr = insert_sort_delete_head(&MCB_ptr->MCB_insert_sort_list);
	}
	schedule();
	CPU_CRITICAL_EXIT();
	return FUN_EXECUTE_SUCCESSFULLY;
}

int delete_MCB(MCB *MCB_ptr)
{
	if(NULL == MCB_ptr)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(delete_MCB,MCB_ptr);
		return -ERROR_NULL_INPUT_PTR;
	}
	return _delete_MCB(MCB_ptr);
}
EXPORT_SYMBOL(delete_MCB);

int _p(
	MCB *MCB_ptr,
	MCB_WAIT_FLAG flag,
	unsigned int time
)
{
	ASSERT(NULL != MCB_ptr);
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();/*enter critical*/
	
	if (g_interrupt_count > 0)
	{
		CPU_CRITICAL_EXIT();
		return -ERROR_FUN_USE_IN_INTER;
	}

	if((flag ==  MCB_FLAG_NON_BLOCKING) && (MCB_ptr->resource_num <= 0))
	{
		CPU_CRITICAL_EXIT();
		return -MCB_OUT_OF_RESOURCE;
	}

	--(MCB_ptr->resource_num);
	if(MCB_ptr->resource_num >= 0)
	{
		CPU_CRITICAL_EXIT();
		return FUN_EXECUTE_SUCCESSFULLY;
	}

/*the following program is used to deal with FLAG_WAIT && MCB_OUT_OF_RESOURCE*/
	DECLEAR_INSERT_SORT_DATA(data);
	init_insert_sort_data(&data,OSTCBCurPtr);
	insert_sort_insert_into(&data,&MCB_ptr->MCB_insert_sort_list);
	if(time == 0)/*0 represent forever*/
	{
		CPU_CRITICAL_EXIT(); //exit  critical
		sys_suspend(STATE_WAIT_MCB_FOREVER);
		/*go back here=========go back here*/
		CPU_CRITICAL_ENTER();/*enter critical*/
	}
	else
	{
		UINT64 time_record = _get_tick();
		CPU_CRITICAL_EXIT(); /*exit  critical*/
		sys_delay(time,STATE_WAIT_MCB_TIMEOUT);
		/*go back here=========go back here*/
		CPU_CRITICAL_ENTER();/*enter critical*/
		if(_get_tick() >= (time_record + time))/*prove that timeout happened*/
		{
			++(MCB_ptr->resource_num);
			insert_sort_delete_data(&MCB_ptr->MCB_insert_sort_list,OSTCBCurPtr);
			CPU_CRITICAL_EXIT(); /*exit  critical*/
			return -MCB_OUT_OF_TIME;
		}
	}
	if(TCB_state_is_bad(OSTCBCurPtr))
	{
		clear_bad_state(OSTCBCurPtr);
		CPU_CRITICAL_EXIT(); /*exit  critical*/
		return -ERROR_MODULE_DELETED;
	}
	CPU_CRITICAL_EXIT(); /*exit  critical*/
	return FUN_EXECUTE_SUCCESSFULLY;
}

int p(
	MCB *MCB_ptr,
	MCB_WAIT_FLAG flag,
	unsigned int time)
{
	if(NULL == MCB_ptr)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(p,MCB_ptr);
		return -ERROR_NULL_INPUT_PTR;
	}
	return _p(MCB_ptr,flag,time);
}
EXPORT_SYMBOL(p);

int _v(MCB *MCB_ptr)
{
	ASSERT(NULL != MCB_ptr);
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();/*enter critical*/
	if((++(MCB_ptr->resource_num) > 1) && MCB_type_is_binary(MCB_ptr))
	{
		ASSERT(2 == MCB_ptr->resource_num);
		MCB_ptr->resource_num = 1;
	}
	if(MCB_ptr->resource_num <= 0)
	{
		TCB *TCB_ptr = (TCB *)(insert_sort_delete_head(&MCB_ptr->MCB_insert_sort_list))->data_ptr;
		
		if(STATE_WAIT_MCB_FOREVER == TCB_ptr->task_state)
			_remove_from_suspend_list(TCB_ptr);
		else if(STATE_WAIT_MCB_TIMEOUT == TCB_ptr->task_state)/*STATE_WAIT_MESSAGE_TIMEOUT*/
			_remove_from_delay_heap(TCB_ptr);
		schedule();
	}
	CPU_CRITICAL_EXIT(); /*exit  critical*/
	return FUN_EXECUTE_SUCCESSFULLY;
}

int v(MCB *MCB_ptr)
{
	if(NULL == MCB_ptr)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(v,MCB_ptr);
		return -ERROR_NULL_INPUT_PTR;
	}
  	return _v(MCB_ptr);
}
EXPORT_SYMBOL(v);

void _clear_MCB_index(MCB *MCB_ptr)
{
	ASSERT(NULL != MCB_ptr);
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	if(MCB_ptr->resource_num > 0)
		MCB_ptr->resource_num = 0;
	CPU_CRITICAL_EXIT();
}

int clear_MCB_index(MCB *MCB_ptr)
{
	if(NULL == MCB_ptr)
	{
		return -ERROR_NULL_INPUT_PTR;
	}
	_clear_MCB_index(MCB_ptr);
	return FUN_EXECUTE_SUCCESSFULLY;
}
EXPORT_SYMBOL(clear_MCB_index);

/*************************end of MCB*****************************/
