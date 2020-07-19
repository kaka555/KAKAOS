// mm test
#include <user.h>
#include <osinit.h>
#include <os_delay.h>

extern volatile TCB *OSTCBCurPtr;
extern volatile TCB *OSTCBHighRdyPtr;

void six(void *para)
{

}
void five(void *para)
{
	while (1)
	{
		ka_printf("five\n");
		sleep(20);
	}
}
void four(void *para)
{
	while (1)
	{
		ka_printf("four\n");
		sleep(30);
	}
}
void three(void *para)
{
	void *ptr[64];
	int i;
	for (i = 0; i < 64; ++i)
	{
		ptr[i] = ka_malloc(sizeof(int));
		ka_printf("%d get ptr address is %x \n", i, (int)ptr[i]);
		ka_printf("value %d is %x\n", i, *(int *)ptr[i]);
		*(int *)ptr[i] = 66;
	}

	void *kaka = ka_malloc(48);
	ka_printf("ptr--kaka get ptr address is %x \n", (int)kaka);

	//sleep(300);

	ka_free(kaka);
	ka_printf("free complete\n");
	ka_free(ptr[24]);
	ka_printf("free complete\n");

	ka_free(ptr[31]);
	ka_printf("free complete\n");
	ka_free(ptr[7]);

	ka_free(ptr[62]);

	ka_printf("free complete\n");

	kaka = ka_malloc(120);
	ka_printf("ptr--kaka get ptr address is %x \n", (int)kaka);

	//sleep(300);

	ka_free(kaka);
	ka_printf("free kaka complete\n");

	while (1)
	{
	}
}
