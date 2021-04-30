#ifndef UTIL_H
#define UTIL_H

/** @defgroup util util
 * @{
 *
 * Has some usefull functions
 */

/**
 * Gets LSB of a 2 byte integer
 * @param val  2 byte integer
 * @param lsb address of memory location to be updated 
 * @return 0 on success, non-zero otherwise
*/
int(util_get_LSB)(uint16_t val, uint8_t *lsb);

/**
 * Gets MSB of a 2 byte integer
 * @param val 2 byte integer
 * @param msb address of memory locaton to be updated
 * @return 0 on success, non-zero otherwise
*/
int(util_get_MSB)(uint16_t val, uint8_t *msb);

/**
 * Reads value into uint8_t variable
 * @param port to be read
 * @param value value to be updated with value read
 * @return 0 on success, non-zero otherwise
*/
int (util_sys_inb)(int port, uint8_t *value);

#endif // UTIL_H
