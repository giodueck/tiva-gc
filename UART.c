// UART.c
// Runs on LM3S811, LM3S1968, LM3S8962, LM4F120, TM4C123
// Simple device driver for the UART.
// Daniel Valvano
// May 30, 2014
// Modified by EE345L students Charlie Gough && Matt Hawk
// Modified by EE345M students Agustinus Darmawan && Mingjie Qiu

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2014
   Program 4.12, Section 4.9.4, Figures 4.26 and 4.40

 Copyright 2014 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

// U0Rx (VCP receive) connected to PA0
// U0Tx (VCP transmit) connected to PA1
#include <stdint.h>
#include <tm4c123gh6pm.h>

#include "UART.h"

#define UART_FR_TXFF            0x00000020  // UART Transmit FIFO Full
#define UART_FR_RXFE            0x00000010  // UART Receive FIFO Empty
#define UART_LCRH_WLEN_8        0x00000060  // 8 bit word length
#define UART_LCRH_FEN           0x00000010  // UART Enable FIFOs
#define UART_CTL_UARTEN         0x00000001  // UART Enable



//------------UART_Init_Int------------
// Initialize the UART for 115,200 baud rate (assuming 16 MHz UART clock),
// 8 bit word length, no parity bits, one stop bit, FIFOs enabled
// Send and receive using interruption
// Input: none
// Output: none
void UART_Init_Int (void) {
}

//------------UART_Init------------
// Initialize the UART for 115,200 baud rate (assuming 16 MHz UART clock),
// 8 bit word length, no parity bits, one stop bit, FIFOs enabled
// Input: none
// Output: none
void UART_Init(void){
  SYSCTL->RCGCUART |= 0x01;            // activate UART0
  SYSCTL->RCGCGPIO |= 0x01;            // activate port A
  while((SYSCTL->PRGPIO & 0x01) == 0 || (SYSCTL->PRUART & 0x01) == 0){};
  
  UART0->CTL &= ~UART_CTL_UARTEN;      // disable UART
  UART0->IBRD = 8;                // IBRD = int(16,000,000 / (16 * 115,200)) = int(8.6806)
  UART0->FBRD = 44;               // FBRD = int(0.6806 * 64 + 0.5) = 44
                                  // 8 bit word length (no parity bits, one stop bit, FIFOs)
  UART0->LCRH = (UART_LCRH_WLEN_8|UART_LCRH_FEN);
  UART0->CTL |= UART_CTL_UARTEN;  // enable UART
  GPIOA->AFSEL |= 0x03;           // enable alt funct on PA1-0
  GPIOA->DEN |= 0x03;             // enable digital I/O on PA1-0
                                  // configure PA1-0 as UART
  GPIOA->PCTL = (GPIOA->PCTL & 0xFFFFFF00) | 0x00000011;
  GPIOA->AMSEL &= ~0x03;          // disable analog functionality on PA
}

//------------UART_Init_baudrate------------
// Initialize the UART for a given baud rate, also the clock speed is a parameter,
// 8 bit word length, no parity bits, one stop bit, FIFOs enabled
// Input: none
// Output: none
void UART_Init_baudrate(uint32_t baudRate, uint32_t clk){
	float baudrateReg = (float)clk / ((float)16.0 * (float)baudRate);
	uint32_t intBaudrateReg = (int) baudrateReg;
	uint32_t fracBaudrateReg = (int) ((baudrateReg - (float)intBaudrateReg) * (float)64.0 + (float)0.5);
	
  SYSCTL->RCGCUART |= 0x01;            // activate UART0
  SYSCTL->RCGCGPIO |= 0x01;            // activate port A
  //while((SYSCTL_PRGPIO_R&0x01) == 0){};
  UART0->CTL &= ~UART_CTL_UARTEN;      // disable UART
  UART0->IBRD = intBaudrateReg;   // IBRD = int(clk / (16 * baudRate))
  UART0->FBRD = fracBaudrateReg;  // FBRD = int(frac(clk / (16 * baudRate)) * 64 + 0.5) 
                                  // 8 bit word length (no parity bits, one stop bit, FIFOs)
  UART0->LCRH = (UART_LCRH_WLEN_8|UART_LCRH_FEN);
  UART0->CTL |= UART_CTL_UARTEN;  // enable UART
  GPIOA->AFSEL |= 0x03;           // enable alt funct on PA1-0
  GPIOA->DEN |= 0x03;             // enable digital I/O on PA1-0
                                  // configure PA1-0 as UART
  GPIOA->PCTL = (GPIOA->PCTL & 0xFFFFFF00) | 0x00000011;
  GPIOA->AMSEL &= ~0x03;          // disable analog functionality on PA
}

