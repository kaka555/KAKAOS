#include <vector.h>
#include <os_error.h>
#include <myMicroLIB.h>
#include <myassert.h>
#include <kakaosstdint.h>

/*
initilize the Vector entity with vector_ptr->max_len = size,vector_ptr->cur_len = 0,
vector_ptr->data_size = len_per_data; then use malloc() to allocate room 
 */
int Vector_init(Vector *vector_ptr,unsigned int size,int len_per_data,unsigned int extension_factor)
{
	ASSERT(NULL != vector_ptr);
	ASSERT(len_per_data > 0);
#if CONFIG_PARA_CHECK
	if(NULL == vector_ptr)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(Vector_init,vector_ptr);
		return -ERROR_NULL_INPUT_PTR;
	}
	if(len_per_data <= 0)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(Vector_init,len_per_data);
		return -ERROR_VALUELESS_INPUT;
	}
#endif
	vector_ptr->max_len = size;
	vector_ptr->cur_len = 0;
	vector_ptr->data_size = len_per_data;
	vector_ptr->extension_factor = extension_factor;
	vector_ptr->data_ptr = f_malloc(vector_ptr->max_len * vector_ptr->data_size);
	if(NULL == vector_ptr->data_ptr)
	{
		return -ERROR_ALLOCATE_ROOM;
	}
	return FUN_EXECUTE_SUCCESSFULLY;
}

/*
allocate a bigger room for vector with factor extension_factor
then change the attribution max_len
 */
static int expand_room(Vector *vector_ptr)
{
	ASSERT(NULL != vector_ptr);
	ASSERT(vector_ptr->cur_len == vector_ptr->max_len);

	void *buffer_ptr = vector_ptr->data_ptr;
	if(VFISMUL(vector_ptr->extension_factor))
	{
		vector_ptr->data_ptr = f_malloc(GET_EXPENSION_FACTOR(vector_ptr->extension_factor) * vector_ptr->max_len * vector_ptr->data_size);
	}
	else
	{
		vector_ptr->data_ptr = f_malloc((GET_EXPENSION_FACTOR(vector_ptr->extension_factor) + vector_ptr->max_len) * vector_ptr->data_size);
	}
	ASSERT(NULL != vector_ptr->data_ptr);
	if(NULL == vector_ptr->data_ptr)
	{
		return -ERROR_ALLOCATE_ROOM;
	}
	f_memcpy(vector_ptr->data_ptr,buffer_ptr,vector_ptr->max_len * vector_ptr->data_size);
	if(VFISMUL(vector_ptr->extension_factor))
	{
		vector_ptr->max_len *= GET_EXPENSION_FACTOR(vector_ptr->extension_factor);
	}
	else
	{
		vector_ptr->max_len += GET_EXPENSION_FACTOR(vector_ptr->extension_factor);
	}
	
	f_free(buffer_ptr);
	return FUN_EXECUTE_SUCCESSFULLY;
}

/*
add a data at tail
 */
int Vector_push_back(Vector *vector_ptr,void *push_data_ptr)
{
	ASSERT(NULL != vector_ptr);
	ASSERT(NULL != push_data_ptr);
#if CONFIG_PARA_CHECK
	if((NULL == vector_ptr) || (NULL == push_data_ptr))
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(Vector_push_back,vector_ptr);
		OS_ERROR_PARA_MESSAGE_DISPLAY(Vector_push_back,push_data_ptr);
		return -ERROR_NULL_INPUT_PTR;
	}
#endif
	if(vector_ptr->cur_len < vector_ptr->max_len)
	{
		f_memcpy((char*)(vector_ptr->data_ptr) + vector_ptr->cur_len * vector_ptr->data_size,push_data_ptr,vector_ptr->data_size);
		++(vector_ptr->cur_len);
	}
	else
	{
		ASSERT(vector_ptr->cur_len == vector_ptr->max_len);
		if(expand_room(vector_ptr) < 0)
		{
			return -ERROR_ALLOCATE_ROOM;
		}
		f_memcpy((char*)(vector_ptr->data_ptr) + vector_ptr->cur_len * vector_ptr->data_size,push_data_ptr,vector_ptr->data_size);
		++(vector_ptr->cur_len);
	}
	return FUN_EXECUTE_SUCCESSFULLY;
}

/*
pop the tail datta and store it into pop_data_ptr
 */
int Vector_pop_back(Vector *vector_ptr,void *pop_data_ptr)
{
	ASSERT(NULL != vector_ptr);
#if CONFIG_PARA_CHECK
	if(NULL == vector_ptr)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(Vector_pop_back,vector_ptr);
		return -ERROR_NULL_INPUT_PTR;
	}
