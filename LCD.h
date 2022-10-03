#ifndef LCD_H
#define LCD_H

#include <stdint.h>
#include "tiva-gc-inc.h"

// Effective size is smaller, since the screen has a 3 pixels deep zone on the borders that are not visible
#define LCD_HEIGHT 132
#define LCD_WIDTH  132

// Pixel: 18-bits
// Only bits [5:0] are used
typedef struct pixel
{
    uint8_t r, g, b;
} pixel;

#define LCD_BLACK       (pixel) { 0x00, 0x00, 0x00 }
#define LCD_DARK_GREY   (pixel) { 0x10, 0x10, 0x10 }
#define LCD_GREY        (pixel) { 0x20, 0x20, 0x20 }
#define LCD_LIGHT_GREY  (pixel) { 0x30, 0x30, 0x30 }
#define LCD_WHITE       (pixel) { 0x3F, 0x3F, 0x3F }

#define LCD_RED         (pixel) { 0x3F, 0x00, 0x00 }
#define LCD_GREEN       (pixel) { 0x00, 0x3F, 0x00 }
#define LCD_BLUE        (pixel) { 0x00, 0x00, 0x3F }
#define LCD_YELLOW      (pixel) { 0x3F, 0x3F, 0x00 }
#define LCD_MAGENTA     (pixel) { 0x3F, 0x00, 0x3F }
#define LCD_CYAN        (pixel) { 0x00, 0x3F, 0x3F }

#define LCD_DARK_RED    (pixel) { 0x1F, 0x00, 0x00 }
#define LCD_DARK_GREEN  (pixel) { 0x00, 0x1F, 0x00 }
#define LCD_DARK_BLUE   (pixel) { 0x00, 0x00, 0x1F }
#define LCD_DARK_YELLOW (pixel) { 0x1F, 0x1F, 0x00 }
#define LCD_PURPLE      (pixel) { 0x1F, 0x00, 0x1F }
#define LCD_TEAL        (pixel) { 0x00, 0x1F, 0x1F }

#define LCD_BROWN       (pixel) { 0x22, 0x11, 0x04 }
#define LCD_PINK        (pixel) { 0xFF, 0x05, 0x24 }
#define LCD_TURQUOISE   (pixel) { 0x10, 0x38, 0x34 }
#define LCD_ORANGE      (pixel) { 0x3f, 0x11, 0x00 }
#define LCD_GOLD        (pixel) { 0x3f, 0x29, 0x00 }

typedef struct LCD_Settings
{
    uint8_t InversionMode;
    uint8_t ColorMode;
    uint8_t MemoryAccessCTL;
    pixel BGColor;
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
//      pixel: new background color
void LCD_SetBGColor(pixel bgColor);

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

// Convert a 3 byte pixel (Eg #FF004A) uint32_t into pixel
// Precision loss: 8-bit -> 6-bit
//  Param:
//      p: 32-bit integer. Bits [23:0] are used
pixel LCD_Ui32ToPixel(uint32_t p);

// Clear screen to background color
void LCD_gClear(void);

// Vertical line (fast)
//  Param:
//      x: Column
//      y1: Start row
//      y2: End row
//      stroke: Line thickness. Line is centered on x if stroke is > 1
//      color: Line color
void LCD_gVLine(int16_t x, int16_t y1, int16_t y2, uint8_t stroke, pixel color);

// Horizontal line (fast)
//  Param:
//      x1: Start column
//      x2: End column
//      y: Row
//      stroke: Line thickness. Line is centered on y if stroke is > 1
//      color: Line color
void LCD_gHLine(int16_t x1, int16_t x2, int16_t y, uint8_t stroke, pixel color);

// Just a line in any direction. For straight vertical or horizontal lines
// LCD_gVLine and LCD_gHLine are used instead
//  Param:
//      x1: Start column
//      x2: End column
//      y1: Start row
//      y2: End row
//      stroke: Line thickness
//      color: Line color
void LCD_gLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t stroke, pixel color);

// Filled rectangle
//  Param:
//      x, y: column and row of first corner
//      w, h: width and height
//      color: pixel
void LCD_gFillRect(int16_t x, int16_t y, uint8_t w, uint8_t h, pixel color);

// Rectangle outline
//  Param:
//      x, y: column and row of first corner
//      w, h: width and height
//      stroke: edge width
//      color: pixel
void LCD_gRect(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t stroke, pixel color);

// Filled Triangle
//  Param:
//      v1: first vertex
//      v2: second vertex
//      v3: third vertex
//      color: pixel
void LCD_gFillTriangle(point v1, point v2, point v3, pixel color);

// Triangle outline
//  Param:
//      v1: first vertex
//      v2: second vertex
//      v3: third vertex
//      stroke: edge width
//      color: pixel
void LCD_gTriangle(point v1, point v2, point v3, uint8_t stroke, pixel color);

// Arbitrary polygon outline
// Edges are drawn in order from the first to the last and back to the first
//  Param:
//      vertices: array of vertices
//      n_vertices: size of vertices array
//      stroke: edge width
//      color: pixel
void LCD_gPolygon(point *vertices, int n_vertices, uint8_t stroke, pixel color);

// Filled cricle
//  Param:
//      x, y: circle center position
//      r: circle radius
//      color: pixel
void LCD_gFillCircle(int16_t x, int16_t y, float r, pixel color);

// Circle outline
//  Param:
//      x, y: circle center position
//      r: circle radius
//      stroke: outline width
//      color: pixel
void LCD_gCircle(int16_t x, int16_t y, float r, uint8_t stroke, pixel color);

// Draw character
// Draws a 5x7 character on the given position. If the background color is the same as the
// text color, the background is transparent (calls LCD_gCharT)
//  Param:
//      x, y: top left corner position
//      c: character to draw
//      textColor: character color
//      bgColor: background color
//      size: scale of the character
void LCD_gChar(int16_t x, int16_t y, char c, pixel textColor, pixel bgColor, uint8_t size);

// Draw character with transparent background
// Draws a 5x7 character on the given position.
//  Param:
//      x, y: top left corner position
//      c: character to draw
//      textColor: character color
//      size: scale of the character
void LCD_gCharT(int16_t x, int16_t y, char c, pixel textColor, uint8_t size);

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
uint32_t LCD_gString(int16_t x, int16_t y, char *str, uint8_t len, pixel textColor);

#endif // LCD_H
