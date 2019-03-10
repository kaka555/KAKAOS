#include <shell_debug.h>
#include <MCB.h>
#include <myassert.h>
#include <myMicroLIB.h>
#include <kakaosstdint.h>

#if CONFIG_SHELL_DEBUG_EN && CONFIG_SHELL_EN

static MCB MCB_for_shell_debug;
static int flag_for_shell_debug = 1;

static void __init_shell_variable_array(void);

void __init_shell_debug(void)
{
#if CONFIG_ASSERT_DEBUG
	int error;
	error = init_MCB(&MCB_for_shell_debug,0,MCB_TYPE_FLAG_BINARY);
#else
	init_MCB(&MCB_for_shell_debug,0,MCB_TYPE_FLAG_BINARY);
#endif
	ASSERT(FUN_EXECUTE_SUCCESSFULLY == error);
	__init_shell_variable_array();
}

void insert_break_point(char* file_name,unsigned line,const char* function_name)
{
#if CONFIG_ASSERT_DEBUG
	int error;
#endif
	if(flag_for_shell_debug)
	{
		ka_printf("program stop,now going to file: %s,line %u,function name is %s\n",file_name,line,function_name);
		ka_printf("%s","kaka_os>>");
#if CONFIG_ASSERT_DEBUG
		error = _p(&MCB_for_shell_debug,MCB_FLAG_WAIT,0);
#else
		_p(&MCB_for_shell_debug,MCB_FLAG_WAIT,0);
#endif
		ASSERT(FUN_EXECUTE_SUCCESSFULLY == error);
	}
}

void shell_debug_next(int argc, char const *argv[])
{
	(void)argc;
	(void)argv;
	_v(&MCB_for_shell_debug);
}

void shell_debug_run(int argc, char const *argv[])
{
	(void)argc;
	(void)argv;
	_v(&MCB_for_shell_debug);
	flag_for_shell_debug = 0;
}

void shell_debug_stop(int argc, char const *argv[])
{
	(void)argc;
	(void)argv;
	flag_for_shell_debug = 1;
}

/*the following is about shell variable*/
static struct singly_list_head variable_array[SHELL_V_ARRAY_SIZE];

void shell_debug_info(int argc, char const *argv[])
{
	int i;
	(void)argc;
	(void)argv;
	struct shell_variable *buffer;
	struct singly_list_head *pos;
	for(i=0;i<SHELL_V_ARRAY_SIZE;++i)
	{
		if(!singly_list_empty(&variable_array[i]))
		{	
			singly_list_for_each(pos,&variable_array[i])
			{
				buffer = singly_list_entry(pos,struct shell_variable,list);
				ka_printf("shell_variable's name is \"%s\"\n",buffer->shell_v_name);
			}
		}
	}
}

static void __init_shell_variable_array(void)
{
	int i;
	for(i=0;i<SHELL_V_ARRAY_SIZE;++i)
	{
		INIT_SINGLY_LIST_HEAD(&variable_array[i]);
	}
}

struct shell_variable *find_in_variable_array(const char *name)
{
	if(NULL == name)
	{
		return NULL;
	}
	unsigned int hash_value = shell_v_hash(name);
	ASSERT(hash_value < SHELL_V_ARRAY_SIZE);
	struct shell_variable *shell_variable_ptr;
	struct singly_list_head *pos;
	singly_list_for_each(pos,&variable_array[hash_value])
	{
		shell_variable_ptr = singly_list_entry(pos,struct shell_variable,list);
		if(0 == ka_strncmp(name,shell_variable_ptr->shell_v_name,ka_strlen(shell_variable_ptr->shell_v_name)))
		{
			return shell_variable_ptr;
		}
	}
	return NULL;
}

