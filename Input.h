#ifndef INPUT_H
#define INPUT_H

/*
    Input reading functions, raw functions read the current values for the input device and the non-raw
    functions perform some extra logic, like debouncing and noise filtering.
*/

#include <stdint.h>
#include "inc/tm4c123gh6pm.h"
#include "tiva-gc-inc.h"

// Input_ReadButton functions always return 0 for the tiva board switches if DISABLE_LCD is not defined
#define BUTTON_TIVAC_SW1   0
#define BUTTON_TIVAC_SW2   1
#define BUTTON_EDUMKII_SW1 2
#define BUTTON_EDUMKII_SW2 3
#define BUTTON_EDUMKII_SEL 4

// Reads a button.
//  Param:
//      button: one of BUTTON_EDUMKII_SW1, BUTTON_EDUMKII_SW2, BUTTON_EDUMKII_SEL
//  Return:
//      0 if the button was not closed, 1 if it was
int Input_ReadButtonRaw(int button);

// Reads a button and returns 1 if the button was pressed since the last call.
//  Param:
//      button: one of BUTTON_EDUMKII_SW1, BUTTON_EDUMKII_SW2, BUTTON_EDUMKII_SEL
//  Return:
//      0 if the button was not pressed or was not released since the last press, 1 otherwise
int Input_ReadButton(int button);

// Reads the value given by the current joystick position.
//  Return:
//      point describing the position of the joystick, 0-4095
point Input_ReadJoystickRaw(void);

// Reads the position of the joystick and compares it against the previous value.
// If the value changes about 1 pixel in any direction, the value returned is the value
// given by Input_ReadJoystickRaw, otherwise any change is assumed to be noise and
// the old value is returned
//  Return:
//      point describing the position of the joystick, 0-4095
point Input_ReadJoystick(void);

#endif // INPUT_H
