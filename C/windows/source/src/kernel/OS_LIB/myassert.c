#include <myassert.h>
#include <myMicroLIB.h>

extern void HardFault_Handler(void);
void my_abort(void)
{
	ka_printf("os stop\n");
	while(1);
}
	

void _ASSERT(char* strFile,unsigned uLine)
{
	ka_printf("\nAssertion failed: %s,line %u\r\n",strFile,uLine);
	my_abort();
}
