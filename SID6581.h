#ifndef SID6581_H_
#define SID6581_H_

#define SID_OSC_WAVETYPE_NOISE    0x80
#define SID_OSC_WAVETYPE_SQUARE   0x40
#define SID_OSC_WAVETYPE_SAW      0x20
#define SID_OSC_WAVETYPE_TRIANGLE 0x10

#define SID_LEFT_CHIP             0x80
#define SID_RIGHT_CHIP            0x40
#define SID_BOTH_CHIPS            0x00

#define SID_SERIAL_LATCH_DIR TRISDbits.TRISD0
#define SID_SERIAL_LATCH     LATDbits.LATD0

extern unsigned char sid_dataTmp;
extern unsigned char sid_addrCtrlTmp;

extern void SID_Play_C_accord(void);
extern void SID_Play_Demo(void);
extern void SID_SetDefault_Instrument(void);
extern void SID_HWinit(void);
extern void SID_SWinit(void);
extern void SID_waitXcycles(unsigned long count);
extern void SID_waitOneCycle(void);
extern void SID_waitForClock(void);
extern void SID_HwReset(void);
extern void SID_SwReset(void);
extern void SID_write(unsigned char addr, unsigned char data, unsigned char chipSelect);
extern void SID_setNote(unsigned int noteData, unsigned char osc);
extern void SID_setVolume(unsigned char value, unsigned char chipSelect);
extern void SID_turnOnNote(unsigned char osc);
extern void SID_turnOffNote(unsigned char osc);
extern void SID_setOscWaveType(unsigned char type, unsigned char osc);

void SID_SPI_WriteData(void);

#endif