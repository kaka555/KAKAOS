#include "os_cpu.h"
#include "osinit.h"

int main()
{
	CPU_IntDis();
	os_start();
	while(1)
	{
		;//should never go here;
	}
}
