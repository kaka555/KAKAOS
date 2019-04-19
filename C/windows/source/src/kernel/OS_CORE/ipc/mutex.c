#include <mutex.h>
#include <os_schedule.h>
#include <myassert.h>
#include <os_cpu.h>
#include <os_suspend.h>
#include <export.h>

extern volatile TCB * OSTCBCurPtr;

static int compare(struct insert_sort_data *data1,struct insert_sort_data *data2)
{
	return ((TCB*)(data1->data_ptr))->prio - ((TCB*)(data2->data_ptr))->prio;
}

static int value_cmp(void *data1,void *data2)
{
	return ((TCB*)data1)->prio - ((TCB*)data2)->prio;
}

int _mutex_init(MUTEX *MUTEX_ptr)
{
	ASSERT(NULL != MUTEX_ptr,ASSERT_INPUT);
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
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

int mutex_init(MUTEX *MUTEX_ptr)
{
	if(NULL == MUTEX_ptr)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(mutex_init,MUTEX_ptr);
		return -ERROR_NULL_INPUT_PTR;
	}
	return _mutex_init(MUTEX_ptr);
}
EXPORT_SYMBOL(mutex_init);

int _mutex_lock(MUTEX *MUTEX_ptr)
{
	ASSERT(NULL != MUTEX_ptr,ASSERT_INPUT);
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();/*enter critical*/
	if(MUTEX_UNLOCK == MUTEX_ptr->mutex_flag)
	{
		MUTEX_ptr->mutex_flag = MUTEX_LOCK;
		MUTEX_ptr->owner_TCB_ptr = (TCB *)OSTCBCurPtr;
		CPU_CRITICAL_EXIT();
	}
	else
	{
		ASSERT(MUTEX_LOCK == MUTEX_ptr->mutex_flag,ASSERT_PARA_AFFIRM);
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
		CPU_CRITICAL_EXIT(); 
		/*go back here=========go back here*/
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

int mutex_lock(MUTEX *MUTEX_ptr)
{
	if(NULL == MUTEX_ptr)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(mutex_lock,MUTEX_ptr);
		return -ERROR_NULL_INPUT_PTR;
	}
	if (g_interrupt_count > 0)
	{
		return -ERROR_FUN_USE_IN_INTER;
	}
	return _mutex_lock(MUTEX_ptr);
}
EXPORT_SYMBOL(mutex_lock);

int _mutex_try_lock(MUTEX *MUTEX_ptr)
{
	ASSERT(NULL != MUTEX_ptr,ASSERT_INPUT);
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	if(MUTEX_UNLOCK == MUTEX_ptr->mutex_flag)
	{
		MUTEX_ptr->mutex_flag = MUTEX_LOCK;
		MUTEX_ptr->owner_TCB_ptr = (TCB *)OSTCBCurPtr;
		CPU_CRITICAL_EXIT();
		return FUN_EXECUTE_SUCCESSFULLY;
	}
	else
	{
		ASSERT(MUTEX_LOCK == MUTEX_ptr->mutex_flag,ASSERT_PARA_AFFIRM);
		CPU_CRITICAL_EXIT(); 
		return -ERROR_SYS;
	}
}

int mutex_try_lock(MUTEX *MUTEX_ptr)
{
	if(NULL == MUTEX_ptr)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(mutex_try_lock,MUTEX_ptr);
		return -ERROR_NULL_INPUT_PTR;
	}
	return _mutex_try_lock(MUTEX_ptr);
}
EXPORT_SYMBOL(mutex_try_lock);

int _mutex_unlock(MUTEX *MUTEX_ptr)
{
	ASSERT(NULL != MUTEX_ptr,ASSERT_INPUT);
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	if(MUTEX_UNLOCK == MUTEX_ptr->mutex_flag)
	{
		CPU_CRITICAL_EXIT();
		return FUN_EXECUTE_SUCCESSFULLY;
	}
	else if(MUTEX_ptr->owner_TCB_ptr != OSTCBCurPtr)
	{
		CPU_CRITICAL_EXIT();
		return -ERROR_SYS;
	}
	if(RESERVED_PRIO != OSTCBCurPtr->reserve_prio)
	{
		TASK_PRIO_TYPE buffer = OSTCBCurPtr->reserve_prio;
		OSTCBCurPtr->reserve_prio = RESERVED_PRIO;
		task_change_prio_self(buffer);
	}
	struct insert_sort_data *buffer_ptr = insert_sort_delete_head(&MUTEX_ptr->mutex_insert_sort_TCB_list);
	if(NULL == buffer_ptr) /* no task is waiting for the mutex*/
	{
		MUTEX_ptr->mutex_flag = MUTEX_UNLOCK;
		MUTEX_ptr->owner_TCB_ptr = NULL;
	}
	else/* a task is waiting for the mutex*/
	{
		_remove_from_suspend_list((TCB *)(buffer_ptr->data_ptr));
		MUTEX_ptr->owner_TCB_ptr = (TCB *)(buffer_ptr->data_ptr);
		schedule();
	}
	CPU_CRITICAL_EXIT();
	return FUN_EXECUTE_SUCCESSFULLY;
}

int mutex_unlock(MUTEX *MUTEX_ptr)
{
	if(NULL == MUTEX_ptr)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(mutex_unlock,MUTEX_ptr);
		return -ERROR_NULL_INPUT_PTR;
	}
	return _mutex_unlock(MUTEX_ptr);
}
EXPORT_SYMBOL(mutex_unlock);

int _mutex_del(MUTEX *MUTEX_ptr)
{
	ASSERT(NULL != MUTEX_ptr,ASSERT_INPUT);
	TCB *TCB_ptr;
	struct insert_sort_data *insert_sort_data_ptr = insert_sort_delete_head(&MUTEX_ptr->mutex_insert_sort_TCB_list);
	while(NULL != insert_sort_data_ptr)
	{
		TCB_ptr = (TCB *)(insert_sort_data_ptr->data_ptr);
		ASSERT(STATE_WAIT_MUTEX_FOREVER == TCB_ptr->task_state,ASSERT_PARA_AFFIRM);
		_remove_from_suspend_list(TCB_ptr);
		set_bad_state(TCB_ptr);
		insert_sort_data_ptr = insert_sort_delete_head(&MUTEX_ptr->mutex_insert_sort_TCB_list);
	}
	return FUN_EXECUTE_SUCCESSFULLY;
}

int mutex_del(MUTEX *MUTEX_ptr)
{
	if(NULL == MUTEX_ptr)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(mutex_del,MUTEX_ptr);
		return -ERROR_NULL_INPUT_PTR;
	}
	return _mutex_del(MUTEX_ptr);
}
EXPORT_SYMBOL(mutex_del);
