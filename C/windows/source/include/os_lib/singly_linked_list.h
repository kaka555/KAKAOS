#ifndef _SINGLY_LINKED_LIST_H
#define _SINGLY_LINKED_LIST_H

struct singly_list_head{
	struct singly_list_head *next;
};

#define SINGLY_LIST_HEAD_INIT(name) { &(name) }

#define SINGLY_LIST_HEAD(name) \
    struct singly_list_head name = SINGLY_LIST_HEAD_INIT(name)

static inline void INIT_SINGLY_LIST_HEAD(struct singly_list_head *list)
{
	list->next = list;
}

static inline void singly_list_add(struct singly_list_head *new,struct singly_list_head *head)
{
	new->next = head->next;
	head->next = new;
}

static inline void _singly_list_del_next(struct singly_list_head *pre)
{
	pre->next = pre->next->next;
}

static inline int singly_list_empty(struct singly_list_head *head)
{
	return head->next == head;
}

#define singly_ka_offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define singly_list_for_each(pos,head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

#define singly_list_for_each_del(pos, pre,head) \
    for (pos = (head)->next,pre = head; pos != (head); pre = pos,pos = pos->next)

#define singly_list_entry(ptr, type, member) ({          \
    const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
    (type *)( (char *)__mptr - singly_ka_offsetof(type,member) );})

#define singly_list_for_each_entry(pos, head, member)              \
    for (pos = singly_list_entry((head)->next, typeof(*pos), member);  \
         &pos->member != (head);    \
         pos = singly_list_entry(pos->member.next, typeof(*pos), member))

#define singly_list_for_each_entry_safe(pos, n, head, member)          \
    for (pos = singly_list_entry((head)->next, typeof(*pos), member),  \
        n = singly_list_entry(pos->member.next, typeof(*pos), member); \
         &pos->member != (head);                    \
         pos = n, n = singly_list_entry(n->member.next, typeof(*n), member))

#define singly_list_for_each_entry_safe_n(pos, n, head, member)          \
    for (pos = singly_list_entry((head)->next, typeof(*pos), member),  \
        n = singly_list_entry(pos->member.next, typeof(*pos), member); \
         &n->member != (head);                    \
         pos = n, n = singly_list_entry(n->member.next, typeof(*n), member))

void singly_list_del(
	struct singly_list_head *head,
	struct singly_list_head *entity,
	void (*copy_data)(struct singly_list_head *from,struct singly_list_head *to));

#define singly_list_del_safe(head,entity) 				singly_list_del(head,entity,NULL)
#define singly_list_del_quick(head,entity,copy_data) 	singly_list_del(head,entity,copy_data)

#endif
