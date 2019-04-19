#include <mem_pool.h>
#include <myassert.h>
#include <os_error.h>
#include <myMicroLIB.h>
#include <os_cpu.h>
#include <TCB.H>
#include <printf_debug.h>
#include <os_time.h>
#include <os_schedule.h>
#include <double_linked_list.h>
#include <buddy.h>

#if CONFIG_MEM_POOL

static mp *mp_head;
extern TCB * OSTCBCurPtr;
extern int get_set_bit_place(unsigned int num);

static int compare(struct insert_sort_data *data1,struct insert_sort_data *data2)
{
	return ((TCB*)(data1->data_ptr))->prio - ((TCB*)(data2->data_ptr))->prio;
}

static int value_cmp(void *data1,void *data2)
{
	return ((TCB*)data1)->prio - ((TCB*)data2)->prio;
}

int _init_mp(mp *mp_ptr,const char *name,void *start_add,UINT32 block_num,UINT32 block_size)
{
/*init all the parameters*/
	ASSERT((NULL != mp_ptr) && (NULL != name) && (NULL != start_add) &&
				(0 != block_num) && (block_size >= 8) && !(block_size & 0x03),ASSERT_INPUT);
	mp_ptr->flag = MEM_POOL_INIT;
	mp_ptr->name = (char *)name;
	mp_ptr->start_add = start_add;
	mp_ptr->end_add = (void *)((UINT32)start_add + block_num * block_size);
	mp_ptr->full_block_num = block_num;
	mp_ptr->block_size = block_size;
	mp_ptr->current_block_num = block_num;
	INIT_LIST_HEAD(&mp_ptr->block_head);
	if(NULL == mp_head)
	{
		mp_head = mp_ptr;
		mp_ptr->next_mem_pool_ptr = NULL;
	}
	else
	{
		mp_ptr->next_mem_pool_ptr = mp_head;
		mp_head = mp_ptr;
	}
	if(init_insert_sort_entity(&mp_ptr->mp_delay_insert_sort_list,
		compare,
		NULL,
		value_cmp) < 0)
	{
		return -ERROR_SYS;
	}
/* build the list so that we can get mem easily */
	unsigned int i;
	struct list_head *list_head_ptr = (struct list_head *)start_add;
	for(i=0;i<block_num;++i)
	{
		ASSERT(((UINT32)list_head_ptr >= (UINT32)start_add) &&
				((UINT32)list_head_ptr <= (UINT32)start_add + (block_num-1)*block_size),
				ASSERT_PARA_AFFIRM);
		INIT_LIST_HEAD(list_head_ptr);
		list_add_tail(list_head_ptr,&mp_ptr->block_head);
		list_head_ptr = (struct list_head *)((UINT32)list_head_ptr + block_size);
	}
	ASSERT((UINT32)list_head_ptr == (UINT32)start_add + block_num*block_size,ASSERT_PARA_AFFIRM);
	return FUN_EXECUTE_SUCCESSFULLY;
}

/**
 * @Author      kaka
 * @DateTime    2019-04-18
 * @description : initialize the mem pool
 * @param       mp_ptr     [the addr of struct mp]
 * @param       name       [mp's name]
 * @param       start_add  [the space addr witch will be allocated]
 * @param       block_num  
 * @param       block_size 
 * @return      error code
 */
int init_mp(mp *mp_ptr,const char *name,void *start_add,UINT32 block_num,UINT32 block_size)
{
	if((NULL == mp_ptr) || (NULL == name) || (NULL == start_add))
	{
		return -ERROR_NULL_INPUT_PTR;
	}
	if((0 == block_num) || (block_size < 8) || (block_size & 0x03))
	{
		return -ERROR_USELESS_INPUT;
	}
	return _init_mp(mp_ptr,name,start_add,block_num,block_size);
}

int _create_mp(const char *name,UINT32 block_num,UINT32 block_size,mp **mp_store_ptr)
{
	ASSERT((NULL != name) && (0 != block_num) && (block_size >= 8) && !(block_size & 0x03),ASSERT_INPUT);
/* use malloc for a space for struct mp */
	mp *mp_ptr = ka_malloc(sizeof(mp));
	if(NULL == mp_ptr)
	{
		KA_WARN(DEBUG_MEM_POOL,"mem pool allocate struct fail\n");
		goto out2;
	}
/* calculate the space that the pool need and increase the actual block_num if can*/
	unsigned int mem_len = block_num * block_size;
	unsigned int level = get_set_bit_place(mem_len / PAGE_SIZE_BYTE) + 2;
	unsigned int actual_block_num = ka_pow(2,level-1) * PAGE_SIZE_BYTE / block_size;
/* use malloc for a space for pool */
extern void *_case_alloc_buddy(unsigned int level);
	void *start_add = _case_alloc_buddy(level);
	if(NULL == start_add)
	{
		KA_WARN(DEBUG_MEM_POOL,"mem pool space allocate struct fail\n");
		goto out1;
	}
/* and than init it */
	int error = init_mp(mp_ptr,name,start_add,actual_block_num,block_size);
	if(error < 0)
	{
		KA_WARN(DEBUG_MEM_POOL,"init_mp fail\n");
		ka_free(mp_ptr);
extern void _case_slab_free_buddy(void *ptr,void *end_ptr);
		_case_slab_free_buddy(start_add,(void *)(unsigned int)start_add + PAGE_SIZE_BYTE*ka_pow(2,level-1));
		return error;
	}
	set_mp_flag_create(mp_ptr);
	*mp_store_ptr = mp_ptr;
	return FUN_EXECUTE_SUCCESSFULLY;
out1:
	ka_free(mp_ptr);
out2:
	return -ERROR_NO_MEM;
}

