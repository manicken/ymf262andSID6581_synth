
#include <xc.h>
#include "OPL3.h"
#include "SPI.h"
#include "instruments.h"

#define MAX_ACTIVE_NOTES 18

 unsigned char opl3_modeInAscii = OPL3_2OP_MODE_IN_ASCII;
 unsigned char opl3_max_notes = OPL3_2OP_MAX_NOTES;
 unsigned char opl3_max_notes_half = OPL3_2OP_MAX_NOTES/2;
 

const unsigned char opl_regOfs[2][9] = {  
	//+------------+--------------------------- -----------------------------------+
    //| Channel    | 0  1  2  3  4  5  6  7  8   9  10  11  12  13  14  15  16  17 |
    //+------------+--------------------------- -----------------------------------+
    //| Operator A | 0  1  2  6  7  8  12 13 14  18 19  20  24  25  26  30  31  32 |
    //| Operator B | 3  4  5  9  10 11 15 16 17  21 22  23  27  28  29  33  34  35 |
    //+------------+--------------------------- -----------------------------------+
    { 0x00, 0x01, 0x02, 0x08, 0x09, 0x0A, 0x10, 0x11, 0x12 } ,   /*  initializers for operator A */
	{ 0x03, 0x04, 0x05, 0x0B, 0x0C, 0x0D, 0x13, 0x14, 0x15 }     /*  initializers for operator B */
			};

const unsigned char opl_ref_notes[2][12] = {
         //C     C#    D     D#    E     F     F#    G     G#    A     A#    B
/*LOW*/  { 0x59, 0x6D, 0x83, 0x9A, 0xB3, 0xCC, 0xE8, 0x05, 0x23, 0x44, 0x66, 0x88 }, // calculated @ block("octave") 4
/*HIGH*/ { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02 }  // calculated @ block("octave") 4
};

unsigned char activeNote[OPL3_MAX_ACTIVE_NOTES];
//unsigned char A0_temp[OPL3_MAX_ACTIVE_NOTES];
unsigned char B0_temp[OPL3_MAX_ACTIVE_NOTES];// used so that we don't need recalc (f-num hi) and block on key off

unsigned char reg_20_A_temp = 0;
unsigned char reg_40_A_temp = 0;
unsigned char reg_60_A_temp = 0;
unsigned char reg_80_A_temp = 0;
unsigned char reg_E0_A_temp = 0;

unsigned char reg_20_B_temp = 0;
unsigned char reg_40_B_temp = 0;
unsigned char reg_60_B_temp = 0;
unsigned char reg_80_B_temp = 0;
unsigned char reg_E0_B_temp = 0;

unsigned char reg_20_C_temp = 0;
unsigned char reg_40_C_temp = 0;
unsigned char reg_60_C_temp = 0;
unsigned char reg_80_C_temp = 0;
unsigned char reg_E0_C_temp = 0;

unsigned char reg_20_D_temp = 0;
unsigned char reg_40_D_temp = 0;
unsigned char reg_60_D_temp = 0;
unsigned char reg_80_D_temp = 0;
unsigned char reg_E0_D_temp = 0;

unsigned char reg_C0_temp = 0;
unsigned char reg_C0_4op_temp = 0;

unsigned char opl3_currentInstrument = 0;

unsigned char opl3_dataTmp = 0x00;
unsigned char opl3_addrCtrlTmp = 0x00;

//unsigned char regTemp_20_35[42];
//unsigned char regTemp_40_55[42];
//unsigned char regTemp_60_75[42];
//unsigned char regTemp_80_95[42];
//unsigned char regTemp_B0_B8[18];

 /*unsigned char* GetInstrumentValues(unsigned char index)
  {
     return &instrumentValues[index];
 }*/

unsigned char OPL3_setMode(unsigned char modeInAscii)
{
    if (modeInAscii == OPL3_2OP_MODE_IN_ASCII)
    {
        opl3_modeInAscii = OPL3_2OP_MODE_IN_ASCII;
        opl3_max_notes = OPL3_2OP_MAX_NOTES;
        opl3_max_notes_half = OPL3_2OP_MAX_NOTES/2;
        OPL3_write(OPL3_PART_TWO, 0x04, 0x00);
        OPL3_writeAllOps(); // resets op reg:s according to op mode
        OPL3_ResetActiveNotes();
        return 1;
    }
    else if (modeInAscii == OPL3_4OP_MODE_IN_ASCII)
    {
        opl3_modeInAscii = OPL3_4OP_MODE_IN_ASCII;
        opl3_max_notes = OPL3_4OP_MAX_NOTES;
        opl3_max_notes_half = OPL3_4OP_MAX_NOTES/2;
        OPL3_write(OPL3_PART_TWO, 0x04, 0x3F);
        //OPL3_setInstrument(opl3_currentInstrument); // copy values from AB to CD
        OPL3_writeAllOps(); // resets op reg:s according to op mode
        OPL3_ResetActiveNotes();
        return 1;
    }
    else
    {
        return 0;
    }
}

