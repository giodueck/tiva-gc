#include "LCD.h"
#include "TM4C123GH6PM.h"
#include "delay.h"
#include "tiva-gc-inc.h"

#define DATAMODE_ACTIVESTATE HIGH
#define RESET_ACTIVESTATE    LOW

#define LCD_PIXEL_FORMAT_444 3 /* 12-bit/pixel */
#define LCD_PIXEL_FORMAT_565 5 /* 16-bit/pixel */
#define LCD_PIXEL_FORMAT_666 6 /* 18-bit/pixel */

#define LCD_GAMMA_PREDEFINED_1 (1<<0) /* Gamma Curve 1 */
#define LCD_GAMMA_PREDEFINED_2 (1<<1) /* Gamma Curve 2 */
#define LCD_GAMMA_PREDEFINED_3 (1<<2) /* Gamma Curve 3 */
#define LCD_GAMMA_PREDEFINED_4 (1<<3) /* Gamma Curve 4 */

#define LCD_MADCTL_MY  (1<<7) /* Row Address Order */
#define LCD_MADCTL_MX  (1<<6) /* Column Address Order */
#define LCD_MADCTL_MV  (1<<5) /* Row / Column Exchange */
#define LCD_MADCTL_ML  (1<<4) /* Vertical Refresh Order */
#define LCD_MADCTL_BGR (1<<3) /* RGB or BGR Order */
#define LCD_MADCTL_MH  (1<<2) /* Horizontal Refresh Order */
#define LCD_MADCTL_DEFAULT 0  /* Default state */

/* MV flag default state based on the Reset Table, see: (pdf v1.4 p89-91) */
#define FLAG_MADCTL_MV_DEFAULT 0

/* default pixel format; Based on the Reset Table, see: (pdf v1.4 p89-91) */
#define INTERFACE_PIXEL_FORMAT_DEFAULT LCD_PIXEL_FORMAT_666

/* ST7735S driver commands (pdf v1.4 p5) */
#define LCD_SWRESET 0x01
#define LCD_SLPIN   0x10
#define LCD_SLPOUT  0x11
#define LCD_INVOFF  0x20
#define LCD_INVON   0x21
#define LCD_GAMSET  0x26
#define LCD_DISPOFF 0x28
#define LCD_DISPON  0x29
#define LCD_CASET   0x2A
#define LCD_RASET   0x2B
#define LCD_RAMWR   0x2C
#define LCD_TEOFF   0x34
#define LCD_TEON    0x35
#define LCD_MADCTL  0x36
#define LCD_IDMOFF  0x38
#define LCD_IDMON   0x39
#define LCD_COLMOD  0x3A
#define LCD_RGBSET  0x2D
#define LCD_FRMCTR1 0xB1
#define LCD_PWCTR1  0xC0
#define LCD_PWCTR2  0xC1
#define LCD_PWCTR3  0xC2
#define LCD_PWCTR4  0xC3
#define LCD_PWCTR5  0xC4

/* Active settings */
static LCD_Settings _active_settings = {0};

