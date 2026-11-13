#include <xc.h>
#include <stdint.h>

#include "onewire.h"

#define _XTAL_FREQ 8000000UL

#define OW_TRIS     TRISBbits.TRISB1
#define OW_LAT      LATBbits.LATB1
#define OW_PORT     PORTBbits.RB1


uint8_t OW_Reset(void)
{
    uint8_t presence;

    OW_LAT = 0;
    OW_TRIS = 0;
    __delay_us(480);

    OW_TRIS = 1;
    __delay_us(70);

    presence = !OW_PORT;

    __delay_us(410);

    return presence;
}


void OW_WriteBit(uint8_t bit)
{
    OW_LAT = 0;
    OW_TRIS = 0;

    if (bit)
    {
        __delay_us(6);
        OW_TRIS = 1;
        __delay_us(64);
    }
    else
    {
        __delay_us(60);
        OW_TRIS = 1;
        __delay_us(10);
    }
}


uint8_t OW_ReadBit(void)
{
    uint8_t bit;

    OW_LAT = 0;
    OW_TRIS = 0;
    __delay_us(6);

    OW_TRIS = 1;
    __delay_us(9);

    bit = OW_PORT;

    __delay_us(55);

    return bit;
}


void OW_WriteByte(uint8_t data)
{
    uint8_t i;

    for (i = 0; i < 8; i++)
    {
        OW_WriteBit(data & 0x01);
        data >>= 1;
    }
}


uint8_t OW_ReadByte(void)
{
    uint8_t i;
    uint8_t data = 0;

    for (i = 0; i < 8; i++)
    {
        if (OW_ReadBit())
        {
            data |= (1 << i);
        }
    }

    return data;
}
