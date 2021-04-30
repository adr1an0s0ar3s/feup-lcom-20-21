#include <lcom/lcf.h>
#include <lcom/liblm.h>
#include <lcom/proj.h>

#include <stdbool.h>
#include <stdint.h>

#include "i8042.h"
#include "keyboard.h"
#include "util.h"

uint8_t keys[2];
static int hook_id_keyboard;

int (kbc_subscribe_int)(uint8_t *bit_no)
{
  hook_id_keyboard = *bit_no;
  
  if (sys_irqsetpolicy(KBC_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &hook_id_keyboard) != OK)
    return 1;

  return 0;
}

int (kbc_unsubscribe_int)()
{  
  if (sys_irqrmpolicy(&hook_id_keyboard) != OK)
    return 1;

  return 0;
}

int (keyboard_ih)()
{
  static int counter = 0;

  uint8_t stat;

  util_sys_inb(KBC_OUT_BUF, &keys[counter]);
  util_sys_inb(KBC_STATUS, &stat);

  if ((stat & (KBC_PAR_ERR | KBC_TO_ERR)) != 0) return false;

  counter++;

  if (counter == 1 && keys[counter-1] != 0xE0) {
    counter = 0;
    return true;
  }
  else if (counter == 2) {
    counter = 0;
    return true;
  }

  return false;
}

int (kbc_write)(int port, uint8_t cmd)
{
  uint8_t stat;

  while (1) {
    util_sys_inb(KBC_STATUS, &stat);
    if((stat & KBC_ST_IBF) == 0) {
      sys_outb(port, cmd);
      return 0;
    }
    tickdelay(micros_to_ticks(DELAY_US));
  }
}
