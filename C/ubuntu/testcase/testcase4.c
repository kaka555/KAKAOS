/* test breakpoint */
#include <user.h>
#include <osinit.h>
#include <os_delay.h>

extern volatile TCB *OSTCBCurPtr;
extern volatile TCB *OSTCBHighRdyPtr;

void three(void *para)
{
	int i = 5;
	int ia = 7;
	double kaka = 6.66;
	shell_insert_variable_INT("i", &i);
	shell_insert_variable_INT("ia", &ia);
	shell_insert_variable_FLOAT("kaka", &kaka);
	ka_printf("address of i is 0x%p\n", &i);
	ka_printf("address of ia is 0x%p\n", &ia);
	ka_printf("address of kaka is 0x%p\n", &kaka);
	ka_printf("i is %d\n", i);
	ka_printf("ia is %d\n", ia);
	ka_printf("kaka is %f\n", kaka);
	INSERT_BREAK_POINT();
	ka_printf("i is %d\n", i);
	ka_printf("ia is %d\n", ia);
	ka_printf("kaka is %f\n", kaka);
	INSERT_BREAK_POINT();
	ka_printf("i is %d\n", i);
	ka_printf("ia is %d\n", ia);
	ka_printf("kaka is %f\n", kaka);
	while (1)
	{
		ka_printf("three\n");
		sleep(10);
	}
}
