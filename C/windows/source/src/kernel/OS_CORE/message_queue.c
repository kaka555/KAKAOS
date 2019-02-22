#include <message_queue.h>
#include <myassert.h>
#include <os_schedule.h>
#include <os_cpu.h>
#include <os_time.h>
#include <os_delay.h>
#include <os_suspend.h>

extern TCB * OSTCBCurPtr;

inline static int compare(struct insert_sort_data *data1,struct insert_sort_data *data2)
{
	return ((TCB*)(data1->data_ptr))->prio - ((TCB*)(data2->data_ptr))->prio;
}

inline static int value_cmp(void *data1,void *data2)
{
	return ((TCB*)data1)->prio - ((TCB*)data2)->prio;
}

int msg_init(MQB *MQB_ptr,char *name,unsigned int max_message_num)
{
	ASSERT(NULL != MQB_ptr);
	ASSERT(max_message_num > 0);
#if CONFIG_PARA_CHECK
	if((NULL == MQB_ptr) || (0 == max_message_num))
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(msg_init,MQB_ptr);
		OS_ERROR_PARA_MESSAGE_DISPLAY(msg_init,max_message_num);
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
	INIT_LIST_HEAD(&MQB_ptr->message_list);
	init_insert_sort_entity(
		&MQB_ptr->message_array_insert_sort_wait_TCB_list,
		compare,
		NULL,
		value_cmp);
	init_insert_sort_entity(
		&MQB_ptr->message_array_insert_sort_put_TCB_list,
		compare,
		NULL,
		value_cmp);
	MQB_ptr->put_num = 0;
	MQB_ptr->wait_num = 0;
	MQB_ptr->name = name;
	MQB_ptr->current_message_num = 0;
	MQB_ptr->max_message_num = max_message_num;
	CPU_CRITICAL_EXIT();
	return FUN_EXECUTE_SUCCESSFULLY;
}

int msg_send(MQB *MQB_ptr,struct message *message_ptr,MSG_FLAG flag,unsigned int time)
{
	ASSERT(NULL != MQB_ptr);
	ASSERT(NULL != message_ptr);
#if CONFIG_PARA_CHECK
	if((NULL == MQB_ptr) || (NULL == message_ptr))
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(msg_send,MQB_ptr);
		OS_ERROR_PARA_MESSAGE_DISPLAY(msg_send,max_message_num);
		return -ERROR_NULL_INPUT_PTR;
	}
#endif
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	if(MQB_ptr->current_message_num < MQB_ptr->max_message_num)
	{
		//add to tail
		list_add_tail(&message_ptr->message_list,&MQB_ptr->message_list);
		++(MQB_ptr->current_message_num);
		//then check if there is a task waiting for receiving
		if(MQB_ptr->wait_num)
		{
			struct insert_sort_data *insert_sort_data_ptr;
			TCB *TCB_ptr;
			insert_sort_data_ptr = insert_sort_delete_head(&MQB_ptr->message_array_insert_sort_wait_TCB_list);
			ASSERT(NULL != insert_sort_data_ptr);
			TCB_ptr = (TCB *)(insert_sort_data_ptr->data_ptr);
			if(STATE_WAIT_MESSAGE_QUEUE_FOREVER == TCB_ptr->task_state)
			{
				remove_from_suspend_list(TCB_ptr);
			}
			else
			{
				ASSERT(STATE_WAIT_MESSAGE_QUEUE_TIMEOUT == TCB_ptr->task_state);
				remove_from_delay_heap(TCB_ptr);
			}
			schedule();
		}
		CPU_CRITICAL_EXIT();
		return FUN_EXECUTE_SUCCESSFULLY;
	}
	// else : MQB_ptr->current_message_num == MQB_ptr->max_message_num
	ASSERT(MQB_ptr->current_message_num == MQB_ptr->max_message_num);
	if(MSG_FLAG_NON_BLOCKING == flag)
	{
		CPU_CRITICAL_EXIT();
		return -ERROR_MSG_FULL;
	}
	// else : flag == FLAG_WAIT
	DECLEAR_INSERT_SORT_DATA(data);
	init_insert_sort_data(&data,OSTCBCurPtr);
	insert_sort_insert_into(&data,&MQB_ptr->message_array_insert_sort_put_TCB_list);
	++(MQB_ptr->put_num);
	if(0 == time)
	{
		CPU_CRITICAL_EXIT(); //exit  critical
		sys_suspend(STATE_PUT_MESSAGE_QUEUE_FOREVER);
		//go back here=========go back here
		CPU_CRITICAL_ENTER();
	}
	else
	{
		CPU_CRITICAL_EXIT(); //exit  critical
		int num = sys_delay(time,STATE_PUT_MESSAGE_QUEUE_TIMEOUT);
		//go back here=========go back here
		CPU_CRITICAL_ENTER();//enter critical
		if(num <= 0)//prove that timeout happened
		{
			--(MQB_ptr->put_num);
			insert_sort_delete_data(&MQB_ptr->message_array_insert_sort_put_TCB_list,OSTCBCurPtr);
			CPU_CRITICAL_EXIT(); //exit  critical
			return -MSG_OUT_OF_TIME;
		}
	}
	if(TCB_state_is_bad(OSTCBCurPtr))
	{
		clear_bad_state(OSTCBCurPtr);
		CPU_CRITICAL_EXIT();
		return -ERROR_MODULE_DELETED;
	}
	--(MQB_ptr->put_num);
	//add to tail
	list_add_tail(&message_ptr->message_list,&MQB_ptr->message_list);
	++(MQB_ptr->current_message_num);
	ASSERT(MQB_ptr->current_message_num <= MQB_ptr->max_message_num);
	CPU_CRITICAL_EXIT();
	return FUN_EXECUTE_SUCCESSFULLY;
}