// standard ascii 5x7 font
// originally from glcdfont.c from Adafruit project
static const uint8_t Font[] = {
  0x00, 0x00, 0x00, 0x00, 0x00,
  0x3E, 0x5B, 0x4F, 0x5B, 0x3E,
  0x3E, 0x6B, 0x4F, 0x6B, 0x3E,
  0x1C, 0x3E, 0x7C, 0x3E, 0x1C,
  0x18, 0x3C, 0x7E, 0x3C, 0x18,
  0x1C, 0x57, 0x7D, 0x57, 0x1C,
  0x1C, 0x5E, 0x7F, 0x5E, 0x1C,
  0x00, 0x18, 0x3C, 0x18, 0x00,
  0xFF, 0xE7, 0xC3, 0xE7, 0xFF,
  0x00, 0x18, 0x24, 0x18, 0x00,
  0xFF, 0xE7, 0xDB, 0xE7, 0xFF,
  0x30, 0x48, 0x3A, 0x06, 0x0E,
  0x26, 0x29, 0x79, 0x29, 0x26,
  0x40, 0x7F, 0x05, 0x05, 0x07,
  0x40, 0x7F, 0x05, 0x25, 0x3F,
  0x5A, 0x3C, 0xE7, 0x3C, 0x5A,
  0x7F, 0x3E, 0x1C, 0x1C, 0x08,
  0x08, 0x1C, 0x1C, 0x3E, 0x7F,
  0x14, 0x22, 0x7F, 0x22, 0x14,
  0x5F, 0x5F, 0x00, 0x5F, 0x5F,
  0x06, 0x09, 0x7F, 0x01, 0x7F,
  0x00, 0x66, 0x89, 0x95, 0x6A,
  0x60, 0x60, 0x60, 0x60, 0x60,
  0x94, 0xA2, 0xFF, 0xA2, 0x94,
  0x08, 0x04, 0x7E, 0x04, 0x08,
  0x10, 0x20, 0x7E, 0x20, 0x10,
  0x08, 0x08, 0x2A, 0x1C, 0x08,
  0x08, 0x1C, 0x2A, 0x08, 0x08,
  0x1E, 0x10, 0x10, 0x10, 0x10,
  0x0C, 0x1E, 0x0C, 0x1E, 0x0C,
  0x30, 0x38, 0x3E, 0x38, 0x30,
  0x06, 0x0E, 0x3E, 0x0E, 0x06,
  0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x5F, 0x00, 0x00,
  0x00, 0x07, 0x00, 0x07, 0x00,
  0x14, 0x7F, 0x14, 0x7F, 0x14,
  0x24, 0x2A, 0x7F, 0x2A, 0x12,
  0x23, 0x13, 0x08, 0x64, 0x62,
  0x36, 0x49, 0x56, 0x20, 0x50,
  0x00, 0x08, 0x07, 0x03, 0x00,
  0x00, 0x1C, 0x22, 0x41, 0x00,
  0x00, 0x41, 0x22, 0x1C, 0x00,
  0x2A, 0x1C, 0x7F, 0x1C, 0x2A,
  0x08, 0x08, 0x3E, 0x08, 0x08,
  0x00, 0x80, 0x70, 0x30, 0x00,
  0x08, 0x08, 0x08, 0x08, 0x08,
  0x00, 0x00, 0x60, 0x60, 0x00,
  0x20, 0x10, 0x08, 0x04, 0x02,
  0x3E, 0x51, 0x49, 0x45, 0x3E, // 0
  0x00, 0x42, 0x7F, 0x40, 0x00, // 1
  0x72, 0x49, 0x49, 0x49, 0x46, // 2
  0x21, 0x41, 0x49, 0x4D, 0x33, // 3
  0x18, 0x14, 0x12, 0x7F, 0x10, // 4
  0x27, 0x45, 0x45, 0x45, 0x39, // 5
  0x3C, 0x4A, 0x49, 0x49, 0x31, // 6
  0x41, 0x21, 0x11, 0x09, 0x07, // 7
  0x36, 0x49, 0x49, 0x49, 0x36, // 8
  0x46, 0x49, 0x49, 0x29, 0x1E, // 9
  0x00, 0x00, 0x14, 0x00, 0x00,
  0x00, 0x40, 0x34, 0x00, 0x00,
  0x00, 0x08, 0x14, 0x22, 0x41,
  0x14, 0x14, 0x14, 0x14, 0x14,
  0x00, 0x41, 0x22, 0x14, 0x08,
  0x02, 0x01, 0x59, 0x09, 0x06,
  0x3E, 0x41, 0x5D, 0x59, 0x4E,
  0x7C, 0x12, 0x11, 0x12, 0x7C, // A
  0x7F, 0x49, 0x49, 0x49, 0x36, // B
  0x3E, 0x41, 0x41, 0x41, 0x22, // C
  0x7F, 0x41, 0x41, 0x41, 0x3E, // D
  0x7F, 0x49, 0x49, 0x49, 0x41, // E
  0x7F, 0x09, 0x09, 0x09, 0x01, // F
  0x3E, 0x41, 0x41, 0x51, 0x73, // G
  0x7F, 0x08, 0x08, 0x08, 0x7F, // H
  0x00, 0x41, 0x7F, 0x41, 0x00, // I
  0x20, 0x40, 0x41, 0x3F, 0x01, // J
  0x7F, 0x08, 0x14, 0x22, 0x41, // K
  0x7F, 0x40, 0x40, 0x40, 0x40, // L
  0x7F, 0x02, 0x1C, 0x02, 0x7F, // M
  0x7F, 0x04, 0x08, 0x10, 0x7F, // N
  0x3E, 0x41, 0x41, 0x41, 0x3E, // O
  0x7F, 0x09, 0x09, 0x09, 0x06, // P
  0x3E, 0x41, 0x51, 0x21, 0x5E, // Q
  0x7F, 0x09, 0x19, 0x29, 0x46, // R
  0x26, 0x49, 0x49, 0x49, 0x32, // S
  0x03, 0x01, 0x7F, 0x01, 0x03, // T
  0x3F, 0x40, 0x40, 0x40, 0x3F, // U
  0x1F, 0x20, 0x40, 0x20, 0x1F, // V
  0x3F, 0x40, 0x38, 0x40, 0x3F, // W
  0x63, 0x14, 0x08, 0x14, 0x63, // X
  0x03, 0x04, 0x78, 0x04, 0x03, // Y
  0x61, 0x59, 0x49, 0x4D, 0x43, // Z
  0x00, 0x7F, 0x41, 0x41, 0x41,
  0x02, 0x04, 0x08, 0x10, 0x20,
  0x00, 0x41, 0x41, 0x41, 0x7F,
  0x04, 0x02, 0x01, 0x02, 0x04,
  0x40, 0x40, 0x40, 0x40, 0x40,
  0x00, 0x03, 0x07, 0x08, 0x00,
  0x20, 0x54, 0x54, 0x78, 0x40, // a
  0x7F, 0x28, 0x44, 0x44, 0x38, // b
  0x38, 0x44, 0x44, 0x44, 0x28, // c
  0x38, 0x44, 0x44, 0x28, 0x7F, // d
  0x38, 0x54, 0x54, 0x54, 0x18, // e
  0x00, 0x08, 0x7E, 0x09, 0x02, // f
  0x18, 0xA4, 0xA4, 0x9C, 0x78, // g
  0x7F, 0x08, 0x04, 0x04, 0x78, // h
  0x00, 0x44, 0x7D, 0x40, 0x00, // i
  0x20, 0x40, 0x40, 0x3D, 0x00, // j
  0x7F, 0x10, 0x28, 0x44, 0x00, // k
  0x00, 0x41, 0x7F, 0x40, 0x00, // l
  0x7C, 0x04, 0x78, 0x04, 0x78, // m
  0x7C, 0x08, 0x04, 0x04, 0x78, // n
  0x38, 0x44, 0x44, 0x44, 0x38, // o
  0xFC, 0x18, 0x24, 0x24, 0x18, // p
  0x18, 0x24, 0x24, 0x18, 0xFC, // q
  0x7C, 0x08, 0x04, 0x04, 0x08, // r
  0x48, 0x54, 0x54, 0x54, 0x24, // s
  0x04, 0x04, 0x3F, 0x44, 0x24, // t
  0x3C, 0x40, 0x40, 0x20, 0x7C, // u
  0x1C, 0x20, 0x40, 0x20, 0x1C, // v
  0x3C, 0x40, 0x30, 0x40, 0x3C, // w
  0x44, 0x28, 0x10, 0x28, 0x44, // x
  0x4C, 0x90, 0x90, 0x90, 0x7C, // y
  0x44, 0x64, 0x54, 0x4C, 0x44, // z
  0x00, 0x08, 0x36, 0x41, 0x00,
  0x00, 0x00, 0x77, 0x00, 0x00,
  0x00, 0x41, 0x36, 0x08, 0x00,
  0x02, 0x01, 0x02, 0x04, 0x02,
  0x3C, 0x26, 0x23, 0x26, 0x3C,
  0x1E, 0xA1, 0xA1, 0x61, 0x12,
  0x3A, 0x40, 0x40, 0x20, 0x7A,
  0x38, 0x54, 0x54, 0x55, 0x59,
  0x21, 0x55, 0x55, 0x79, 0x41,
  0x21, 0x54, 0x54, 0x78, 0x41,
  0x21, 0x55, 0x54, 0x78, 0x40,
  0x20, 0x54, 0x55, 0x79, 0x40,
  0x0C, 0x1E, 0x52, 0x72, 0x12,
  0x39, 0x55, 0x55, 0x55, 0x59,
  0x39, 0x54, 0x54, 0x54, 0x59,
  0x39, 0x55, 0x54, 0x54, 0x58,
  0x00, 0x00, 0x45, 0x7C, 0x41,
  0x00, 0x02, 0x45, 0x7D, 0x42,
  0x00, 0x01, 0x45, 0x7C, 0x40,
  0xF0, 0x29, 0x24, 0x29, 0xF0,
  0xF0, 0x28, 0x25, 0x28, 0xF0,
  0x7C, 0x54, 0x55, 0x45, 0x00,
  0x20, 0x54, 0x54, 0x7C, 0x54,
  0x7C, 0x0A, 0x09, 0x7F, 0x49,
  0x32, 0x49, 0x49, 0x49, 0x32,
  0x32, 0x48, 0x48, 0x48, 0x32,
  0x32, 0x4A, 0x48, 0x48, 0x30,
  0x3A, 0x41, 0x41, 0x21, 0x7A,
  0x3A, 0x42, 0x40, 0x20, 0x78,
  0x00, 0x9D, 0xA0, 0xA0, 0x7D,
  0x39, 0x44, 0x44, 0x44, 0x39,
  0x3D, 0x40, 0x40, 0x40, 0x3D,
  0x3C, 0x24, 0xFF, 0x24, 0x24,
  0x48, 0x7E, 0x49, 0x43, 0x66,
  0x2B, 0x2F, 0xFC, 0x2F, 0x2B,
  0xFF, 0x09, 0x29, 0xF6, 0x20,
  0xC0, 0x88, 0x7E, 0x09, 0x03,
  0x20, 0x54, 0x54, 0x79, 0x41,
  0x00, 0x00, 0x44, 0x7D, 0x41,
  0x30, 0x48, 0x48, 0x4A, 0x32,
  0x38, 0x40, 0x40, 0x22, 0x7A,
  0x00, 0x7A, 0x0A, 0x0A, 0x72,
  0x7D, 0x0D, 0x19, 0x31, 0x7D,
  0x26, 0x29, 0x29, 0x2F, 0x28,
  0x26, 0x29, 0x29, 0x29, 0x26,
  0x30, 0x48, 0x4D, 0x40, 0x20,
  0x38, 0x08, 0x08, 0x08, 0x08,
  0x08, 0x08, 0x08, 0x08, 0x38,
  0x2F, 0x10, 0xC8, 0xAC, 0xBA,
  0x2F, 0x10, 0x28, 0x34, 0xFA,
  0x00, 0x00, 0x7B, 0x00, 0x00,
  0x08, 0x14, 0x2A, 0x14, 0x22,
  0x22, 0x14, 0x2A, 0x14, 0x08,
  0xAA, 0x00, 0x55, 0x00, 0xAA,
  0xAA, 0x55, 0xAA, 0x55, 0xAA,
  0x00, 0x00, 0x00, 0xFF, 0x00,
  0x10, 0x10, 0x10, 0xFF, 0x00,
  0x14, 0x14, 0x14, 0xFF, 0x00,
  0x10, 0x10, 0xFF, 0x00, 0xFF,
  0x10, 0x10, 0xF0, 0x10, 0xF0,
  0x14, 0x14, 0x14, 0xFC, 0x00,
  0x14, 0x14, 0xF7, 0x00, 0xFF,
  0x00, 0x00, 0xFF, 0x00, 0xFF,
  0x14, 0x14, 0xF4, 0x04, 0xFC,
  0x14, 0x14, 0x17, 0x10, 0x1F,
  0x10, 0x10, 0x1F, 0x10, 0x1F,
  0x14, 0x14, 0x14, 0x1F, 0x00,
  0x10, 0x10, 0x10, 0xF0, 0x00,
  0x00, 0x00, 0x00, 0x1F, 0x10,
  0x10, 0x10, 0x10, 0x1F, 0x10,
  0x10, 0x10, 0x10, 0xF0, 0x10,
  0x00, 0x00, 0x00, 0xFF, 0x10,
  0x10, 0x10, 0x10, 0x10, 0x10,
  0x10, 0x10, 0x10, 0xFF, 0x10,
  0x00, 0x00, 0x00, 0xFF, 0x14,
  0x00, 0x00, 0xFF, 0x00, 0xFF,
  0x00, 0x00, 0x1F, 0x10, 0x17,
  0x00, 0x00, 0xFC, 0x04, 0xF4,
  0x14, 0x14, 0x17, 0x10, 0x17,
  0x14, 0x14, 0xF4, 0x04, 0xF4,
  0x00, 0x00, 0xFF, 0x00, 0xF7,
  0x14, 0x14, 0x14, 0x14, 0x14,
  0x14, 0x14, 0xF7, 0x00, 0xF7,
  0x14, 0x14, 0x14, 0x17, 0x14,
  0x10, 0x10, 0x1F, 0x10, 0x1F,
  0x14, 0x14, 0x14, 0xF4, 0x14,
  0x10, 0x10, 0xF0, 0x10, 0xF0,
  0x00, 0x00, 0x1F, 0x10, 0x1F,
  0x00, 0x00, 0x00, 0x1F, 0x14,
  0x00, 0x00, 0x00, 0xFC, 0x14,
  0x00, 0x00, 0xF0, 0x10, 0xF0,
  0x10, 0x10, 0xFF, 0x10, 0xFF,
  0x14, 0x14, 0x14, 0xFF, 0x14,
  0x10, 0x10, 0x10, 0x1F, 0x00,
  0x00, 0x00, 0x00, 0xF0, 0x10,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
  0xFF, 0xFF, 0xFF, 0x00, 0x00,
  0x00, 0x00, 0x00, 0xFF, 0xFF,
  0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
  0x38, 0x44, 0x44, 0x38, 0x44,
  0x7C, 0x2A, 0x2A, 0x3E, 0x14,
  0x7E, 0x02, 0x02, 0x06, 0x06,
  0x02, 0x7E, 0x02, 0x7E, 0x02,
  0x63, 0x55, 0x49, 0x41, 0x63,
  0x38, 0x44, 0x44, 0x3C, 0x04,
  0x40, 0x7E, 0x20, 0x1E, 0x20,
  0x06, 0x02, 0x7E, 0x02, 0x02,
  0x99, 0xA5, 0xE7, 0xA5, 0x99,
  0x1C, 0x2A, 0x49, 0x2A, 0x1C,
  0x4C, 0x72, 0x01, 0x72, 0x4C,
  0x30, 0x4A, 0x4D, 0x4D, 0x30,
  0x30, 0x48, 0x78, 0x48, 0x30,
  0xBC, 0x62, 0x5A, 0x46, 0x3D,
  0x3E, 0x49, 0x49, 0x49, 0x00,
  0x7E, 0x01, 0x01, 0x01, 0x7E,
  0x2A, 0x2A, 0x2A, 0x2A, 0x2A,
  0x44, 0x44, 0x5F, 0x44, 0x44,
  0x40, 0x51, 0x4A, 0x44, 0x40,
  0x40, 0x44, 0x4A, 0x51, 0x40,
  0x00, 0x00, 0xFF, 0x01, 0x03,
  0xE0, 0x80, 0xFF, 0x00, 0x00,
  0x08, 0x08, 0x6B, 0x6B, 0x08,
  0x36, 0x12, 0x36, 0x24, 0x36,
  0x06, 0x0F, 0x09, 0x0F, 0x06,
  0x00, 0x00, 0x18, 0x18, 0x00,
  0x00, 0x00, 0x10, 0x10, 0x00,
  0x30, 0x40, 0xFF, 0x01, 0x01,
  0x00, 0x1F, 0x01, 0x01, 0x1E,
  0x00, 0x19, 0x1D, 0x17, 0x12,
  0x00, 0x3C, 0x3C, 0x3C, 0x3C,
  0x00, 0x00, 0x00, 0x00, 0x00,
};

