#include <myMicroLIB.h>
#include <command_processor.h>
#include <MCB.h>
#include <myassert.h>
#include <shell_fun.h>
#include <TCB.h>
#include <slab.h>
#include <shell_debug.h>
#include <shell.h>
#include <buddy.h>
#include <os_ready.h>
#include <os_TCB_list.h>
#include <os_cpu.h>
#include <export.h>

#if CONFIG_SHELL_EN

static MCB MCB_for_shell;

static struct shell_buffer main_shell_buffer;
static char main_buffer[BUFFER_SIZE];
static char main_buffer_reserve[BUFFER_SIZE];

static struct shell_buffer *using_shell_buffer_ptr = NULL;

static inline void clear_input_buffer(void)
{
	using_shell_buffer_ptr->index = 0;
	using_shell_buffer_ptr->buffer[0] = '\0';
}

void shell_buffer_wait_str(const char *str_ptr) // thread will going to sleep
{
	ASSERT(NULL != str_ptr);
#if CONFIG_PARA_CHECK	
	if(NULL == str_ptr)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(shell_buffer_wait_str,str_ptr);
		return ;
	}
#endif
#if CONFIG_ASSERT_DEBUG
	int error;
#endif
	while(1)
	{
#if CONFIG_ASSERT_DEBUG
		error = p(&MCB_for_shell,MCB_FLAG_WAIT,0);
#else
		p(&MCB_for_shell,MCB_FLAG_WAIT,0);
#endif
		ASSERT(FUN_EXECUTE_SUCCESSFULLY == error);
		if(0 == ka_strncmp(using_shell_buffer_ptr->buffer,str_ptr,ka_strlen(str_ptr)))
		{
			return ;
		}
		ka_printf("wrong input\n");
		clear_input_buffer();
	}
}

struct shell_buffer *change_shell_buffer(struct shell_buffer *shell_buffer_ptr)
{
	ASSERT(NULL != shell_buffer_ptr);
#if CONFIG_PARA_CHECK
	if(NULL == shell_buffer_ptr)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(change_shell_buffer,shell_buffer_ptr);
		return NULL;
	}
#endif
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	struct shell_buffer *buffer = using_shell_buffer_ptr;
	using_shell_buffer_ptr = shell_buffer_ptr;
	CPU_CRITICAL_EXIT();
	return buffer;
}

int __init_shell_buffer(struct shell_buffer *shell_buffer_ptr,
	char *buffer_ptr,
	char *buffer_reserve_ptr) // if buffer_reserve_ptr is NULL,means no reserve buffer
{
	ASSERT((shell_buffer_ptr != NULL) 	&& 
			(shell_buffer_ptr != NULL));
#if CONFIG_PARA_CHECK
	if((NULL == shell_buffer_ptr) 	|| 
		(NULL == shell_buffer_ptr))
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(__init_shell_buffer,three_shell_buffer);
		return -ERROR_NULL_INPUT_PTR;
	}
#endif
	shell_buffer_ptr->buffer 			= buffer_ptr;
	shell_buffer_ptr->buffer_reserve 	= buffer_reserve_ptr;
	shell_buffer_ptr->index 			= 0;
	shell_buffer_ptr->index_reserve 	= 0;
	return FUN_EXECUTE_SUCCESSFULLY;
}

void put_in_shell_buffer(char c)  // deal with input layer
{
	if(c == 0x0a)
	{
		return ;
	}
	if( ! (	IS_LOWER(c) || IS_UPPER(c) || (0x0d == c) || 
			(0x03 == c) || (0x08 == c) || (' ' == c)  || 
			IS_NUM(c) 	|| IS_DOT(c)   || ('-' == c)  || 
			('+' == c)))
	{
		ka_printf("\nerror input\n");
		ka_printf("%s",using_shell_buffer_ptr->buffer);
		return ;
	}
	if(IS_UPPER(c)) 
	{
		c += 'a' - 'A'; 
	}
	if(0x08 == c) // backspace key
	{
		if(using_shell_buffer_ptr->index>0)
		{
			--(using_shell_buffer_ptr->index);
			ka_printf(" \b");
		}
		else
		{
			ASSERT(0 == using_shell_buffer_ptr->index);
			ka_putchar('\a');
		}
		return ;
	}
	if(using_shell_buffer_ptr->index < BUFFER_SIZE*0.8)
	{
		using_shell_buffer_ptr->buffer[(using_shell_buffer_ptr->index)++] = c;
		using_shell_buffer_ptr->buffer[using_shell_buffer_ptr->index] = '\0';
	}
	else
	{
		ka_printf("\ntoo long,invalid input,clear buffer...\n");
		using_shell_buffer_ptr->index = 0;
		return ;
	}
	if(0x0d == c)
	{
#if DEBUG_SHELL
		using_shell_buffer_ptr->buffer[using_shell_buffer_ptr->index-1] = '\0';
		ka_printf("%s\n",using_shell_buffer_ptr->buffer);
#endif
		v(&MCB_for_shell);
	}
}


