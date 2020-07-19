/* test mem pool */
#include <user.h>
#include <osinit.h>
#include <os_delay.h>

extern volatile TCB *OSTCBCurPtr;
extern volatile TCB *OSTCBHighRdyPtr;

void three(void *para)
{
	mp *mp_ptr;
	int error = create_mp("kaka", 11, 100, &mp_ptr);
	if (error < 0)
	{
		ka_printf("error is %d\n", error);
		return ;
	}
	void *ptr[30];
	unsigned int i;
	for (i = 0; i < 30; ++i)
	{
		ptr[i] = mp_alloc(mp_ptr, 3 * HZ);
		ka_printf("%u\n", i);
	}
	for (i = 0; i < 30; ++i)
	{
		ka_printf("ptr[%u] is %p\n", i, ptr[i]);
	}
	sleep(10 * HZ);
	ka_printf("release pools\n");
	for (i = 0; i < 30; ++i)
	{
		mp_free(ptr[i]);
	}
	ka_printf("complete\n");
	error = create_mp("qq", 11, 100, &mp_ptr);
}
