#include "delay.h"

void _delay(uint32_t cycles);

void delay(uint32_t ms)
{
    _delay(16000000 / 3000 * ms);
}