#endif
	if(0 == vector_ptr->cur_len)
	{
		return -ERROR_DATA_EMPTY;
	}
	--(vector_ptr->cur_len);
	if(pop_data_ptr) /* if NULL != pop_data_ptr*/
		f_memcpy(pop_data_ptr,(char*)(vector_ptr->data_ptr) + vector_ptr->cur_len * vector_ptr->data_size,vector_ptr->data_size);
	return FUN_EXECUTE_SUCCESSFULLY;
}

/*
get vector[index] data into *data_store_ptr
 */
int Vector_get_index_data(Vector *vector_ptr,unsigned int index,void *data_store_ptr)
{
	ASSERT(NULL != vector_ptr);
	ASSERT(NULL != data_store_ptr);
#if CONFIG_PARA_CHECK
	if((NULL == vector_ptr) || (NULL == data_store_ptr))
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(Vector_get_index_data,vector_ptr);
		OS_ERROR_PARA_MESSAGE_DISPLAY(Vector_get_index_data,data_store_ptr);
		return -ERROR_NULL_INPUT_PTR;
	}
	if(index >= vector_ptr->max_len)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(Vector_get_index_data,index);
		return -ERROR_VALUELESS_INPUT;
	}
#endif
	f_memcpy(data_store_ptr,(char*)(vector_ptr->data_ptr) + index * vector_ptr->data_size,vector_ptr->data_size);
	return FUN_EXECUTE_SUCCESSFULLY;
}

/*
set vector[index] data with *data_store_ptr
 */
int Vector_set_index_data(Vector *vector_ptr,unsigned int index,void *data_store_ptr)
{
	ASSERT(NULL != vector_ptr);
	ASSERT(NULL != data_store_ptr);
#if CONFIG_PARA_CHECK
	if((NULL == vector_ptr) || (NULL == data_store_ptr))
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(Vector_set_index_data,vector_ptr);
		OS_ERROR_PARA_MESSAGE_DISPLAY(Vector_set_index_data,data_store_ptr);
		return -ERROR_NULL_INPUT_PTR;
	}
	if(index >= vector_ptr->max_len)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(Vector_set_index_data,index);
		return -ERROR_VALUELESS_INPUT;
	}
#endif
	f_memcpy((char*)(vector_ptr->data_ptr) + index * vector_ptr->data_size,data_store_ptr,vector_ptr->data_size);
	return FUN_EXECUTE_SUCCESSFULLY;
}

int Vector_get_index_address(Vector *vector_ptr,unsigned int index,void **data_add_ptr)
{
	ASSERT(NULL != vector_ptr);
	ASSERT(NULL != data_add_ptr);
#if CONFIG_PARA_CHECK
	if((NULL == vector_ptr) || (NULL == data_add_ptr))
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(Vector_get_index_address,vector_ptr);
		OS_ERROR_PARA_MESSAGE_DISPLAY(Vector_get_index_address,data_add_ptr);
		return -ERROR_NULL_INPUT_PTR;
	}
	if(index >= vector_ptr->max_len)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(Vector_get_index_address,index);
		return -ERROR_VALUELESS_INPUT;
	}
#endif
	*data_add_ptr = (void *)((char*)(vector_ptr->data_ptr) + index * vector_ptr->data_size);
	return FUN_EXECUTE_SUCCESSFULLY;
}

/*
delete vector
 */
inline int Vector_delete(Vector *vector_ptr)
{
	ASSERT(NULL != vector_ptr);
#if CONFIG_PARA_CHECK
	if(NULL == vector_ptr)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(Vector_delete,vector_ptr);
		return -ERROR_NULL_INPUT_PTR;
	}
#endif
	f_free(vector_ptr->data_ptr);
	return FUN_EXECUTE_SUCCESSFULLY;
}

/*
delete the data between index "from" to "to"
 */
int Vector_erase_data(Vector *vector_ptr,unsigned int from,unsigned int to)
{
	ASSERT(NULL != vector_ptr);
#if CONFIG_PARA_CHECK
	if(NULL == vector_ptr)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(Vector_erase_data,vector_ptr);
		return -ERROR_NULL_INPUT_PTR;
	}
#endif
	ASSERT(from <= to);
#if CONFIG_PARA_CHECK
	if(from > to)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(Vector_erase_data,from);
		OS_ERROR_PARA_MESSAGE_DISPLAY(Vector_erase_data,to);
		return -ERROR_VALUELESS_INPUT;
	}
#endif
	/*else (from <= to)*/
	ASSERT(to < vector_ptr->cur_len);
#if CONFIG_PARA_CHECK
	if(to >= vector_ptr->cur_len)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(Vector_erase_data,to);
		return -ERROR_VALUELESS_INPUT;
	}
