#include <message_queue.h>
#include <myassert.h>
#include <os_schedule.h>
#include <os_cpu.h>
#include <os_time.h>
#include <os_delay.h>
#include <os_suspend.h>
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

int _msg_init(MQB *MQB_ptr,char *name,unsigned int max_message_num)
{
	ASSERT(NULL != MQB_ptr,ASSERT_INPUT);
	ASSERT(max_message_num > 0,ASSERT_INPUT);
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
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

/**
 * @Author      kaka
 * @DateTime    2019-04-21
 * @description : init the mqb
 * @param       MQB_ptr         
 * @param       name            
 * @param       max_message_num 
 * @return                      
 */
int msg_init(MQB *MQB_ptr,char *name,unsigned int max_message_num)
{
	if((NULL == MQB_ptr) || (NULL == name))
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(msg_init,MQB_ptr);
		OS_ERROR_PARA_MESSAGE_DISPLAY(msg_init,name);
		return -ERROR_NULL_INPUT_PTR;
	}
	if(0 == max_message_num)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(msg_init,max_message_num);
		return -ERROR_USELESS_INPUT;
	}
	if (g_interrupt_count > 0)
	{
		return -ERROR_FUN_USE_IN_INTER;
	}
	return _msg_init(MQB_ptr,name,max_message_num);
}
EXPORT_SYMBOL(msg_init);

