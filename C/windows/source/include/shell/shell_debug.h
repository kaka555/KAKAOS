#ifndef _SHELL_DEBUG_H
#define _SHELL_DEBUG_H

#include <ka_configuration.h>
#include <singly_linked_list.h>


#if CONFIG_SHELL_DEBUG_EN && CONFIG_SHELL_EN

#if CONFIG_BREAK_POINT_DEBUG
	#define INSERT_BREAK_POINT() insert_break_point(__FILE__,__LINE__,__FUNCTION__)
#else
	#define INSERT_BREAK_POINT()  
#endif

typedef char Shell_V_Type;
#define SHELL_V_TYPE_UINT8   1
#define SHELL_V_TYPE_UINT16  2
#define SHELL_V_TYPE_UINT32  3
#define SHELL_V_TYPE_INT8    4
#define SHELL_V_TYPE_INT16   5
#define SHELL_V_TYPE_INT32   6
#define SHELL_V_TYPE_FLOAT   7
#define SHELL_V_TYPE_DOUBLE  8
#define SHELL_V_TYPE_CHAR    9

#define shell_insert_variable_INT(name,ptr) 	shell_insert_variable((name),(ptr),SHELL_V_TYPE_INT32)
#define shell_insert_variable_UINT(name,ptr) 	shell_insert_variable((name),(ptr),SHELL_V_TYPE_UINT32)
#define shell_insert_variable_FLOAT(name,ptr) 	shell_insert_variable((name),(ptr),SHELL_V_TYPE_FLOAT)
#define shell_insert_variable_DOUBLE(name,ptr) 	shell_insert_variable((name),(ptr),SHELL_V_TYPE_DOUBLE)

#define SHELL_V_ARRAY_SIZE 8
struct shell_variable
{
	const char *shell_v_name;
	Shell_V_Type shell_v_type;
	void *data_ptr;
	struct singly_list_head list;
};

unsigned int shell_v_hash(const char *name);
void shell_v_display(struct shell_variable *shell_variable_ptr);
void shell_v_display_addr(struct shell_variable *shell_variable_ptr);
void shell_v_write(struct shell_variable *shell_variable_ptr,const char *input_buffer_ptr);
void shell_insert_variable(const char *name,void *ptr,Shell_V_Type type);
struct shell_variable *find_in_variable_array(const char *name);


void shell_debug_info(int argc, char const *argv[]);
void insert_break_point(const char* file_name,unsigned line,const char* function_name);
void shell_debug_next(int argc, char const *argv[]);
void shell_debug_run(int argc, char const *argv[]);
void shell_debug_stop(int argc, char const *argv[]);

#endif

#endif
