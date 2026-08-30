#include "vectrex.h"
#include "vectrex/bios.h"
#include "cmoc.h"

#include "game_constants.h"
#include "core_structs.h"
#include "poke_fixed_catalogue.h"

#include "calculate_roam.h"

void calculate_roam_screen(uint8_t *screen_mode, poke_details_flexible *enemy_poke_party) {
    calculate_roam_buttons_TEMP(screen_mode, enemy_poke_party);
}

void calculate_roam_buttons_TEMP(uint8_t *screen_mode, poke_details_flexible *enemy_poke_party) {
    /*
    . Buttons to control broad action selection
    . Move Vs PokeSwitch Vs Bag Vs Run
    */
    uint8_t buttons = read_buttons();
    if(buttons & JOY1_BTN1_MASK) {          // Generate encounter + good to combat screen
        generate_wild_encounter(enemy_poke_party);
        *screen_mode = 1;   // Switch to combat screen

        reset_beam();
        print_str_c(0, 0, (char*)"PRESSED 1 P1");
        set_scale(128);
    }
}


void generate_wild_encounter(poke_details_flexible *enemy_poke_party) {
    poke_details_flexible poke_flexible_starly = {
        4,

        20,
        18,
        1,
        0,
        2,
        8,
        1,
        5,
        24,

        {1,2,0,0},

        0,

        &(POKE_FIXED_CATALOGUE[2]),
    };
    enemy_poke_party[0] = poke_flexible_starly;
}

// ###
// ###
// ### NEED TO FIX POKE COMBAT CALC WHERE IT USES FRIENDLY/ENEMY FOR ALL TEXT, NOT FIRST OR SECOND
// ###
// ###