
#include <xc.h>
#include "SID6581.h"
#include "SPI.h"



unsigned char sid_dataTmp = 0x00;
unsigned char sid_addrCtrlTmp = 0x00;

unsigned char mode_vol_regTmp = 0x00;
unsigned char osc_control_regTmp[6];

unsigned char oscOnCount = 0;

void SID_HWinit(void) // this init involves both hardware and software
{
    SPI_init();
    SID_SERIAL_LATCH = 0; // SID serial latch
    SID_SERIAL_LATCH_DIR = 0; // SID serial latch as output
    
    sid_addrCtrlTmp = 0xFF;
    sid_dataTmp = 0x00;
    SID_SPI_WriteData();
}

void SID_SWinit(void)
{
    unsigned char i = 0;
    // Reset SID chip
    SID_SwReset();
    
    for (i = 0; i < 6; i++)
    {
        osc_control_regTmp[i] = 0x00;
    }
}

void SID_SetDefault_Instrument()
{
    // Synth sw development prototype states init
    SID_setVolume(15, SID_BOTH_CHIPS);

    SID_setOscWaveType(SID_OSC_WAVETYPE_SQUARE, 0);
    SID_setOscWaveType(SID_OSC_WAVETYPE_SQUARE, 1);
    SID_setOscWaveType(SID_OSC_WAVETYPE_SQUARE, 2);
    SID_setOscWaveType(SID_OSC_WAVETYPE_SQUARE, 3);
    SID_setOscWaveType(SID_OSC_WAVETYPE_SQUARE, 4);
    SID_setOscWaveType(SID_OSC_WAVETYPE_SQUARE, 5);

    SID_write(0x03, 0x06, SID_BOTH_CHIPS); // OSC1 pwm (square wave) highReg
    SID_write(0x0A, 0x06, SID_BOTH_CHIPS); // OSC2 pwm (square wave) highReg
    SID_write(0x11, 0x06, SID_BOTH_CHIPS); // OSC3 pwm (square wave) highReg
    
    SID_write(0x05, 0x09, SID_BOTH_CHIPS); // OSC1 attack/decay
    SID_write(0x06, 0x00, SID_BOTH_CHIPS); // OSC1 sustain
    SID_write(0x0C, 0x09, SID_BOTH_CHIPS); // OSC2 attack/decay
    SID_write(0x0D, 0x00, SID_BOTH_CHIPS); // OSC2 sustain
    SID_write(0x13, 0x09, SID_BOTH_CHIPS); // OSC3 attack/decay
    SID_write(0x14, 0x00, SID_BOTH_CHIPS); // OSC3 sustain
}

void SID_SPI_WriteData(void)
{
    SID_SERIAL_LATCH = 0;
    SPI_Write(sid_dataTmp);
    SPI_Write(sid_addrCtrlTmp);
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    SID_SERIAL_LATCH = 1; // latch in the written serial data
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    SID_SERIAL_LATCH = 0;
}

void SID_setOscWaveType(unsigned char type, unsigned char osc)
{
    osc_control_regTmp[osc] = (osc_control_regTmp[osc] & 0x0F) | type;
    if (osc < 3)
        SID_write(7 * osc + 4, osc_control_regTmp[osc], SID_LEFT_CHIP);
    else
        SID_write(7 * (osc - 3) + 4, osc_control_regTmp[osc], SID_RIGHT_CHIP);
}

void SID_setNote(unsigned int noteData, unsigned char osc)
{
    if (osc < 3)
    {
        SID_write(7 * osc, (noteData & 0x00FF), SID_LEFT_CHIP); // freq LOW_REG  osc x
        SID_write(7 * osc + 1, (noteData >> 8) & 0x00FF, SID_LEFT_CHIP); // freq HIGH_REG  osc x
    }    
    else
    {
        osc -= 3;
        SID_write(7 * osc, (noteData & 0x00FF), SID_RIGHT_CHIP); // freq LOW_REG  osc x
        SID_write(7 * osc + 1, (noteData >> 8) & 0x00FF, SID_RIGHT_CHIP); // freq HIGH_REG  osc x
    }    
    
    
}

