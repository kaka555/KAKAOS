#include <ka_configuration.h>
#include <osinit.h>
#include <os_delay.h>
#include <os_ready.h>
#include <os_suspend.h>
#include <os_TCB_list.h>
#include <os_schedule.h>
#include <TCB.h>
#include <os_cpu.h>
#include <kakaosstdint.h>
#include <myMicroLIB.h>
#include <shell.h>
#include <user.h>
#include <myassert.h>
#include <slab.h>
#include <double_linked_list.h>
#include <module.h>
#include <sys_init_fun.h>
#include <printf_debug.h>

#if CONFIG_SHELL_EN
	static TCB TCB_shell;
#endif
static TCB TCB_init;
static TCB TCB_idle;
static TCB TCB_count_init;

#if CONFIG_TIMER_EN
	TCB TCB_timer_task;
#endif

UINT64 idle_num = 0;
unsigned int  count_max_num = 0;/*used for counting cpu used rate*/

extern int g_schedule_lock;
extern UINT64 g_time_tick_count;
extern volatile int g_interrupt_count;
extern TCB *OSTCBCurPtr;
extern TCB *OSTCBHighRdyPtr;
extern struct singly_list_head *const cache_chain_head_ptr;

#if CONFIG_TIMER_EN
	extern void timer_task(void *para);
#endif

static void thread_init(void *para);
static void count_init(void *para);
static void idle(void *para);
static void set_inter_stack(void);

extern unsigned long _ka_init_fun_begin1;
extern unsigned long _ka_init_fun_end1;
extern unsigned long _ka_init_fun_begin2;
extern unsigned long _ka_init_fun_end2;
extern unsigned long _ka_init_fun_begin3;
extern unsigned long _ka_init_fun_end3;
static void __INIT os_init(void)
{	
	bsp_init();

	g_time_tick_count = 0;
	g_interrupt_count = 0;

/*===========C_lib==========================*/
	__init_my_micro_lib();

	struct init_fun *struct_init_fun_ptr;
	for(struct_init_fun_ptr=(struct init_fun *)(&_ka_init_fun_begin1);
		struct_init_fun_ptr != (struct init_fun *)(&_ka_init_fun_end1);++struct_init_fun_ptr)
	{
		(*(struct_init_fun_ptr->fun))(); /* execute all the init function */
	}
	for(struct_init_fun_ptr=(struct init_fun *)(&_ka_init_fun_begin2);
		struct_init_fun_ptr != (struct init_fun *)(&_ka_init_fun_end2);++struct_init_fun_ptr)
	{
		(*(struct_init_fun_ptr->fun))(); /* execute all the init function */
	}
	for(struct_init_fun_ptr=(struct init_fun *)(&_ka_init_fun_begin3);
		struct_init_fun_ptr != (struct init_fun *)(&_ka_init_fun_end3);++struct_init_fun_ptr)
	{
		(*(struct_init_fun_ptr->fun))(); /* execute all the init function */
	}
}

void __INIT _os_start(void)
{
/*===========init all module==================*/
	os_init();
/*============================================*/

/*==============register initialization task===================*/
	if(0 != task_init_ready(&TCB_count_init,1023,PRIO_MAX-2,5,"count_init",count_init,NULL))
	{
		panic("create task count_init error\n");
	}
	if(0 != task_init_ready(&TCB_idle,128,PRIO_MAX-1,HZ,"idle",idle,NULL))
	{
		panic("create task idle error\n");
	}
/*==================================================================*/

	OSTCBCurPtr = _get_highest_prio_ready_TCB();
	OSTCBHighRdyPtr = OSTCBCurPtr;

/*===========system_tick====================*/
	__init_systick();
/*==========================================*/
	set_inter_stack();
	OSStartHighRdy();
	ASSERT(0);
	panic("should not go to the end of function _os_start()");/*should no go here*/
}

void start_kernel(void)
{
	CPU_IntDis();
	_os_start();
	panic("start_kernel() should not go here\n");
}

void task_start(void)
{
	if(0 != task_init_ready(&TCB_init,508,0,3,"init",thread_init,NULL))
	{
		panic("create task init error\n");
	}
#if CONFIG_SHELL_EN
	if(0 != task_init_ready(&TCB_shell,1000,0,3,"shell",shell,NULL))
	{
		panic("create task shell error\n");
	}
	#endif
#if CONFIG_TIMER_EN
	if(0 != task_init_ready(&TCB_timer_task,256,0,3,"timer_task",timer_task,NULL))
	{
		panic("create task timer_task error\n");
	}
#endif
	if(0 != task_creat_ready(500,4,5,"three",three,NULL,NULL))
	{
		panic("create task three error\n");
	}
/*
	if(0 != task_creat_ready(256,5,5,"four",four,NULL,NULL))
	{
		panic("create task four error\n");
	}
*/
/*
	if(0 != task_creat_ready(256,6,5,"five",five,NULL,NULL))
	{
		panic("create task five error\n");
	}
	if(0 != task_creat_ready(256,7,3,"six",six,NULL,NULL))
	{
		panic("create task six error\n");
	}
*/
}

