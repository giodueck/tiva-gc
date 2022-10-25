#include <stdint.h>
#include "TM4C123GH6PM.h"
#include "tiva-gc.h"

// Indicates a reset is needed for the currently selected update function or the menu
static uint8_t fReset = 0;
static LCD_Settings settings;
// Can be used to perform some update at a regular interval using GE_STGet or GE_STPop
static uint32_t UPS = 5;

void menu(void);
int snake(void);
//int resetter(void);
//int random(void);
int pong(void);

void menu()
{
    // Persistent and read-only vars
    static uint8_t drawn = 0;
    const uint8_t nOptions = 2;
    const char *options[] = {
        "Snake    ",
        "Pong     "
        //"Resetter ",
        //"Random   "
    };
    int (*games[])(void) = {
        snake,
        pong
        //resetter,
        //random
    };
    static int choice = 0, chosen = -1, held = 0;
    static GE_Joystick JS_old = {0};

    // Per loop vars
    uint8_t changed = 0;

    // Controls
    static GE_Button *menuSelect = &SW1;

    // Reset to the main menu
    if (fReset)
    {
        drawn = 0;
        choice = 0;
        chosen = -1;
        held = 0;
        fReset = 0;
        LCD_gClear();
        return;
    }

    // One time drawing
    if (!drawn)
    {
        drawn = 1;
        LCD_gClear();
        LCD_gFillRect(0, 0, LCD_WIDTH - 1, 9, LCD_DARK_GREY);
        LCD_SetBGColor(LCD_RED);
        LCD_gString(0, 0, "Menu V1", 0, LCD_RED);

        LCD_gFillRect(0, LCD_HEIGHT - 8, LCD_WIDTH - 1, 8, LCD_DARK_GREY);
        // LCD_gString(14, 15, "Menu V1", 0, LCD_RED);
        LCD_SetBGColor(LCD_BLACK);
        settings = LCD_GetSettings();
        changed = 1;
    }

    // Check inputs
    if (JS.up && !JS_old.up)
    {
        choice--;
        if (choice < 0)
            choice = nOptions - 1;
        changed = 1;
    }
    if (JS.down && !JS_old.down)
    {
        choice++;
        choice %= nOptions;
        changed = 1;
    }
    JS_old = JS;

    if (menuSelect->pressed)
    {
        chosen = choice;
        changed = 1;
    }
    if (menuSelect->held != held)
    {
        held = menuSelect->held;
        changed = 1;
    }

    // Draw
    if (changed)
    {
        // Draw menu elements
        for (int i = 0; i < nOptions; i++)
        {
            if (choice == i)
            {
                if (held)
                    LCD_SetBGColor(LCD_WHITE);
                else
                    LCD_SetBGColor(LCD_LIGHT_GREY);
                LCD_gString(1, 2 + i, options[i], 9, LCD_BLACK);
                LCD_SetBGColor(LCD_BLACK);
            }
            else
            {
                LCD_gString(1, 2 + i, options[i], 9, LCD_LIGHT_GREY);
            }
        }
    }

    // If a choice was made, start the game
    if (chosen > -1)
    {
        GE_SetUpdate(games[chosen]);
        LCD_gClear();
        fReset = 1;
    }
}

/*
int resetter()
{
    return 0;
}

int random()
{
    uint32_t n;
    if (SEL.pressed)
    {
        fReset = 1;
        return 0;
    } else
    {
        fReset = 0;
        LCD_SetArea(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
        LCD_ActivateWrite();
        for (int i = 0; i < LCD_HEIGHT; i++)
            for (int j = 0; j < LCD_WIDTH; j++)
            {
                n = GE_Rand();
                LCD_PushPixel(n & 0x3F, (n >> 6) & 0x3F, (n >> 12) & 0x3F);
            }
    }
    return 1;
}
*/

enum snake_direction { UP = 0, RIGHT = 1, DOWN = 2, LEFT = 3 };

void snake_spawn_food(int8_t cells[][2], uint16_t tail_idx);
int snake_config(void);

