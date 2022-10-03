#include <stdint.h>
#include "TM4C123GH6PM.h"
#include "tiva-gc.h"

void GEdemoMenu(void)
{
    pixel colors[21] = {
        LCD_DARK_GREY,
        LCD_GREY,
        LCD_LIGHT_GREY,
        LCD_WHITE,
        LCD_RED,
        LCD_GREEN,
        LCD_BLUE,
        LCD_YELLOW,
        LCD_MAGENTA,
        LCD_CYAN,
        LCD_DARK_RED,
        LCD_DARK_GREEN,
        LCD_DARK_BLUE,
        LCD_DARK_YELLOW,
        LCD_PURPLE,
        LCD_TEAL,
        LCD_BROWN,
        LCD_PINK,
        LCD_TURQUOISE,
        LCD_ORANGE,
        LCD_GOLD
    };
    static uint8_t color = 0;

    LCD_SetBGColor(colors[color]);
    LCD_gString(0, 0, "Inside menu loop", 0, LCD_GREEN);
    LCD_gChar(0, 8, color, LCD_MAGENTA, colors[color], 2);

    color++;
    if (color == 21) color = 0;
    delay(250);
}

int GEdemo(void)
{
    GE_Setup();

    GE_SetMainMenu(GEdemoMenu);
    
    GE_Loop();
}

int textdemo(void)
{
    char c = 0;
    uint8_t x = 0, y = 0;
    char enable = 1;
    pixel colors[21] = {
        LCD_DARK_GREY,
        LCD_GREY,
        LCD_LIGHT_GREY,
        LCD_WHITE,
        LCD_RED,
        LCD_GREEN,
        LCD_BLUE,
        LCD_YELLOW,
        LCD_MAGENTA,
        LCD_CYAN,
        LCD_DARK_RED,
        LCD_DARK_GREEN,
        LCD_DARK_BLUE,
        LCD_DARK_YELLOW,
        LCD_PURPLE,
        LCD_TEAL,
        LCD_BROWN,
        LCD_PINK,
        LCD_TURQUOISE,
        LCD_ORANGE,
        LCD_GOLD
    };
    uint8_t color = 0;

    // Input init
    InitGPIO_EdumkiiButtons();
    InitGPIO_EdumkiiJoystick();

    // Output init
    LCD_Init();
    LCD_CS(LOW);
    
    LCD_SetBGColor(LCD_BLACK);
    LCD_gClear();

    while (1)
    {
        if (Input_ReadButton(BUTTON_EDUMKII_SW1))
        {
            enable = !enable;
        }
        if (Input_ReadButton(BUTTON_EDUMKII_SW2))
        {
            LCD_gClear();
            LCD_gString(3, 4, "Hello! :)", 0, colors[color]);
            delay(1000);
            LCD_gClear();
            x = 0;
            y = 0;
            c = 0;
        }
        if (Input_ReadButton(BUTTON_EDUMKII_SEL))
        {
            color++;
            if (color == 21) color = 0;
        }

        if (!enable)
            continue;
        c++;
        if (x + 12 >= LCD_WIDTH)
        {
            x = 0;
            y += 8;
        }
        else
            x += 6;
        if (y - 8 >= LCD_HEIGHT)
            y = 0;
        LCD_gChar(x, y, c, colors[color], LCD_BLACK, 1);
    }
}

