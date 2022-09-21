#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>
#include "TM4C123GH6PM.h"
#include "tiva-gc-inc.h"

#define TIVAC_SW1   0
#define TIVAC_SW2   1
#define EDUMKII_SW1 2
#define EDUMKII_SW2 3
#define EDUMKII_SEL 4

typedef struct
{
    int32_t x, y;
} point;

int ReadButton(int button);

point ReadJoystickRaw(void);

point ReadJoystick(point old);

#endif // INPUT_H
