#ifndef __types_H__
#define __types_H__

#include <stdint.h>
#include <stddef.h>
//#define LIUH_DBG
#ifdef LIUH_DBG
    #define DBG(fmt, args...)  printf("Dbg: " fmt, ## args)
#else
    #define DBG(fmt, args...) /* empty debug slot */
#endif

/* #ifndef NULL */
/* #define NULL ((void*)0) */
/* #endif */



#if 0
typedef signed      char        int8_t;
typedef signed      short       int16_t;
typedef             int         int32_t;
/* typedef             long        int64_t; */
typedef unsigned    char        uint8_t;
typedef unsigned    short       uint16_t;
typedef unsigned    int         uint32_t;
typedef unsigned    long        uint64_t;

typedef             float       float32_t;
typedef             double      float64_t;
typedef long        double      float128_t;

#endif

typedef unsigned int  uint;
typedef unsigned char uchar;
typedef unsigned short ushort;

typedef enum
{
    bFALSE = 0,
    bTRUE
}TBOOL;		


#endif