// Because compiler complained
void InitSPI(void);
void WriteSPI(uint8_t data);
void LCD_Command(uint8_t command);
void LCD_Data(uint8_t data);
void LCD_DataBuffer(uint8_t *buffer, uint32_t count);
void LCD_gCharT(int16_t x, int16_t y, char c, pixel textColor, uint8_t size);

// Initializes SSI as SPI to EDUMKII display
void InitSPI(void)
{
    SYSCTL->RCGCSSI |= (1 << 2);            // Enable SSI2
    SYSCTL->RCGCGPIO |= 0x03;               // Enable GPIOA & B
    while (!(SYSCTL->PRGPIO & 0x03));       // Wait for enabled signal
    
    GPIOB->AFSEL |= (1 << 4) | (1 << 7);    // AF for PB4, PB7 which are Clk and MOSI
    GPIOB->PCTL |= (2 << 16) | (2 << 28);   // Select right AF pins
    GPIOB->DEN |= (1 << 4) | (1 << 7);      // Digital enable
    
    GPIOA->CR |= (1 << 4);                  // Enable pin 4
    GPIOA->AFSEL &= ~(1 << 4);              // Disable AF for PA4 which is CS
    GPIOA->DIR |= (1 << 4);                 // PA4 is output
    GPIOA->DEN |= (1 << 4);                 // Digital enable
    GPIOA->DATA |= (1 << 4);                // CS high disables transmission
    
    SSI2->CR1 &= ~(1 << 1);                 // Ensure SSE bit is 0 before making changes
    SSI2->CR1 = 0x0;                        // Set SSI as master
    SSI2->CC = 0x0;                         // Set SSI clock source
    SSI2->CPSR = 2;                         // Clock prescale divisor
    SSI2->CR0 = (2 << 8) | 0x07;            // Set serial clock rate, clock phase/polarity, protocol mode, data size (DSS)
    SSI2->CR1 |= (1 << 1);                  // Enable SSI by setting SSE bit
    
    SYSCTL->RCGCGPIO |= (1 << 5);           // Enable GPIOF
    while (!(SYSCTL->PRGPIO & (1 << 5)));   // Wait for enabled signal
    
    GPIOF->CR |= 1 | (1 << 4);              // PF0 is Reset negative, low is reset, PF4 is Register select, or D/C pin
    GPIOF->PUR &= ~(1 | (1 << 4));          // No pull up, in case buttons were set up
    GPIOF->DIR |= 1 | (1 << 4);             // Output
    GPIOF->DEN |= 1 | (1 << 4);             // Digital enable
}

