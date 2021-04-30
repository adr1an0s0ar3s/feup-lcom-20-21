#include <lcom/lcf.h>

#include "mouse.h"
#include "util.h"

int hook_id_mouse, hook_id_timer;
struct packet pp;
int packet_counter = 0;
uint64_t count_time;

void (mouse_ih)()
{
  uint8_t stat, data;

  util_sys_inb(KBC_OUT_BUF, &data);
  util_sys_inb(KBC_STATUS, &stat);

  if ((stat & (KBC_PAR_ERR | KBC_TO_ERR)) != 0 || (packet_counter == 0 && !(data & BIT(3)))){return;}

  (pp.bytes)[packet_counter] = data;

  switch (packet_counter)
  {
  case 0:
    pp.lb = data & BIT(0);
    pp.rb = data & BIT(1);
    pp.mb = data & BIT(2);
    pp.x_ov = data & BIT(6);
    pp.y_ov = data & BIT(7);
    packet_counter++;
    break;
  case 1:
    pp.delta_x = (pp.bytes)[1];
    if (((pp.bytes)[0] & BIT(4)) != 0)
      pp.delta_x += 65280;
    packet_counter++;
    break;
  case 2:
    pp.delta_y = (pp.bytes)[2];
    if (((pp.bytes)[0] & BIT(5)) != 0)
      pp.delta_y += 65280;
    packet_counter = 0;
    break;
  }
}

void (timer_int_handler)() {count_time++;}

int (mouse_subscribe_int)(uint8_t *bit_no)
{
  hook_id_mouse = *bit_no;
  
  if (sys_irqsetpolicy(MOUSE_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &hook_id_mouse) != OK)
    return 1;

  return 0;
}

int (mouse_unsubscribe_int)()
{  
  if (sys_irqrmpolicy(&hook_id_mouse) != OK)
    return 1;

  return 0;
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

int (util_sys_inb)(int port, uint8_t *value) {
  
  if (value != NULL){
    u32_t temp;
    sys_inb(port, &temp);
    *value = (uint8_t) temp;
    return 0;
  }

  return 1;
}

int (kbc_write)(int port, uint8_t cmd)
{
  uint8_t stat;

  while(1) {
    util_sys_inb(KBC_STATUS, &stat);
    if((stat & KBC_ST_IBF) == 0) {
      sys_outb(port, cmd);
      return 0;
    }
    tickdelay(micros_to_ticks(DELAY_US));
  }
}

int (new_mouse_enable_data_reporting)()
{
  kbc_write(KBC_IN, MOUSE_WRITE_COM);
  kbc_write(KBC_OUT_BUF, MOUSE_EN_DATA);
  uint8_t ret;
  util_sys_inb(KBC_OUT_BUF, &ret);
  if (ret != MOUSE_ACK)
    return 1;
  
  return 0;
}

int (mouse_disable_data_reporting)()
{
  kbc_write(KBC_IN, MOUSE_WRITE_COM);
  kbc_write(KBC_OUT_BUF, MOUSE_DI_DATA);
  uint8_t ret;
  util_sys_inb(KBC_OUT_BUF, &ret);
  if (ret != MOUSE_ACK)
    return 1;
  
  return 0;
}

int (find_symbol)(uint8_t x_len, uint8_t tolerance) {
  static int sum_x = 0;
  static int sum_y = 0;
  static int state = 0;

  mouse_print_packet(&pp);
  printf("Sum_x: %d\nSum_y: %d\nUp: %d\n", sum_x, sum_y, state);


  switch (state)
  {
  case 0:

    if (pp.lb && !pp.rb && !pp.mb)
    {
      if ((pp.delta_x < 0 && abs(pp.delta_x) > tolerance) || (pp.delta_y < 0 && abs(pp.delta_y) > tolerance))
      {
        sum_x = 0;
        sum_y = 0;
        return 0;
      }
      else
      {
        sum_x += pp.delta_x;
        sum_y += pp.delta_y;
        return 0;
      }
    }
    else if (!pp.lb && !pp.rb && !pp.mb && sum_x != 0 && abs(sum_y/sum_x) >= 1 && sum_x >= x_len) {
      sum_x = 0;
      sum_y = 0;
      state = 1;
      return 0;
    }
    else 
    {
      sum_x = 0;
      sum_y = 0;
      return 0;
    }

    break;
  
  case 1:

    if (!pp.lb && !pp.rb && !pp.mb)
    {
      if ((pp.delta_x < 0 && abs(pp.delta_x) > tolerance) || (pp.delta_y < 0 && abs(pp.delta_y) > tolerance))
      {
        state = 0;
        return 0;
      }
    }
    else if (!pp.lb && pp.rb && !pp.mb)
    {
      state = 2;
      return 0;
    }

    break;

  case 2:

    if (!pp.lb && pp.rb && !pp.mb)
    {
      if ((pp.delta_x < 0 && abs(pp.delta_x) > tolerance) || (pp.delta_y > 0 && abs(pp.delta_y) > tolerance))
      {
        sum_x = 0;
        sum_y = 0;
        state = 0;
        return 0;
      }
      else
      {
        sum_x += pp.delta_x;
        sum_y += pp.delta_y;
        return 0;
      }
    }
    else if (!pp.lb && !pp.rb && !pp.mb && sum_x != 0 && abs(sum_y/sum_x) >= 1 && sum_x >= x_len) {
      return 1;
    }
    else 
    {
      sum_x = 0;
      sum_y = 0;
      return 0;
    }

    break;
  }

  return 0;
}