int msg_receive(MQB *MQB_ptr,struct message **message_ptr,MSG_FLAG flag,unsigned int time)
{
	ASSERT(NULL != MQB_ptr);
	ASSERT(NULL != message_ptr);
#if CONFIG_PARA_CHECK
	if((NULL == MQB_ptr) || (NULL == message_ptr))
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(msg_receive,MQB_ptr);
		OS_ERROR_PARA_MESSAGE_DISPLAY(msg_receive,max_message_num);
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
	if(0 < MQB_ptr->current_message_num)
	{
		--(MQB_ptr->current_message_num);
		//get the first message
		*message_ptr = list_entry(MQB_ptr->message_list.next,struct message,message_list);
		//detatch it from list
		list_del(&(*message_ptr)->message_list);
		//then check if there is a task waiting for sending
		if(MQB_ptr->put_num)
		{
			struct insert_sort_data *insert_sort_data_ptr;
			TCB *TCB_ptr;
			insert_sort_data_ptr = insert_sort_delete_head(&MQB_ptr->message_array_insert_sort_put_TCB_list);
			ASSERT(NULL != insert_sort_data_ptr);
			TCB_ptr = (TCB *)(insert_sort_data_ptr->data_ptr);
			if(STATE_PUT_MESSAGE_QUEUE_FOREVER == TCB_ptr->task_state)
			{
				remove_from_suspend_list(TCB_ptr);
			}
			else
			{
				ASSERT(STATE_PUT_MESSAGE_QUEUE_TIMEOUT == TCB_ptr->task_state);
				remove_from_delay_heap(TCB_ptr);
			}
			schedule();
		}
		CPU_CRITICAL_EXIT();
		return FUN_EXECUTE_SUCCESSFULLY;
	}
	// else : MQB_ptr->current_message_num == 0
	ASSERT(0 == MQB_ptr->current_message_num);
	if(MSG_FLAG_NON_BLOCKING == flag)
	{
		CPU_CRITICAL_EXIT();
		return -ERROR_MSG_FULL;
	}
	// else : flag == FLAG_WAIT
	DECLEAR_INSERT_SORT_DATA(data);
	init_insert_sort_data(&data,OSTCBCurPtr);
	insert_sort_insert_into(&data,&MQB_ptr->message_array_insert_sort_wait_TCB_list);
	++(MQB_ptr->wait_num);
	if(0 == time)
	{
		CPU_CRITICAL_EXIT(); //exit  critical
		sys_suspend(STATE_WAIT_MESSAGE_QUEUE_FOREVER);
		//go back here=========go back here
		CPU_CRITICAL_ENTER();
	}
	else
	{
		CPU_CRITICAL_EXIT(); //exit  critical
		int num = sys_delay(time,STATE_WAIT_MESSAGE_QUEUE_TIMEOUT);
		//go back here=========go back here
		CPU_CRITICAL_ENTER();//enter critical
		if(num <= 0)//prove that timeout happened
		{
			--(MQB_ptr->wait_num);
			insert_sort_delete_data(&MQB_ptr->message_array_insert_sort_wait_TCB_list,OSTCBCurPtr);
			CPU_CRITICAL_EXIT(); //exit  critical
			return -MSG_OUT_OF_TIME;
		}
	}
	if(TCB_state_is_bad(OSTCBCurPtr))
	{
		clear_bad_state(OSTCBCurPtr);
		CPU_CRITICAL_EXIT();
		return -ERROR_MODULE_DELETED;
	}
	--(MQB_ptr->wait_num);
	ASSERT(MQB_ptr->current_message_num > 0);
	--(MQB_ptr->current_message_num);
	//get the first message
	*message_ptr = list_entry(MQB_ptr->message_list.next,struct message,message_list);
	//detatch it from list
	list_del(&(*message_ptr)->message_list);
	CPU_CRITICAL_EXIT();
	return FUN_EXECUTE_SUCCESSFULLY;
}

