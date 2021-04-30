#ifndef GRAPH_H
#define GRAPH_H

/** @defgroup graph graph
 * @{
 *
 * Handles Graphics Card and has some constants for programming the Graphics Card
 */

#define DELAY_US    20000
#define TIMER0_IRQ  0
#define KBC_IRQ     1
#define KBC_STATUS  0x64
#define KBC_OUT_BUF 0x60
#define KBC_PAR_ERR BIT(7)
#define KBC_TO_ERR  BIT(6)

/** 
 * Sets VBE to specified mode
 * @param mode mode to set the VBE to
 * @return 0 on success, non-zero otherwise
*/
int (set_vbe_mode)(uint16_t mode);

/**
 * Allocates the video_mem
 * @param mode mode to set
 * @return virtual address VRAM was mapped to. NULL on failure
*/
void *(vg_init)(uint16_t mode);

/**
 * Draws given XPM, starting at the specified coordinates
 * @param map the array of hexadecimal colors to be drawn
 * @param img Image specifications
 * @param x horizontal coordinate of the initial point
 * @param y vertical coordinate of the initial point
 * @return 0 on success
*/
int (draw_xpm)(uint8_t *map, xpm_image_t img, uint16_t x, uint16_t y);

/**
 * Draws part of given XPM, from specified coordinates onwards
 * @param map the array of hexadecimal colors to be drawn
 * @param img Image specifications
 * @param x horizontal coordinate of the initial point
 * @param y vertical coordinate of the initial point
 * @param part_x horizontal coordinate to the part of the image that we want to draw
 * @param part_y vertical coordinate to the part of the image that we want to draw
 * @param width the width of the part of the image that we want to draw
 * @param height the height of the part of the image that we want to draw
 * @return 0 on success
*/
int (draw_part_of_xpm)(uint8_t *map, xpm_image_t img, uint16_t x, uint16_t y, uint16_t part_x, uint16_t part_y, uint16_t width, uint16_t height);

/**
 * Gets information of VBE mode
 * @param mode mode whose information should be returned
 * @param vmi_p address of vbe_mode_info_t structure to be initialized
 * @return 0 on success, non-zero otherwise
*/
int (get_mode_info)(uint16_t mode, vbe_mode_info_t *vmi_p);

/**
 * Displays a frame by changing video_mem to buffer and vice-versa (page flipping)
 * @return 0 on success, non-zero otherwise
*/
int (display_frame)();

#endif //GRAPH_H
