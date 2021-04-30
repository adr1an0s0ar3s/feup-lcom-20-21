#ifndef GRAPH_H
#define GRAPH_H

#define DELAY_US    20000
#define TIMER0_IRQ 0
#define KBC_IRQ 1
#define KBC_STATUS 0x64
#define KBC_OUT_BUF 0x60
#define KBC_PAR_ERR BIT(7)
#define KBC_TO_ERR BIT(6)

//#include <machine/int86.h>

int (set_vbe_mode)(uint16_t mode);
void *(vg_init)(uint16_t mode);
int (vg_draw_hline)(uint16_t x, uint16_t y, uint16_t len, uint32_t color);
int (vg_draw_rectangle)(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color);
int (draw_pattern)(uint8_t no_rectangles, uint32_t first, uint8_t step);
int (draw_xpm)(xpm_image_t img, uint8_t *map, uint16_t x, uint16_t y);
int (kbc_subscribe_int)(uint8_t *bit_no);
int (kbc_unsubscribe_int)();
void (kbc_ih)();
int (timer_subscribe_int)(uint8_t *bit_no);
int (timer_unsubscribe_int)();

int (get_mode_info)(uint16_t mode, vbe_mode_info_t *vmi_p);

#endif //GRAPH_H
