#include <stdint.h>
#include "TM4C123GH6PM.h"
#include "UART.h"
#include "tiva-gc.h"

int main()
{
    LCD_pixel bgColor;
    point old_pos, pos, js = { 2048, 2048 };    // Joystick centered to begin with
    
    // Input init
    InitGPIO_EdumkiiButtons();
    InitGPIO_EdumkiiJoystick();

    // Output init
    LCD_Init();
    LCD_CS(LOW);
    
    bgColor = LCD_GetSettings().BGColor;
    js = ReadJoystick(js);    // don't need that much precision
    js.x = js.x;
    js.y = js.y;
    pos.x = (int32_t)(js.x / 4096.0f * LCD_WIDTH);
    pos.y = LCD_HEIGHT - (int32_t)(js.y / 4096.0f * LCD_HEIGHT);
    if (pos.x < 2)
        pos.x += 2;
    else if (pos.x > LCD_WIDTH - 2)
        pos.x -= 2;
    if (pos.y < 2)
        pos.y += 2;
    else if (pos.y > LCD_HEIGHT - 2)
        pos.y -= 2;

    LCD_gClear();

    while (1)
    {
        old_pos = pos;
        js = ReadJoystick(js);
        js.x = js.x;
        js.y = js.y;
        pos.x = (int32_t)(js.x / 4096.0f * LCD_WIDTH);
        pos.y = LCD_HEIGHT - (int32_t)(js.y / 4096.0f * LCD_HEIGHT);
        if (pos.x < 2)
            pos.x += 2;
        else if (pos.x > LCD_WIDTH - 2)
            pos.x -= 2;
        if (pos.y < 2)
            pos.y += 2;
        else if (pos.y > LCD_HEIGHT - 2)
            pos.y -= 2;

        if (old_pos.x != pos.x || old_pos.y != pos.y)
        {
            LCD_gFillRectangle((uint8_t) old_pos.x, (uint8_t) old_pos.y, 5, 5, bgColor);
            LCD_gRectangle((uint8_t) pos.x, (uint8_t) pos.y, 5, 5, 1, LCD_RED);
        }
    }
}

int main0()
{
    uint32_t color = 0x3f000;
    uint32_t counter = 0;
    
    LCD_Init();
    
    LCD_CS(LOW);
    LCD_SetArea(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    LCD_ActivateWrite();
    while (1)
    {
        LCD_PushPixel(color >> 12 & 0xFF, (color >> 6) & 0xFF, color & 0xFF);
        
        color = 0;//(color >= 0x3FFFF) ? 0 : color + 32;
        
        if (++counter == LCD_HEIGHT * LCD_WIDTH)
            counter = 0;
    }
}

int main1()
{
    point js;
    
    InitGPIO_TivaButtons();
    InitGPIO_EdumkiiButtons();
    InitGPIO_EdumkiiJoystick();
    
    UART_Init();
    UART_OutString("\r\nJoystick and buttons test\r\n");
    
    while (1)
    {
        js = ReadJoystickRaw();       // if these readings are bad (crosstalk) check if boosterpack is connected nicely
        UART_OutChar('(');
        UART_OutUDec(js.x);
        UART_OutString(", ");
        UART_OutUDec(js.y);
        UART_OutString(") ");
        if (ReadButton(TIVAC_SW1))
            UART_OutString("TSW1 ");
        if (ReadButton(TIVAC_SW2))
            UART_OutString("TSW2 ");
        if (ReadButton(EDUMKII_SW1))
            UART_OutString("ESW1 ");
        if (ReadButton(EDUMKII_SW2))
            UART_OutString("ESW2 ");
        if (ReadButton(EDUMKII_SEL))
            UART_OutString("ESEL ");
        
        UART_OutString("                                      \r");
    }
}
