#include <stdint.h>
#include <stdlib.h> // rand
#include "TM4C123GH6PM.h"
#include "tiva-gc.h"

// Indicates a reset is needed for the currently selected update function or the menu
static uint8_t fReset = 0;

void menu(void);
int snake(float elapsedTime);
int resetter(float elapsedTime);
int random(float elapsedTime);

void menu()
{
    // Persistent and read-only vars
    static uint8_t drawn = 0;
    const uint8_t nOptions = 3;
    const char *options[] = {
        "Snake    ",
        "Resetter ",
        "Random   "
    };
    const int (*games[])(float elapsedTime) = {
        snake,
        resetter,
        random
    };
    static int choice = 0, chosen = -1, held = 0;
    static GE_Joystick JS_old = {0};

    // Per loop vars
    uint8_t changed = 0;

    // Controls
    static GE_Button *menuSelect = &SEL;

    // Reset to the main menu
    if (fReset)
    {
        drawn = 0;
        choice = 0;
        chosen = -1;
        held = 0;
        fReset = 0;
        LCD_gClear();
        return;
    }

    // One time drawing
    if (!drawn)
    {
        drawn = 1;
        LCD_gClear();
        LCD_gFillRect(0, 0, LCD_WIDTH - 1, 8, LCD_DARK_GREY);
        LCD_SetBGColor(LCD_RED);
        LCD_gString(0, 0, "Menu V1", 0, LCD_RED);

        LCD_gFillRect(0, LCD_HEIGHT - 8, LCD_WIDTH - 1, 7, LCD_DARK_GREY);
        // LCD_gString(13, 15, "Menu V1", 0, LCD_RED);
        LCD_SetBGColor(LCD_BLACK);
        changed = 1;
    }

    // Check inputs
    if (JS.up && !JS_old.up)
    {
        choice--;
        if (choice < 0)
            choice = nOptions - 1;
        changed = 1;
    }
    if (JS.down && !JS_old.down)
    {
        choice++;
        choice %= nOptions;
        changed = 1;
    }
    JS_old = JS;

    if (menuSelect->pressed)
    {
        chosen = choice;
        changed = 1;
    }
    if (menuSelect->held != held)
    {
        held = menuSelect->held;
        changed = 1;
    }

    // Draw
    if (changed)
    {
        // Draw menu elements
        for (int i = 0; i < nOptions; i++)
        {
            if (choice == i)
            {
                if (held)
                    LCD_SetBGColor(LCD_WHITE);
                else
                    LCD_SetBGColor(LCD_LIGHT_GREY);
                LCD_gString(1, 2 + i, options[i], 9, LCD_BLACK);
                LCD_SetBGColor(LCD_BLACK);
            }
            else
            {
                LCD_gString(1, 2 + i, options[i], 9, LCD_LIGHT_GREY);
            }
        }
    }

    // If a choice was made, start the game
    if (chosen > -1)
    {
        GE_SetUpdate(games[chosen]);
        fReset = 1;
    }
}

int resetter(float elapsedTime)
{
    return 0;
}

int random(float elapsedTime)
{
    if (SEL.pressed)
    {
        fReset = 1;
        return 0;
    } else
    {
        fReset = 0;
        LCD_SetArea(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
        LCD_ActivateWrite();
        for (int i = 0; i < LCD_HEIGHT; i++)
            for (int j = 0; j < LCD_WIDTH; j++)
                LCD_PushPixel(rand() % 0x3F, rand() % 0x3F, rand() % 0x3F);
    }
    return 1;
}

int snake(float elapsedTime)
{
    LCD_gClear();
    LCD_gHLine(0, LCD_WIDTH - 1, 0, 1, LCD_WHITE);
    LCD_gHLine(0, LCD_WIDTH - 1, LCD_HEIGHT - 1, 1, LCD_WHITE);
    LCD_gVLine(0, 0, LCD_HEIGHT - 1, 1, LCD_WHITE);
    LCD_gVLine(LCD_WIDTH - 1, 0, LCD_HEIGHT - 1, 1, LCD_WHITE);
    delay(2000);
    return 0;
}

int main()
{
    GE_Setup();

    GE_SetMainMenu(menu);
    
    GE_Loop();
}
