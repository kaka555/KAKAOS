#ifndef _HEAP_OO_H
#define _HEAP_OO_H

#include <vector.h>

#define heap_remove_top(little_heap_ptr,data_store_ptr)  heap_remove_index_data((little_heap_ptr),1,(data_store_ptr))
#define heap_get_top(little_heap_ptr,data_store_ptr)  heap_get_index_data((little_heap_ptr),(data_store_ptr),1)
#define heap_get_top_safe(little_heap_ptr,data_store_ptr)  heap_get_index_data_safe((little_heap_ptr),(data_store_ptr),1)

struct little_heap
{
/*private*/
	Vector data;   /* use array to store the data , index start is 1*/
	/* return value <0 means data[index1] < data[index2] */
	int (*cmp)(Vector *Vector_ptr,unsigned int index1,unsigned int index2); /* object oriented*/
	/*auto used by heap_adjust(),if you do not need if,give a empty function to it*/
	void (*index_change_record)(Vector *Vector_ptr,int index);/* object oriented*/
};

int heap_init(struct little_heap *const little_heap_ptr,
	unsigned int size,
	int (*cmp)(Vector *Vector_ptr,unsigned int index1,unsigned int index2),
	void (*index_change_record)(Vector *Vector_ptr,int index));

static inline void heap_delete(struct little_heap *little_heap_ptr)
{
	Vector_delete(&little_heap_ptr->data);
}

/*get the data with index "index" and store it into *data_store_ptr*/
static inline void * heap_get_index_data(struct little_heap *little_heap_ptr,unsigned int index)
{
	return Vector_get_index_data(&little_heap_ptr->data,index);
}

static inline unsigned int heap_get_cur_len(struct little_heap *little_heap_ptr)
{
	return get_Vector_cur_len(&little_heap_ptr->data);
}

int heap_push(struct little_heap *const little_heap_ptr,void *push_data_ptr);
int heap_get_index_data_safe(struct little_heap *const little_heap_ptr,void **data_store_ptr,unsigned int index);
int heap_set_index_data(struct little_heap *const little_heap_ptr,unsigned int index,void *data_store_ptr);
/*if data_store_ptr is NULL,do not store*/
int heap_remove_index_data(struct little_heap *const little_heap_ptr,unsigned int index,void *data_store_ptr);
int heap_erase_data(struct little_heap *const little_heap_ptr,unsigned int from,unsigned int to);

#endif
