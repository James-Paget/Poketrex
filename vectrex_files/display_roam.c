#include "vectrex.h"
#include "vectrex/bios.h"
#include "cmoc.h"

#include "game_constants.h"
#include "core_structs.h"

#include "display_roam.h"

void display_roam_screen() {
    reset_beam();
    set_text_size(-5, 40);
    intensity(0x5f);
    print_str_c(0,-40, (char*)"PRESS BUTTON 1 FOR ENCOUNTER");
}