void shell_insert_variable(char *name,void *data_ptr,Shell_V_Type type)
{
	ASSERT(NULL != data_ptr);
	ASSERT( (SHELL_V_TYPE_UINT8  == type)  ||
			(SHELL_V_TYPE_UINT16 == type)  ||
			(SHELL_V_TYPE_UINT32 == type)  ||
			(SHELL_V_TYPE_INT8   == type)  ||
			(SHELL_V_TYPE_INT16  == type)  ||
			(SHELL_V_TYPE_INT32  == type)  ||
			(SHELL_V_TYPE_FLOAT  == type)  ||
			(SHELL_V_TYPE_DOUBLE == type)  ||
			(SHELL_V_TYPE_CHAR   == type)   ) ;
	struct shell_variable *ptr = ka_malloc(sizeof(struct shell_variable));
	ASSERT(NULL != ptr);
	ptr->shell_v_name = name;
	ptr->shell_v_type = type;
	ptr->data_ptr = data_ptr;
	unsigned int hash = shell_v_hash(name);
	singly_list_add(&ptr->list,&variable_array[hash]);
}

unsigned int shell_v_hash(const char *name)
{
	unsigned int sum = 0;
	while((*name != '\0') && (*name != ' ') && (*name != 0x0d) && (*name != 0x0a))
	{
		sum += *name++;
	}
	return sum % SHELL_V_ARRAY_SIZE;
}

void shell_v_display(struct shell_variable *shell_variable_ptr)
{
	ASSERT(shell_variable_ptr != NULL);
	switch(shell_variable_ptr->shell_v_type)
	{
		case SHELL_V_TYPE_UINT8:
		case SHELL_V_TYPE_UINT16:
		case SHELL_V_TYPE_UINT32:
			ka_printf("%u\n",*(unsigned int*)shell_variable_ptr->data_ptr);
			break ;

		case SHELL_V_TYPE_INT8:
		case SHELL_V_TYPE_INT16:
		case SHELL_V_TYPE_INT32:
			ka_printf("%d\n",*(int*)shell_variable_ptr->data_ptr);
			break ;

		case SHELL_V_TYPE_FLOAT:
			ka_printf("%f\n",*(float*)shell_variable_ptr->data_ptr);
			break;
		case SHELL_V_TYPE_DOUBLE:
			ka_printf("%f\n",*(double*)shell_variable_ptr->data_ptr);
			break ;

		case SHELL_V_TYPE_CHAR:
			ka_printf("%c\n",*(char*)shell_variable_ptr->data_ptr);
			break ;

		default :
			ASSERT(0);
			break ;
	}
}

void shell_v_display_addr(struct shell_variable *shell_variable_ptr)
{
	ASSERT(shell_variable_ptr != NULL);
	ka_printf("variable %s address is %p\n",shell_variable_ptr->shell_v_name,shell_variable_ptr->data_ptr);
}

void shell_v_write(struct shell_variable *shell_variable_ptr,const char *input_buffer_ptr)
{
	ASSERT(NULL != shell_variable_ptr);
	ASSERT(NULL != input_buffer_ptr);
	switch(shell_variable_ptr->shell_v_type)
	{
		case SHELL_V_TYPE_UINT8:
		case SHELL_V_TYPE_UINT16:
		case SHELL_V_TYPE_UINT32:
		case SHELL_V_TYPE_INT8:
		case SHELL_V_TYPE_INT16:
		case SHELL_V_TYPE_INT32:
		{
			INT32 int_buffer;
			int_buffer = ka_atoi(input_buffer_ptr);
			*(INT32 *)(shell_variable_ptr->data_ptr) = int_buffer;
			break ;
		}

		case SHELL_V_TYPE_FLOAT:
		{
			float float_buffer;
			float_buffer = ka_atof(input_buffer_ptr);
			*(float *)(shell_variable_ptr->data_ptr) = float_buffer;
			break ;
		}
		case SHELL_V_TYPE_DOUBLE:
		{
			double double_buffer;
			double_buffer = ka_atof(input_buffer_ptr);
			*(double *)(shell_variable_ptr->data_ptr) = double_buffer;
			break ;
		}

		case SHELL_V_TYPE_CHAR:
		{
			char char_buffer;
			char_buffer = *input_buffer_ptr;
			*(char *)(shell_variable_ptr->data_ptr) = char_buffer;
			break ;
		}

		default :
			ASSERT(0);
			break ;
	}
}

#endif
