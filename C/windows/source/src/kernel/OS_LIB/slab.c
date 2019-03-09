#include <slab.h>
#include <myassert.h>


void insert_into_cache_chain(
	struct list_head *head,
	struct kmem_cache *kmem_cache_ptr,
	int block_size/*bytes*/
	)
{
	IL_init(&kmem_cache_ptr->kmem_cache_insert_chain,block_size);
	insert_chain(head,&kmem_cache_ptr->kmem_cache_insert_chain);
	INIT_LIST_HEAD(&kmem_cache_ptr->slabs_full);
	INIT_LIST_HEAD(&kmem_cache_ptr->slabs_partial);
	INIT_LIST_HEAD(&kmem_cache_ptr->slabs_empty);
}

void load_slab(
	void *start_ptr,
	void *end_ptr,
	unsigned int   block_size,
	struct list_head *slab_chain_ptr
	)
{
#if CONFIG_ASSERT_DEBUG
	int i = 0;
#endif
	struct list_head *list_head_ptr;
	void *boundary_ptr;
	struct slab *slab_ptr = (struct slab *)start_ptr;
	list_add(&slab_ptr->slab_chain,slab_chain_ptr);
	slab_ptr->start_ptr = start_ptr;
	slab_ptr->end_ptr   = end_ptr;
	slab_ptr->block_size = block_size;
	unsigned int room_size = (unsigned int)((unsigned int)end_ptr - (unsigned int)start_ptr);
	slab_ptr->full_block_num = (room_size - sizeof(struct slab)) / block_size;
	slab_ptr->current_block_num = slab_ptr->full_block_num;
	INIT_LIST_HEAD(&slab_ptr->block_head);
	list_head_ptr = (struct list_head *)((int)end_ptr - block_size);
	boundary_ptr = (void *)((unsigned int)start_ptr + sizeof(struct slab));
	while((unsigned int)list_head_ptr >= (unsigned int)boundary_ptr)
	{
		list_add(list_head_ptr,&slab_ptr->block_head);
		list_head_ptr = (struct list_head *)((unsigned int)list_head_ptr - block_size);
		ASSERT(++i <= slab_ptr->full_block_num);
	}
}


