#ifndef TIVA_GE_H
#define TIVA_GE_H

/*
    Game engine. Provides functions for setup and the gameloop, which handles input for the user.
    Graphics have to be handled by the game running, which can be set using a function pointer. This
    function is run continuously and takes the elapsed time since the last loop.
*/

#include "InitGPIO.h"
#include "LCD.h"
#include "Input.h"
#include "systick.h"
#include "tiva-gc-inc.h"

typedef struct GE_Button
{
    uint8_t pressed, held;
} GE_Button;

extern GE_Button SW1, SW2, SEL;

typedef struct GE_Joystick
{
    point pos;
    uint8_t changed;
    uint8_t up, down, left, right;
    point threshold;;
} GE_Joystick;

extern GE_Joystick JS;

// function pointer main menu

// function pointer current game, can be null

// Performs input and screen initializations and clears screen to black.
void GE_Setup(void);

// Set the main menu function, without it the engine will not function properly as
// it is the function responsible for setting which game will be run.
//  Param:
//      func: pointer to void function
void GE_SetMainMenu(void (*func)(void));

// Set update function/game
// The logic for the game must be in a function to be looped by the game engine.
// Input is handled by the engine, and the elapsed time is given as a parameter.
// The function must return:
//  1: to keep looping
//  0: to break the loop and return to the main menu
// Param:
//      func: pointer to function taking one float parameter and returning int
void GE_SetUpdate(int (*func)(void));

// Runs the main gameloop
void GE_Loop(void);

// Get time from SysTick
// Return:
//      Ticks since last reload
int GE_STGet(void);

// Get time from SysTick and reload
// Return:
//      Ticks since last reload
int GE_STPop(void);

// See if SysTick reloaded since the last time this function was called
// Return:
//      1 if reloaded, 0 if not
char GE_STGetCount(void);

#endif // TIVA_GE_H