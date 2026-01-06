#ifndef _PIC_GAME_PRG_READ
#define _PIC_GAME_PRG_READ
#include <xc.h>
#include <stdint.h>

inline uint16_t read_prg_word(uint16_t address)
{
    // Clear the NVMREGS bit of the NVMCON1 register
    // to access the program memory.
    NVMCON1 = 0x00;
    
    // Write the address into the NVMADRH:NVMADRL pair
    NVMADRH = address >> 8;
    NVMADRL = address & 0xff;
    
    // Set the RD bit of NVMCON1 to initiate the read.
    NVMCON1 = 0x01;
    
    // Read NVMDATH:NVMDATL pair
    return (uint16_t)((NVMDATH << 8) | NVMDATL);
}

#endif