// LCD initialization
//  Color mode: 18-bit/6-6-6 RGB
void LCD_Init(void)
{
    InitSPI();
    
    GPIOF->DATA |= HIGH;                    // Pull reset down, is negative logic
    delay(150);
    
    LCD_CS(LOW);
    LCD_Command(LCD_SWRESET);
    delay(150);           // 120 ms or more wait after SW reset
    
    LCD_Command(LCD_SLPOUT);                // Turn off sleep mode
    delay(150);           // 120 ms or more wait after sleep out
    
    LCD_Command(LCD_FRMCTR1);               // Framerate control
    LCD_Data(0x00);
    LCD_Data(0x06);
    LCD_Data(0x03);
    
    LCD_Command(LCD_PWCTR1);
    LCD_Data(0xA2);
    LCD_Data(0x02);
    LCD_Data(0x84);
    LCD_Command(LCD_PWCTR2);
    LCD_Data(0xC5);
    LCD_Command(LCD_PWCTR3);
    LCD_Data(0x0A);
    LCD_Data(0x00);
    LCD_Command(LCD_PWCTR4);
    LCD_Data(0x8A);
    LCD_Data(0x2A);
    LCD_Command(LCD_PWCTR5);
    LCD_Data(0xEE);
    LCD_Data(0x8A);
    
    _active_settings.MemoryAccessCTL = LCD_MADCTL_MX | LCD_MADCTL_MY | LCD_MADCTL_BGR;
    LCD_Command(LCD_MADCTL);                // Set memory access control
    LCD_Data(_active_settings.MemoryAccessCTL);
    
    _active_settings.ColorMode = LCD_PIXEL_FORMAT_666;
    LCD_Command(LCD_COLMOD);                // Set interface pixel format
    LCD_Data(_active_settings.ColorMode);
    delay(10);
    
    _active_settings.InversionMode = LCD_INVOFF;    // Screen inversion off
    LCD_Command(_active_settings.InversionMode);
    LCD_Command(LCD_GAMMA_PREDEFINED_4);    // Gamma curve 4
    
    LCD_Command(LCD_TEOFF);
    
    LCD_Command(LCD_DISPON);                // Turn LCD on
    delay(150);

    _active_settings.BGColor = LCD_BLACK;
    LCD_CS(HIGH);
}

