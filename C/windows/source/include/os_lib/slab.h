#ifndef _SLAB_H
#define _SLAB_H

#include "insert_sort.h"

struct kmem_cache
{
	IL kmem_cache_insert_chain;//used for sort
	struct list_head slabs_full; // which space is fully exhausted
	struct list_head slabs_partial;  
	struct list_head slabs_empty; // which space is not used
};

void insert_into_cache_chain(struct list_head *head,struct kmem_cache *kmem_cache_ptr,int block_size);


//sizeof(struct slab)=36
struct slab
{
	void *start_ptr;
	void *end_ptr;
	unsigned int current_block_num;
	unsigned int full_block_num;
	unsigned int block_size;
	struct list_head slab_chain;
	struct list_head block_head;
};

void load_slab(
//	struct slab *slab_ptr,
	void *start_ptr,
	void *end_ptr,
	unsigned int   block_size,
	struct list_head *slab_chain_ptr
	);

void shell_check_kmem(int argc, char const *argv[]);
void shell_check_slab(int argc, char const *argv[]);

#endif
