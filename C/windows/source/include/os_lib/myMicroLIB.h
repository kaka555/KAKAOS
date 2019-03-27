#ifndef _MYMICROLIB_H
#define _MYMICROLIB_H
#include <ka_configuration.h>

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

#if CONFIG_MALLOC && CONFIG_ASSERT_DEBUG
	void *KA_MALLOC(unsigned int size);
	void _KA_FREE(void *ptr,const char* file_name,unsigned line,const char* function_name);
	#define KA_FREE(ptr)    _KA_FREE(ptr,__FILE__,__LINE__,__FUNCTION__)
#else
	void *ka_malloc(unsigned int size);
	void ka_free(void *ptr);
	#define KA_MALLOC(size) ka_malloc(size)
	#define KA_FREE(ptr) ka_free(ptr)
#endif

#endif