//static int process(char *buffer_ptr);
static void redo(int argc, char const *argv[])
{
	(void)argc;
	(void)argv;
	if(using_shell_buffer_ptr->buffer_reserve)
	{
		unsigned int i;
		ASSERT(using_shell_buffer_ptr->index_reserve < BUFFER_SIZE);
		ka_printf("redo command: ");
		for(i=0;i<using_shell_buffer_ptr->index_reserve;++i)
		{
			ka_puts((const char *)(using_shell_buffer_ptr->argv_reserve)[i]);
		}
		ka_putchar('\n');
		match_and_execute_command(using_shell_buffer_ptr->index_reserve,
			(const char **)(using_shell_buffer_ptr->argv_reserve),
			get_command_processer(ka_strlen((using_shell_buffer_ptr->argv_reserve)[0])));
	}
	else
	{
		ka_printf("no reserved buffer,command not saved\n");
	}
}


//struct command -- add command in the corresponding array here(step 2)
// and write the function to execute
static struct command resident_command_1[] = 
{
	{
		.command_name = "r",
		.f = redo,
	}
};
static struct command resident_command_2[] = 
{
	{
		.command_name = "ka",
		.f = test,
	},
	{
		.command_name = "mm",
		.f = shell_memory,
	}
};
static struct command resident_command_3[] = 
{
#if CONFIG_SHELL_DEBUG_EN && CONFIG_SHELL_EN
	{
		.command_name = "run",
		.f = shell_debug_run,
	},
	{
		.command_name = "set",
		.f = shell_set,
	},
#endif
#if CONFIG_CPU_USE_RATE_CALCULATION
	{
		.command_name = "cpu",
		.f = cpu_rate,
	},
#endif
	{
		.command_name = "tcb",
		.f = shell_TCB_check,
	}
};
static struct command resident_command_4[] = 
{
#if CONFIG_TIME_EN
	{
		.command_name = "time",
		.f = shell_time,
	},
#endif
	{
		.command_name = "kmem",
		.f = shell_check_kmem,
	},
	{
		.command_name = "slab",
		.f = shell_check_slab,
	}
#if CONFIG_SHELL_DEBUG_EN && CONFIG_SHELL_EN
	,{
		.command_name = "next",
		.f = shell_debug_next,
	},
	{
		.command_name = "stop",
		.f = shell_debug_stop,
	},
	{
		.command_name = "echo",
		.f = shell_echo,
	},
	{
		.command_name = "addr",
		.f = shell_addr,
	}
#endif
};
static struct command resident_command_5[] = 
{	
	{
		.command_name = "stack",
		.f = shell_stack_check,
	},
	{
		.command_name = "clear",
		.f = shell_clear,
	},
	{
		.command_name = "buddy",
		.f = shell_buddy_debug,
	}
#if CONFIG_POWER_MANAGEMENT
	,{
		.command_name = "sleep",
		.f = shell_sleep,
	}
#endif
};
static struct command resident_command_6[] = 
{
	{
		.command_name = "reboot",
		.f = shell_reboot,
	}
#if CONFIG_MODULE
	,{
		.command_name = "module",
		.f = shell_module,
	}
#endif
};
static struct command resident_command_7[] = 
{
	{
		.command_name = "version",
		.f = shell_version,
	}
#if CONFIG_SHELL_DEBUG_EN && CONFIG_SHELL_EN
	,{
		.command_name = "debinfo",
		.f = shell_debug_info,
	}
#endif
#if CONFIG_MODULE
	,{
		.command_name = "symlist",
		.f = shell_symbol_list_display,
	}
#endif
	,{
		.command_name = "showreg",
		.f = shell_show_tasks_registers,
	}
};
static struct command resident_command_8[] = 
{
#if CONFIG_POWER_MANAGEMENT
	{
		.command_name = "shutdown",
		.f = shell_shutdown,
	}
#endif
};

