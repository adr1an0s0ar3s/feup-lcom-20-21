// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>
#include <lcom/liblm.h>
#include <lcom/proj.h>

#include <stdbool.h>
#include <stdint.h>

/** @defgroup proj proj
 * @{
 *
 * Main Loop
 */

// Any header files included below this line should have been created by you

#include "keyboard.h"
#include "mouse.h"
#include "graph.h"
#include "serial.h"
#include "menu.h"
#include "game.h"
#include "rtc.h"

extern uint8_t keys[2];
extern struct packet pp;
extern uint8_t player;

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/proj/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/proj/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

static int print_usage() {
  printf("Usage: Absolutely Nothing\n");
  return 1;
}

int (connect_four)()
{
  // Initializing Graphics Mode
  if (vg_init(0x115) == NULL) {
    printf("vg_init(): Error\n");
    return 1;
  }

  // Subscriving Timer Interrupts and Configuring Timer
  timer_set_frequency(60, 0);
  uint8_t hook_id_timer = 0, irq_set_timer = BIT(hook_id_timer);
  if (timer_subscribe_int(&hook_id_timer) != OK) {
    printf("timer_subscribe_int(): Error\n");
    return 2;
  }

  // Subscriving Keyboard Interrupts
  uint8_t hook_id_keyboard = 1, irq_set_keyboard = BIT(hook_id_keyboard);
  if (kbc_subscribe_int(&hook_id_keyboard) != OK) {
    printf("kbc_subscribe_int(): Error\n");
    return 3;
  }

  // Subscriving Mouse Interrupts and Configuring Mouse
  enable_data_reporting();
  uint8_t hook_id_mouse = 2, irq_set_mouse = BIT(hook_id_mouse);
  if (mouse_subscribe_int(&hook_id_mouse) != OK) {
    printf("mouse_subscribe_int(): Error\n");
    return 4;
  }

  // Subscriving RTC Interrupts
  uint8_t hook_id_rtc = 3, irq_set_rtc = BIT(hook_id_rtc);
  //activate_alarm();
  if (rtc_subscribe_int(&hook_id_rtc) != OK) {
    printf("rtc_subscribe_int(): Error\n");
    return 5;
  }

  // Subscriving Serial Interrupts and Configuring Serial Port
  serial_config();
  uint8_t hook_id_serial = 4, irq_set_serial = BIT(hook_id_serial);
  if (serial_subscribe_int(&hook_id_serial) != OK) {
    printf("serial_subscribe_int(): Error\n");
    return 6;
  }

  //------------------ Game Loop ----------------------

  int ipc_status, r;
  message msg;
  uint8_t state = 0, game_action = 0;
  bool mouseMoved = false, done = false;
  bool host = true, established_connection = false, request_sent = false;

  startMenu();

  while (!done)
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
            if (state == 0 && mouseMoved)
            {
              switch (showMenu(pp.delta_x, pp.delta_y, pp.lb))
              {
              // Button 1 Pressed - Initiate Local Game
              case 1:
                initiate_game();
                activate_alarm();
                setAlarm();
                state = 2;
                break;
              // Button 2 Pressed - Initiate Serial Port Game  
              case 2:
                if (host) showOnlineWaitingMenu();
                state = 1;
                break;
              // Button 3 Pressed - Show Instructions
              case 3:
                showInstructionsMenu();
                state = 3;
                break;
              // Button 4 Pressed - Exit  
              case 4:
                done = true;
                break;
              }

              mouseMoved = false;
            }
            else if (state == 1)
            {
              // Code 4 is for starting synchronizing PCS via UART, Code 5 is the Confirmation
              if (!request_sent)
              {
                if (host) {
                  if (serial_send(SER_REQUEST) == OK)
                    request_sent = true;
                }
                else if (!host)
                {
                  if (serial_send(SER_CONFIRM) == OK)
                    request_sent = true;
                }
              }

              // If it's not the host, since the Confirmation has already been sent, start the game
              if (!host) established_connection = true;

              if (established_connection) {
                initiate_game();
                state = 2;
              }
            }
            else if (state == 2)
            {
              switch (game_action)
              {
              case 1:
                if (column_select_animation(true)) game_action = 0;
                break;
              case 2:
                if (column_select_animation(false)) game_action = 0;
                break;
              case 3:
                if (chip_falling_animation())
                {
                  if (checkGameOver())
                    game_action = 5;
                  else
                  {
                    nextPlay();
                    if (!established_connection) setAlarm();
                    game_action = 0;
                  }
                }
                break;
              case 4:
                if (random_play_column_select_animation()) game_action = 3;
                break;
              case 5:
                if (game_over())
                {
                  if (established_connection) 
                  {
                    established_connection = false;
                    host = true;
                    request_sent = false;
                  }
                  else deactivate_alarm();

                  startMenu();
                  state = 0;
                  game_action = 0;
                }
                break;
              }
            }
          }

          if (msg.m_notify.interrupts & irq_set_keyboard)
          {
            if (state == 0 && keyboard_ih() && keys[0] == 1)
              done = true;
            else if (state == 1 && keyboard_ih() && keys[0] == 1)
            {
              serial_send(SER_CANCEL_REQUEST);
              request_sent = false;
              startMenu();
              state = 0;
            }
            else if (state == 2 && keyboard_ih())
            {
              if (game_action == 0)
              {
                if (player == 1 && (established_connection ? host : true))
                {
                  // Enter
                  if (keys[0] == 0x1C && validPlay())
                    game_action = 3;

                  // Left Arrow
                  if (keys[0] == 0xE0 && keys[1] == 0x4B)
                    game_action = 1;

                  // Right Arrow
                  if (keys[0] == 0xE0 && keys[1] == 0x4D)
                    game_action = 2;
                }

                // Space for player two to play if middle button is non-existant
                if (player == 2 && keys[0] == 0x39 && validPlay() && (established_connection ? !host : true))
                  game_action = 3;

                if (!established_connection && game_action != 0) setAlarm();

                if (established_connection && game_action != 0) {printf("Game_Action: %d\n", game_action); serial_send(game_action);}
              }

              // ESC Key
              if (keys[0] == 1)
              {
                if (established_connection)
                {
                  serial_send(SER_EXIT);
                  established_connection = false;
                  host = true;
                  request_sent = false;
                }
                else deactivate_alarm();
                startMenu();
                state = 0;
              }
            }
            else if (state == 3 && keyboard_ih() && keys[0] == 1)
            {
              startMenu();
              state = 0;
            }
          }

          if (msg.m_notify.interrupts & irq_set_mouse)
          {
            if (state == 0 && mouse_interrupt_handler())
              mouseMoved = true;
            else if (state == 1)
              mouse_interrupt_handler();
            else if (state == 2 && mouse_interrupt_handler() && player == 2 && game_action == 0 && (established_connection ? !host : true))
            {
              // Left Button
              if (pp.lb)
                game_action = 1;

              // Right Button
              if (pp.rb)
                game_action = 2;

              // Middle Button
              if (pp.mb && validPlay())
                game_action = 3;

              if (!established_connection && game_action != 0) setAlarm();

              if (established_connection && game_action != 0) serial_send(game_action);  
            }
            else if (state == 3)
              mouse_interrupt_handler();
          }

          if (msg.m_notify.interrupts & irq_set_rtc)
          {
            handle_alarm_int();
            randomPlay();
            game_action = 4;
          }

          if (msg.m_notify.interrupts & irq_set_serial)
          {
            int data = serial_ih();

            switch (data)
            {
            case SER_ERR:
              return 11;
              break;
            case SER_REQUEST:
              host = false;
              break;
            case SER_CONFIRM:
              established_connection = true;
            case SER_THR_EMPTY:
              break;
            case SER_CANCEL_REQUEST:
              host = true;
              break;
            case SER_EXIT:
              established_connection = false;
              host = true;
              request_sent = false;
              startMenu();
              state = 0;
              break;
            default:
              game_action = data;
              break;
            }

            printf("%d\n", data);
          }

          break;
      }	
    }
  }

  // Unsuscribing Timer Interrupts
  if (timer_unsubscribe_int() != OK) {
    printf("timer_unsubsribe_int(): Error\n");
    return 7;
  }

  // Unsuscribing Keyboard Interrupts
  if (kbc_unsubscribe_int() != OK) {
    printf("kbc_unsubsribe_int(): Error\n");
    return 8;
  }

  // Unsuscribing Mouse Interrupts
  disable_data_reporting();
  if (mouse_unsubscribe_int() != OK) {
    printf("mouse_unsubsribe_int(): Error\n");
    return 9;
  }

  //Unsubscribing RTC Interrupts
  if (rtc_unsubscribe_int() != OK) {
    printf("rtc_unsubsribe_int(): Error\n");
    return 10;
  }

  // Unsuscribing Serial Interrupts
  if (serial_unsubscribe_int() != OK) {
    printf("serial_unsubsribe_int(): Error\n");
    return 11;
  }
  
  // Disabling Video Mode
  if (vg_exit() != OK) {
    printf("vg_exit(): Error\n");
    return 12;
  }

  return 0;
}

int(proj_main_loop)(int argc, char *argv[])
{
  if (argc != 0)
    return print_usage();

  return connect_four();
}
