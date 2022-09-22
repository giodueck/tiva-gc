#ifndef LCD_H
#define LCD_H

#include <stdint.h>

#define HIGH 1
#define LOW  0

#define LCD_HEIGHT 132
#define LCD_WIDTH  132

// Pixel: 18-bits
// Only bits [5:0] are used
typedef struct LCD_pixel
{
    uint8_t r, g, b;
} LCD_pixel;

#define LCD_BLACK   (LCD_pixel) { 0x00, 0x00, 0x00 }
#define LCD_WHITE   (LCD_pixel) { 0x3F, 0x3F, 0x3F }
#define LCD_RED     (LCD_pixel) { 0x3F, 0x00, 0x00 }
#define LCD_GREEN   (LCD_pixel) { 0x00, 0x3F, 0x00 }
#define LCD_BLUE    (LCD_pixel) { 0x00, 0x00, 0x3F }
#define LCD_YELLOW  (LCD_pixel) { 0x3F, 0x3F, 0x00 }
#define LCD_MAGENTA (LCD_pixel) { 0x3F, 0x00, 0x3F }
#define LCD_CYAN    (LCD_pixel) { 0x00, 0x3F, 0x3F }

struct LCD_Settings
{
    uint8_t InversionMode;
    uint8_t ColorMode;
    uint8_t MemoryAccessCTL;
    LCD_pixel BGColor;
};



/* Initialization and settings
 */

// LCD initialization
//  Color mode: 18-bit/6-6-6 RGB
void LCD_Init(void);

// Get LCD settings
//  Return:
//      struct LCD_Settings: currently active settings
struct LCD_Settings LCD_GetSettings(void);

// Set LCD Background color
// Used by some drawing primitives
//  Param:
//      LCD_pixel: new background color
void LCD_SetBGColor(LCD_pixel bgColor);



/* Low level control and interfacing
 */

// LCD SPI Chip Select
// Tell LCD whether to process transmitted data
//  Param:
//      flag: HIGH = Deselect, LOW = Select
void LCD_CS(uint8_t flag);

// LCD send command byte
// Sets Register select to Command mode and sends a byte
//  Param:
//      command: opcode
void LCD_Command(uint8_t command);

// LCD send data byte
// Sets Register select to Data mode and sends a byte
//  Param:
//      data: data byte
void LCD_Data(uint8_t data);

// LCD send data bytes
// Sets Register select to Data mode and sends multiple bytes
//  Param:
//      buffer: data byte array
//      count: buffer element count
void LCD_DataBuffer(uint8_t *buffer, uint32_t count);

// LCD set window position / area
// Define area to draw inside of. Out of range vaues ignored
//  Param:
//      colStart: starting column
//      rowStart: starting row
//      colEnd: ending column < LCD_WIDTH
//      rowEnd: ending row < LCD_HEIGHT
void LCD_SetArea(int16_t colStart, int16_t rowStart, int16_t colEnd, int16_t rowEnd);

// LCD activate memory write
// Sends RAM write command, after which any number of pixels can be sent
void LCD_ActivateWrite(void);

// LCD write pixel data
// Sends pixel data to current active window. Requires LCD_ActivateWrite
// to have been the last command
//  Param:
//      red, green, blue: color value. Bits [5:0] (6 bits) are sent
void LCD_PushPixel(uint8_t red, uint8_t green, uint8_t blue);



/* Graphics primitives
 */

// LCD Draw pixel
// Sets area of 1 pixel and sends pixel data. Requires LCD_ActivateWrite
// to have been the last command
//  Param:
//      red, green, blue: color value. Bits [5:0] (6 bits) are sent
void LCD_gDrawPixel(uint8_t x, uint8_t y, uint8_t red, uint8_t green, uint8_t blue);

// Convert a 3 byte pixel (Eg #FF004A) uint32_t into LCD_pixel
// Precision loss: 8-bit -> 6-bit
//  Param:
//      p: 32-bit integer. Bits [23:0] are used
LCD_pixel LCD_Ui32ToPixel(uint32_t p);

// Clear screen to background color
void LCD_gClear(void);

// Vertical line (fast)
//  Param:
//      x: Column
//      y1: Start row
//      y2: End row
//      stroke: Line thickness. Line is centered on x if stroke is > 1
//      color: Line color
void LCD_gVLine(int16_t x, int16_t y1, int16_t y2, uint8_t stroke, LCD_pixel color);

// Horizontal line (fast)
//  Param:
//      x1: Start column
//      x2: End column
//      y: Row
//      stroke: Line thickness. Line is centered on y if stroke is > 1
//      color: Line color
void LCD_gHLine(int16_t x1, int16_t x2, int16_t y, uint8_t stroke, LCD_pixel color);

// Just a line in any direction. For straight vertical or horizontal lines
// LCD_gVLine and LCD_gHLine are used instead
//  Param:
//      x1: Start column
//      x2: End column
//      y1: Start row
//      y2: End row
//      stroke: Line thickness
//      color: Line color
void LCD_gLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t stroke, LCD_pixel color);

// Filled rectangle
//  Param:
//      x, y: column and row of first corner
//      w, h: width and height
//      color: LCD_pixel
void LCD_gFillRectangle(int16_t x, int16_t y, uint8_t w, uint8_t h, LCD_pixel color);

// Rectangle outline
//  Param:
//      x, y: column and row of first corner
//      w, h: width and height
//      stroke: edge width
//      color: LCD_pixel
void LCD_gRectangle(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t stroke, LCD_pixel color);

void LCD_gFillTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, LCD_pixel color);

void LCD_gTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, uint8_t stroke, LCD_pixel color);

void LCD_gFillCircle(int16_t x, int16_t y, uint8_t radius, LCD_pixel color);

void LCD_gCircle(int16_t x, int16_t y, uint8_t radius, uint8_t stroke, LCD_pixel color);

void LCD_gText(int16_t x, int16_t y, LCD_pixel color);

#endif // LCD_H