int _msg_send(MQB *MQB_ptr,struct message *message_ptr,MSG_FLAG flag,unsigned int time)
{
	ASSERT(NULL != MQB_ptr,ASSERT_INPUT);
	ASSERT(NULL != message_ptr,ASSERT_INPUT);
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	if(MQB_ptr->current_message_num < MQB_ptr->max_message_num)
	{
		/*add to tail*/
		list_add_tail(&message_ptr->message_list,&MQB_ptr->message_list);
		++(MQB_ptr->current_message_num);
		/*then check if there is a task waiting for receiving*/
		if(MQB_ptr->wait_num)
		{
			struct insert_sort_data *insert_sort_data_ptr;
			TCB *TCB_ptr;
			insert_sort_data_ptr = insert_sort_delete_head(&MQB_ptr->message_array_insert_sort_wait_TCB_list);
			ASSERT(NULL != insert_sort_data_ptr,ASSERT_PARA_AFFIRM);
			TCB_ptr = (TCB *)(insert_sort_data_ptr->data_ptr);
			if(STATE_WAIT_MESSAGE_QUEUE_FOREVER == TCB_ptr->task_state)
			{
				_remove_from_suspend_list(TCB_ptr);
			}
			else
			{
				ASSERT(STATE_WAIT_MESSAGE_QUEUE_TIMEOUT == TCB_ptr->task_state,ASSERT_PARA_AFFIRM);
				_remove_from_delay_heap(TCB_ptr);
			}
			schedule();
		}
		CPU_CRITICAL_EXIT();
		return FUN_EXECUTE_SUCCESSFULLY;
	}
	/* else : MQB_ptr->current_message_num == MQB_ptr->max_message_num*/
	ASSERT(MQB_ptr->current_message_num == MQB_ptr->max_message_num,ASSERT_PARA_AFFIRM);
	if(MSG_FLAG_NON_BLOCKING == flag)
	{
		CPU_CRITICAL_EXIT();
		return -ERROR_MSG_FULL;
	}
	/* else : flag == FLAG_WAIT*/
	DECLEAR_INSERT_SORT_DATA(data);
	init_insert_sort_data(&data,OSTCBCurPtr);
	insert_sort_insert_into(&data,&MQB_ptr->message_array_insert_sort_put_TCB_list);
	++(MQB_ptr->put_num);
	if(0 == time)
	{
		CPU_CRITICAL_EXIT(); /*exit  critical*/
		sys_suspend(STATE_PUT_MESSAGE_QUEUE_FOREVER);
		/*go back here=========go back here*/
		CPU_CRITICAL_ENTER();
	}
	else
	{
		CPU_CRITICAL_EXIT(); /*exit  critical*/
		int num = sys_delay(time,STATE_PUT_MESSAGE_QUEUE_TIMEOUT);
		/*go back here=========go back here*/
		CPU_CRITICAL_ENTER();/*enter critical*/
		if(num <= 0)/*prove that timeout happened*/
		{
			--(MQB_ptr->put_num);
			insert_sort_delete_data(&MQB_ptr->message_array_insert_sort_put_TCB_list,OSTCBCurPtr);
			CPU_CRITICAL_EXIT(); /*exit  critical*/
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
	/*add to tail*/
	list_add_tail(&message_ptr->message_list,&MQB_ptr->message_list);
	++(MQB_ptr->current_message_num);
	ASSERT(MQB_ptr->current_message_num <= MQB_ptr->max_message_num,ASSERT_PARA_AFFIRM);
	CPU_CRITICAL_EXIT();
	return FUN_EXECUTE_SUCCESSFULLY;
}

/**
 * @Author      kaka
 * @DateTime    2019-04-21
 * @description : send a message, user should use struct message to define message,
 * also, user can choose weather to wait for sending or not
 * @param       MQB_ptr     
 * @param       message_ptr 
 * @param       flag        
 * @param       time        
 * @return                  
 */
int msg_send(MQB *MQB_ptr,struct message *message_ptr,MSG_FLAG flag,unsigned int time)
{
	if((NULL == MQB_ptr) || (NULL == message_ptr))
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(msg_send,MQB_ptr);
		OS_ERROR_PARA_MESSAGE_DISPLAY(msg_send,max_message_num);
		return -ERROR_NULL_INPUT_PTR;
	}
	return _msg_send(MQB_ptr,message_ptr,flag,time);
}
EXPORT_SYMBOL(msg_send);

int _msg_receive(MQB *MQB_ptr,struct message **message_ptr,MSG_FLAG flag,unsigned int time)
{
	ASSERT(NULL != MQB_ptr,ASSERT_INPUT);
	ASSERT(NULL != message_ptr,ASSERT_INPUT);
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	if(0 < MQB_ptr->current_message_num)
	{
		--(MQB_ptr->current_message_num);
		/*get the first message*/
		*message_ptr = list_entry(MQB_ptr->message_list.next,struct message,message_list);
		/*detatch it from list*/
		list_del(&(*message_ptr)->message_list);
		/*then check if there is a task waiting for sending*/
		if(MQB_ptr->put_num)
		{
			struct insert_sort_data *insert_sort_data_ptr;
			TCB *TCB_ptr;
			insert_sort_data_ptr = insert_sort_delete_head(&MQB_ptr->message_array_insert_sort_put_TCB_list);
			ASSERT(NULL != insert_sort_data_ptr,ASSERT_PARA_AFFIRM);
			TCB_ptr = (TCB *)(insert_sort_data_ptr->data_ptr);
			if(STATE_PUT_MESSAGE_QUEUE_FOREVER == TCB_ptr->task_state)
			{
				_remove_from_suspend_list(TCB_ptr);
			}
			else
			{
				ASSERT(STATE_PUT_MESSAGE_QUEUE_TIMEOUT == TCB_ptr->task_state,ASSERT_PARA_AFFIRM);
				_remove_from_delay_heap(TCB_ptr);
			}
			schedule();
		}
		CPU_CRITICAL_EXIT();
		return FUN_EXECUTE_SUCCESSFULLY;
	}
	/* else : MQB_ptr->current_message_num == 0*/
	ASSERT(0 == MQB_ptr->current_message_num,ASSERT_PARA_AFFIRM);
	if(MSG_FLAG_NON_BLOCKING == flag)
	{
		CPU_CRITICAL_EXIT();
		return -ERROR_MSG_FULL;
	}
	/* else : flag == FLAG_WAIT*/
	DECLEAR_INSERT_SORT_DATA(data);
	init_insert_sort_data(&data,OSTCBCurPtr);
	insert_sort_insert_into(&data,&MQB_ptr->message_array_insert_sort_wait_TCB_list);
	++(MQB_ptr->wait_num);
	if(0 == time)
	{
		CPU_CRITICAL_EXIT(); /*exit  critical*/
		sys_suspend(STATE_WAIT_MESSAGE_QUEUE_FOREVER);
		/*go back here=========go back here*/
		CPU_CRITICAL_ENTER();
	}
	else
	{
		CPU_CRITICAL_EXIT(); /*exit  critical*/
		int num = sys_delay(time,STATE_WAIT_MESSAGE_QUEUE_TIMEOUT);
		/*go back here=========go back here*/
		CPU_CRITICAL_ENTER();/*enter critical*/
		if(num <= 0)/*prove that timeout happened*/
		{
			--(MQB_ptr->wait_num);
			insert_sort_delete_data(&MQB_ptr->message_array_insert_sort_wait_TCB_list,OSTCBCurPtr);
			CPU_CRITICAL_EXIT(); /*exit  critical*/
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
	ASSERT(MQB_ptr->current_message_num > 0,ASSERT_PARA_AFFIRM);
	--(MQB_ptr->current_message_num);
	/*get the first message*/
	*message_ptr = list_entry(MQB_ptr->message_list.next,struct message,message_list);
	/*detatch it from list*/
	list_del(&(*message_ptr)->message_list);
	CPU_CRITICAL_EXIT();
	return FUN_EXECUTE_SUCCESSFULLY;
}

/**
 * @Author      kaka
 * @DateTime    2019-04-21
 * @description : get a message
 * @param       MQB_ptr     [description]
 * @param       message_ptr [description]
 * @param       flag        [description]
 * @param       time        [description]
 * @return                  [description]
 */
int msg_receive(MQB *MQB_ptr,struct message **message_ptr,MSG_FLAG flag,unsigned int time)
{
	if((NULL == MQB_ptr) || (NULL == message_ptr))
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(msg_receive,MQB_ptr);
		OS_ERROR_PARA_MESSAGE_DISPLAY(msg_receive,max_message_num);
		return -ERROR_NULL_INPUT_PTR;
	}
	if (g_interrupt_count > 0)
	{
		return -ERROR_FUN_USE_IN_INTER;
	}
	return _msg_receive(MQB_ptr,message_ptr,flag,time);
}
EXPORT_SYMBOL(msg_receive);

int _msg_del(MQB *MQB_ptr)
{
	ASSERT(NULL != MQB_ptr,ASSERT_INPUT);
	TCB *TCB_ptr;
	struct insert_sort_data *insert_sort_data_ptr = insert_sort_delete_head(&MQB_ptr->message_array_insert_sort_wait_TCB_list);
	while(NULL != insert_sort_data_ptr)
	{
		TCB_ptr = (TCB *)(insert_sort_data_ptr->data_ptr);
		if(STATE_WAIT_MESSAGE_QUEUE_FOREVER == TCB_ptr->task_state)
		{
			_remove_from_suspend_list(TCB_ptr);
		}
		else
		{
			ASSERT(STATE_WAIT_MESSAGE_QUEUE_TIMEOUT == TCB_ptr->task_state,ASSERT_PARA_AFFIRM);
			_remove_from_delay_heap(TCB_ptr);
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
			_remove_from_suspend_list(TCB_ptr);
		}
		else
		{
			ASSERT(STATE_PUT_MESSAGE_QUEUE_TIMEOUT == TCB_ptr->task_state,ASSERT_PARA_AFFIRM);
			_remove_from_delay_heap(TCB_ptr);
		}
		set_bad_state(TCB_ptr);
		insert_sort_data_ptr = insert_sort_delete_head(&MQB_ptr->message_array_insert_sort_put_TCB_list);
	}
	return FUN_EXECUTE_SUCCESSFULLY;
}

int msg_del(MQB *MQB_ptr)
{
	if(NULL == MQB_ptr)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(msg_del,MQB_ptr);
		return -ERROR_NULL_INPUT_PTR;
	}
	return _msg_del(MQB_ptr);
}
EXPORT_SYMBOL(msg_del);

/**
 * @Author      kaka
 * @DateTime    2019-04-21
 * @description : init the struct message
 * @param       message_ptr  [description]
 * @param       message_size [description]
 * @param       data         [description]
 * @return                   [description]
 */
int message_init(struct message *message_ptr,unsigned int message_size,void *data)
{
	ASSERT(NULL != message_ptr,ASSERT_INPUT);
	if(NULL == message_ptr)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(message_init,message_ptr);
		return -ERROR_NULL_INPUT_PTR;
	}
	INIT_LIST_HEAD(&message_ptr->message_list);
	message_ptr->message_size = message_size;
	message_ptr->message_data_ptr = data;
	message_ptr->tick = get_tick();
	return FUN_EXECUTE_SUCCESSFULLY;
}
EXPORT_SYMBOL(message_init);
