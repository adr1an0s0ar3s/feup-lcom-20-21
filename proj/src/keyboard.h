#ifndef KEYBOARD_H
#define KEYBOARD_H

/** @defgroup keyboard keyboard
 * @{
 *
 * Handles Keyboard
 */

/**
 * Subscribes keyboard interrupts
 * @param bit_no position of the correspondent bit in interrupt "vector" of message
 * @return 0 on success, non-zero otherwise
*/
int (kbc_subscribe_int)(uint8_t *bit_no);

/**
 * Unsubscribes keyboard interrupts
 * @return 0 on success, non-zero otherwise
*/
int (kbc_unsubscribe_int)();

/**
 * Handles interrupts from keyboard
 * @return true on success, non-true otherwise
*/
int (keyboard_ih)();

/**
 * Writes to controller
 * @param port port to be written
 * @param cmd command to be written
 * @return 0 on success, non-zero otherwise
*/
int (kbc_write)(int port, uint8_t cmd);

#endif // KEYBOARD_H
