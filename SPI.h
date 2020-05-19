#ifndef SPI_H_
#define SPI_H_

extern void SPI_init(void);
extern void SPI_Write(unsigned int data);
extern unsigned int SPI_Read(void);

#endif