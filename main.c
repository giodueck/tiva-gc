#include <stdint.h>
#include <string.h>
#include "TM4C123GH6PM.h"
#include "tiva-gc.h"

void menu(void);
int snake(float elapsedTime);

void menu()
{
    static uint8_t drawn = 0;
    
    uint8_t nOptions = 3;
    char *options[] = {
        "Settings ",
        "Option 2 ",
        "Option 3 "
    };
    static int choice = 0, chosen = -1, held = 0;
    static GE_Joystick JS_old = {0};

    uint8_t changed = 0;

    GE_Button *menuSelect = &SEL;

    // One time drawing
    if (!drawn)
    {
        drawn = 1;
        LCD_gFillRect(3, 3, LCD_WIDTH - 6, 7, LCD_DARK_GREY);
        LCD_SetBGColor(LCD_RED);
        LCD_gString(0, 0, "Menu V1", 0, LCD_RED);

        LCD_gFillRect(3, LCD_HEIGHT - 10, LCD_WIDTH - 6, 7, LCD_DARK_GREY);
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

        // Draw chosen
        if (chosen >= 0)
        {
            if (held)
                LCD_SetBGColor(LCD_CYAN);
            else
                LCD_SetBGColor(LCD_TURQUOISE);
            
            LCD_gString(2 + strlen(options[0]), 2, options[chosen], 9, LCD_BLACK);
            
            LCD_SetBGColor(LCD_BLACK);
        }
    }
}

int main()
{
    GE_Setup();

    GE_SetMainMenu(menu);
    
    GE_Loop();
}
