#include "game.h"
#include "packed_data.h"
#include "rand.h"
#include "screen.h"
#include "io.h"
#include <stdint.h>
#include <string.h>
#include <xc.h>

#define BTN_L  0x01
#define BTN_R  0x02
#define BTN_RL 0x04
#define BTN_RR 0x08
#define BTN_SD 0x10
#define BTN_HD 0x20 

#define STATUS_SELECT_SKIN 0
#define STATUS_NORMAL 1
#define STATUS_SCORING 2
#define STATUS_CLEARING 3
#define STATUS_MOVING 4
#define STATUS_DEAD 0xff

#define GRAVITY_INCREMENT 2

#define TETROMINO_DATA_INDEX(id,rot) ((uint8_t)((((uint8_t)(id << 2))|rot)<<3))

static uint8_t skin;

static uint8_t status;
static uint8_t playfield[TILEMAP_SIZE];
static uint8_t tetromino_id;
static uint8_t pending_tetromino_id;
static uint8_t tetromino_rotation;
static uint8_t tetromino_x;
static uint8_t tetromino_y;
static uint8_t gravity_counter;
static uint8_t gravity_speed;
static uint8_t frame_counter = 0;

static uint8_t move_src_y;
static uint8_t move_dest_y;

static uint8_t soft_drop_counter = 0;

static uint8_t buttons = 0;
static uint8_t presses = 0;

static uint8_t completed_rows[4];
static uint8_t completed_row_count;

static uint8_t level_row_count;
static uint16_t total_row_count;

static const uint8_t column_xs[] = { 4, 16, 28, 40, 52, 64, 76, 88, 100, 112 };
static const uint8_t row_ys[]    = { 0xff, 0xff, 0xff, 0, 12, 24, 36, 48, 60, 72, 84, 96, 108, 120, 132, 144 };

static const uint8_t preview_tiles_left[]  = { 4, 13, 10, 0, 10, 10, 15 };
static const uint8_t preview_tiles_right[] = { 4,  6,  8, 9, 11,  5,  5 };

static const uint8_t tetromino_data[] = {
0, 1, 1, 1, 2, 1, 3, 1, 2, 0, 2, 1, 2, 2, 2, 3, 3, 2, 2, 2, 1, 2, 0, 2, 1, 3, 1, 2, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 2, 1, 2, 0, 1, 0, 1, 1, 1, 2, 2, 2, 2, 1, 1, 1, 0, 1, 0, 2, 1, 2, 1, 1, 1, 0, 2, 0, 0, 1, 1, 1, 2, 1, 2, 2, 1, 0, 1, 1, 1, 2, 0, 2, 2, 1, 1, 1, 0, 1, 0, 0, 1, 2, 1, 1, 1, 0, 1, 0, 2, 0, 1, 1, 2, 1, 3, 0, 3, 1, 2, 0, 2, 1, 3, 2, 2, 2, 3, 1, 2, 1, 1, 2, 1, 1, 2, 2, 2, 1, 1, 0, 2, 0, 0, 1, 1, 1, 2, 1, 2, 2, 1, 0, 1, 1, 1, 2, 0, 2, 2, 1, 1, 1, 0, 1, 0, 0, 1, 2, 1, 1, 1, 0, 0, 1, 1, 1, 2, 1, 2, 1, 1, 0, 1, 1, 1, 2, 1, 2, 2, 1, 1, 1, 0, 1, 0, 1, 1, 2, 1, 1, 1, 0, 0, 0, 1, 0, 1, 1, 2, 1, 2, 0, 2, 1, 1, 1, 1, 2, 2, 2, 1, 2, 1, 1, 0, 1, 0, 2, 0, 1, 1, 1, 1, 0
};

static const uint8_t score_tiles[] = {
    'S'-' ',
    'C'-' ',
    'O'-' ',
    'R'-' ',
    'E'-' ',
};

#define SKIN_M 0
#define SKIN_D 1
#define SKIN_J 2
#define SKIN_K 3

