#include <lcom/lcf.h>
#include <lcom/liblm.h>
#include <lcom/proj.h>

#include <stdbool.h>
#include <stdint.h>

#include "game_xpm.h"
#include "game.h"
#include "graph.h"
#include <math.h>

// Background
static xpm_image_t back_info;
static uint8_t *background;

// Board
static xpm_image_t board_info;
static uint8_t *board_image;

// Board Column
static xpm_image_t board_column_info;
static uint8_t *board_column;

// Red Chip
static xpm_image_t red_info;
static uint8_t *red_chip;

// Yellow Chip
static xpm_image_t yellow_info;
static uint8_t *yellow_chip;

// Player 1 Indicator
static xpm_image_t player_indicator_info;
static uint8_t *player1_indicator;

// Player 2 Indicator
static uint8_t *player2_indicator;

// Player 1 Won Indicator
static xpm_image_t player_won_indicator_info;
static uint8_t *player1_won_indicator;

// Player 2 Won Indicator
static uint8_t *player2_won_indicator;


uint8_t player = 1;
static uint8_t board_x = 0;
static uint8_t board_y = 0;
static uint8_t randomPlay_board_x = 0;

#define columns 7
#define rows 6
static uint8_t board[columns][rows] = {{0}};


int (draw_background)() {return draw_xpm(background, back_info, 0, 0);}

int (draw_board)() {return draw_xpm(board_image, board_info, 110, 89);}

int (draw_chip)(uint16_t x, uint16_t y)
{
    if (x > 800 || y > 600)
        return 1;

    switch (player)
    {
    case 1:
        draw_xpm(red_chip, red_info, x, y);
        break;
    case 2:
        draw_xpm(yellow_chip, yellow_info, x, y);
        break;
    }

    return 0;
}

int (draw_column)() {return draw_xpm(board_column, board_column_info, 110+8+84*board_x, 89);}

int (draw_player_indicator)()
{
    if (player == 1) return draw_xpm(player1_indicator, player_indicator_info, 16, 464);
    else return draw_xpm(player2_indicator, player_indicator_info, 16, 464);
}

int (draw_player_won_indicator)()
{
    if (player == 1) return draw_xpm(player1_won_indicator, player_won_indicator_info, 161, 219);
    else return draw_xpm(player2_won_indicator, player_won_indicator_info, 161, 219);
}

int (initiate_game)()
{
    static bool loaded = false;

    if (!loaded)
    {
        // Loading XPMS
        background = xpm_load(background_xpm, XPM_8_8_8, &back_info);
        board_image = xpm_load(board_xpm, XPM_8_8_8, &board_info);
        board_column = xpm_load(board_column_xpm, XPM_8_8_8, &board_column_info);
        red_chip = xpm_load(red_chip_xpm, XPM_8_8_8, &red_info);
        yellow_chip = xpm_load(yellow_chip_xpm, XPM_8_8_8, &yellow_info);
        player1_indicator = xpm_load(player1_indicator_xpm, XPM_8_8_8, &player_indicator_info);
        player2_indicator = xpm_load(player2_indicator_xpm, XPM_8_8_8, &player_indicator_info);
        player1_won_indicator = xpm_load(player1_won_indicator_xpm, XPM_8_8_8, &player_won_indicator_info);
        player2_won_indicator = xpm_load(player2_won_indicator_xpm, XPM_8_8_8, &player_won_indicator_info);

        loaded = true;
    }

    // Erasing Contents of Board
    memset(board, 0, columns*rows);

    // Reseting board_x, board_y and player
    board_x = 0; board_y = 0; player = 1;

    // Initializing Board and Backgound in Buffer and Video Mem (synchronizing)
    draw_background();
    draw_board();
    draw_chip(110+8, 14);
    draw_player_indicator();
    display_frame();

    draw_background();
    draw_board();
    draw_chip(110+8, 14);
    draw_player_indicator();
    display_frame();

    return 0;
}

int (column_select_animation)(bool left)
{
    static int x = 0;
    static uint64_t time = 0;
    bool done = false;

    // Se já estiver no canto esq ou dir não faz a animação nem modifica nada
    if (time == 0 && ((left && board_x == 0) || (!left && board_x == 6)))
        return true;

    if (time == 0 && left) board_x--;
    else if (time == 0 && !left) board_x++;

    if (left) x = ceil(110+8+(board_x+1)*84 - 0.5*time*time);
    else x = 110+8+(board_x-1)*84 + 0.5*time*time;  

    if (!left && x > 110+8+84*board_x)
    {
        x = 110+8+84*board_x;
        done = true;
    }
    else if (left && x < 110+8+84*board_x)
    {
        x = 110+8+84*board_x;
        done = true;
    }

    if (time >= 2 && left)
        draw_part_of_xpm(background, back_info, 0, 0, ceil(110+8+(board_x+1)*84 - 0.5*(time-2)*(time-2)), 14, 63, 63);
    else if (time >= 2 && !left)
        draw_part_of_xpm(background, back_info, 0, 0, 110+8+(board_x-1)*84 + 0.5*(time-2)*(time-2), 14, 63, 63);
    
    draw_chip(x, 14);
    display_frame();
    
    //time++;

    if (done)
    {
        if (left) draw_part_of_xpm(background, back_info, 0, 0, ceil(110+8+(board_x+1)*84 - 0.5*(time-2)*(time-2)), 14, 63, 63);
        else draw_part_of_xpm(background, back_info, 0, 0, 110+8+(board_x-1)*84 + 0.5*(time-2)*(time-2), 14, 63, 63);
        
        draw_chip(x, 14);
        display_frame();

        // Reset Parameters
        time = 0;
        return true;
    }

    time++;
    return false;
}

