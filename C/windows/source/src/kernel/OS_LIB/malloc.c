#include <myMicroLIB.h>
#include <slab.h>
#include <buddy.h>
#include <myassert.h>
#include <os_cpu.h>
#include <TCB.h>

extern int in_buddy_range(void *ptr);
extern void _case_slab_free_buddy(void *ptr,void *end_ptr);

static struct singly_list_head cache_chain_head;
static struct kmem_cache kmem_cache16;
static struct kmem_cache kmem_cache32;
static struct kmem_cache kmem_cache_kmem_cache;
static struct kmem_cache kmem_cache_TCB;
struct singly_list_head *const cache_chain_head_ptr = &cache_chain_head;

static struct singly_list_head page_alloc_record_head;

void dis_page_alloc_record(void)
{
	unsigned int i = 0;
	struct page_alloc_record *record_ptr;
	singly_list_for_each_entry(record_ptr,&page_alloc_record_head,list)
	{
		ka_printf("num.%u\n",++i);
		ka_printf("level is %u\n",record_ptr->level);
		ka_printf("address is %p\n",record_ptr->ptr);
	}
}

int add_page_alloc_record(unsigned int level,void *ptr)
{
	struct page_alloc_record *recoed_ptr = (struct page_alloc_record *)ka_malloc(sizeof(struct page_alloc_record));
	if(NULL == recoed_ptr)
	{
		return -1;
	}
	recoed_ptr->level = level;
	recoed_ptr->ptr = ptr;
	singly_list_add(&recoed_ptr->list,&page_alloc_record_head);
	return FUN_EXECUTE_SUCCESSFULLY;
}

static int find_and_remove_record(void *ptr)
{
	struct singly_list_head *pos;
	struct page_alloc_record *record_ptr; 
	singly_list_for_each(pos,&page_alloc_record_head)
	{
		record_ptr = singly_list_entry(pos,struct page_alloc_record,list);
		if(record_ptr->ptr == ptr)
		{
			_case_slab_free_buddy(ptr,(void *)((unsigned int)ptr + ka_pow(2,record_ptr->level-1) * PAGE_SIZE_BYTE));
			singly_list_del_safe(&page_alloc_record_head,pos);
			ka_free(record_ptr);
			return FUN_EXECUTE_SUCCESSFULLY;
		}
	}
	return -1;
}

void init_malloc(void)
{
	INIT_SINGLY_LIST_HEAD(&cache_chain_head);
	insert_into_cache_chain(&cache_chain_head,&kmem_cache_TCB,sizeof(TCB));
	insert_into_cache_chain(&cache_chain_head,&kmem_cache_kmem_cache,sizeof(struct kmem_cache));
	insert_into_cache_chain(&cache_chain_head,&kmem_cache32,32);
	insert_into_cache_chain(&cache_chain_head,&kmem_cache16,16);
	INIT_SINGLY_LIST_HEAD(&page_alloc_record_head);
}
/**
 * This is a system function
 * @Author      kaka
 * @param       the num witch will be get the higest "1"
 * @DateTime    2018-09-26
 * @description :
 * @param       num        [description]
 * @return      the higest "1" position
 */
int get_set_bit_place(unsigned int num)
{
	int n=1;
    if(num==0) return -1;
    if ((num>>16) == 0) {n = n+16; num = num<<16;}
    if ((num>>24) == 0) {n = n+8; num = num<<8;}
    if ((num>>28) == 0) {n = n+4; num = num<<4;}
    if ((num>>30) == 0) {n = n+2; num = num<<2;}
    n = n-(num>>31);
    return 31-n;
}