// Get LCD settings
//  Return:
//      struct LCD_Settings: currently active settings
LCD_Settings LCD_GetSettings()
{
    return _active_settings;
}

// Set LCD Background color
// Used by some drawing primitives
//  Param:
//      pixel: new background color
void LCD_SetBGColor(pixel bgColor)
{
    _active_settings.BGColor = bgColor;
}

// Turn display inversion on or off
//  Param:
//      flag: 1 or 0, ON or OFF
void LCD_SetInversion(uint8_t flag)
{
    _active_settings.InversionMode = flag;
    LCD_Command(flag ? LCD_INVON : LCD_INVOFF);
}

// Write a byte of data to the SPI buffer and wait for it to be sent
void WriteSPI(uint8_t data)
{
    SSI2->DR = data;
    while (!(SSI2->SR & 0x1));
}

// LCD send command byte
// Sets Register select to Command mode and sends a byte
//  Param:
//      command: opcode
void LCD_Command(uint8_t command)
{
    GPIOF->DATA &= ~(1 << 4);   // Command mode
    WriteSPI(command);
}

// LCD send data byte
// Sets Register select to Data mode and sends a byte
//  Param:
//      data: data byte
void LCD_Data(uint8_t data)
{
    GPIOF->DATA |= (1 << 4);    // Data mode
    WriteSPI(data);
}

// LCD send data bytes
// Sets Register select to Data mode and sends multiple bytes
//  Param:
//      buffer: data byte array
//      count: buffer element count
void LCD_DataBuffer(uint8_t *buffer, uint32_t count)
{
    GPIOF->DATA |= (1 << 4);    // Data mode
    for (uint32_t i = 0; i < count; i++)
    {
        WriteSPI(buffer[i]);
    }
}

// LCD SPI Chip Select
// Tell LCD whether to process transmitted data
//  Param:
//      flag: HIGH = Deselect, LOW = Select
void LCD_CS(uint8_t flag)
{
    if (!flag)
        GPIOA->DATA &= ~(1 << 4);
    else
    {
        for (int i = 0; i < 15; i++);   // Small delay to end transmission with enough time to spare
        GPIOA->DATA |= (1 << 4);
    }
}

// LCD set window position / area
// Define area to draw inside of. Out of range vaues ignored
//  Param:
//      colStart: starting column
//      rowStart: starting row
//      colEnd: ending column < LCD_WIDTH
//      rowEnd: ending row < LCD_HEIGHT
void LCD_SetArea(int16_t colStart, int16_t rowStart, int16_t colEnd, int16_t rowEnd)
{
    uint8_t buffer[4];
    
    // Swap values if invalid
    if (colEnd < colStart)
    {
        buffer[0] = colEnd;
        colEnd = colStart;
        colStart = buffer[0];
    }
    if (rowEnd < rowStart)
    {
        buffer[0] = rowEnd;
        rowEnd = rowStart;
        rowStart = buffer[0];
    }

    // Check for range
    if (colStart < 0)
        colStart = 0;
    if (rowStart < 0)
        rowStart = 0;
    if (colEnd >= LCD_WIDTH)
        colEnd = LCD_WIDTH - 1;
    if (rowEnd >= LCD_HEIGHT)
        rowEnd = LCD_HEIGHT - 1;
    
    colStart += 2;
    colEnd += 2;
    rowStart += 3;
    rowEnd += 3;
    
    /* write column address; requires 4 bytes of the buffer */
    buffer[0] = (colStart >> 8) & 0x00FF; /* MSB */ /* =0 for ST7735S */
    buffer[1] =  colStart       & 0x00FF; /* LSB */
    buffer[2] = (colEnd   >> 8) & 0x00FF; /* MSB */ /* =0 for ST7735S */
    buffer[3] =  colEnd         & 0x00FF; /* LSB */
    LCD_Command(LCD_CASET);
    LCD_DataBuffer(buffer, 4);
    
    /* write row address; requires 4 bytes of the buffer */
    buffer[0] = (rowStart >> 8) & 0x00FF; /* MSB */ /* =0 for ST7735S */
    buffer[1] =  rowStart       & 0x00FF; /* LSB */
    buffer[2] = (rowEnd   >> 8) & 0x00FF; /* MSB */ /* =0 for ST7735S */
    buffer[3] =  rowEnd         & 0x00FF; /* LSB */
    LCD_Command(LCD_RASET);
    LCD_DataBuffer(buffer, 4);
}

// LCD activate memory write
// Sends RAM write command, after which any number of pixels can be sent
void LCD_ActivateWrite(void)
{
    LCD_Command(LCD_RAMWR);
}

// LCD Draw pixel
// Sets area of 1 pixel and sends pixel data. Requires LCD_ActivateWrite
// to have been the last command
//  Param:
//      red, green, blue: color value. Bits [5:0] (6 bits) are sent
void LCD_gDrawPixel(uint8_t x, uint8_t y, uint8_t red, uint8_t green, uint8_t blue)
{
    LCD_SetArea(x, y, x, y);
    LCD_ActivateWrite();

    LCD_PushPixel(red, green, blue);
}

// LCD write pixel data
// Sends pixel data to current active window. Requires LCD_ActivateWrite
// to have been the last command
//  Param:
//      red, green, blue: color value. Bits [5:0] (6 bits) are sent
void LCD_PushPixel(uint8_t red, uint8_t green, uint8_t blue)
{
    LCD_Data(red << 2);
    LCD_Data(green << 2);
    LCD_Data(blue << 2);
}

