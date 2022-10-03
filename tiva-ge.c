#include "tiva-ge.h"

GE_Button SW1 = {0}, SW2 = {0}, SEL = {0};
GE_Joystick JS = {0};

static void (*_mainMenu)(void) = NULL;
static int (*_update)(float elapsedTime) = NULL;

void GE_Input(void);

// Performs input and screen initializations and clears screen to black.
void GE_Setup(void)
{
    // Input init
    InitGPIO_EdumkiiButtons();
    InitGPIO_EdumkiiJoystick();

    // Output init
    LCD_Init();
    LCD_CS(LOW);
    
    LCD_SetBGColor(LCD_BLACK);
    LCD_gClear();

    // Settings
    JS.threshold = (point) { .x = 1024, .y = 1024 };
}

// Reads from all input buttons and the joystick. Used by the game engine
void GE_Input(void)
{
    uint8_t sw1_s, sw2_s, sel_s;
    point old = JS.pos;

    sw1_s = Input_ReadButton(BUTTON_EDUMKII_SW1);
    sw2_s = Input_ReadButton(BUTTON_EDUMKII_SW2);
    sel_s = Input_ReadButton(BUTTON_EDUMKII_SEL);
    SW1.pressed = Input_ReadButton(BUTTON_EDUMKII_SW1);
    SW2.pressed = Input_ReadButton(BUTTON_EDUMKII_SW2);
    SEL.pressed = Input_ReadButton(BUTTON_EDUMKII_SEL);
    SW1.held = SW1.pressed || sw1_s && Input_ReadButton(BUTTON_EDUMKII_SW1);
    SW2.held = SW2.pressed || sw2_s && Input_ReadButton(BUTTON_EDUMKII_SW2);
    SEL.held = SEL.pressed || sel_s && Input_ReadButton(BUTTON_EDUMKII_SEL);

    JS.pos = Input_ReadJoystick();
    JS.changed = (old.x != JS.pos.x || old.y != JS.pos.y);
    if (JS.changed)
    {
        JS.up = (JS.pos.y < 2048 - JS.threshold.y);
        JS.down = (JS.pos.y > 2048 + JS.threshold.y);
        JS.left = (JS.pos.x < 2048 - JS.threshold.x);
        JS.right = (JS.pos.x > 2048 + JS.threshold.x);
    }
}

// Set the main menu function, without it the engine will not function properly as
// it is the function responsible for setting which game will be run.
//  Param:
//      func: pointer to void function
void GE_SetMainMenu(void (*func)(void))
{
    _mainMenu = func;
}

// Set update function/game
// The logic for the game must be in a function to be looped by the game engine.
// Input is handled by the engine, and the elapsed time is given as a parameter.
// The function must return:
//  1: to keep looping
//  0: to break the loop and return to the main menu
// Param:
//      func: pointer to function taking one float parameter and returning int
void GE_SetUpdate(int (*func)(float elapsed_time))
{
    _update = func;
}

// Runs the main menu and gameloop
void GE_Loop(void)
{
    LCD_SetBGColor(LCD_BLACK);
    LCD_gClear();

    // Checks for menus or games set
    if (!_mainMenu && !_update)
    {
        LCD_gString(0, 0, "E: No main menu", 0, LCD_RED);
        LCD_gString(3, 1, "function set!", 0, LCD_RED);
        while (1);
    }

    // Main program loop
    while (1)
    {
        // Menu
        while (!_update)
        {
            GE_Input();
            _mainMenu();
        }

        // Gameloop
        while (_update)
        {
            GE_Input();
            if (!_update(0.0f))
                _update = NULL;
        }
    }

}
