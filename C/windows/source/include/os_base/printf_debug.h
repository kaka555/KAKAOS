#ifndef _USER_DEBUG
#define _USER_DEBUG
#include <ka_configuration.h>

#if CONFIG_PRINTF_DEBUG
	#define PRINTF(format, ...) ka_printf (format, ##__VA_ARGS__)
#else
	#define PRINTF(format, ...)  
#endif

#define KA_DEBUG_LOG(type, format, ...)                                       \
do                                                                            \
{                                                                             \
    if (type)                                                                 \
        ka_printf (format, ##__VA_ARGS__);                                   \
}                                                                             \
while(0)

#define DEBUG_TYPE_VFS 			1
#define DEBUG_TYPE_SHELL_TAB 	1

#endif
