#ifndef ONEWIRE_H
#define ONEWIRE_H

#include <stdint.h>

uint8_t OW_Reset(void);
void OW_WriteBit(uint8_t bit);
uint8_t OW_ReadBit(void);
void OW_WriteByte(uint8_t data);
uint8_t OW_ReadByte(void);

#endif
