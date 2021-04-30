#include <lcom/lcf.h>
#include <lcom/liblm.h>
#include <lcom/proj.h>

#include <stdbool.h>
#include <stdint.h>

#include "menu_xpm.h"
#include "menu.h"
#include "graph.h"

// Menu
static xpm_image_t menu_info;
static uint8_t *menu;

// Button 1 Highlight
static xpm_image_t button_info;
static uint8_t *button1_highlight;

// Button 2 Highlight
static uint8_t *button2_highlight;

// Button 3 Highlight
static uint8_t *button3_highlight;

// Button 4 Highlight
static uint8_t *button4_highlight;

// Cursor
static xpm_image_t cursor_info;
static uint8_t *cursor;

// Online Waiting Menu
static uint8_t *online_waiting_menu;

// instructions Menu
static uint8_t *instructions_menu;

int (startMenu)()
{
    static bool loaded = false;

    if (!loaded) {
        menu = xpm_load(menu_xpm, XPM_8_8_8, &menu_info);
        button1_highlight = xpm_load(button1_highlight_xpm, XPM_8_8_8, &button_info);
        button2_highlight = xpm_load(button2_highlight_xpm, XPM_8_8_8, &button_info);
        button3_highlight = xpm_load(button3_highlight_xpm, XPM_8_8_8, &button_info);
        button4_highlight = xpm_load(button4_highlight_xpm, XPM_8_8_8, &button_info);
        cursor = xpm_load(cursor_xpm, XPM_8_8_8, &cursor_info);
        online_waiting_menu = xpm_load(online_waiting_menu_xpm, XPM_8_8_8, &menu_info);
        instructions_menu = xpm_load(instructions_menu_xpm, XPM_8_8_8, &menu_info);

        loaded = true;
    }

    draw_xpm(menu, menu_info, 0, 0);
    draw_xpm(cursor, cursor_info, 400, 300);
    display_frame();

    draw_xpm(menu, menu_info, 0, 0);
    draw_xpm(cursor, cursor_info, 400, 300);
    display_frame();

    return 0;
}

int (showMenu)(int16_t d_x, int16_t d_y, bool click)
{
    static int16_t x = 400, y = 300;
    static int16_t before[2][2] = {{400,300},{400,300}};

    before[0][0] = before[1][0]; before[0][1] = before[1][1];
    before[1][0] = x; before[1][1] = y;

    x += d_x; y -= d_y;
    
    // Setting the limits of the Screen
    if (x < 0) x = 0;
    else if (x > 800) x = 800;
    if (y < 0) y = 0;
    else if (y > 600) y = 600;

    // Erase Last Cursor
    draw_part_of_xpm(menu, menu_info, 0, 0, before[0][0], before[0][1], 20, 20);

    //printf("%d %d -- %d %d\n", before[0][0], before[0][1], before[1][0], before[1][1]);

    // Draw Buttons Highlight
    if (94 <= x && x <= 94+292 && 353 <= y && y <= 353+82)
        draw_xpm(button1_highlight, button_info, 94, 353);
    else if (!(94 <= x && x <= 94+292 && 353 <= y && y <= 353+82) && (94 <= before[0][0] && before[0][0] <= 94+292 && 353 <= before[0][1] && before[0][1] <= 353+82))
        draw_part_of_xpm(menu, menu_info, 0, 0, 94, 353, 292, 82);
    
    if (414 <= x && x <= 414+292 && 353 <= y && y <= 353+82)
        draw_xpm(button2_highlight, button_info, 414, 353);
    else if (!(414 <= x && x <= 414+292 && 353 <= y && y <= 353+82) && (414 <= before[0][0] && before[0][0] <= 414+292 && 353 <= before[0][1] && before[0][1] <= 353+82))
        draw_part_of_xpm(menu, menu_info, 0, 0, 414, 353, 292, 82);
    
    if (94 <= x && x <= 94+292 && 461 <= y && y <= 461+82)
        draw_xpm(button3_highlight, button_info, 94, 461);
    else if (!(94 <= x && x <= 94+292 && 461 <= y && y <= 461+82) && (94 <= before[0][0] && before[0][0] <= 94+292 && 461 <= before[0][1] && before[0][1] <= 461+82))
        draw_part_of_xpm(menu, menu_info, 0, 0, 94, 461, 292, 82);
    
    if (414 <= x && x <= 414+292 && 461 <= y && y <= 461+82)
        draw_xpm(button4_highlight, button_info, 414, 461);
    else if (!(414 <= x && x <= 414+292 && 461 <= y && y <= 461+82) && (414 <= before[0][0] && before[0][0] <= 414+292 && 461 <= before[0][1] && before[0][1] <= 461+82))
        draw_part_of_xpm(menu, menu_info, 0, 0, 414, 461, 292, 82);

    // Check if Player Clicked a Button
    if (click && 94 <= x && x <= 94+292 && 353 <= y && y <= 353+82) {
        x = 400; y = 300;
        before[0][0] = 400; before[0][1] = 300;
        before[1][0] = 400; before[1][1] = 300;
        return 1;
    }
    else if (click && 414 <= x && x <= 414+292 && 353 <= y && y <= 353+82) {
        x = 400; y = 300;
        before[0][0] = 400; before[0][1] = 300;
        before[1][0] = 400; before[1][1] = 300;
        return 2;
    }
    else if (click && 94 <= x && x <= 94+292 && 461 <= y && y <= 461+82) {
        x = 400; y = 300;
        before[0][0] = 400; before[0][1] = 300;
        before[1][0] = 400; before[1][1] = 300;
        return 3;
    }
    else if (click && 414 <= x && x <= 414+292 && 461 <= y && y <= 461+82)
        return 4;

    // Draw Cursor
    draw_xpm(cursor, cursor_info, x, y);
    display_frame();

    return 0;
}

int (showOnlineWaitingMenu)()
{
    if (draw_xpm(online_waiting_menu, menu_info, 0, 0) != OK)
        return 1;

    display_frame();
    
    return 0;
}

int (showInstructionsMenu)()
{
    if (draw_xpm(instructions_menu, menu_info, 0, 0) != OK)
        return 1;

    display_frame();
    
    return 0;
}
