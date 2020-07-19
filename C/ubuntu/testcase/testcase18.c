/* test exec() */
#include <user.h>
#include <osinit.h>
#include <os_delay.h>

extern volatile TCB *OSTCBCurPtr;
extern volatile TCB *OSTCBHighRdyPtr;

void five(void *para)
{
	while (1)
	{
		float b = 4.4;
		float a = 3.3;
		ka_printf("b is %f\n", b * a);
		ka_printf("now come into function five()\n");
		sleep(10 * HZ);
	}
}

void kaka(void *para)
{
	ka_printf("now come into function kaka()\n");
	ka_printf("para is %u\n", (unsigned int)para);
	if (0 != task_creat_ready(256, 5, 5, "five", five, NULL, NULL))
	{
		ka_printf("os_init_fail...stop booting...\n");
		while (1);
	}
	while (1)
	{
		float b = 4.5;
		float a = 3.3;
		ka_printf("a is %f\n", a * b);
		sleep(5 * HZ);
		ka_printf("now come into function kaka()\n");
	}
}

#include <printf_debug.h>
void three(void *para)
{
	ka_printf("now come into function three()\n");
	INSERT_BREAK_POINT();
	exec("kaka", kaka, (void *)5);
	ka_printf("function three() end!!!\n");
	panic("should not go here\n");

}
