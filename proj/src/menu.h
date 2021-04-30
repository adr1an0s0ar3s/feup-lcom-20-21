#ifndef MENU_H
#define MENU_H

/** @defgroup menu menu
 * @{
 *
 * Handles the Menu Display and Mouse Updates
 */

/**
 * Loads XPMs and displays the menu background
 * @return 0 on success, non-zero otherwise
*/
int (startMenu)();

/**
 * Updates the mouse location and updates the button highlight
 * @param d_x mouse x displacement
 * @param d_y mouse y displacement
 * @param click tells if left-button of mouse has been pressed or not
 * @return 0 on success, non-zero otherwise
*/
int (showMenu)(int16_t d_x, int16_t d_y, bool click);

/**
 * Displays the Online Waiting Menu
 * @return 0 on success, non-zero otherwise
*/
int (showOnlineWaitingMenu)();

/**
 * Displays the Instructions Menu
 * @return 0 on success, non-zero otherwise
*/
int (showInstructionsMenu)();

#endif //MENU_H
