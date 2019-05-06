#include <heap_oo.h>
#include <myassert.h>
#include <os_error.h>
#include <myMicroLIB.h>
#include <ka_configuration.h>
#include <kakaosstdint.h>

/* this file realize the heap, some component of OS use it to to do some 
   management job */

#define MIN(a,b) ((a) < (b) ? (a) : (b))

int heap_init(struct little_heap *const little_heap_ptr,
	unsigned int size, /* parameter for vector*/
	int (*cmp)(Vector *Vector_ptr,unsigned int index1,unsigned int index2),
	void (*index_change_record)(Vector *Vector_ptr,int index))
{
	int ret;
	ASSERT((NULL != little_heap_ptr) && (NULL != cmp),ASSERT_INPUT);
	ret = Vector_init(&little_heap_ptr->data,size,MKVFMUL(2));
	if(ret < 0)
	{
		return ret;
	}
	little_heap_ptr->data.cur_len = 1;
	little_heap_ptr->cmp = cmp;
	little_heap_ptr->index_change_record = index_change_record;
	return FUN_EXECUTE_SUCCESSFULLY;
}

/* subfunction for heap sort*/
static void heap_adjust(struct little_heap *little_heap_ptr,int index,int size)
{
	int j;
	Vector *Vector_ptr = &little_heap_ptr->data;
	Vector_set_inner(Vector_ptr,0,index);
	for(j=2*index;j<=size;j*=2)
	{
		if(j<size && (little_heap_ptr->cmp(Vector_ptr,j+1,j) < 0))
			++j;
		if(little_heap_ptr->cmp(Vector_ptr,0,j) <= 0)
			break;
		Vector_set_inner(Vector_ptr,index,j);
		if(little_heap_ptr->index_change_record)
		{
			little_heap_ptr->index_change_record(&little_heap_ptr->data,index);
		}
		index = j;
	}
	Vector_set_inner(Vector_ptr,index,0);
	if(little_heap_ptr->index_change_record)
	{
		little_heap_ptr->index_change_record(&little_heap_ptr->data,index);
	}
}

/*add a data with push_data_ptr, then adjust the heap */
int heap_push(struct little_heap *little_heap_ptr,void *push_data_ptr)
{
	ASSERT((NULL != little_heap_ptr) && (NULL != push_data_ptr),ASSERT_INPUT);
	int ret,i,size;
	ret = Vector_push_back(&little_heap_ptr->data,push_data_ptr);
	if(little_heap_ptr->index_change_record)
	{
		little_heap_ptr->index_change_record(&little_heap_ptr->data,heap_get_cur_len(little_heap_ptr)-1);
	}
	ASSERT(0 == ret,ASSERT_PARA_AFFIRM);
	if(ret < 0)
		return ret;
	size = get_Vector_cur_len(&little_heap_ptr->data) - 1;
	for(i=size/2;i>0;i/=2)
	{
		heap_adjust(little_heap_ptr,i,size);
	}
	return FUN_EXECUTE_SUCCESSFULLY;
}

int heap_get_index_data_safe(struct little_heap *little_heap_ptr,void **data_store_ptr,unsigned int index)
{
	ASSERT((NULL != little_heap_ptr) && (NULL != data_store_ptr),ASSERT_INPUT);
	if(get_Vector_cur_len(&little_heap_ptr->data) <= index)
	{
		return -ERROR_USELESS_INPUT;
	}
	*data_store_ptr = Vector_get_index_data(&little_heap_ptr->data,index);
	return FUN_EXECUTE_SUCCESSFULLY;
}

/*get the data with index "index" with data *data_store_ptr; then adjust the heap*/
int heap_set_index_data(struct little_heap *little_heap_ptr,unsigned int index,void *data_store_ptr)
{
	ASSERT((NULL != little_heap_ptr) && (NULL != data_store_ptr),ASSERT_INPUT);
	Vector_set_index_data(&little_heap_ptr->data,index,data_store_ptr);
	unsigned int size = get_Vector_cur_len(&little_heap_ptr->data) - 1;
	unsigned int i;
	for(i=MIN(index,size/2);i>0;--i)
	{
		heap_adjust(little_heap_ptr,i,size);
	}
	return FUN_EXECUTE_SUCCESSFULLY;
}

/*remove the data with index "index" and store it into *data_store_ptr; then adjust the heap*/
int heap_remove_index_data(struct little_heap *little_heap_ptr,unsigned int index,void *data_store_ptr)
{
	ASSERT(NULL != little_heap_ptr,ASSERT_INPUT);
	ASSERT(0 != index,ASSERT_INPUT);
	int ret,i;
	ret = Vector_remove_index_data(&little_heap_ptr->data,index,data_store_ptr);
	if(ret < 0)
	{
		return ret;
	}
	/*rebuild heap*/
	int size = get_Vector_cur_len(&little_heap_ptr->data) - 1;
	for(i=size/2;i>0;--i)
	{
		heap_adjust(little_heap_ptr,i,size);
	}
	return FUN_EXECUTE_SUCCESSFULLY;
}

/*erase the data between index "from" to "to"; then adjust the heap*/
int heap_erase_data(struct little_heap *little_heap_ptr,unsigned int from,unsigned int to)
{
	ASSERT(NULL != little_heap_ptr,ASSERT_INPUT);
	int ret,i;
	ret = Vector_erase_data(&little_heap_ptr->data,from,to);
	if(ret < 0)
	{
		return ret;
	}
	/*rebuild heap*/
	int size = get_Vector_cur_len(&little_heap_ptr->data) - 1;
	for(i=size/2;i>0;--i)
	{
		heap_adjust(little_heap_ptr,i,size);
	}
	return FUN_EXECUTE_SUCCESSFULLY;
}

