#include <os_schedule.h>
#include <os_cpu.h>
#include <os_delay.h>
#include <TCB.h>
#include <myassert.h>
#include <os_ready.h>
#include <os_suspend.h>
#include <os_TCB_list.h>
#include <osinit.h>
#include <myMicroLIB.h>
#include <os_time.h>
#include <export.h>

extern volatile TCB *OSTCBCurPtr;
extern volatile TCB *OSTCBHighRdyPtr;
extern volatile int g_interrupt_count;
extern UINT64 g_time_tick_count;

volatile int g_schedule_lock = 0;/* 0:do not lock;  !0: lock*/

static void _schedule(void)
{
	if(g_schedule_lock)
		return ;
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	OSTCBHighRdyPtr = _get_highest_prio_ready_TCB();
	if(OSTCBHighRdyPtr != OSTCBCurPtr)
	{
		OSIntCtxSw();
		CPU_CRITICAL_EXIT();
		return ;
	}
	CPU_CRITICAL_EXIT();
	return ;
}

void schedule(void)
{
	_schedule();
}
EXPORT_SYMBOL(schedule);

int _sys_delay(unsigned int delay_ticks_num,TASK_STATE state)
{
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	if (g_interrupt_count > 0)
	{
		CPU_CRITICAL_EXIT();
		return -ERROR_FUN_USE_IN_INTER;
	}
	OSTCBCurPtr->delay_reach_time = delay_ticks_num + g_time_tick_count;
	OSTCBCurPtr->task_state = state;
	_delete_TCB_from_ready((TCB *)OSTCBCurPtr);
	_insert_into_delay_heap((TCB *)OSTCBCurPtr);
	schedule();
	CPU_CRITICAL_EXIT();
	return (unsigned int)(OSTCBCurPtr->delay_reach_time - _get_tick());
}

int sys_delay(unsigned int delay_ticks_num,TASK_STATE state)
{
	ASSERT((STATE_DELAY == state) 						|| 
		   (STATE_WAIT_MCB_TIMEOUT == state) 			|| 
		   (STATE_WAIT_MESSAGE_QUEUE_TIMEOUT == state) 	|| 
		   (STATE_PUT_MESSAGE_QUEUE_TIMEOUT == state));
	ASSERT(0 != delay_ticks_num);
	if(!((STATE_DELAY == state) || (STATE_WAIT_MCB_TIMEOUT == state) || 
		(STATE_WAIT_MESSAGE_QUEUE_TIMEOUT == state) || (STATE_PUT_MESSAGE_QUEUE_TIMEOUT == state)))
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(sys_delay,state);
		return -ERROR_USELESS_INPUT;
	}
	if(0 == delay_ticks_num)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(sys_delay,delay_ticks_num);
		return -ERROR_USELESS_INPUT;
	}
	return _sys_delay(delay_ticks_num,state);
}
EXPORT_SYMBOL(sys_delay);

int _sys_suspend(TASK_STATE state)
{
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	if (g_interrupt_count > 0)
	{
		CPU_CRITICAL_EXIT();
		return -ERROR_FUN_USE_IN_INTER;
	}
	OSTCBCurPtr->task_state = state;
	_delete_TCB_from_ready((TCB *)OSTCBCurPtr);
	_insert_into_suspend_list((TCB *)OSTCBCurPtr);
	schedule();
	CPU_CRITICAL_EXIT();
	return FUN_EXECUTE_SUCCESSFULLY;
}

int sys_suspend(TASK_STATE state)
{
	ASSERT((STATE_SUSPEND_NORMAL == state) 				||
		   (STATE_WAIT_MCB_FOREVER == state) 			||
		   (STATE_WAIT_MESSAGE_QUEUE_FOREVER == state) 	||
		   (STATE_PUT_MESSAGE_QUEUE_FOREVER == state)	||
		   (STATE_WAIT_MUTEX_FOREVER == state));
	if(!((STATE_SUSPEND_NORMAL == state) 				||
		   (STATE_WAIT_MCB_FOREVER == state) 			||
		   (STATE_WAIT_MESSAGE_QUEUE_FOREVER == state) 	||
		   (STATE_PUT_MESSAGE_QUEUE_FOREVER == state)	||
		   (STATE_WAIT_MUTEX_FOREVER == state)))
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(sys_delay,state);
		return -ERROR_USELESS_INPUT;
	} 
	return _sys_suspend(state);
}
EXPORT_SYMBOL(sys_suspend);

