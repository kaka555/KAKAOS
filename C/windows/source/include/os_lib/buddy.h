#ifndef _BUDDY_H
#define _BUDDY_H

#include <bsp.h>
#include <kakaosstdint.h>
#include <singly_linked_list.h>

#define PAGE_SIZE_BYTE 	1024 //bytes
#define PAGE_SIZE_KB	(PAGE_SIZE_BYTE/1024) //KB
#define NOTHING 		OS_UINT16_MAX

#define sizeof_level(i) (0x0001<<(i-1))

#define BUDDY_SPACE_SIZE (PAGE_SIZE_BYTE*(current_used_buddy_ptr->info.page_num)/sizeof(Buddy_Space_Type))

typedef struct{
	int a[PAGE_SIZE_BYTE/sizeof(int)];
}buddy_space_struct;

typedef UINT32 Flag_Type;
typedef UINT16 Page_Num_Type;
typedef buddy_space_struct Buddy_Space_Type;


struct buddy_info{
	unsigned int flag_array_num;
	unsigned int page_num;
	unsigned int max_level;
	unsigned int prio;
	unsigned int level_flag_base_size;
};

struct order_link{
	Page_Num_Type num;		//indicate page num
	Page_Num_Type next;		//indicate the next struct order_link's sequence number
};

struct page_alloc_record{
	unsigned int level;
	void *ptr;
	struct singly_list_head list;
};

/**************example******************
* flag allocation:          two buddy
* 0-511      :  level 1    :  2K      :   512bits  : 0 -15
* 511-767    :  level 2    :  4K      :   256bits  : 16-23
* 768-895    :  level 3    :  8K      :   128bits  : 24-27
* 896-959    :  level 4    :  16K     :   64bits
* 960-991    :  level 5    :  32K     :   32bits
* 992-1007   :  level 6    :  64K     :   16bits
* 1008-1015  :  level 7    :  128k    :   8bits
* 1016-1019  :  level 8    :  256k    :   4bits
* 1020-1021  :  level 9    :  512k    :   2bits
* 1022-1022  :  level 10   :  1024k   :   1bit
* ***************************************/
struct buddy{
	Flag_Type *flag;					//flag[FLAG_ARRAY_NUM]
	struct order_link *link_body; 	//link_body[PAGE_NUM]
	Page_Num_Type order_link_first;	
	Page_Num_Type *order_link_flag;			//order_link_flag[PAGE_NUM],indicate the next free link_body
	Page_Num_Type *order_array;		//order_array[MAX_LEVEL]
	UINT16 *level_flag_base;		//level_flag_base[]
	unsigned int buddy_struct_size;	//used spaces without sizeof(struct buddy)
	unsigned int current_page_num;
	struct buddy_info info;
	struct buddy *next;
	Buddy_Space_Type *buddy_space_start_ptr;
};

void *alloc_power1_page(void);
void *alloc_power2_page(void);
void *alloc_power3_page(void);
void *alloc_power4_page(void);
void *alloc_power5_page(void);
void *alloc_power6_page(void);
void *alloc_power7_page(void);
void *alloc_power8_page(void);
void *alloc_power9_page(void);
void *alloc_power10_page(void);

void return_power1_page(void *ptr);
void return_power2_page(void *ptr);
void return_power3_page(void *ptr);
void return_power4_page(void *ptr);
void return_power5_page(void *ptr);
void return_power6_page(void *ptr);
void return_power7_page(void *ptr);
void return_power8_page(void *ptr);
void return_power9_page(void *ptr);
void return_power10_page(void *ptr);

void buddy_init(const struct dev_mem_para *dev_mem_para_ptr);
void shell_buddy_debug(int argc, char const *argv[]);
const struct buddy *get_os_buddy_ptr_head(void);
const struct buddy *get_next_buddy_ptr_head(const struct buddy *buddy_ptr);
unsigned int get_current_buddy_space(void);

#endif