static void *_case_alloc_buddy(unsigned int level)
{
	struct slab *slab_ptr;
	struct buddy *buddy_ptr = (struct buddy *)_get_os_buddy_ptr_head();
	ASSERT(NULL != buddy_ptr);
	while(NULL != buddy_ptr)
	{
		if(buddy_ptr->info.max_level < level)
		{
			buddy_ptr = (struct buddy *)_get_next_buddy_ptr_head(buddy_ptr);
			continue;
		}
		switch(level)
		{
			case 1 :
				slab_ptr = (struct slab *)_alloc_power1_page();
				if(NULL == slab_ptr)
				{
					buddy_ptr = (struct buddy *)_get_next_buddy_ptr_head(buddy_ptr);
					continue ;
				}
				slab_ptr->end_ptr = (void *)((unsigned int)slab_ptr + 1 * PAGE_SIZE_BYTE);
				break;
			case 2 :
				slab_ptr = (struct slab *)_alloc_power2_page();
				if(NULL == slab_ptr)
				{
					buddy_ptr = (struct buddy *)_get_next_buddy_ptr_head(buddy_ptr);
					continue ;
				}
				slab_ptr->end_ptr = (void *)((unsigned int)slab_ptr + 2 * PAGE_SIZE_BYTE);
				break;
			case 3 :
				slab_ptr = (struct slab *)_alloc_power3_page();
				if(NULL == slab_ptr)
				{
					buddy_ptr = (struct buddy *)_get_next_buddy_ptr_head(buddy_ptr);
					continue ;
				}
				slab_ptr->end_ptr = (void *)((unsigned int)slab_ptr + 4 * PAGE_SIZE_BYTE);
				break;
			case 4 :
				slab_ptr = (struct slab *)_alloc_power4_page();
				if(NULL == slab_ptr)
				{
					buddy_ptr = (struct buddy *)_get_next_buddy_ptr_head(buddy_ptr);
					continue ;
				}
				slab_ptr->end_ptr = (void *)((unsigned int)slab_ptr + 8 * PAGE_SIZE_BYTE);
				break;
			case 5 :
				slab_ptr = (struct slab *)_alloc_power5_page();
				if(NULL == slab_ptr)
				{
					buddy_ptr = (struct buddy *)_get_next_buddy_ptr_head(buddy_ptr);
					continue ;
				}
				slab_ptr->end_ptr = (void *)((unsigned int)slab_ptr + 16 * PAGE_SIZE_BYTE);
				break;
			case 6 :
				slab_ptr = (struct slab *)_alloc_power6_page();
				if(NULL == slab_ptr)
				{
					buddy_ptr = (struct buddy *)_get_next_buddy_ptr_head(buddy_ptr);
					continue ;
				}
				slab_ptr->end_ptr = (void *)((unsigned int)slab_ptr + 32 * PAGE_SIZE_BYTE);
				break;
			case 7 :
				slab_ptr = (struct slab *)_alloc_power7_page();
				if(NULL == slab_ptr)
				{
					buddy_ptr = (struct buddy *)_get_next_buddy_ptr_head(buddy_ptr);
					continue ;
				}
				slab_ptr->end_ptr = (void *)((unsigned int)slab_ptr + 64 * PAGE_SIZE_BYTE);
				break;
			default :
				ka_printf("too big! FATAL ERROR!\n");
				ASSERT(0);
		}
		return slab_ptr;
	}
	ka_printf("no space!!!\n");
	return NULL;
}