#endif
	f_memcpy((char*)(vector_ptr->data_ptr) + from * vector_ptr->data_size,
		(char*)(vector_ptr->data_ptr) + (to + 1) * vector_ptr->data_size,
		(vector_ptr->cur_len - to - 1)*vector_ptr->data_size);
	vector_ptr->cur_len -= to - from + 1;
	/*add memory management here*/
	return FUN_EXECUTE_SUCCESSFULLY;
}

/*
remove the data Vector[index] and store it into *data_store_ptrs
 */
/*if data_store_ptr is NULL,do not store*/
int Vector_remove_index_data(Vector *vector_ptr,unsigned int index,void *data_store_ptr)
{
	ASSERT(NULL != vector_ptr);
	ASSERT(index < vector_ptr->cur_len);
#if CONFIG_PARA_CHECK
	if(NULL == vector_ptr)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(Vector_remove_index_data,vector_ptr);
		return -ERROR_NULL_INPUT_PTR;
	}
	if(index >= vector_ptr->cur_len)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(Vector_remove_index_data,index);
		return -ERROR_VALUELESS_INPUT;
	}
#endif
	if(data_store_ptr)
	{
		f_memcpy(data_store_ptr,(char*)(vector_ptr->data_ptr) + index * vector_ptr->data_size,vector_ptr->data_size);
	}
	f_memcpy((char*)(vector_ptr->data_ptr) + index * vector_ptr->data_size,
		(char*)(vector_ptr->data_ptr) + (index + 1) * vector_ptr->data_size,
		(vector_ptr->cur_len - index - 1)*vector_ptr->data_size);
	--vector_ptr->cur_len;
	/*add memory management here*/
	return FUN_EXECUTE_SUCCESSFULLY;
}

/*
	set Vector[dest_index] = Vector[src_index]
 */ 
int Vector_set_inner(Vector *vector_ptr,unsigned int dest_index,unsigned int src_index)
{
	ASSERT(NULL != vector_ptr);
	ASSERT((dest_index < vector_ptr->cur_len) && (src_index < vector_ptr->cur_len));
#if CONFIG_PARA_CHECK
	if(NULL == vector_ptr)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(Vector_set_inner,vector_ptr);
		return -ERROR_NULL_INPUT_PTR;
	}
	if((dest_index >= vector_ptr->cur_len) || (src_index >= vector_ptr->cur_len))
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(Vector_set_inner,dest_index);
		OS_ERROR_PARA_MESSAGE_DISPLAY(Vector_set_inner,src_index);
		return -ERROR_VALUELESS_INPUT;
	}
#endif
	if(dest_index == src_index)
	{
		return FUN_EXECUTE_SUCCESSFULLY;
	}
	f_memcpy((char*)(vector_ptr->data_ptr) + dest_index * vector_ptr->data_size,
		(char*)(vector_ptr->data_ptr) + src_index * vector_ptr->data_size,
		vector_ptr->data_size);
	return FUN_EXECUTE_SUCCESSFULLY;
}

/*
swap the data between index "index1" and "index2"
 */
int Vector_swap_inner(Vector *vector_ptr,unsigned int index1,unsigned int index2)
{
	ASSERT(NULL != vector_ptr);
	ASSERT((index1 < vector_ptr->cur_len) && (index2 < vector_ptr->cur_len));
#if CONFIG_PARA_CHECK
	if(NULL == vector_ptr)
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(Vector_swap_inner,vector_ptr);
		return -ERROR_NULL_INPUT_PTR;
	}
	if((index1 >= vector_ptr->cur_len) || (index2 >= vector_ptr->cur_len))
	{
		OS_ERROR_PARA_MESSAGE_DISPLAY(Vector_swap_inner,index1);
		OS_ERROR_PARA_MESSAGE_DISPLAY(Vector_swap_inner,index2);
		return -ERROR_VALUELESS_INPUT;
	}
#endif
	if(index1 == index2)
	{
		return FUN_EXECUTE_SUCCESSFULLY;
	}
	void *buffer = f_malloc(vector_ptr->data_size);
	if(NULL == buffer)
		return -ERROR_ALLOCATE_ROOM;
	f_memcpy(buffer,
		(char*)(vector_ptr->data_ptr) + index1 * vector_ptr->data_size,
		vector_ptr->data_size);
	f_memcpy((char*)(vector_ptr->data_ptr) + index1 * vector_ptr->data_size,
		(char*)(vector_ptr->data_ptr) + index2 * vector_ptr->data_size,
		vector_ptr->data_size);
	f_memcpy((char*)(vector_ptr->data_ptr) + index2 * vector_ptr->data_size,
		buffer,
		vector_ptr->data_size);
	f_free(buffer);
	return FUN_EXECUTE_SUCCESSFULLY;
}
