#include "timer.h"
#include <xc.h>

void timer_init()
{
    T0CON1 = 0b01000101;
    T0CON0 = 0b10010000;
}

void timer_wait_frame()
{
    do
    {
        TMR0L;
    } while (TMR0H < 33);
    TMR0H = 0;
    TMR0L = 0;
}
