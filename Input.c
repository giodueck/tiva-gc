#include "Input.h"

int ReadButton(int button)
{
    switch (button)
    {
        case TIVAC_SW1:
            return ((GPIOF->DATA & 0x10) >> 4) ^ 0x01;
        case TIVAC_SW2:
            return (GPIOF->DATA & 0x01) ^ 0x01;
        case EDUMKII_SW1:
            return ((GPIOD->DATA & 0x40) >> 6) ^ 0x01;
        case EDUMKII_SW2:
            return ((GPIOD->DATA & 0x80) >> 7) ^ 0x01;
        case EDUMKII_SEL:
            return ((GPIOE->DATA & 0x10) >> 4) ^ 0x01;
        default:
            return 0;
    }
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
