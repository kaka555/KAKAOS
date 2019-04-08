#include <os_timer.h>
#include <os_error.h>
#include <myassert.h>
#include <myMicroLIB.h>
#include <heap_oo.h>
#include <os_schedule.h>
#include <os_time.h>
#include <os_cpu.h>
#include <export.h>
#include <sys_init_fun.h>

#if CONFIG_TIMER_EN

static struct little_heap timer_heap;

static int cmp(Vector *Vector_ptr,unsigned int index1,unsigned int index2)
{
	const struct timer *a, *b;
	a = Vector_get_index_address (Vector_ptr, index1);
	b = Vector_get_index_address (Vector_ptr, index2);
	if (TIME_FIRST_SMALLER_THAN_SECOND(a->wake_time,b->wake_time))
	{
	  return -1;
	}
	else if (TIME_FIRST_BIGGER_THAN_SECOND(a->wake_time,b->wake_time))
	{
	  return 1;
	}
	else
	{
	  return 0;
	}
}

static inline void index_change_record(Vector *Vector_ptr,int index)
{
	struct timer *timer_ptr;
	timer_ptr = Vector_get_index_data(Vector_ptr,index);
	timer_ptr->heap_position_index = index;
}

/**
 * This is a system function
 * @Author      kaka
 * @DateTime    2018-10-03
 * @description : used while os's initialization
 */
static void __INIT __init_timer(void)
{
	if(0 != heap_init(&timer_heap,16,cmp,index_change_record))
	{
		ka_printf("init_delay_heap error!\nstop booting.....\n");
		while(1);
		/* os stop here */
	}
}
INIT_FUN(__init_timer,1);

/**
 * This is a system function
 * @Author      kaka
 * @DateTime    2018-10-03
 * @description : initialize the timer
 */
int timer_init(
	struct timer *timer_ptr,
	TIMER_TYPE type,
	char *name,
	void (*fun)(void *para),
	void *para,
	unsigned int period,
	unsigned int num)
{
	ASSERT((NULL != timer_ptr) && (NULL != name) && (NULL != fun));
	if((NULL == timer_ptr) || (NULL == name) || (NULL == fun))
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(timer_init,timer_ptr);
		OS_ERROR_PARA_MESSAGE_DISPLAY(timer_init,name);
		OS_ERROR_PARA_MESSAGE_DISPLAY(timer_init,fun);
		return -ERROR_NULL_INPUT_PTR;
	}
	if (g_interrupt_count > 0)
	{
		return -ERROR_FUN_USE_IN_INTER;
	}
	timer_ptr->type  			= type;
	timer_ptr->state 			= TIMER_DISABLE;
	timer_ptr->timer_name 		= name;
	timer_ptr->fun 				= fun;
	timer_ptr->para 			= para;
	timer_ptr->period 			= period;
	timer_ptr->rest_num 		= INIT_REST_NUM(num);
	return FUN_EXECUTE_SUCCESSFULLY;
}
EXPORT_SYMBOL(timer_init);

/**
 * This is a system function
 * @Author      kaka
 * @DateTime    2018-10-03
 * @description : create and initialize the timer
 */
int timer_create(
	struct timer **timer_ptr,
	TIMER_TYPE type,
	char *name,
	void (*fun)(void *para),
	void *para,
	unsigned int period,
	unsigned int num)
{
	struct timer *ptr;
	ptr = ka_malloc(sizeof(struct timer));
	if(NULL == ptr)
	{
		return -ERROR_NO_MEM;
	}
	if(NULL != timer_ptr)
	{
		*timer_ptr = ptr;
	}
	timer_init(ptr,type,name,fun,para,period,num);
	ptr->rest_num = CREAT_REST_NUM(num);
	return FUN_EXECUTE_SUCCESSFULLY;
}
EXPORT_SYMBOL(timer_create);

/**
 * This is a system function
 * @Author      kaka
 * @DateTime    2018-10-03
 * @description : after using timer_create(), use this function to 
 * activate the timer
 * @param       timer_ptr  
 * @return      0 means success
 */
int _timer_enable(struct timer *timer_ptr)
{
	ASSERT(NULL != timer_ptr);
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	timer_ptr->state = TIMER_ENABLE;
	timer_ptr->wake_time = timer_ptr->period + _get_tick();
	int ret = heap_push(&timer_heap,timer_ptr);
	CPU_CRITICAL_EXIT();
	return ret;
}

