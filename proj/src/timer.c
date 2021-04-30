#include <lcom/lcf.h>
#include <lcom/liblm.h>
#include <lcom/proj.h>

#include <stdbool.h>
#include <stdint.h>

#include "i8254.h"

static int hook_id;

int (timer_set_frequency)(uint8_t timer, uint32_t freq)
{
  if (timer > 2 || freq < 19 || freq >= TIMER_FREQ/2)
    return 1;
  
  uint16_t val = (uint16_t) (TIMER_FREQ / freq);

  uint8_t st, lsb, msb;

  util_get_LSB(val, &lsb);
  util_get_MSB(val, &msb);

  if (timer_get_conf(timer, &st) != OK)
    return 2;

  if (sys_outb(TIMER_CTRL, timer << 6 | TIMER_LSB_MSB | (st & TIMER_LSB_STATUS)) != OK)
    return 3;
  
  if (sys_outb(TIMER_0 + timer, lsb) != OK)
    return 4;

  if (sys_outb(TIMER_0 + timer, msb) != OK)
    return 5;

  return 0;
}

int (timer_subscribe_int)(uint8_t *bit_no)
{
  hook_id = *bit_no;
  
  if (sys_irqsetpolicy(TIMER0_IRQ, IRQ_REENABLE, &hook_id) != OK)
    return 1;

  return 0;
}

int (timer_unsubscribe_int)()
{  
  if (sys_irqrmpolicy(&hook_id) != OK)
    return 1;

  return 0;
}

int (timer_get_conf)(uint8_t timer, uint8_t *st)
{
  if (timer > 2 || st == NULL)
    return 1;

  if (sys_outb(TIMER_CTRL, TIMER_RB_CMD | TIMER_RB_COUNT_ | TIMER_RB_SEL(timer)) != OK)
    return 2;

  return util_sys_inb(TIMER_0 + timer, st);
}