void *ka_malloc(unsigned int size)
{
	void *ptr;
	struct kmem_cache *kmem_cache_ptr;
	CPU_SR_ALLOC();
	if(0 == size)
	{
		return NULL;
	}
	CPU_CRITICAL_ENTER();
	if(size <= 512)
	{
		kmem_cache_ptr = find_first_bigger_cache(&cache_chain_head,size); /*find a suitable cache*/
		while(1)
		{
			if(NULL != kmem_cache_ptr)/* this means that there is a slab.size bigger than size*/
			{
				/*first try to get room from slabs_partial*/
				if(!list_empty(&kmem_cache_ptr->slabs_partial)) /* if chain "slabs_partial" is not empty*/
				{
					struct slab *slab_ptr = list_entry(kmem_cache_ptr->slabs_partial.next,struct slab,slab_chain); /* get slab-ptr*/
					ptr = slab_ptr->block_head.next; /* ptr is the point of the allocated space*/
					list_del(slab_ptr->block_head.next); /* delete it from slab chain "block_head"*/
					if(0 == --(slab_ptr->current_block_num)) /* if this slab has no more space*/
					{
						list_del(&slab_ptr->slab_chain);
						list_add(&slab_ptr->slab_chain,&kmem_cache_ptr->slabs_full); /* put it into the chain "slabs_full"*/
					}
					CPU_CRITICAL_EXIT();
					return ptr;
				}
				else if(!list_empty(&kmem_cache_ptr->slabs_empty)) /* else if chain "slabs_empty" is not empty*/
				{
					struct slab *slab_ptr = list_entry(kmem_cache_ptr->slabs_empty.next,struct slab,slab_chain);
					ptr = slab_ptr->block_head.next;
					list_del(slab_ptr->block_head.next);
					list_del(&slab_ptr->slab_chain);
					list_add(&slab_ptr->slab_chain,&kmem_cache_ptr->slabs_partial);
					--(slab_ptr->current_block_num);
					CPU_CRITICAL_EXIT();
					return ptr;
				}
				else /* at this case we have to allocate mem from buddy*/
				{
					struct slab *slab_ptr;
					unsigned int level;
					level = get_set_bit_place((10 * kmem_cache_ptr->kmem_cache_slab_size + sizeof(struct slab)) / PAGE_SIZE_BYTE) + 2;
					/*at least allocate 10 times of corresponding space and a room of sizeof(struct slab)*/
					slab_ptr = (struct slab *)_case_alloc_buddy(level);
					if(NULL != slab_ptr)
					{
						load_slab((void *)slab_ptr,slab_ptr->end_ptr,kmem_cache_ptr->kmem_cache_slab_size,&kmem_cache_ptr->slabs_partial);
						ptr = slab_ptr->block_head.next; 	 /* ptr is the point of the allocated space*/
						list_del(slab_ptr->block_head.next); /* delete it from slab chain "block_head"*/
						--(slab_ptr->current_block_num);
						CPU_CRITICAL_EXIT();
						return ptr;
					}
					else /* NULL == slab_ptr*/
					{
						struct singly_list_head *pos = kmem_cache_ptr->node.next;
						if(cache_chain_head_ptr == pos)
						{
							CPU_CRITICAL_EXIT();
							return NULL;
						}
						kmem_cache_ptr = singly_list_entry(pos,struct kmem_cache,node);
						continue ;
					}
				}
			}
			else /* NULL == IL_ptr*/
			{
				kmem_cache_ptr = ka_malloc(sizeof(struct kmem_cache));
				ASSERT(NULL != kmem_cache_ptr);
				if(NULL == kmem_cache_ptr)
				{
					CPU_CRITICAL_EXIT();
					return NULL;
				}
				insert_into_cache_chain(&cache_chain_head,kmem_cache_ptr,size);
				kmem_cache_ptr = find_first_bigger_cache(&cache_chain_head,size);
				continue ;
			}
		}
	}
	else /*size > 512*/
	{
		int level = get_set_bit_place(size/PAGE_SIZE_BYTE) + 2;
		ptr = _case_alloc_buddy((unsigned int)level);
		if(NULL == ptr)
		{
			CPU_CRITICAL_EXIT();
			return NULL;
		}
		if(FUN_EXECUTE_SUCCESSFULLY == add_page_alloc_record(level,ptr))
		{
			CPU_CRITICAL_EXIT();
			return ptr;
		}
		else
		{
			ka_printf("fatal error from add_page_alloc_record()\n");
			_case_slab_free_buddy(ptr,(void *)((unsigned int)ptr + ka_pow(2,level-1) * PAGE_SIZE_BYTE));
			CPU_CRITICAL_EXIT();
			return NULL;
		}
	}
}

/* this functin try to free ptr,if ptr is in the scope of one slab in the slab 
** chain "head",then insert the ptr into corresponding slab and return slab_ptr,
** or if ptr not belongs to any slab in this slab chain, then return NULL means 
** that return ptr fail;*/
static struct slab *free_find_in_slab_chain(struct list_head *head,void *ptr)
{
	struct list_head *pos;
	struct slab *slab_ptr;
	list_for_each(pos, head)
	{
		slab_ptr = list_entry(pos,struct slab,slab_chain);
		if(((int)ptr > (int)slab_ptr->start_ptr) && ((int)ptr < (int)slab_ptr->end_ptr))
		{
			ASSERT(0 == (((unsigned int)slab_ptr->end_ptr - (unsigned int)ptr) % (slab_ptr->block_size)));
			++(slab_ptr->current_block_num);
			ASSERT(slab_ptr->current_block_num <= slab_ptr->full_block_num);
			list_add((struct list_head *)ptr,&slab_ptr->block_head);	
			return slab_ptr;
		}
	}
	return NULL;
}

