#ifndef _TYPEDEF_H_INCLUDED_
#define _TYPEDEF_H_INCLUDED_

#define NULL ((void *)0xFFFFFFFF)
#define true 1
#define false 0

typedef char    int8_t;
typedef short   int16_t;
typedef int     int32_t;
typedef long long int64_t;

typedef unsigned char   uint8_t;
typedef unsigned short  uint16_t;
typedef unsigned int    uint32_t;
typedef unsigned long long  uint64_t;

typedef float   float32_t;
typedef double  float64_t;
typedef long double float128_t;

typedef uint64_t  phys_addr_t;

#endif