const uint8_t tetromino_tile_data_m[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    1, 2, 3, 0, 16, 17, 18, 19, 8, 9, 10, 11, 12, 13, 14, 15,
    15, 12, 13, 14, 0, 1, 2, 3, 4, 5, 6, 7, 20, 21, 22, 23,
    17, 18, 19, 16, 2, 3, 0, 1, 21, 22, 23, 20, 12, 13, 14, 15,
    17, 18, 19, 16, 2, 3, 0, 1, 14, 15, 12, 13, 24, 25, 26, 27,
    1, 2, 3, 0, 28, 29, 30, 31, 32, 33, 34, 35, 12, 13, 14, 15,
    0, 1, 2, 3, 36, 37, 38, 39, 21, 22, 23, 20, 12, 13, 14, 15,
};


const uint8_t tetromino_tile_data_d[] = {
    0, 1, 2, 3, 4, 5, 4, 5, 4, 5, 4, 5, 2, 3, 0, 1, 1, 2, 3, 0, 6, 7, 8, 9, 4, 5, 4, 5, 2, 3, 0, 1, 1, 2, 3, 0, 0, 1, 2, 3, 4, 5, 4, 5, 9, 6, 7, 8, 7, 8, 9, 6, 8, 9, 6, 7, 6, 7, 8, 9, 9, 6, 7, 8, 7, 8, 9, 6, 2, 3, 0, 1, 0, 1, 2, 3, 9, 6, 7, 8, 1, 2, 3, 0, 10, 11, 12, 13, 14, 15, 16, 17, 2, 3, 0, 1, 0, 1, 2, 3, 8, 9, 6, 7, 6, 7, 8, 9, 2, 3, 0, 1
};

const uint8_t tetromino_tile_data_j[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 4, 5, 6, 7, 8, 9, 10, 11, 1, 2, 3, 0, 12, 13, 14, 15, 4, 5, 6, 7, 8, 9, 10, 11, 11, 8, 9, 10, 0, 1, 2, 3, 4, 5, 6, 7, 15, 12, 13, 14, 13, 14, 15, 12, 14, 15, 12, 13, 12, 13, 14, 15, 15, 12, 13, 14, 16, 17, 18, 19, 8, 9, 10, 11, 0, 1, 2, 3, 15, 12, 13, 14, 1, 2, 3, 0, 20, 21, 22, 23, 4, 24, 25, 7, 8, 9, 10, 11, 10, 11, 8, 9, 14, 15, 12, 13, 19, 16, 17, 18, 2, 3, 0, 1
};

const uint8_t skin_data_indices[] = {
    DATA_INDEX_TILES_M,
    DATA_INDEX_TILES_D,
    DATA_INDEX_TILES_J,
    DATA_INDEX_K,
};

const uint8_t *skin_tile_data_ptrs[] = {
    tetromino_tile_data_m,
    tetromino_tile_data_d,
    tetromino_tile_data_j,
    NULL,
};

const uint8_t palette_m[] = {
    214,84,0,0,239,54,253,121,162,84,187,202,158,221
};
const uint8_t palette_d[] = {
    214,84,0,0,187,202,98,5
};
const uint8_t palette_j[] = {
    214,84,0,0,123,239,255,128
};
const uint8_t palette_k[] = {
    214,84,0,0,123,239,198,24,255,255,49,134,16,162
};

const uint8_t *skin_palettes[] = {
    palette_m,
    palette_d,
    palette_j,
    palette_k,
};

const uint8_t skin_palette_color_counts[] = {
    7, 4, 4, 7
};

void randomize_pending_tetromino()
{
    do
    {
        pending_tetromino_id = rand8() & 0x7;
    } while (pending_tetromino_id == 7);
}


uint8_t get_tile_id(uint8_t tetromino_id, uint8_t tetromino_rotation, uint8_t i) {
    uint8_t tile_data_index = TILE_DATA_INDEX(tetromino_id, tetromino_rotation, i);
    const uint8_t *tile_data = skin_tile_data_ptrs[skin];
    uint8_t tile_id = skin_data_indices[skin];
    if (tile_data != NULL)
    {
        tile_id += tile_data[tile_data_index];
    }
    return tile_id;
}

