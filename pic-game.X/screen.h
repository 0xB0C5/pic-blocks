#include <stdint.h>
#include <stddef.h>

#ifndef _PIC_GAME_SCREEN
#define _PIC_GAME_SCREEN

#define TILEMAP_WIDTH 10
#define TILEMAP_HEIGHT 16
#define TILEMAP_SIZE (TILEMAP_WIDTH*TILEMAP_HEIGHT)

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 160

#define TILE_INDEX(x, y) (((uint8_t)((x) << 4)) | (y))

#define TILE_DATA_INDEX(id, rot, i) ((uint8_t)(((id) << (4)) | ((i) << 2) | (rot)))

extern uint8_t tilemap[TILEMAP_SIZE];

void screen_init(void);
void screen_update(void);
void screen_set_palette(const uint8_t *palette, uint8_t color_count);

#endif
