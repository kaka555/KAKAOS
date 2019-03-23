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
#include <module.h>
#include <vfs.h>
#include <printf_debug.h>
#include <sys_init_fun.h>

static unsigned int _process(char *buffer_ptr);

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

void _shell_buffer_wait_str(const char *str_ptr) /* thread will going to sleep*/
{
	ASSERT(NULL != str_ptr);
	int error;
	while(1)
	{
		error = _p(&MCB_for_shell,MCB_FLAG_WAIT,0);
		ASSERT(FUN_EXECUTE_SUCCESSFULLY == error);
		if(0 == ka_strncmp(using_shell_buffer_ptr->buffer,str_ptr,ka_strlen(str_ptr)))
		{
			return ;
		}
		ka_printf("wrong input\n");
		clear_input_buffer();
	}
}

char *get_para_add(int argc, char const *argv[], const char *pre_name)
{
	unsigned int i;
	for(i=1;i<argc;++i)
	{
		if(0 == ka_strncmp(pre_name,argv[i],ka_strlen(pre_name)))
		{
			return (char *)(argv[i] + ka_strlen(pre_name));
		}
	}
	return NULL;
}

struct shell_buffer *_change_shell_buffer(struct shell_buffer *shell_buffer_ptr)
{
	ASSERT(NULL != shell_buffer_ptr);
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	struct shell_buffer *buffer = using_shell_buffer_ptr;
	using_shell_buffer_ptr = shell_buffer_ptr;
	CPU_CRITICAL_EXIT();
	return buffer;
}

int __init_shell_buffer(struct shell_buffer *shell_buffer_ptr,
	char *buffer_ptr,
	char *buffer_reserve_ptr,
	unsigned int buffer_size) /* if buffer_reserve_ptr is NULL,means no reserve buffer*/
{
	ASSERT((shell_buffer_ptr != NULL) 	&& 
			(buffer_ptr != NULL));
	shell_buffer_ptr->buffer 			= buffer_ptr;
	shell_buffer_ptr->buffer_reserve 	= buffer_reserve_ptr;
	shell_buffer_ptr->index 			= 0;
	shell_buffer_ptr->index_reserve 	= 0;
	shell_buffer_ptr->buffer_size 		= buffer_size;
	return FUN_EXECUTE_SUCCESSFULLY;
}

extern unsigned int command_list_hash(const char *command_ptr);
static void deal_with_tab(void)
{
	ASSERT(using_shell_buffer_ptr->buffer != NULL);
	unsigned int num = _process(using_shell_buffer_ptr->buffer);
	if((num <= 1) || (num >= ARGV_SIZE)) /* useless input */
	{
		KA_WARN(DEBUG_TYPE_SHELL_TAB,"invalid num\n");
		return ;
	}
	KA_WARN(DEBUG_TYPE_SHELL_TAB,"get num is %u\n",num);
	unsigned int len = ka_strlen(using_shell_buffer_ptr->argv[num-1]);
	unsigned int index = 0;
	unsigned int same = 0;
	struct command_processer *command_processer_ptr = _get_command_processer(ka_strlen(using_shell_buffer_ptr->argv[0]));
	if(NULL == command_processer_ptr)
	{
		ka_printf("command not found\n");
		return ;
	}
	KA_WARN(DEBUG_TYPE_SHELL_TAB,"command_processer length is %u\n",command_processer_ptr->command_length);
	struct command *struct_command_ptr;
	struct singly_list_head *head = &command_processer_ptr->command_list_address[command_list_hash(using_shell_buffer_ptr->argv[0])];
	singly_list_for_each_entry(struct_command_ptr,head,list)
	{
		if(0 == ka_strncmp(using_shell_buffer_ptr->argv[0],struct_command_ptr->command_name,command_processer_ptr->command_length))
		{
			/* get command */
			if(NULL == struct_command_ptr->para_arv)
			{
				KA_WARN(DEBUG_TYPE_SHELL_TAB,"command %s has no parament\n",struct_command_ptr->command_name);
				return ;
			}
			unsigned int i;
			for(i=0;i<struct_command_ptr->para_len;++i)
			{
				if(0 == ka_strncmp(struct_command_ptr->para_arv[i],using_shell_buffer_ptr->argv[num-1],len))
				{
					++same;
					if(same > 1)
					{
						KA_WARN(DEBUG_TYPE_SHELL_TAB,"more than one parament\n");
						return ;
					}
					index = i;
				}
			}
			if(0 == same)
			{
				KA_WARN(DEBUG_TYPE_SHELL_TAB,"no same parament\n");
			}
			else
			{
				ASSERT(1 == same);
				ka_strcpy(using_shell_buffer_ptr->buffer + using_shell_buffer_ptr->index,
						struct_command_ptr->para_arv[index] + len);
				using_shell_buffer_ptr->index += ka_strlen(struct_command_ptr->para_arv[index]) - len;
				ka_printf("%s",struct_command_ptr->para_arv[index] + len);
			}
			return ;
		}
	}
	KA_WARN(DEBUG_TYPE_SHELL_TAB,"command not found\n");
	return ;
}

