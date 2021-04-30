// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <stdint.h>
#include <stdio.h>

// Any header files included below this line should have been created by you

#include "mouse.h"
#include "util.h"

extern struct packet pp;
extern uint64_t count_time;

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need/ it]
  lcf_trace_calls("/home/lcom/labs/lab4/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab4/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}


int (mouse_test_packet)(uint32_t cnt) {
  
  uint32_t counter = 0;
  int ipc_status, r;
  message msg;
  uint8_t hook_id = 2, irq_set = BIT(hook_id);

  if (new_mouse_enable_data_reporting() != OK)
    return 1;

  if (mouse_subscribe_int(&hook_id) != OK)
    return 2;

  while(counter/3 != cnt)
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
            mouse_ih();
            counter++;

            if (counter%3 == 0)
              mouse_print_packet(&pp);
          }

          break;

        default:
          break;
      }	
    }
  }

  if (mouse_unsubscribe_int() != OK)
    return 3;

  if (mouse_disable_data_reporting() != OK)
    return 4;

  return 0;  
}

int (mouse_test_async)(uint8_t idle_time) {
  count_time = 0;
  uint32_t counter = 0;
  int ipc_status, r;
  message msg;
  uint8_t hook_id_timer = 0, hook_id_mouse = 2, irq_set_timer = BIT(hook_id_timer), irq_set_mouse = BIT(hook_id_mouse), frequency = (uint8_t) sys_hz();

  if (new_mouse_enable_data_reporting() != OK)
    return 1;

  if (timer_subscribe_int(&hook_id_timer) != OK)
    return 2;

  if (mouse_subscribe_int(&hook_id_mouse) != OK)
    return 3;
  
  while(count_time/frequency != idle_time)
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
            timer_int_handler();
            //printf("%d", count_time/frequency); //funciona se idle_time < 5 pq?????????
          }

          if (msg.m_notify.interrupts & irq_set_mouse)
          {
            mouse_ih();
            counter++;
            count_time = 0;

            if (counter%3 == 0)
              mouse_print_packet(&pp);
          }
          break;

        default:
          break;
      }	
    }
  }

  if (timer_unsubscribe_int() != OK)
    return 4;

  if (mouse_unsubscribe_int() != OK)
    return 5;

  if (mouse_disable_data_reporting() != OK)
    return 6;

  return 0;  
}

int (mouse_test_gesture)(uint8_t x_len, uint8_t tolerance) {
  uint32_t counter = 0;
  int ipc_status, r;
  message msg;
  uint8_t hook_id = 2, irq_set = BIT(hook_id);
  int done = 0;

  if (new_mouse_enable_data_reporting() != OK)
    return 1;

  if (mouse_subscribe_int(&hook_id) != OK)
    return 2;

  while(!done)
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
            mouse_ih();
            counter++;
            if (counter%3 == 0) {
              //mouse_print_packet(&pp);
              done = find_symbol(x_len, tolerance);
            }
          }

          break;

        default:
          break;
      }	
    }
  }

  if (mouse_unsubscribe_int() != OK)
    return 3;

  if (mouse_disable_data_reporting() != OK)
    return 4;

  return 0; 
}

int (mouse_test_remote)(uint16_t period, uint8_t cnt)
{
  int packet_counter = 0;
  uint8_t stat = 0, data;
  bool packet_done = false;
  bool called_packet = false;
  
  while(packet_counter/3 != cnt)
  {
    while (!packet_done)
    {
      if (!called_packet) {
        kbc_write(KBC_IN, MOUSE_WRITE_COM);
        kbc_write(KBC_OUT_BUF, 0xEB);
        called_packet = true;
      }
      util_sys_inb(KBC_STATUS, &stat);

      if ((stat & KBC_AUX) && (stat & KBC_OBF_FULL))
      {
        util_sys_inb(KBC_OUT_BUF, &data);

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
          packet_done = true;
          break;
        }
        called_packet = false;
        tickdelay(micros_to_ticks(period));
      }
    }

    packet_done = false;
    mouse_print_packet(&pp);
  }

  if (mouse_disable_data_reporting() != OK)
    return 6;

  kbc_write(KBC_IN, KBC_WRITE_COM);
  kbc_write(KBC_OUT_BUF, minix_get_dflt_kbc_cmd_byte());

  return 0;
}
