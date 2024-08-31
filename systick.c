#include "systick.h"
#include "inc/tm4c123gh6pm.h"
#include <stdint.h>

void SysTick_Init(int n, char intEn)
{
    // SysTick->LOAD = (n & 0xFFFFFF) - 1;
    // SysTick->VAL = 0;
    // SysTick->CTRL = 0x05 | ((intEn) ? (1 << 1) : 0);

    // Convert from Keil to Tivaware definitions
    NVIC_ST_CTRL_R = 0;         // (a) disable SysTick during setup
    NVIC_ST_RELOAD_R = n - 1;   // (b) reload value
    NVIC_ST_CURRENT_R = 0;      // (c) any write to current clears it
    NVIC_ST_CTRL_R = 0x05 | ((intEn) ? (1 << 1) : 0); // (e) enable SysTick with core clock and interrupts only if intEn allows it
}
