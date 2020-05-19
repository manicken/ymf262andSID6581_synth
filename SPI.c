
#include <xc.h>
#include "SPI.h"

static unsigned char initialized = 0;

#define SSPSTAT SSP1STAT
#define SSPCON1 SSP1CON1
#define SSPBUF SSP1BUF
#define SSPIF PIR3bits.SSP1IF

void SPI_init(void)
{
    if (initialized)
        return;
    SPI1STAT = 0x0000;
    SPI1CON = 0x0000;
    
    SPI1CONbits.MSTEN = 1; // 1 = Master mode
    SPI1CONbits.PPRE = 0; // 1 = Primary prescale 16:1
    SPI1CONbits.SPRE = 7; // 7 = Secondary prescale 1:1
    SPI1CONbits.MODE16 = 0; // 0 = Communication is byte-wide (8 bits)
    // total prescale = 32 => @30Mhz Fcy = 0.9375Mhz
    SPI1CONbits.CKE = 1; // 1 = Serial output data CHANGES on transition from active clock state to Idle clock state (see bit 6) 
                         // (i.e. the data transmit on transition from Idle clock state to active clock state)
    
    TRISFbits.TRISF2 = 1; // SDI input as input
    TRISFbits.TRISF6 = 0; // SCK as output
    TRISFbits.TRISF3 = 0; // SDO as output
    
    
    //SPI1STAT = 0x0000;
    //SPI1CON = 0x2100;
    
    SPI1STATbits.SPIEN = 1; // 1 = Enables module and configures SCKx, SDOx, SDIx and SSx as serial port pins
    
    initialized = 1;
}    

void SPI_Write(unsigned int data)
{
    unsigned int dummyRead = 0x00;
    SPI1BUF = data;
    asm("NOP");
    while (!SPI1STATbits.SPIRBF)
    {
        asm("NOP");
    }
    dummyRead = SPI1BUF;
    //SSPIF = 0;
}

unsigned int SPI_Read(void)
{
    /*SPI1BUF = 0x0000; // dummy
    while (!SPI1STATbits.SPIRBF)
    {
        asm("NOP");
    }
    //SSPIF = 0;
    return SPI1BUF;*/
    return 0;
}        