#ifndef _MYASSERT_H
#define _MYASSERT_H

#include <ka_configuration.h>
#include <myMicroLIB.h>
#if CONFIG_ASSERT_DEBUG
	void _ASSERT(char*,unsigned);
#define ASSERT(f,format, ...) \
	if(f)\
		 ;\
	else\
	do{\
		ka_printf (format, ##__VA_ARGS__);\
		_ASSERT(__FILE__,__LINE__);\
	}while(0)

#define ASSERT_INPUT "input parameter check\n"
#define ASSERT_BAD_EXE_LOCATION "should not go here\n"
#define ASSERT_PARA_AFFIRM "data consistency affirm\n"
#else
	#define ASSERT(f,format,...)     
#endif

#endif
