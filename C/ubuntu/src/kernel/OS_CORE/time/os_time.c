#include <os_time.h>
#include <os_cpu.h>
#include <myMicroLIB.h>
#include <myassert.h>
#include <os_error.h>
#include <export.h>
#include <sys_init_fun.h>

#if CONFIG_TIME_EN

static struct time sys_time;

static const UINT8 month_date_table[] = {
	31, /*January*/
	28, /*Feburary*/
	31, /*March*/
	30, /*April*/
	31, /*May*/
	30, /*June*/
	31, /*July*/
	31, /*August*/
	30, /*September*/
	31, /*October*/
	30, /*November*/
	31  /*December*/
};

static void __INIT __init_system_time(void)
{
	sys_time.date    =  OS_DATE;
	sys_time.hour    =  OS_HOUR;
	sys_time.minute  = 	OS_MINUTE;
	sys_time.month   =  OS_MONTH;
	sys_time.second  = 	OS_SECOND;
	sys_time.year    =  OS_YEAR;
	sys_time.day     =  OS_DAY;
}
INIT_FUN(__init_system_time,1);

/**
 * This is a system function
 * @Author      kaka
 * @DateTime    2018-10-12
 * @description : this function can only be used by os, be used in tick interrupt
 */
void 
_system_time_increase(void)
{
	++sys_time.second;
	if(60 == sys_time.second)
	{
		sys_time.second = 0;
		++sys_time.minute;
		if(60 == sys_time.minute)
		{
			sys_time.minute = 0;
		  	++sys_time.hour;
			if(24 == sys_time.hour)
			{
				sys_time.hour = 0;
				++sys_time.date;
				++sys_time.day;
				if(month_date_table[sys_time.month-1] < sys_time.date)
				{
					sys_time.date = 1;
					++sys_time.month;
					if(sys_time.month == 13)
					{
						sys_time.month = 1;
						++sys_time.year;
					}
				}
				if(8 == sys_time.day)
				{
					sys_time.day = 1;
				}
			}
		}
	}
}

void _system_time_display(void)
{
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	ka_printf("%d年%d月%d日--%d时%d分%d秒--星期%d\r\n",sys_time.year,
	sys_time.month,sys_time.date,sys_time.hour,sys_time.minute,sys_time.second,sys_time.day);
	CPU_CRITICAL_EXIT();
}

void system_time_display(void)
{
	_system_time_display();
}
EXPORT_SYMBOL(system_time_display);

int _set_time(struct time *time_ptr)
{
	ASSERT(NULL != time_ptr,ASSERT_INPUT);
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	sys_time.date    =   time_ptr->date;
	sys_time.hour    =   time_ptr->hour;
	sys_time.minute  = 	 time_ptr->minute;
	sys_time.month   =   time_ptr->month;
	sys_time.second  =   time_ptr->second;
	sys_time.year    =   time_ptr->year;
	sys_time.day     =   time_ptr->day;
	CPU_CRITICAL_EXIT();
	return FUN_EXECUTE_SUCCESSFULLY;
}

int set_time(struct time *time_ptr)
{
	if(NULL == time_ptr)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(set_time,time_ptr);
		return -ERROR_NULL_INPUT_PTR;
	}
	if((time_ptr->second >= 60) || (time_ptr->month >= 13)
			|| (time_ptr->hour >= 24) || (time_ptr->day > 7)
			|| (time_ptr->date > month_date_table[time_ptr->month]))
	{
		return -ERROR_USELESS_INPUT;
	}
	return _set_time(time_ptr);
}
EXPORT_SYMBOL(set_time);

#endif	
