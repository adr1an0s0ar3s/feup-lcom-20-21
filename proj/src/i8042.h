#ifndef I8042_H
#define I8042_H

/** @defgroup i8042 i8042
 * @{
 *
 * Constants for programming the i8042 KBC
 */

#define DELAY_US    20000
#define BIT(n)   (1 << (n))

// KBC
#define KBC_IRQ 1
#define TIMER_IRQ 0
#define KBC_STATUS 0x64
#define KBC_IN 0x64
#define KBC_OUT_BUF 0x60
#define KBC_OBF BIT(0)
#define KBC_ST_IBF BIT(1)
#define KBC_PAR_ERR BIT(7)
#define KBC_TO_ERR BIT(6)
#define KBC_MSBIT BIT(7)
#define KBC_AUX BIT(5)

// Mouse
#define MOUSE_IRQ 12
#define MOUSE_EN_DATA 0xF4
#define MOUSE_DI_DATA 0xF5
#define MOUSE_WRITE_COM 0xD4
#define MOUSE_ACK 0xFA


#endif // I8042_H
