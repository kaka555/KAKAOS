#include <singly_linked_list.h>
#include <kakaosstdint.h>

static void _singly_list_del(
	struct singly_list_head *head,
	const struct singly_list_head *entity)
{
	struct singly_list_head *pos,*pre;
	singly_list_for_each_del(pos,pre,head)
	{
		if(pos == entity)
		{
			_singly_list_del_next(pre);
			return ;
		}
	}
}

void singly_list_del(
	struct singly_list_head *head,
	struct singly_list_head *entity,
	void (*copy_data)(struct singly_list_head *from,struct singly_list_head *to))
{
	if((NULL != copy_data) && (head != entity->next))
	{
		copy_data(entity->next,entity);
		_singly_list_del_next(entity);
	}
	else
	{
		_singly_list_del(head,entity);
	}
}
