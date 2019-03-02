


#include <myMicroLIB.h>
#include <os_schedule.h>
#include <osinit.h>

extern void led(void);

int a = 5;
int main()
{
	
	ka_printf("hello KAKAOS\n"); 
	sleep(5*HZ);
	ka_printf("a is %d\n",a); 
	a = 66;

	led();
        //while(1);
	return 0;
}






