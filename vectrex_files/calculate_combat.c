#include "vectrex.h"
#include "vectrex/bios.h"
#include "cmoc.h"

#include "game_constants.h"
#include "core_structs.h"

#include "poke_fixed_catalogue.h"

#include "calculate_combat.h"

void calculate_combat_screen(poke_details_flexible *friendly_poke_party, poke_details_flexible *enemy_poke_party, uint8_t *staging, uint8_t *friendly_active_action, uint8_t *enemy_active_action, uint8_t *friendly_active_poke_index, uint8_t *enemy_active_poke_index, uint8_t *t1) {
    switch(*staging) {
        case 0:     // Select action broad
            calculate_combat_buttons_actionSelection(t1, staging);
            break;
        case 1:     // Select move action
            calculate_combat_buttons_moveSelection(friendly_poke_party, friendly_active_action, friendly_active_poke_index, t1, staging);
            break;
        case 2:     // Perform combat
            break;
        // ...
        default:
            break;
    }
}

void calculate_combat_buttons_actionSelection(uint8_t *friendly_hovered_action, uint8_t *staging) {
    /*
    . Buttons to control broad action selection
    . Move Vs PokeSwitch Vs Bag Vs Run
    */
    uint8_t buttons = read_buttons();
    if(buttons & JOY1_BTN1_MASK) {              // Cycle options
        *friendly_hovered_action = (*friendly_hovered_action +1)%4;

        // ###
        // ### NOTE THIS IS TERRIBLE FOR PERFORMANCE
        // ###      BUT IS GOOD FOR BUG-FIXING AND NOT A BIG ISSUE FOR 1 TIME USES
        // ###
        reset_beam();
        print_str_c(0, 0, (char*)"PRESSED 1 P1");
        set_scale(128);
    }

    if(buttons & JOY1_BTN2_MASK) {              // Select option
        switch(*friendly_hovered_action) {  // Go to required staging for this selection
            case 0:     // Fight
                *staging = 1;
                *friendly_hovered_action = 0;
                break;
            case 1:     // Poke
                break;
            case 2:     // Bag
                break;
            case 3:     // Run
                break;
        }

        reset_beam();
        print_str_c(0, 0, (char*)"PRESSED 2");
        set_scale(128);
    }
}
void calculate_combat_buttons_moveSelection(poke_details_flexible *friendly_poke_party, uint8_t *friendly_active_action, uint8_t *friendly_active_poke_index, uint8_t *friendly_hovered_move, uint8_t *staging) {
    /*
    . Buttons to control specific move action selection
    . Moves 1-4 selection
    */
    uint8_t buttons = read_buttons();
    if(buttons & JOY1_BTN1_MASK) {              // Cycle options
        *friendly_hovered_move = (*friendly_hovered_move +1)%4;

        // ###
        // ### NOTE THIS IS TERRIBLE FOR PERFORMANCE
        // ###      BUT IS GOOD FOR BUG-FIXING AND NOT A BIG ISSUE FOR 1 TIME USES
        // ###
        reset_beam();
        print_str_c(0, 0, (char*)"PRESSED 1");
        set_scale(128);
    }

    if(buttons & JOY1_BTN2_MASK) {              // Select option
        if( (0 <= *friendly_active_poke_index) && (*friendly_active_poke_index < POKE_PARTY_LENGTH) ) {
            *friendly_active_action = *friendly_hovered_move +1;    // Select move
            *friendly_hovered_move = 0;                             // Reset the temporary hover variable
            *staging = 2;                                           // Move staging
        }

        reset_beam();
        print_str_c(0, 0, (char*)"PRESSED 2");
        set_scale(128);
    }

    if(buttons & JOY1_BTN3_MASK) {              // Back out of options
        *staging = 0;
        *friendly_hovered_move = 0;     // Reset the temporary hover variable

        reset_beam();
        print_str_c(0, 0, (char*)"PRESSED 3");
        set_scale(128);
    }
}




void calculate_combat_buttons(poke_details_flexible *friendly_poke_party, poke_details_flexible *enemy_poke_party) {
    uint8_t buttons = read_buttons();
    if(buttons & JOY1_BTN1_MASK) {              // Button 1 in combat == Cycle options
        poke_details_flexible poke_flexible_starly = {
            3,

            14,
            7,
            1,
            0,
            3,
            1,
            5,

            {3,5,0,4},

            0,

            &(POKE_FIXED_CATALOGUE[2]),
        };
        friendly_poke_party[0] = poke_flexible_starly;

        reset_beam();
        print_str_c(0, 0, (char*)"PRESSED 1");
        set_scale(128);
    }

    if(buttons & JOY1_BTN2_MASK) {              // Button 2 in combat == Select option
        poke_details_flexible poke_flexible_charmander = {
            3,

            14,
            13,
            1,
            0,
            3,
            1,
            5,

            {3,5,0,4},

            0,

            &(POKE_FIXED_CATALOGUE[1]),
        };
        friendly_poke_party[0] = poke_flexible_charmander;

        reset_beam();
        print_str_c(0, 0, (char*)"PRESSED 2");
        set_scale(128);
    }

    if(buttons & JOY1_BTN3_MASK) {              // Button 3 in combat == Back out of options
        //pass

        reset_beam();
        print_str_c(0, 0, (char*)"PRESSED 3");
        set_scale(128);
    }
}