// Convert a 3 byte pixel (Eg #FF004A) uint32_t into pixel
// Precision loss: 8-bit -> 6-bit
//  Param:
//      p: 32-bit integer. Bits [23:0] are used
pixel LCD_Ui32ToPixel(uint32_t p)
{
    pixel pixel;
    pixel.r = ((p >> 16) & 0xFF) >> 2;
    pixel.g = ((p >> 8) & 0xFF) >> 2;
    pixel.b = (p & 0xFF) >> 2;
    return pixel;
}

// Clear screen to background color
void LCD_gClear()
{
    LCD_gFillRect(0, 0, LCD_WIDTH, LCD_HEIGHT, _active_settings.BGColor);
}

void LCD_gVLine(int16_t x, int16_t y1, int16_t y2, uint8_t stroke, pixel color)
{
    int16_t aux;
    
    if (stroke == 0)
        return;
    
    if (y1 > y2)
    {
        aux = y1;
        y1 = y2;
        y2 = aux;
    }

    // Check that the line is inside the screen. If not, no point clipping it to the edge
    if ((x - ((stroke - 1) >> 1) < 0 && x + (stroke >> 1) < 0) || (x - ((stroke - 1) >> 1) > (LCD_WIDTH - 1) && x + (stroke >> 1) > (LCD_WIDTH - 1)))
        return;

    x = max(0, x - ((stroke - 1) >> 1));
    aux = min(LCD_WIDTH - 1, x + (stroke >> 1));
    LCD_gFillRect(x, y1, aux - x + 1, y2 - y1 + 1, color);
}

void LCD_gHLine(int16_t x1, int16_t x2, int16_t y, uint8_t stroke, pixel color)
{
    int16_t aux;
    
    if (stroke == 0)
        return;
    
    if (x1 > x2)
    {
        aux = x1;
        x1 = x2;
        x2 = aux;
    }
    
    // Check that the line is inside the screen. If not, no point clipping it to the edge
    if ((y - ((stroke - 1) >> 1) < 0 && y + (stroke >> 1) < 0) || (y - ((stroke - 1) >> 1) > (LCD_HEIGHT - 1) && y + (stroke >> 1) > (LCD_HEIGHT - 1)))
        return;

    y = max(0, y - ((stroke - 1) >> 1));
    aux = min(LCD_HEIGHT - 1, y + (stroke >> 1));
    LCD_gFillRect(x1, y, x2 - x1 + 1, aux - y + 1, color);
}

void LCD_gLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t stroke, pixel color)
{
    /*
        Steps:
            1. Define octant
            2. Bresenham: draw pixel by pixel using LCD_gDrawPixel
            3. If stroke is not complete, define shift in position for next line
    */

    float m;
    int16_t aux;
    uint8_t octant;
    int16_t x, y;
   
    // 1. Define octant
    // Stay within one half to simplify calculations. This can be done by
    // swapping x's and y's when needed

    // straight lines are faster with their dedicated primitives 
    if (x1 == x2)
        return LCD_gVLine(x1, y1, y2, stroke, color);
    if (y1 == y2)
        return LCD_gHLine(x1, x2, y1, stroke, color);

    // move everything to quadrants 1 & 4
    if (x2 < x1)
    {
        aux = x2;
        x2 = x1;
        x1 = aux;
        aux = y2;
        y2 = y1;
        y1 = aux;
    }
    
    // find slope avoiding division by 0 with vertical lines
    m = (float) (y2 - y1) / (x2 - x1);

    // find octant
    if (m > 1.0f)
        octant = 2;
    else if (m > 0.0f)
        octant = 1;
    else if (m > -1.0f)
        octant = 8;
    else
        octant = 7;
    
    // 2. Bresenham's
    x = x1;
    y = y1;

    switch (octant)
    {
    case 2:
        for (y = y1; y <= y2 && y < LCD_HEIGHT; y++)
        {
            x = (y - y1) / m + x1;
            LCD_gDrawPixel((uint8_t) x, (uint8_t) y, color.r, color.g, color.b);
        }
        break;
    case 1:
        for (x = x1; x <= x2 && x < LCD_WIDTH; x++)
        {
            y = m * (x - x1) + y1;
            LCD_gDrawPixel((uint8_t) x, (uint8_t) y, color.r, color.g, color.b);
        }
        break;
    case 8:
        for (x = x1; x <= x2 && x < LCD_WIDTH; x++)
        {
            y = m * (x - x1) + y1;
            LCD_gDrawPixel((uint8_t) x, (uint8_t) y, color.r, color.g, color.b);
        }
        break;
    case 7:
        for (y = y1; y >= y2 && y >= 0; y--)
        {
            x = (y - y1) / m + x1;
            LCD_gDrawPixel((uint8_t) x, (uint8_t) y, color.r, color.g, color.b);
        }
        break;
    default:
        return;
    }

    // 3. If stroke is not complete, define shift in position for next line
    if (stroke > 1)
    {
        for (int i = - (stroke >> 1); i <= ((stroke - 1) >> 1); i++)
        {
            // original line is already drawn
            if (i == 0)
                continue;
            
            // move coordinates somewhere else and draw line with stroke 1
            switch (octant)
            {
            case 2: case 7:
                x1++;
                x2++;
                break;
            case 1: case 8:
                y1++;
                y2++;
                break;
            default:
                break;
            }

            LCD_gLine(x1, y1, x2, y2, 1, color);
        }
        return;
    }
}

// Filled rectangle
//  Param:
//      x, y: column and row of first corner
//      w, h: width and height
//      color: pixel
void LCD_gFillRect(int16_t x, int16_t y, uint8_t w, uint8_t h, pixel color)
{
    LCD_SetArea(x, y, x + w - 1, y + h - 1);
    LCD_ActivateWrite();

    for (int i = 0; i <= h; i++)
        for (int j = 0; j <= w; j++)
            LCD_PushPixel(color.r, color.g, color.b);
}

// Rectangle outline
//  Param:
//      x, y: column and row of first corner
//      w, h: width and height
//      color: pixel
void LCD_gRect(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t stroke, pixel color)
{
    if (w == 0 || h == 0)
        return;

    LCD_gVLine(x, y, y + h, stroke, color);
    LCD_gVLine(x + w - (stroke >> 1), y, y + h, stroke, color);
    LCD_gHLine(x, x + w, y, stroke, color);
    LCD_gHLine(x, x + w, y + h - (stroke >> 1), stroke, color);
}

