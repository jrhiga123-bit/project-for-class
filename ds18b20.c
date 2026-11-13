#include <xc.h>
#include <stdint.h>

#include "onewire.h"
#include "ds18b20.h"

#define _XTAL_FREQ 8000000UL

#define MATCH_ROM          0x55
#define CONVERT_T          0x44
#define READ_SCRATCHPAD    0xBE

volatile uint8_t sensor_present;


int16_t DS18B20_ReadTemperature(
    const uint8_t rom_address[8]
)
{
    uint8_t i;
    uint8_t scratchpad[9];
    int16_t raw_temperature;

    uint8_t saved_gieh;
    uint8_t saved_giel;

    /*
     * Disable interrupts during precise 1-Wire communication.
     */
    saved_gieh = INTCONbits.GIEH;
    saved_giel = INTCONbits.GIEL;

    INTCONbits.GIEH = 0;
    INTCONbits.GIEL = 0;

    sensor_present = OW_Reset();

    if (!sensor_present)
    {
        INTCONbits.GIEH = saved_gieh;
        INTCONbits.GIEL = saved_giel;
        return -999;
    }

    OW_WriteByte(MATCH_ROM);

    for (i = 0; i < 8; i++)
    {
        OW_WriteByte(rom_address[i]);
    }

    OW_WriteByte(CONVERT_T);

    /*
     * Re-enable interrupts during the 750 ms conversion.
     */
    INTCONbits.GIEH = saved_gieh;
    INTCONbits.GIEL = saved_giel;

    __delay_ms(750);

    /*
     * Disable interrupts again for scratchpad reading.
     */
    saved_gieh = INTCONbits.GIEH;
    saved_giel = INTCONbits.GIEL;

    INTCONbits.GIEH = 0;
    INTCONbits.GIEL = 0;

    sensor_present = OW_Reset();

    if (!sensor_present)
    {
        INTCONbits.GIEH = saved_gieh;
        INTCONbits.GIEL = saved_giel;
        return -999;
    }

    OW_WriteByte(MATCH_ROM);

    for (i = 0; i < 8; i++)
    {
        OW_WriteByte(rom_address[i]);
    }

    OW_WriteByte(READ_SCRATCHPAD);

    for (i = 0; i < 9; i++)
    {
        scratchpad[i] = OW_ReadByte();
    }

    INTCONbits.GIEH = saved_gieh;
    INTCONbits.GIEL = saved_giel;

    raw_temperature =
        ((int16_t)scratchpad[1] << 8)
        | scratchpad[0];

    return (raw_temperature * 10) / 16;
}
