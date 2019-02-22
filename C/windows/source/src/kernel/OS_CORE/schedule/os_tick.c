#include <osinit.h>
#include <ka_configuration.h>
#include <kakaosstdint.h>
#include <TCB.h>
#include <os_delay.h>
#include <os_ready.h>
#include <os_TCB_list.h>
#include <myassert.h>
#include <double_linked_list.h>
#include <os_time.h>
#include <os_schedule.h>
#include <os_timer.h>
#include <os_suspend.h>

#if CONFIG_CPU_USE_RATE_CALCULATION
	static unsigned int use_rate = 0;
	extern UINT64 idle_num;
	extern UINT64 count_max_num;
#endif

extern volatile TCB *OSTCBCurPtr;
extern volatile TCB *OSTCBHighRdyPtr;

#if CONFIG_TIMER_EN
	extern TCB TCB_timer_task;
#endif


extern volatile UINT64 g_time_tick_count;	

/**
 * This is a system function
 * @Author   kaka
 * @param    none
 * @DateTime 2018-09-24
 * @description : change the infomation about time
 */
inline static void tick_time_handler(void)
{
#if CONFIG_TIME_EN
	static volatile int time_flag = 0;
	time_flag++;
#endif
	++g_time_tick_count;
#if CONFIG_TIME_EN
	if(TICK_PER_SEC == time_flag)
	{
		time_flag = 0;
		system_time_increase();
#if CONFIG_CPU_USE_RATE_CALCULATION
		use_rate = idle_num / (count_max_num/100);
		idle_num = 0;
#endif		
	}
#endif
}

#if CONFIG_CPU_USE_RATE_CALCULATION
unsigned int get_cpu_use_rate(void)
{
	return use_rate;
}
#endif	

/**
 * This is a system function
 * @Author   kaka
 * @param    miaosu
 * @DateTime 2018-09-24
 * @description : check if there is a task reach the delay time
 */
static void delay_task_check(void)
{
	TCB *TCB_ptr = delay_heap_get_top_TCB();
	while(NULL != TCB_ptr)
	{
		if(TCB_ptr->delay_reach_time == g_time_tick_count)
		{
			delay_heap_remove_top_TCB();
			insert_ready_TCB(TCB_ptr);
			TCB_ptr = delay_heap_get_top_TCB();
			set_rescheduled_flag();
			continue ;
		}
		else
		{
			break ;
		}
	}
}

#if CONFIG_TIMER_EN
void timer_task_check(void)
{
	struct timer *timer_ptr;
	if(get_timer_heap_top(&timer_ptr) < 0)
	{
		return ;
	}
	if(TIME_FIRST_SMALLER_THAN_SECOND(get_tick(),timer_ptr->wake_time))
	{
		return;
	}
	else
	{
		remove_from_suspend_list(&TCB_timer_task);
		set_rescheduled_flag();
	}
}
#endif

/**
 * This is a system function
 * @Author      kaka
 * @param       void
 * @DateTime    2018-09-24
 * @description : used for counting time slice
 */
static void run_task_handler(void)
{
	TCB *TCB_ptr = (TCB *)OSTCBCurPtr;
	if(--(TCB_ptr->timeslice_rest_time))
	{
		return ;
	}
	TCB_ptr->timeslice_rest_time = TCB_ptr->timeslice_hope_time;
	if(1 == get_ready_num_from_TCB_list(TCB_ptr->prio))
	{
		return ;
	}
	else
	{
		struct list_head *head;
		head = get_from_TCB_list(TCB_ptr->prio);
		list_del(&TCB_ptr->same_prio_list);
		list_add_tail(&TCB_ptr->same_prio_list,head);
		set_rescheduled_flag();
	}
}

/**
 * This is a system function
 * @Author   kaka
 * @param    none
 * @DateTime 2018-09-23
 * @return   [none]
 */
void  OS_CPU_SysTickHandler(void)
{
	SYS_ENTER_INTERRUPT();
	tick_time_handler();
	delay_task_check();
#if CONFIG_TIMER_EN
	timer_task_check();
#endif
	run_task_handler();
	if(need_rescheduled() && !sys_schedule_islock())
	{
		clear_rescheduled_flag();
		schedule();
	}
	SYS_EXIT_INTERRUPT();
}
