#ifndef _INSERT_SORT_H
#define _INSERT_SORT_H

#include "kakaosstdint.h"
#include "double_linked_list.h"

typedef UINT32 COMPARE_TYPE;

typedef struct insert_list{
	struct list_head insert;
	COMPARE_TYPE prio;
}IL;

void IL_init(IL *IL_ptr,COMPARE_TYPE prio);
void insert_chain(struct list_head *head,IL *insert_node);
IL *delete_from_chain(struct list_head *head,COMPARE_TYPE prio);/*there is a bug in
this function that if two IL have the same prio,it may delete a wrong member*/
IL *find_in_chain(struct list_head *head,COMPARE_TYPE prio);
IL *delete_first_in_chain(struct list_head *head);
IL *find_first_bigger_IL(struct list_head *head,unsigned int prio);
IL *get_next_IL(IL *IL_ptr,struct list_head *list_ptr);
#endif
