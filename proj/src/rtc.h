#ifndef RTC_H
#define RTC_H

/** @defgroup rtc rtc
 * @{
 *
 * Handles RTC and has some constants for programming the RTC
 */

#define BIT(n)   (1 << (n))

#define RTC_ADDR_REG 0x70
#define RTC_DATA_REG 0x71
#define RTC_IRQ 8

/**
 * Subscribes RTC interrupts
 * @param bit_no position of the correspondent bit in interrupt "vector" of message
 * @return 0 on succsess, non-zero otherwise
*/
int (rtc_subscribe_int)(uint8_t *bit_no);

/**
 * Unsubscribes RTC interrupts
 * @return 0 on sucsess, non-zero otherwise
*/
int (rtc_unsubscribe_int)();

/**
 * Activates alarm interrupts
*/ 
void (activate_alarm)();

/**
 * Deactivates alarm interrupts
*/
void (deactivate_alarm)();

/**
 * Handles alarm interrupts from the RTC
*/
void (handle_alarm_int)();

/**
 * Reads date and time of day
*/
void (readDate)();

/**
 * Sets an alarm 60 seconds from current time
*/
void (setAlarm)();

/**
 * Transforms BCD number into binary
 * @param n BCD number
 * @return Binary number
*/
uint8_t (bcd_to_bin)(uint8_t n);

/**
 *  Transforms binary number into BCD
 * @param n Binary number
 * @return BCD number
*/
uint8_t (bin_to_bcd)(uint8_t n);

#endif // RTC_H
