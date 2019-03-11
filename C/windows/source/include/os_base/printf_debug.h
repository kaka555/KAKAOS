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
        rt_kprintf (format, ##__VA_ARGS__);                                   \
}                                                                             \
while (0)

#endif
