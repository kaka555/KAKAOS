#include <shell_debug.h>
#include <MCB.h>
#include <myassert.h>
#include <myMicroLIB.h>
#include <kakaosstdint.h>
#include <sys_init_fun.h>

/**
 * this file realize the shell debug function
 */

#if CONFIG_SHELL_DEBUG_EN && CONFIG_SHELL_EN

static MCB MCB_for_shell_debug;
static int flag_for_shell_debug = 1;

static void __init_shell_variable_array(void);

static void __INIT __init_shell_debug(void)
{
#if CONFIG_ASSERT_DEBUG
	int error;
	error = init_MCB(&MCB_for_shell_debug,0,MCB_TYPE_FLAG_BINARY);
#else
	init_MCB(&MCB_for_shell_debug,0,MCB_TYPE_FLAG_BINARY);
#endif
	ASSERT(FUN_EXECUTE_SUCCESSFULLY == error,ASSERT_PARA_AFFIRM);
	__init_shell_variable_array();
}
INIT_FUN(__init_shell_debug,1);

/**
 * @Author      kaka
 * @DateTime    2019-06-11
 * @description : user is not recommanded to call this function, 
 * it is strongly recommanded to use the macro INSERT_BREAK_POINT()
 * @param       file_name     [description]
 * @param       line          [description]
 * @param       function_name [description]
 */
void insert_break_point(const char* file_name,unsigned line,const char* function_name)
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
		ASSERT(FUN_EXECUTE_SUCCESSFULLY == error,ASSERT_PARA_AFFIRM);
	}
}

/**
 * @Author      kaka
 * @DateTime    2019-06-11
 * @description : "next" command
 * @param       argc       [description]
 * @param       argv       [description]
 */
void shell_debug_next(int argc, char const *argv[])
{
	(void)argc;
	(void)argv;
	_v(&MCB_for_shell_debug);
}

/**
 * @Author      kaka
 * @DateTime    2019-06-11
 * @description : "run" command
 * @param       argc       [description]
 * @param       argv       [description]
 */
void shell_debug_run(int argc, char const *argv[])
{
	(void)argc;
	(void)argv;
	_v(&MCB_for_shell_debug);
	flag_for_shell_debug = 0;
}

/**
 * @Author      kaka
 * @DateTime    2019-06-11
 * @description : "stop" command
 * @param       argc       [description]
 * @param       argv       [description]
 */
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
	ASSERT(hash_value < SHELL_V_ARRAY_SIZE,ASSERT_PARA_AFFIRM);
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

/**
 * @Author      kaka
 * @DateTime    2019-06-11
 * @description : user call this function to insert a variable for debug, and then you can
 * write, read, and print it in the shell
 * @param       name       [description]
 * @param       data_ptr   [description]
 * @param       type       [description]
 */
void shell_insert_variable(const char *name,void *data_ptr,Shell_V_Type type)
{
	ASSERT(NULL != data_ptr,ASSERT_INPUT);
	ASSERT( (SHELL_V_TYPE_UINT8  == type)  ||
			(SHELL_V_TYPE_UINT16 == type)  ||
			(SHELL_V_TYPE_UINT32 == type)  ||
			(SHELL_V_TYPE_INT8   == type)  ||
			(SHELL_V_TYPE_INT16  == type)  ||
			(SHELL_V_TYPE_INT32  == type)  ||
			(SHELL_V_TYPE_FLOAT  == type)  ||
			(SHELL_V_TYPE_DOUBLE == type)  ||
			(SHELL_V_TYPE_CHAR   == type),ASSERT_INPUT   ) ;
	struct shell_variable *ptr = ka_malloc(sizeof(struct shell_variable));
	ASSERT(NULL != ptr,ASSERT_INPUT);
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
	ASSERT(shell_variable_ptr != NULL,ASSERT_INPUT);
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
			ASSERT(0,ASSERT_BAD_EXE_LOCATION);
			break ;
	}
}

void shell_v_display_addr(struct shell_variable *shell_variable_ptr)
{
	ASSERT(shell_variable_ptr != NULL,ASSERT_INPUT);
	ka_printf("variable %s address is %p\n",shell_variable_ptr->shell_v_name,shell_variable_ptr->data_ptr);
}

void shell_v_write(struct shell_variable *shell_variable_ptr,const char *input_buffer_ptr)
{
	ASSERT(NULL != shell_variable_ptr,ASSERT_INPUT);
	ASSERT(NULL != input_buffer_ptr,ASSERT_INPUT);
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
			ASSERT(0,ASSERT_BAD_EXE_LOCATION);
			break ;
	}
}

#endif
