#include <os_delay.h>
#include <TCB.h>
#include <ka_configuration.h>
#include <heap_oo.h>
#include <myassert.h>
#include <os_ready.h>
#include <myMicroLIB.h>
#include <sys_init_fun.h>
#include <printf_debug.h>

/**
 * the delay TCB use heap to manage, the value of comparation is the reach time
 * the top TCB will be the soonest reach TCB
*/

#define TIME_FIRST_SMALLER_THAN_SECOND(first,second) ((INT64)(first)-(INT64)(second) < 0)
#define TIME_FIRST_BIGGER_THAN_SECOND(first,second)  ((INT64)(first)-(INT64)(second) > 0)

static struct little_heap delay_heap;

/*this function is used by delay_heap*/
static int _cmp(Vector *Vector_ptr,unsigned int index1,unsigned int index2)
{
	const TCB *a, *b;
	a = Vector_get_index_data(Vector_ptr, index1);
	b = Vector_get_index_data(Vector_ptr, index2);
	if (TIME_FIRST_SMALLER_THAN_SECOND(a->delay_reach_time,b->delay_reach_time))
	{
	  return -1;
	}
	else if (TIME_FIRST_BIGGER_THAN_SECOND(a->delay_reach_time,b->delay_reach_time))
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
	TCB *TCB_ptr;
	TCB_ptr = Vector_get_index_data(Vector_ptr,index);
	TCB_ptr->delay_heap_position = index;
}
	
static void __INIT __init_delay_heap(void)
{
	if(0 != heap_init(&delay_heap,PRIO_MAX/4,_cmp,index_change_record))
	{
		panic("init_delay_heap error!\nstop booting.....\n");
		/*os stop here*/
	}
}
INIT_FUN(__init_delay_heap,1);

/*before inserting action,os should set the delay_reach_time
this function do not change the task_state of the TCB,os shoule
change it before using this function*/
int _insert_into_delay_heap(TCB *TCB_ptr)
{
	ASSERT(NULL != TCB_ptr,ASSERT_INPUT);
	return heap_push(&delay_heap,TCB_ptr);
}

int _remove_from_delay_heap(TCB *TCB_ptr)
{
	ASSERT(NULL != TCB_ptr,ASSERT_INPUT);
	TCB *TCB_ptr_buffer;
	int i;
	int ret;
	int len = heap_get_cur_len(&delay_heap);
	for(i=1;i<len;++i) /* check from first to the end*/
	{
		TCB_ptr_buffer = heap_get_index_data(&delay_heap,i);
		if(TCB_ptr == TCB_ptr_buffer)
		{
			ret = heap_remove_index_data(&delay_heap,i,NULL);
			if(0 != ret)
			{
				return ret;
			}
			_insert_ready_TCB(TCB_ptr);
			return FUN_EXECUTE_SUCCESSFULLY;
		}
	}
	return -ERROR_USELESS_INPUT;
}

TCB* _delay_heap_get_top_TCB(void)
{
	TCB *TCB_ptr;
	if(0 != heap_get_top_safe(&delay_heap,(void *)&TCB_ptr))
	{
		return NULL;
	}
	return TCB_ptr;
}

TCB* _delay_heap_remove_top_TCB(void)
{
	TCB *TCB_ptr;
	if(0 != heap_remove_top(&delay_heap,&TCB_ptr))
	{
		return NULL;
	}
	return TCB_ptr;
}

#if CONFIG_SHELL_EN
void shell_delay_heap_check(void)
{
	TCB *TCB_ptr1,*TCB_ptr2;
	unsigned int delay_TCB_num;
	unsigned int i;
	delay_TCB_num = heap_get_cur_len(&delay_heap) - 1;
	for(i=1;i<=delay_TCB_num;--i)
	{
		if(2 * i > delay_TCB_num)
		{
			break;
		}
		TCB_ptr1 = heap_get_index_data(&delay_heap,2*i);
		TCB_ptr2 = heap_get_index_data(&delay_heap,i);
		if(TIME_FIRST_SMALLER_THAN_SECOND(TCB_ptr1->delay_reach_time,TCB_ptr2->delay_reach_time))
		{
			ka_printf("heap member %u has incorrect position with member %u\n",i,i/2);
		}
	}
}
#endif

