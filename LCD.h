#ifndef LCD_H
#define LCD_H

#include <stdint.h>
#include "tiva-gc-inc.h"

#define HIGH 1
#define LOW  0
#define ON   1
#define OFF  0

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

typedef struct LCD_Settings
{
    uint8_t InversionMode;
    uint8_t ColorMode;
    uint8_t MemoryAccessCTL;
    LCD_pixel BGColor;
} LCD_Settings;



/* Initialization and settings
 */

// LCD initialization
//  Color mode: 18-bit/6-6-6 RGB
void LCD_Init(void);

// Get LCD settings
//  Return:
//      LCD_Settings: currently active settings
LCD_Settings LCD_GetSettings(void);

// Set LCD Background color
// The background color is used by some drawing primitives like LCD_gClear
//  Param:
//      LCD_pixel: new background color
void LCD_SetBGColor(LCD_pixel bgColor);

// Turn display inversion on or off
//  Param:
//      flag: 1 or 0, ON or OFF
void LCD_SetInversion(uint8_t flag);



/* Low level control and interfacing
 */

// LCD SPI Chip Select
// Tell LCD whether to process transmitted data
//  Param:
//      flag: HIGH = Deselect, LOW = Select
void LCD_CS(uint8_t flag);

// LCD set window position / area
// Define area to draw inside of. Useful when paired with LCD_ActivateWrite and LCD_PushPixel
//  Param:
//      colStart: starting column
//      rowStart: starting row
//      colEnd: ending column < LCD_WIDTH
//      rowEnd: ending row < LCD_HEIGHT
void LCD_SetArea(int16_t colStart, int16_t rowStart, int16_t colEnd, int16_t rowEnd);

// LCD activate memory write
// Sends RAM write command, after which any number of pixels can be sent.
// Useful when paired with LCD_SetArea and LCD_PushPixel
void LCD_ActivateWrite(void);

// LCD write pixel data
// Sends pixel data to current active window area. Requires LCD_ActivateWrite
// to have been the last command, and requires an area to have been defined
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

// Filled Triangle
//  Param:
//      v1: first vertex
//      v2: second vertex
//      v3: third vertex
//      color: LCD_pixel
void LCD_gFillTriangle(point v1, point v2, point v3, LCD_pixel color);

// Triangle outline
//  Param:
//      v1: first vertex
//      v2: second vertex
//      v3: third vertex
//      stroke: edge width
//      color: LCD_pixel
void LCD_gTriangle(point v1, point v2, point v3, uint8_t stroke, LCD_pixel color);

// Arbitrary polygon outline
// Edges are drawn in order from the first to the last and back to the first
//  Param:
//      vertices: array of vertices
//      n_vertices: size of vertices array
//      stroke: edge width
//      color: LCD_pixel
void LCD_gPolygon(point *vertices, int n_vertices, uint8_t stroke, LCD_pixel color);

void LCD_gFillCircle(int16_t x, int16_t y, uint8_t radius, LCD_pixel color);

void LCD_gCircle(int16_t x, int16_t y, uint8_t radius, uint8_t stroke, LCD_pixel color);

void LCD_gText(int16_t x, int16_t y, LCD_pixel color);

#endif // LCD_H
