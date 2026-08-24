#include "vectrex.h"
#include "vectrex/bios.h"
#include "cmoc.h"

#include "game_constants.h"
#include "core_structs.h"

#include "poke_fixed_catalogue.h"
#include "poke_move_catalogue.h"

#include "calculate_combat.h"

void calculate_combat_screen(uint8_t *screen_mode, poke_details_flexible *friendly_poke_party, poke_details_flexible *enemy_poke_party, uint8_t *staging, uint8_t *stage_timer, uint8_t *stage_speed, uint8_t *friendly_active_action, uint8_t *enemy_active_action, uint8_t *friendly_active_poke_index, uint8_t *enemy_active_poke_index, int8_t *poke_first_counter, int8_t *poke_second_counter, uint8_t *poke_first_counter_type, uint8_t *poke_second_counter_type, uint8_t *is_critical, uint8_t *is_miss, uint8_t *timer, uint8_t *t1) {
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

        . (11) Experience gain
        . (12) Poke fainted
        . (13) End combat

        . (14) Switch <-- Jumped to when needed
        ### MAYBE SPLIT INTO 2 -> 1 FOR MANUAL SWITCH OTHER FOR FORCED ###


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
            if(poke_first->HP > 0) {
                if(*poke_second_counter_type==0) {   // Should be found on first tick then ignored after that -> will also not be called on stages with no counter triggers so not an issue there
                    *poke_second_counter_type = 1;   // HP
                    // ###
                    // ### SHOULD ONLY USE THIS IF 'action' IN RANGE [1,4] HENCE IS A MOVE NOT AN ITEM OR SMTHING ELSE
                    // ###
                    *is_critical = GET_MOVE_CRITICAL( &POKE_MOVE_CATALOGUE[ poke_first->moves[poke_first_active_action-1] ] );
                    *is_miss = GET_MOVE_MISS( &POKE_MOVE_CATALOGUE[ poke_first->moves[poke_first_active_action-1] ] );
                    *poke_second_counter = GET_POKE_DAMAGE(poke_first, poke_second, &POKE_MOVE_CATALOGUE[ poke_first->moves[poke_first_active_action-1] ], is_critical, is_miss);   // *NOTE; This DOES account for effectiveness, crits and misses despite the staging for the text for these occurring after
                    if(*poke_second_counter > poke_second->HP) { *poke_second_counter = poke_second->HP; }
                }
            } else { *stage_timer = 250; }
            break;
        case (4+2):
            if(poke_first->HP > 0) {
                if(GET_EFFECTIVENESS(poke_second->fixed_details->types, &POKE_MOVE_CATALOGUE[ poke_first->moves[poke_first_active_action-1] ]) == 2) { *stage_timer = 250; }   // If normal effectiveness, skip the dialoague
            } else { *stage_timer = 250; }
            break;
        case (4+3):
            if(poke_first->HP > 0) {    
                if(*is_critical == 0) { *stage_timer = 250; }   // If non-critical, skip dialogue
            } else { *stage_timer = 250; }
            break;
        case (4+4):
            if(poke_first->HP > 0) {
                if(*is_miss == 0) { *stage_timer = 250; }   // If NOT missing, don't show dialogue for missing
            } else { *stage_timer = 250; }
            break;
        case (4+5):
            if(poke_first->HP > 0) {    
                if(1==1) { *stage_timer = 250; }   // ### SKIP ALWAYS FOR NOW --> Should check if a status triggers in future ###
            } else { *stage_timer = 250; }
            break;

        // Poke second
        case (4+6):
            if(poke_second->HP > 0) {
                if(*poke_first_counter_type==0) {
                    *poke_first_counter_type = 1;    // HP
                    // ###
                    // ### SHOULD ONLY USE THIS IF 'action' IN RANGE [1,4] HENCE IS A MOVE NOT AN ITEM OR SMTHING ELSE
                    // ###
                    *is_critical = GET_MOVE_CRITICAL( &POKE_MOVE_CATALOGUE[ poke_second->moves[poke_second_active_action-1] ] );
                    *is_miss = GET_MOVE_MISS( &POKE_MOVE_CATALOGUE[ poke_second->moves[poke_second_active_action-1] ] );
                    *poke_first_counter = GET_POKE_DAMAGE(poke_second, poke_first, &POKE_MOVE_CATALOGUE[ poke_second->moves[poke_second_active_action-1] ], is_critical, is_miss);   // *NOTE; This DOES account for effectiveness, crits and misses despite the staging for the text for these occurring after
                    if(*poke_first_counter > poke_first->HP) { *poke_first_counter = poke_first->HP; }
                }
            } else { *stage_timer = 250; }
            break;
        case (4+7):
            if(poke_second->HP > 0) {
                if(GET_EFFECTIVENESS(poke_first->fixed_details->types, &POKE_MOVE_CATALOGUE[ poke_second->moves[poke_second_active_action-1] ]) == 2) { *stage_timer = 250; }   // If normal effectiveness, skip the dialoague
            } else { *stage_timer = 250; }
            break;
        case (4+8):
            if(poke_second->HP > 0) {
                if(*is_critical == 0) { *stage_timer = 250; }   // If non-critical, skip dialogue
            } else { *stage_timer = 250; }
            break;
        case (4+9):
            if(poke_second->HP > 0) {
                if(*is_miss == 0) { *stage_timer = 250; }   // If NOT missing, don't show dialogue for missing
            } else { *stage_timer = 250; }
            break;
        case (4+10):
            if(poke_second->HP > 0) {
                if(1==1) { *stage_timer = 250; }   // ### SKIP ALWAYS FOR NOW --> Should check if a status triggers in future ###
            } else { *stage_timer = 250; }
            break;

        // Others
        case (15):    // EXPERIENCE
            // Experience to second
            if(poke_first->HP <= 0) {
                if(*poke_second_counter_type==0) {
                    *poke_second_counter_type = 7;    // EXP
                    *poke_second_counter = GET_EXPERIENCE(poke_first);
                }
            }

            // Experience to first
            if(poke_second->HP <= 0) {
                if(*poke_first_counter_type==0) {
                    *poke_first_counter_type = 7;    // EXP
                    *poke_first_counter = GET_EXPERIENCE(poke_second);
                    if(*poke_first_counter > poke_first->EXP_MAX-poke_first->EXP) { *poke_first_counter = poke_first->HP; }
                }
            }

            if( (poke_first->HP > 0) && (poke_second->HP > 0) ) { *stage_timer=250; }   // If both pokes are healthy, no need to show exp gain
            // if(*stage_timer >= 250) {/*JUMP TO COMBAT AGAIN IF NEEDED*/}
            break;

        case (16):    // FAINT -> Pick new pokes
            if(*stage_timer >= 250) {   // One time trigger at frame 250

                if(friendly_poke_party[*friendly_active_poke_index].HP <= 0) {  // If friendly poke fainted, GO TO poke switch screen to choose a new poke
                    if(sum_fainted_poke(friendly_poke_party) < 6) {      // Only do this IF another poke is viable to be chosen (if NONE viable, then will end combat in a couple stages anyway so leaving the fainted poke out in the open is fine)
                        *staging = 18;  // SWITCH
                    }
                }
                
                if(friendly_poke_party[*friendly_active_poke_index].HP <= 0) {  // If enemy poke fainted, randomly choose another to come out
                    if(sum_fainted_poke(friendly_poke_party) < 6) {      // Only do this IF another poke is viable to be chosen (if NONE viable, then will end combat in a couple stages anyway so leaving the fainted poke out in the open is fine)
                        for(uint8_t i=0; i<POKE_PARTY_LENGTH; i++) {    // For now, simply pick the earliest poke in the list that is not fainted to switch to
                            if(enemy_poke_party[i].HP > 0) {
                                *enemy_active_poke_index = i;
                                break;
                            }
                        }
                    }
                }
                // *Note; Animation for switching poke can from frame X-250, where X is in (125, 250) --> This switch can then happen just before or right at the end of this animation
                
            }
            if( (poke_first->HP > 0) && (poke_second->HP > 0) ) { *stage_timer=250; }   // If both pokes are healthy, no need to show faint dialogue
            break;
        case (17):    // END COMBAT
            if(*stage_timer >= 200) {   // Stage_timer >=250 hence still triggers parameter reset still

                if(sum_fainted_poke(friendly_poke_party) >= POKE_PARTY_LENGTH) {    // If all friendly pokes have fainted, then you are forced to leave combat
                    // ###
                    // ### TAKE YOU TO A SCREEN SAYING YOU WHITED OUT FIRST
                    // ###
                    *screen_mode = 0;   // Switch to roaming screen
                }
                if(sum_fainted_poke(enemy_poke_party) >= POKE_PARTY_LENGTH) {    // If all enemy pokes have fainted, then you are forced to leave combat
                    *screen_mode = 0;   // Switch to roaming screen
                }

                // *staging=30;    // To go to end of staging to trigger next round of combat

            }
            if(*stage_timer >= 220) {
                *staging = 30;          // Force reset to next round of combat
                *stage_timer = 250;     //  + reset parameters properly
            }

            break;
        case (18):    // SWITCH
            if(*stage_timer >= 250) {/*JUMP TO COMBAT AGAIN IF NEEDED*/}
            // ######
            // ### NEED TO TEST THIS WITH BUG-FIX VISUALISER
            // ######
            calculate_combat_buttons_pokeSwitch(friendly_poke_party, friendly_active_poke_index, staging, t1);
            break;
        case (19):    // POKE BAG
            if(*stage_timer >= 250) {/*JUMP TO COMBAT AGAIN IF NEEDED*/}
            calculate_combat_buttons_pokeBag();
            break;

        // ...
        default:
            *staging = *staging +1; // If unsure, just progress staging to try reset combat
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
            case 7:     // EXP
                if(poke_first->EXP >= poke_first->EXP_MAX) {    // When you reach the LVL UP threshold, reset EXP, increased EXP_MAX and increased LVL
                    poke_first->EXP = 0;
                    poke_first->EXP_MAX = (uint8_t)((5*(uint16_t)(poke_first->EXP_MAX))/4);  //EXP_MAX*(5/4) => 25% increased each time
                    poke_first->level = poke_first->level +1;
                }
                if(*poke_first_counter > 0) {   // If EXP to give, give it 1 at a time and reduce
                    // ###
                    // ### FOR LARGE EXP VALUES, WILL NEED TO GIVE N AT A TIME
                    // ###      THIS IS FINE FOR TESTING PURPOSES NOW, BUT FIX LIKE WITH HP SOON
                    // ###
                    poke_first->EXP = poke_first->EXP +1;
                    *poke_first_counter = *poke_first_counter -1;
                }
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
            case 7:     // EXP
                break;
            // ...
        }
    }

    // Update the stage counter IF NOT in any of the button menus (only automatically progress in automatic section)
    // ALSO don't progress time if on SWITCH or BAG stages (18 and ...)
    if( (*staging >= 4) && (*staging != 18) ) { if((*timer % *stage_speed)==0) {*stage_timer = *stage_timer +4;}; }  // Update stage timer slower than base timer
    if(*stage_timer >= 250) {
        *stage_timer = 0;  // Manually reset stage_timer (REQUIRED)
        *staging = *staging +1;

        *poke_first_counter = 0;        // Reset all counters after each stage
        *poke_first_counter_type = 0;   //
        *poke_second_counter = 0;       //
        *poke_second_counter_type = 0;  //
        // *Note; Staging jumps (e.g. from poke ball stage back to combat) occurs within the specific stage above; this is for typical stage progression
        
        // Reset combat staging once all resolved
        if(*staging >= 30) {
            *staging = 0;
        }
    }

    // ### BUG-FIXING ###
    // uint8_t dead_count_e = sum_fainted_poke(enemy_poke_party);
    // reset_beam();
    // set_scale(128);
    // set_text_size(-5, 40);
    // char value_buffer[4];
    // // STAGING
    // value_buffer[0] = '0';
    // value_buffer[1] = '0'+(dead_count_e /10);
    // value_buffer[2] = '0'+(dead_count_e %10);
    // value_buffer[3] = '\0';
    // print_str_c(-50, -75, value_buffer);
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
        if( (0 <= *friendly_active_poke_index) && (*friendly_active_poke_index < POKE_PARTY_LENGTH) ) {     // If is a valid poke
            if(POKE_MOVE_CATALOGUE[ friendly_poke_party[*friendly_active_poke_index].moves[*friendly_hovered_move] ].type != 0) {    // If is a non-null move
                *friendly_active_action = *friendly_hovered_move +1;    // Select move
                *friendly_hovered_move = 0;                             // Reset the temporary hover variable
                *staging = 4;                                           // Move staging
            }
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
void calculate_combat_buttons_pokeSwitch(poke_details_flexible *friendly_poke_party, uint8_t *friendly_active_poke_index, uint8_t *staging, uint8_t *hovered_poke_index) {
    /*
    . Buttons to control which poke to switch to
    . Occurs for a manual switch and for a forced switch (when your friendly poke faints)
    . NO option to BACK OUT when in this menu -> simplifies it working for both forced and manual switching
    */
    uint8_t buttons = read_buttons();
    if(buttons & JOY1_BTN1_MASK) {              // Cycle options
        *hovered_poke_index = (*hovered_poke_index +1)%(POKE_PARTY_LENGTH);

        // ###
        // ### NOTE THIS IS TERRIBLE FOR PERFORMANCE
        // ###      BUT IS GOOD FOR BUG-FIXING AND NOT A BIG ISSUE FOR 1 TIME USES
        // ###
        reset_beam();
        print_str_c(0, 0, (char*)"PRESSED 1");
        set_scale(128);
    }

    if(buttons & JOY1_BTN2_MASK) {              // Select poke to switch to
        if( (0 <= *hovered_poke_index) && (*hovered_poke_index < POKE_PARTY_LENGTH) ) {     // If within bounds
            if(friendly_poke_party[*hovered_poke_index].HP > 0) {                           // If alive
                *friendly_active_poke_index = *hovered_poke_index;    // Select poke
                *hovered_poke_index = 0;                             // Reset the temporary hover variable
                *staging = 17;                                       // Move staging
            }
        }

        reset_beam();
        print_str_c(0, 0, (char*)"PRESSED 2");
        set_scale(128);
    }

    // if(buttons & JOY1_BTN3_MASK) {              // Back out of options
    //     //pass

    //     reset_beam();
    //     print_str_c(0, 0, (char*)"PRESSED 3");
    //     set_scale(128);
    // }
}
void calculate_combat_buttons_pokeBag() {
    /*
    . Buttons to control which item to choose in the player's bag
    . *Note; This is purely for using an item mid combat, not while roaming, etc
    */
    uint8_t buttons = read_buttons();
    if(buttons & JOY1_BTN1_MASK) {              // Cycle options
        // pass

        // ###
        // ### NOTE THIS IS TERRIBLE FOR PERFORMANCE
        // ###      BUT IS GOOD FOR BUG-FIXING AND NOT A BIG ISSUE FOR 1 TIME USES
        // ###
        reset_beam();
        print_str_c(0, 0, (char*)"PRESSED 1");
        set_scale(128);
    }

    if(buttons & JOY1_BTN2_MASK) {              // Select option
        // pass

        reset_beam();
        print_str_c(0, 0, (char*)"PRESSED 2");
        set_scale(128);
    }

    if(buttons & JOY1_BTN3_MASK) {              // Back out of options
        //pass

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
            2,
            15,

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
            0,
            15,

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

uint8_t sum_fainted_poke(poke_details_flexible *poke_party) {
    /*
    . Specify a pointer to a list of pokes of length POKE_PARTY_LENGTH
    . Returns the number of pokes in the party specified which have fainted (HP<=0) <-- =0 in reality since stored as uint8_t hence cannot be <0
    */
    uint8_t fainted_number = 0;
    for(uint8_t i=0; i<POKE_PARTY_LENGTH; i++) { 
        if(poke_party[i].HP <= 0) {fainted_number+=1;}
    }
    return fainted_number;
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
uint8_t GET_EFFECTIVENESS(const uint8_t *types, const poke_move *m) {
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
uint8_t GET_EXPERIENCE(poke_details_flexible *fainted_poke) {
    /*
    . Returns the experience given by the fainted_poke provided
    */
    // ###
    // ### PLACEHOLDER FOR ACTUAL/NEAR ENOUGH EQUATION
    // ###
    return fainted_poke->level;
}