int (random_play_column_select_animation)()
{
    static bool first_time = true;
    static bool left = true;

    if (first_time)
    {
        if (randomPlay_board_x < board_x)
            left = true;
        else left = false;
    }

    if ((first_time && randomPlay_board_x == board_x) || (column_select_animation(left) && randomPlay_board_x == board_x))
    {
        first_time = true;
        return true;
    }

    if (first_time) first_time = false;
    return false;
}

int (validPlay)() {return board[board_x][rows-1] == 0;}

int (makePlay)()
{
    for (int i = 0; i < rows; i++) {
        if (board[board_x][i] == 0) {
            board[board_x][i] = player;
            board_y = i;
            break;
        }
    }
    
    return true;
}

int (chip_falling_animation)()
{
    static int y;
    static uint64_t time = 0; // 1/60 seconds

    if (time == 0) makePlay();

    y = 14 + (0.1)*(time*time); // y(t) = y0 + v0*t + a/2*t^2

    if (y > 89+8+84*(5-board_y)+1)
        y = 89+8+84*(5-board_y)+1;

    if (time >= 2)
        draw_part_of_xpm(background, back_info, 0, 0, 110+8+84*board_x, 14+(0.1)*(time-2)*(time-2), 63, 63);
    
    draw_chip(110+8+84*board_x, y);
    draw_column();
    display_frame();

    if (y == 89+8+84*(5-board_y)+1)
    {
        // In the end synchronize Video Mem with Buffer
        draw_part_of_xpm(background, back_info, 0, 0, 110+8+84*board_x, 14+(0.1)*(time-2)*(time-2), 63, 63);
        draw_chip(110+8+84*board_x, y);
        draw_column();
        display_frame();

        // Reset initial parameters
        time = 0;
        
        printf("%d - %d - %d\n", board_x, board_y, player);

        return true;
    }

    time++;
    return false;
}

int (checkGameOver)()
{
    int count = 0;
    int new_x = 0;
    int new_y = 0;

    // Check if it has no column available to play a chip (draw)
    for (int n = 0; n < columns; n++)
        if (board[n][rows-1] != 0)
            count++;

    if (count == columns) return true;

    count = 0;
    
    // vertical ending - only need to check column that was played
    for (int n = 0; n < rows; n++){
        if (board[board_x][n] == player){
            count++;
            if (count == 4)
                return true;
        }
        else if (board[board_x][n] == 0)
            break;
        else
            count = 0;
    }
    
    count = 0;

    // horizontal ending
    for (int i = 0; i < columns; i++){
        if (board[i][board_y] == player){
            count++;
            if (count == 4)
                return true;
        }
        else
            count = 0;
    }

    count = 0;

    // Beta 1/3 Verification

    new_x = board_x; new_y = board_y;

    while (new_x != 0 && new_y != 0)
    {
        new_x--;
        new_y--;
    }

    while (new_x != 6+1 && new_y != 5+1)
    {
        if (board[new_x][new_y] == player){
            count++;
            if (count == 4)
                return true;
        }
        else
            count = 0;

        new_x++;
        new_y++;
    }
    
    count = 0;
    
    //Beta 2/4 Verification

    new_x = board_x; new_y = board_y;

    while (new_x != 0 && new_y != 5)
    {
        new_x--;
        new_y++;
    }

    while (new_x != 6+1 && new_y != 0-1)
    {
        if (board[new_x][new_y] == player){
            count++;
            //printf("x = %d - y = %d - count = %d\n", new_x, new_y, count);
            if (count == 4)
                return true;
        }
        else
            count = 0;

        new_x++;
        new_y--;
    }

    return false;
}

int (game_over)()
{
    static uint64_t time = 0;

    if (time == 0)
    {
        draw_player_won_indicator();
        display_frame();
    }

    time++;

    if (time == 60*5)
    {
        time = 0;
        return true;
    }

    return false;
}

void (nextPlay)()
{
    // Change Player
    player++;
    if (player > 2)
        player = 1;

    board_x = 0;

    draw_chip(110+8, 14);
    draw_player_indicator();
    display_frame();

    draw_chip(110+8, 14);
    draw_player_indicator();
    display_frame();
}

void (randomPlay)()
{
    while (1){
        randomPlay_board_x = rand() % 7;
        printf("%d\n", randomPlay_board_x);
        if (board[randomPlay_board_x][rows-1] == 0)
            break;
    }
}