// Triangle outline
//  Param:
//      v1: first vertex
//      v2: second vertex
//      v3: third vertex
//      stroke: edge width
//      color: pixel
void LCD_gTriangle(point v1, point v2, point v3, uint8_t stroke, pixel color)
{
    LCD_gLine(v1.x, v1.y, v2.x, v2.y, stroke, color);
    LCD_gLine(v2.x, v2.y, v3.x, v3.y, stroke, color);
    LCD_gLine(v3.x, v3.y, v1.x, v1.y, stroke, color);
}

// Filled Triangle
//  Param:
//      v1: first vertex
//      v2: second vertex
//      v3: third vertex
//      color: pixel
void LCD_gFillTriangle(point v1, point v2, point v3, pixel color)
{
    // Algorithm taken from https://www.sunshine2k.de/coding/java/TriangleRasterization/TriangleRasterization.html
    // Note that in this function top is higher y, when in reality this is not necessarily the case.
    // This does not matter though

    point top_v, bottom_v, middle_v, aux_v;
    float invslope1, invslope2;
    float curx1, curx2;

    // Sort points
    if (v1.y < v2.y && v1.y < v3.y)
    {
        bottom_v = v1;
        // cmp v2 v3 for top
        if (v2.y > v3.y)
        {
            top_v = v2;
            middle_v = v3;
        } else
        {
            top_v = v3;
            middle_v = v2;
        }
    } else if (v1.y > v2.y && v1.y > v3.y)
    {
        top_v = v1;
        // cmp v2 v3 for bottom
        if (v2.y < v3.y)
        {
            bottom_v = v2;
            middle_v = v3;
        } else
        {
            bottom_v = v3;
            middle_v = v2;
        }
    } else
    {
        middle_v = v1;
        // cmp v2 v3 for top and bottom
        if (v2.y > v3.y)
        {
            top_v = v2;
            bottom_v = v3;
        } else
        {
            top_v = v3;
            bottom_v = v2;
        }
    }

    // auxiliary point to divide the triangle into 2 sections
    aux_v.x = (int)(top_v.x + ((float)(middle_v.y - top_v.y) / (float)(bottom_v.y - top_v.y)) * (bottom_v.x - top_v.x));
    aux_v.y = middle_v.y;

    // Draw flat bottom triangle
    if (middle_v.y - top_v.y != 0 && aux_v.y - top_v.y != 0)
    {
        invslope1 = (float) (middle_v.x - top_v.x) / (middle_v.y - top_v.y);
        invslope2 = (float) (aux_v.x - top_v.x) / (aux_v.y - top_v.y);

        curx1 = top_v.x;
        curx2 = top_v.x;

        for (int16_t scanlineY = top_v.y; scanlineY > middle_v.y; scanlineY--)
        {
            LCD_gHLine((int16_t) curx1, (int16_t) curx2, scanlineY, 1, color);
            curx1 += invslope1;
            curx2 += invslope2;
        }
    }

    // Draw flat top triangle
    if (bottom_v.y - middle_v.y != 0 && bottom_v.y - aux_v.y != 0)
    {
        invslope1 = (float) (bottom_v.x - middle_v.x) / (bottom_v.y - middle_v.y);
        invslope2 = (float) (bottom_v.x - aux_v.x) / (bottom_v.y - aux_v.y);

        curx1 = bottom_v.x;
        curx2 = bottom_v.x;

        for (int16_t scanlineY = bottom_v.y; scanlineY <= middle_v.y; scanlineY++)
        {
            LCD_gHLine((int16_t) curx1, (int16_t) curx2, scanlineY, 1, color);
            curx1 -= invslope1;
            curx2 -= invslope2;
        }
    }
}

// Arbitrary polygon outline
// Edges are drawn in order from the first to the last and back to the first
//  Param:
//      vertices: array of vertices
//      n_vertices: size of vertices array
//      stroke: edge width
//      color: pixel
void LCD_gPolygon(point *vertices, int n_vertices, uint8_t stroke, pixel color)
{
    int i;
    for (i = 0; i < n_vertices - 1; i++)
    {
        LCD_gLine(vertices[i].x, vertices[i].y, vertices[i + 1].x, vertices[i + 1].y, stroke, color);
    }
    LCD_gLine(vertices[i].x, vertices[i].y, vertices[0].x, vertices[0].y, stroke, color);
}

// Circle outline
//  Param:
//      x, y: circle center position
//      r: circle radius
//      stroke: outline width
//      color: pixel
void LCD_gCircle(int16_t x, int16_t y, float r, uint8_t stroke, pixel color)
{
    int16_t x_ = 1, y_ = r;
    int16_t r2 = r * r + 1;
    int16_t res, res2;

    if (r == 0)
        return;
    
    // intersections with X and Y are easy using the radius
    LCD_gDrawPixel(x + r, y, color.r, color.g, color.b);
    LCD_gDrawPixel(x - r, y, color.r, color.g, color.b);
    LCD_gDrawPixel(x, y + r, color.r, color.g, color.b);
    LCD_gDrawPixel(x, y - r, color.r, color.g, color.b);

    while (y_ / x_ >= 1)
    {
        // calculate next point, which will be down 1 pixel or on the same height
        // see which one satisfies x^2 + y^2 >= r^2 and maximizes x^2 + y^2
        // draw point, copy over to other octants, update x_ and y_
        res = x_ * x_ + y_ * y_;
        res2 = x_ * x_ + (y_ - 1) * (y_ - 1);
        
        if (res > r2 || res2 > res)
        {
            y_--;
        }

        LCD_gDrawPixel(x + x_, y - y_, color.r, color.g, color.b);
        LCD_gDrawPixel(x - x_, y - y_, color.r, color.g, color.b);
        LCD_gDrawPixel(x + x_, y + y_, color.r, color.g, color.b);
        LCD_gDrawPixel(x - x_, y + y_, color.r, color.g, color.b);
        LCD_gDrawPixel(x + y_, y - x_, color.r, color.g, color.b);
        LCD_gDrawPixel(x - y_, y - x_, color.r, color.g, color.b);
        LCD_gDrawPixel(x + y_, y + x_, color.r, color.g, color.b);
        LCD_gDrawPixel(x - y_, y + x_, color.r, color.g, color.b);
        x_++;
    }

    return;
}

