// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <lcom/lab5.h>

#include <stdint.h>
#include <stdio.h>

// Any header files included below this line should have been created by you

#include "graph.h"
//#include "test.c"
//#include "test2.c"

extern uint8_t ob_byte;
//extern char *test[]; // Test image
//extern char *test2[];

/*extern unsigned h_res;
extern unsigned v_res;*/
//unsigned bits_per_pixel;

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab5/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab5/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(video_test_init)(uint16_t mode, uint8_t delay) {
  if (set_vbe_mode(mode) != OK)
    return 1;
  tickdelay(micros_to_ticks(delay*1e6));
  if (vg_exit() != 0)
    return 2;
  return 0;
}

int(video_test_rectangle)(uint16_t mode, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color)
{
  if (vg_init(mode) == NULL)
    return 1;

  for (uint16_t yd=y; yd != y + height; yd++)
    vg_draw_hline(x,yd,width,color);
  
  int ipc_status, r;
  message msg;
  uint8_t hook_id = 2, irq_set = BIT(hook_id);

  if (kbc_subscribe_int(&hook_id) != OK)
    return 1;

  while (ob_byte != 0x81)
  {
    if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) { 
        printf("driver_receive failed with: %d", r);
        continue;
    }

    if (is_ipc_notify(ipc_status))
    {
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:				
          if (msg.m_notify.interrupts & irq_set)
            kbc_ih();

          break;

        default:
          break;
      }	
    }
  }

  if (kbc_unsubscribe_int() != OK)
    return 2;

  if (vg_exit() != 0)
    return 3;
  
  return 0;
}

int(video_test_pattern)(uint16_t mode, uint8_t no_rectangles, uint32_t first, uint8_t step)
{
  if (vg_init(mode) == NULL)
    return 1;

  if (draw_pattern(no_rectangles, first, step) != OK)
    return 2;


  int ipc_status, r;
  message msg;
  uint8_t hook_id = 2, irq_set = BIT(hook_id);

  if (kbc_subscribe_int(&hook_id) != OK)
    return 1;

  while (ob_byte != 0x81)
  {
    if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) { 
        printf("driver_receive failed with: %d", r);
        continue;
    }

    if (is_ipc_notify(ipc_status))
    {
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:				
          if (msg.m_notify.interrupts & irq_set)
            kbc_ih();

          break;

        default:
          break;
      }	
    }
  }

  if (kbc_unsubscribe_int() != OK)
    return 2;

  if (vg_exit() != 0)
    return 3;
  
  return 0;
}

int(video_test_xpm)(xpm_map_t xpm, uint16_t x, uint16_t y)
{
  if (vg_init(0x105) == NULL) // 0x115 for test image
    return 1;

  xpm_image_t img;
  uint8_t *map;
  // get the pixmap from the XPM
  //map = xpm_load((xpm_map_t) test, XPM_8_8_8, &img); // for test image
  //map = xpm_load((xpm_map_t) test2, XPM_8_8_8, &img); 
  map = xpm_load(xpm, XPM_INDEXED, &img);

  draw_xpm(img, map, x, y);

  int ipc_status, r;
  message msg;
  uint8_t hook_id = 2, irq_set = BIT(hook_id);

  if (kbc_subscribe_int(&hook_id) != OK)
    return 1;

  while (ob_byte != 0x81)
  {
    if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) { 
        printf("driver_receive failed with: %d", r);
        continue;
    }

    if (is_ipc_notify(ipc_status))
    {
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:				
          if (msg.m_notify.interrupts & irq_set)
            kbc_ih();

          break;

        default:
          break;
      }	
    }
  }

  if (kbc_unsubscribe_int() != OK)
    return 2;

  if (vg_exit() != 0)
    return 3;
  
  return 0;
}

int(video_test_move)(xpm_map_t xpm, uint16_t xi, uint16_t yi, uint16_t xf, uint16_t yf, int16_t speed, uint8_t fr_rate)
{
  if (vg_init(0x105) == NULL)
    return 1;

  xpm_image_t img;
  uint8_t *map;
  // get the pixmap from the XPM
  map = xpm_load(xpm, XPM_INDEXED, &img);

  uint64_t count = 0;
  int frames_passed = 0;
  int ipc_status, r;
  message msg;
  uint8_t hook_id_timer = 0, hook_id_kbc = 2, irq_set_timer = BIT(hook_id_timer), irq_set_kbc = BIT(hook_id_kbc);

  if (kbc_subscribe_int(&hook_id_kbc) != OK)
    return 1;

  if (timer_subscribe_int(&hook_id_timer) != OK)
    return 2;

  draw_xpm(img, map, xi, yi);

  while (ob_byte != 0x81)
  {
    if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) { 
        printf("driver_receive failed with: %d", r);
        continue;
    }

    if (is_ipc_notify(ipc_status))
    {
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:
          if (msg.m_notify.interrupts & irq_set_timer)
          {
            count++;
            if (count % (sys_hz()/fr_rate) == 0)
            {
              if (speed > 0)
              {
                if (xi != xf)
                  xi+=speed;
                else if (yi != yf)
                  yi+=speed;
              }
              else
              {
                frames_passed++;
                if (abs(speed) == frames_passed)
                {
                  frames_passed = 0;
                  if (xi != xf)
                    xi+=1;
                  else if (yi != yf)
                    yi+=1;
                }
              }

              draw_xpm(img, map, xi, yi);
            }
          }

          if (msg.m_notify.interrupts & irq_set_kbc)
            kbc_ih();

          break;

        default:
          break;
      }	
    }
  }

  if (kbc_unsubscribe_int() != OK)
    return 3;

  if (timer_unsubscribe_int() != OK)
    return 4;

  if (vg_exit() != 0)
    return 5;
  
  return 0;
}

int(video_test_controller)() {
  /* To be completed */
  printf("%s(): under construction\n", __func__);

  return 1;
}
