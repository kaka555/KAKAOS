#ifndef _OS_TIMER_H
#define _OS_TIMER_H

#include <ka_configuration.h>
#include <kakaosstdint.h>

#if CONFIG_TIMER_EN

#define TIME_FIRST_SMALLER_THAN_SECOND(first,second) ((INT64)(first)-(INT64)(second) < 0)
#define TIME_FIRST_BIGGER_THAN_SECOND(first,second)  ((INT64)(first)-(INT64)(second) > 0)

#define CREAT_REST_NUM(x)	((x) | 0x80000000)
#define INIT_REST_NUM(x)	(x)
#define GET_REST_NUM(x) 	((x)&0x7f)
#define TIMER_IS_CREAT(ptr)	(((ptr)->rest_num) & 0x80000000)

typedef enum Timer_Type {
	TIMER_ONE_TIME = 0,
	TIMER_PERIODIC = 1,
	TIMER_TIME     = 2
}TIMER_TYPE;

typedef enum Timer_State {
	DISABLE = 0,
	ENABLE = 1
}TIMER_STATE;

struct timer
{
	TIMER_TYPE  type;
	TIMER_STATE state;
	unsigned int rest_num;
	char *timer_name;
	void (*fun)(void *para);
	void *para;
	unsigned int period; /*ticks*/
	unsigned int heap_position_index;
	UINT64 wake_time;
};

int timer_init(
	struct timer *timer_ptr,
	TIMER_TYPE type,
	char *name,
	void (*fun)(void *para),
	void *para,
	unsigned int period,
	unsigned int num);

int timer_create(
	struct timer **timer_ptr,
	TIMER_TYPE type,
	char *name,
	void (*fun)(void *para),
	void *para,
	unsigned int period,
	unsigned int num);

int _timer_enable(struct timer *timer_ptr);
int timer_enable(struct timer *timer_ptr);
int _timer_disable(struct timer *timer_ptr);
int timer_disable(struct timer *timer_ptr);
int timer_delete(struct timer *timer_ptr);
int _get_timer_heap_top(struct timer **timer_ptr);

#endif

#endif