static void count_init(void *para)	
{
	(void)para;
	
	sleep(2); /* for synchronization*/
	idle_num = 0;
	sleep(COUNT_DELAY_NUM);
	count_max_num = idle_num / COUNT_DELAY_NUM; /* COUNT_DELAY_NUM;*/
	idle_num = 0;
	
#if CONFIG_DEBUG_COUNT_INIT
	ka_printf("succeed to get count_max_num = %u\n",count_max_num);
#endif

	SYS_ENTER_CRITICAL();
	task_start();
	SYS_EXIT_CRITICAL();
}

static void idle(void *para)
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

/* set the stack for interrupt */
static void set_inter_stack(void)
{
	unsigned int MSP_top = (unsigned int)_alloc_power1_page(); /* allocate one page for interrupt stack*/
	MSP_top += PAGE_SIZE_BYTE;
	asm("MOVS R0, %0\t\n"
		"MSR MSP, R0\t\n"
		:
		:"r"(MSP_top)
		);
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
	if(0 != in_os_memory(ptr))
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(_case_free_buddy,ptr);
		return ;
	}
	unsigned int level = ((unsigned int)end_ptr - (unsigned int)ptr) / PAGE_SIZE_BYTE;
	ASSERT(0 == (level & (level-1)));
	level = FastLog2(level) + 1;
	struct buddy *buddy_ptr = (struct buddy *)_get_os_buddy_ptr_head();
	ASSERT(NULL != buddy_ptr);
	while(NULL != buddy_ptr)
	{
		if(0 == in_buddy_range(ptr))
		{
			switch(level)
			{
				case 1 :
					_return_power1_page(ptr);
					break;
				case 2 :
					_return_power2_page(ptr);
					break;
				case 3 :
					_return_power3_page(ptr);
					break;
				case 4 :
					_return_power4_page(ptr);
					break;
				case 5 :
					_return_power5_page(ptr);
					break;
				case 6 :
					_return_power6_page(ptr);
					break;
				case 7 :
					_return_power7_page(ptr);
					break;
				default :
					ka_printf("error level! FATAL ERROR!\n");
					ASSERT(0);
			}
			return ;
		}
		else
		{
			buddy_ptr = (struct buddy *)_get_next_buddy_ptr_head(buddy_ptr);
			continue;
		}
	}
	ka_printf("ptr fatal error!!!\n");
	return ;
}

static void thread_init(void *para)	
{
	(void)para;
	struct kmem_cache *kmem_cache_ptr;
	while(1)
	{
		sleep(THREAD_INIT_PERIOD * HZ); /*period*/
		/*free the memory */
		singly_list_for_each_entry(kmem_cache_ptr,cache_chain_head_ptr,node)
		{
			if(!list_empty(&kmem_cache_ptr->slabs_empty))
			{
				struct list_head *pos1,*pos2;
				list_for_each_safe(pos1,pos2,&kmem_cache_ptr->slabs_empty)
				{
					struct slab *slab_ptr = list_entry(pos1,struct slab,slab_chain);
					ASSERT(slab_ptr->current_block_num == slab_ptr->full_block_num);
					ASSERT(slab_ptr->block_size == kmem_cache_ptr->kmem_cache_slab_size);
					list_del(pos1);
					_case_slab_free_buddy(slab_ptr,slab_ptr->end_ptr);
				}
			}
		}
	}
#if CONFIG_MALLOC && CONFIG_ASSERT_DEBUG
extern void slab_list_check(const struct slab *slab_ptr);
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	singly_list_for_each_entry(kmem_cache_ptr, cache_chain_head_ptr, node)
	{
		if(!list_empty(&kmem_cache_ptr->slabs_full))
		{
			struct list_head *buffer;
			list_for_each(buffer,&kmem_cache_ptr->slabs_full)
			{
				const struct slab *slab_ptr = list_entry(buffer,struct slab,slab_chain);
				slab_list_check(slab_ptr);
			}
		}
		if(!list_empty(&kmem_cache_ptr->slabs_partial))
		{
			struct list_head *buffer;
			list_for_each(buffer,&kmem_cache_ptr->slabs_partial)
			{
				const struct slab *slab_ptr = list_entry(buffer,struct slab,slab_chain);
				slab_list_check(slab_ptr);
			}
		}
		if(!list_empty(&kmem_cache_ptr->slabs_empty))
		{
			struct list_head *buffer;
			list_for_each(buffer,&kmem_cache_ptr->slabs_empty)
			{
				const struct slab *slab_ptr = list_entry(buffer,struct slab,slab_chain);
				slab_list_check(slab_ptr);
			}
		}
	}
	CPU_CRITICAL_EXIT();
#endif
}

void recycle_memory(void)
{
	_remove_from_delay_heap(&TCB_init);
}
