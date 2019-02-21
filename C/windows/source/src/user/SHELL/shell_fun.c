#include <shell_fun.h>
#include <os_time.h>
#include <myMicroLIB.h>
#include <buddy.h>
#include <shell_debug.h>
#include <myassert.h>
#include <shell.h>
#include <os_delay.h>
#include <os_ready.h>
#include <os_TCB_list.h>
#include <printf_debug.h>
#include <buddy.h>

#if CONFIG_SHELL_EN

//add corresponding function here
void test(int argc, char const *argv[])
{
	(void)argc;
	(void)argv;
	int i;
	for(i=0;i<argc;++i)
	{
		ka_printf("parament %d : %s\n",i,argv[i]);
	}
}
void shell_version(int argc, char const *argv[])
{
	(void)argc;
	(void)argv;
	ka_printf("VERSION : KAKAOS--0.07\n");
}
#if CONFIG_TIME_EN
void shell_time(int argc, char const *argv[])
{
	(void)argc;
	(void)argv;
	system_time_display();
}
#endif
void shell_memory(int argc, char const *argv[])
{
	(void)argc;
	(void)argv;
	struct buddy *buddy_ptr = (struct buddy *)get_os_buddy_ptr_head();
	ASSERT(NULL != buddy_ptr);
	unsigned int j = 0;
	while(NULL != buddy_ptr)
	{
		PRINTF("NO.%u memory : \n",j++);
		PRINTF("=====================================================\n");
		PRINTF("start address is %p\n",buddy_ptr->buddy_space_start_ptr);
		PRINTF("rest buddy space is %u\n",get_current_buddy_space());
		PRINTF("the total space is %uKB\n",PAGE_SIZE_KB * buddy_ptr->info.page_num);
		PRINTF("now there are %u%% left\n",100*get_current_buddy_space()/(PAGE_SIZE_KB * buddy_ptr->info.page_num));
		PRINTF("\n\n");
		buddy_ptr = (struct buddy *)get_next_buddy_ptr_head(buddy_ptr);
	}
}
void shell_clear(int argc, char const *argv[])
{
	(void)argc;
	(void)argv;
	unsigned int i;
	for(i=0;i<40;++i)
	{
		ka_printf("\n");
	}
}
#if CONFIG_SHELL_DEBUG_EN && CONFIG_SHELL_EN
void shell_echo(int argc, char const *argv[])
{
	if(argc < 2)
	{
		ka_printf("too few arguments!\n");
		return ;
	}
	struct shell_variable *shell_variable_ptr = find_in_variable_array(argv[1]);
	if(NULL == shell_variable_ptr)
	{
		ka_printf("no such variable\n");
		return ;
	}
	shell_v_display(shell_variable_ptr);
	return ;
}

void shell_set(int argc, char const *argv[])
{
	if(argc < 3)
	{
		ka_printf("too few arguments!\n");
		return ;
	}
	struct shell_variable *shell_variable_ptr;
	shell_variable_ptr = find_in_variable_array(argv[1]);
	if(NULL == shell_variable_ptr)
	{
		ka_printf("no such variable!\n");
		return ;
	}
	shell_v_write(shell_variable_ptr,(const char *)argv[2]);
	return ;
}

void shell_addr(int argc, char const *argv[])
{
	if(argc < 2)
	{
		ka_printf("too few arguments!\n");
		return ;
	}
	struct shell_variable *shell_variable_ptr = find_in_variable_array(argv[1]);
	if(NULL == shell_variable_ptr)
	{
		ka_printf("no such variable\n");
		return ;
	}
	shell_v_display_addr(shell_variable_ptr);
}
#endif

void shell_TCB_check(int argc, char const *argv[])
{
	(void)argc;
	(void)argv;
	shell_check_TCB_list();
	shell_check_os_ready();
	shell_delay_heap_check();
}

extern void ReBoot();
void shell_reboot(int argc, char const *argv[])
{
	(void)argc;
	(void)argv;
	ReBoot();
}

#if CONFIG_POWER_MANAGEMENT
extern void sys_sleep(void);
void shell_sleep(int argc, char const *argv[])
{
	(void)argc;
	(void)argv;
	ka_printf("kernel going to sleep\n"
				"any interrupt or a click can wake up kernel\n");
	sys_sleep();
	ka_printf("kernel run\n");
}

extern void sys_shutdown(void);
void shell_shutdown(int argc, char const *argv[])
{
	(void)argc;
	(void)argv;
	ka_printf("shutdown now\n");
	sys_shutdown();
}
#endif

#if CONFIG_CPU_USE_RATE_CALCULATION
extern unsigned int get_cpu_use_rate(void);
inline void cpu_rate(int argc, char const *argv[])
{
	(void)argc;
	(void)argv;
	ka_printf("cpu use rate is %u%%\n",get_cpu_use_rate());
}
#endif

#endif

