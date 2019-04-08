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
#include <osinit.h>
#include <module.h>
#include <vfs.h>

#if CONFIG_SHELL_EN

/*add corresponding function here*/
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
	ka_printf("VERSION : KAKAOS--0.09\n");
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
	struct buddy *buddy_ptr = (struct buddy *)_get_os_buddy_ptr_head();
	ASSERT(NULL != buddy_ptr);
	unsigned int j = 0;
	while(NULL != buddy_ptr)
	{
		ka_printf("NO.%u memory : \n",j++);
		ka_printf("=====================================================\n");
		ka_printf("start address is %p\n",buddy_ptr->buddy_space_start_ptr);
		ka_printf("rest buddy space is %u\n",_get_current_buddy_space());
		ka_printf("the total space is %uKB\n",PAGE_SIZE_KB * buddy_ptr->info.page_num);
		ka_printf("now there are %u%% left\n",100*_get_current_buddy_space()/(PAGE_SIZE_KB * buddy_ptr->info.page_num));
		ka_printf("\n\n");
		buddy_ptr = (struct buddy *)_get_next_buddy_ptr_head(buddy_ptr);
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
	if(2 == argc)
	{
		struct shell_variable *shell_variable_ptr = find_in_variable_array(argv[1]);
		if(NULL == shell_variable_ptr)
		{
			ka_printf("no such variable\n");
			return ;
		}
		shell_v_display(shell_variable_ptr);
	}
	else if(4 == argc)
	{
		shell_vfs_echo(argv);
	}
	else
	{
		ka_printf("command error\n");
	}
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
#if CONFIG_DEBUG_ON
	shell_check_os_ready();
	shell_delay_heap_check();
#endif
}

extern void ReBoot();
void shell_reboot(int argc, char const *argv[])
{
	(void)argc;
	(void)argv;
	ReBoot();
}

#if CONFIG_MODULE
extern void show_get_size(void);
extern int add_page_alloc_record(unsigned int level,void *ptr);
/******************
* 
*   insmod -name=dmodule -prio=20 -stacksize=2048 -filesize=4096 // install a new module
*   insmod -r -name=dmodule -prio=20 -stacksize=2048 -filesize=4096//restart a module which has been loaded 
*
*******************/

void shell_module(int argc, char const *argv[])
{
	void *buf;
	char *shell_buf_ptr;
	int error;
	unsigned int stack_size = D_MODULE_DEFAULT_STACK_SIZE;
	TASK_PRIO_TYPE prio = D_MODULE_DEFAULT_PRIO;
	char *value_ptr;
	char *name = D_MODULE_DEFAULT_NAME;

/*get the parament*/
	value_ptr = get_para_add(argc,argv,"-stacksize=");
	if(NULL != value_ptr)
	{
		stack_size = ka_atoi(value_ptr);
		KA_WARN(DEBUG_TYPE_MODULE,"get stacksize %u\n",stack_size);
	}
	value_ptr = get_para_add(argc,argv,"-prio=");
	if(NULL != value_ptr)
	{
		prio = ka_atoi(value_ptr);
	}

/*get module's name*/
	value_ptr = get_para_add(argc,argv,"-name=");
	if(NULL != value_ptr)
	{
		name = value_ptr;
		KA_WARN(DEBUG_TYPE_MODULE,"get name %s\n",name);
	}

	value_ptr = get_para_add(argc,argv,"-r");
	if(NULL != value_ptr)
	{
		/*restart a module*/
		_restart_module(stack_size,prio,name);
		return ;
	}

/*Look for duplicate names*/
	error = _check_same_mod_name(name);
	if(0 != error)
	{
		ka_printf("please change an other module name\n");
		return ;
	}
/* allocate room for receiving module's data*/
	value_ptr = get_para_add(argc,argv,"-filesize=");
	if(NULL != value_ptr)
	{
		buf = ka_malloc(ka_atoi(value_ptr));
	}
	else
	{
		buf = ka_malloc(2047);
	}
	if (NULL == buf)
	{
		ka_printf("no enough room for module\n");
		return ;
	}
/* allocate room for shell*/
	struct shell_buffer shell_buffer;
	shell_buf_ptr = (char *)ka_malloc(20);
	if(NULL == shell_buf_ptr)
	{
		ka_printf("no enough room for module\n");
		goto out;
	}
	if(__init_shell_buffer(&shell_buffer,shell_buf_ptr,NULL,20) < 0)
	{
		ka_printf("module change buffer error\n");
		goto out1;
	}
	struct shell_buffer *sys_shell_buffer_ptr = _change_shell_buffer(&shell_buffer); /* save the system input buffer */
	_set_module_buffer(buf);

	ka_printf("now transfer the module,input 'end' to end the transformation\n");
	
	_shell_buffer_wait_str("end");  /* thread will going to sleep*/

	show_get_size();

	_change_shell_buffer(sys_shell_buffer_ptr);
	ka_free(shell_buf_ptr);

	ka_printf("execute module\n");
	_dlmodule_exec(stack_size,prio,name);
	ka_free(buf);
	_clear_module_buffer();
	return ;
out1:
	ka_free(shell_buf_ptr);
out:
	ka_free(buf);
	return ;
}
#endif

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
void cpu_rate(int argc, char const *argv[])
{
	(void)argc;
	(void)argv;
	ka_printf("cpu use rate is %u%%\n",get_cpu_use_rate());
}
#endif

#if CONFIG_DEBUG_ON
extern int in_os_memory(void *ptr);
void shell_check_memory(int argc, char const *argv[])
{
	if(2 != argc)
	{
		ka_printf("parameter error\n");
		return ;
	}
	UINT32 num = ka_atoi(argv[1]);
	if(in_os_memory((void *)num) < 0)
	{
		ka_printf("add 0x%p is not a legal address\n",(void *)num);
		return ;
	}
	ka_printf("value of add 0x%p is 0x%x\n",(void *)num,*(UINT32 *)num);
}
#endif

#endif

