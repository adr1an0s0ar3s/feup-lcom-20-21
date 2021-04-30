#include <lcom/lcf.h>

#include <lcom/lab3.h>

#include <stdbool.h>
#include <stdint.h>

#include "i8042.h"
#include "keyboard.h"

extern uint8_t ob_byte, discard;
extern int count, timer;

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab3/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab3/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(kbd_test_scan)() {
  
  int ipc_status, r, counter = 0;
  message msg;
  uint8_t hook_id = 0, irq_set = BIT(hook_id), bytes[3];

  if (kbc_subscribe_int(&hook_id) != OK)
    return 1;

  while(ob_byte != 0x81)
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
          {
            kbc_ih();

            if (discard)
              return 2;

            bytes[counter] = ob_byte;

            if  (ob_byte != 0xE0) {
              kbd_print_scancode((~ob_byte & KBC_MSBIT) >> 7, counter+1, bytes);
              counter = 0;
            }
            else
              counter++;
          }

          break;

        default:
          break;
      }	
    }
  }

  if (kbc_unsubscribe_int() != OK)
    return 1;  

  kbd_print_no_sysinb(count);  

  return 0;
}

int(kbd_test_poll)()
{
  uint8_t bytes[3];
  int counter = 0;

  while(ob_byte != 0x81)
  {
    kbc_read();

    if (discard)
      return 1;

    bytes[counter] = ob_byte;

    if  (ob_byte != 0xE0)
    {
      kbd_print_scancode((~ob_byte & KBC_MSBIT) >> 7, counter+1, bytes);
      counter = 0;
    }
    else
      counter++;
  }
  
  kbd_print_no_sysinb(count);
  kbc_reset();
  return 0;
}

int(kbd_test_timed_scan)(uint8_t n)
{
  int ipc_status, r, counter = 0;
  message msg;
  uint8_t hook_id = 1, irq_set = BIT(hook_id);
  uint8_t timer_hook_id = 0, timer_irq_set = BIT(timer_hook_id);
  uint8_t bytes[3];

  if (kbc_subscribe_int(&hook_id) != OK)
    return 1;

  if (timer_subscribe_int(&timer_hook_id) != OK)
    return 2;

  while((ob_byte != 0x81) && (timer/60 != n))
  {
    if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) { 
        printf("driver_receive failed with: %d", r);
        continue;
    }

    if (is_ipc_notify(ipc_status))
    {
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:

          if (msg.m_notify.interrupts & timer_irq_set) {timer_int_handler();}

          if (msg.m_notify.interrupts & irq_set)
          {
            timer = 0;
            kbc_ih();

            if (discard)
              return 3;

            bytes[counter] = ob_byte;

            if  (ob_byte != 0xE0) {
              kbd_print_scancode((~ob_byte & KBC_MSBIT) >> 7, counter+1, bytes);
              counter = 0;
            }
            else
              counter++;
          }
                
          break;

        default:
          break;
      }	
    }
  }

  if (kbc_unsubscribe_int() != OK)
    return 4;

  if (timer_unsubscribe_int() != OK)
    return 5;

  kbd_print_no_sysinb(count);  

  return 0;
}
