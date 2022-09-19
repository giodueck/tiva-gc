#include "InitGPIO.h"
#include "TM4C123GH6PM.h"

// SW1: PF4
// SW2: PF0
void InitGPIO_TivaButtons(void)
{
    SYSCTL->RCGCGPIO |= 0x020;          // Port F on
    while (!(SYSCTL->PRGPIO & 0x20));   // wait for GPIO ready
    
    GPIOF->LOCK  = 0x4C4F434B;  // unlock
    GPIOF->CR    |= 0x11;       // only use button pins
    //GPIOF->DIR   &= 0XEE;     // set bits 0 & 4 to 0 (input)
    GPIOF->PUR   |= 0x11;       // pull-up for buttons
    GPIOF->DEN   |= 0x11;       // buttons are digital
}

// SW1: PD6
// SW2: PD7
void InitGPIO_EdumkiiButtons(void)
{
    SYSCTL->RCGCGPIO |= 0x08;           // Port D on
    while (!(SYSCTL->PRGPIO & 0x08));   // wait for GPIO ready
    
    GPIOD->LOCK  = 0x4C4F434B;  // unlock
    GPIOD->CR    |= 0xC0;       // Button pins are PD6 & PD7
    GPIOD->PUR   |= 0xC0;       // pull-up for buttons
    GPIOD->DEN   |= 0xC0;       // buttons are digital
}

// X: PB5
// Y: PD3
// SEL: PE4
void InitGPIO_EdumkiiJoystick(void)
{
    // Vertical
    SYSCTL->RCGCGPIO |= 0x08;   // Port D on
    
    // Horizontal
    SYSCTL->RCGCGPIO |= 0x02;   // Port B on
    
    // Joystick select
    SYSCTL->RCGCGPIO |= 0x10;   // Port E on
    
    while (!(SYSCTL->PRGPIO & 0x08));   // wait for GPIO ready
    while (!(SYSCTL->PRGPIO & 0x02));
    while (!(SYSCTL->PRGPIO & 0x10));
    
    GPIOD->LOCK  = 0x4C4F434B;  // unlock
    GPIOD->CR    |= 0x08;       // Vertical axis enable
    GPIOD->AMSEL |= 0x08;       // Analog input
    GPIOD->AFSEL |= 0x08;       // ADC
    
    GPIOB->LOCK  = 0x4C4F434B;  // unlock
    GPIOB->CR    |= 0x20;       // Horizontal axis enable
    GPIOB->AMSEL |= 0x20;       // Analog input
    GPIOB->AFSEL |= 0x20;       // ADC
    
    GPIOE->LOCK  = 0x4C4F434B;  // unlock
    GPIOE->CR    |= 0x10;       // Select enable
    GPIOE->PUR   |= 0x10;       // pull-up for button
    GPIOE->DEN   |= 0x10;       // buttons are digital
    
    // Enable ADCs
    // AIN4 -> PD3 (Ver)
    // AIN11 -> PB5 (Hor)
    SYSCTL->RCGCADC |= 0x03;    // Enable ADC0 & ADC1
    while (!(SYSCTL->PRADC & 0x03));
    
    ADC0->PC &= ~0xF;       // clear max sample rate field
    ADC0->PC |= 0x1;        // configure for 125K samples/sec
    ADC0->SSPRI = 0x3210;   // Sequencer 3 is lowest priority
    ADC0->ACTSS &= ~0x4;    // disable sample sequencer 2
    ADC0->EMUX &= 0x0F00;   // seq2 is software trigger
    ADC0->SSMUX2 = 0x4B;    // set channels for SS2. Change between 4 and 11 for Ver and Hor
    ADC0->SSCTL2 = 0x60;    // no TS0 D0 IE0 END0 TS1 D1, yes IE1 END1
    ADC0->IM &= ~0x4;       // disable SS2 interrupts
    ADC0->ACTSS |= 0x4;     // enable sample sequencer 2
}

void EdumkiiLEDInit(void);