void OPL3_waitXcycles(unsigned long count)
{
    while (count != 0)
    {
        //asm("NOP");
        count--;
    }    
}

void OPL3_ExecMidiNoteOn(unsigned char note, unsigned char velocity)
{

    unsigned char count = 0;
    unsigned char block = 0;
    unsigned char blockNote = 0;
    //UART2_sendText("n_on");
    while (count < opl3_max_notes)
    {
        if ((activeNote[count] == 0x00) || (activeNote[count] == note)) // (activeNote[count] == note) is failsafe
        {
            activeNote[count] = note;
            block = note / 12;
            blockNote = note % 12; // modulus
            B0_temp[count] = ((block << 2) & 0x1C) | opl_ref_notes[1][blockNote];
            if (count < opl3_max_notes_half)
            {
                OPL3_write(OPL3_PART_ONE, 0xA0 + count, opl_ref_notes[0][blockNote]); // Frequency Number (Lower 8 bits)
                OPL3_write(OPL3_PART_ONE, 0xB0 + count, B0_temp[count] | OPL3_KEY_ON_MASK); // Key-On, Block Number, F-Number (high bits)
            }
            else
            {
                OPL3_write(OPL3_PART_TWO, 0xA0 + (count - opl3_max_notes_half), opl_ref_notes[0][blockNote]); // Frequency Number (Lower 8 bits)
                OPL3_write(OPL3_PART_TWO, 0xB0 + (count - opl3_max_notes_half), B0_temp[count] | OPL3_KEY_ON_MASK); // Key-On, Block Number, F-Number (high bits)
            }
            
            break;
        }
        count++;
    }
}

void OPL3_ExecMidiNoteOff(unsigned char note, unsigned char velocity)
{
    unsigned char count = 0;
    //UART2_sendText("n_of");
    while (count < opl3_max_notes)
    {
        if (activeNote[count] == note)
        {
            activeNote[count] = 0x00;
            if (count < opl3_max_notes_half)
            {
                OPL3_write(OPL3_PART_ONE, 0xB0 + count, B0_temp[count]); // Key-Off, Block Number, F-Number (high bits)
            }
            else
            {
                OPL3_write(OPL3_PART_TWO, 0xB0 + (count - opl3_max_notes_half), B0_temp[count]); // Key-Off, Block Number, F-Number (high bits)
            }
        }
        count++;
    }
}

void OPL3_set_reg_20_A() { unsigned char i; for (i = 0; i < opl3_max_notes_half; i++) { OPL3_write_both(opl_regOfs[0][i] + 0x20, reg_20_A_temp); } }
void OPL3_set_reg_40_A() { unsigned char i; for (i = 0; i < opl3_max_notes_half; i++) { OPL3_write_both(opl_regOfs[0][i] + 0x40, reg_40_A_temp); } }
void OPL3_set_reg_60_A() { unsigned char i; for (i = 0; i < opl3_max_notes_half; i++) { OPL3_write_both(opl_regOfs[0][i] + 0x60, reg_60_A_temp); } }
void OPL3_set_reg_80_A() { unsigned char i; for (i = 0; i < opl3_max_notes_half; i++) { OPL3_write_both(opl_regOfs[0][i] + 0x80, reg_80_A_temp); } }
void OPL3_set_reg_E0_A() { unsigned char i; for (i = 0; i < opl3_max_notes_half; i++) { OPL3_write_both(opl_regOfs[0][i] + 0xE0, reg_E0_A_temp); } }

