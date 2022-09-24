#include "Input.h"
#include "delay.h"

int ReadButtonRaw(int button)
{
    switch (button)
    {
        #ifdef DISABLE_LCD
        case BUTTON_TIVAC_SW1:
            return ((GPIOF->DATA & 0x10) >> 4) ^ 0x01;
        case BUTTON_TIVAC_SW2:
            return (GPIOF->DATA & 0x01) ^ 0x01;
        #endif
        case BUTTON_EDUMKII_SW1:
            return ((GPIOD->DATA & 0x40) >> 6) ^ 0x01;
        case BUTTON_EDUMKII_SW2:
            return ((GPIOD->DATA & 0x80) >> 7) ^ 0x01;
        case BUTTON_EDUMKII_SEL:
            return ((GPIOE->DATA & 0x10) >> 4) ^ 0x01;
        default:
            return 0;
    }
}

int ReadButton(int button)
{
    #ifdef DISABLE_LCD
    static uint8_t tsw1 = 0, tsw2 = 0;
    #endif
    static uint8_t esw1 = 0, esw2 = 0, esel = 0;
    uint8_t r = 0;

    // Read once, depending on read and state: wait, read again
    // If the first read is 0 or the button was already pressed, the result is 0
    // Whatever value the second read gets is ANDed with the first, which is 1
    // So second read is also the result
    switch (button)
    {
        #ifdef DISABLE_LCD
        case BUTTON_TIVAC_SW1:
            r = ReadButtonRaw(button);
            if (!r)
                tsw1 = 0;
            else if (!tsw1)
            {
                delay(10);
                tsw1 = ReadButtonRaw(button);
                return tsw1;
            }
            break;
        case BUTTON_TIVAC_SW2:
            r = ReadButtonRaw(button);
            if (!r)
                tsw2 = 0;
            else if (!tsw2)
            {
                delay(10);
                tsw2 = ReadButtonRaw(button);
                return tsw2;
            }
            break;
        #endif
        case BUTTON_EDUMKII_SW1:
            r = ReadButtonRaw(button);
            if (!r)
                esw1 = 0;
            else if (!esw1)
            {
                delay(10);
                esw1 = ReadButtonRaw(button);
                return esw1;
            }
            break;
        case BUTTON_EDUMKII_SW2:
            r = ReadButtonRaw(button);
            if (!r)
                esw2 = 0;
            else if (!esw2)
            {
                delay(10);
                esw2 = ReadButtonRaw(button);
                return esw2;
            }
            break;
        case BUTTON_EDUMKII_SEL:
            r = ReadButtonRaw(button);
            if (!r)
                esel = 0;
            else if (!esel)
            {
                delay(10);
                esel = ReadButtonRaw(button);
                return esel;
            }
            break;
        default:
            break;
    }

    return 0;
}

point ReadJoystickRaw(void)
{
    point p;
    
    ADC0->SSMUX2 = 4;
    ADC0->PSSI = 0x0004;            // initiate SS2
    while((ADC0->RIS & 0x04) == 0); // wait for conversion done
    p.x = ADC0->SSFIFO2 & 0xFFF;    // read first result
    ADC0->ISC = 0x0004;             // acknowledge completion
    
    ADC0->SSMUX2 = 11;
    ADC0->PSSI = 0x0004;            // initiate SS2
    while((ADC0->RIS & 0x04) == 0); // wait for conversion done
    p.y = ADC0->SSFIFO2 & 0xFFF;    // read second result
    ADC0->ISC = 0x0004;             // acknowledge completion
    
    return p;
}

point ReadJoystick(point old)
{
    point new = ReadJoystickRaw();
    if (abs(old.x - new.x) < 32 && abs(old.y - new.y) < 32)   // check for a change of about 0.78% (~1 pixel) in either axis
        return old;
    else
        return new;
}
