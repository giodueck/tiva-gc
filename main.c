#include <stdint.h>
#include "TM4C123GH6PM.h"
#include "tiva-gc.h"

int main()
{
    LCD_Settings actSettings;
    point old_pos, pos, js = { 2048, 2048 };    // Joystick centered to begin with
    LCD_pixel colors[6] = { LCD_RED, LCD_YELLOW, LCD_GREEN, LCD_CYAN, LCD_BLUE, LCD_MAGENTA };
    uint8_t i = 0, changeFlag = 0;
    point old_rhombus[4] = {0}, rhombus[4] = {0};
    
    // Input init
    InitGPIO_EdumkiiButtons();
    InitGPIO_EdumkiiJoystick();

    // Output init
    LCD_Init();
    LCD_CS(LOW);
    
    LCD_SetBGColor(LCD_GREY);
    actSettings = LCD_GetSettings();
    js = ReadJoystick(js);    // don't need that much precision
    js.x = js.x;
    js.y = js.y;
    pos.x = (int32_t)(js.x / 4095.0f * LCD_WIDTH);
    pos.y = LCD_HEIGHT - (int32_t)(js.y / 4095.0f * LCD_HEIGHT);
    if (pos.x < 2)
        pos.x += 2;
    else if (pos.x >= LCD_WIDTH)
        pos.x = LCD_WIDTH - 1;
    if (pos.y < 2)
        pos.y += 2;
    else if (pos.y >= LCD_HEIGHT)
        pos.y = LCD_HEIGHT - 1;

    rhombus[0] = pos;
    rhombus[0].y -= 5;
    rhombus[1] = pos;
    rhombus[1].x += 5;
    rhombus[2] = pos;
    rhombus[2].y += 5;
    rhombus[3] = pos;
    rhombus[3].x -= 5;

    LCD_gClear();

    while (1)
    {
        // Detecta pulsaciones de los botones
        if (ReadButton(BUTTON_EDUMKII_SW1))
        {
            i = (i + 1) % 6;    // Cycle through primary and secondary RGB colors
            changeFlag = 1;
        }
        if (ReadButton(BUTTON_EDUMKII_SW2))
        {
            i = (i + 5) % 6;
            changeFlag = 1;
        }
        if (ReadButton(BUTTON_EDUMKII_SEL))
        {
            LCD_SetInversion(!actSettings.InversionMode);
            actSettings.InversionMode = !actSettings.InversionMode;
            changeFlag = 1;
        }

        // Detecta la posicion del joystick y la convierte a un punto en el display
        old_pos = pos;
        js = ReadJoystick(js);
        js.x = js.x;
        js.y = js.y;
        pos.x = (int32_t)(js.x / 4095.0f * LCD_WIDTH);
        pos.y = LCD_HEIGHT - (int32_t)(js.y / 4095.0f * LCD_HEIGHT);
        if (pos.x < 2)
            pos.x += 2;
        else if (pos.x >= LCD_WIDTH)
            pos.x = LCD_WIDTH - 1;
        if (pos.y < 2)
            pos.y += 2;
        else if (pos.y >= LCD_HEIGHT)
            pos.y = LCD_HEIGHT - 1;
        
        for (int j = 0; j < 4; j++)
            old_rhombus[j] = rhombus[j];

        rhombus[0] = pos;
        rhombus[0].y -= 5;
        rhombus[1] = pos;
        rhombus[1].x += 5;
        rhombus[2] = pos;
        rhombus[2].y += 5;
        rhombus[3] = pos;
        rhombus[3].x -= 5;

        // Dibuja sobre el display
        if (old_pos.x != pos.x || old_pos.y != pos.y || changeFlag)
        {
            // Erase previous shapes
            // LCD_gFillRectangle((uint8_t) old_pos.x - 5, (uint8_t) old_pos.y - 5, 10, 10, actSettings.BGColor);
            LCD_gLine(64, 64, (uint8_t) old_pos.x, (uint8_t) old_pos.y, 1, actSettings.BGColor);
            // LCD_gTriangle((point) {old_pos.x, old_pos.y - 5}, (point) {old_pos.x + 5, old_pos.y + 5}, (point) {old_pos.x - 5, old_pos.y + 5}, 1, actSettings.BGColor);
            // LCD_gFillTriangle((point) {old_pos.x, old_pos.y - 5}, (point) {old_pos.x + 5, old_pos.y + 5}, (point) {old_pos.x - 5, old_pos.y + 5}, actSettings.BGColor);
            // LCD_gPolygon(old_rhombus, 4, 1, actSettings.BGColor);
            LCD_gCircle(old_pos.x, old_pos.y, 7, 1, actSettings.BGColor);
            
            // Draw new shapes
            // LCD_gRectangle((uint8_t) pos.x - 5, (uint8_t) pos.y - 5, 10, 10, 1, colors[i]);
            LCD_gLine(64, 64, (uint8_t) pos.x, (uint8_t) pos.y, 1, colors[i]);
            // LCD_gTriangle((point) {pos.x, pos.y - 5}, (point) {pos.x + 5, pos.y + 5}, (point) {pos.x - 5, pos.y + 5}, 1, colors[i]);
            // LCD_gFillTriangle((point) {pos.x, pos.y - 5}, (point) {pos.x + 5, pos.y + 5}, (point) {pos.x - 5, pos.y + 5}, colors[i]);
            // LCD_gPolygon(rhombus, 4, 1, colors[i]);
            LCD_gCircle(pos.x, pos.y, 7, 1, colors[i]);

            changeFlag = 0;
        }
    }
}
