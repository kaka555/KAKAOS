#ifndef _TIME_H
#define _TIME_H

#include <kakaosstdint.h>
#include <ka_configuration.h>

UINT64 get_tick(void);

#if CONFIG_TIME_EN

#define   OS_YEAR  2018
#define  OS_MONTH    10
#define   OS_DATE     5
#define   OS_HOUR    11
#define OS_MINUTE    22
#define OS_SECOND     0
#define    OS_DAY     5

struct time{
	UINT8 second; //0-59
	UINT8 minute; //0-59
	UINT8   hour; //0-23
	UINT8   date; //1-31
	UINT8  month; //1-12
	UINT8    day; //1-7
	UINT16  year; // >2018
};

void __init_system_time(void);
void system_time_increase(void);
void system_time_display(void);
int  set_time(struct time *time_ptr);

#endif 

#endif	
