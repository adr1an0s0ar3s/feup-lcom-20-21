#ifndef GAME_H
#define GAME_H

/** @defgroup game game
 * @{
 *
 * Handles Game Mechanics and Animations
 */

/**
 * Initiates the game, loading and clearing the necessary things
 * @return 0
*/
int (initiate_game)();

/**
 * Draws background
 * @return 0
*/
int (draw_background)();

/**
 * Draws the gameboard
 * @return 0
*/
int (draw_board)();

/**
 * Draws chip in given positions
 * @param x position on the x axys
 * @param y position on the y axys
 * @return 0 on success, non-zero otherwise
*/
int (draw_chip)(uint16_t x, uint16_t y);

/**
 * Draws a column
 * @return 0
*/
int (draw_column)();

/**
 * Draws an indicator telling which player is playing
 * @return 0
*/
int (draw_player_indicator)();

/**
 * Draws an indicator telling which player won
 * @return 0
*/
int (draw_player_won_indicator)();

/**
 * Animates the column selection
 * @param left boolean indicating if it will move left or right
 * @return true on success, non-true otherwise
*/
int (column_select_animation)(bool left);

/**
 * Animates the column selection of a random play
 * @return true on success, non-true otherwise
*/
int (random_play_column_select_animation)();

/**
 * Animates the chip's fall
 * @return true on success, non-true otherwise
*/
int (chip_falling_animation)();

/**
 * Checks if play is valid
 * @return true if valid, non-true otherwise
*/
int (validPlay)();

/**
 * Makes play
 * @return true
*/
int (makePlay)();

/** Checks if game is over
 * @return true if game is over, non-true otherwise
*/
int (checkGameOver)();

/**
 * Displays the winning player indicator
 * @return true if success, non-true otherwise
*/
int (game_over)();

/**
 * Changes player and makes the necessary adjustments
*/
void (nextPlay)();

/**
 * Makes a random play
*/
void (randomPlay)();

#endif //GAME_H