int graphicsdemo(void)
{
    LCD_Settings actSettings;
    point old_pos, pos, js = { 2048, 2048 };    // Joystick centered to begin with
    pixel colors[6] = { LCD_RED, LCD_YELLOW, LCD_GREEN, LCD_CYAN, LCD_BLUE, LCD_MAGENTA };
    uint8_t i = 0, changeFlag = 0;
    point old_rhombus[4], rhombus[4];
    
    // Input init
    InitGPIO_EdumkiiButtons();
    InitGPIO_EdumkiiJoystick();

    // Output init
    LCD_Init();
    LCD_CS(LOW);
    
    LCD_SetBGColor(LCD_BLACK);
    actSettings = LCD_GetSettings();
    js = Input_ReadJoystick();    // don't need that much precision
    js.x = js.x;
    js.y = js.y;
    pos.x = (int32_t)(js.x / 4095.0f * LCD_WIDTH);
    pos.y = LCD_HEIGHT - (int32_t)(js.y / 4095.0f * LCD_HEIGHT);
    if (pos.x < 2)
        pos.x = 2;
    else if (pos.x >= LCD_WIDTH)
        pos.x = LCD_WIDTH - 1;
    if (pos.y < 2)
        pos.y = 2;
    else if (pos.y >= LCD_HEIGHT)
        pos.y = LCD_HEIGHT - 1;

    // rhombus[0].x = pos.x;
    // rhombus[0].y = max(0, pos.y - 5);
    // rhombus[1].x = min(LCD_WIDTH, pos.x + 5);
    // rhombus[1].y = pos.y;
    // rhombus[2].x = pos.x;
    // rhombus[2].y = min(LCD_HEIGHT, pos.y + 5);
    // rhombus[3].x = max(0, pos.x - 5);
    // rhombus[3].y = pos.y;

    LCD_gClear();

    while (1)
    {
        // Detecta pulsaciones de los botones
        if (Input_ReadButton(BUTTON_EDUMKII_SW1))
        {
            i = (i + 1) % 6;    // Cycle through primary and secondary RGB colors
            changeFlag = 1;
        }
        if (Input_ReadButton(BUTTON_EDUMKII_SW2))
        {
            i = (i + 5) % 6;
            changeFlag = 1;
        }
        if (Input_ReadButton(BUTTON_EDUMKII_SEL))
        {
            LCD_SetInversion(!actSettings.InversionMode);
            actSettings.InversionMode = !actSettings.InversionMode;
            changeFlag = 1;
        }

        // Detecta la posicion del joystick y la convierte a un punto en el display
        old_pos = pos;
        js = Input_ReadJoystick();
        js.x = js.x;
        js.y = js.y;
        pos.x = (int32_t)(js.x / 4095.0f * LCD_WIDTH);
        pos.y = LCD_HEIGHT - (int32_t)(js.y / 4095.0f * LCD_HEIGHT);
        if (pos.x < 2)
            pos.x = 2;
        else if (pos.x >= LCD_WIDTH)
            pos.x = LCD_WIDTH - 1;
        if (pos.y < 2)
            pos.y = 2;
        else if (pos.y >= LCD_HEIGHT)
            pos.y = LCD_HEIGHT - 1;
        
        // for (int j = 0; j < 4; j++)
        //     old_rhombus[j] = rhombus[j];

        // rhombus[0].x = pos.x;
        // rhombus[0].y = max(0, pos.y - 5);
        // rhombus[1].x = min(LCD_WIDTH, pos.x + 5);
        // rhombus[1].y = pos.y;
        // rhombus[2].x = pos.x;
        // rhombus[2].y = min(LCD_HEIGHT, pos.y + 5);
        // rhombus[3].x = max(0, pos.x - 5);
        // rhombus[3].y = pos.y;

        // Dibuja sobre el display
        if (old_pos.x != pos.x || old_pos.y != pos.y || changeFlag)
        {
            // Erase previous shapes
            // LCD_gFillRectangle((uint8_t) old_pos.x - 5, (uint8_t) old_pos.y - 5, 10, 10, actSettings.BGColor);
            LCD_gLine(64, 64, (uint8_t) old_pos.x, (uint8_t) old_pos.y, 1, actSettings.BGColor);
            // LCD_gTriangle((point) {old_pos.x, old_pos.y - 5}, (point) {old_pos.x + 5, old_pos.y + 5}, (point) {old_pos.x - 5, old_pos.y + 5}, 1, actSettings.BGColor);
            // LCD_gFillTriangle((point) {old_pos.x, old_pos.y - 5}, (point) {old_pos.x + 5, old_pos.y + 5}, (point) {old_pos.x - 5, old_pos.y + 5}, actSettings.BGColor);
            // LCD_gPolygon(old_rhombus, 4, 1, actSettings.BGColor);
            LCD_gCircle(old_pos.x, old_pos.y, 6, 1, actSettings.BGColor);
            // LCD_gFillCircle(old_pos.x, old_pos.y, 7, actSettings.BGColor);
            
            // Draw new shapes
            // LCD_gRectangle((uint8_t) pos.x - 5, (uint8_t) pos.y - 5, 10, 10, 1, colors[i]);
            LCD_gLine(64, 64, (uint8_t) pos.x, (uint8_t) pos.y, 1, colors[i]);
            // LCD_gTriangle((point) {pos.x, pos.y - 5}, (point) {pos.x + 5, pos.y + 5}, (point) {pos.x - 5, pos.y + 5}, 1, colors[i]);
            // LCD_gFillTriangle((point) {pos.x, pos.y - 5}, (point) {pos.x + 5, pos.y + 5}, (point) {pos.x - 5, pos.y + 5}, colors[i]);
            // LCD_gPolygon(rhombus, 4, 1, colors[i]);
            LCD_gCircle(pos.x, pos.y, 6, 1, colors[i]);
            // LCD_gFillCircle(pos.x, pos.y, 7, colors[i]);

            changeFlag = 0;
        }
    }
}