void OPL3_set_reg_20_B() { unsigned char i; for (i = 0; i < opl3_max_notes_half; i++) { OPL3_write_both(opl_regOfs[1][i] + 0x20, reg_20_B_temp); } }
void OPL3_set_reg_40_B() { unsigned char i; for (i = 0; i < opl3_max_notes_half; i++) { OPL3_write_both(opl_regOfs[1][i] + 0x40, reg_40_B_temp); } }
void OPL3_set_reg_60_B() { unsigned char i; for (i = 0; i < opl3_max_notes_half; i++) { OPL3_write_both(opl_regOfs[1][i] + 0x60, reg_60_B_temp); } }
void OPL3_set_reg_80_B() { unsigned char i; for (i = 0; i < opl3_max_notes_half; i++) { OPL3_write_both(opl_regOfs[1][i] + 0x80, reg_80_B_temp); } }
void OPL3_set_reg_E0_B() { unsigned char i; for (i = 0; i < opl3_max_notes_half; i++) { OPL3_write_both(opl_regOfs[1][i] + 0xE0, reg_E0_B_temp); } }

void OPL3_set_reg_20_C() { unsigned char i; for (i = 0; i < opl3_max_notes_half; i++) { OPL3_write_both(opl_regOfs[0][i+3] + 0x20, reg_20_C_temp); } }
void OPL3_set_reg_40_C() { unsigned char i; for (i = 0; i < opl3_max_notes_half; i++) { OPL3_write_both(opl_regOfs[0][i+3] + 0x40, reg_40_C_temp); } }
void OPL3_set_reg_60_C() { unsigned char i; for (i = 0; i < opl3_max_notes_half; i++) { OPL3_write_both(opl_regOfs[0][i+3] + 0x60, reg_60_C_temp); } }
void OPL3_set_reg_80_C() { unsigned char i; for (i = 0; i < opl3_max_notes_half; i++) { OPL3_write_both(opl_regOfs[0][i+3] + 0x80, reg_80_C_temp); } }
void OPL3_set_reg_E0_C() { unsigned char i; for (i = 0; i < opl3_max_notes_half; i++) { OPL3_write_both(opl_regOfs[0][i+3] + 0xE0, reg_E0_C_temp); } }

void OPL3_set_reg_20_D() { unsigned char i; for (i = 0; i < opl3_max_notes_half; i++) { OPL3_write_both(opl_regOfs[1][i+3] + 0x20, reg_20_D_temp); } }
void OPL3_set_reg_40_D() { unsigned char i; for (i = 0; i < opl3_max_notes_half; i++) { OPL3_write_both(opl_regOfs[1][i+3] + 0x40, reg_40_D_temp); } }
void OPL3_set_reg_60_D() { unsigned char i; for (i = 0; i < opl3_max_notes_half; i++) { OPL3_write_both(opl_regOfs[1][i+3] + 0x60, reg_60_D_temp); } }
void OPL3_set_reg_80_D() { unsigned char i; for (i = 0; i < opl3_max_notes_half; i++) { OPL3_write_both(opl_regOfs[1][i+3] + 0x80, reg_80_D_temp); } }
void OPL3_set_reg_E0_D() { unsigned char i; for (i = 0; i < opl3_max_notes_half; i++) { OPL3_write_both(opl_regOfs[1][i+3] + 0xE0, reg_E0_D_temp); } }

void OPL3_set_reg_C0()   { unsigned char i; for (i = 0; i < opl3_max_notes_half; i++) { OPL3_write_both(0xC0 + i, reg_C0_temp); } }
void OPL3_set_reg_C0_4OP()   { unsigned char i; for (i = 0; i < opl3_max_notes_half; i++) { OPL3_write_both(0xC0 + i+3, reg_C0_4op_temp); } }

