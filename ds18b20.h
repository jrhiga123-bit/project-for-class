#ifndef DS18B20_H
#define DS18B20_H

#include <stdint.h>

extern volatile uint8_t sensor_present;

int16_t DS18B20_ReadTemperature(
    const uint8_t rom_address[8]
);

#endif
