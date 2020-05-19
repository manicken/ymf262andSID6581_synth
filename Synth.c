
#include <xc.h>
#include "Synth.h"
#include "UART.h"
#include "Notes.h"
#include "SID6581.h"
#include "OPL3.h"

#include "instruments.h"

unsigned char midiRxDebug = 0;

unsigned char count = 0;
unsigned char octave = 0;
unsigned char note = 0;
unsigned int noteDataRaw = 0;

unsigned char tempValue1 = 0;
unsigned char tempValue2 = 0;
unsigned char tempValue3 = 0;
unsigned char tempValue4 = 0;
unsigned char tempValue5 = 0;
unsigned char iTemp = 0;

unsigned char midiCmdTemp = 0;
unsigned char midiKeyTemp = 0;
unsigned char midiVelTemp = 0;

//unsigned char activeNote[MAX_ACTIVE_NOTES];

void waitXcycles(unsigned long count)
{
    while (count)
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

        count--;
    }    
}

void Synth_init(void)
{
    uart_flags.val = 0x00;
    
    DEBUG_LED1_DIR = 0;
    DEBUG_LED2_DIR = 0;
    DEBUG_LED1 = 0;
    DEBUG_LED2 = 0;
    
    UARTs_init();
    DEBUG_LED1 = 1;
    
    UART2_sendText("STARTING");
    
    SID_OPL_RST_OE = 0; // SID/OPL3 serial OE/RST# in RST# state
    SID_OPL_RST_OE_DIR = 0; // SID/OPL3 serial OE/RST# as output

    OPL3_HWinit();
    waitXcycles(10);
    SID_OPL_RST_OE = 1; // enable OE (open enable)
    OPL3_SWinit();
    OPL3_setInstrument(0x54); // PIANO
    
    //SID_HWinit();
    //waitXcycles(10);
    //SID_SWinit();
    //SID_SetDefault_Instrument();

    UART2_sendText(" [OK]\n");
}

void Synth_Task(void)
{
    //if (DEBUG_LED2 == 0)
    //    DEBUG_LED2 = 1;
    //else
    //    DEBUG_LED2 = 0;
    
    if (uart_flags.MIDI_MSG_RX)
    {
        uart_flags.MIDI_MSG_RX = 0;
        while (midiMsgFifoDataCount != 0)
        {
            Decode_UART_midi();
            midiMsgFifoDataCount--;
        }
    }
    if (uart_flags.CMD_MSG_RX)
    {
        uart_flags.CMD_MSG_RX = 0;
        if (Decode_UART_cmd())
            UART2_sendText("OK\n");
        else
            UART2_sendText("KO\n");
    }
}

