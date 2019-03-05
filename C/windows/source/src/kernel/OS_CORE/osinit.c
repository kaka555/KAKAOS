#include <ka_configuration.h>
#include <osinit.h>
#include <os_delay.h>
#include <os_ready.h>
#include <os_suspend.h>
#include <os_TCB_list.h>
#include <os_schedule.h>
#include <TCB.h>
#include <os_cpu_stm32.h>
#include <kakaosstdint.h>
#include <myMicroLIB.h>
#include <shell.h>
#include <user.h>
#include <myassert.h>
#include <slab.h>
#include <double_linked_list.h>
#include <module.h>


#if CONFIG_SHELL_EN
	static TCB TCB_shell;
#endif
static TCB TCB_init;
static TCB TCB_idle;
TCB TCB_count_init;

#if CONFIG_TIMER_EN
	TCB TCB_timer_task;
#endif

UINT64 idle_num = 0;
unsigned int  count_max_num = 0;//used for counting cpu used rate
#if PRECISE_TIME_DELAY
	UINT64 num_pre_tick = 0;//used for precise timing
	TCB TCB_precise_timing;
#endif

extern int g_schedule_lock;
extern UINT64 g_time_tick_count;
extern volatile int g_interrupt_count;
extern TCB *OSTCBCurPtr;
extern TCB *OSTCBHighRdyPtr;
extern struct list_head *const cache_chain_head_ptr;

#if CONFIG_TIMER_EN
	extern void __init_timer(void);
	extern void timer_task(void *para);
#endif

static void thread_init(void *para);

static void __INIT os_init(void)
{
	bsp_init();

	g_time_tick_count = 0;
	g_interrupt_count = 0;

//===========my_lib==========================
	__init_my_micro_lib();

//===========TCB=======schedule==============	
	__init_ready_group();
	__init_delay_heap();
	__init_TCB_list();
	__init_suspend_list();

#if CONFIG_TIME_EN
	__init_system_time();
#endif	

#if CONFIG_TIMER_EN
	__init_timer();
#endif

#if CONFIG_SHELL_DEBUG_EN && CONFIG_SHELL_EN
	__init_shell_debug();
#endif

#if CONFIG_MODULE
	__init_module();
#endif
}

void task_start(void)
{
	if(0 != task_init_ready(&TCB_init,500,0,3,"init",thread_init,NULL))
	{
		ka_printf("os_init_fail...stop booting...\n");
		while(1);
	}
#if CONFIG_SHELL_EN
	if(0 != task_init_ready(&TCB_shell,1000,0,3,"shell",shell,NULL))
	{
		ka_printf("os_init_fail...stop booting...\n");
		while(1);
	}
	#endif
#if CONFIG_TIMER_EN
	if(0 != task_init_ready(&TCB_timer_task,256,0,3,"timer_task",timer_task,NULL))
	{
		ka_printf("os_init_fail...stop booting...\n");
		while(1);
	}
#endif
	if(0 != task_creat_ready(500,4,5,"three",three,NULL,NULL))
	{
		ka_printf("os_init_fail...stop booting...\n");
		while(1);
	}
//	if(0 != task_creat_ready(256,5,5,"four",four,NULL,NULL))
//	{
//		ka_printf("os_init_fail...stop booting...\n");
//		while(1);
//	}
//	if(0 != task_creat_ready(256,6,5,"five",five,NULL,NULL))
//	{
//		ka_printf("os_init_fail...stop booting...\n");
//		while(1);
//	}
//	if(0 != task_creat_ready(256,7,3,"six",six,NULL,NULL))
//	{
//		ka_printf("os_init_fail...stop booting...\n");
//		while(1);
//	}
}

static void count_init(void *para)	
{
	(void)para;
#if PRECISE_TIME_DELAY
	sleep(2); // for synchronization
	num_pre_tick = 0;
	sleep(PRECISE_DELAY_NUM);
	num_pre_tick /= PRECISE_DELAY_NUM;
#if CONFIG_DEBUG_COUNT_INIT
	ka_printf("succeed to get num_pre_tick = %l \n",num_pre_tick);
#endif
	task_delete(&TCB_precise_timing);
#endif
	
	sleep(2); // for synchronization
	idle_num = 0;
	sleep(COUNT_DELAY_NUM);
	count_max_num = idle_num / COUNT_DELAY_NUM; // COUNT_DELAY_NUM;
	idle_num = 0;
	
#if CONFIG_DEBUG_COUNT_INIT
	ka_printf("succeed to get count_max_num = %u\n",count_max_num);
#endif

	SYS_ENTER_CRITICAL();
	task_start();
	SYS_EXIT_CRITICAL();
	schedule();
}

static void  idle(void *para)
{
	CPU_SR_ALLOC();
	(void)para;
	while(1)
	{
		CPU_CRITICAL_ENTER();
		++idle_num;
		CPU_CRITICAL_EXIT();
	}
}

