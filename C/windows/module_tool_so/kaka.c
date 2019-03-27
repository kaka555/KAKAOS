#define module_init(initfn)                 \
    int init_module(void) __attribute__((alias(#initfn)))

#define module_exit(initfn)                 \
    int exit_module(void) __attribute__((alias(#initfn)))

#include <myMicroLIB.h>
#include <os_schedule.h>
#include <osinit.h>

extern void led(void);

int a = 5;

static int init(void)
{
	ka_printf("init fun\n");
	a = 6;
	return 0;
}

module_init(init);


static void exit(void)
{
	ka_printf("byebye module\n");
}


module_exit(exit);

int main()
{
	
	ka_printf("hello KAKAOS\n"); 
	sleep(5*HZ);
	ka_printf("a is %d\n",a); 
	a = 66;

	led();
	return 0;
}






