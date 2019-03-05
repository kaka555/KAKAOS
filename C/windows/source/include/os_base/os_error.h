#ifndef _OS_ERROR_H
#define _OS_ERROR_H

#include <myMicroLIB.h>

#define FUN_EXECUTE_SUCCESSFULLY 0

//when the following #define, add a ‘-’ before it
//example:
//return -ERROR_NULL_INPUT_PTR;
#define ERROR_NULL_INPUT_PTR   		0XFF
#define ERROR_VALUELESS_INPUT  		0XFE
#define ERROR_FUN_USE_IN_INTER 		0XFD
#define ERROR_EXECUTE_IN_INTERRUPT  0XFC
#define ERROR_NO_MEM				0XFB
#define ERROR_MODULE_DELETED		0XFA
#define ERROR_LOGIC					0XF9

#define _must_check 
#define PUBLIC  
#define PRIVATE   

#define KA_TRUE		1
#define KA_FALSE	0

#define OS_ERROR_MESSAGE_DISPLAY(text) 			ka_printf(#text)
#define OS_ERROR_PARA_MESSAGE_DISPLAY(fun,para)	ka_printf("error fun name : " #fun "\n""error para : " #para "\n")

#endif
