#ifndef SYNTH_H_
#define SYNTH_H_

#include <xc.h>

//#define MAX_ACTIVE_NOTES 6

#define SID_OPL_RST_OE_DIR  TRISDbits.TRISD1
#define SID_OPL_RST_OE      LATDbits.LATD1

extern void Synth_init(void);
extern void Synth_Task(void);

unsigned char Decode_UART_cmd(void);
unsigned char Decode_UART_SID(void);
unsigned char Decode_UART_OPL3(void);
void Decode_UART_midi(void);



#define DEBUG_LED1_DIR TRISBbits.TRISB9
#define DEBUG_LED1     LATBbits.LATB9
#define DEBUG_LED2_DIR TRISBbits.TRISB10
#define DEBUG_LED2     LATBbits.LATB10

#endif