#include <lcom/lcf.h>
#include <lcom/liblm.h>
#include <lcom/proj.h>

#include <stdbool.h>
#include <stdint.h>

#include "serial.h"
#include "util.h"

static int hook_id_serial;
static bool thr_empty;

int (serial_subscribe_int)(uint8_t *bit_no)
{
    hook_id_serial = *bit_no;
    
    if (sys_irqsetpolicy(SER_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &hook_id_serial) != OK)
        return 1;

    return 0;
}

int (serial_unsubscribe_int)()
{  
    if (sys_irqrmpolicy(&hook_id_serial) != OK)
        return 1;

    // Configuring Interrupts
    if (sys_outb(SER_PORT + SER_IER, 0) != OK)
        return 2;
    
    return 0;
}

int (serial_config)()
{
    // Setting Config and Accessing DLAB
    if (sys_outb(SER_PORT + SER_LCR, LCR_8BITCHAR | LCR_1STOPBIT | LCR_NOPARITY | LCR_DLAB) != OK)
        return 1;
    
    // Setting Bit Rate - 19200
    if (sys_outb(SER_PORT + SER_DLL, 6) != OK)
        return 2;
    if (sys_outb(SER_PORT + SER_DLM, 0) != OK)
        return 3;

    // Disabling DLAB
    if (sys_outb(SER_PORT + SER_LCR, LCR_8BITCHAR | LCR_1STOPBIT | LCR_NOPARITY) != OK)
        return 4;

    // Disabling FIFOS
    if (sys_outb(SER_PORT + SER_FCR, 0) != OK)
        return 5;

    // Configuring Interrupts
    if (sys_outb(SER_PORT + SER_IER, IER_AV_DATA_INT | IER_THR_EMPTY_INT | IER_ERR_INT) != OK)
        return 6;

    thr_empty = true;
    
    return 0;
}

int (serial_send)(uint8_t packet)
{
    if (thr_empty) printf("THR: True\n");
    else printf("THR: False\n");

    if (!thr_empty) return 1;

    if (sys_outb(SER_PORT + SER_THR, packet) != OK)
        return 2;

    thr_empty = false;
    
    return 0;
}

int (serial_ih)()
{
    uint8_t iir, data;
    if (util_sys_inb(SER_PORT + SER_IIR, &iir) != OK)
        return SER_ERR;

    printf("INT IN SERIAL_IH: %d\n", iir);
    
    switch (iir) {
    // In case something changed in the LSR
    case IIR_LSR_CHANGE_INT:
        printf("serial_ih(): Something went wrong!\n");
        return SER_ERR;
    // Data has arrived
    case IIR_AV_DATA_INT:
        if (util_sys_inb(SER_PORT + SER_RBR, &data) != OK)
            return SER_ERR;
        printf("SERIAL PASSED DATA: %d\n", data);
        return data;
    // The Character has been sent correctly and now the THR is empty
    case IIR_THR_EMPTY_INT:
        thr_empty = true;
        return SER_THR_EMPTY;
    }

    return SER_ERR;
}
