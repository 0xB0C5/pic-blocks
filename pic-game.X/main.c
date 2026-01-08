#include "mcc_generated_files/mcc.h"
#include "screen.h"
#include "io.h"
#include "game.h"
#include "timer.h"
#include "audio.h"

void main(void)
{
    SYSTEM_Initialize();

    io_init();
    screen_init();
   
    audio_init();
    
    game_init();
    timer_init();
    
    while (1)
    {
        game_update();
        timer_wait_frame();
        screen_update();
    }
}
