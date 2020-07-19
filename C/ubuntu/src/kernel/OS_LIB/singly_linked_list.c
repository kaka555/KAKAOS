#include <singly_linked_list.h>
#include <kakaosstdint.h>

/* this file imitate the linux struct node, realizes a singly linked list with
 singly_linked_list.h */

void singly_list_del(
    struct singly_list_head *head,
    struct singly_list_head *entity)
{
	struct singly_list_head **cur = &head->next;
	while (*cur != head)
	{
		struct singly_list_head *entry = *cur;
		if (entity == entry)
		{
			*cur = entry->next;
			return ;
		}
		cur = &entry->next;
	}
}

void singly_list_add_tail(struct singly_list_head *new, struct singly_list_head *head)
{
	struct singly_list_head **cur = &head->next;
	while (*cur != head)
	{
		cur = &(*cur)->next;
	}
	singly_list_add(new, *cur);
}
