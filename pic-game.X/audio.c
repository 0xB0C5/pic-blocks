#include "audio.h"
#include "io.h"

void audio_init()
{
    T1CON = 0x31; // Prescale 1:8, enable timer
    T1CLK = 0x01; // FOSC/4
    PIE4 = 0x01; // TMR1IE
    INTCON = 0xc0; // PEIE, GIE
    /*
    CCP3CON = 0x80;
    
    PWM3CON = 0;
    PR2 = 0x1f;
    PWM3DCH = 0x0f;
    
    PIR4 = 0;
    T2CLKCON = 0x02; // FOSC
    T2CON = 0x80; // Enable

    while (!(PIR4 & 2));

    TRISA = 0x37 ^ PINA_SOUND;
    RA0PPS = 0x0b; // PWM3OUT
    PWM3CON = 0x80; // Enable PWM3
    
    while(1);
    */
}

uint8_t pitch_hi = 0xf8;
uint8_t pitch_lo = 0x00;
uint8_t level = 0;
/*
uint8_t pitch_counter = 0;
uint8_t volume = 1;
uint8_t on = 0;
 * */
void __interrupt () isr(uint8_t id)
{
    // Set the timer.
    TMR1H = pitch_hi;
    TMR1L = pitch_lo;

    // Flip the level.
    //level ^= PINA_SOUND;
    set_porta(PINA_SOUND, level);
    
    /*
    on ^= 1;
    
    PWM3DCH = on ? volume : 0;

    if (pitch_lo++ == 0)
    {
        if (pitch_hi++ == 0xff)
        {
            pitch_hi = 0xf0;
            
            if (volume++ == 0x10)
            {
                volume = 1;
            }
        }
    }
    */
    // Acknowledge the interrupt.
    TMR1IF = 0;
}
