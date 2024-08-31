#ifndef TIVA_GC_INC_H
#define TIVA_GC_INC_H

#include <stddef.h>
#include <stdint.h>

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))
#define abs(a)    ((a) < 0 ? -(a) : (a))

#define HIGH 1
#define LOW  0
#define ON   1
#define OFF  0

// #define NULL ((void *)0)

#define CLOCKS_PER_SEC 16000000

typedef struct point
{
    int32_t x, y;
} point;

typedef struct fpoint
{
    float x, y;
} fpoint;

#endif // TIVA_GC_INC_H