//------------UART_Transmit_FIFO_Full------------
// Check if transmit FIFO is full
// Input: none
// Output: 0 not full, 
//         1 transmit FIFO full
int UART_Transmit_FIFO_Full () {
	return ((UART0->FR & (1ul << 5)) >> 5);
}

//------------UART_Transmit_FIFO_Empty------------
// Check if transmit FIFO is empty
// Input: none
// Output: 0 not empty, 
//         1 transmit FIFO empty
int UART_Transmit_FIFO_Empty () {
	return (UART0->FR & 1ul);
}

//------------UART_DataAvailable------------
// Check if there is data in the receive FIFO
// Input: none
// Output: 0 if no data is available, 
//         1 if data is available
int UART_DataAvailable(void){
  return !((UART0->FR & (1ul << 4)) >> 4);
}

//------------UART_InChar------------
// Wait for new serial port input
// Input: none
// Output: ASCII code for key typed
char UART_InChar(void){
  while((UART0->FR & UART_FR_RXFE) != 0);
  return((char)(UART0->DR & 0xFF));
}

//------------UART_OutChar------------
// Output 8-bit to serial port. Wait until the transmit fifo has a 
// place for the new data.
// Input: letter is an 8-bit ASCII character to be transferred
// Output: none
void UART_OutChar(char data){
  while((UART0->FR & UART_FR_TXFF) != 0);
  UART0->DR = data;
}

//------------UART_OutString------------
// Output String (NULL termination)
// Input: pointer to a NULL-terminated string to be transferred
// Output: none
void UART_OutString(char *pt){
  while(*pt){
    UART_OutChar(*pt);
    pt++;
  }
}

//------------UART_InSDec------------
// InUDec accepts ASCII input in signed decimal format
//     and converts to a 32-bit unsigned number
//     valid range is -2147483648 to +2147483647 (-2^(32-1) to 2^(32-1)-1)
// Input: none
// Output: 32-bit unsigned number
// If you enter an invalid number, it will return an incorrect value
// Backspace will remove last digit typed
int32_t UART_InSDec(void){
	int32_t number=0, sign = 1;
	uint32_t length=0;
	char character;
	
  character = UART_InChar();
	if (character == '-'){
		sign = -1;
		UART_OutChar(character);
		character = UART_InChar();
	} else if (character == '+'){
		sign = 1;
		UART_OutChar(character);
		character = UART_InChar();
	} else {
		// si es cualquier otra cosa, una letra o un n�mero se considera positivo
		sign = 1;
	}
  while(character != CR_A){ // accepts until <enter> is typed
// The next line checks that the input is a digit, 0-9.
// If the character is not 0-9, it is ignored and not echoed
    if((character>='0') && (character<='9')) {
      number = 10*number+(character-'0');   // this line overflows if above 4294967295
      length++;
      UART_OutChar(character);
    }
// If the input is a backspace, then the return number is
// changed and a backspace is outputted to the screen
    else if((character==BS_A) && length){
      number /= 10;
      length--;
      UART_OutChar(character);
    }
    character = UART_InChar();
  }
  return sign * number;
}

//------------UART_InUDec------------
// InUDec accepts ASCII input in unsigned decimal format
//     and converts to a 32-bit unsigned number
//     valid range is 0 to 4294967295 (2^32-1)
// Input: none
// Output: 32-bit unsigned number
// If you enter a number above 4294967295, it will return an incorrect value
// Backspace will remove last digit typed
uint32_t UART_InUDec(void){
uint32_t number=0, length=0;
char character;
  character = UART_InChar();
  while(character != CR_A){ // accepts until <enter> is typed
// The next line checks that the input is a digit, 0-9.
// If the character is not 0-9, it is ignored and not echoed
    if((character>='0') && (character<='9')) {
      number = 10*number+(character-'0');   // this line overflows if above 4294967295
      length++;
      UART_OutChar(character);
    }
// If the input is a backspace, then the return number is
// changed and a backspace is outputted to the screen
    else if((character==BS_A) && length){
      number /= 10;
      length--;
      UART_OutChar(character);
    }
    character = UART_InChar();
  }
  return number;
}

