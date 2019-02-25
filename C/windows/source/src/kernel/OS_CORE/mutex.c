#include <mutex.h>
#include <os_schedule.h>
#include <myassert.h>
#include <os_cpu.h>
#include <os_suspend.h>
#include <export.h>

extern volatile TCB * OSTCBCurPtr;

inline static int compare(struct insert_sort_data *data1,struct insert_sort_data *data2)
{
	return ((TCB*)(data1->data_ptr))->prio - ((TCB*)(data2->data_ptr))->prio;
}

inline static int value_cmp(void *data1,void *data2)
{
	return ((TCB*)data1)->prio - ((TCB*)data2)->prio;
}

int mutex_init(MUTEX *MUTEX_ptr)
{
	ASSERT(NULL != MUTEX_ptr);
#if CONFIG_PARA_CHECK
	if(NULL == MUTEX_ptr)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(mutex_init,MUTEX_ptr);
		return -ERROR_NULL_INPUT_PTR;
	}
#endif
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	if (g_interrupt_count > 0)
	{
		CPU_CRITICAL_EXIT();
		return -ERROR_FUN_USE_IN_INTER;
	}
	MUTEX_ptr->mutex_flag = MUTEX_UNLOCK;
	MUTEX_ptr->owner_TCB_ptr = NULL;
	init_insert_sort_entity(
		&MUTEX_ptr->mutex_insert_sort_TCB_list,
		compare,
		NULL,
		value_cmp
		);
	CPU_CRITICAL_EXIT();
	return FUN_EXECUTE_SUCCESSFULLY;
}
EXPORT_SYMBOL(mutex_init);

int mutex_lock(MUTEX *MUTEX_ptr)
{
	ASSERT(NULL != MUTEX_ptr);
#if CONFIG_PARA_CHECK
	if(NULL == MUTEX_ptr)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(mutex_lock,MUTEX_ptr);
		return -ERROR_NULL_INPUT_PTR;
	}
#endif
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();//enter critical
	if (g_interrupt_count > 0)
	{
		CPU_CRITICAL_EXIT();
		return -ERROR_FUN_USE_IN_INTER;
	}
	if(MUTEX_UNLOCK == MUTEX_ptr->mutex_flag)
	{
		MUTEX_ptr->mutex_flag = MUTEX_LOCK;
		MUTEX_ptr->owner_TCB_ptr = (TCB *)OSTCBCurPtr;
		CPU_CRITICAL_EXIT();
	}
	else
	{
		ASSERT(MUTEX_LOCK == MUTEX_ptr->mutex_flag);
		if(MUTEX_ptr->owner_TCB_ptr == (TCB *)OSTCBCurPtr)
		{
			CPU_CRITICAL_EXIT();
			return FUN_EXECUTE_SUCCESSFULLY;
		}
		struct insert_sort_data data;
		init_insert_sort_data(&data,(void *)OSTCBCurPtr);
		insert_sort_insert_into(&data,&MUTEX_ptr->mutex_insert_sort_TCB_list);
		TCB *TCB_ptr = (TCB *)(insert_sort_get_first_data_ptr(&MUTEX_ptr->mutex_insert_sort_TCB_list)->data_ptr);
		if(MUTEX_ptr->owner_TCB_ptr->prio > TCB_ptr->prio)
		{
			if(RESERVED_PRIO == MUTEX_ptr->owner_TCB_ptr->reserve_prio)
			{
				MUTEX_ptr->owner_TCB_ptr->reserve_prio = MUTEX_ptr->owner_TCB_ptr->prio;
			}
			task_change_prio(MUTEX_ptr->owner_TCB_ptr,TCB_ptr->prio);
		}
		sys_suspend(STATE_WAIT_MUTEX_FOREVER);
		CPU_CRITICAL_EXIT(); //exit  critical
		//go back here=========go back here
		CPU_CRITICAL_ENTER();
		if(TCB_state_is_bad(OSTCBCurPtr))
		{
			clear_bad_state(OSTCBCurPtr);
			CPU_CRITICAL_EXIT();
			return -ERROR_MODULE_DELETED;
		}
		CPU_CRITICAL_EXIT();
	}
	return FUN_EXECUTE_SUCCESSFULLY;
}
EXPORT_SYMBOL(mutex_lock);