int snake_config(void)
{
    static int option = 1;
    static GE_Joystick JS_old = {0};
    char *options[4] = {
        "Slow  ",
        "Medium",
        "Fast  ",
        "Sanic "
    };
    uint32_t speeds[4] = { 5, 7, 10, 15};
    
    if (fReset == 1)
    {
        LCD_gString(1, 2, "Snake speed:", 0, LCD_WHITE);
        LCD_gString(14, 2, options[0], 0, LCD_WHITE);
        LCD_SetBGColor(LCD_LIGHT_GREY);
        LCD_gString(14, 3, options[1], 0, LCD_BLACK);
        LCD_SetBGColor(settings.BGColor);
        LCD_gString(14, 4, options[2], 0, LCD_WHITE);
        LCD_gString(14, 5, options[3], 0, LCD_WHITE);
        
        option = 1;
        fReset = 2;
    }

    if (JS.down && !JS_old.down)
    {
        LCD_gString(14, 2 + option, options[option], 0, LCD_WHITE);
        
        option = (option + 1) & 0x03;
        
        LCD_SetBGColor(LCD_LIGHT_GREY);
        LCD_gString(14, 2 + option, options[option], 0, LCD_BLACK);
        LCD_SetBGColor(settings.BGColor);
    } else if (JS.up && !JS_old.up)
    {
        LCD_gString(14, 2 + option, options[option], 0, LCD_WHITE);
        
        option = (option + 3) & 0x03;
        
        LCD_SetBGColor(LCD_LIGHT_GREY);
        LCD_gString(14, 2 + option, options[option], 0, LCD_BLACK);
        LCD_SetBGColor(settings.BGColor);
    }
    JS_old = JS;

    if (SW1.pressed)
    {
        UPS = speeds[option];
        return 1;
    }
    
    return 0;
}

void snake_spawn_food(int8_t cells[][2], uint16_t tail_idx)
{
    int8_t x = -1, y = -1, i, c = 31;
    // Se elige primero una x con espacio libre, luego una y libre en esa linea

    while (c == 31)
    {
        x = GE_Rand() % 31;
        for (i = 0, c = 0; i <= tail_idx; i++)
        {
            if (cells[i][0] == x)
                c++;
        }
    }
    while (y == -1)
    {
        y = GE_Rand() % 31;
        for (i = 0; i <= tail_idx; i++)
        {
            if (cells[i][0] == x && cells[i][1] == y)
            {
                y = -1;
                break;
            }
        }
    }
    
    cells[tail_idx + 1][0] = x;
    cells[tail_idx + 1][1] = y;
}