int in_os_memory(void *ptr)
{
	struct buddy *buddy_ptr = (struct buddy *)_get_os_buddy_ptr_head();
	ASSERT(NULL != buddy_ptr);
	while(NULL != buddy_ptr)
	{
		if(FUN_EXECUTE_SUCCESSFULLY == in_buddy_range(ptr))
		{
			return FUN_EXECUTE_SUCCESSFULLY;
		}
		buddy_ptr = (struct buddy *)_get_next_buddy_ptr_head(buddy_ptr);
	}
	return -1;
}

void ka_free(void *ptr)
{
	struct kmem_cache *kmem_cache_ptr;
	struct slab *slab_ptr;
	CPU_SR_ALLOC();
	if(FUN_EXECUTE_SUCCESSFULLY != in_os_memory(ptr))
	{
		return ;
	}
	CPU_CRITICAL_ENTER();
	singly_list_for_each_entry(kmem_cache_ptr, &cache_chain_head, node)
	{
		if(!list_empty(&kmem_cache_ptr->slabs_partial))
		{
			slab_ptr = free_find_in_slab_chain(&kmem_cache_ptr->slabs_partial,ptr);
			if(NULL != slab_ptr) /* free success*/
			{
				if(slab_ptr->current_block_num == slab_ptr->full_block_num)
				{
					list_del(&slab_ptr->slab_chain);
					list_add(&slab_ptr->slab_chain,&kmem_cache_ptr->slabs_empty);
				}
				CPU_CRITICAL_EXIT();
				return ;
			}
		}
		if(!list_empty(&kmem_cache_ptr->slabs_full))
		{
			slab_ptr = free_find_in_slab_chain(&kmem_cache_ptr->slabs_full,ptr);
			if(NULL != slab_ptr) /* free success*/
			{
				list_del(&slab_ptr->slab_chain);
				list_add(&slab_ptr->slab_chain,&kmem_cache_ptr->slabs_partial);
				CPU_CRITICAL_EXIT();
				return ;
			}
		}
	}
	if(FUN_EXECUTE_SUCCESSFULLY == find_and_remove_record(ptr))
	{
		CPU_CRITICAL_EXIT();
		return;
	}
	ka_printf("not in a slab or record,fatal error\n");
	ASSERT(0);/*should not go here*/
	while(1);
}

#if CONFIG_SHELL_EN
void shell_check_kmem(int argc, char const *argv[])
{
	(void)argc;
	(void)argv;
	struct kmem_cache *kmem_cache_ptr;
	int i = 0;
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	singly_list_for_each_entry(kmem_cache_ptr, &cache_chain_head, node)
	{
		ka_printf("===========get NO.%d kmem===========\n",++i);
		ka_printf("the size of this kmem_cache is %u\n",kmem_cache_ptr->kmem_cache_slab_size);
		if(!list_empty(&kmem_cache_ptr->slabs_full))
		{
			struct list_head *buffer;
			int num = 0;
			list_for_each(buffer,&kmem_cache_ptr->slabs_full)
			{
				++num;
			}
			ka_printf("chain slabs_full has %d slab\n",num);
		}
		else
		{
			ka_printf("chain slabs_full is empty\n");
		}
		if(!list_empty(&kmem_cache_ptr->slabs_partial))
		{
			struct list_head *buffer;
			int num = 0;
			list_for_each(buffer,&kmem_cache_ptr->slabs_partial)
			{
				++num;
			}
			ka_printf("chain slabs_partial has %d slab\n",num);
		}
		else
		{
			ka_printf("chain slabs_partial is empty\n");
		}
		if(!list_empty(&kmem_cache_ptr->slabs_empty))
		{
			struct list_head *buffer;
			int num = 0;
			list_for_each(buffer,&kmem_cache_ptr->slabs_empty)
			{
				++num;
			}
			ka_printf("chain slabs_empty has %d slab\n",num);
		}
		else
		{
			ka_printf("chain slabs_empty is empty\n");
		}
		ka_printf("\n");
	}
	CPU_CRITICAL_EXIT();
}
#endif

