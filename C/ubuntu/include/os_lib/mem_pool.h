#ifndef _MEM_POOL_H
#define _MEM_POOL_H

#include <kakaosstdint.h>
#include <double_linked_list.h>
#include <insert_sort_oo.h>

typedef struct mem_pool
{
	UINT32 flag;
	char *name;
	void *start_add; /*so the end addr is start_add+block_size*full_block_num*/
	void *end_add;
	UINT32 full_block_num;
	UINT32 block_size; /* should bigger than 8 bytes, and can be divisible by 4*/
	UINT32 current_block_num;
	struct list_head block_head;
	struct mem_pool *next_mem_pool_ptr;
	struct insert_sort_entity mp_delay_insert_sort_list; /*store the wait_delay_TCB*/
}mp;

#define MEM_POOL_INIT	 	0X00
#define MEM_POOL_CREATE 	0X01

static inline void set_mp_flag_create(mp *mp_ptr)
{
	mp_ptr->flag |= MEM_POOL_CREATE;
}

static inline int mp_is_create(mp *mp_ptr)
{
	return (mp_ptr->flag & MEM_POOL_CREATE);
}

int init_mp(mp *mp_ptr,const char *name,void *start_add,UINT32 block_num,UINT32 block_size);
int create_mp(const char *name,UINT32 block_num,UINT32 block_size,mp **mp_store_ptr);
int delete_mp(mp *mp_ptr);
void *mp_alloc(mp *mp_ptr,UINT32 wait_time);
void mp_free(void *ptr);
void shell_mem_pool(int argc, char const *argv[]);

#endif