#if PRECISE_TIME_DELAY
static void precise_timing(void *para)
{
	(void)para;
	while(1)
	{
		++num_pre_tick;
	}
}
#endif

static void set_inter_stack(void)
{
	unsigned int MSP_top = (unsigned int)alloc_power1_page(); // allocate one page for interrupt stack;
	MSP_top += PAGE_SIZE_BYTE;
	asm("MOVS R0, %0\t\n"
		"MSR MSP, R0\t\n"
		:
		:"r"(MSP_top)
		);
}

void __INIT os_start(void)
{
//===========init all module==================
	os_init();
//============================================

//==============register initialization task===================
	if(0 != task_init_ready(&TCB_count_init,256,PRIO_MAX-2,5,"count_init",count_init,NULL))
	{
		ka_printf("os_init_fail...stop booting...\n");
		while(1);
	}
	if(0 != task_init_ready(&TCB_idle,128,PRIO_MAX-1,HZ,"idle",idle,NULL))
	{
		ka_printf("os_init_fail...stop booting...\n");
		while(1);
	}
#if PRECISE_TIME_DELAY
	if(0 != task_init_ready(&TCB_precise_timing,128,PRIO_MAX-2,HZ,"precise timing",precise_timing,NULL))
	{
		ka_printf("os_init_fail...stop booting...\n");
		while(1);
	}
#endif
//==================================================================

	OSTCBCurPtr = get_highest_prio_ready_TCB();
	OSTCBHighRdyPtr = OSTCBCurPtr;

//===========system_tick====================
	__init_systick();
//==========================================
	set_inter_stack();
	OSStartHighRdy();
	ASSERT(0);
	while(1);//should no go here
}

static int __attribute__((optimize("O1"))) FastLog2(int x)
{
    float fx;
    unsigned long ix, exp;
 
    fx = (float)x;
    ix = *(unsigned long*)&fx;
    exp = (ix >> 23) & 0xFF;
 
    return exp - 127;
}

extern int in_buddy_range(void *ptr);
extern int in_os_memory(void *ptr);
void _case_slab_free_buddy(void *ptr,void *end_ptr)
{
	if(NULL == ptr)
	{
		return ;
	}
	ASSERT(0 == in_os_memory(ptr));
#if CONFIG_PARA_CHECK
	if(0 != in_os_memory(ptr))
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(_case_free_buddy,ptr);
		return ;
	}
#endif
	unsigned int level = ((unsigned int)end_ptr - (unsigned int)ptr) / PAGE_SIZE_BYTE;
	ASSERT(0 == (level & (level-1)));
	level = FastLog2(level) + 1;
	struct buddy *buddy_ptr = (struct buddy *)get_os_buddy_ptr_head();
	ASSERT(NULL != buddy_ptr);
	while(NULL != buddy_ptr)
	{
		if(0 == in_buddy_range(ptr))
		{
			switch(level)
			{
				case 1 :
					return_power1_page(ptr);
					break;
				case 2 :
					return_power2_page(ptr);
					break;
				case 3 :
					return_power3_page(ptr);
					break;
				case 4 :
					return_power4_page(ptr);
					break;
				case 5 :
					return_power5_page(ptr);
					break;
				case 6 :
					return_power6_page(ptr);
					break;
				case 7 :
					return_power7_page(ptr);
					break;
				default :
					ka_printf("error level! FATAL ERROR!\n");
					ASSERT(0);
			}
			return ;
		}
		else
		{
			buddy_ptr = (struct buddy *)get_next_buddy_ptr_head(buddy_ptr);
			continue;
		}
	}
	ka_printf("ptr fatal error!!!\n");
	return ;
}

static void thread_init(void *para)	
{
	(void)para;
	struct list_head *pos;
	IL *IL_ptr;
	struct kmem_cache *kmem_cache_ptr;
	while(1)
	{
		sleep(THREAD_INIT_PERIOD * HZ); //period
		//free the memory 
		list_for_each(pos,cache_chain_head_ptr)
		{
			IL_ptr = list_entry(pos,IL,insert);
			kmem_cache_ptr = list_entry(IL_ptr,struct kmem_cache,kmem_cache_insert_chain);
			if(!list_empty(&kmem_cache_ptr->slabs_empty))
			{
				struct list_head *pos1,*pos2;
				list_for_each_safe(pos1,pos2,&kmem_cache_ptr->slabs_empty)
				{
					struct slab *slab_ptr = list_entry(pos1,struct slab,slab_chain);
					ASSERT(slab_ptr->current_block_num == slab_ptr->full_block_num);
					ASSERT(slab_ptr->block_size == kmem_cache_ptr->kmem_cache_insert_chain.prio);
					list_del(pos1);
					_case_slab_free_buddy(slab_ptr,slab_ptr->end_ptr);
				}
			}
		}
	}
}

void recycle_memory(void)
{
	remove_from_delay_heap(&TCB_init);
}
