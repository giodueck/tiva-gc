#include "Input.h"
#include "delay.h"

// Reads a button.
//  Param:
//      button: one of BUTTON_EDUMKII_SW1, BUTTON_EDUMKII_SW2, BUTTON_EDUMKII_SEL
//  Return:
//      0 if the button was not closed, 1 if it was
int Input_ReadButtonRaw(int button)
{
    switch (button)
    {
        #ifdef DISABLE_LCD
        case BUTTON_TIVAC_SW1:
            return ((GPIO_PORTF_DATA_R & 0x10) >> 4) ^ 0x01;
        case BUTTON_TIVAC_SW2:
            return (GPIO_PORTF_DATA_R & 0x01) ^ 0x01;
        #endif
        case BUTTON_EDUMKII_SW1:
            return ((GPIO_PORTD_DATA_R & 0x40) >> 6) ^ 0x01;
        case BUTTON_EDUMKII_SW2:
            return ((GPIO_PORTD_DATA_R & 0x80) >> 7) ^ 0x01;
        case BUTTON_EDUMKII_SEL:
            return ((GPIO_PORTE_DATA_R & 0x10) >> 4) ^ 0x01;
        default:
            return 0;
    }
}

// Reads a button and returns 1 if the button was pressed since the last call.
//  Param:
//      button: one of BUTTON_EDUMKII_SW1, BUTTON_EDUMKII_SW2, BUTTON_EDUMKII_SEL
//  Return:
//      0 if the button was not pressed or was not released since the last press, 1 otherwise
int Input_ReadButton(int button)
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
            r = Input_ReadButtonRaw(button);
            if (!r)
                tsw1 = 0;
            else if (!tsw1)
            {
                delay(10);
                tsw1 = Input_ReadButtonRaw(button);
                return tsw1;
            }
            break;
        case BUTTON_TIVAC_SW2:
            r = Input_ReadButtonRaw(button);
            if (!r)
                tsw2 = 0;
            else if (!tsw2)
            {
                delay(10);
                tsw2 = Input_ReadButtonRaw(button);
                return tsw2;
            }
            break;
        #endif
        case BUTTON_EDUMKII_SW1:
            r = Input_ReadButtonRaw(button);
            if (!r)
                esw1 = 0;
            else if (!esw1)
            {
                delay(10);
                esw1 = Input_ReadButtonRaw(button);
                return esw1;
            }
            break;
        case BUTTON_EDUMKII_SW2:
            r = Input_ReadButtonRaw(button);
            if (!r)
                esw2 = 0;
            else if (!esw2)
            {
                delay(10);
                esw2 = Input_ReadButtonRaw(button);
                return esw2;
            }
            break;
        case BUTTON_EDUMKII_SEL:
            r = Input_ReadButtonRaw(button);
            if (!r)
                esel = 0;
            else if (!esel)
            {
                delay(10);
                esel = Input_ReadButtonRaw(button);
                return esel;
            }
            break;
        default:
            break;
    }

    return 0;
}

// Reads the value given by the current joystick position.
//  Return:
//      point describing the position of the joystick, 0-4095
point Input_ReadJoystickRaw(void)
{
    point p;

    ADC0_SSMUX2_R = 4;
    ADC0_PSSI_R = 0x0004;            // initiate SS2
    while((ADC0_RIS_R & 0x04) == 0); // wait for conversion done
    p.x = ADC0_SSFIFO2_R & 0xFFF;    // read first result
    ADC0_ISC_R = 0x0004;             // acknowledge completion

    ADC0_SSMUX2_R = 11;
    ADC0_PSSI_R = 0x0004;            // initiate SS2
    while((ADC0_RIS_R & 0x04) == 0); // wait for conversion done
    p.y = ADC0_SSFIFO2_R & 0xFFF;    // read second result
    ADC0_ISC_R = 0x0004;             // acknowledge completion

    return p;
}

// Reads the position of the joystick and compares it against the previous value.
// If the value changes about 1 pixel in any direction, the value returned is the value
// given by Input_ReadJoystickRaw, otherwise any change is assumed to be noise and
// the old value is returned
//  Return:
//      point describing the position of the joystick, 0-4095
point Input_ReadJoystick()
{
    static point old;
    point new = Input_ReadJoystickRaw();
    if (abs(old.x - new.x) < 31 && abs(old.y - new.y) < 31)   // check for a change of about 0.76% (~1 pixel) in either axis
        return old;
    else
    {
        old = new;
        return new;
    }
}
