#ifndef _SLAB_H
#define _SLAB_H

#include <kakaosstdint.h>
#include <singly_linked_list.h>
#include <double_linked_list.h>

struct kmem_cache
{
	struct singly_list_head node;
	unsigned int kmem_cache_slab_size;	/*used for sort*/
	struct list_head slabs_full; 	/* which space is fully exhausted*/
	struct list_head slabs_partial;  
	struct list_head slabs_empty; 	/* which space is not used */
};

void insert_into_cache_chain(struct singly_list_head *head,struct kmem_cache *kmem_cache_ptr,unsigned int block_size);
struct kmem_cache *find_first_bigger_cache(struct singly_list_head *head,unsigned int size);

/*sizeof(struct slab)=36*/
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

unsigned int load_slab(
	void *start_ptr,
	void *end_ptr,
	unsigned int   block_size,
	struct list_head *slab_chain_ptr
	);

void shell_check_kmem(int argc, char const *argv[]);
void shell_check_slab(int argc, char const *argv[]);

#endif
