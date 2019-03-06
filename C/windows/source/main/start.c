#include "os_cpu.h"
#include "osinit.h"

void start_kernel()
{
	CPU_IntDis();
	os_start();
	while(1)
	{
		;//should never go here;
	}
}
