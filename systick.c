#include "systick.h"
#include "TM4C123GH6PM.h"

void SysTick_Init(int n, char intEn)
{
    SysTick->LOAD = (n & 0xFFFFFF) - 1;
    SysTick->VAL = 0;
    SysTick->CTRL = 0x05 | ((intEn) ? (1 << 1) : 0);
}
