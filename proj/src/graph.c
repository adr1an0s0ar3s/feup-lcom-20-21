#include <lcom/lcf.h>
#include <lcom/liblm.h>
#include <lcom/proj.h>

#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "graph.h"

static char *video_mem;
static char *buffer;
static vbe_mode_info_t vmi_p;

static uint16_t line;

int (set_vbe_mode)(uint16_t mode)
{
    reg86_t r;
    memset(&r, 0, sizeof(r));
    r.ax = 0x4F02;
    r.bx = 1<<14|mode;
    r.intno = 0x10;
    if( sys_int86(&r) != OK ) {
        printf("set_vbe_mode: sys_int86() failed \n");
        return 1;
    }
    return 0;
}

void *(vg_init)(uint16_t mode)
{
    if (get_mode_info(mode,&vmi_p) != OK)
        return NULL;
    
    int r;
    struct minix_mem_range mr;
    unsigned int vram_base = vmi_p.PhysBasePtr;
    unsigned int vram_size = vmi_p.XResolution*vmi_p.YResolution*ceil(vmi_p.BitsPerPixel/8)*2;

    /* Allow memory mapping */
    mr.mr_base = (phys_bytes) vram_base;
    mr.mr_limit = mr.mr_base + vram_size;
    if(OK != (r = sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr)))
        panic("sys_privctl (ADD_MEM) failed: %d\n", r);
    
    /* Map memory */
    video_mem = vm_map_phys(SELF, (void *)mr.mr_base, vram_size);
    if(video_mem == MAP_FAILED)
        panic("couldn’t map video memory");
    
    buffer = video_mem + vmi_p.XResolution*vmi_p.YResolution*(int)ceil(vmi_p.BitsPerPixel/8);
    line = vmi_p.YResolution;

    if (set_vbe_mode(mode) != OK)
        return NULL;
    
    return video_mem;
}

int (draw_xpm)(uint8_t *map, xpm_image_t img, uint16_t x, uint16_t y)
{
    for (uint16_t yd = y; yd != y + img.height && yd < vmi_p.YResolution; yd++)
    {
        for (uint16_t xd = x; xd != x + img.width && xd < vmi_p.XResolution; xd++)
        {
            if (!(map[((xd-x)+(yd-y)*img.width)*(int)ceil(vmi_p.BitsPerPixel/8)+2] == 0x00 && map[((xd-x)+(yd-y)*img.width)*(int)ceil(vmi_p.BitsPerPixel/8)+1] == 0xb1 && map[((xd-x)+(yd-y)*img.width)*(int)ceil(vmi_p.BitsPerPixel/8)] == 0x40))
                memcpy(buffer+(xd+yd*vmi_p.XResolution)*(int)ceil(vmi_p.BitsPerPixel/8), map+((xd-x)+(yd-y)*img.width)*(int)ceil(vmi_p.BitsPerPixel/8), (int)ceil(vmi_p.BitsPerPixel/8));
        }
    }
        
    return 0;
}

int (draw_part_of_xpm)(uint8_t *map, xpm_image_t img, uint16_t x, uint16_t y, uint16_t part_x, uint16_t part_y, uint16_t width, uint16_t height)
{
    for (uint16_t yd = y+part_y; yd != y + part_y + height && yd < vmi_p.YResolution; yd++)
    {
        for (uint16_t xd = x+part_x; xd != x + part_x + width && xd < vmi_p.XResolution; xd++)
        {
            if (!(map[((xd-x)+(yd-y)*img.width)*(int)ceil(vmi_p.BitsPerPixel/8)+2] == 0x00 && map[((xd-x)+(yd-y)*img.width)*(int)ceil(vmi_p.BitsPerPixel/8)+1] == 0xb1 && map[((xd-x)+(yd-y)*img.width)*(int)ceil(vmi_p.BitsPerPixel/8)] == 0x40))
                memcpy(buffer+(xd+yd*vmi_p.XResolution)*(int)ceil(vmi_p.BitsPerPixel/8), map+((xd-x)+(yd-y)*img.width)*(int)ceil(vmi_p.BitsPerPixel/8), (int)ceil(vmi_p.BitsPerPixel/8));
        }
    }
    
    return 0;
}

int (display_frame)()
{
    reg86_t r;

    memset(&r, 0, sizeof(r));

    r.ax = 0x4F07;
    r.bh = 0x00;
    r.bl = 0x00;
    r.cx = 0;
    r.dx = line;
    r.intno = 0x10;
    
    if (sys_int86(&r) != OK) {
        printf("display_frame(): sys_int86() failed \n");
        return 1;
    }

    if (line == 0) line = vmi_p.YResolution; else line = 0;

    char *temp = buffer;
    buffer = video_mem;
    video_mem = temp;
    
    return 0;
}

int (get_mode_info)(uint16_t mode, vbe_mode_info_t *vmi_p)
{
    reg86_t r;
    mmap_t map;

    memset(&r, 0, sizeof(r));

    if (lm_alloc(sizeof(vbe_mode_info_t), &map) == NULL)
        return 1;

    r.ax = 0x4F01;
    r.es = PB2BASE(map.phys);
    r.di = PB2OFF(map.phys);
    r.cx = mode;
    r.intno = 0x10;

    if (sys_int86(&r) != OK) {
        printf("get_mode_info(): sys_int86() failed \n");
        return 2;
    }

    *vmi_p = * ((vbe_mode_info_t *) map.virt);

    if (lm_free(&map) == false)
        return 3;

    return 0;
}