int mutex_try_lock(MUTEX *MUTEX_ptr)
{
	ASSERT(NULL != MUTEX_ptr);
#if CONFIG_PARA_CHECK
	if(NULL == MUTEX_ptr)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(mutex_try_lock,MUTEX_ptr);
		return -ERROR_NULL_INPUT_PTR;
	}
#endif
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();//enter critical
	if(MUTEX_UNLOCK == MUTEX_ptr->mutex_flag)
	{
		MUTEX_ptr->mutex_flag = MUTEX_LOCK;
		MUTEX_ptr->owner_TCB_ptr = (TCB *)OSTCBCurPtr;
		CPU_CRITICAL_EXIT();
		return FUN_EXECUTE_SUCCESSFULLY;
	}
	else
	{
		ASSERT(MUTEX_LOCK == MUTEX_ptr->mutex_flag);
		CPU_CRITICAL_EXIT(); //exit  critical
		return -ERROR_MUTEX_LOCK_FAIL;
	}
}
EXPORT_SYMBOL(mutex_try_lock);

int mutex_unlock(MUTEX *MUTEX_ptr)
{
	ASSERT(NULL != MUTEX_ptr);
#if CONFIG_PARA_CHECK
	if(NULL == MUTEX_ptr)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(mutex_unlock,MUTEX_ptr);
		return -ERROR_NULL_INPUT_PTR;
	}
#endif
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();//enter critical
	if(MUTEX_UNLOCK == MUTEX_ptr->mutex_flag)
	{
		CPU_CRITICAL_EXIT();
		return FUN_EXECUTE_SUCCESSFULLY;
	}
	else if(MUTEX_ptr->owner_TCB_ptr != OSTCBCurPtr)
	{
		CPU_CRITICAL_EXIT();
		return -ERROR_MUTEX_UNLOCK_FAIL;
	}
	if(RESERVED_PRIO != OSTCBCurPtr->reserve_prio)
	{
		TASK_PRIO_TYPE buffer = OSTCBCurPtr->reserve_prio;
		OSTCBCurPtr->reserve_prio = RESERVED_PRIO;
		task_change_prio_self(buffer);
	}
	struct insert_sort_data *buffer_ptr = insert_sort_delete_head(&MUTEX_ptr->mutex_insert_sort_TCB_list);
	if(NULL == buffer_ptr) // no task is waiting for the mutex
	{
		MUTEX_ptr->mutex_flag = MUTEX_UNLOCK;
		MUTEX_ptr->owner_TCB_ptr = NULL;
		CPU_CRITICAL_EXIT();
		return FUN_EXECUTE_SUCCESSFULLY;
	}
	else// a task is waiting for the mutex
	{
		remove_from_suspend_list((TCB *)(buffer_ptr->data_ptr));
		MUTEX_ptr->owner_TCB_ptr = (TCB *)(buffer_ptr->data_ptr);
		schedule();
		CPU_CRITICAL_EXIT();
		return FUN_EXECUTE_SUCCESSFULLY;
	}
}
EXPORT_SYMBOL(mutex_unlock);

int mutex_del(MUTEX *MUTEX_ptr)
{
	ASSERT(NULL != MUTEX_ptr);
#if CONFIG_PARA_CHECK
	if(NULL == MUTEX_ptr)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(mutex_del,MUTEX_ptr);
		return -ERROR_NULL_INPUT_PTR;
	}
#endif
	TCB *TCB_ptr;
	struct insert_sort_data *insert_sort_data_ptr = insert_sort_delete_head(&MUTEX_ptr->mutex_insert_sort_TCB_list);
	while(NULL != insert_sort_data_ptr)
	{
		TCB_ptr = (TCB *)(insert_sort_data_ptr->data_ptr);
		ASSERT(STATE_WAIT_MUTEX_FOREVER == TCB_ptr->task_state);
		remove_from_suspend_list(TCB_ptr);
		set_bad_state(TCB_ptr);
		insert_sort_data_ptr = insert_sort_delete_head(&MUTEX_ptr->mutex_insert_sort_TCB_list);
	}
	return FUN_EXECUTE_SUCCESSFULLY;
}
EXPORT_SYMBOL(mutex_del);
