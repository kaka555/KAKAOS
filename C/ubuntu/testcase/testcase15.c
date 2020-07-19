//test malloc
#include <user.h>
#include <osinit.h>
#include <os_delay.h>

extern volatile TCB *OSTCBCurPtr;
extern volatile TCB *OSTCBHighRdyPtr;

void three(void *para)
{
	void *ptr[10];
	unsigned int i;
	ka_printf("going to task three\n");
	for (i = 0; i < 10; ++i)
	{
		ptr[i] = ka_malloc(700);
		ka_printf("i is %u,address is %p\n", i, ptr[i]);
	}
	INSERT_BREAK_POINT();
	ka_printf("release address\n");
	for (i = 0; i < 10; ++i)
	{
		ka_free(ptr[i]);
		INSERT_BREAK_POINT();
	}
	INSERT_BREAK_POINT();
	ka_printf("end\n");
}
