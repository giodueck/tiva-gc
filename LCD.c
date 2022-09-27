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

void InitSPI(void);
void WriteSPI(uint8_t data);
void LCD_Command(uint8_t command);
void LCD_Data(uint8_t data);
void LCD_DataBuffer(uint8_t *buffer, uint32_t count);

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
LCD_Settings LCD_GetSettings()
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

void LCD_gVLine(int16_t x, int16_t y1, int16_t y2, uint8_t stroke, LCD_pixel color)
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

    // Rectangular area, y2 - y1 pixels high, stroke pixels wide
    LCD_SetArea(max(0, x - ((stroke - 1) >> 1)), y1, min(LCD_WIDTH - 1, x + (stroke >> 1)), y2);
    LCD_ActivateWrite();

    for (int i = 0; i < (y2 - y1) * stroke * x; i++)
        LCD_PushPixel(color.r, color.g, color.b);
}

void LCD_gHLine(int16_t x1, int16_t x2, int16_t y, uint8_t stroke, LCD_pixel color)
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

    // Rectangular area, y2 - y1 pixels high, stroke pixels wide
    LCD_SetArea(x1, max(0, y - ((stroke - 1) >> 1)), x2, min(LCD_HEIGHT - 1, y + (stroke >> 1)));
    LCD_ActivateWrite();

    for (int i = 0; i < (x2 - x1) * stroke * y; i++)
        LCD_PushPixel(color.r, color.g, color.b);
}

void LCD_gLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t stroke, LCD_pixel color)
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
//      color: LCD_pixel
void LCD_gFillRectangle(int16_t x, int16_t y, uint8_t w, uint8_t h, LCD_pixel color)
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
void LCD_gRectangle(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t stroke, LCD_pixel color)
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
//      color: LCD_pixel
void LCD_gTriangle(point v1, point v2, point v3, uint8_t stroke, LCD_pixel color)
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
//      color: LCD_pixel
void LCD_gFillTriangle(point v1, point v2, point v3, LCD_pixel color)
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
//      color: LCD_pixel
void LCD_gPolygon(point *vertices, int n_vertices, uint8_t stroke, LCD_pixel color)
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
//      color: LCD_pixel
void LCD_gCircle(int16_t x, int16_t y, float r, uint8_t stroke, LCD_pixel color)
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
//      color: LCD_pixel
void LCD_gFillCircle(int16_t x, int16_t y, float r, LCD_pixel color)
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