void game_init(void)
{
    status = STATUS_SELECT_SKIN;
    for (uint8_t i = 0; i < TILEMAP_SIZE; i++)
    {
        playfield[i] = 0;
    }
    tetromino_id = 0xff;
    tetromino_rotation = 0;
    tetromino_x = 0;
    tetromino_y = 0;
    pending_tetromino_id = 0xff;
    
    gravity_speed = 3;
    gravity_counter = 0;
    
    level_row_count = 0;
    total_row_count = 0;
}

uint8_t tetromino_intersects()
{
    uint8_t data_index = TETROMINO_DATA_INDEX(tetromino_id,tetromino_rotation);
    for (uint8_t i = 0; i < 4; i++)
    {
        uint8_t x = tetromino_x + tetromino_data[data_index++];
        uint8_t y = tetromino_y + tetromino_data[data_index++];
        
        if (x >= TILEMAP_WIDTH
            || y >= TILEMAP_HEIGHT)
        {
            return 1;
        }
        
        uint8_t tile_index = TILE_INDEX(x,y);
        if (playfield[tile_index])
        {
            return 1;
        }
    }
    
    return 0;
}

void place_tetromino()
{
    uint8_t data_index = TETROMINO_DATA_INDEX(tetromino_id,tetromino_rotation);
    for (uint8_t i = 0; i < 4; i++)
    {
        uint8_t x = tetromino_x + tetromino_data[data_index++];
        uint8_t y = tetromino_y + tetromino_data[data_index++];

        if (x >= TILEMAP_WIDTH
            || y >= TILEMAP_HEIGHT)
        {
            status = STATUS_DEAD;
            continue;
        }

        uint8_t tile_index = TILE_INDEX(x,y);
        if (playfield[tile_index])
        {
            status = STATUS_DEAD;
        }
        
        playfield[tile_index] = get_tile_id(tetromino_id, tetromino_rotation, i);
    }
    
    tetromino_id = 0xff;
    
    if (status == STATUS_NORMAL)
    {
        status = STATUS_SCORING;
    }
}

void game_update_select_skin(void)
{
    if (presses & (BTN_R | BTN_RR)) {
        skin++;
    }
    
    if (presses & (BTN_L | BTN_RL)) {
        skin--;
    }
    
    skin &= 3;
    
    screen_set_palette(skin_palettes[skin], skin_palette_color_counts[skin]);
    
    if (presses & (BTN_HD | BTN_SD)) {
        // Hard drop or soft drop was pressed.
        // Transition to gameplay.
        tetromino_id = 0xff;
        tetromino_x = 0;
        tetromino_y = 0;
        status = STATUS_NORMAL;
        
        randomize_pending_tetromino();
        
        return;
    }
    
    // Use the active tetromino to show the skin.
    tetromino_id = 0;
    tetromino_x = 3;
    tetromino_y = 3;
}

void game_update_normal(void)
{
    if (tetromino_id == 0xff)
    {
        // NES rules - randomize, then if we got a repeat, randomize once more.
        tetromino_id = pending_tetromino_id;
        randomize_pending_tetromino();
        if (pending_tetromino_id == tetromino_id)
        {
            randomize_pending_tetromino();
        }
        
        tetromino_x = 3;
        tetromino_y = 0;
        tetromino_rotation = 0;
        return;
    }
    
    // Soft drop
    if (presses & BTN_SD)
    {
        soft_drop_counter = 1;
    }
    
    if (buttons & BTN_SD)
    {
        if (soft_drop_counter)
        {
            if (--soft_drop_counter == 0)
            {
                // TODO : score
                gravity_counter = 255;
                soft_drop_counter = 2;
            }
        }
    }
    else
    {
        soft_drop_counter = 0;
    }
    
    if (presses & BTN_HD)
    {
        do
        {
            tetromino_y++;
        } while (!tetromino_intersects());
        tetromino_y--;
        gravity_counter = 255;
        // TODO : score
    }
    
    if (presses & BTN_L)
    {
        tetromino_x--;
        if (tetromino_intersects()) tetromino_x++;
    }
    
    if (presses & BTN_R)
    {
        tetromino_x++;
        if (tetromino_intersects()) tetromino_x--;
    }
    
    if (presses & BTN_RL)
    {
        tetromino_rotation = (tetromino_rotation-1) & 3;
        if (tetromino_intersects()) tetromino_rotation = (tetromino_rotation+1) & 3;
    }
    
    if (presses & BTN_RR)
    {
        tetromino_rotation = (tetromino_rotation+1) & 3;
        if (tetromino_intersects()) tetromino_rotation = (tetromino_rotation-1) & 3;
    }
    
    uint8_t old_gravity_counter = gravity_counter;
    gravity_counter += gravity_speed;
    gravity_counter++;
    
    if (gravity_counter <= old_gravity_counter)
    {
        tetromino_y++;
        
        if (tetromino_intersects())
        {
            tetromino_y--;
            place_tetromino();
            soft_drop_counter = 0;
            return;
        }
    }
}