extern void __init_command_n_ptr_hash_array(void);
static void shell_init(void)
{
	unsigned int i;
#if CONFIG_ASSERT_DEBUG
	int error;
#endif
	__init_command_n_ptr_hash_array();
	for(i=0;i<sizeof(resident_command_1)/sizeof(struct command);++i)
	{
		insert_struct_command_1(resident_command_1+i);
	}
	for(i=0;i<sizeof(resident_command_2)/sizeof(struct command);++i)
	{
		insert_struct_command_2(resident_command_2+i);
	}
	for(i=0;i<sizeof(resident_command_3)/sizeof(struct command);++i)
	{
		insert_struct_command_3(resident_command_3+i);
	}
	for(i=0;i<sizeof(resident_command_4)/sizeof(struct command);++i)
	{
		insert_struct_command_4(resident_command_4+i);
	}
	for(i=0;i<sizeof(resident_command_5)/sizeof(struct command);++i)
	{
		insert_struct_command_5(resident_command_5+i);
	}
	for(i=0;i<sizeof(resident_command_6)/sizeof(struct command);++i)
	{
		insert_struct_command_6(resident_command_6+i);
	}
	for(i=0;i<sizeof(resident_command_7)/sizeof(struct command);++i)
	{
		insert_struct_command_7(resident_command_7+i);
	}
	for(i=0;i<sizeof(resident_command_8)/sizeof(struct command);++i)
	{
		insert_struct_command_8(resident_command_8+i);
	}
#if CONFIG_ASSERT_DEBUG
	error = init_MCB(&MCB_for_shell,0,MCB_TYPE_FLAG_BINARY);
#else
	init_MCB(&MCB_for_shell,0,MCB_TYPE_FLAG_BINARY);
#endif
	ASSERT(FUN_EXECUTE_SUCCESSFULLY == error);
#if CONFIG_ASSERT_DEBUG
	error = __init_shell_buffer(&main_shell_buffer,main_buffer,main_buffer_reserve);
#else
	__init_shell_buffer(&main_shell_buffer,main_buffer,main_buffer_reserve);
#endif
	ASSERT(FUN_EXECUTE_SUCCESSFULLY == error);
	using_shell_buffer_ptr = &main_shell_buffer;
}

//public
static int process(char *buffer_ptr)
{
	char *ptr = buffer_ptr;
	unsigned int num = 0;
	int result;
	while(*ptr == ' ')
	{
		++ptr;
	}
	if((0x0d == *ptr) || (0x0a == *ptr))
	{
		return 0;
	}
	using_shell_buffer_ptr->argv[num++] = ptr;
	while((0x0d != *ptr) && (0x0a != *ptr))
	{
		if(' ' != *ptr)
		{
			++ptr;
			continue;
		}
		else
		{
			*ptr = '\0';
			++ptr;
			while(' ' == *ptr)
			{
				++ptr;
			}
			if((0x0d != *ptr) && (0x0a != *ptr))
			{
				using_shell_buffer_ptr->argv[num++] = ptr;
				if(ARGV_SIZE+1 == num)
				{
					ka_printf("too many argument\n");
					return 0;
				}
			}
		}
	}
	*ptr = '\0';
		
	result = match_and_execute_command(num,
		(const char **)(using_shell_buffer_ptr->argv),
		get_command_processer(ka_strlen(using_shell_buffer_ptr->argv[0])));
    if(result == 1)
    {
    	return 0;
    }
    return num;
}

extern TCB TCB_count_init;
void shell(void *para)
{
#if CONFIG_ASSERT_DEBUG
	int error;
#endif
	(void)para;
	int result;
	task_delete(&TCB_count_init);
	shell_init();
	ka_printf("%s\n","/*************************");
	ka_printf("%s\n","*");
	ka_printf("%s\n","*   kaka_os  shell");
	ka_printf("%s\n","*");
	ka_printf("%s\n","*************************/");
	ka_printf("%s","kaka_os>>");
	while(1)
	{
#if CONFIG_ASSERT_DEBUG
		error = p(&MCB_for_shell,MCB_FLAG_WAIT,0);
#else
		p(&MCB_for_shell,MCB_FLAG_WAIT,0);
#endif
		ASSERT(FUN_EXECUTE_SUCCESSFULLY == error);
		result = process(using_shell_buffer_ptr->buffer);
		if(0 != result && using_shell_buffer_ptr->buffer_reserve)
		{
			ka_memcpy(using_shell_buffer_ptr->buffer_reserve,
				using_shell_buffer_ptr->buffer,
				using_shell_buffer_ptr->index);
			ka_memcpy(using_shell_buffer_ptr->argv_reserve,using_shell_buffer_ptr->argv,result*sizeof(char *));
			unsigned int i;
			unsigned int buf = (unsigned int)(using_shell_buffer_ptr->buffer_reserve) - (unsigned int)(using_shell_buffer_ptr->buffer);
			for(i=0;i<result;++i)
			{
				using_shell_buffer_ptr->argv_reserve[i] += buf;
			}
			using_shell_buffer_ptr->index_reserve = result;
		}
		clear_input_buffer();
		ka_printf("%s","kaka_os>>");
	}
		
}

#endif