unsigned char Decode_UART_cmd(void)
{
    if (cmdUartRxLength < 1){ UART2_sendText("rxLength<1 "); return 0;}
    
    if (cmdUartRxBuff[0] == 's') // set
    {
        if (cmdUartRxLength < 2){ UART2_sendText("rxLength<2 "); return 0;}
        
        if (cmdUartRxBuff[1] == 's') // SID
        {
            return Decode_UART_SID();
        }
        else if (cmdUartRxBuff[1] == 'o') // OPL3
        {
            return Decode_UART_OPL3();
        }
        else if (cmdUartRxBuff[1] == 'c') // system Ctrl
        {
            if (cmdUartRxBuff[2] == 'm') // midi
            {
                if (cmdUartRxBuff[3] == 'd') // debug
                {
                    if (cmdUartRxBuff[4] == '0')
                    {
                        midiRxDebug = 0;
                        return 1;
                    }
                    else if (cmdUartRxBuff[4] == '1')
                    {
                        midiRxDebug = 1;
                        return 1;
                    }
                }
            }
        }
    }
    else if (cmdUartRxBuff[0] == 'g') // get
    {
        if (cmdUartRxBuff[1] == 'i') // instrument
        {
            if (cmdUartRxBuff[2] == 'l') // list
            {
                for (iTemp = 0; iTemp < INSTRUMENT_COUNT; iTemp++)
                {
                    UART2_sendText("gil(");
                    UART2_sendHex(iTemp);
                    UART2_sendText("):");
                    UART2_sendText(&instrumentNames[iTemp]);
                    UART2_sendText("\n");
                }
                return 1;
            }
            else if (cmdUartRxBuff[2] == 'c') // current instrument
            {
                UART2_sendText("gic(");
                UART2_sendHex(opl3_currentInstrument);
                UART2_sendText("):");
                UART2_sendHex(reg_20_A_temp);
                UART2_sendHex(reg_40_A_temp);
                UART2_sendHex(reg_60_A_temp);
                UART2_sendHex(reg_80_A_temp);
                UART2_sendHex(reg_E0_A_temp);
                UART2_sendHex(reg_C0_temp);
                UART2_sendHex(reg_20_B_temp);
                UART2_sendHex(reg_40_B_temp);
                UART2_sendHex(reg_60_B_temp);
                UART2_sendHex(reg_80_B_temp);
                UART2_sendHex(reg_E0_B_temp);
                if (opl3_modeInAscii == OPL3_4OP_MODE_IN_ASCII)
                {
                    UART2_sendHex(reg_20_C_temp);
                    UART2_sendHex(reg_40_C_temp);
                    UART2_sendHex(reg_60_C_temp);
                    UART2_sendHex(reg_80_C_temp);
                    UART2_sendHex(reg_E0_C_temp);
                    UART2_sendHex(reg_C0_4op_temp);
                    UART2_sendHex(reg_20_D_temp);
                    UART2_sendHex(reg_40_D_temp);
                    UART2_sendHex(reg_60_D_temp);
                    UART2_sendHex(reg_80_D_temp);
                    UART2_sendHex(reg_E0_D_temp);
                }
                UART2_send('\n');
                return 1;
            }
            tempValue1 = convAsciiHex(&cmdUartRxBuff[2]); // instrument nr
            UART2_sendText("gi:");
            UART_sendArrayAsHex(&instrumentValues[tempValue1], 12);
            UART2_sendText("\n");
            return 1;
        }
        else if (cmdUartRxBuff[1] == 'n') // note status, only to debug errors
        {
            UART2_sendText("noteStatus:");
            for (iTemp = 0; iTemp < OPL3_MAX_ACTIVE_NOTES; iTemp++)
            {
                UART2_sendHex(activeNote[iTemp]);
            }
            UART2_send('\n');
            return 1;
        }
    }
    UART2_sendText("unknownCmd ");
    return 0;
}
unsigned char Decode_UART_OPL3(void)
{
    tempValue1 = cmdUartRxBuff[2];
    if (tempValue1 == '0') tempValue1 = OPL3_PART_ONE;
    else if (tempValue1 == '1') tempValue1 = OPL3_PART_TWO;
    else if (tempValue1 == 'i') // set instrument
    {
        tempValue1 = convAsciiHex(&cmdUartRxBuff[3]); // instrument nr
        OPL3_setInstrument(tempValue1);
        return 1;
    }
    else if (tempValue1 == 'd') // raw debug 
    {
        if (cmdUartRxLength != 7){ UART2_sendText("rxLength!=7 "); return 0;}
            
        opl3_dataTmp = convAsciiHex(&cmdUartRxBuff[3]);
        opl3_addrCtrlTmp = convAsciiHex(&cmdUartRxBuff[5]);
        UART2_sendHex(opl3_dataTmp);
        UART2_sendHex(opl3_addrCtrlTmp);
        OPL3_SPI_WriteData();
        return 1;
    }
    else if (tempValue1 == 'r') // register
    {
        tempValue2 = cmdUartRxBuff[3]; 
        tempValue3 = cmdUartRxBuff[4]; // A or B
        tempValue4 = convAsciiHex(&cmdUartRxBuff[5]);
        
        if (tempValue2 == '2')
        {
            if (tempValue3 == 'A') { reg_20_A_temp = tempValue4; OPL3_set_reg_20_A(); return 1; }
            else if (tempValue3 == 'B') { reg_20_B_temp = tempValue4; OPL3_set_reg_20_B(); return 1; }
            else if (tempValue3 == 'C') { reg_20_C_temp = tempValue4; OPL3_set_reg_20_C(); return 1; }
            else if (tempValue3 == 'D') { reg_20_D_temp = tempValue4; OPL3_set_reg_20_D(); return 1; }
            else return 0;
        }
        else if (tempValue2 == '4')
        {
            if (tempValue3 == 'A') { reg_40_A_temp = tempValue4; OPL3_set_reg_40_A(); return 1; }
            else if (tempValue3 == 'B') { reg_40_B_temp = tempValue4; OPL3_set_reg_40_B(); return 1; }
            else if (tempValue3 == 'C') { reg_40_C_temp = tempValue4; OPL3_set_reg_40_C(); return 1; }
            else if (tempValue3 == 'D') { reg_40_D_temp = tempValue4; OPL3_set_reg_40_D(); return 1; }
            else return 0;
        }
        else if (tempValue2 == '6')
        {
            if (tempValue3 == 'A') { reg_60_A_temp = tempValue4; OPL3_set_reg_60_A(); return 1; }
            else if (tempValue3 == 'B') { reg_60_B_temp = tempValue4; OPL3_set_reg_60_B(); return 1; }
            else if (tempValue3 == 'C') { reg_60_C_temp = tempValue4; OPL3_set_reg_60_C(); return 1; }
            else if (tempValue3 == 'D') { reg_60_D_temp = tempValue4; OPL3_set_reg_60_D(); return 1; }
            else return 0;
        }
        else if (tempValue2 == '8')
        {
            if (tempValue3 == 'A') { reg_80_A_temp = tempValue4; OPL3_set_reg_80_A(); return 1; }
            else if (tempValue3 == 'B') { reg_80_B_temp = tempValue4; OPL3_set_reg_80_B(); return 1; }
            else if (tempValue3 == 'C') { reg_80_C_temp = tempValue4; OPL3_set_reg_80_C(); return 1; }
            else if (tempValue3 == 'D') { reg_80_D_temp = tempValue4; OPL3_set_reg_80_D(); return 1; }
            else return 0;
        }
        else if (tempValue2 == 'C')
        {
            if (tempValue3 == 'A') { reg_C0_temp = tempValue4; OPL3_set_reg_C0(); return 1; }
            else if (tempValue3 == 'C') { reg_C0_4op_temp = tempValue4; OPL3_set_reg_C0_4OP(); return 1; }
            else return 0;
        }
        else if (tempValue2 == 'E')
        {
            if (tempValue3 == 'A') { reg_E0_A_temp = tempValue4; OPL3_set_reg_E0_A(); return 1; }
            else if (tempValue3 == 'B') { reg_E0_B_temp = tempValue4; OPL3_set_reg_E0_B(); return 1; }
            else if (tempValue3 == 'C') { reg_E0_C_temp = tempValue4; OPL3_set_reg_E0_C(); return 1; }
            else if (tempValue3 == 'D') { reg_E0_D_temp = tempValue4; OPL3_set_reg_E0_D(); return 1; }
            else return 0;
        }
        else
        {
            UART2_sendText("OPL_REG_PARA != 2|3|6|8|C|E ");
            return 0;
        }
    }
    else if (tempValue1 == 'm') // mode
    {
        return OPL3_setMode(cmdUartRxBuff[3]);
    }
    else
    {
        UART2_sendText("OPL_PART_PARA != 0|1|i|d|r|m ");
        return 0;
    }
    if (cmdUartRxLength != 7){ UART2_sendText("rxLength!=7 "); return 0;}
            
    //cmdUartRxBuff[2] = convOneAsciiHex(cmdUartRxBuff[2]); // defines which part of OPL3 to write 
    tempValue2 = convAsciiHex(&cmdUartRxBuff[3]); // reg addr
    tempValue3 = convAsciiHex(&cmdUartRxBuff[5]); // reg data
    OPL3_write(tempValue1,tempValue2,tempValue3);
    return 1;
}
unsigned char Decode_UART_SID(void)
{
    if (cmdUartRxLength != 7){ UART2_sendText("rxLength!=7 "); return 0;}

    tempValue1 = cmdUartRxBuff[2];
    if (tempValue1 == '1') tempValue1 = SID_LEFT_CHIP;
    else if (tempValue1 == '2') tempValue1 = SID_LEFT_CHIP;
    else if (tempValue1 == '3') tempValue1 = SID_BOTH_CHIPS;
    else if (tempValue1 == 'r') // raw debug 
    {
        sid_dataTmp = convAsciiHex(&cmdUartRxBuff[3]);
        sid_addrCtrlTmp = convAsciiHex(&cmdUartRxBuff[5]);
        UART2_sendHex(sid_dataTmp);
        UART2_sendHex(sid_addrCtrlTmp);
        SID_SPI_WriteData();
        return 1;
    }
    else
    {
        UART2_sendText("SID_CS_PARA != 1|2|3|r|R ");
        return 0;
    }
    //cmdUartRxBuff[2] = convOneAsciiHex(cmdUartRxBuff[2]) // defines which SID to write 
    tempValue2 = convAsciiHex(&cmdUartRxBuff[3]); // reg addr
    tempValue3 = convAsciiHex(&cmdUartRxBuff[5]); // reg data
    SID_write(tempValue2, tempValue3, tempValue1);
    return 1;
}
void Decode_UART_midi(void)
{
    #define KEYBOARD_NOTE_SHIFT_CORRECTION 21-12

    midiCmdTemp = midiMsgFifo[midiMsgFifoReadPointer][0];
    midiKeyTemp = midiMsgFifo[midiMsgFifoReadPointer][1] + KEYBOARD_NOTE_SHIFT_CORRECTION;
    midiVelTemp = midiMsgFifo[midiMsgFifoReadPointer][2];
    
    if (midiMsgFifoReadPointer != (MIDI_MSG_FIFO_SIZE - 1)) midiMsgFifoReadPointer++;
    else midiMsgFifoReadPointer = 0;
    
    if ((midiCmdTemp & 0xF0) == 0x90) //Note on command
        OPL3_ExecMidiNoteOn(midiKeyTemp, midiVelTemp);
    else if ((midiCmdTemp & 0xF0) == 0x80) //Note off command
        OPL3_ExecMidiNoteOff(midiKeyTemp, midiVelTemp);
    if (midiRxDebug)
    {
        UART2_sendText("midi:");
        UART2_sendHex(midiCmdTemp);
        UART2_sendHex(midiKeyTemp);
        UART2_sendHex(midiVelTemp);
        UART2_send('\n');
    }
    
}


 