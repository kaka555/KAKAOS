#include <stdarg.h>
#include <bsp_usart.h>
#include <myMicroLIB.h>
#include <myassert.h>
#include <kakaosstdint.h>
#include <os_cpu.h>
#include <export.h>

unsigned long ka_pow(int x,unsigned int y)
{
	unsigned long sum = 1;
	while(y--)
	{
		sum *= x;
	}
	return sum;
}

static void m_putstr(const char *str)  
{  
    while(*str)  
    {  
        ka_putchar(*str++);  
    }
}

void ka_puts(const char *string)
{
	while(*string)  
    {  
        ka_putchar(*string++);  
    }
    ka_putchar('\r');
    ka_putchar('\n');
}

unsigned int ka_strlen(const char *s)
{
	unsigned int num = 0;
	while('\0' != *s++)
	{
		++num;
	}
	return num;
}

void ka_memcpy(void *dest, const void *src, int n)
{
	int i;
	int *ptr1 = (int *)dest;
	const int *ptr2 = (const int *)src;
	int num = n>>2;
	for(i=num-1;i>=0;--i)
	{
		*ptr1++ = *ptr2++; /* from low to high*/
	}
	num = n & 3;
	if(num)
	{
		char *ptr3 = (char *)ptr1;
		const char *ptr4 = (char *)ptr2;
		for(i=num-1;i>=0;--i)
		{
			*ptr3++ = *ptr4++;
		}
	}
}