// Filled cricle
//  Param:
//      x, y: circle center position
//      r: circle radius
//      color: pixel
void LCD_gFillCircle(int16_t x, int16_t y, float r, pixel color)
{
    int16_t x_ = 1, y_ = r;
    int16_t r2 = r * r + 1;
    int16_t res, res2;

    if (r == 0)
        return;
    
    // intersections with X and Y are easy using the radius
    LCD_gHLine(x - r, x + r, y, 1, color);
    LCD_gDrawPixel(x, y + r, color.r, color.g, color.b);
    LCD_gDrawPixel(x, y - r, color.r, color.g, color.b);

    while (y_ / x_ >= 1)
    {
        // calculate next point, which will be down 1 pixel or on the same height
        // see which one satisfies x^2 + y^2 >= r^2 and maximizes x^2 + y^2
        // draw point, copy over to other octants, update x_ and y_
        res = x_ * x_ + y_ * y_;
        res2 = x_ * x_ + (y_ - 1) * (y_ - 1);
        
        // Lines only need to be drawn once for some octants, single pixels can be drawn if y_ does not change
        if (res > r2 || res2 > res)
        {
            y_--;

            LCD_gHLine(x - x_, x + x_, y - y_, 1, color);
            LCD_gHLine(x - x_, x + x_, y + y_, 1, color);
        } else
        {
            LCD_gDrawPixel(x + x_, y - y_, color.r, color.g, color.b);
            LCD_gDrawPixel(x - x_, y - y_, color.r, color.g, color.b);
            LCD_gDrawPixel(x + x_, y + y_, color.r, color.g, color.b);
            LCD_gDrawPixel(x - x_, y + y_, color.r, color.g, color.b);
        }

        LCD_gHLine(x - y_, x + y_, y - x_, 1, color);
        LCD_gHLine(x - y_, x + y_, y + x_, 1, color);

        x_++;
    }
}

// Draw character
// Draws a 5x7 character on the given position. If the background color is the same as the
// text color, the background is transparent
//  Param:
//      x, y: top left corner position
//      c: character to draw
//      textColor: character color
//      bgColor: background color
//      size: scale of the character
void LCD_gChar(int16_t x, int16_t y, char c, pixel textColor, pixel bgColor, uint8_t size)
{
    uint8_t line;

    // no clipping the edges of the screen
    if ((x + 5 * size - 1) >= LCD_WIDTH ||
        (y + 8 * size - 1) >= LCD_HEIGHT ||
        x < 0 || y < 0)
        return;
    
    if (textColor.r == bgColor.r && textColor.g == bgColor.g && textColor.b == bgColor.b)
    {
        LCD_gCharT(x, y, c, textColor, size);
        return;
    }

    LCD_SetArea(x + 1, y + 1, x + 6 * size, y + 8 * size);
    LCD_ActivateWrite();

    line = 0x01;    // print top row first
    // print rows, starting at the top
    for (int row = 0; row < 8; row++, line <<= 1)
    {
        for (int i = 0; i < size; i++)
        {
            // print columns, starting on the left
            for (int col = 0; col < 5; col++)
            {
                // Only look at pixel in the correct row
                if (Font[c * 5 + col] & line)
                {
                    for (int j = 0; j < size; j++)
                        LCD_PushPixel(textColor.r, textColor.g, textColor.b);
                } else
                {
                    for (int j = 0; j < size; j++)
                        LCD_PushPixel(bgColor.r, bgColor.g, bgColor.b);
                }
            }
            // print blank column to the right of the character
            for (int j = 0; j < size; j++)
                LCD_PushPixel(bgColor.r, bgColor.g, bgColor.b);
        }
    }
    // print black column on the left of the character
    LCD_gVLine(x, y + 1, y + 8 * size, 1, bgColor);
}

// Draw character
// Draws a 5x7 character on the given position. If the background color is the same as the
// text color, the background is transparent
//  Param:
//      x, y: top left corner position
//      c: character to draw
//      textColor: character color
//      bgColor: background color
//      size: scale of the character
void LCD_gCharT(int16_t x, int16_t y, char c, pixel textColor, uint8_t size)
{
    uint8_t line;

    // no clipping the edges of the screen
    if ((x + 5 * size - 1) >= LCD_WIDTH ||
        (y + 8 * size - 1) >= LCD_HEIGHT ||
        x < 0 || y < 0)
        return;

    for (int i = 0; i < 6; i++)
    {
        if (i == 5)
            line = 0x00;
        else
            line = Font[c * 5 + i];
        for (int j = 0; j < 8; j++, line >>= 1)
        {
            if (line & 0x01)
            {
                if (size == 1)
                    LCD_gDrawPixel((uint8_t) x + i + 1, (uint8_t) y + j + 1, textColor.r, textColor.g, textColor.b);
                else
                    LCD_gFillRect(x + 1, y + 1, i * size, j * size, textColor);
            }
        }
    }
}

// Draw string
// Draws a series of 5x7 monospace characters. Size is fixed to 1 and backround for
// the text is the background color set with LCD_SetBGColor. If the background color
// is the same as the text color, the background is transparent
// 16 rows (0 - 15) and 21 columns (0 - 20)
//  Param:
//      x: column (0 - 21)
//      y: row (0 - 15)
//      str: string to draw
//      len: amount of characters to be printed, if 0 prints as many as possible
//      textColor: character color
//  Return:
//      number of characters printed
uint32_t LCD_gString(int16_t x, int16_t y, char *str, uint8_t len, pixel textColor)
{
    uint32_t count = 0;

    if (y > 15) return 0;
    
    while (*str)
    {
        LCD_gChar(x * 6, y * 8, *str, textColor, _active_settings.BGColor, 1);
        str++;
        x += 1;
        if (x > 20) return count;
        count++;
        if (len && count == len)
            return count;
    }
    return count;
}
