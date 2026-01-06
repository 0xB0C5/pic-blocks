#include "screen.h"
#include "packed_data.h"
#include "prg_read.h"
#include "io.h"
#include <xc.h>

#define ST77XX_SWRESET 0x01
#define ST77XX_RDDID 0x04
#define ST77XX_RDDST 0x09

#define ST77XX_SLPIN 0x10
#define ST77XX_SLPOUT 0x11
#define ST77XX_PTLON 0x12
#define ST77XX_NORON 0x13

#define ST77XX_INVOFF 0x20
#define ST77XX_INVON 0x21
#define ST77XX_DISPOFF 0x28
#define ST77XX_DISPON 0x29
#define ST77XX_CASET 0x2A
#define ST77XX_RASET 0x2B
#define ST77XX_RAMWR 0x2C
#define ST77XX_RAMRD 0x2E

#define ST77XX_PTLAR 0x30
#define ST77XX_TEOFF 0x34
#define ST77XX_TEON 0x35
#define ST77XX_MADCTL 0x36
#define ST77XX_COLMOD 0x3A

#define ST77XX_MADCTL_MY 0x80
#define ST77XX_MADCTL_MX 0x40
#define ST77XX_MADCTL_MV 0x20
#define ST77XX_MADCTL_ML 0x10
#define ST77XX_MADCTL_RGB 0x00

#define ST77XX_RDID1 0xDA
#define ST77XX_RDID2 0xDB
#define ST77XX_RDID3 0xDC
#define ST77XX_RDID4 0xDD

#define ST7735_MADCTL_BGR 0x08
#define ST7735_MADCTL_MH 0x04

#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR 0xB4
#define ST7735_DISSET5 0xB6

#define ST7735_PWCTR1 0xC0
#define ST7735_PWCTR2 0xC1
#define ST7735_PWCTR3 0xC2
#define ST7735_PWCTR4 0xC3
#define ST7735_PWCTR5 0xC4
#define ST7735_VMCTR1 0xC5

#define ST7735_PWCTR6 0xFC

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

uint8_t tilemap[TILEMAP_SIZE];
uint8_t old_tilemap[TILEMAP_SIZE];

const uint8_t init_commands[] = {
  ST7735_FRMCTR1, 3, 0x01, 0x2C, 0x2D,
  ST7735_FRMCTR2, 3, 0x01, 0x2C, 0x2D,
  ST7735_FRMCTR3, 6, 0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D,
  ST7735_INVCTR,  1, 0x07,
  ST7735_PWCTR1,  3, 0xA2, 0x02, 0x84,
  ST7735_PWCTR2,  1, 0xC5,
  ST7735_PWCTR3,  2, 0x0A, 0x00,
  ST7735_PWCTR4,  2, 0x8A, 0x2A,
  ST7735_PWCTR5,  2, 0x8A, 0xEE,
  ST7735_VMCTR1,  1, 0x0E,
  ST77XX_INVOFF,  0,
  ST77XX_MADCTL,  1, 0xC8,
  ST77XX_COLMOD,  1, 0x05,
  ST77XX_CASET,   4, 0x00, 0x00, 0x00, 0x7F,
  ST77XX_RASET,   4, 0x00, 0x00, 0x00, 0x9F,
  ST7735_GMCTRP1, 16,
      0x02, 0x1c, 0x07, 0x12,
      0x37, 0x32, 0x29, 0x2d,
      0x29, 0x25, 0x2B, 0x39,
      0x00, 0x01, 0x03, 0x10,
  ST7735_GMCTRN1, 16,
      0x03, 0x1d, 0x07, 0x06,
      0x2E, 0x2C, 0x29, 0x2D,
      0x2E, 0x2E, 0x37, 0x3F,
      0x00, 0x00, 0x02, 0x10,
  ST77XX_NORON, 0,
  0,
};

void spin_delay(void) {
    for (uint32_t i = 0; i < 400000L; i++) {
        asm("");
    }
}

void screen_write_byte(uint8_t b)
{
    SSP1BUF = b;
    while ((SSP1STAT & 1) == 0);
    // SSP1BUF;
}

uint8_t palette_left[8];
uint8_t palette_right[8];

void screen_set_palette(const uint8_t *palette, uint8_t color_count)
{
    for (uint8_t i = 0; i < color_count; i++)
    {
        palette_left[i] = *(palette++);
        palette_right[i] = *(palette++);
    }
}

