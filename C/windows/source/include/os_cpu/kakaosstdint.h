#ifndef   __STDINT_H
#define   __STDINT_H

//the following is the typedef which help the transplantation
typedef   signed          char INT8;
typedef   signed short     int INT16;
typedef   signed           int INT32;
typedef   signed     long long INT64;

typedef   unsigned        char UINT8;
typedef   unsigned short   int UINT16;
typedef   unsigned         int UINT32;
typedef   unsigned   long long UINT64;
    /* minimum values of minimum-width signed integer types */
#define OS_INT8_MIN                   -128
#define OS_INT16_MIN                -32768
#define OS_INT32_MIN          (~0x7fffffff)
#define OS_INT_MIN  __ESCAPE__(~0x7fffffffffffffffll)

    /* maximum values of minimum-width signed integer types */
#define OS_INT8_MAX                    127
#define OS_INT16_MAX                 32767
#define OS_INT32_MAX            2147483647
#define OS_INT64_MAX  __ESCAPE__(9223372036854775807ll)

    /* maximum values of minimum-width unsigned integer types */
#define OS_UINT8_MAX                   255
#define OS_UINT16_MAX                65535
#define OS_UINT32_MAX           4294967295u
#define OS_UINT64_MAX __ESCAPE__(18446744073709551615ull)

#undef NULL
#define NULL 0

typedef unsigned int size_t;

#endif