void draw_active_tetromino()
{
    uint8_t data_index = TETROMINO_DATA_INDEX(tetromino_id,tetromino_rotation);
        
    for (uint8_t i = 0; i < 4; i++)
    {
        uint8_t x = tetromino_x + tetromino_data[data_index++];
        uint8_t y = tetromino_y + tetromino_data[data_index++];

        if (x >= TILEMAP_WIDTH
            || y >= TILEMAP_HEIGHT)
        {
            continue;
        }

        uint8_t tile_index = TILE_INDEX(x,y);
        uint8_t tile_data_index = TILE_DATA_INDEX(tetromino_id, tetromino_rotation, i);
        tilemap[tile_index] = get_tile_id(tetromino_id, tetromino_rotation, i);
    }
}

void draw_inactive_tetromino(uint8_t id, uint8_t rotation, uint8_t x, uint8_t y, uint8_t tile)
{
    uint8_t data_index = TETROMINO_DATA_INDEX(id, rotation);

    for (uint8_t i = 0; i < 4; i++)
    {
        uint8_t x = tetromino_x + tetromino_data[data_index++];
        uint8_t y = tetromino_y + tetromino_data[data_index++];

        if (x >= TILEMAP_WIDTH
            || y >= TILEMAP_HEIGHT)
        {
            continue;
        }

        uint8_t tile_index = TILE_INDEX(x,y);
        uint8_t tile_data_index = TILE_DATA_INDEX(id, rotation, i);
        tilemap[tile_index] = tile;
    }
}

uint8_t is_row_completed(uint8_t y) {
    for (uint8_t x = 0; x < TILEMAP_WIDTH; x++)
    {
        uint8_t tile_index = TILE_INDEX(x, y);
        if (!tilemap[tile_index]) {
            return 0;
        }
    } 
    return 1;
}

uint8_t is_row_clear(uint8_t y) {
    for (uint8_t x = 0; x < TILEMAP_WIDTH; x++)
    {
        uint8_t tile_index = TILE_INDEX(x, y);
        if (tilemap[tile_index]) {
            return 0;
        }
    } 
    return 1;
}

void game_update_scoring() {
    // Find any completed lines.
    completed_row_count = 0;
    for (uint8_t y = 0; y < TILEMAP_HEIGHT; y++)
    {
        if (is_row_completed(y))
        {
            completed_rows[completed_row_count++] = y;
        }
    }
    
    if (completed_row_count == 0)
    {
        // Continue gameplay.
        status = STATUS_NORMAL;
        return;
    }
    
    // TODO : add to score.
    
    total_row_count += completed_row_count;
    if (total_row_count > 999) total_row_count = 999;
    level_row_count += completed_row_count;
    if (level_row_count >= 10) {
        level_row_count -= 10;
        // Increase gravity.
        if (gravity_speed >= 0xff - GRAVITY_INCREMENT) {
            gravity_speed = 0xff;
        } else {
            gravity_speed += GRAVITY_INCREMENT;
        }
    }
    
    tetromino_x = 0;
    status = STATUS_CLEARING;
}

void game_update_clearing() {
    if (frame_counter & 1) return;
    
    for (uint8_t i = 0; i < completed_row_count; i++)
    {
        uint8_t y = completed_rows[i];
        uint8_t tile_index = TILE_INDEX(tetromino_x, y);
        playfield[tile_index] = 0;
    }
    
    tetromino_x++;
    if (tetromino_x >= TILEMAP_WIDTH)
    {
        move_src_y = TILEMAP_HEIGHT-1;
        move_dest_y = TILEMAP_HEIGHT-1;
        status = STATUS_MOVING;
    }
}

