#ifndef OPL3_H_
#define OPL3_H_

#define OPL3_MAX_ACTIVE_NOTES 18

#define OPL3_SERIAL_LATCH_DIR TRISDbits.TRISD8
#define OPL3_SERIAL_LATCH     LATDbits.LATD8

#define OPL3_PART_ONE 0
#define OPL3_PART_TWO 1

#define OPL3_A0 0x10
#define OPL3_A1 0x20

#define OPL3_KEY_ON_MASK 0x20 

#define OPL3_2OP_MODE_IN_ASCII '2'
#define OPL3_4OP_MODE_IN_ASCII '4'
#define OPL3_2OP_MAX_NOTES 18
#define OPL3_4OP_MAX_NOTES 6

extern unsigned char opl3_modeInAscii;
extern unsigned char opl3_max_notes;
extern unsigned char opl3_max_notes_half;

extern unsigned char activeNote[OPL3_MAX_ACTIVE_NOTES];

extern unsigned char opl3_currentInstrument;

extern unsigned char opl3_dataTmp;
extern unsigned char opl3_addrCtrlTmp;

extern unsigned char reg_20_A_temp;
extern unsigned char reg_40_A_temp;
extern unsigned char reg_60_A_temp;
extern unsigned char reg_80_A_temp;
extern unsigned char reg_E0_A_temp;

extern unsigned char reg_20_B_temp;
extern unsigned char reg_40_B_temp;
extern unsigned char reg_60_B_temp;
extern unsigned char reg_80_B_temp;
extern unsigned char reg_E0_B_temp;

extern unsigned char reg_20_C_temp;
extern unsigned char reg_40_C_temp;
extern unsigned char reg_60_C_temp;
extern unsigned char reg_80_C_temp;
extern unsigned char reg_E0_C_temp;

extern unsigned char reg_20_D_temp;
extern unsigned char reg_40_D_temp;
extern unsigned char reg_60_D_temp;
extern unsigned char reg_80_D_temp;
extern unsigned char reg_E0_D_temp;

extern unsigned char reg_C0_temp;
extern unsigned char reg_C0_4op_temp;

extern unsigned char OPL3_setMode(unsigned char modeInAscii);

extern void OPL3_ExecMidiNoteOn(unsigned char note, unsigned char velocity);
extern void OPL3_ExecMidiNoteOff(unsigned char note, unsigned char velocity);
extern void OPL3_ResetActiveNotes();
extern void OPL3_HWinit(void);
extern void OPL3_SWinit(void);

extern void OPL3_set_reg_20_A();
extern void OPL3_set_reg_40_A();
extern void OPL3_set_reg_60_A();
extern void OPL3_set_reg_80_A();
extern void OPL3_set_reg_E0_A();

extern void OPL3_set_reg_20_B();
extern void OPL3_set_reg_40_B();
extern void OPL3_set_reg_60_B();
extern void OPL3_set_reg_80_B();
extern void OPL3_set_reg_E0_B();

extern void OPL3_set_reg_20_C();
extern void OPL3_set_reg_40_C();
extern void OPL3_set_reg_60_C();
extern void OPL3_set_reg_80_C();
extern void OPL3_set_reg_E0_C();

extern void OPL3_set_reg_20_D();
extern void OPL3_set_reg_40_D();
extern void OPL3_set_reg_60_D();
extern void OPL3_set_reg_80_D();
extern void OPL3_set_reg_E0_D();

extern void OPL3_set_reg_C0();
extern void OPL3_set_reg_C0_4OP();

extern void OPL3_writeAllOps();
extern void OPL3_setInstrument(unsigned char index);

//extern unsigned char* GetInstrumentValues(unsigned char index);

void OPL3_SwReset(void);

void OPL3_SPI_WriteData(void);

extern void OPL3_write_both(unsigned char addr, unsigned char data);
extern void OPL3_write(unsigned char part, unsigned char addr, unsigned char data);

extern void OPL3_setOperatorMode(unsigned char chNr, unsigned char opNr, unsigned char data);

extern void OPL3_setVolume(unsigned char chNr, unsigned char opNr, unsigned char data);
extern void OPL3_setChannelPitch(unsigned char chNr, unsigned char data);

extern void OPL3_setAttack(unsigned char chNr, unsigned char opNr, unsigned char value); // 0x60 - 0x75 (high nibbles)
extern void OPL3_setDecay(unsigned char chNr, unsigned char opNr, unsigned char value); // 0x60 - 0x75 (low nibbles)
extern void OPL3_setSustain(unsigned char chNr, unsigned char opNr, unsigned char value); // 0x80 - 0x95 (high nibbles)
extern void OPL3_setRelease(unsigned char chNr, unsigned char opNr, unsigned char value); // 0x80 - 0x95 (low nibbles)

extern void OPL3_setNote(unsigned int noteData, unsigned char octave, unsigned char chNr);
extern void OPL3_turnOnNote(unsigned char chNr);
extern void OPL3_turnOffNote(unsigned char chNr);

extern void OPL3_setChannelMode(unsigned char ch, unsigned char data);
extern void OPL3_setOperatorWaveform(unsigned char chNr, unsigned char opNr, unsigned char waveForm);


#endif