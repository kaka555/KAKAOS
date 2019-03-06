#ifndef _BARRIER_H
#define _BARRIER_H

#define barrier() __asm__ __volatile__("" ::: "memory")

#endif
