#ifndef TIVA_GC_INC_H
#define TIVA_GC_INC_H

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))
#define abs(a)    ((a) < 0 ? -(a) : (a))

#define HIGH 1
#define LOW  0
#define ON   1
#define OFF  0

#define NULL ((void *)0)

typedef struct point
{
    int32_t x, y;
} point;

#endif // TIVA_GC_INC_H
