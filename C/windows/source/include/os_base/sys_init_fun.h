#ifndef _SYS_INIT_FUN_H
#define _SYS_INIT_FUN_H

#define __INIT  __attribute__((section(".INIT.TEXT"))) 

typedef void (*init_fun_type)(void);

struct init_fun
{
	init_fun_type fun;
};

#define INIT_FUN(function)	\
const struct init_fun __init_fun_##function __attribute__((section(".INIT_FUN"))) = \
{	\
	.fun = function,\
}

#endif
