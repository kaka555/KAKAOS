#ifndef _VECTOR_H
#define _VECTOR_H

#include <myMicroLIB.h>

#define f_free(ptr)				ka_free(ptr)
#define f_memcpy(des,src,size)	ka_memcpy(des,src,size)
#define f_malloc(size)			ka_malloc(size)


#define VECTOR_DATA_SIZE sizeof(void *)
typedef struct vector
{
    void **data_ptr; /* type of the element is void* */
    unsigned int max_len;
    unsigned int cur_len;
/*
	if the highest bit is 1, multiply
	if the highest bit is 0, add
	use the following macro to produce this member:MKVFMUL  MKVFADD
 */
    unsigned int extension_factor;
}Vector;

#define ERROR_ALLOCATE_ROOM 1
#define ERROR_DATA_EMPTY    2

/*
get vector[index] data into *data_store_ptr
 */
static inline void * Vector_get_index_data(Vector *vector_ptr,unsigned int index)
{
	return vector_ptr->data_ptr[index];
}

/*
set vector[index] data with *data_store_ptr
 */
static inline void Vector_set_index_data(Vector *vector_ptr,unsigned int index,void *data_store_ptr)
{
	vector_ptr->data_ptr[index] = data_store_ptr;
}

/*
delete vector
 */
static inline void Vector_delete(Vector *vector_ptr)
{
	f_free(vector_ptr->data_ptr);
}

static inline void * Vector_get_index_address(Vector *vector_ptr,unsigned int index)
{
	return &vector_ptr->data_ptr[index];
}

int Vector_init(Vector *vector_ptr,unsigned int size,unsigned int extension_factor);
int Vector_push_back(Vector *vector_ptr,void *push_data_ptr);
int Vector_pop_back(Vector *vector_ptr,void **pop_data_ptr);
void Vector_set_index_data(Vector *vector_ptr,unsigned int index,void *data_store_ptr);
int Vector_erase_data(Vector *vector_ptr,unsigned int from,unsigned int to);
int Vector_remove_index_data(Vector *vector_ptr,unsigned int index,void **data_store_ptr);/*if data_store_ptr is NULL,do not store*/
int Vector_set_inner(Vector *vector_ptr,unsigned int dest_index,unsigned int src_index);
int Vector_swap_inner(Vector *vector_ptr,unsigned int index1,unsigned int index2);


#define MKVFMUL(x) (0x80000000 | (x))
#define MKVFADD(x) (~0x80000000 & (x))
#define VFISMUL(x) (0x80000000 & (x))
#define VFISADD(x) (!(VFISMUL(x)))
#define GET_EXPENSION_FACTOR(x) (~0x80000000 & (x))
#define DECLARE_VECTOR(name,type,len,extension_factor) \
Vector name; \
do{ \
	Vector_init(&(name),len,sizeof(type),extension_factor); \
}while(0)

static inline unsigned int get_Vector_size(Vector *vector_ptr)
{
	return vector_ptr->max_len;
}
static inline unsigned int get_Vector_cur_len(Vector *vector_ptr)
{
	return vector_ptr->cur_len;
}

/*
description: you can put a Vector into a struct,then use Vector_init() to initialize it

or in application's function , use DECLARE_VECTOR
 */

#endif
