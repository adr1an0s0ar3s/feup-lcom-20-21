#ifndef MOUSE_H
#define MOUSE_H

/** @defgroup mouse mouse
 * @{
 *
 * Handles Mouse
 */

/**
 * Handles the interrupts of the mouse and forms the mouse packets
 * @return 0 on success, non-zero otherwise
*/
int (mouse_interrupt_handler)();

/**
 * Subscribes Mouse interrupts
 * @param bit_no position of the correspondent bit in interrupt "vector" of message
 * @return 0 on success, non-zero otherwise
*/
int (mouse_subscribe_int)(uint8_t *bit_no);

/**
 * Unsubscribes keyboard interrupts
 * @return 0 on success, non-zero otherwise
*/
int (mouse_unsubscribe_int)();

/**
 * Enables data reporting
 * @return 0 on success, non-zero otherwise
*/
int (enable_data_reporting)();

/**
 * Disables data reporting
 * @return 0 on success, non-zero otherwise
*/
int (disable_data_reporting)();

#endif // MOUSE_H
