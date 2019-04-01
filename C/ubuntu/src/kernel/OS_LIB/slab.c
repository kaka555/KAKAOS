#include <slab.h>
#include <myassert.h>


void insert_into_cache_chain(
	struct singly_list_head *head,
	struct kmem_cache *kmem_cache_ptr,
	unsigned int block_size/*bytes*/
	)
{
	struct kmem_cache *pos,*n;
	INIT_SINGLY_LIST_HEAD(&kmem_cache_ptr->node);
	kmem_cache_ptr->kmem_cache_slab_size = block_size;
	pos = singly_list_entry(head->next,struct kmem_cache,node);
	if((singly_list_empty(head)) || 
			(singly_list_entry(head->next,struct kmem_cache,node)->kmem_cache_slab_size > block_size))
	{
		singly_list_add(&kmem_cache_ptr->node,head);
	}
	else
	{
		singly_list_for_each_entry_safe_n(pos,n,head,node)
		{
			if(pos->kmem_cache_slab_size == block_size)
			{
				goto outside ;
			}
			if(n->kmem_cache_slab_size > block_size)
			{
				singly_list_add(&kmem_cache_ptr->node,&pos->node);
				goto outside ;
			}
		}
		singly_list_add(&kmem_cache_ptr->node,&pos->node);
	}
outside:
	INIT_LIST_HEAD(&kmem_cache_ptr->slabs_full);
	INIT_LIST_HEAD(&kmem_cache_ptr->slabs_partial);
	INIT_LIST_HEAD(&kmem_cache_ptr->slabs_empty);
}

struct kmem_cache *find_first_bigger_cache(struct singly_list_head *head,unsigned int size)/* polymorphic*/
{
	struct kmem_cache *kmem_cache_ptr;
	singly_list_for_each_entry(kmem_cache_ptr, head ,node)
	{
      if(size <= kmem_cache_ptr->kmem_cache_slab_size)
      {
      	return kmem_cache_ptr;
      }
	}
	return NULL;
}

unsigned int load_slab(
	void *start_ptr,
	void *end_ptr,
	unsigned int   block_size,
	struct list_head *slab_chain_ptr
	)
{
#if CONFIG_ASSERT_DEBUG
	unsigned int i = 0;
#endif
	struct list_head *list_head_ptr;
	void *boundary_ptr;
	struct slab *slab_ptr = (struct slab *)start_ptr;
	list_add(&slab_ptr->slab_chain,slab_chain_ptr);
	slab_ptr->start_ptr = start_ptr;
	slab_ptr->end_ptr   = end_ptr;
	unsigned int room_size = (unsigned int)((unsigned int)end_ptr - (unsigned int)start_ptr - sizeof(struct slab));
	slab_ptr->full_block_num = room_size / block_size;
	slab_ptr->current_block_num = slab_ptr->full_block_num;
	unsigned int remaind = room_size - block_size * slab_ptr->full_block_num;
	if(remaind > slab_ptr->full_block_num)
	{
		unsigned int add_size = remaind / slab_ptr->full_block_num;
		/* assert that add_size is divisible by 4 */
		add_size &= ~0x03;
		block_size += add_size;
	}
	slab_ptr->block_size = block_size;
	INIT_LIST_HEAD(&slab_ptr->block_head);
	list_head_ptr = (struct list_head *)((unsigned int)end_ptr - block_size);
	boundary_ptr = (void *)((unsigned int)start_ptr + sizeof(struct slab));
	while((unsigned int)list_head_ptr >= (unsigned int)boundary_ptr)
	{
		ASSERT((unsigned int)list_head_ptr >= (unsigned int)start_ptr + sizeof(struct slab));
		list_add(list_head_ptr,&slab_ptr->block_head);
		list_head_ptr = (struct list_head *)((unsigned int)list_head_ptr - block_size);
		ASSERT(++i <= slab_ptr->full_block_num);
	}
	return block_size;
}


