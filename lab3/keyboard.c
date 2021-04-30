#include <lcom/lcf.h>

#include "i8042.h"
#include "keyboard.h"

int hook_id, timer_hook_id, count, timer;
uint8_t ob_byte, discard;


int (kbc_subscribe_int)(uint8_t *bit_no)
{
  hook_id = *bit_no;
  
  if (sys_irqsetpolicy(KBC_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &hook_id) != OK)
    return 1;

  return 0;
}

int (timer_subscribe_int)(uint8_t *bit_no)
{
  timer_hook_id = *bit_no;
  
  if (sys_irqsetpolicy(TIMER_IRQ, IRQ_REENABLE, &timer_hook_id) != OK)
    return 1;

  return 0;
}

int (kbc_unsubscribe_int)()
{  
  if (sys_irqrmpolicy(&hook_id) != OK)
    return 1;

  return 0;
}

int (timer_unsubscribe_int)()
{  
  if (sys_irqrmpolicy(&timer_hook_id) != OK)
    return 1;

  return 0;
}

int (util_sys_inb)(int port, uint8_t *value) {
  
  if (value != NULL){
    count++;
    u32_t temp;
    sys_inb(port, &temp);
    *value = (uint8_t) temp;
    return 0;
  }

  return 1;
}

void (kbc_ih)()
{
  uint8_t stat;

  util_sys_inb(KBC_OUT_BUF, &ob_byte);
  util_sys_inb(KBC_STATUS, &stat);

  if ((stat & (KBC_PAR_ERR | KBC_TO_ERR)) == 0) {discard = 0;}
  else {discard = 1;}
}

void (timer_int_handler)() {timer++;}

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

void (kbc_read)()
{
  uint8_t stat;

  while(1) {
    util_sys_inb(KBC_STATUS, &stat);
    if ((stat & KBC_OBF) != 0 && (stat & KBC_AUX) == 0) {
      util_sys_inb(KBC_OUT_BUF, &ob_byte);
      if ((stat & (KBC_PAR_ERR | KBC_TO_ERR)) == 0) {discard = 0;}
      else {discard = 1;}
      return;
    }
    tickdelay(micros_to_ticks(DELAY_US));
  }
}

int (kbc_reset)(){
    kbc_write(KBC_IN,0x20);
    uint8_t command_byte;
    util_sys_inb(KBC_OUT_BUF,&command_byte);
    command_byte = command_byte | BIT(0);
    kbc_write(KBC_IN,0x60);
    kbc_write(KBC_OUT_BUF,command_byte);
    return 0;
}