/**
 * @Author      kaka
 * @DateTime    2019-04-18
 * @description : this function will ask mem space of struct mp and mem pool from OS,
 *              	if succeed, initialize the mem pool; this funcion can not be used
 *              	in bsp_init
 * @param       name         [mp's name]
 * @param       block_num    
 * @param       block_size   
 * @param       mp_store_ptr [the addr of mp pointer]
 * @return      error code
 */
int create_mp(const char *name,UINT32 block_num,UINT32 block_size,mp **mp_store_ptr)
{
	if((0 == block_num) || (block_size < 8) || (block_size & 0x03))
	{
		return -ERROR_USELESS_INPUT;
	}
	return _create_mp(name,block_num,block_size,mp_store_ptr);
}

int _delete_mp(mp *mp_ptr)
{
	ASSERT(NULL != mp_ptr,ASSERT_INPUT);
/* critical area */
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
/* if there are still some pools that not being returned, deny delete */
	if(mp_ptr->full_block_num != mp_ptr->current_block_num)
	{
		KA_WARN(DEBUG_MEM_POOL,"some pool not being returned\n");
		CPU_CRITICAL_EXIT();
		return -ERROR_SYS;
	}
/* delete mp record from os mp list */
	mp *mp_buffer_ptr = mp_head;
	ASSERT(mp_buffer_ptr != NULL,ASSERT_PARA_AFFIRM);
	while(mp_buffer_ptr != NULL)
	{
	    if(mp_buffer_ptr->next_mem_pool_ptr == mp_ptr)
	    {
	    	mp_buffer_ptr->next_mem_pool_ptr = mp_ptr->next_mem_pool_ptr;
	    	break;
	    }
	    mp_buffer_ptr = mp_buffer_ptr->next_mem_pool_ptr;
	    ASSERT(mp_buffer_ptr != NULL,ASSERT_BAD_EXE_LOCATION);
	}
/* check the flag to see if the mem pool should return to OS */
	if(mp_is_create(mp_ptr))
	{
		unsigned int mem_len = mp_ptr->full_block_num * mp_ptr->block_size;
		unsigned int level = get_set_bit_place(mem_len / PAGE_SIZE_BYTE) + 2;
extern void _case_slab_free_buddy(void *ptr,void *end_ptr);
		_case_slab_free_buddy(mp_ptr->start_add,(void *)(unsigned int)mp_ptr->start_add + PAGE_SIZE_BYTE*ka_pow(2,level-1));
		ka_free(mp_ptr);
	}
	CPU_CRITICAL_EXIT();
	return FUN_EXECUTE_SUCCESSFULLY;
}

/**
 * @Author      kaka
 * @DateTime    2019-04-18
 * @description :
 * @param       mp_ptr     [the mp pointer will be deleted]
 * @return      error code
 */
int delete_mp(mp *mp_ptr)
{
	if(NULL == mp_ptr)
	{
		return -ERROR_NULL_INPUT_PTR;
	}
	return _delete_mp(mp_ptr);
}

/* allocate a block with no check */
static void *mp_get_a_block(mp *mp_ptr)
{
	ASSERT(NULL != mp_ptr,ASSERT_INPUT);
	ASSERT(!list_empty(&mp_ptr->block_head),ASSERT_INPUT);
	ASSERT(mp_ptr->current_block_num > 0,ASSERT_INPUT);
	struct list_head *block_addr = mp_ptr->block_head.next;
/* and than deal with the member of mp */
	list_del(block_addr);
	--(mp_ptr->current_block_num);
	return (void *)block_addr;
}


