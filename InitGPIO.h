#ifndef INITGPIO_H
#define INITGPIO_H

/*
    GPIO initializations for the buttons, joystick, and other devices, except for the LCD.
*/

#include <stdint.h>

// SW1: PF4
// SW2: PF0
void InitGPIO_TivaButtons(void);

// SW1: PD6
// SW2: PD7
void InitGPIO_EdumkiiButtons(void);

// X: PB5
// Y: PD3
// SEL: PE4
void InitGPIO_EdumkiiJoystick(void);

//void EdumkiiLEDInit(void);

#endif // INITGPIO_H
