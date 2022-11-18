#include "tiva-ge.h"
#include "xorshift.h"
#include "delay.h"

GE_Button SW1 = {0}, SW2 = {0}, SEL = {0};
GE_Joystick JS = {0};

static void (*_mainMenu)(void) = NULL;
static int (*_update)(void) = NULL;

static uint32_t xorshift32_state = 0x12345678;

void GE_Input(void);
void GE_SRand(uint32_t seed);
void GE_Intro(void);

int GE_STPop(void)
{
    int t = GE_STGet();
    SysTick->VAL = 0;
    return t;
}

int GE_STGet(void)
{
    return CLOCKS_PER_SEC - SysTick->VAL;
}

char GE_STGetCount(void)
{
    return SysTick->CTRL >> 16;
}

uint32_t GE_Rand(void)
{
    return xorshift32_state = xorshift32(xorshift32_state);
}

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
    
    // SysTick init
    SysTick_Init(CLOCKS_PER_SEC, OFF);

    // RNG seed, 32 bits of noise generated from ADC
    for (int i = 0; i < 8; i++)
    {
        JS.pos = Input_ReadJoystickRaw();
        xorshift32_state ^= ((JS.pos.x & 0x03) << 2) | (JS.pos.y & 0x03);
        xorshift32_state <<= 4;
    }

    // Settings
    JS.threshold = (point) { .x = 1024, .y = 1024 };
}

// Reads from all input buttons and the joystick. Used by the game engine
void GE_Input(void)
{
    uint8_t sw1_s, sw2_s, sel_s;
    point old = JS.pos;

    sw1_s = Input_ReadButtonRaw(BUTTON_EDUMKII_SW1);
    sw2_s = Input_ReadButtonRaw(BUTTON_EDUMKII_SW2);
    sel_s = Input_ReadButtonRaw(BUTTON_EDUMKII_SEL);
    SW1.pressed = Input_ReadButton(BUTTON_EDUMKII_SW1);
    SW2.pressed = Input_ReadButton(BUTTON_EDUMKII_SW2);
    SEL.pressed = Input_ReadButton(BUTTON_EDUMKII_SEL);
    SW1.held = SW1.pressed || (sw1_s && Input_ReadButtonRaw(BUTTON_EDUMKII_SW1));
    SW2.held = SW2.pressed || (sw2_s && Input_ReadButtonRaw(BUTTON_EDUMKII_SW2));
    SEL.held = SEL.pressed || (sel_s && Input_ReadButtonRaw(BUTTON_EDUMKII_SEL));

    JS.pos = Input_ReadJoystick();
    JS.changed = (old.x != JS.pos.x || old.y != JS.pos.y);
    if (JS.changed)
    {
        JS.down = (JS.pos.y < 2048 - JS.threshold.y);
        JS.up = (JS.pos.y > 2048 + JS.threshold.y);
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
void GE_SetUpdate(int (*func)(void))
{
    _update = func;
}

// Show a little intro card, with the project name and a small wireframe of the console
void GE_Intro(void)
{
    LCD_gFillRect(LCD_WIDTH / 8, LCD_HEIGHT / 8, LCD_WIDTH * 3 / 4, LCD_HEIGHT * 3 / 4, LCD_RED);
    LCD_gRect(LCD_WIDTH / 8, LCD_HEIGHT / 8, LCD_WIDTH * 3 / 4, LCD_HEIGHT * 3 / 4, 2, LCD_WHITE);

    LCD_SetBGColor(LCD_RED);
    LCD_gString(7, 3, "Tiva GC", 0, LCD_WHITE);

    // EDUMKII outline
    LCD_gCircle(48, 64, 10, 1, LCD_WHITE);
    LCD_gCircle(80, 64, 10, 1, LCD_WHITE);
    LCD_gFillRect(50, 54, 29, 20, LCD_RED);
    LCD_gLine(49, 54, 79, 54, 1, LCD_WHITE);
    LCD_gLine(49, 74, 79, 74, 1, LCD_WHITE);
    // Tiva outcrop
    LCD_gRect(56, 74, 16, 4, 1, LCD_WHITE);
    // EDUMKII JS
    LCD_gCircle(48, 63, 4, 1, LCD_WHITE);
    LCD_gCircle(48, 63, 5, 1, LCD_WHITE);
    // EDUMKII Buttons
    LCD_gCircle(83, 60, 2, 1, LCD_WHITE);
    LCD_gCircle(83, 65, 2, 1, LCD_WHITE);
    // EDUMKII LCD
    LCD_gRect(60, 61, 8, 10, 1, LCD_WHITE);

    LCD_SetBGColor(LCD_BLACK);

    delay(1500);
}

// Runs the main menu and gameloop
void GE_Loop(void)
{
    LCD_SetBGColor(LCD_BLACK);
    LCD_gClear();
    GE_Intro();

    // Checks for menus or games set
    if (!_mainMenu && !_update)
    {
        LCD_gString(0, 0, "E: No main menu", 0, LCD_RED);
        LCD_gString(3, 1, "function set!", 0, LCD_RED);
        while (1);
    }

    // Let inputs stabilize, especially JS
    for (int i = 0; i < 3; i++)
    {
        GE_Input();
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
            if (!_update())
                _update = NULL;
        }
    }
}