int ka_printf(const char *str,...)
{
	va_list ap;
	int val,r_val;
	double val_f;
	long val_l,r_val_l;
	unsigned long val_l1,r_val_l1;
	unsigned int val1,r_val1;
	char count,ch;
	char *s = NULL;
	int res = 0;
	int buffer;
	
	va_start(ap,str);
	while('\0' != *str)
	{
		switch(*str)  
	    { 
			case '%':
				str++;
				switch(*str)  
				{
					case 'i':
				 	case 'd': /* decimal*/
				 		val = va_arg(ap, int); 
				 		if(0 == val) 
				 		{
				 			ka_putchar('0');
				 			break;
				 		}
						if(val < 0)
						{
							ka_putchar('-');
							val = -val;
						}
	                    r_val = val;   
	                    count = 0;
	                    while(r_val)
	                    {   
	                         count++;
	                         r_val /= 10;  
	                    } 
	                    res += count; 
	                    r_val = val;  
	                    while(count)  
	                    {   
	                          ch = r_val / ka_pow(10,count - 1);  
	                          r_val %= ka_pow(10,count - 1); 
	                          ka_putchar(ch + '0');
	                          count--;  
	                    }  
	                    break; 

	                case 'f': /*float*/
	                	val_f = va_arg(ap, double);
	                	if(val_f < 0)
	                	{
	                		ka_putchar('-');
	                		val_f = -val_f;
	                	}
	                	val = (int)val_f;
	                	val_f -= val;
	                	if(0 == val) 
				 		{
				 			ka_putchar('0');
				 		} 
				 		r_val = val;   
	                    count = 0;
	                    while(r_val)
	                    {   
	                         count++;
	                         r_val /= 10;  
	                    } 
	                    res += count; 
	                    r_val = val;  
	                    while(count)  
	                    {   
	                          ch = r_val / ka_pow(10,count - 1);  
	                          r_val %= ka_pow(10,count - 1); 
	                          ka_putchar(ch + '0');
	                          count--;  
	                    }
	                    /* the upper is integer part*/
	                    ka_putchar('.');
	                    /*the following is decimal part*/
	                    count = 6+1;
	                    while(--count)
	                    {
	                    	val_f *= 10;
	                    	val = (int)val_f;
	                    	ASSERT(val<10 && val>-10);
	                    	ka_putchar(val + '0');
							val_f -= val;
	                    }
	                    break ;

	                case 'l': /* decimal*/
	                	++str;
	                	switch(*str)
	                	{
	                		case 'd':
		                		val_l = va_arg(ap, long); 
						 		if(0 == val_l) 
						 		{
						 			ka_putchar('0');
						 			break;
						 		} 
						 		if(val_l < 0)
						 		{
						 			val_l = -val_l;
						 			ka_putchar('-');
						 		}
			                    r_val_l = val_l;   
			                    count = 0;
			                    while(r_val_l)
			                    {   
			                         count++;
			                         r_val_l /= 10;  
			                    } 
			                    res += count; 
			                    r_val_l = val_l;  
			                    while(count)  
			                    {   
			                          ch = r_val_l / ka_pow(10,count - 1);  
			                          r_val_l %= ka_pow(10,count - 1); 
			                          ka_putchar(ch + '0');
			                          count--;  
			                    }  
			                    break;

		                    case 'u':
		                		val_l1 = va_arg(ap, long); 
						 		if(0 == val_l1) 
						 		{
						 			ka_putchar('0');
						 			break;
						 		} 
						 		if(val_l1 < 0)
						 		{
						 			val_l1 = -val_l1;
						 			ka_putchar('-');
						 		}
			                    r_val_l1 = val_l1;   
			                    count = 0;
			                    while(r_val_l1)
			                    {   
			                         count++;
			                         r_val_l1 /= 10;  
			                    } 
			                    res += count; 
			                    r_val_l1 = val_l1;  
			                    while(count)  
			                    {   
			                          ch = r_val_l1 / ka_pow(10,count - 1);  
			                          r_val_l1 %= ka_pow(10,count - 1); 
			                          ka_putchar(ch + '0');
			                          count--;  
			                    }  
			                    break;

			                case 'x': /* hexadecimal*/
			                 	val_l1 = va_arg(ap, unsigned long); 
			                 	if(0 == val_l1) 
						 		{
						 			ka_puts("0x00");
						 			break;
						 		} 
			                    r_val_l1 = val_l1;   
			                    count = 0;  
			                    while(r_val_l1)  
			                    {   
			                         count++; /*整数的长度   */
			                         r_val_l1 /= 16;   
			                    }
			                    res += count;/*返回值长度增加​   */
			                    r_val_l1 = val_l1; 
			                    while(count)   
			                    {   
			                        ch = r_val_l1 / ka_pow(16, count - 1);   
			                        r_val_l1 %= ka_pow(16, count - 1);  
			                        if(ch <= 9) 
			                            ka_putchar(ch + '0');
			                        else   
			                            ka_putchar(ch - 10 + 'a');
			                        count--;  
			                    }
			                    break;

			                default :
				                ka_putchar(*str);
		            			res += 1;
	                	}
				 		break;

	                case 'u': /* decimal*/
				 		val1 = va_arg(ap,unsigned int);  
				 		if(0 == val1) 
				 		{
				 			ka_putchar('0');
				 			break;
				 		} 
	                    r_val1 = val1;   
	                    count = 0;
	                    while(r_val1)
	                    {   
	                         count++;
	                         r_val1 /= 10;  
	                    } 
	                    res += count; 
	                    r_val1 = val1;  
	                    while(count)  
	                    {   
	                          ch = r_val1 / ka_pow(10,count - 1);  
	                          r_val1 %= ka_pow(10,count - 1); 
	                          ka_putchar(ch + '0');
	                          count--;  
	                    }  
	                    break;

	                case 'o': /* octal*/
				 		val1 = va_arg(ap,unsigned int);  
				 		if(0 == val1) 
				 		{
				 			ka_putchar('0');
				 			break;
				 		} 
	                    r_val1 = val1;   
	                    count = 0;
	                    while(r_val1)
	                    {   
	                        count++;
	                        r_val1 /= 8;  
	                    } 
	                    res += count; 
	                    r_val1 = val1;  
	                    while(count)  
	                    {   
	                        ch = r_val1 / ka_pow(8,count - 1);  
	                        r_val1 %= ka_pow(8,count - 1); 
	                        ka_putchar(ch + '0');
	                        count--;  
	                    }  
	                    break;

	                case 'x': /* hexadecimal*/
	                 	val = va_arg(ap, int); 
	                 	if(0 == val) 
				 		{
				 			ka_puts("0x00");
				 			break;
				 		} 
	                    r_val = val;   
	                    count = 0;  
	                    while(r_val)  
	                    {   
	                         count++; /*整数的长度   */
	                         r_val /= 16;   
	                    }
	                    res += count;/*返回值长度增加​   */
	                    r_val = val; 
	                    while(count)   
	                    {   
	                        ch = r_val / ka_pow(16, count - 1);   
	                        r_val %= ka_pow(16, count - 1);  
	                        if(ch <= 9) 
	                            ka_putchar(ch + '0');
	                        else   
	                            ka_putchar(ch - 10 + 'a');
	                        count--;  
	                    }
	                    break;


	                case 's': /* string*/
	                    s = va_arg(ap, char *);   
	                    m_putstr(s);  
	                    res += ka_strlen(s);  
	                  	break;


	                case 'c': /* char */
	                    ka_putchar( (char)va_arg(ap, int )); 
	                    res += 1;  
	                    break; 

	                case '%':
	                	ka_putchar('%');
	                	res += 1;  
	                    break;

	                case 'p': /*pointer*/
	                	val = va_arg(ap, int);   
	                    r_val = val;   
	                    count = 0;  
	                    while(r_val)  
	                    {   
	                         count++; /*整数的长度   */
	                         r_val /= 16;   
	                    }
	                    res += count;/*返回值长度增加​   */
	                    r_val = val;
	                    buffer = 8-count;
	                    while(buffer--)
	                    {
	                    	ka_putchar('0');
	                    }
	                    while(count)   
	                    {   
	                        ch = r_val / ka_pow(16, count - 1);   
	                        r_val %= ka_pow(16, count - 1);  
	                        if(ch <= 9) 
	                            ka_putchar(ch + '0');
	                        else   
	                            ka_putchar(ch - 10 + 'a');
	                        count--;  
	                    }
	                    break;

	                default :
	                	ka_putchar(*str);
	            		res += 1;
				}
				break ;

			case '\n':  
					ka_putchar('\r');
	                ka_putchar('\n');   
	                res += 1;
	                break;

	        default :
	            	ka_putchar(*str);
	            	res += 1;
		}
		++str;
	}
	va_end(ap);  
    return res;	
}
EXPORT_SYMBOL(ka_printf);

