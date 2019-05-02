#ifndef _SYS_INIT_FUN_H
#define _SYS_INIT_FUN_H

#include <osbase.h>

#define __INIT  __section(".INIT.TEXT")

#define __RAMFUNC __attribute__ ((long_call, section (".ramfunctions")))

typedef void (*init_fun_type)(void);

struct init_fun
{
	init_fun_type fun;
};

#define INIT_FUN(function,level)	\
const struct init_fun __init_fun_##function __section(".INIT_FUN" #level) = \
{	\
	.fun = function,\
}

#endif
