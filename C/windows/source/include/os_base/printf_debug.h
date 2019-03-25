#ifndef _USER_DEBUG
#define _USER_DEBUG
#include <ka_configuration.h>

#if CONFIG_PRINTF_DEBUG
	#define PRINTF(format, ...) ka_printf (format, ##__VA_ARGS__)
#else
	#define PRINTF(format, ...)  
#endif

#if CONFIG_PRINTF_DEBUG
	#define KA_WARN(type, format, ...)                                       \
	do                                                                            \
	{                                                                             \
	    if (type)                                                                 \
	        ka_printf (format, ##__VA_ARGS__);                                   \
	}                                                                             \
	while(0)

	#define DEBUG_TYPE_VFS 			1
	#define DEBUG_TYPE_SHELL_TAB 	0
	#define DEBUG_TYPE_MODULE 		1
	#define DEBUG_TYPE_VECTOR 		1	
	#define DEBUG_TYPE_SHELL 		0

	#endif
#else
	#define KA_WARN(type, format, ...)  
#endif
		