static void recover_shell_buffer(void)
{
	unsigned int i;
	char *buffer = using_shell_buffer_ptr->buffer;
	for(i=0;i<using_shell_buffer_ptr->index;++i)
	{
		if('\0' == *(buffer + i))
		{
			*(buffer + i) = ' ';
		}
	}
}

void _put_in_shell_buffer(char c)  /* deal with input layer*/
{
	if(c == 0x0a)
	{
		return ;
	}
	if( ! (	IS_LOWER(c) || IS_UPPER(c) || (0x0d == c) || 
			(0x03 == c) || (0x08 == c) || (' ' == c)  || 
			IS_NUM(c) 	|| IS_DOT(c)   || ('-' == c)  || 
			('+' == c)	|| ('=' == c)  || ('/' == c)  ||
			(0x09 == c) || ('>' == c)))
	{
		ka_printf("\nerror input\n");
		ka_printf("%s",using_shell_buffer_ptr->buffer);
		return ;
	}
	if(IS_UPPER(c)) 
	{
		c += 'a' - 'A'; 
	}
	else if(0x08 == c) /* backspace key*/
	{
		if(using_shell_buffer_ptr->index > 0)
		{
			--(using_shell_buffer_ptr->index);
			ka_printf("\b \b");
		}
		else
		{
			ASSERT(0 == using_shell_buffer_ptr->index);
		}
		return ;
	}
	else if(0x09 == c) /* tab */
	{
		using_shell_buffer_ptr->buffer[(using_shell_buffer_ptr->index)] = 0x0d;
		deal_with_tab();
		recover_shell_buffer();
		return ;
	}
	else if(0x0d == c) /* enter */
	{
		ka_printf("\n");
		using_shell_buffer_ptr->buffer[(using_shell_buffer_ptr->index)++] = 0x0d;
		using_shell_buffer_ptr->buffer[using_shell_buffer_ptr->index] = '\0';
		KA_WARN(DEBUG_TYPE_SHELL,"%s\n",using_shell_buffer_ptr->buffer);
		_v(&MCB_for_shell);
		return ;
	}
	if(using_shell_buffer_ptr->index < using_shell_buffer_ptr->buffer_size)
	{
		ka_putchar(c);
		using_shell_buffer_ptr->buffer[(using_shell_buffer_ptr->index)++] = c;
		using_shell_buffer_ptr->buffer[using_shell_buffer_ptr->index] = '\0';
	}
	else
	{
		ka_printf("\ntoo long,invalid input,clear buffer...\n");
		using_shell_buffer_ptr->index = 0;
		return ;
	}
}

static int _shell_exec(const char *command)
{
	unsigned int len = ka_strlen(command);
	if(len + 1 > using_shell_buffer_ptr->buffer_size)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(_shell_exec,command);
		return -ERROR_USELESS_INPUT;
	}
	unsigned int i;
	for(i=0;i<len;++i)
	{
		_put_in_shell_buffer(command[i]);
	}
	_put_in_shell_buffer(0x0d);
	return FUN_EXECUTE_SUCCESSFULLY;
}

int shell_exec(const char *command)
{
	if(NULL == command)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(shell_exec,command);
		return -ERROR_NULL_INPUT_PTR;
	}
	return _shell_exec(command);
}

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
		_match_and_execute_command(using_shell_buffer_ptr->index_reserve,
			(const char **)(using_shell_buffer_ptr->argv_reserve),
			_get_command_processer(ka_strlen((using_shell_buffer_ptr->argv_reserve)[0])));
	}
	else
	{
		ka_printf("no reserved buffer,command not saved\n");
	}
}

/* parament list */
static char *insmod_list[] = {"-name=","-prio=","-stacksize=","-filesize="};

