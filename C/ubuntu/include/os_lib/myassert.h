#ifndef _MYASSERT_H
#define _MYASSERT_H

#include <ka_configuration.h>
#if CONFIG_ASSERT_DEBUG
	void _ASSERT(char*,unsigned);
#define ASSERT(f) \
	if(f)\
		 ;\
	else\
	_ASSERT(__FILE__,__LINE__)
#else
	#define ASSERT(f)     
#endif

#endif