//-----------------------UART_OutUDec-----------------------
// Output a 32-bit number in unsigned decimal format
// Input: 32-bit number to be transferred
// Output: none
// Variable format 1-10 digits with no space before or after
void UART_OutUDec(uint32_t n){
// This function uses recursion to convert decimal number
//   of unspecified length as an ASCII string
  if(n >= 10){
    UART_OutUDec(n/10);
    n = n%10;
  }
  UART_OutChar(n+'0'); /* n is between 0 and 9 */
}

//-----------------------UART_OutSDec-----------------------
// Output a 32-bit number in signed decimal format
// Input: 32-bit number to be transferred
// Output: none
// Variable format 1-10 digits with no space before or after
void UART_OutSDec(int32_t n){
// This function uses recursion to convert decimal number
//   of unspecified length as an ASCII string
	if (n < 0){
		n *= -1;	// does not work for max negative number
		UART_OutChar('-');
	}
  if(n >= 10){
    UART_OutSDec(n/10);
    n = n%10;
  }
  UART_OutChar(n+'0'); /* n is between 0 and 9 */
}

//---------------------UART_InUHex----------------------------------------
// Accepts ASCII input in unsigned hexadecimal (base 16) format
// Input: none
// Output: 32-bit unsigned number
// No '$' or '0x' need be entered, just the 1 to 8 hex digits
// It will convert lower case a-f to uppercase A-F
//     and converts to a 16 bit unsigned number
//     value range is 0 to FFFFFFFF
// If you enter a number above FFFFFFFF, it will return an incorrect value
// Backspace will remove last digit typed
uint32_t UART_InUHex(void){
uint32_t number=0, digit, length=0;
char character;
  character = UART_InChar();
  while(character != CR_A){
    digit = 0x10; // assume bad
    if((character>='0') && (character<='9')){
      digit = character-'0';
    }
    else if((character>='A') && (character<='F')){
      digit = (character-'A')+0xA;
    }
    else if((character>='a') && (character<='f')){
      digit = (character-'a')+0xA;
    }
// If the character is not 0-9 or A-F, it is ignored and not echoed
    if(digit <= 0xF){
      number = number*0x10+digit;
      length++;
      UART_OutChar(character);
    }
// Backspace outputted and return value changed if a backspace is inputted
    else if((character==BS_A) && length){
      number /= 0x10;
      length--;
      UART_OutChar(character);
    }
    character = UART_InChar();
  }
  return number;
}

//--------------------------UART_OutUHex----------------------------
// Output a 32-bit number in unsigned hexadecimal format
// Input: 32-bit number to be transferred
// Output: none
// Variable format 1 to 8 digits with no space before or after
void UART_OutUHex(uint32_t number){
// This function uses recursion to convert the number of
//   unspecified length as an ASCII string
  if(number >= 0x10){
    UART_OutUHex(number/0x10);
    UART_OutUHex(number%0x10);
  } else {
    if(number < 0xA){
      UART_OutChar(number+'0');
     }
    else{
      UART_OutChar((number-0x0A)+'A');
    }
  }
}

//------------UART_InString------------
// Accepts ASCII characters from the serial port
//    and adds them to a string until <enter> is typed
//    or until max length of the string is reached.
// It echoes each character as it is inputted.
// If a backspace is inputted, the string is modified
//    and the backspace is echoed
// terminates the string with a null character
// uses busy-waiting synchronization on RDRF
// Input: pointer to empty buffer, size of buffer
// Output: Null terminated string
// -- Modified by Agustinus Darmawan + Mingjie Qiu --
void UART_InString(char *bufPt, uint16_t max) {
//int length=0;
//char character;
//  character = UART_InChar();
//  while(character != CR_A){
//    if(character == BS_A){
//      if(length){
//        bufPt--;
//        length--;
//        UART_OutChar(BS_A);
//      }
//    }
//    else if(length < max){
//      *bufPt = character;
//      bufPt++;
//      length++;
//      UART_OutChar(character);
//    }
//    character = UART_InChar();
//  }
//  *bufPt = 0;
	int i;
	char c;

	for (i = 0; i < max -1 && (c = UART_InChar()) != '\r'; i++, bufPt++) {
		if (c == BS_A) {
			if(i){
				bufPt--;
				i--;
				UART_OutChar(BS_A);
			}
		}
		*bufPt = c;
		UART_OutChar(c);
	}

	*bufPt = '\0';	// agregamos el fin de cadena
}