void *_mp_alloc(mp *mp_ptr,UINT32 wait_time)
{
/* check the input para */
	ASSERT(NULL != mp_ptr,ASSERT_INPUT);
/* critical area */
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
/* if there is a free pool, allocate it */
	if(mp_ptr->current_block_num > 0)
	{
		void *buffer = mp_get_a_block(mp_ptr);
		CPU_CRITICAL_EXIT();
		return buffer;
	}
/* if there is not free pool, check the input para */
	if(0 == wait_time)
	{
/* if the task do not want to wait */
		CPU_CRITICAL_EXIT();
		return NULL;
	}
/* if the task want to wait in a window */
	DECLEAR_INSERT_SORT_DATA(data);
	init_insert_sort_data(&data,OSTCBCurPtr);
	insert_sort_insert_into(&data,&mp_ptr->mp_delay_insert_sort_list);
	UINT64 time_record = _get_tick();
	CPU_CRITICAL_EXIT(); /*exit  critical*/
	sys_delay(wait_time,STATE_WAIT_MEM_POOL_TIMEOUT);
/* go back here=========go back here */
	CPU_CRITICAL_ENTER();/*enter critical*/
	if(_get_tick() >= (time_record + wait_time))/*prove that timeout happened*/
	{
		insert_sort_delete_data(&mp_ptr->mp_delay_insert_sort_list,OSTCBCurPtr);
		CPU_CRITICAL_EXIT(); /*exit  critical*/
		return NULL;
	}
	else
	{
/* now there is a free pool for allocation */
		void *buffer = mp_get_a_block(mp_ptr);
		CPU_CRITICAL_EXIT();
		return buffer;
	}
}

/**
 * @Author      kaka
 * @DateTime    2019-04-19
 * @description : request a pool from mp_ptr
 * @param       mp_ptr     [the mem pool that allocate the mem block]
 * @param       wait_time  [0 means ]
 * @return      the start addr of the block, NULL means allocation fail
 */
void *mp_alloc(mp *mp_ptr,UINT32 wait_time)
{
	if(NULL == mp_ptr)
	{
		return NULL;
	}
	return _mp_alloc(mp_ptr,wait_time);
}

void _mp_free(void *ptr)
{
	ASSERT(NULL != ptr,ASSERT_INPUT);
/* critical area */
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
/* first find the mp that the ptr belongs to */
	mp *mp_ptr = mp_head;
	ASSERT(NULL != mp_ptr,ASSERT_PARA_AFFIRM);
	while(NULL != mp_ptr)
	{
		if(((UINT32)ptr >= (UINT32)mp_ptr->start_add) && 
			((UINT32)ptr < (UINT32)mp_ptr->end_add))
		{
			break ;
		}
		mp_ptr = mp_ptr->next_mem_pool_ptr;
	}
/* then check the ptr location*/ 
	if((NULL == mp_ptr) || 
			(((UINT32)ptr - (UINT32)mp_ptr->start_add) % mp_ptr->block_size))
	{
		KA_WARN(DEBUG_MEM_POOL,"mp_free invalid return ptr\n");
		CPU_CRITICAL_EXIT();
		return ;
	}
/* if ptr legal, deal with the member of mp to return */
	struct list_head *list_head_ptr = (struct list_head *)ptr;
	list_add(list_head_ptr,&mp_ptr->block_head);
	++(mp_ptr->current_block_num);
	ASSERT(mp_ptr->current_block_num <= mp_ptr->full_block_num,ASSERT_PARA_AFFIRM);
	CPU_CRITICAL_EXIT();
	return ;
}

void mp_free(void *ptr)
{
	if(NULL == ptr)
	{
		KA_WARN(DEBUG_MEM_POOL,"mp_free return NULL\n");
		return ;
	}
	_mp_free(ptr);
}

void shell_mem_pool(int argc, char const *argv[])
{
	mp *mp_ptr = mp_head;
	if(NULL == mp_ptr)
	{
		ka_printf("no mem pool in os\n");
		return ;
	}
	while(mp_ptr)
	{
		ka_printf("mem pool's name is %s\n",mp_ptr->name);
		ka_printf("mem pool is ");
		if(mp_is_create(mp_ptr))
		{
			ka_printf("created\n");
		}
		else
		{
			ka_printf("init\n");
		}
		ka_printf("mem pool's start_add is %p\n",mp_ptr->start_add);
		ka_printf("mem pool's end_add is %p\n",mp_ptr->end_add);
		ka_printf("mem pool's full_block_num is %u\n",mp_ptr->full_block_num);
		ka_printf("mem pool's block_size is %u\n",mp_ptr->block_size);
		ka_printf("mem pool's current_block_num is %u\n",mp_ptr->current_block_num);
		struct list_head *pos;
		unsigned int i = 0;
		list_for_each(pos,&mp_ptr->block_head)
		{
			++i;
		}
		ASSERT(i == mp_ptr->current_block_num,ASSERT_PARA_AFFIRM);
		ka_printf("data num of mp_delay_insert_sort_list is %u\n",
			mp_ptr->mp_delay_insert_sort_list.data_num);
		mp_ptr = mp_ptr->next_mem_pool_ptr;
	}
}

#endif
