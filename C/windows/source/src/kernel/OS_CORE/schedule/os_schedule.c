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

extern volatile TCB *OSTCBCurPtr;
extern volatile TCB *OSTCBHighRdyPtr;
extern volatile int g_interrupt_count;
extern UINT64 g_time_tick_count;

#if PRECISE_TIME_DELAY
	extern UINT64 num_pre_tick;
#endif

volatile int g_schedule_lock = 0;// 0:do not lock;  !0: lock

void schedule(void)
{
	if(g_schedule_lock)
		return ;
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	OSTCBHighRdyPtr = get_highest_prio_ready_TCB();
	if(OSTCBHighRdyPtr != OSTCBCurPtr)
	{
		OSIntCtxSw();
		CPU_CRITICAL_EXIT();
		return ;
	}
	CPU_CRITICAL_EXIT();
	return ;
}

#if PRECISE_TIME_DELAY
void delay_ms(unsigned int ms)
{
#define MS_PER_TICK (1000u/TICK_PER_SEC)
	UINT64 num = 0.2 * (ms * num_pre_tick / MS_PER_TICK);
	while(--num);
}

void delay_us(unsigned int us)
{
#define US_PER_TICK (1000000u/TICK_PER_SEC)
	UINT64 num = 0.2 * (us * num_pre_tick / US_PER_TICK);
	while(--num);
}
#endif

int sys_delay(unsigned int delay_ticks_num,TASK_STATE state)
{
	ASSERT((STATE_DELAY == state) 						|| 
		   (STATE_WAIT_MCB_TIMEOUT == state) 			|| 
		   (STATE_WAIT_MESSAGE_QUEUE_TIMEOUT == state) 	|| 
		   (STATE_PUT_MESSAGE_QUEUE_TIMEOUT == state));
	ASSERT(0 != delay_ticks_num);
#if CONFIG_PARA_CHECK
	if(!((STATE_DELAY == state) || (STATE_WAIT_MCB_TIMEOUT == state) || 
		(STATE_WAIT_MESSAGE_QUEUE_TIMEOUT == state) || (STATE_PUT_MESSAGE_QUEUE_TIMEOUT == state)))
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(sys_delay,state);
		return -ERROR_VALUELESS_INPUT;
	}
	if(0 == delay_ticks_num)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(sys_delay,delay_ticks_num);
		return -ERROR_VALUELESS_INPUT;
	}
#endif	
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	if (g_interrupt_count > 0)
	{
		CPU_CRITICAL_EXIT();
		return -ERROR_FUN_USE_IN_INTER;
	}
	OSTCBCurPtr->delay_reach_time = delay_ticks_num + g_time_tick_count;
	OSTCBCurPtr->task_state = state;
	delete_TCB_from_ready((TCB *)OSTCBCurPtr);
	insert_into_delay_heap((TCB *)OSTCBCurPtr);
	schedule();
	CPU_CRITICAL_EXIT();
	return (unsigned int)(OSTCBCurPtr->delay_reach_time - get_tick());
}

int sys_suspend(TASK_STATE state)
{
	ASSERT((STATE_SUSPEND_NORMAL == state) 				||
		   (STATE_WAIT_MCB_FOREVER == state) 			||
		   (STATE_WAIT_MESSAGE_QUEUE_FOREVER == state) 	||
		   (STATE_PUT_MESSAGE_QUEUE_FOREVER == state)	||
		   (STATE_WAIT_MUTEX_FOREVER == state));
#if CONFIG_PARA_CHECK
	if(!((STATE_SUSPEND_NORMAL == state) 				||
		   (STATE_WAIT_MCB_FOREVER == state) 			||
		   (STATE_WAIT_MESSAGE_QUEUE_FOREVER == state) 	||
		   (STATE_PUT_MESSAGE_QUEUE_FOREVER == state)	||
		   (STATE_WAIT_MUTEX_FOREVER == state)))
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(sys_delay,state);
		return -ERROR_VALUELESS_INPUT;
	} 
#endif
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	if (g_interrupt_count > 0)
	{
		CPU_CRITICAL_EXIT();
		return -ERROR_FUN_USE_IN_INTER;
	}
	OSTCBCurPtr->task_state = state;
	delete_TCB_from_ready((TCB *)OSTCBCurPtr);
	insert_into_suspend_list((TCB *)OSTCBCurPtr);
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
	int ret;
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	if (g_interrupt_count > 0)
	{
		CPU_CRITICAL_EXIT();
		return -ERROR_FUN_USE_IN_INTER;
	}
	TCB *TCB_ptr = task_creat(stack_size,prio,timeslice_hope_time,name,function,para,STATE_READY);
	if(NULL == TCB_ptr)
	{
		return -ERROR_CREATE_TASK;
	}
	if(NULL != ptr)
	{
		*ptr = TCB_ptr;
	}
	ret = insert_ready_TCB(TCB_ptr);
	if(0 != ret)
	{
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
	char *name,
	functionptr function,
	void *para)
{	
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	if (g_interrupt_count > 0)
	{
		CPU_CRITICAL_EXIT();
		return -ERROR_FUN_USE_IN_INTER;
	}
	int ret;
	if(0 != task_init(TCB_ptr,stack_size,prio,timeslice_hope_time,name,function,para,STATE_READY))
	{
		CPU_CRITICAL_EXIT();
		return -ERROR_TASK_CREATE;
	}
	ret = insert_ready_TCB(TCB_ptr);
	if(0 != ret)
	{
		CPU_CRITICAL_EXIT();
		return ret;
	}
	schedule();
	CPU_CRITICAL_EXIT();
	return FUN_EXECUTE_SUCCESSFULLY;
}

inline void sys_schedule_lock(void)
{
	++g_schedule_lock;
}

inline void sys_schedule_unlock(void)
{
	--g_schedule_lock;
}

inline int sys_schedule_islock(void)
{
	return (0 != g_schedule_lock);
}
