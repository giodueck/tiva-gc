#ifndef TIVA_GC_INC_H
#define TIVA_GC_INC_H

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))
#define abs(a)    ((a) < 0 ? -(a) : (a))

typedef struct
{
    int32_t x, y;
} point;

#endif // TIVA_GC_INC_H