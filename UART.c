
#include <xc.h>
#include "UART.h"

unsigned char midiMsgFifo[MIDI_MSG_FIFO_SIZE][3];
unsigned char midiMsgFifoWritePointer;
unsigned char midiMsgFifoReadPointer;
unsigned char midiMsgFifoDataCount;

unsigned char midiUartRxRawByte;
unsigned char midiUartRxRawBuff[3];
unsigned char midiUartRxRawCount;

unsigned char cmdUartRxByte;
unsigned char cmdUartRxBuff[CMD_MSG_BUFF_SIZE];
unsigned char cmdUartRxCount;
unsigned char cmdUartRxLength;

BYTE_FLAGS uart_flags;


void __attribute__ ((interrupt, auto_psv)) _U2RXInterrupt(void) // Control input
//void Synth_UART_CMD_Interrupt_Task(void)
{
    asm("BTG LATB,#9");
    cmdUartRxByte = U2RXREG;
    UART2_RXIF = 0;
    if (cmdUartRxByte == '[') // startbyte
    {
        cmdUartRxCount = 0;
    }
    else if (cmdUartRxByte == ']') // stopbyte
    {
        uart_flags.CMD_MSG_RX = 1;
        cmdUartRxLength = cmdUartRxCount;
    }
    else
    {
        cmdUartRxBuff[cmdUartRxCount] = cmdUartRxByte;
        if (cmdUartRxCount != (CMD_MSG_BUFF_SIZE-1))
            cmdUartRxCount++;
    }
}

void __attribute__ ((interrupt, auto_psv)) _U1RXInterrupt(void) // MIDI input
//void Synth_UART_MIDI_Interrupt_Task(void)
{
    midiUartRxRawByte = U1RXREG;
    UART1_RXIF = 0;
    if (midiUartRxRawByte & 0x80) // start byte allwas has bit 7 set
    {
        midiUartRxRawBuff[0] = midiUartRxRawByte;
        midiUartRxRawCount = 1;
    }
    else
    {
        midiUartRxRawBuff[midiUartRxRawCount] = midiUartRxRawByte;
        if (midiUartRxRawCount != 2)
            midiUartRxRawCount++;
        else
        {
            midiUartRxRawCount = 0;
            
            midiMsgFifo[midiMsgFifoWritePointer][0] =  midiUartRxRawBuff[0];
            midiMsgFifo[midiMsgFifoWritePointer][1] =  midiUartRxRawBuff[1];
            midiMsgFifo[midiMsgFifoWritePointer][2] =  midiUartRxRawBuff[2];
            
            if (midiMsgFifoWritePointer != (MIDI_MSG_FIFO_SIZE - 1))
                midiMsgFifoWritePointer++;
            else
                midiMsgFifoWritePointer = 0;
            
            midiMsgFifoDataCount++;
            uart_flags.MIDI_MSG_RX = 1;
        }
    }
}

//#define UART_IPEN 

void UARTs_init(void)
{
    UART1_TX_PIN = 1; // default value when uart is disabled
    UART2_TX_PIN = 1; // default value when uart is disabled
    UART1_TXIE = 0;
    UART1_RXIE = 0;
    UART1_TXIF = 0;
    UART1_RXIF = 0;
    
    UART2_TXIE = 0;
    UART2_RXIE = 0;
    UART2_TXIF = 0;
    UART2_RXIF = 0;
    
    UART2_RXIP = 7;
    UART1_RXIP = 6;
    //INTCONbits.GIE = 0;
    INTCON1bits.NSTDIS = 1; // 1 = Interrupt nesting is disabled
    
    U1MODE = 0x0000; // 8bit data no parity 1 stopbit
    U2MODE = 0x0000; // 8bit data no parity 1 stopbit
    U1MODEbits.ALTIO = 1; // 1 = UART communicates using UxATX and UxARX I/O pins
    
    U1STA = 0x0000; //Interrupt flag bit is set when a character is received
    U2STA = 0x0000; //Interrupt flag bit is set when a character is received
    
    UART1_RX_DIR = 1; // RX as input
    UART1_TX_DIR = 0; // TX as output
    UART2_RX_DIR = 1; // RX as input
    UART2_TX_DIR = 0; // TX as output
    
    // baudrate register value calculated by: 30MHz/(16*31250)-1 = 59dec = 0x3B; // 00111011
    // baudrate register value calculated by: 30MHz/(16*15625)-1 = 119dec = 0x77; // 01110111
    // baudrate register value calculated by: 30MHz/(16*38400)-1 = 47.828 ~ 48 => 38265baud => 48 = 32+16 = 00110000 = 0x30 
    U1BRG = 0x003B;
    U2BRG = 0x0030;
        
    //IEC0

    //INTCONbits.PEIE = 1; // peripheral interrups enable
   // INTCONbits.GIE = 1;
    
    midiMsgFifoWritePointer = 0;
    midiMsgFifoReadPointer = 0;
    midiUartRxRawCount = 0;
    
    cmdUartRxCount = 0;

    U1MODEbits.UARTEN = 1;
    U2MODEbits.UARTEN = 1;
    U1STAbits.UTXEN = 1;
    U2STAbits.UTXEN = 1;
    
    UART1_RXIE = 1;
    UART2_RXIE = 1;
    
    //CORCONbits.
}

void UART1_send(unsigned char data)
{
    while (U1STAbits.TRMT == 0)
    {
         asm("NOP");
    }
    U1TXREG = data;
}
void UART2_send(unsigned char data)
{
    U2TXREG = data;
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    while (U2STAbits.TRMT == 0)
    {
         asm("NOP");
    }
}
void UART2_sendText(char *data)
{
    while (*data != 0x00)
    {
        UART2_send(*data);
        data++;
    }
}

void UART_sendArrayAsHex(const unsigned char *data, unsigned char lenght)
{
    while (lenght != 0)
    {
        UART2_sendHex(*data);
        data++;
        lenght--;
    }
}

unsigned char convAsciiHex(unsigned char *data)
{
    return convOneAsciiHex(data[0])*16 + convOneAsciiHex(data[1]);
}
unsigned char convOneAsciiHex(unsigned char data)
{
    if ((data >= '0') && (data <= '9'))
    {
        return data - '0';
    }
    else if ((data >= 'A') && (data <= 'F'))
    {
        return data - ('A' - 10);
    }
    else if ((data >= 'a') && (data <= 'f'))
    {
        return data - ('a' - 10);
    }
    return 0x00;
}

 void UART2_sendHex(unsigned char data)
 {
     UART2_sendNibbleHex((data & 0xF0) >> 4);
     UART2_sendNibbleHex(data & 0x0F);     
 }
 void UART2_sendNibbleHex(unsigned char nibble)
 {
     nibble = nibble & 0x0F;
     if (nibble < 0x0A) nibble += '0';
     else nibble += ('A' - 10);
     UART2_send(nibble);
 }