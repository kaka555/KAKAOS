#ifndef _USER_DEBUG
#define _USER_DEBUG
#include <ka_configuration.h>

#if CONFIG_PRINTF_DEBUG
	#define PRINTF(format, ...) ka_printf (format, ##__VA_ARGS__)
#else
	#define PRINTF(format, ...)  
#endif

#endif