void OPL3_writeAllOps()
{
    unsigned char i;
    unsigned char regOffsetTmp = 0;
    for (i = 0; i < opl3_max_notes_half; i++)
    {
        regOffsetTmp = opl_regOfs[0][i];
        OPL3_write_both(regOffsetTmp + 0x20, reg_20_A_temp);
        OPL3_write_both(regOffsetTmp + 0x40, reg_40_A_temp);
        OPL3_write_both(regOffsetTmp + 0x60, reg_60_A_temp);
        OPL3_write_both(regOffsetTmp + 0x80, reg_80_A_temp);
        OPL3_write_both(regOffsetTmp + 0xE0, reg_E0_A_temp);
        regOffsetTmp = opl_regOfs[1][i];
        OPL3_write_both(regOffsetTmp + 0x20, reg_20_B_temp);
        OPL3_write_both(regOffsetTmp + 0x40, reg_40_B_temp);
        OPL3_write_both(regOffsetTmp + 0x60, reg_60_B_temp);
        OPL3_write_both(regOffsetTmp + 0x80, reg_80_B_temp);
        OPL3_write_both(regOffsetTmp + 0xE0, reg_E0_B_temp);
        OPL3_write_both(i + 0xC0, reg_C0_temp);
        
        if (opl3_modeInAscii == OPL3_4OP_MODE_IN_ASCII)
        {
            regOffsetTmp = opl_regOfs[0][i+3];
            OPL3_write_both(regOffsetTmp + 0x20, reg_20_C_temp);
            OPL3_write_both(regOffsetTmp + 0x40, reg_40_C_temp);
            OPL3_write_both(regOffsetTmp + 0x60, reg_60_C_temp);
            OPL3_write_both(regOffsetTmp + 0x80, reg_80_C_temp);
            OPL3_write_both(regOffsetTmp + 0xE0, reg_E0_C_temp);
            regOffsetTmp = opl_regOfs[1][i+3];
            OPL3_write_both(regOffsetTmp + 0x20, reg_20_D_temp);
            OPL3_write_both(regOffsetTmp + 0x40, reg_40_D_temp);
            OPL3_write_both(regOffsetTmp + 0x60, reg_60_D_temp);
            OPL3_write_both(regOffsetTmp + 0x80, reg_80_D_temp);
            OPL3_write_both(regOffsetTmp + 0xE0, reg_E0_D_temp);
            OPL3_write_both(i+3 + 0xC0, reg_C0_4op_temp);
        }
        
    }
}

void OPL3_setInstrument(unsigned char ii)
{
    opl3_currentInstrument = ii;
    
    // reg 20 40 60 80 E0 each contains 9 ch
    reg_20_A_temp = instrumentValues[ii][1];
    reg_40_A_temp = instrumentValues[ii][2];
    reg_60_A_temp = instrumentValues[ii][3];
    reg_80_A_temp = instrumentValues[ii][4];
    reg_E0_A_temp = instrumentValues[ii][5];
    reg_20_B_temp = instrumentValues[ii][7];
    reg_40_B_temp = instrumentValues[ii][8];
    reg_60_B_temp = instrumentValues[ii][9];
    reg_80_B_temp = instrumentValues[ii][10];
    reg_E0_B_temp = instrumentValues[ii][11];
    reg_C0_temp = instrumentValues[ii][6] | 0x30; // 0x30 means LEFT+RIGHT
    
    if (opl3_modeInAscii == OPL3_4OP_MODE_IN_ASCII)
    {
        reg_20_C_temp = instrumentValues[ii][1];
        reg_40_C_temp = instrumentValues[ii][2];
        reg_60_C_temp = instrumentValues[ii][3];
        reg_80_C_temp = instrumentValues[ii][4];
        reg_E0_C_temp = instrumentValues[ii][5];
        reg_20_D_temp = instrumentValues[ii][7];
        reg_40_D_temp = instrumentValues[ii][8];
        reg_60_D_temp = instrumentValues[ii][9];
        reg_80_D_temp = instrumentValues[ii][10];
        reg_E0_D_temp = instrumentValues[ii][11];
        reg_C0_4op_temp = instrumentValues[ii][6] | 0x30; // 0x30 means LEFT+RIGHT
    }
    OPL3_writeAllOps();
}

void OPL3_HWinit(void)
{
    SPI_init();
    OPL3_SERIAL_LATCH = 0; // OPL3 serial latch
    OPL3_SERIAL_LATCH_DIR = 0; // OPL3 serial latch as output
    
    opl3_dataTmp = 0x00;
    opl3_addrCtrlTmp = 0xC0; // set both CS# and WR# high
    OPL3_SPI_WriteData();
}
void OPL3_ResetActiveNotes()
{
    unsigned char count = 0;
    for (count = 0; count < MAX_ACTIVE_NOTES; count++)
    {
        activeNote[count] = 0x00;
        if (count < (MAX_ACTIVE_NOTES/2))
           OPL3_write(OPL3_PART_ONE, 0xB0 + count, B0_temp[count]);
        else
            OPL3_write(OPL3_PART_TWO, 0xB0 + (count - (MAX_ACTIVE_NOTES/2)), B0_temp[count]);
    }
}

