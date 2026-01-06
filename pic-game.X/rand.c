#include "rand.h"

uint8_t rand_a = 0;
uint8_t rand_b = 1;

uint8_t rand8()
{
    uint8_t carry_a = rand_a & 1;
    rand_a >>= 1;
    uint8_t carry_b = rand_b & 0x80;
    rand_b <<= 1;
    rand_b |= carry_a;
    
    if (carry_b)
    {
        rand_a ^= 0xb4;
    }
    
    return rand_a ^ rand_b;
}


void mix_seed(uint8_t value)
{
    rand_a ^= value;
    if ((rand_a | rand_b) == 0)
    {
        rand_a = 1;
    }
}
