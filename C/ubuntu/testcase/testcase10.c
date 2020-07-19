//test new memory management
#include <user.h>
#include <osinit.h>
#include <os_delay.h>

extern volatile TCB *OSTCBCurPtr;
extern volatile TCB *OSTCBHighRdyPtr;

void three(void *para)
{
	void *ptr[31];
	int i;
	_get_os_buddy_ptr_head();
	ka_printf("%p\n", _alloc_power2_page());
	ka_printf("%p\n", _alloc_power3_page());
	ka_printf("%p\n", _alloc_power4_page());
	ka_printf("%p\n", _alloc_power6_page());
	while (1)
	{
		for (i = 0; i < 31; ++i)
		{
			ptr[i] = ka_malloc(500);
			ka_printf("ptr[%d] is %p\n", i, ptr[i]);
		}
		sleep(200 * HZ);
		for (i = 0; i < 31; ++i)
		{
			ka_free(ptr[i]);
		}
		ka_printf("free complete\n");
		sleep(500 * HZ);
	}
}
