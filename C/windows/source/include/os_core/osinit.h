#ifndef _OSINIT_H
#define _OSINIT_H

#ifndef TICK_PER_SEC
	#define TICK_PER_SEC 100
#endif
#define HZ TICK_PER_SEC

void OSStartHighRdy(void);

#endif
