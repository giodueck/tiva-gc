#include "LCD.h"
#include "TM4C123GH6PM.h"
#include "delay.h"

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
static struct LCD_Settings _active_settings = {0};

void InitSPI(void);
void WriteSPI(uint8_t data);

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
    
    GPIOF->DATA |= HIGH;
    delay(150);
    GPIOF->DATA |= LOW;
    delay(150);
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
struct LCD_Settings LCD_GetSettings()
{
    return _active_settings;
}

// Set LCD Background color
// Used by some drawing primitives
//  Param:
//      LCD_pixel: new background color
void LCD_SetBGColor(LCD_pixel bgColor)
{
    _active_settings.BGColor = bgColor;
}

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
    for (int i = 0; i < count; i++)
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

// Convert a 3 byte pixel (Eg #FF004A) uint32_t into LCD_pixel
// Precision loss: 8-bit -> 6-bit
//  Param:
//      p: 32-bit integer. Bits [23:0] are used
LCD_pixel LCD_Ui32ToPixel(uint32_t p)
{
    LCD_pixel pixel;
    pixel.r = (uint8_t)((float)(p & 0xFF0000 >> 16) / 0xFF * 0x3F);
    pixel.g = (uint8_t)((float)(p & 0xFF00 >> 8) / 0xFF * 0x3F);
    pixel.b = (uint8_t)((float)(p & 0xFF) / 0xFF * 0x3F);
    return pixel;
}

// Clear screen to background color
void LCD_gClear()
{
    LCD_gFillRectangle(0, 0, LCD_WIDTH, LCD_HEIGHT, _active_settings.BGColor);
}

void LCD_gVLine(uint8_t x, uint8_t y1, uint8_t y2, uint8_t stroke, LCD_pixel color)
{
    if (stroke == 0)
        return;

    // Rectangular area, y2 - y1 pixels high, stroke pixels wide
    LCD_SetArea(x - stroke / 2, y1, x + stroke / 2 + stroke % 2, y2);
    LCD_ActivateWrite();

    for (int i = 0; i < (y2 - y1) * stroke; i++)
        LCD_PushPixel(color.r, color.g, color.b);
}

void LCD_gHLine(uint8_t x1, uint8_t x2, uint8_t y, uint8_t stroke, LCD_pixel color)
{
    if (stroke == 0)
        return;

    // Rectangular area, y2 - y1 pixels high, stroke pixels wide
    LCD_SetArea(x1, y - stroke / 2, x2, y + stroke / 2 + stroke % 2);
    LCD_ActivateWrite();

    for (int i = 0; i < (x2 - x1) * stroke; i++)
        LCD_PushPixel(color.r, color.g, color.b);
}

void LCD_gLine(uint8_t x1, uint8_t x2, uint8_t y1, uint8_t y2, uint8_t stroke, LCD_pixel color)
{
    /*
        Steps:
            1. Define octant
            2. Brezenham in a buffer
            3. Draw pixel by pixel using LCD_gDrawPixel
            4. If stroke is not complete, define shift in position for next line
            5. Goto 3 if stroke stroke not complete
    */
}

// Filled rectangle
//  Param:
//      x, y: column and row of first corner
//      w, h: width and height
//      color: LCD_pixel
void LCD_gFillRectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h, LCD_pixel color)
{
    LCD_SetArea(x, y, x + w, y + h);
    LCD_ActivateWrite();

    for (int i = 0; i <= h; i++)
        for (int j = 0; j <= w; j++)
            LCD_PushPixel(color.r, color.g, color.b);
}

// Rectangle outline
//  Param:
//      x, y: column and row of first corner
//      w, h: width and height
//      color: LCD_pixel
void LCD_gRectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t stroke, LCD_pixel color)
{
    if (w == 0 || h == 0)
        return;

    LCD_gFillRectangle(x, y, w, h, color);
    
    if (w <= 2 * stroke || h <= 2 * stroke)
        return;

    LCD_gFillRectangle(x + stroke, y + stroke, w - 2 * stroke, h - 2 * stroke, _active_settings.BGColor);
}