int snake()
{
    static uint32_t time = 0;
    static uint8_t game_over = 0;
    uint8_t food_hit = 0;

    // 31x31 grid of 4x4 pixel cells, 31*31 = 961
    // Each cell holds 2 values: x, y
    // Cells are stored in order: 0 is head, and the tail is indicated by the tail_idx variable.
    // The cell immediately after the tail is the food pellet.
    // The game is won if the 961st cell is the tail cell, i.e. the cell takes up all of the grid.
    static int8_t cells[961][2] = {0};
    static uint16_t tail_idx = 1;
    int8_t old_tailx, old_taily, new_headx, new_heady;
    static enum snake_direction facing, input, old_facing;

    // First time drawing and setup
    if (fReset)
    {
        if (!snake_config())
            return 1;

        // Draw borders
        // Playing field area: 124x124 pixels, 31x31 4x4 pixel squares
        LCD_gClear();
        LCD_gHLine(0, LCD_WIDTH - 1, 0, 1, LCD_WHITE);
        LCD_gHLine(0, LCD_WIDTH - 1, LCD_HEIGHT - 1, 1, LCD_WHITE);
        LCD_gVLine(0, 0, LCD_HEIGHT - 1, 1, LCD_WHITE);
        LCD_gVLine(LCD_WIDTH - 1, 0, LCD_HEIGHT - 1, 1, LCD_WHITE);

        // Starter snake, 2 cells long
        cells[0][0] = 16;
        cells[0][1] = 16;
        cells[1][0] = 16;
        cells[1][1] = 17;
        tail_idx = 1;
        old_tailx = 16;
        old_taily = 17;
        facing = UP;
        input = UP;
        old_facing = UP;
        time = CLOCKS_PER_SEC;
        GE_STPop();

        // Started food pellet
        cells[2][0] = 16;
        cells[2][1] = 10;

        // Initial drawing
            // snake
        LCD_gFillRect(2 + (cells[0][0] << 2), 2 + (cells[0][1] << 2), 4, 4, LCD_DARK_GREEN);
        LCD_gFillRect(3 + (cells[0][0] << 2), 2 + (cells[0][1] << 2), 2, 4, LCD_GREEN);
        LCD_gFillRect(2 + (cells[1][0] << 2), 2 + (cells[1][1] << 2), 4, 4, LCD_DARK_GREEN);
        LCD_gFillRect(3 + (cells[1][0] << 2), 2 + (cells[1][1] << 2), 2, 4, LCD_GREEN);

            // food
        LCD_gFillRect(2 + (cells[2][0] << 2), 3 + (cells[2][1] << 2), 4, 2, LCD_RED);
        LCD_gFillRect(3 + (cells[2][0] << 2), 5 + (cells[2][1] << 2), 2, 1, LCD_RED);
        LCD_gFillRect(3 + (cells[2][0] << 2), 2 + (cells[2][1] << 2), 2, 1, LCD_GREEN);
        
        fReset = 0;
        game_over = 0;
        
        return 1;
    }

    /* Input */
    if (game_over)
    {
        if (SEL.pressed || SW1.pressed || SW2.pressed)
        {
            fReset = 1;
            return 0;
        }
        return 1;
    } else
    {
        if (JS.down && facing != UP)
            input = DOWN;
        else if (JS.up && facing != DOWN)
            input = UP;
        else if (JS.right && facing != LEFT)
            input = RIGHT;
        else if (JS.left && facing != RIGHT)
            input = LEFT;
    }

    /* Logic */

    // The game is very fast, a delay before the logic and drawing code but after checking inputs allows for 
    // better reaction to inputs
    time += GE_STPop();
    if (time >= (CLOCKS_PER_SEC / UPS)) // delay
        time = 0;
    else

    return 1;
    
    old_facing = facing;    // for drawing purposes
    facing = input;         // accept last input

    // Move
        // head
    switch (facing)
    {
    case UP:
        new_headx = cells[0][0];
        new_heady = cells[0][1] - 1;
        break;
    case DOWN:
        new_headx = cells[0][0];
        new_heady = cells[0][1] + 1;
        break;
    case RIGHT:
        new_headx = cells[0][0] + 1;
        new_heady = cells[0][1];
        break;
    case LEFT:
        new_headx = cells[0][0] - 1;
        new_heady = cells[0][1];
        break;
    }
        // body
    old_tailx = cells[tail_idx][0];
    old_taily = cells[tail_idx][1];
    for (int i = tail_idx; i > 0; i--)
    {
        // check if head collides with body
        if (new_headx == cells[i - 1][0] && new_heady == cells[i - 1][1])
        {
            // game over
            LCD_SetBGColor(LCD_LIGHT_GREY);
            LCD_gHLine(42, 96, 48, 1, LCD_LIGHT_GREY);
            LCD_gString(7, 6, "GAME OVER", 0, LCD_RED);
            LCD_SetBGColor(settings.BGColor);
            game_over = 1;
            return 1;
        }
        cells[i][0] = cells[i - 1][0];
        cells[i][1] = cells[i - 1][1];
    }
    cells[0][0] = new_headx;
    cells[0][1] = new_heady;

        // check if head collides with a wall
    if (cells[0][0] < 0 || cells[0][1] < 0 || cells[0][0] >= 31 || cells[0][1] >= 31)
    {
        // game over
        LCD_SetBGColor(LCD_LIGHT_GREY);
        LCD_gHLine(42, 96, 48, 1, LCD_LIGHT_GREY);
        LCD_gString(7, 6, "GAME OVER", 0, LCD_RED);
        LCD_SetBGColor(settings.BGColor);
        game_over = 1;
        return 1;
    }
        // check if head collides with food
    if (cells[0][0] == cells[tail_idx + 1][0] && cells[0][1] == cells[tail_idx + 1][1])
    {
        food_hit = 1;
        tail_idx++;
        if (tail_idx == 961)    // victory
        {
            LCD_SetBGColor(LCD_LIGHT_GREY);
            LCD_gHLine(42, 96, 48, 1, LCD_LIGHT_GREY);
            LCD_gString(8, 6, "VICTORY", 0, LCD_BLUE);
            LCD_SetBGColor(settings.BGColor);
            game_over = 1;
            return 1;
        }

        cells[tail_idx][0] = old_tailx;
        cells[tail_idx][1] = old_taily;
        snake_spawn_food(cells, tail_idx);
    }

    /* Drawing */

    // Draw food
    if (food_hit)
    {
        LCD_gFillRect(2 + (cells[tail_idx + 1][0] << 2), 3 + (cells[tail_idx + 1][1] << 2), 4, 2, LCD_RED);
        LCD_gFillRect(3 + (cells[tail_idx + 1][0] << 2), 5 + (cells[tail_idx + 1][1] << 2), 2, 1, LCD_RED);
        LCD_gFillRect(3 + (cells[tail_idx + 1][0] << 2), 2 + (cells[tail_idx + 1][1] << 2), 2, 1, LCD_GREEN);
    }

    // Erase old tail if food was not eaten
    else
        LCD_gFillRect(2 + (old_tailx << 2), 2 + (old_taily << 2), 4, 4, settings.BGColor);

    // Draw new head segment
    LCD_gFillRect(2 + (cells[0][0] << 2), 2 + (cells[0][1] << 2), 4, 4, LCD_DARK_GREEN);

    // new head segment will just be straight
    if (facing == UP || facing == DOWN)
    {
        LCD_gFillRect(3 + (cells[0][0] << 2), 2 + (cells[0][1] << 2), 2, 4, LCD_GREEN);
    } else
    {
        LCD_gFillRect(2 + (cells[0][0] << 2), 3 + (cells[0][1] << 2), 4, 2, LCD_GREEN);
    }

    // neck has different coloring if the snake is turning
    if (old_facing != facing)
    {
        LCD_gFillRect(2 + (cells[1][0] << 2), 2 + (cells[1][1] << 2), 4, 4, LCD_DARK_GREEN);

        if (old_facing == UP || facing == DOWN)
            LCD_gFillRect(3 + (cells[1][0] << 2), 3 + (cells[1][1] << 2), 2, 3, LCD_GREEN);
        else if (old_facing == DOWN || facing == UP)
            LCD_gFillRect(3 + (cells[1][0] << 2), 2 + (cells[1][1] << 2), 2, 3, LCD_GREEN);
        
        if (old_facing == RIGHT || facing == LEFT)
            LCD_gFillRect(2 + (cells[1][0] << 2), 3 + (cells[1][1] << 2), 1, 2, LCD_GREEN);
        else if (old_facing == LEFT || facing == RIGHT)
            LCD_gFillRect(5 + (cells[1][0] << 2), 3 + (cells[1][1] << 2), 1, 2, LCD_GREEN);
    }
    
    return 1;
}

