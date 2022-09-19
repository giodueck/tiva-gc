#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>
#include "TM4C123GH6PM.h"

#define TIVAC_SW1   0
#define TIVAC_SW2   1
#define EDUMKII_SW1 2
#define EDUMKII_SW2 3
#define EDUMKII_SEL 4

typedef struct
{
    uint32_t x, y;
} point;

int ReadButton(int button);

point ReadJoystick(void);

#endif // INPUT_H
