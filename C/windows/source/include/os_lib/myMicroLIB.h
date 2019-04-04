#ifndef _MYMICROLIB_H
#define _MYMICROLIB_H
#include <ka_configuration.h>
#include <kakaosstdint.h>

#define IS_UPPER(x) (x>='A' && x<='Z')
#define IS_LOWER(x) (x>='a' && x<='z')
#define IS_NUM(x)   (x>='0' && x<='9')
#define IS_HEX(x)	(x>='a' && x<='f')
#define IS_DOT(x)   ('.' == x)


void __init_my_micro_lib(void);
int __attribute__((format(printf,1,2))) ka_printf(const char *str,...);
unsigned int ka_strlen(const char *s);
void ka_memcpy(void *dest, const void *src, int n);
void ka_putchar(const char ch);
void ka_puts(const char *string);
unsigned long ka_pow(int x,unsigned int y);
int ka_strncmp(const char * str1, const char * str2, int num);
void ka_memset(void *s,const int ch, int n);
void ka_strcpy(char *strDest, const char *strSrc);
int ka_atoi(const char *char_ptr);
double ka_atof(const char *char_ptr);
int ka_strcmp(const char * str1, const char * str2);

/* flag of malloc */
#define FLAG_MALLOC_NORMAL  	0X00
/* bit 0 */
#define FLAG_MALLOC_FIRST  		0X01  /* request the highest priority's memory */
/* bit 1 */
#define FLAG_MALLOC_NOT_FIRST  	0X02  /* request not the highest priority's memory */
/* end of flag of malloc */

#if CONFIG_MALLOC && CONFIG_ASSERT_DEBUG
	void *ka_malloc(unsigned int size);
	void KA_FREE(void *ptr,const char* file_name,unsigned line,const char* function_name);
	#define ka_free(ptr)    KA_FREE(ptr,__FILE__,__LINE__,__FUNCTION__)

#define DEBUG_MAGIC "bug"
struct malloc_debug_record
{
	UINT32 req_size;
	UINT32 provide_size;
	char magic[4];
};
void add_debug_info(unsigned int req_size,unsigned int provide_size,void *ptr);
#else
	void *_ka_malloc(unsigned int size);
	void _ka_free(void *ptr);
	#define ka_malloc(size) _ka_malloc(size)
	#define ka_free(ptr) _ka_free(ptr)
#endif

#endif
