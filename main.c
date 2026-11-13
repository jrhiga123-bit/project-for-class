#include <xc.h>
#include <stdint.h>

#include "onewire.h"
#include "ds18b20.h"

#define _XTAL_FREQ 8000000UL

const uint8_t SENSOR1_ROM[8] =
{
    0x28, 0x8A, 0xC7, 0xD2,
    0x00, 0x00, 0x00, 0x4E
};

volatile int16_t temperature;
volatile uint8_t temp_above_75 = 0;
volatile uint8_t temp_above_60 = 0;

volatile uint8_t high_temp_active = 0;
volatile uint8_t cooling_period = 0;
volatile uint16_t cooling_ticks = 0;
volatile uint8_t led_tick = 0;

#define FANS_ON()       (LATC |= 0xF0)
#define FANS_OFF()      (LATC &= 0x0F)

#define ALARM_ON()      (LATBbits.LATB7 = 1)
#define ALARM_OFF()     (LATBbits.LATB7 = 0)

#define LED_ON()        (LATDbits.LATD5 = 1)
#define LED_OFF()       (LATDbits.LATD5 = 0)
#define LED_TOGGLE()    (LATDbits.LATD5 ^= 1)


void IO_Init(void)
{
    ANSELB = 0x00;
    ANSELC = 0x00;
    ANSELD = 0x00;

    // DS18B20 data line
    TRISBbits.TRISB1 = 1;
    LATBbits.LATB1 = 0;

    // Alarm output
    TRISBbits.TRISB7 = 0;
    LATBbits.LATB7 = 0;

    // Fan outputs RC4-RC7
    TRISC &= 0x0F;
    LATC &= 0x0F;

    // Warning LED
    TRISDbits.TRISD5 = 0;
    LATDbits.LATD5 = 0;
}


void Timer0_Init(void)
{
    /*
     * PIC18F46K22 Timer0:
     * 16-bit mode
     * Internal instruction clock, FOSC/4
     * 1:8 prescaler
     */

    T0CON = 0b10000010;

    // Approximately 100 ms at 8 MHz
    TMR0H = 0x9E;
    TMR0L = 0x58;

    INTCONbits.TMR0IF = 0;
    INTCON2bits.TMR0IP = 1;
    INTCONbits.TMR0IE = 1;

    RCONbits.IPEN = 1;

    INTCONbits.GIEH = 1;
    INTCONbits.GIEL = 1;
}


void __interrupt(high_priority) HighPriorityISR(void)
{
    if (INTCONbits.TMR0IF)
    {
        TMR0H = 0x9E;
        TMR0L = 0x58;

        INTCONbits.TMR0IF = 0;

        // Above 75 C
        if (temp_above_75)
        {
            high_temp_active = 1;
            cooling_period = 0;
            cooling_ticks = 0;
            led_tick = 0;

            FANS_ON();
            ALARM_ON();
            LED_ON();
        }

        // Ten-second cooldown
        else if (high_temp_active)
        {
            if (!cooling_period)
            {
                cooling_period = 1;
                cooling_ticks = 100;
                led_tick = 0;
            }

            FANS_ON();
            ALARM_ON();

            led_tick++;

            // Flash RD5 every 500 ms
            if (led_tick >= 5)
            {
                LED_TOGGLE();
                led_tick = 0;
            }

            if (cooling_ticks > 0)
            {
                cooling_ticks--;
            }
            else
            {
                high_temp_active = 0;
                cooling_period = 0;

                FANS_OFF();
                ALARM_OFF();
                LED_OFF();
            }
        }

        // Above 60 C
        else if (temp_above_60)
        {
            FANS_ON();
            ALARM_OFF();
            LED_OFF();
        }

        // Below 60 C
        else
        {
            FANS_OFF();
            ALARM_OFF();
            LED_OFF();
        }
    }
}


void main(void)
{
    /*
     * Select internal 8 MHz oscillator.
     * IRCF = 110 gives 8 MHz.
     * SCS = 10 selects the internal oscillator.
     */
    OSCCON = 0x62;

    IO_Init();
    Timer0_Init();

    while (1)
    {
        temperature =
            DS18B20_ReadTemperature(SENSOR1_ROM);

        if (temperature > 750)
        {
            temp_above_75 = 1;
        }
        else
        {
            temp_above_75 = 0;
        }

        if (temperature > 600)
        {
            temp_above_60 = 1;
        }
        else
        {
            temp_above_60 = 0;
        }

        __delay_ms(100);
    }
}
