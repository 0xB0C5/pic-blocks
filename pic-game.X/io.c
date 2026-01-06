#include "io.h"

uint8_t porta;
uint8_t portc;
void io_init(void) {
    // Disable latching.
    LATA = 0x00;
    LATC = 0x00;

    // Set sound and TFT pins to output.
    TRISA = 0x37 ^ PINA_SOUND;
    TRISC = 0x3f ^ (PINC_TFT_DC | PINC_TFT_RST | PINC_TFT_SCLK | PINC_TFT_MOSI);
    
    // Enable analog on sound pin only.
    ANSELC = 0x00; // PINA_SOUND;
    ANSELA = 0x00;

    // Enable weak pull-up on button pins.
    WPUA = PINA_BTNS;
    WPUC = PINC_BTNS;

    // No open drain only pins.
    ODCONA = 0x00;
    ODCONC = 0x00;

    // Disable slew on TFT pins for maximum write speed.
    SLRCONA = 0x1F;
    SLRCONC = 0x3F ^ (PINC_TFT_DC | PINC_TFT_SCLK | PINC_TFT_MOSI);

    // Default input level thresholds.
    // I'm not actually sure what this is.
    INLVLA = 0x3F;
    INLVLC = 0x3F;
    
    set_porta(0xff, 0);
    set_portc(0xff, 0);
}
