#include "main.h"
#include <xc.h>
#include "Synth.h"

void main(void)
{
    PIC_init();
    Synth_init();

    while(1)
    {
        asm("BTG LATB,#10");
        asm("BTG LATB,#10");
        //if ((LATB & 0x0200) == 0x0200)
        //    DEBUG_LED2 = 0;
        //else
        //    DEBUG_LED2 = 1;
        Synth_Task();
    }    
}

void PIC_init(void)
{
    ADPCFG = 0xFFFF; // all digital I/O   

    //LATCbits.LATC0 = 0;
    //TRISCbits.TRISC0 = 0; // debug
}

//void __attribute__ ((interrupt, auto_psv)) _U1RXInterrupt(void) // MIDI input
//{
//    Synth_UART_MIDI_Interrupt_Task();
//}  
//void __attribute__ ((interrupt, auto_psv)) _U2RXInterrupt(void) // Control input
//{
//    Synth_UART_CMD_Interrupt_Task();
//} 