static _must_check int _task_creat_ready(
	unsigned int stack_size,
	TASK_PRIO_TYPE prio,
	unsigned int timeslice_hope_time,
	const char *name,
	functionptr function,
	void *para,
	TCB **ptr)
{
	int ret;
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	TCB *TCB_ptr = _task_creat(stack_size,prio,timeslice_hope_time,name,function,para,STATE_READY);
	if(NULL == TCB_ptr)
	{
		return -ERROR_SYS;
	}
	if(NULL != ptr)
	{
		*ptr = TCB_ptr;
	}
	ret = _insert_ready_TCB(TCB_ptr);
	if(0 != ret)
	{
		return ret;
	}
	schedule();
	CPU_CRITICAL_EXIT();
	return FUN_EXECUTE_SUCCESSFULLY;
}

int _must_check task_creat_ready(
	unsigned int stack_size,
	TASK_PRIO_TYPE prio,
	unsigned int timeslice_hope_time,
	const char *name,
	functionptr function,
	void *para,
	TCB **ptr)
{
	if (g_interrupt_count > 0)
	{
		return -ERROR_FUN_USE_IN_INTER;
	}
	if(prio >= PRIO_MAX)
	{
		return -ERROR_LOGIC;
	}
	if((NULL == name) || (NULL == function))
	{
		return -ERROR_NULL_INPUT_PTR;
	}
	return _task_creat_ready(
				stack_size,
				prio,
				timeslice_hope_time,
				name,
				function,
				para,
				ptr);
}
EXPORT_SYMBOL(task_creat_ready);

int _must_check _task_init_ready(
	TCB *TCB_ptr,
	unsigned int stack_size,
	TASK_PRIO_TYPE prio,
	unsigned int timeslice_hope_time,
	const char *name,
	functionptr function,
	void *para)
{
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	int ret;
	if(0 != _task_init(TCB_ptr,stack_size,prio,timeslice_hope_time,name,function,para,STATE_READY))
	{
		CPU_CRITICAL_EXIT();
		return -ERROR_SYS;
	}
	ret = _insert_ready_TCB(TCB_ptr);
	if(0 != ret)
	{
		CPU_CRITICAL_EXIT();
		return ret;
	}
	schedule();
	CPU_CRITICAL_EXIT();
	return FUN_EXECUTE_SUCCESSFULLY;
}

int _must_check task_init_ready(
	TCB *TCB_ptr,
	unsigned int stack_size,
	TASK_PRIO_TYPE prio,
	unsigned int timeslice_hope_time,
	const char *name,
	functionptr function,
	void *para)
{	
	if (g_interrupt_count > 0)
	{
		return -ERROR_FUN_USE_IN_INTER;
	}
	if(prio >= PRIO_MAX)
	{
		return -ERROR_LOGIC;
	}
	if((NULL == TCB_ptr) || (NULL == name) || (NULL == function))
	{
		return -ERROR_NULL_INPUT_PTR;
	}
	return _task_init_ready(
				TCB_ptr,
				stack_size,
				prio,
				timeslice_hope_time,
				name,
				function,
				para);
}
EXPORT_SYMBOL(task_init_ready);

void sys_schedule_lock(void)
{
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	++g_schedule_lock;
	CPU_CRITICAL_EXIT();
}

void sys_schedule_unlock(void)
{
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	--g_schedule_lock;
	CPU_CRITICAL_EXIT();
}

int sys_schedule_islock(void)
{
	return (0 != g_schedule_lock);
}
