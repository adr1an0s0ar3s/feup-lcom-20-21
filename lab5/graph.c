#include <lcom/lcf.h>
#include "graph.h"
#include <lcom/vbe.h>
#include <math.h>

// Keyboard
uint8_t ob_byte, discard;
int hook_id_kbc;

// Timer
int hook_id_timer;

static char *video_mem;		/* Process (virtual) address to which VRAM is mapped */

static unsigned h_res;	        /* Horizontal resolution in pixels */
static unsigned v_res;	        /* Vertical resolution in pixels */
static unsigned bits_per_pixel; /* Number of VRAM bits per pixel */

static uint8_t RedMaskSize;
static uint8_t GreenMaskSize;
static uint8_t BlueMaskSize;


int (set_vbe_mode)(uint16_t mode)
{
    reg86_t r;
    memset(&r, 0, sizeof(r));	/* zero the structure */
    r.ax = 0x4F02; // VBE call, function 02 -- set VBE mode
    r.bx = 1<<14|mode; // set bit 14: linear framebuffer
    r.intno = 0x10; /* BIOS video services */
    if( sys_int86(&r) != OK ) {
        printf("set_vbe_mode: sys_int86() failed \n");
        return 1;
    }
    return 0;
}

void *(vg_init)(uint16_t mode)
{
    vbe_mode_info_t vmi_p;
    if (get_mode_info(mode,&vmi_p) != OK)
        return NULL;
    
    h_res = vmi_p.XResolution;
    v_res = vmi_p.YResolution;
    bits_per_pixel = vmi_p.BitsPerPixel;
    RedMaskSize = vmi_p.RedMaskSize;
    GreenMaskSize = vmi_p.GreenMaskSize;
    BlueMaskSize = vmi_p.BlueMaskSize;

    int r;
    struct minix_mem_range mr; /* physical memory range */
    unsigned int vram_base = vmi_p.PhysBasePtr; /* VRAM’s physical addresss */
    unsigned int vram_size = h_res*v_res*ceil(bits_per_pixel/8); /* VRAM’s size, but you can use the frame-buffer size, instead */
    /* Allow memory mapping */
    mr.mr_base = (phys_bytes) vram_base;
    mr.mr_limit = mr.mr_base + vram_size;
    if( OK != (r = sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr)))
        panic("sys_privctl (ADD_MEM) failed: %d\n", r);
    /* Map memory */
    video_mem = vm_map_phys(SELF, (void *)mr.mr_base, vram_size);
    if(video_mem == MAP_FAILED)
        panic("couldn’t map video memory");

    if (set_vbe_mode(mode) != OK)
        return NULL;
    
    return video_mem;
}

int (vg_draw_hline)(uint16_t x, uint16_t y, uint16_t len, uint32_t color)
{
    if (color > (pow(2, bits_per_pixel) - 1))
        return 1;

    for (uint16_t xd = x; xd != x + len; xd++)
    {
        if (y >= 0 && y < v_res && x >= 0 && xd < h_res)
        {
            if (bits_per_pixel == 8)
                video_mem[xd+y*h_res] = color;
            else
            {
                // It's inverted!
                video_mem[(xd+y*h_res)*(int)ceil(bits_per_pixel/8)]=color;
                video_mem[(xd+y*h_res)*(int)ceil(bits_per_pixel/8)+1]=color>>BlueMaskSize;
                video_mem[(xd+y*h_res)*(int)ceil(bits_per_pixel/8)+2]=color>>(BlueMaskSize+GreenMaskSize);
            }
        }
    }

    return 0;
}

int (vg_draw_rectangle)(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color)
{
    for (uint16_t yd=y; yd != y + height; yd++)
        if (vg_draw_hline(x,yd,width,color) != OK)
            return 1;
    
    return 0;
}

