#include "InitGPIO.h"
#include "inc/tm4c123gh6pm.h"

// SW1: PF4
// SW2: PF0
void InitGPIO_TivaButtons(void)
{
    SYSCTL_RCGCGPIO_R |= 0x020;          // Port F on
    while (!(SYSCTL_PRGPIO_R & 0x20));   // wait for GPIO ready

    GPIO_PORTF_LOCK_R = 0x4C4F434B;  // unlock
    GPIO_PORTF_CR_R    |= 0x11;       // only use button pins
    //GPIOF->DIR   &= 0XEE;     // set bits 0 & 4 to 0 (input)
    GPIO_PORTF_PUR_R   |= 0x11;       // pull-up for buttons
    GPIO_PORTF_DEN_R   |= 0x11;       // buttons are digital
}

// SW1: PD6
// SW2: PD7
void InitGPIO_EdumkiiButtons(void)
{
    SYSCTL_RCGCGPIO_R |= 0x08;           // Port D on
    while (!(SYSCTL_PRGPIO_R & 0x08));   // wait for GPIO ready

    GPIO_PORTD_LOCK_R  = 0x4C4F434B;  // unlock
    GPIO_PORTD_CR_R    |= 0xC0;       // Button pins are PD6 & PD7
    GPIO_PORTD_PUR_R   |= 0xC0;       // pull-up for buttons
    GPIO_PORTD_DEN_R   |= 0xC0;       // buttons are digital
}

// X: PB5
// Y: PD3
// SEL: PE4
void InitGPIO_EdumkiiJoystick(void)
{
    // Vertical
    SYSCTL_RCGCGPIO_R |= 0x08;   // Port D on

    // Horizontal
    SYSCTL_RCGCGPIO_R |= 0x02;   // Port B on

    // Joystick select
    SYSCTL_RCGCGPIO_R |= 0x10;   // Port E on

    while (!(SYSCTL_PRGPIO_R & 0x08));   // wait for GPIO ready
    while (!(SYSCTL_PRGPIO_R & 0x02));
    while (!(SYSCTL_PRGPIO_R & 0x10));

    GPIO_PORTD_LOCK_R  = 0x4C4F434B;  // unlock
    GPIO_PORTD_CR_R    |= 0x08;       // Vertical axis enable
    GPIO_PORTD_AMSEL_R |= 0x08;       // Analog input
    GPIO_PORTD_AFSEL_R |= 0x08;       // ADC

    GPIO_PORTB_LOCK_R  = 0x4C4F434B;  // unlock
    GPIO_PORTB_CR_R    |= 0x20;       // Horizontal axis enable
    GPIO_PORTB_AMSEL_R |= 0x20;       // Analog input
    GPIO_PORTB_AFSEL_R |= 0x20;       // ADC

    GPIO_PORTE_LOCK_R  = 0x4C4F434B;  // unlock
    GPIO_PORTE_CR_R    |= 0x10;       // Select enable
    GPIO_PORTE_PUR_R   |= 0x10;       // pull-up for button
    GPIO_PORTE_DEN_R   |= 0x10;       // buttons are digital

    // Enable ADCs
    // AIN4 -> PD3 (Ver)
    // AIN11 -> PB5 (Hor)
    SYSCTL_RCGCADC_R |= 0x03;    // Enable ADC0 & ADC1
    while (!(SYSCTL_PRADC_R & 0x03));

    ADC0_PC_R &= ~0xF;       // clear max sample rate field
    ADC0_PC_R |= 0x1;        // configure for 125K samples/sec
    ADC0_SSPRI_R = 0x3210;   // Sequencer 3 is lowest priority
    ADC0_ACTSS_R &= ~0x4;    // disable sample sequencer 2
    ADC0_EMUX_R &= 0x0F00;   // seq2 is software trigger
    ADC0_SSMUX2_R = 0x4B;    // set channels for SS2. Change between 4 and 11 for Ver and Hor
    ADC0_SSCTL2_R = 0x60;    // no TS0 D0 IE0 END0 TS1 D1, yes IE1 END1
    ADC0_IM_R &= ~0x4;       // disable SS2 interrupts
    ADC0_ACTSS_R |= 0x4;     // enable sample sequencer 2
}

void EdumkiiLEDInit(void);

