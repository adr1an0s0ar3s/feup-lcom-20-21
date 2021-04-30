#include <lcom/lcf.h>
#include <lcom/lab2.h>

#include <stdbool.h>
#include <stdint.h>

int counter;

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab2/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab2/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(timer_test_read_config)(uint8_t timer, enum timer_status_field field)
{
  uint8_t st;
  int test = timer_get_conf(timer, &st);

  if (test != OK)
    return test;
  
  return timer_display_conf(timer, st, field);
}

int(timer_test_time_base)(uint8_t timer, uint32_t freq) 
{
  // Interesting frequencies: 60 Hz - Default Speed; 4660 Hz should activate MSB_only mode!

  return timer_set_frequency(timer, freq);
}

int(timer_test_int)(uint8_t time)
{
  int ipc_status, r;
  message msg;
  uint8_t hook_id = 0, irq_set = BIT(hook_id);

  counter = 0;

  if (timer_subscribe_int(&hook_id) != OK)
    return 1;

  while(counter/60 != time)
  {
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) { 
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) { /* received notification */
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE: /* hardware interrupt notification */				
          if (msg.m_notify.interrupts & irq_set) { /* subscribed interrupt */
            timer_int_handler();
            if (counter % 60 == 0)
              timer_print_elapsed_time();
          }
          break;
      }
    }
  }

  if (timer_unsubscribe_int() != OK)
    return 2;

  return 0;
}