int timer_enable(struct timer *timer_ptr)
{
	if(NULL == timer_ptr)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(timer_enable,timer_ptr);
		return -ERROR_NULL_INPUT_PTR;
	}
	return _timer_enable(timer_ptr);
}
EXPORT_SYMBOL(timer_enable);

/**
 * This is a system function
 * @Author      kaka
 * @DateTime    2018-10-03
 * @description :	use this function to inactivate the timer
 * @param       timer_ptr  
 * @return      0 means success
 */
int _timer_disable(struct timer *timer_ptr)
{
	ASSERT(NULL != timer_ptr);
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	int ret = heap_remove_index_data(&timer_heap,timer_ptr->heap_position_index,NULL);
	if(ret < 0)
	{
		CPU_CRITICAL_EXIT();
		return ret;
	}
	timer_ptr->state = TIMER_DISABLE;
	CPU_CRITICAL_EXIT();
	return FUN_EXECUTE_SUCCESSFULLY;
}

int timer_disable(struct timer *timer_ptr)
{
	if(NULL == timer_ptr)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(timer_disable,timer_ptr);
		return -ERROR_NULL_INPUT_PTR;
	}
	return _timer_disable(timer_ptr);
}
EXPORT_SYMBOL(timer_disable);

/**
 * This is a system function
 * @Author      kaka
 * @DateTime    2018-10-03
 * @description :	use this function to inactivate the timer and free
 * the memory if necessary
 * @param       timer_ptr  
 * @return                 
 */
int _timer_delete(struct timer *timer_ptr)
{
	ASSERT(NULL != timer_ptr);
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	if(TIMER_ENABLE == timer_ptr->state)
	{
		_timer_disable(timer_ptr);
	}
	if(TIMER_IS_CREAT(timer_ptr))
	{
		ka_free(timer_ptr);
	}
	CPU_CRITICAL_EXIT();
	return FUN_EXECUTE_SUCCESSFULLY;
}

int timer_delete(struct timer *timer_ptr)
{
	if(NULL == timer_ptr)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(timer_delete,timer_ptr);
		return -ERROR_NULL_INPUT_PTR;
	}
	return _timer_delete(timer_ptr);
}
EXPORT_SYMBOL(timer_delete);

/**
 * This is a system function
 * @Author      kaka
 * @DateTime    2018-10-03
 * @description :	this function is used by system tick function
 * to check if waking up the timer task is needed or not
 * @param       timer_ptr  [description]
 * @return                 [description]
 */
int _get_timer_heap_top(struct timer **timer_ptr)
{
	ASSERT(NULL != timer_ptr);
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	int ret = heap_get_top_safe(&timer_heap,(void **)timer_ptr);
	CPU_CRITICAL_EXIT();
	return ret;
}

/**
 * This is a system function
 * @Author      kaka
 * @DateTime    2018-10-03
 * @description : timer task, all the works are being done in
 * this task, whose priority is 0(the max priority)
 * @param       para       [description]
 */
void timer_task(void *para)
{
	(void)para;
	UINT64 tick;
	struct timer *timer_ptr;
	int ret = heap_get_top_safe(&timer_heap,(void **)&timer_ptr);
	while(1)
	{
		if(ret < 0)
		{
			suspend();
		}
		else
		{
			tick = get_tick();
			if(TIME_FIRST_SMALLER_THAN_SECOND(tick,timer_ptr->wake_time))
			{
				suspend();
			}
			else
			{
				heap_remove_top(&timer_heap,&timer_ptr);
				(timer_ptr->fun)(timer_ptr->para);
				switch(timer_ptr->type)
				{
					case TIMER_ONE_TIME:
						if(TIMER_IS_CREAT(timer_ptr))
						{
							ka_free(timer_ptr);
						}
						break;
					case TIMER_TIME:
						--(timer_ptr->rest_num);
						if(0 == GET_REST_NUM(timer_ptr->rest_num))
						{
							if(TIMER_IS_CREAT(timer_ptr))
							{
								ka_free(timer_ptr);
							}
							break;
						}
					case TIMER_PERIODIC:
						timer_ptr->wake_time += timer_ptr->period;
						heap_push(&timer_heap,timer_ptr);
						break;
					default:
						ASSERT(0);
						break;
				}
			}
		}
		ret = heap_get_top_safe(&timer_heap,(void **)&timer_ptr);
	}
}

#endif
