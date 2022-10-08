#include <stdint.h>
#include <stdlib.h> // rand
#include "TM4C123GH6PM.h"
#include "tiva-gc.h"

// Indicates a reset is needed for the currently selected update function or the menu
static uint8_t fReset = 0;
static LCD_Settings settings;

void menu(void);
int snake(float elapsedTime);
int resetter(float elapsedTime);
int random(float elapsedTime);

void menu()
{
    // Persistent and read-only vars
    static uint8_t drawn = 0;
    const uint8_t nOptions = 3;
    const char *options[] = {
        "Snake    ",
        "Resetter ",
        "Random   "
    };
    const int (*games[])(float elapsedTime) = {
        snake,
        resetter,
        random
    };
    static int choice = 0, chosen = -1, held = 0;
    static GE_Joystick JS_old = {0};

    // Per loop vars
    uint8_t changed = 0;

    // Controls
    static GE_Button *menuSelect = &SEL;

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
        fReset = 1;
    }
}

int resetter(float elapsedTime)
{
    return 0;
}

int random(float elapsedTime)
{
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
                LCD_PushPixel(rand() % 0x3F, rand() % 0x3F, rand() % 0x3F);
    }
    return 1;
}

enum snake_direction { UP = 0, RIGHT = 1, DOWN = 2, LEFT = 3 };

void snake_spawn_food(int8_t cells[][2], uint16_t tail_idx)
{
    int8_t x = -1, y = -1, i, c = 31;
    // Se elige primero una x con espacio libre, luego una y libre en esa linea

    while (c == 31)
    {
        x = rand() % 31;
        for (i = 0, c = 0; i <= tail_idx; i++)
        {
            if (cells[i][0] == x)
                c++;
        }
    }
    while (y == -1)
    {
        y = rand() % 31;
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

int snake(float elapsedTime)
{
    static float time = 0;
    // const float fps = 5;
    static GE_Joystick old_JS;
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
        fReset = 0;
        game_over = 0;
        old_JS = JS;

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
        old_facing = UP;

        // Started food pellet
        cells[2][0] = 16;
        cells[2][1] = 10;

        // Initial drawing
            // snake
        LCD_gFillRect(2 + (cells[0][0] << 2), 2 + (cells[0][1] << 2), 4, 8, LCD_DARK_GREEN);
        LCD_gFillRect(3 + (cells[0][0] << 2), 2 + (cells[0][1] << 2), 2, 8, LCD_GREEN);
            // food
        LCD_gFillRect(2 + (cells[2][0] << 2), 3 + (cells[2][1] << 2), 4, 2, LCD_RED);
        LCD_gFillRect(3 + (cells[2][0] << 2), 5 + (cells[2][1] << 2), 2, 1, LCD_RED);
        LCD_gFillRect(3 + (cells[2][0] << 2), 2 + (cells[2][1] << 2), 2, 1, LCD_GREEN);

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

    // The game is very fast, a looping delay before the logic and drawing code allows for capturing inputs
    // in between loops
    time++;
    if (time == 1250) // delay
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

int main()
{
    GE_Setup();

    GE_SetMainMenu(menu);
    
    GE_Loop();
}
