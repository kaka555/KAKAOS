/*
normal task; basic function
 */
#include <user.h>
#include <osinit.h>
#include <os_delay.h>


extern volatile TCB *OSTCBCurPtr;
extern volatile TCB *OSTCBHighRdyPtr;

void three(void *para)
{
	ka_printf("go into task three\n");
	ka_printf("get out of task three\n");
}
void four(void *para)
{
	while (1)
	{
		ka_printf("four\n");
		suspend();
	}
}

void five(void *para)
{
	int i, j;
	while (1)
	{
		//ka_printf("55\n");
		for (i = 0; i < 5000; ++i)
		{
			for (j = 0; j < 1000; ++j)
			{
				;
			}
		}
	}
}

void six(void *para)
{
	int i, j;
	while (1)
	{
		ka_printf("six\n");
		for (i = 0; i < 5000; ++i)
		{
			for (j = 0; j < 1000; ++j)
			{
				;
			}
		}

	}
}
