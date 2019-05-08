#ifndef _BARRIER_H
#define _BARRIER_H

#define barrier() __asm__ __volatile__("" ::: "memory")
#define do_nothing()  NULL
#define __section(S) __attribute__ ((__section__(S)))

#endif
