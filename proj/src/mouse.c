#include <lcom/lcf.h>
#include <lcom/liblm.h>
#include <lcom/proj.h>

#include <stdbool.h>
#include <stdint.h>

#include "mouse.h"
#include "keyboard.h"
#include "i8042.h"
#include "util.h"

struct packet pp;
static int hook_id_mouse;

int (mouse_interrupt_handler)()
{
  static int packet_counter = 0;

  packet_counter %= 3;

  uint8_t stat, data;

  util_sys_inb(KBC_OUT_BUF, &data);
  util_sys_inb(KBC_STATUS, &stat);

  if ((stat & (KBC_PAR_ERR | KBC_TO_ERR)) != 0 || (packet_counter == 0 && !(data & BIT(3)))){return false;}

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
    packet_counter++;
    break;
  }

  return (packet_counter == 3);
}

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

int (enable_data_reporting)()
{
  kbc_write(KBC_IN, MOUSE_WRITE_COM);
  kbc_write(KBC_OUT_BUF, MOUSE_EN_DATA);
  uint8_t ret;
  util_sys_inb(KBC_OUT_BUF, &ret);
  if (ret != MOUSE_ACK)
    return 1;
  
  return 0;
}

int (disable_data_reporting)()
{
  kbc_write(KBC_IN, MOUSE_WRITE_COM);
  kbc_write(KBC_OUT_BUF, MOUSE_DI_DATA);
  uint8_t ret;
  util_sys_inb(KBC_OUT_BUF, &ret);
  if (ret != MOUSE_ACK)
    return 1;
  
  return 0;
}