int ka_strncmp(const char * str1, const char * str2, int num)
{
	if(0 == num)
	{
		return 0;
	}
	while(*str1 == *str2)
	{
		if((0 == --num) || ('\0' == *str1))
		{
			return 0;
		}
		++str1;
		++str2;
	}
	return *str1 - *str2;
}

void ka_memset(void *s,const int ch, int n)
{
	char *ptr = (char *)s;
	const char buffer = (char)ch;
	while(n--)
	{
		*ptr++ = buffer;
	}
}

void ka_strcpy(char *strDest, const char *strSrc)
{
	if(!(strDest && strSrc))
		return;
	while((*strDest++ = *strSrc++));
}

int ka_atoi(const char *char_ptr)
{
	int num = 0;
	int sign = 1;
	if(NULL == char_ptr)
	{
		return 0;
	}
	while(' ' == *char_ptr)
	{
		++char_ptr;
	}
	if('-' == *char_ptr)
	{
		sign = -1;
	}
	if(('-' == *char_ptr) || ('+' == *char_ptr))
	{
		++char_ptr;
	}
	while(IS_NUM(*char_ptr))
	{
		num = 10 * num + *char_ptr - '0';
		++char_ptr;
	}
	return (sign*num);
}

double ka_atof(const char *char_ptr)
{
	double num = 0.0;
	int sign = 1;
	double divisor = 10.0;
	if(NULL == char_ptr)
	{
		return 0.0;
	}
	while(' ' == *char_ptr)
	{
		++char_ptr;
	}
	if('-' == *char_ptr)
	{
		sign = -1;
	}
	if(('-' == *char_ptr) || ('+' == *char_ptr))
	{
		++char_ptr;
	}
	while(IS_NUM(*char_ptr))
	{
		num = divisor * num + *char_ptr - '0';
		++char_ptr;
	}
	if('.' != *char_ptr)
	{
		return (sign * num);
	}
	++char_ptr;
	while(IS_NUM(*char_ptr))
	{
		num += (double)(*char_ptr - '0') / divisor;
		divisor *= 10.0;
		++char_ptr;
	}
	return (sign * num);
}

int ka_strcmp(const char * str1, const char * str2)
{
	while(*str1 == *str2)
	{
		if('\0' == *str1)
		{
			return 0;
		}
		++str1;
		++str2;
	}
	return (*str1 - *str2);
}

extern void init_malloc(void);
void __init_my_micro_lib(void)
{
	init_malloc();
}


