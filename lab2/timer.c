#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>

#include "i8254.h"

int hook_id;
extern int counter;

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
  
  /*switch (timer)
  {
    case 0:
      sys_outb(TIMER_CTRL, TIMER_SEL0 | TIMER_LSB_MSB | (st & TIMER_LSB_STATUS));
      sys_outb(TIMER_0, lsb);
      sys_outb(TIMER_0, msb);
      break;
        
    case 1:
      sys_outb(TIMER_CTRL, TIMER_SEL1 | TIMER_LSB_MSB | (st & TIMER_LSB_STATUS));
      sys_outb(TIMER_1, lsb);
      sys_outb(TIMER_1, msb);
      break;

    case 2:
      sys_outb(TIMER_CTRL, TIMER_SEL2 | TIMER_LSB_MSB | (st & TIMER_LSB_STATUS));
      sys_outb(TIMER_2, lsb);
      sys_outb(TIMER_2, msb);
      break;
  }

  if (msb == 0 && lsb != 0)
  {
    switch (timer)
    {
      case 0:
        sys_outb(TIMER_CTRL, TIMER_SEL0 | TIMER_LSB | (st & (BIT(3) | BIT(2) | BIT(1) | BIT(0))));
        sys_outb(TIMER_0, lsb);
        break;
        
      case 1:
        sys_outb(TIMER_CTRL, TIMER_SEL1 | TIMER_LSB | (st & (BIT(3) | BIT(2) | BIT(1) | BIT(0))));
        sys_outb(TIMER_1, lsb);
        break;

      case 2:
        sys_outb(TIMER_CTRL, TIMER_SEL2 | TIMER_LSB | (st & (BIT(3) | BIT(2) | BIT(1) | BIT(0))));
        sys_outb(TIMER_2, lsb);
        break;
    }
  }

  if (msb != 0 && lsb == 0)
  {
    switch (timer)
    {
      case 0:
        sys_outb(TIMER_CTRL, TIMER_SEL0 | TIMER_MSB | (st & (BIT(3) | BIT(2) | BIT(1) | BIT(0))));
        sys_outb(TIMER_0, msb);
        break;
        
      case 1:
        sys_outb(TIMER_CTRL, TIMER_SEL1 | TIMER_MSB | (st & (BIT(3) | BIT(2) | BIT(1) | BIT(0))));
        sys_outb(TIMER_1, msb);
        break;

      case 2:
        sys_outb(TIMER_CTRL, TIMER_SEL2 | TIMER_MSB | (st & (BIT(3) | BIT(2) | BIT(1) | BIT(0))));
        sys_outb(TIMER_2, msb);
        break;
    }
  }*/

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

void (timer_int_handler)() {counter++;}

int (timer_get_conf)(uint8_t timer, uint8_t *st)
{
  if (timer > 2 || st == NULL)
    return 1;

  // Primeiro digo ao clock para pedir a config
  if (sys_outb(TIMER_CTRL, TIMER_RB_CMD | TIMER_RB_COUNT_ | TIMER_RB_SEL(timer)) != OK)
    return 2;
  
  // Agora vejo a config no registo de data do respetivo clock
  return util_sys_inb(TIMER_0 + timer, st);
}

int (timer_display_conf)(uint8_t timer, uint8_t st, enum timer_status_field field)
{
  if (timer > 2)
    return 1;

  union timer_status_field_val temp;

  if (field == tsf_all)
    temp.byte = st;
  else if (field == tsf_initial)
  {
    temp.in_mode = (unsigned int) ((st & TIMER_LSB_MSB) >> 4);
    if (temp.in_mode > 3)  // Caso seja 0 já é inválido, caso seja maior que 3 o enum assume ser inválido (passando o int a 0)
      temp.in_mode = 0;
  }
  else if (field == tsf_mode)
  {
    temp.count_mode = ((st & TIMER_MODE) >> 1);
    if (temp.count_mode > 5)  // Caso count_mode for 6 passa para modo 2, caso seja 7 passa para modo 3 (compatibilidade)
      temp.count_mode -= 4;
  }
  else if (field == tsf_base)
    temp.bcd = (bool) (st & BIT(0));
  
  return timer_print_config(timer, field, temp);
}
