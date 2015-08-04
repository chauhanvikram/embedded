#ifndef SPI_H
#define SPI_H

#include <stdint.h>

void initSPI(void);
void transmitEndResponse(void);
uint8_t getReceiveData(void);
void sendData(uint8_t data);
uint16_t getCount(void);

#endif /* SPI_H */