int pong(void)
{
    static uint16_t p1Score, p2Score, scored;
    static uint16_t p1Pos, p2Pos, movSpeed;
    static uint16_t paddleThickness, paddleWidth;
    static point ballSize;
    static point ballPos, ballSpeed;
    static uint32_t time;
    const int32_t topBorder = 9, bottomBorder = LCD_HEIGHT - 1, paddleX = 2, scoredTimeout = UPS;
    static char scoreBoard[2][4];
    
    if (fReset)
    {
        UPS = 25;
        p1Score = 0;
        p2Score = 0;
        scored = 0;
        p1Pos = p2Pos = (LCD_HEIGHT + 8) / 2;
        movSpeed = 5;
        paddleThickness = 3;
        paddleWidth = 16;
        ballPos = (point) {.x = LCD_WIDTH / 2, .y = (LCD_HEIGHT + 8) / 2};
        ballSpeed = (point) {.x = -4, .y = 0};
        ballSize = (point) {.x = 4, .y = 4};
        
        time = 0;
        GE_STPop();
        fReset = 0;
        
        // Borders
        LCD_gFillRect(0, 0, LCD_WIDTH, topBorder, LCD_WHITE);
        LCD_gHLine(0, LCD_WIDTH, bottomBorder, 1, LCD_WHITE);
        
        scoreBoard[0][2] = '0';
        scoreBoard[0][1] = ' ';
        scoreBoard[0][0] = ' ';
        scoreBoard[0][3] = '\0';
        scoreBoard[1][2] = '0';
        scoreBoard[1][1] = ' ';
        scoreBoard[1][0] = ' ';
        scoreBoard[1][3] = '\0';
        LCD_SetBGColor(LCD_WHITE);
        LCD_gString(1, 0, scoreBoard[0], 0, LCD_BLACK);
        LCD_gString(17, 0, scoreBoard[1], 0, LCD_BLACK);
        LCD_SetBGColor(settings.BGColor);
        
        // Paddles
        LCD_gFillRect(paddleX, p1Pos - (paddleWidth >> 1), paddleThickness, paddleWidth, LCD_LIGHT_GREY);
        LCD_gFillRect(LCD_WIDTH - paddleX - paddleThickness, p2Pos - (paddleWidth >> 1), paddleThickness, paddleWidth, LCD_LIGHT_GREY);
        
        // Ball
        LCD_gFillRect(ballPos.x - (ballSize.x >> 1), ballPos.y - (ballSize.y >> 1), ballSize.x, ballSize.y, LCD_LIGHT_GREY);
        
        return 1;
    }
    
    time += GE_STPop();
    if (time < (CLOCKS_PER_SEC / UPS))
        return 1;
    time = 0;

    // Scored and reset precedure
    if (scored)
    {
        if (scored == scoredTimeout)
        {
            LCD_gFillRect(ballPos.x - (ballSize.x >> 1), ballPos.y - (ballSize.y >> 1), ballSize.x, ballSize.y, settings.BGColor);

            ballPos = (point) {.x = LCD_WIDTH / 2, .y = (LCD_HEIGHT + 8) / 2};

            ballSpeed.y = GE_Rand() % (5) - 2;

            // Scoreboard update
            scoreBoard[0][2] = '0' + p1Score % 10;
            scoreBoard[0][1] = (p1Score >= 10) ? '0' + (p1Score % 100 - p1Score % 10) / 10 : ' ';
            scoreBoard[0][0] = (p1Score >= 100) ? '0' + (p1Score % 1000 - p1Score % 100) / 100 : ' ';
            scoreBoard[0][3] = '\0';
            scoreBoard[1][2] = '0' + p2Score % 10;
            scoreBoard[1][1] = (p2Score >= 10) ? '0' + (p2Score % 100 - p2Score % 10) / 10 : ' ';
            scoreBoard[1][0] = (p2Score >= 100) ? '0' + (p2Score % 1000 - p2Score % 100) / 100 : ' ';
            scoreBoard[1][3] = '\0';
            LCD_SetBGColor(LCD_WHITE);
            LCD_gString(1, 0, scoreBoard[0], 0, LCD_BLACK);
            LCD_gString(17, 0, scoreBoard[1], 0, LCD_BLACK);
            LCD_SetBGColor(settings.BGColor);
        }
        scored--;
    }
    
    // Erase if moved
        // Paddles
    if (JS.down || JS.up) LCD_gFillRect(paddleX, p1Pos - (paddleWidth >> 1), paddleThickness, paddleWidth, settings.BGColor);
    if (SW1.held ^ SW2.held) LCD_gFillRect(LCD_WIDTH - paddleX - paddleThickness, p2Pos - (paddleWidth >> 1), paddleThickness, paddleWidth, settings.BGColor);
    
        // Ball
    if (!scored) LCD_gFillRect(ballPos.x - (ballSize.x >> 1), ballPos.y - (ballSize.y >> 1), ballSize.x, ballSize.y, settings.BGColor);
    if (ballPos.y > LCD_HEIGHT - ballSize.y) LCD_gHLine(0, LCD_WIDTH, bottomBorder, 1, LCD_WHITE);
    if (ballPos.y <= topBorder + ballSize.y)
    {
        LCD_SetBGColor(LCD_WHITE);
        LCD_gString(1, 0, scoreBoard, 0, LCD_BLACK);
        LCD_SetBGColor(settings.BGColor);
    }
    
    // Move
        // Paddle player 1
    if (JS.down)
        p1Pos = min(p1Pos + movSpeed, bottomBorder - (paddleWidth >> 1));
    else if (JS.up)
        p1Pos = max(p1Pos - movSpeed, topBorder + (paddleWidth >> 1));
    
        // Paddle player 2
    if (SW1.held && !SW2.held)
        p2Pos = max(p2Pos - movSpeed, topBorder + (paddleWidth >> 1));
    else if (!SW1.held && SW2.held)
        p2Pos = min(p2Pos + movSpeed, bottomBorder - (paddleWidth >> 1));
    
        // Ball
    // Collisions
        // Goals
    if (!scored)
    {
        if (ballPos.x + ballSpeed.x - (ballSize.x >> 1) <= 0)
        {
            scored = scoredTimeout;
            p2Score++;
            ballSpeed.x = -ballSpeed.x;
        }
        else if (ballPos.x + ballSpeed.x + (ballSize.x >> 1) > LCD_WIDTH)
        {
            scored = scoredTimeout;
            p1Score++;
            ballSpeed.x = -ballSpeed.x;
        }
            // Top and bottom borders
        if (ballPos.y + ballSpeed.y - (ballSize.y >> 1) <= topBorder ||
            ballPos.y + ballSpeed.y + (ballSize.y >> 1) > bottomBorder)
            ballSpeed.y = -ballSpeed.y;
        
            // P1 paddle
        if (ballPos.x + ballSpeed.x - (ballSize.x >> 1) <= paddleX + paddleThickness &&
            (ballPos.y + ballSpeed.y - (ballSize.y >> 1) <= p1Pos + (paddleWidth >> 1) &&
            ballPos.y + ballSpeed.y + (ballSize.y >> 1) >= p1Pos - (paddleWidth >> 1)))
        {
            ballSpeed.x = -ballSpeed.x;
            ballSpeed.y = (ballPos.y - p1Pos) >> 1;
        }
            // P2 paddle
        if (ballPos.x + ballSpeed.x + (ballSize.x >> 1) > LCD_WIDTH - paddleX - paddleThickness &&
            (ballPos.y + ballSpeed.y - (ballSize.y >> 1) <= p2Pos + (paddleWidth >> 1) &&
            ballPos.y + ballSpeed.y + (ballSize.y >> 1) >= p2Pos - (paddleWidth >> 1)))
        {
            ballSpeed.x = -ballSpeed.x;
            ballSpeed.y = (ballPos.y - p2Pos) >> 1;
        }

        ballPos.x += ballSpeed.x;
        ballPos.y += ballSpeed.y;
    }
    
    // Draw  if moved
        // Paddles
    if (JS.down || JS.up) LCD_gFillRect(paddleX, p1Pos - (paddleWidth >> 1), paddleThickness, paddleWidth, LCD_LIGHT_GREY);
    if (SW1.held ^ SW2.held) LCD_gFillRect(LCD_WIDTH - paddleX - paddleThickness, p2Pos - (paddleWidth >> 1), paddleThickness, paddleWidth, LCD_LIGHT_GREY);
    
        // Ball
    if (scored != scoredTimeout) LCD_gFillRect(ballPos.x - (ballSize.x >> 1), ballPos.y - (ballSize.y >> 1), ballSize.x, ballSize.y, LCD_LIGHT_GREY);
    
    return 1;
}

int main()
{
    GE_Setup();

    GE_SetMainMenu(menu);
    
    GE_Loop();
}