void SID_turnOnNote(unsigned char osc)
{
    osc_control_regTmp[osc] = osc_control_regTmp[osc] | 0x01;
    if (osc < 3)
        SID_write(7 * osc + 4, osc_control_regTmp[osc], SID_LEFT_CHIP);
    else
        SID_write(7 * (osc - 3) + 4, osc_control_regTmp[osc], SID_RIGHT_CHIP);
    oscOnCount++;
    
}

void SID_turnOffNote(unsigned char osc)
{
    osc_control_regTmp[osc] = osc_control_regTmp[osc] & 0xFE;
    if (osc < 3)
        SID_write(7 * osc + 4, osc_control_regTmp[osc], SID_LEFT_CHIP);
    else
        SID_write(7 * (osc - 3) + 4, osc_control_regTmp[osc], SID_RIGHT_CHIP);
    oscOnCount--;
}

void SID_setVolume(unsigned char value, unsigned char chipSelect)
{
    mode_vol_regTmp = (mode_vol_regTmp & 0xF0) | value;
    SID_write(0x18, mode_vol_regTmp, chipSelect);
}

void SID_SwReset()
{
    unsigned char addr = 0;
    for (addr = 0; addr < 33; addr++)
    {
        SID_write(addr, 0x00, SID_BOTH_CHIPS);
    }    
}

void SID_write(unsigned char addr, unsigned char data, unsigned char chipSelect)
{
    sid_dataTmp = data;
    sid_addrCtrlTmp = 0xFF;
    //addrCtrlRegTmp = (addr & 0x1F) + 0xC0; // adress + both chipselect high
    SID_SPI_WriteData();
    SID_waitOneCycle();
    
    //addrCtrlRegTmp = addrCtrlRegTmp & (chipSelect + 0x1F); // chipSelect defines witch one that should be low
    SID_SPI_WriteData();
    SID_waitOneCycle();
    
    //addrCtrlRegTmp = addrCtrlRegTmp | 0xE0; // set both CS#:s high
    SID_SPI_WriteData();
    SID_waitOneCycle();

    if (addr == 0x04)
        osc_control_regTmp[0] = data;
    else if (addr == 0x0B)
        osc_control_regTmp[1] = data;
    else if (addr == 0x12)
        osc_control_regTmp[2] = data;
    else if (addr == 0x18)
        mode_vol_regTmp = data;
}

void SID_HwReset(void)
{
    //PORTEbits.RE2 = 0; // RESET#
    //SID_waitXcycles(12);
    //PORTEbits.RE2 = 1; // RESET#
}

void SID_waitXcycles(unsigned long count)
{
    while (count)
    {
        SID_waitOneCycle();
        count--;
    }    
}
    
void SID_waitOneCycle(void)
{
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
}
// parallell interfacing
    /*
    // init
    // setup PIC ports
    TRISCbits.TRISC1 = 0; // SID clock
    TRISE = 0x08;
    PORTE = 0x05; // RES# = 1, R/W# = 0, CS# = 1
    
    TRISD = 0x00;
    PORTD = 0x00;
    
    TRISA = 0xE0;
    PORTA = 0x00;

    // setup SID clock from (PIC capture and compare with pwm module)
    CCPR2L = 0x06;
    CCP2CON = 0x00;
    
    PR2 = 0x0B; // 12MHz / (11 + 1)
    T2CON = 0x00;
    T2CON = 0x04;
    CCP2CON = 0x0C;
    */
    
    // write data
        /*//LATEbits.LATE0 = 0; // CS#
    SID_waitOneCycle();
    LATEbits.LATE0 = 0; // CS#
    SID_waitOneCycle();
    LATEbits.LATE0 = 1; // CS#
    SID_waitOneCycle();
    
    LATEbits.LATE0 = 0; // CS#
    SID_waitOneCycle();
    LATEbits.LATE0 = 1; // CS#
    SID_waitOneCycle();
*/



