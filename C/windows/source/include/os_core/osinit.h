#ifndef _OSINIT_H
#define _OSINIT_H

#define __INIT  __attribute__((section(".INIT.TEXT"))) 

#ifndef TICK_PER_SEC
	#define TICK_PER_SEC 100
#endif
#define HZ TICK_PER_SEC

void os_start(void);
void OSStartHighRdy(void);

#endif
