#ifndef _LCOM_I8042_H_
#define _LCOM_I8042_H_

#include <lcom/lcf.h>
#include <minix/sysutil.h>

#define DELAY_US    20000
#define BIT(n)   (1 << (n))
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

#endif /* _LCOM_I8042_H */
