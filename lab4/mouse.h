#ifndef MOUSE_H
#define MOUSE_H

void (mouse_ih)();
void (timer_int_handler)();
int (mouse_subscribe_int)(uint8_t *bit_no);
int (mouse_unsubscribe_int)();
int (timer_subscribe_int)(uint8_t *bit_no);
int (timer_unsubscribe_int)();
int (new_mouse_enable_data_reporting)();
int (mouse_disable_data_reporting)();
int (find_symbol)(uint8_t x_len, uint8_t tolerance);
int (kbc_write)(int port, uint8_t cmd);


#endif /* MOUSE_H */
