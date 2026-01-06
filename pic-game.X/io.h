#ifndef _PIC_GAME_PRG_IO
#define _PIC_GAME_PRG_IO

#include <xc.h>

// TFT display
#define PINC_TFT_SCLK  0x20 // SCK
#define PINC_TFT_MOSI  0x10 // SDA
#define PINC_TFT_DC    0x08 // A0
#define PINC_TFT_RST   0x04 // RESET

// Buttons on PORTC
#define PINC_BTN_R 0x01
#define PINC_BTN_L 0x02
#define PINC_BTNS   (PINC_BTN_L | PINC_BTN_R)

// Buttons on PORTA
#define PINA_BTN_RR 0x02
#define PINA_BTN_RL 0x04
#define PINA_BTN_SD 0x10
#define PINA_BTN_HD 0x20
#define PINA_BTNS   (PINA_BTN_SD|PINA_BTN_HD|PINA_BTN_RR|PINA_BTN_RL)

// Speaker
#define PINA_SOUND 0x01

extern uint8_t porta;
extern uint8_t portc;

void io_init(void);

static inline void set_portc(uint8_t mask, uint8_t value) {
    portc = (portc & ~mask) | (value & mask);
    PORTC = portc;
}

static inline void set_porta(uint8_t mask, uint8_t value) {
    porta = (porta & ~mask) | (value & mask);
    PORTA = porta;
}

#endif
