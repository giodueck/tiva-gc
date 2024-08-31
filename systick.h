#ifndef SYSTICK_H
#define SYSTICK_H
#include "inc/tm4c123gh6pm.h"

#define SYSTICK_LOAD_R (*((volatile uint32_t *)0xE000E014)) // NVIC_ST_RELOAD_R
#define SYSTICK_VAL_R  (*((volatile uint32_t *)0xE000E018)) // NVIC_ST_CURRENT_R
#define SYSTICK_CTRL_R (*((volatile uint32_t *)0xE000E010)) // NVIC_ST_CTRL_R

void SysTick_Init(int n, char intEn);

#endif // SYSTICK_H
