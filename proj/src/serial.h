#ifndef SERIAL_H
#define SERIAL_H

/** @defgroup serial serial
 * @{
 *
 * Handles Serial Port and has some constants for programming the Serial Port
 */

#define SER_PORT 0x3F8
#define SER_IRQ 4

// UART Registers Offset
#define SER_RBR 0
#define SER_THR 0
#define SER_DLL 0
#define SER_IER 1
#define SER_DLM 1
#define SER_IIR 2
#define SER_FCR 2
#define SER_LCR 3
#define SER_LSR 5

// Line Control Register Bit Selection
#define LCR_5BITCHAR 0x00
#define LCR_8BITCHAR 0x11
#define LCR_1STOPBIT 0
#define LCR_2STOPBIT 1<<2
#define LCR_NOPARITY 0
#define LCR_ODDPARITY 1<<3
#define LCR_DLAB 1<<7

// Line Status Resgister Bit Selection
#define LSR_OVERRUN_ERR 1<<1
#define LSR_PARITY_ERR 1<<2
#define LSR_FRAME_ERR 1<<3
#define LSR_THR_EMPTY 1<<5

// Interrupt Enable Register Bits
#define IER_AV_DATA_INT 1
#define IER_THR_EMPTY_INT 1<<1
#define IER_ERR_INT 1<<2

// Interrupt Indentification Register Bits
#define IIR_INT_PEND 1
#define IIR_ID 0x1110
#define IIR_LSR_CHANGE_INT 6
#define IIR_AV_DATA_INT 4
#define IIR_THR_EMPTY_INT 2

// Interrupts
#define SER_ERR 0
#define SER_REQUEST 4
#define SER_CONFIRM 5
#define SER_THR_EMPTY 6
#define SER_CANCEL_REQUEST 7
#define SER_EXIT 8

/**
 * Subscribes Serial Port interrupts
 * @param bit_no position of the correspondent bit in interrupt "vector" of message
 * @return 0 on success, non-zero otherwise
*/
int (serial_subscribe_int)(uint8_t *bit_no);

/**
 * Unsubscribes Serial Port interrupts
 * @return 0 on success, non-zero otherwise
*/
int (serial_unsubscribe_int)();

/**
 * Configures the Serial Port to 19200 Baud Rate, 8 bits per packet, 1 stop bit, no parity, no fifos
 * @return 0 on success, non-zero otherwise
*/
int (serial_config)();

/**
 * Sends packet to the other end of the serial port connection
 * @param packet character we wish to send
 * @return 0 on success, non-zero otherwise
*/
int (serial_send)(uint8_t packet);

/**
 * Interrupt Handler of the Serial Port
 * @return SER_ERR, if error, SER_REQUEST if request to connect has been recieved, SER_CONFIRM if while waiting for the other pc he choose to play "online", SER_THR_EMPTY if THR is empty, SER_CANCEL_REQUEST if the other pc quit waiting and SER_EXIT if the other player quit the game
*/
int (serial_ih)();

#endif //SERIAL_H