int (draw_pattern)(uint8_t no_rectangles, uint32_t first, uint8_t step)
{
    uint16_t width = h_res/no_rectangles, height = v_res/no_rectangles, x = 0, y = 0;
    uint32_t color = 0, R = 0, G = 0, B = 0;

    for (uint16_t row = 0; row < no_rectangles; y+=height, row++)
    {
        for (uint16_t col = 0; col < no_rectangles; x+=width, col++)
        {
            if (bits_per_pixel == 8)
                color = (first + (row * no_rectangles + col) * step) % (1 << bits_per_pixel);
            else
            {
                R = ((first>>(GreenMaskSize+BlueMaskSize))+col*step)%(1<<RedMaskSize);
                G = (((first>>BlueMaskSize)&((1<<GreenMaskSize)-1))+row*step)%(1<<GreenMaskSize);
                B = ((first&((1<<BlueMaskSize)-1))+(col+row)*step)%(1<<BlueMaskSize);
                color = R<<(GreenMaskSize+BlueMaskSize) | G<<BlueMaskSize | B;
            }

            if (vg_draw_rectangle(x, y, width, height, color) != OK)
                return 1;
        }

        x = 0;
    }

    return 0;
}

int (draw_xpm)(xpm_image_t img, uint8_t *map, uint16_t x, uint16_t y)
{
    uint64_t count = 0;

    memset(video_mem, 0, v_res*h_res);

    for (uint16_t yd = y; yd != y + img.height && yd < v_res; yd++)
    {
        for (uint16_t xd = x; xd != x + img.width && xd < h_res; xd++)
        {
            if (bits_per_pixel == 8)
            {
                video_mem[xd+yd*h_res] = map[count];
                count++;
            }
            else
            {
                video_mem[(xd+yd*h_res)*(int)ceil(bits_per_pixel/8)]=map[count];
                count++;
                video_mem[(xd+yd*h_res)*(int)ceil(bits_per_pixel/8)+1]=map[count];
                count++;
                video_mem[(xd+yd*h_res)*(int)ceil(bits_per_pixel/8)+2]=map[count];
                count++;
            }
        }
    }

    return 0;
}

int (kbc_subscribe_int)(uint8_t *bit_no)
{
  hook_id_kbc = *bit_no;
  
  if (sys_irqsetpolicy(KBC_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &hook_id_kbc) != OK)
    return 1;

  return 0;
}


int (kbc_unsubscribe_int)()
{  
  if (sys_irqrmpolicy(&hook_id_kbc) != OK)
    return 1;

  return 0;
}

void (kbc_ih)()
{
  uint8_t stat;

  util_sys_inb(KBC_OUT_BUF, &ob_byte);
  util_sys_inb(KBC_STATUS, &stat);

  if ((stat & (KBC_PAR_ERR | KBC_TO_ERR)) == 0) {discard = 0;}
  else {discard = 1;}
}


int (timer_subscribe_int)(uint8_t *bit_no)
{
    hook_id_timer = *bit_no;

    if (sys_irqsetpolicy(TIMER0_IRQ, IRQ_REENABLE, &hook_id_timer) != OK)
        return 1;

    return 0;
}

int (timer_unsubscribe_int)()
{
    if (sys_irqrmpolicy(&hook_id_timer) != OK)
        return 1;

    return 0;
}

int (util_sys_inb)(int port, uint8_t *value)
{
    if (value != NULL){
        u32_t temp;
        sys_inb(port, &temp);
        *value = (uint8_t) temp;
        return 0;
    }

    return 1;
}

int (get_mode_info)(uint16_t mode, vbe_mode_info_t *vmi_p)
{
    reg86_t r;
    mmap_t map;

    memset(&r, 0, sizeof(r));	/* zero the structure */

    if (lm_alloc(sizeof(vbe_mode_info_t), &map) == NULL)
        return 1;

    r.ax = 0x4F01; /* VBE get mode info */
    /* translate the buffer linear address to a far pointer */
    r.es = PB2BASE(map.phys); /* set a segment base */
    r.di = PB2OFF(map.phys); /* set the offset accordingly */
    r.cx = mode;
    r.intno = 0x10;

    if( sys_int86(&r) != OK ) {
        printf("get_mode_info(): sys_int86() failed \n");
        return 2;
    }

    *vmi_p = * ((vbe_mode_info_t *) map.virt);

    if (lm_free(&map) == false)
        return 3;

    return 0;
}