#if CONFIG_SHELL_EN
void shell_check_slab(int argc, char const *argv[])
{
	(void)argc;
	(void)argv;
	struct kmem_cache *kmem_cache_ptr;
	int i = 0;
	struct slab *slab_ptr;
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	singly_list_for_each_entry(kmem_cache_ptr, &cache_chain_head, node)
	{
		ka_printf("===========get NO.%d kmem===========\n",++i);
		ka_printf("the size of this kmem_cache is %u\n",kmem_cache_ptr->kmem_cache_slab_size);
		if(!list_empty(&kmem_cache_ptr->slabs_full))
		{
			struct list_head *buffer;
			int num = 0;
			ka_printf("in chain slabs_full\n");
			list_for_each(buffer,&kmem_cache_ptr->slabs_full)
			{
				ka_printf("the NO.%d slab information:\n",++num);
				slab_ptr = list_entry(buffer,struct slab,slab_chain);
				ka_printf("start_ptr is %x\n",(int)slab_ptr->start_ptr);
				ka_printf("end_ptr is %x\n",(int)slab_ptr->end_ptr);
				ka_printf("current_block_num is %d\n",slab_ptr->current_block_num);
				ka_printf("full_block_num is %d\n",slab_ptr->full_block_num);
				ka_printf("block_size is %d\n",slab_ptr->block_size);
				ka_printf("\n");
			}
			ka_printf("chain slabs_full has %d slab\n",num);
		}
		else
		{
			ka_printf("chain slabs_full is empty\n");
		}
		ka_printf("\n");
		if(!list_empty(&kmem_cache_ptr->slabs_partial))
		{
			struct list_head *buffer;
			int num = 0;
			ka_printf("in chain slabs_partial\n");
			list_for_each(buffer,&kmem_cache_ptr->slabs_partial)
			{
				ka_printf("the NO.%d slab information:\n",++num);
				slab_ptr = list_entry(buffer,struct slab,slab_chain);
				ka_printf("start_ptr is %x\n",(int)slab_ptr->start_ptr);
				ka_printf("end_ptr is %x\n",(int)slab_ptr->end_ptr);
				ka_printf("current_block_num is %d\n",slab_ptr->current_block_num);
				ka_printf("full_block_num is %d\n",slab_ptr->full_block_num);
				ka_printf("block_size is %d\n",slab_ptr->block_size);
				ka_printf("\n");
			}
			ka_printf("chain slabs_partial has %d slab\n",num);
		}
		else
		{
			ka_printf("chain slabs_partial is empty\n");
		}
		ka_printf("\n");
		if(!list_empty(&kmem_cache_ptr->slabs_empty))
		{
			struct list_head *buffer;
			int num = 0;
			ka_printf("in chain slabs_empty\n");
			list_for_each(buffer,&kmem_cache_ptr->slabs_empty)
			{
				ka_printf("the NO.%d slab information:\n",++num);
				slab_ptr = list_entry(buffer,struct slab,slab_chain);
				ka_printf("start_ptr is %x\n",(int)slab_ptr->start_ptr);
				ka_printf("end_ptr is %x\n",(int)slab_ptr->end_ptr);
				ka_printf("current_block_num is %d\n",slab_ptr->current_block_num);
				ka_printf("full_block_num is %d\n",slab_ptr->full_block_num);
				ka_printf("block_size is %d\n",slab_ptr->block_size);
				ka_printf("\n");
			}
			ka_printf("chain slabs_empty has %d slab\n",num);
		}
		else
		{
			ka_printf("chain slabs_empty is empty\n");
		}
		ka_printf("\n");
	}
	CPU_CRITICAL_EXIT();
}
#endif