void game_update_moving() {
    // Move src_y up to next non-clear row.
    while (move_src_y < TILEMAP_HEIGHT && is_row_clear(move_src_y))
    {
        move_src_y--;
    }
    
    if (move_src_y >= TILEMAP_HEIGHT)
    {
        // Reached the end.
        status = STATUS_NORMAL;
        return;
    }
    
    if (move_src_y != move_dest_y)
    {
        for (uint8_t x = 0; x < TILEMAP_WIDTH; x++)
        {
            uint8_t src_idx = TILE_INDEX(x, move_src_y);
            uint8_t dest_idx = TILE_INDEX(x, move_dest_y);
            playfield[dest_idx] = playfield[src_idx];
            playfield[src_idx] = 0;
        }
    }
    
    move_src_y--;
    move_dest_y--;
}

void game_update_dead(void)
{
    if (presses & (BTN_SD|BTN_HD)) {
        // Start a new game.
        game_init();
        return;
    }
    
    uint8_t score_x = 5;
    uint8_t remaining_score = total_row_count;
    for (int i = 0; i < 3; i++) {
        playfield[TILE_INDEX(5-i, 7)] = remaining_score % 10 + ('0' - ' ');
        remaining_score /= 10;
    }
    
    for (int i = 0; i < sizeof(score_tiles)/sizeof(score_tiles[0]); i++) {
        playfield[TILE_INDEX(2+i, 6)] = score_tiles[i];
    }
}

void game_update(void)
{
    uint8_t new_buttons = 0;
    if ((PORTA & PINA_BTN_SD) == 0)
    {
        new_buttons |= BTN_SD;
    }
    
    if ((PORTA & PINA_BTN_HD) == 0)
    {
        new_buttons |= BTN_HD;
    }
    
    if ((PORTC & PINC_BTN_L) == 0)
    {
        new_buttons |= BTN_L;
    }
    
    if ((PORTC & PINC_BTN_R) == 0)
    {
        new_buttons |= BTN_R;
    }
    
    if ((PORTA & PINA_BTN_RL) == 0)
    {
        new_buttons |= BTN_RL;
    }
    
    if ((PORTA & PINA_BTN_RR) == 0)
    {
        new_buttons |= BTN_RR;
    }
    
    presses = new_buttons & ~buttons;
    buttons = new_buttons;
    
    switch (status)
    {
        case STATUS_SELECT_SKIN:
            game_update_select_skin();
            break;
        
        case STATUS_NORMAL:
            game_update_normal();
            break;
        
        case STATUS_SCORING:
            game_update_scoring();
            break;
            
        case STATUS_CLEARING:
            game_update_clearing();
            break;
            
        case STATUS_MOVING:
            game_update_moving();
            break;
            
        case STATUS_DEAD:
            // TODO
            game_update_dead();
            break;
        
        default:
            playfield[TILEMAP_HEIGHT-1] = 'E' - ' ';
            break;
    }

    // Copy playfield into tilemap.
    memcpy(tilemap, playfield, sizeof(tilemap));

    // Draw dynamic elements.
    if (pending_tetromino_id != 0xff)
    {
        tilemap[(TILEMAP_WIDTH-2)*TILEMAP_HEIGHT+3] = preview_tiles_left[pending_tetromino_id];
        tilemap[(TILEMAP_WIDTH-1)*TILEMAP_HEIGHT+3] = preview_tiles_right[pending_tetromino_id];
    }
    
    if (tetromino_id != 0xff)
    {
        uint8_t original_y = tetromino_y;

        do
        {
            tetromino_y++;
        } while (!tetromino_intersects());
        tetromino_y--;

        if (tetromino_y != original_y)
        {
            draw_inactive_tetromino(tetromino_id, tetromino_rotation, tetromino_x, tetromino_y, '#' - ' ');
            tetromino_y = original_y;
        }

        draw_active_tetromino();
    }

    frame_counter++;
}
