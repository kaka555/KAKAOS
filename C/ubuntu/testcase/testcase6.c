// test int sleep(unsigned int)
#include <user.h>
#include <osinit.h>
#include <os_delay.h>

extern volatile TCB *OSTCBCurPtr;
extern volatile TCB *OSTCBHighRdyPtr;

#include <myassert.h>
void three(void *para)
{

	TCB *TCB_ptr4, *TCB_ptr5;
	if (0 != task_creat_ready(256, 5, 5, "five", five, NULL, &TCB_ptr5))
	{
		ka_printf("os_init_fail...stop booting...\n");
		while (1);
	}
	if (0 != task_creat_ready(256, 5, 5, "four", four, NULL, &TCB_ptr4))
	{
		ka_printf("os_init_fail...stop booting...\n");
		while (1);
	}
	ka_printf("now task three going to sleep\n");
	int ret = sleep(3 * HZ);
	ka_printf("task three ret is %d\n", ret);
	/*
		ka_printf("task three ret is %d\n",ret);
		if(0 != _remove_from_delay_heap(TCB_ptr5))
		{
			ka_printf("fatal error\n");
		}
		if(0 != _remove_from_delay_heap(TCB_ptr4))
		{
			ka_printf("fatal error\n");
		}
		suspend();
		while(1);
	*/
}

void five(void *para)
{
	int ret = sleep(5 * HZ);
	ka_printf("task five ret is %d\n", ret);
	suspend();
	while (1);
}
void four(void *para)
{
	ka_printf("three is %p\n"
	          "four is %p\n"
	          "five is %p\n", three, four, five);
	int ret = sleep(4 * HZ);
	ka_printf("task four ret is %d\n", ret);
	suspend();
	while (1);
}
