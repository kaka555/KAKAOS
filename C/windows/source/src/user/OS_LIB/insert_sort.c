#include <insert_sort.h>
#include <double_linked_list.h>
#include <myassert.h>

#define TIME_FIRST_SMALLER_THAN_SECOND(first,second) ((INT64)(first)-(INT64)(second)<0)
#define TIME_FIRST_NOT_BIGGER_THAN_SECOND(first,second) ((INT64)(second)-(INT64)(first)>=0)

inline void IL_init(IL *IL_ptr,COMPARE_TYPE prio)
{
	INIT_LIST_HEAD(&IL_ptr->insert);
	IL_ptr->prio = prio;
}

IL *get_next_IL(IL *IL_ptr,struct list_head *head_ptr)
{
	ASSERT(NULL != IL_ptr);
	struct list_head *list_buffer;
	list_buffer = &IL_ptr->insert;
	list_buffer = list_buffer->next;
	if(list_buffer == head_ptr)
	{
		return NULL;
	}
	return (list_entry(list_buffer,IL,insert));
}

void 
insert_chain(
struct list_head *head,
IL *insert_node
)
{
	struct list_head *pos,*next;
  ASSERT(NULL != head);
  ASSERT(NULL != insert_node);

	if(list_empty(head))
	{
		list_add(&insert_node->insert,head);
		return;
	}
	
  COMPARE_TYPE prio = insert_node->prio;
	IL *IL_ptr;
	list_for_each_safe(pos,next,head)
	{
		IL_ptr = list_entry(pos,IL,insert);
		//if(prio < IL_ptr->prio)
		if(TIME_FIRST_SMALLER_THAN_SECOND(prio,IL_ptr->prio))
		{
			list_add_tail(&insert_node->insert,pos);
			return ;
		}

	}
	list_add_tail(&insert_node->insert,head);
	return ;
}

IL *find_in_chain(struct list_head *head,COMPARE_TYPE prio) // polymorphic
{
	struct list_head *pos, *next;
	IL *IL_ptr;
	list_for_each_safe(pos, next, head)
	{
      IL_ptr = list_entry(pos,IL,insert);
      if(prio == IL_ptr->prio)
      {
      	return IL_ptr;
      }
	}
	return NULL;
}

IL *delete_from_chain(struct list_head *head,COMPARE_TYPE prio)
{
	struct list_head *pos, *next;
	IL *IL_ptr;
	list_for_each_safe(pos, next, head)
	{
      IL_ptr = list_entry(pos,IL,insert);
      if(prio == IL_ptr->prio)
      {
      	list_del(pos);
      	return IL_ptr;
      }
	}
	return NULL;
}

IL *delete_first_in_chain(struct list_head *head)
{
	struct list_head *pos;
	IL *IL_ptr;
	//ASSERT(!list_empty(head));
	pos = head->next;
	if(pos == head)
	{
		return NULL;
	}
		
	IL_ptr = list_entry(pos,IL,insert);
	list_del(pos);
	return IL_ptr;
}

IL *find_first_bigger_IL(struct list_head *head,unsigned int prio)// polymorphic
{
	struct list_head *pos, *next;
	IL *IL_ptr;
	list_for_each_safe(pos, next, head)
	{
      IL_ptr = list_entry(pos,IL,insert);
      if(prio <= IL_ptr->prio)
      {
      	return IL_ptr;
      }
	}
	return NULL;
}
