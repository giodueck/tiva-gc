#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>
#include "TM4C123GH6PM.h"
#include "tiva-gc-inc.h"

// ReadButton functions always return 0 for the tiva board switches if DISABLE_LCD is not defined
#define BUTTON_TIVAC_SW1   0
#define BUTTON_TIVAC_SW2   1
#define BUTTON_EDUMKII_SW1 2
#define BUTTON_EDUMKII_SW2 3
#define BUTTON_EDUMKII_SEL 4

typedef struct
{
    int32_t x, y;
} point;

// Reads a button.
//  Param:
//      button: one of BUTTON_EDUMKII_SW1, BUTTON_EDUMKII_SW2, BUTTON_EDUMKII_SEL
int ReadButtonRaw(int button);

// Reads a button and returns 1 if the button was pressed since the last call.
//  Param:
//      button: one of BUTTON_EDUMKII_SW1, BUTTON_EDUMKII_SW2, BUTTON_EDUMKII_SEL
int ReadButton(int button);

point ReadJoystickRaw(void);

point ReadJoystick(point old);

#endif // INPUT_H