int msg_del(MQB *MQB_ptr)
{
	ASSERT(NULL != MQB_ptr);
#if CONFIG_PARA_CHECK
	if(NULL == MQB_ptr)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(msg_del,MQB_ptr);
		return -ERROR_NULL_INPUT_PTR;
	}
#endif
	TCB *TCB_ptr;
	struct insert_sort_data *insert_sort_data_ptr = insert_sort_delete_head(&MQB_ptr->message_array_insert_sort_wait_TCB_list);
	while(NULL != insert_sort_data_ptr)
	{
		TCB_ptr = (TCB *)(insert_sort_data_ptr->data_ptr);
		if(STATE_WAIT_MESSAGE_QUEUE_FOREVER == TCB_ptr->task_state)
		{
			remove_from_suspend_list(TCB_ptr);
		}
		else
		{
			ASSERT(STATE_WAIT_MESSAGE_QUEUE_TIMEOUT == TCB_ptr->task_state);
			remove_from_delay_heap(TCB_ptr);
		}
		set_bad_state(TCB_ptr);
		insert_sort_data_ptr = insert_sort_delete_head(&MQB_ptr->message_array_insert_sort_wait_TCB_list);
	}
	insert_sort_data_ptr = insert_sort_delete_head(&MQB_ptr->message_array_insert_sort_put_TCB_list);
	while(NULL != insert_sort_data_ptr)
	{
		TCB_ptr = (TCB *)(insert_sort_data_ptr->data_ptr);
		if(STATE_PUT_MESSAGE_QUEUE_FOREVER == TCB_ptr->task_state)
		{
			remove_from_suspend_list(TCB_ptr);
		}
		else
		{
			ASSERT(STATE_PUT_MESSAGE_QUEUE_TIMEOUT == TCB_ptr->task_state);
			remove_from_delay_heap(TCB_ptr);
		}
		set_bad_state(TCB_ptr);
		insert_sort_data_ptr = insert_sort_delete_head(&MQB_ptr->message_array_insert_sort_put_TCB_list);
	}
	return FUN_EXECUTE_SUCCESSFULLY;
}

int message_init(struct message *message_ptr,unsigned int message_size,void *data)
{
	ASSERT(NULL != message_ptr);
#if CONFIG_PARA_CHECK
	if(NULL == message_ptr)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(message_init,message_ptr);
		return -ERROR_NULL_INPUT_PTR;
	}
#endif
	INIT_LIST_HEAD(&message_ptr->message_list);
	message_ptr->message_size = message_size;
	message_ptr->message_data_ptr = data;
	message_ptr->tick = get_tick();
	return FUN_EXECUTE_SUCCESSFULLY;
}
