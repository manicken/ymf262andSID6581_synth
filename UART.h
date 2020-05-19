
#ifndef UART_H_
#define UART_H_

#define UART1_TX_DIR TRISCbits.TRISC13
#define UART1_RX_DIR TRISCbits.TRISC14
#define UART2_TX_DIR TRISFbits.TRISF5
#define UART2_RX_DIR TRISFbits.TRISF4

#define UART1_TX_PIN LATCbits.LATC13
#define UART2_TX_PIN LATFbits.LATF5

#define UART1_TXIE IEC0bits.U1TXIE
#define UART1_RXIE IEC0bits.U1RXIE
#define UART1_TXIF IFS0bits.U1TXIF
#define UART1_RXIF IFS0bits.U1RXIF

#define UART2_TXIE IEC1bits.U2TXIE
#define UART2_RXIE IEC1bits.U2RXIE
#define UART2_TXIF IFS1bits.U2TXIF
#define UART2_RXIF IFS1bits.U2RXIF

#define UART1_RXIP IPC2bits.U1RXIP
#define UART1_TXIP IPC2bits.U1TXIP
#define UART2_RXIP IPC6bits.U2RXIP
#define UART2_TXIP IPC6bits.U2TXIP

#define MIDI_MSG_FIFO_SIZE 64
#define CMD_MSG_BUFF_SIZE 16

typedef union BYTE_FLAGS
{
    unsigned char val;
    struct
    {
        unsigned MIDI_MSG_RX:1;
        unsigned CMD_MSG_RX:1;
        unsigned b2:1;
        unsigned b3:1;
        unsigned b4:1;
        unsigned b5:1;
        unsigned b6:1;
        unsigned b7:1;
    };
} BYTE_FLAGS;
extern BYTE_FLAGS uart_flags;

extern unsigned char midiMsgFifo[MIDI_MSG_FIFO_SIZE][3];
extern unsigned char midiMsgFifoWritePointer;
extern unsigned char midiMsgFifoReadPointer;
extern unsigned char midiMsgFifoDataCount;

extern unsigned char midiUartRxRawByte;
extern unsigned char midiUartRxRawBuff[3];
extern unsigned char midiUartRxRawCount;

extern unsigned char cmdUartRxByte;
extern unsigned char cmdUartRxBuff[CMD_MSG_BUFF_SIZE];

extern unsigned char cmdUartRxCount;
extern unsigned char cmdUartRxLength;

//extern void Synth_UART_MIDI_Interrupt_Task(void);
//extern void Synth_UART_CMD_Interrupt_Task(void);

extern void UARTs_init(void);
extern void UART1_send(unsigned char data);
extern void UART2_send(unsigned char data);
extern void UART2_sendText(char *data);
extern void UART_sendArrayAsHex(const unsigned char *data, unsigned char lenght);
extern void UART2_sendHex(unsigned char data);
extern void UART2_sendNibbleHex(unsigned char nibble);

extern unsigned char convAsciiHex(unsigned char *data);
extern unsigned char convOneAsciiHex(unsigned char data);

#endif