void OPL3_SWinit(void)
{
    
    // additional sw initiations here...
    OPL3_SwReset();
    OPL3_write(OPL3_PART_TWO, 0x05, 0x01); // activate OPL3 mode
    //OPL3_write(OPL3_PART_ONE, 0x01, 0x20); // activate waveform selection (if OPL2 mode and ifNot enabled only sinus wave is avaliable)
    //OPL3_write(OPL3_PART_TWO, 0x01, 0x20); // activate waveform selection (if OPL2 mode and ifNot enabled only sinus wave is avaliable)
    OPL3_ResetActiveNotes();
    
}

void OPL3_SwReset(void)
{
    unsigned char addr = 0;
    for (addr = 0; addr < 255; addr++)
    {
        OPL3_write(OPL3_PART_ONE, addr, 0x00);
        OPL3_write(OPL3_PART_TWO, addr, 0x00);
    }
}

void OPL3_SPI_WriteData(void)
{
    OPL3_SERIAL_LATCH = 0;
    SPI_Write(opl3_dataTmp);
    SPI_Write(opl3_addrCtrlTmp);
    asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");
    asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");
    OPL3_SERIAL_LATCH = 1; // latch in the serial data
    asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");
    OPL3_SERIAL_LATCH = 0;
}

void OPL3_write_both(unsigned char addr, unsigned char data)
{
    OPL3_write(OPL3_PART_ONE, addr, data);
    OPL3_write(OPL3_PART_TWO, addr, data);
}

void OPL3_write(unsigned char part, unsigned char addr, unsigned char data)
{
    // *** first write the register address ***
    opl3_dataTmp = addr;
    if (part == OPL3_PART_TWO) opl3_addrCtrlTmp = 0xC0 | OPL3_A1;
    else opl3_addrCtrlTmp = 0xC0;
    OPL3_SPI_WriteData();
    OPL3_waitXcycles(32);
    
    // *** write addr-data cycle ***
    opl3_addrCtrlTmp &= ~0xC0; // clear CS# and WR#
    OPL3_SPI_WriteData();
    OPL3_waitXcycles(32);
    
    opl3_addrCtrlTmp |= 0xC0; // set CS# and WR#
    OPL3_SPI_WriteData();
    OPL3_waitXcycles(32);
    
    // *** second write the data ***
    opl3_addrCtrlTmp |= OPL3_A0; // set to write data reg
    opl3_dataTmp = data;
    OPL3_SPI_WriteData();
    OPL3_waitXcycles(32);
    
    // *** write data-data cycle ***
    opl3_addrCtrlTmp &= ~0xC0; // clear CS# and WR#
    OPL3_SPI_WriteData();
    OPL3_waitXcycles(32);
    
    opl3_addrCtrlTmp |= 0xC0; // set CS# and WR#
    OPL3_SPI_WriteData();
    OPL3_waitXcycles(32);
}


void OPL3_setOperatorMode(unsigned char chNr, unsigned char opNr, unsigned char data) // 0x20 - 0x35
{
    
}

void OPL3_setVolume(unsigned char chNr, unsigned char opNr, unsigned char data) // 0x40 - 0x55 (Bit0-5)
{

}

void OPL3_setChannelPitch(unsigned char chNr, unsigned char data) // 0x40-55 (Bit6-7)
{

}

void OPL3_setAttack(unsigned char chNr, unsigned char opNr, unsigned char value) // 0x60-75 (high nibbles)
{

}

void OPL3_setDecay(unsigned char chNr, unsigned char opNr, unsigned char value) // 0x60-75 (low nibbles)
{

}

void OPL3_setSustain(unsigned char chNr, unsigned char opNr, unsigned char value) // 0x80-95 (high nibbles)
{

}

void OPL3_setRelease(unsigned char chNr, unsigned char opNr, unsigned char value) // 0x80-95 (low nibbles)
{

}

void OPL3_setNote(unsigned int noteData, unsigned char octave, unsigned char chNr) // 0xA0-A8, 0xB0-B8 (Bit0-4)
{

}

void OPL3_turnOnNote(unsigned char chNr) // 0xB0-B8 (bit5)
{

}

void OPL3_turnOffNote(unsigned char chNr) // 0xB0-B8 (bit5)
{

}

void OPL3_setChannelMode(unsigned char ch, unsigned char data)
{

}

void OPL3_setOperatorWaveform(unsigned char chNr, unsigned char opNr, unsigned char waveForm)
{

}