void SID_Play_C_accord(void)
{
    SID_write(0x05, 0x0a, SID_LEFT_CHIP); // OSC1a attack/decay
    SID_write(0x06, 0x10, SID_LEFT_CHIP); // OSC1a sustain
    SID_write(0x0C, 0x0a, SID_LEFT_CHIP); // OSC2a attack/decay
    SID_write(0x0D, 0x10, SID_LEFT_CHIP); // OSC2a sustain
    SID_write(0x13, 0x0a, SID_LEFT_CHIP); // OSC3a attack/decay
    SID_write(0x14, 0x10, SID_LEFT_CHIP); // OSC3a sustain
    SID_write(0x13, 0x0a, SID_RIGHT_CHIP); // OSC1b attack/decay
    SID_write(0x14, 0x10, SID_RIGHT_CHIP); // OSC1b sustain

    //SID_setNote(sidNoteData[24],0);
    //SID_setNote(sidNoteData[24+2],1);
    //SID_setNote(sidNoteData[24+4],2);
    //SID_setNote(sidNoteData[24+6],3);
    //SID_setNote(C4, 0);
    //SID_setNote(E4, 1);
    //SID_setNote(G4, 2);
    SID_turnOnNote(0);
    SID_waitXcycles(100000);
    SID_turnOnNote(1);
    SID_waitXcycles(100000);
    SID_turnOnNote(2);
    SID_waitXcycles(100000);
    SID_turnOnNote(3);
    SID_waitXcycles(100000);
    SID_waitXcycles(100000);
    SID_turnOffNote(0);
    SID_turnOffNote(1);
    SID_turnOffNote(2);
    SID_turnOffNote(3);
    SID_waitXcycles(100000);
}

void SID_Play_Demo(void)
{
    SID_write(0x05, 0x0a, SID_LEFT_CHIP); // OSC1 attack/decay
    SID_write(0x06, 0x10, SID_LEFT_CHIP); // OSC1 sustain

    /*SID_setNote(noteData[24],0);
    SID_turnOnNote(0);
    SID_waitXcycles(50000);
    SID_turnOffNote(0);
    SID_waitXcycles(50000);
    SID_turnOnNote(0);
    SID_waitXcycles(50000);
    SID_turnOffNote(0);
    SID_waitXcycles(50000);

    SID_setNote(noteData[24+],0);
    SID_turnOnNote(0);
    SID_waitXcycles(50000);
    SID_turnOffNote(0);
    SID_waitXcycles(50000);
    SID_turnOnNote(0);
    SID_waitXcycles(50000);
    SID_turnOffNote(0);
    SID_waitXcycles(50000);
    
    SID_setNote(A4, 0);
    SID_turnOnNote(0);
    SID_waitXcycles(50000);
    SID_turnOffNote(0);
    SID_waitXcycles(50000);
    SID_turnOnNote(0);
    SID_waitXcycles(50000);
    SID_turnOffNote(0);
    SID_waitXcycles(50000);
    
    SID_setNote(G4, 0);
    SID_turnOnNote(0);
    SID_waitXcycles(50000);
    SID_turnOffNote(0);
    SID_waitXcycles(100000);
    
    SID_setNote(F4, 0);
    SID_turnOnNote(0);
    SID_waitXcycles(50000);
    SID_turnOffNote(0);
    SID_waitXcycles(50000);
    SID_turnOnNote(0);
    SID_waitXcycles(50000);
    SID_turnOffNote(0);
    SID_waitXcycles(50000);
    
    SID_setNote(E4, 0);
    SID_turnOnNote(0);
    SID_waitXcycles(50000);
    SID_turnOffNote(0);
    SID_waitXcycles(50000);
    SID_turnOnNote(0);
    SID_waitXcycles(50000);
    SID_turnOffNote(0);
    SID_waitXcycles(50000);
    
    SID_setNote(D4, 0);
    SID_turnOnNote(0);
    SID_waitXcycles(50000);
    SID_turnOffNote(0);
    SID_waitXcycles(50000);
    SID_turnOnNote(0);
    SID_waitXcycles(50000);
    SID_turnOffNote(0);
    SID_waitXcycles(50000);
    
    SID_setNote(C4, 0);
    SID_turnOnNote(0);
    SID_waitXcycles(50000);
    SID_turnOffNote(0);
    SID_waitXcycles(50000);*/
}