void screen_draw_tile(uint8_t x, uint8_t y, uint8_t c) {
    // Compute data address.
    uint16_t data_addr = PACKED_DATA_START + c;

    data_addr = read_prg_word(data_addr);
    uint16_t w = read_prg_word(data_addr++);

    uint8_t bpp = (w & 0x03) + 1;
    w >>= 2;
    uint8_t bit_count = 12;

    // Column Address Set
    set_portc(PINC_TFT_DC, 0);
    screen_write_byte(0x2a);
    set_portc(PINC_TFT_DC, PINC_TFT_DC);
    screen_write_byte(0);
    screen_write_byte(x);
    screen_write_byte(0);
    screen_write_byte(x + 11);

    // Row Address Set
    set_portc(PINC_TFT_DC, 0);
    screen_write_byte(0x2b);
    set_portc(PINC_TFT_DC, PINC_TFT_DC);
    screen_write_byte(0);
    screen_write_byte(y);
    screen_write_byte(0);
    screen_write_byte(y + 11);
    
    // Draw
    set_portc(PINC_TFT_DC, 0);
    screen_write_byte(ST77XX_RAMWR);
    set_portc(PINC_TFT_DC, PINC_TFT_DC);
    
    for (uint8_t pixel_index = 0; pixel_index < 144; pixel_index++) {
        uint8_t palette_index = 0;
        // TODO : this can be sped up when bit_count >= bpp.
        for (uint8_t i = 0; i < bpp; i++)
        {
            palette_index <<= 1;
            palette_index |= w & 1;
            w >>= 1;
            bit_count--;
            if (bit_count == 0)
            {
                w = read_prg_word(data_addr++);
                bit_count = 14;
            }
        }

        screen_write_byte(palette_left[palette_index]);
        screen_write_byte(palette_right[palette_index]);
    }
}

void screen_init(void) {
    SSP1CON1 = 0x00;
    SSP1STAT = 0x40;
    RC5PPS = 0x15; // RC5 = SCK
    RC4PPS = 0x16; // RC4 = SDO (serial data out)
    // Enable SSP1
    SSP1CON1 = 0x20;

    spin_delay(); // 100

    // Disable reset.
    set_portc(PINC_TFT_RST, PINC_TFT_RST);
    
    spin_delay(); // 100

    // TODO : SPI.beginTransaction()?
    
    screen_write_byte(ST77XX_SWRESET);
    spin_delay(); // 150
    
    screen_write_byte(ST77XX_SLPOUT);
    spin_delay(); // 255
  
    const uint8_t *cmd_ptr = init_commands;
    while (*cmd_ptr) {
        set_portc(PINC_TFT_DC, 0);
        uint8_t cmd = *cmd_ptr;
        screen_write_byte(*cmd_ptr);
        cmd_ptr++;
        uint8_t arg_count = *cmd_ptr;
        cmd_ptr++;
        if (arg_count) {
            set_portc(PINC_TFT_DC, PINC_TFT_DC);
            for (int i = 0; i < arg_count; i++) {
              uint8_t arg = *cmd_ptr;
              cmd_ptr++;
              screen_write_byte(arg);
            }
        }
    }

    set_portc(PINC_TFT_DC, 0);

    spin_delay(); // 10
    screen_write_byte(ST77XX_DISPON);

    spin_delay(); // 100
    screen_write_byte(ST77XX_MADCTL);

    set_portc(PINC_TFT_DC, PINC_TFT_DC);
    screen_write_byte(0xC0);
    set_portc(PINC_TFT_DC, 0);
    
    // Clear the screen.
    set_portc(PINC_TFT_DC, 0);
    screen_write_byte(ST77XX_RAMWR);
    set_portc(PINC_TFT_DC, PINC_TFT_DC);
    
    for (int i = 0; i < SCREEN_WIDTH*SCREEN_HEIGHT; i++)
    {
        screen_write_byte(0xff);
        screen_write_byte(0xff);
    }
    
    for (uint8_t i = 0; i < TILEMAP_SIZE; i++)
    {
        tilemap[i] = 0xff;
        old_tilemap[i] = 0xff;
    }
}

void screen_update()
{
    uint8_t x = 4;
    uint8_t y = 4;
    for (uint8_t i = 0; i < TILEMAP_SIZE; i++)
    {
        if (tilemap[i] != old_tilemap[i])
        {
            if (y >= 3*12)
            {
                screen_draw_tile(x, y-3*12, tilemap[i]);
            }

            old_tilemap[i] = tilemap[i];
        }
        
        y += 12;
        if (y >= 12*TILEMAP_HEIGHT)
        {
            y = 4;
            x += 12;
        }
    }
}