/*struct command -- add command in the corresponding array here(step 2)
** and write the function to execute*/
static struct command resident_command_1[] = 
{
	{
		.command_name = "r",
		.f = redo,
	}
};
static struct command resident_command_2[] = 
{
#if CONFIG_VFS
	{
		.command_name = "ls",
		.f = shell_ls,
	},
	{
		.command_name = "cd",
		.f = shell_cd,
	},
	{
		.command_name = "rm",
		.f = shell_rm,
	},
#endif
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
#if CONFIG_VFS
	{
		.command_name = "pwd",
		.f = shell_pwd,
	},
	{
		.command_name = "cat",
		.f = shell_cat,
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
#if CONFIG_MODULE
	,{
		.command_name = "lsmod",
		.f = shell_list_module,
	}
	,{
		.command_name = "rmmod",
		.f = shell_remove_module,
	}
#endif
#if CONFIG_VFS
	,{
		.command_name = "touch",
		.f = shell_touch,
	}
	,{
		.command_name = "mkdir",
		.f = shell_mkdir,
	}
	,{
		.command_name = "rmdir",
		.f = shell_rmdir,
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
		.command_name = "insmod",
		.f = shell_module,
		.para_arv = insmod_list,
		.para_len = sizeof(insmod_list)/sizeof(*insmod_list),
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
	,{
		.command_name = "modinfo",
		.f = shell_modinfo,
	}
#endif
	,{
		.command_name = "checkmm",
		.f = shell_check_memory,
	}
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

struct command *_get_command_ptr(const char *command_name)
{
	unsigned int len = ka_strlen(command_name);
	struct command_processer *command_processer_ptr = _get_command_processer(len);
	if(NULL == command_processer_ptr)
	{
		return NULL;
	}
	struct singly_list_head *pos;
	struct command *struct_command_ptr;
	singly_list_for_each(pos,&command_processer_ptr->command_list_address[command_list_hash(command_name)])
	{
		struct_command_ptr = singly_list_entry(pos,struct command,list);
		if(0 == ka_strcmp(command_name,struct_command_ptr->command_name))
		{
			return struct_command_ptr;
		}
	}
	return NULL;
}

static void shell_pre(void)
{
#if CONFIG_ASSERT_DEBUG
	int error;
#endif
#if CONFIG_ASSERT_DEBUG
	error = init_MCB(&MCB_for_shell,0,MCB_TYPE_FLAG_BINARY);
#else
	init_MCB(&MCB_for_shell,0,MCB_TYPE_FLAG_BINARY);
#endif
	ASSERT(FUN_EXECUTE_SUCCESSFULLY == error);
#if CONFIG_ASSERT_DEBUG
	error = __init_shell_buffer(&main_shell_buffer,main_buffer,main_buffer_reserve,BUFFER_SIZE);
#else
	__init_shell_buffer(&main_shell_buffer,main_buffer,main_buffer_reserve);
#endif
	ASSERT(FUN_EXECUTE_SUCCESSFULLY == error);
	using_shell_buffer_ptr = &main_shell_buffer;
}

extern void __init_command_n_ptr_hash_array(void);
static void __init_shell(void)
{
	unsigned int i;
	__init_command_n_ptr_hash_array();
	for(i=0;i<sizeof(resident_command_1)/sizeof(struct command);++i)
	{
		_insert_struct_command_1(resident_command_1+i);
	}
	for(i=0;i<sizeof(resident_command_2)/sizeof(struct command);++i)
	{
		_insert_struct_command_2(resident_command_2+i);
	}
	for(i=0;i<sizeof(resident_command_3)/sizeof(struct command);++i)
	{
		_insert_struct_command_3(resident_command_3+i);
	}
	for(i=0;i<sizeof(resident_command_4)/sizeof(struct command);++i)
	{
		_insert_struct_command_4(resident_command_4+i);
	}
	for(i=0;i<sizeof(resident_command_5)/sizeof(struct command);++i)
	{
		_insert_struct_command_5(resident_command_5+i);
	}
	for(i=0;i<sizeof(resident_command_6)/sizeof(struct command);++i)
	{
		_insert_struct_command_6(resident_command_6+i);
	}
	for(i=0;i<sizeof(resident_command_7)/sizeof(struct command);++i)
	{
		_insert_struct_command_7(resident_command_7+i);
	}
	for(i=0;i<sizeof(resident_command_8)/sizeof(struct command);++i)
	{
		_insert_struct_command_8(resident_command_8+i);
	}
}
INIT_FUN(__init_shell,1);

/* return argc */
static unsigned int _process(char *buffer_ptr)
{
	char *ptr = buffer_ptr;
	unsigned int num = 0;
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
					return ARGV_SIZE;
				}
			}
		}
	}
	*ptr = '\0';
	return num;
}

static int process(char *buffer_ptr)
{
	unsigned int num = _process(buffer_ptr);
	if((0 == num) || (num >= ARGV_SIZE))
	{
		return 0;
	}
	int result = _match_and_execute_command(num,
		(const char **)(using_shell_buffer_ptr->argv),
		_get_command_processer(ka_strlen(using_shell_buffer_ptr->argv[0])));
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
	shell_pre();
	ka_printf("%s\n","/*************************");
	ka_printf("%s\n","*");
	ka_printf("%s\n","*   kaka_os  shell");
	ka_printf("%s\n","*");
	ka_printf("%s\n","*************************/");
	ka_printf("%s","kaka_os>>");
	while(1)
	{
#if CONFIG_ASSERT_DEBUG
		error = _p(&MCB_for_shell,MCB_FLAG_WAIT,0);
#else
		_p(&MCB_for_shell,MCB_FLAG_WAIT,0);
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
