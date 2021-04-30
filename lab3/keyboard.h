#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <lcom/lcf.h>

int (kbc_subscribe_int)(uint8_t *bit_no);
int (timer_subscribe_int)(uint8_t *bit_no);
int (kbc_unsubscribe_int)();
int (timer_unsubscribe_int)();
int (util_sys_inb)(int port, uint8_t *value);
void (kbc_ih)();
void (timer_int_handler)();
int (kbc_write)(int port, uint8_t cmd);
void (kbc_read)();
int (kbc_reset)();

#endif /* KEYBOARD_H */
