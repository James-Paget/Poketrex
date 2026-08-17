#include "vectrex.h"
#include "vectrex/bios.h"
#include "cmoc.h"

#include "game_constants.h"
#include "core_structs.h"

#include "poke_fixed_catalogue.h"
#include "poke_move_catalogue.h"

#include "calculate_combat.h"

void calculate_combat_screen(poke_details_flexible *friendly_poke_party, poke_details_flexible *enemy_poke_party, uint8_t *staging, uint8_t *stage_timer, uint8_t *stage_speed, uint8_t *friendly_active_action, uint8_t *enemy_active_action, uint8_t *friendly_active_poke_index, uint8_t *enemy_active_poke_index, int8_t *poke_first_counter, int8_t *poke_second_counter, uint8_t *poke_first_counter_type, uint8_t *poke_second_counter_type, uint8_t *is_critical, uint8_t *is_miss, uint8_t *timer, uint8_t *t1) {
    /*
    . poke_XXX_counter_type = Which type of value the poke_XXX_counter is tracking, e.g.;
        0 = None
        1 = HP
        2 = DEF
        3 = SP_DEF
        4 = ATK
        5 = SP_ATK
        6 = SPD
        ...
    */
    // ###
    // ### *NOTE; Issue with first/second selection possible for moves which lower SPD, resulting in a poke taking both the first and second turns since this is recalculated each tick
    // ###
    // Determine first/second poke from friendly/enemy
    poke_details_flexible *poke_first;  // Left floating for now, assigned immdiately after
    poke_details_flexible *poke_second;  // "" ""
    uint8_t poke_first_active_action;
    uint8_t poke_second_active_action;
    if(friendly_poke_party[*friendly_active_poke_index].SPD >= enemy_poke_party[*enemy_active_poke_index].SPD) {
        poke_first  = &(friendly_poke_party[*friendly_active_poke_index]);
        poke_second = &(enemy_poke_party[*enemy_active_poke_index]);
        poke_first_active_action  = *friendly_active_action;
        poke_second_active_action = *enemy_active_action;
    } else {
        poke_first  = &(enemy_poke_party[*enemy_active_poke_index]);
        poke_second = &(friendly_poke_party[*friendly_active_poke_index]);
        poke_first_active_action  = *enemy_active_action;
        poke_second_active_action = *friendly_active_action;
    }

    // Work through stages
    switch(*staging) {
        case 0:     // Select action broad
            calculate_combat_buttons_actionSelection(t1, staging);
            break;
        case 1:     // Select move action
            calculate_combat_buttons_moveSelection(friendly_poke_party, friendly_active_action, friendly_active_poke_index, t1, staging);
            break;
        case 2:     // Poke switch selection
            break;
        case 3:     // Poke bag selection
            break;

        // Fighting stages
        /*
        Starting from Nth stage - after prior button-based options

        . (0) Fight begin (blank)
        -- For first poke
        . (1) Move used
        . (2) Move effectiveness
        . (3) Is critical?
        . (4) Is miss?
        . (5) Status effect?
        -- Repeated for second poke (6-10)

        . (11) Poke fainted
        . (12) Experience gain
        . (13) Switch
        .   OR
        . (14) End combat

        . (15) Item used
        . (16) Is healed?
        . (17) Is ball thrown + shakes?
        -- Jump back to combat if needed after these have triggered

        * Note; Text for each of these stages in found in the display_combat.c; 
            only counter-based calculations / stage skipping+updating performed here
        */

        // Start
        case (4+0):
            break;

        // Poke first
        case (4+1):
            if(*poke_first_counter_type==0) {   // Should be found on first tick then ignored after that -> will also not be called on stages with no counter triggers so not an issue there
                *poke_first_counter_type = 1;   // HP
                // ###
                // ### SHOULD ONLY USE THIS IF 'action' IN RANGE [1,4] HENCE IS A MOVE NOT AN ITEM OR SMTHING ELSE
                // ###
                *is_critical = GET_MOVE_CRITICAL( &POKE_MOVE_CATALOGUE[ poke_first->moves[poke_first_active_action-1] ] );
                *is_miss = GET_MOVE_MISS( &POKE_MOVE_CATALOGUE[ poke_first->moves[poke_first_active_action-1] ] );
                *poke_first_counter = GET_POKE_DAMAGE(poke_first, poke_second, &POKE_MOVE_CATALOGUE[ poke_first->moves[poke_first_active_action-1] ], is_critical, is_miss);   // *NOTE; This DOES account for effectiveness, crits and misses despite the staging for the text for these occurring after
            }
            break;
        case (4+2):
            break;
        case (4+3):
            break;
        case (4+4):
            break;
        case (4+5):
            break;

        // Poke second
        case (4+6):
            if(*poke_second_counter_type==0) {
                *poke_second_counter_type = 1;    // HP
                // ###
                // ### SHOULD ONLY USE THIS IF 'action' IN RANGE [1,4] HENCE IS A MOVE NOT AN ITEM OR SMTHING ELSE
                // ###
                *is_critical = GET_MOVE_CRITICAL( &POKE_MOVE_CATALOGUE[ poke_second->moves[poke_second_active_action-1] ] );
                *is_miss = GET_MOVE_MISS( &POKE_MOVE_CATALOGUE[ poke_second->moves[poke_second_active_action-1] ] );
                *poke_second_counter = GET_POKE_DAMAGE(poke_second, poke_first, &POKE_MOVE_CATALOGUE[ poke_second->moves[poke_second_active_action-1] ], is_critical, is_miss);   // *NOTE; This DOES account for effectiveness, crits and misses despite the staging for the text for these occurring after
            }
            break;
        case (4+7):
            break;
        case (4+8):
            break;
        case (4+9):
            break;
        case (4+10):
            break;

        // Others
        case (4+11):
            if(*stage_timer >= 250) {/*JUMP TO COMBAT AGAIN IF NEEDED*/}
            break;
        case (4+12):
            if(*stage_timer >= 250) {/*JUMP TO COMBAT AGAIN IF NEEDED*/}
            break;
        case (4+13):
            if(*stage_timer >= 250) {/*JUMP TO COMBAT AGAIN IF NEEDED*/}
            break;
        case (4+14):
            if(*stage_timer >= 250) {/*JUMP TO COMBAT AGAIN IF NEEDED*/}
            break;

        // ...
        default:
            break;
    }

    // Apply counters
    if( (125 < *stage_timer) && (*stage_timer < 250) ) {   // Apply counters in the second half (text written in the first half)
        switch(*poke_first_counter_type) {
            // 0 => NO counter
            case 1:     // HP
                if( (*poke_first_counter > ((*poke_first_counter/(250-*stage_timer)) +1)) || (*stage_timer >= 249) ) {     // Option A: Slowly tick down counter such that it tends to 0 with timer
                    poke_first->HP = poke_first->HP -((*poke_first_counter/(250-*stage_timer)) +1); //### THIS NEEDS TO BE N SUCH THAT AFTER AT LEAST 125 TICKS IT WILL BE DONE ###
                    *poke_first_counter = *poke_first_counter -((*poke_first_counter/(250-*stage_timer)) +1);
                } else {        // Option B: Apply all the remaining counter to end it cleanly
                    poke_first->HP = poke_first->HP -*poke_first_counter;
                    *poke_first_counter = 0;
                }
                break;
            case 2:     // DEF
                //###
                //### COPY ABOVE INTO THESE SECTIONS ONCE ITS WORKING
                //###       could do this more cleanly with a pointer to the specific variable to be removed, then apply above in only one 
                //###       location, BUT should check this works first then do this method
                //###
                break;
            case 3:     // SP_DEF
                break;
            case 4:     // ATK
                break;
            case 5:     // SP_ATK
                break;
            case 6:     // SPD
                break;
            // ...
        }
        switch(*poke_second_counter_type) {
            // 0 => NO counter
            case 1:     // HP
                if( (*poke_second_counter > ((*poke_second_counter/(250-*stage_timer)) +1)) || (*stage_timer >= 249) ) {     // Option A: Slowly tick down counter such that it tends to 0 with timer
                    poke_second->HP = poke_second->HP -((*poke_second_counter/(250-*stage_timer)) +1); //### THIS NEEDS TO BE N SUCH THAT AFTER AT LEAST 125 TICKS IT WILL BE DONE ###
                    *poke_second_counter = *poke_second_counter -((*poke_second_counter/(250-*stage_timer)) +1);
                } else {        // Option B: Apply all the remaining counter to end it cleanly
                    poke_second->HP = poke_second->HP -*poke_second_counter;
                    *poke_second_counter = 0;
                }
                break;
            case 2:     // DEF
                //###
                //### COPY ABOVE INTO THESE SECTIONS ONCE ITS WORKING
                //###       could do this more cleanly with a pointer to the specific variable to be removed, then apply above in only one 
                //###       location, BUT should check this works first then do this method
                //###
                break;
            case 3:     // SP_DEF
                break;
            case 4:     // ATK
                break;
            case 5:     // SP_ATK
                break;
            case 6:     // SPD
                break;
            // ...
        }
    }

    // Update the stage counter IF NOT in any of the button menus (only automatically progress in automatic section)
    if(*staging >= 4) { if((*timer % *stage_speed)==0) {*stage_timer = *stage_timer +4;}; }  // Update stage timer slower than base timer
    if(*stage_timer >= 250) {
        *stage_timer = 0;  // Manually reset stage_timer (REQUIRED)
        *staging = *staging +1;

        *poke_first_counter = 0;        // Reset all counters after each stage
        *poke_first_counter_type = 0;   //
        *poke_second_counter = 0;       //
        *poke_second_counter_type = 0;  //
        // *Note; Staging jumps (e.g. from poke ball stage back to combat) occurs within the specific stage above; this is for typical stage progression
        
        // Reset combat staging once all resolved
        if(*staging >= 19) {
            *staging = 0;
        }
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
            *staging = 4;                                           // Move staging
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

uint8_t GET_POKE_DAMAGE(poke_details_flexible *p1, poke_details_flexible *p2, const poke_move *m, uint8_t *is_critical, uint8_t *is_miss) {
    /*
    . Returns the damage dealt by poke p1 to poke p2 using move m

    . Effectiveness
    . Critical
    . STAB
    . DEF/SP_DEF
    . Miss
    */
    // ###
    // ### CALCULATE THIS PROPERLY, THIS IS JUST A PLACEHOLDER FOR TESTING
    // ###
    return (m->ATK);
}
uint8_t GET_MOVE_CRITICAL(const poke_move *m) {
    /*
    . Returns whether the move m is critical or not (0 or 1) based on a single random roll

    rand() < 1638 => ~5% probability
    */
    if(rand() < m->critical_chance) {
        return 1;
    } else {
        return 0;
    }
}
uint8_t GET_MOVE_MISS(const poke_move *m) {
    /*
    . Returns whether the move m has missed or not (0 or 1) based on a single random roll
    */
    if(rand() < m->miss_chance) {
        return 1;
    } else {
        return 0;
    }
}
uint8_t GET_EFFECTIVENESS(uint8_t *types, const poke_move *m) {
    /*
    . Returns the effectiveness of a move m against a poke's types
    . Effectiveness value returned is a multiplier/2 for damage dealt e.g. SuperEff=>4/2=2 *multi, NotVeryEff=>1/2=0.5 *multi 

    . types = pointer to an array [type1, type2] only
    . m = poke move pointer

    . 0 = Not Effect
    . 1 = Not Very Effective
    . 2 = Normal Effectiveness
    . 4 = Super Effective
    */
    // ### NEEDS TO TAKE INTO ACCOUNT 2nd TYPE AS WELL --> SEE WHAT ACTUAL IMPLEMENTATION IS ###
    return 2;   //type_effectiveness_lookup[18*(uint16_t)(m->type) +types[0]];  <----- ### CURRENTLY NOT USING BOTH TYPES --> EASY FIX ###
}