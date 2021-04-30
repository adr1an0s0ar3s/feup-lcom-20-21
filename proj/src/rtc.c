#include <lcom/lcf.h>
#include <lcom/liblm.h>
#include <lcom/proj.h>

#include <stdbool.h>
#include <stdint.h>

#include "rtc.h"
#include "util.h"

static int hook_id_rtc;

int (rtc_subscribe_int)(uint8_t *bit_no)
{
    hook_id_rtc = *bit_no;
    
    if (sys_irqsetpolicy(RTC_IRQ, IRQ_REENABLE, &hook_id_rtc) != OK)
        return 1;

    return 0;
}

int (rtc_unsubscribe_int)()
{  
    if (sys_irqrmpolicy(&hook_id_rtc) != OK)
        return 1;
    
    return 0;
}

void (activate_alarm)()
{
    uint8_t reg;
    sys_outb(RTC_ADDR_REG, 11);
    util_sys_inb(RTC_DATA_REG, &reg);
    reg = reg | BIT(5);
    sys_outb(RTC_ADDR_REG, 11);
    sys_outb(RTC_DATA_REG, reg);
}

void (deactivate_alarm)(){
    uint8_t reg;
    sys_outb(RTC_ADDR_REG, 11);
    util_sys_inb(RTC_DATA_REG, &reg);
    reg = reg & 0xDF;
    sys_outb(RTC_ADDR_REG, 11);
    sys_outb(RTC_DATA_REG, reg);
}

void (handle_alarm_int)(){
    uint8_t cause;
    sys_outb(RTC_ADDR_REG, 12);
    util_sys_inb(RTC_DATA_REG, &cause);
}

uint8_t (bcd_to_bin)(uint8_t n){
    uint8_t Result = 0;
    uint8_t Multiplier = 1;
    uint8_t Digit = 0;
    while ( n > 0 )
    {
        Digit= n & 0x0F;
        n >>= 4;
        Result += Multiplier * Digit;
        Multiplier *= 10;
    }
    return Result;
}

uint8_t (bin_to_bcd)(uint8_t n){
    uint8_t bcd = 0;
    int shift = 0;
    while (n>0){
        bcd |= (n % 10) << (shift++ << 2);
        n /= 10;
    }
    return bcd;
}

void (readDate)(uint8_t *dia, uint8_t *mes, uint8_t *ano, uint8_t *horas, uint8_t *minutos, uint8_t *segundos)
{
    uint8_t res1; //= 0 ??
    sys_outb(RTC_ADDR_REG, 7);
    util_sys_inb(RTC_DATA_REG, dia);
    sys_outb(RTC_ADDR_REG, 8);
    util_sys_inb(RTC_DATA_REG, mes);
    sys_outb(RTC_ADDR_REG, 9);
    util_sys_inb(RTC_DATA_REG, ano);
    sys_outb(RTC_ADDR_REG, 4);
    util_sys_inb(RTC_DATA_REG, horas);
    sys_outb(RTC_ADDR_REG, 2);
    util_sys_inb(RTC_DATA_REG, minutos);
    sys_outb(RTC_ADDR_REG, 0);
    util_sys_inb(RTC_DATA_REG, segundos);
    sys_outb(RTC_ADDR_REG, 11);
    util_sys_inb(RTC_DATA_REG, &res1);
    if (!(res1 & 0x04)){
        *dia = bcd_to_bin(*dia);
        *mes = bcd_to_bin(*mes);
        *ano = bcd_to_bin(*ano);
        *horas = bcd_to_bin(*horas);
        *minutos = bcd_to_bin(*minutos);
        *segundos = bcd_to_bin(*segundos);
    }
}

void (setAlarm)(){
    uint8_t day,month,year,hours,minutes,seconds;
    uint8_t res; //= 0 ??
    readDate(&day, &month, &year, &hours, &minutes, &seconds);

    seconds += 60;
    if (seconds >= 60) {minutes++; seconds -= 60;}
    if (minutes >= 60) {hours++; minutes -= 60;}
    if (hours >= 24) {hours = 0;}

    sys_outb(RTC_ADDR_REG, 11);
    util_sys_inb(RTC_DATA_REG, &res);
    if (!(res & 0x04)){
        hours = bin_to_bcd(hours);
        minutes = bin_to_bcd(minutes);
        seconds = bin_to_bcd(seconds);
    }

    sys_outb(RTC_ADDR_REG, 1);
    sys_outb(RTC_DATA_REG, seconds);
    sys_outb(RTC_ADDR_REG, 3);
    sys_outb(RTC_DATA_REG, minutes);
    sys_outb(RTC_ADDR_REG, 5);
    sys_outb(RTC_DATA_REG, hours);
}
