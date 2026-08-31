#include "vectrex.h"
#include "vectrex/bios.h"
#include "cmoc.h"

#include "game_constants.h"
#include "core_structs.h"

#include "poke_move_catalogue.h"
#include "core_poke_functions.h"

#include "display_combat.h"

/*
------------
POKE SPRITES
------------
*/


/*
-------------
BATTLE_ASSETS
-------------
. Holds all functions related the vector display within the battle mode

Optimisations;
    Brightness slows program too
    Can manually lower refresh rate too
    Smarter asset linking
    CAN draw every other frame for low detail aspects, etc
*/

void display_combat_screen(poke_details_flexible *friendly_poke_party, poke_details_flexible *enemy_poke_party, uint8_t *staging, uint8_t *stage_timer, uint8_t *friendly_active_action, uint8_t *enemy_active_action, uint8_t *friendly_active_poke_index, uint8_t *enemy_active_poke_index, uint8_t *timer, uint8_t *t1) {
    /*
    ---- Display Order:
    -- SWITCH:
    . (1) Poke elements
    . (2) Hovered icon
    -- BAG:
    . (1) Item elements
    . (2) Hovered icon
    -- MAIN COMBAT:
    . (1) Friendly poke + bars
    . (2) Enemy poke + bars
    . (3) Battle option box
    . (4) Battle option text
    . (5) Encounter text
    */
    char encounter_text[ENCOUNTER_TEXT_LENGTH] = {'','','','','','','','','','','','','','','','','','','','','','','','','\0'};  // Populated with encounter text in each stage

    const uint8_t poke_width = 80;  //40
    const uint8_t poke_vertical_offset = 40;    // Centre to first poke, AND spacing between friendly and enemy poke
    int8_t text_box_height = 60;    // Height of the text box at the bottom of the screen / action box

    reset_beam();

    switch(*staging) {
        case 18:    // SWITCH
            //pass
            break;
        case 19:    // BAG
            //pass
            break;
        default:    // MAIN COMBAT

            reset_beam();
            set_scale(128); // 128 used a standard scale here <-- Lower may improve performance?
            intensity(0x4f);
            if(*timer%2==0) {    // Draw pokes on evens
                move(poke_vertical_offset, 0);        // Position the info slightly higher up the screen to give room for text below
                move(0, -120);
                // move(linear_to_oscil(*timer)>>4, 0);
                display_poke_bars(0,0, &(friendly_poke_party[*friendly_active_poke_index]), poke_width );
                move(linear_to_oscil(*timer)>>4, 0);
                // display_poke_portrait(poke_width, 1);
                display_flat_vectors(friendly_poke_party[*friendly_active_poke_index].fixed_details->flat_sprite_vectors, friendly_poke_party[*friendly_active_poke_index].fixed_details->flat_sprite_vectors_length);

                // move(-linear_to_oscil(*timer)>>4, 0);
                // move(poke_vertical_offset, 120);        // Back to origin + vertical offset
                // move(0, 120-poke_width);    // Enough ahead so edge of poke_width is at far right (120~128)
                reset_beam();   // Need to go back to origin to re-align anyway, so might as well reset_beam()
                move(2*poke_vertical_offset, 120-poke_width);
                display_poke_bars(0,0, &(enemy_poke_party[*enemy_active_poke_index]), poke_width );
                move(linear_to_oscil(64+*timer)>>4, 0);
                // display_poke_portrait(poke_width, 0);
                display_flat_vectors(enemy_poke_party[*enemy_active_poke_index].fixed_details->flat_sprite_vectors, enemy_poke_party[*enemy_active_poke_index].fixed_details->flat_sprite_vectors_length);
            }
            if(*timer%2==1) {    // Draw text + BB on odds
                // reset_beam();
                display_battle_box(&text_box_height, staging, timer, t1);

                reset_beam();
                display_battle_text(friendly_poke_party, enemy_poke_party, friendly_active_poke_index, enemy_active_poke_index, friendly_active_action, enemy_active_action, &poke_vertical_offset, &text_box_height, &poke_width, staging, stage_timer, encounter_text);
            }
            reset_beam();


            // BUG-FIXING
            // char value_buffer[4];
            // value_buffer[0] = '0';
            // value_buffer[1] = '0'+((*enemy_active_action) /10);
            // value_buffer[2] = '0'+((*enemy_active_action) %10);
            // value_buffer[3] = '\0';
            // print_str_c(0, 0, value_buffer);
    }

    // ###
    // ### IF STAGING IS FOR SWITCH, SHOW ENTIRELY DIFFERENT SCREEN
    // ###      --> Show smaller poke sprites
    // ###
    // ### SAME FOR BAG
    // ###

    // ### BUG-FIXING ###
    // bugfix_poke_details_flexible(&(friendly_poke_party[0]), 0, -45);
    // bugfix_poke_details_flexible(&(enemy_poke_party[0]), 0, 45);
    // bugfix_combat_info(staging, stage_timer, friendly_active_action, enemy_active_action, friendly_active_poke_index, enemy_active_poke_index, timer, t1, 60, 0);
}

void display_poke_bars(uint8_t *yOffset, uint8_t *xOffset, poke_details_flexible *poke, uint8_t width) {
    /*
    . Starts far-LHS, should end on far-LHS (Y movement can be accounted for separately)
    */
    const uint8_t lean = (width>>3);
    const uint8_t height = (width>>2);
    
    int8_t vector_lines[8] = {  // Start on left-middle (experience bar left edge), end on top of right-middle
        height, lean,
        0, width, 
        -height, -lean,
        0, -width
    };

    const uint8_t health_ratio = (uint8_t)( ((uint16_t)(poke->HP) << 8) /(poke->HP_MAX) );  // As a ratio of 256, truncated in division if fractional part remaining
    const uint8_t experience_ratio = (uint8_t)( ((uint16_t)(poke->EXP) << 8) /(poke->EXP_MAX) );

    // Draw cage + experience bar, beam ends at bottom-right (above experience)
    move(0,0);
    lines(4, vector_lines);

    const uint8_t health_vector_distance = (uint8_t)( ((uint16_t)health_ratio*7*(width>>3))>>8 );
    const uint8_t experience_vector_distance = (uint8_t)( ((uint16_t)experience_ratio*7*(width>>3))>>8 );

    // EXP line
    move(height>>2, lean>>1);
    line(0, experience_vector_distance);
    // HP line
    move(height>>2, -experience_vector_distance +(lean>>1));
    line(0, health_vector_distance);
    // Move back to bottom left corner
    move(-height>>1, -health_vector_distance);



    // Name + level
    // move(height>>2, -health_vector_distance);

    // char poke_level[7] = {'L', 'V', 'L', ':', '0'+(poke->level /10), '0'+(poke->level %10), '\0'};
    // set_text_size(-(width>>4), width>>1);
    // reset_beam();
    // print_str_c(0,0, poke_level);
    // print_str_c(30,0, (char*)(poke->fixed_details->name));
    // print_str_c(0,0, (char*)("TEST"));
}

void display_poke_portrait(uint8_t width, uint8_t is_charmander_temp) {
    /*
    . Displays the poke starting from the top-left corner
    */
    int8_t vector_lines[8] = {    // ** Note; Non-const as lines requires just char[] argument
        0, width,    // Top-left, travelling CW
        -width, 0,
        0, -width,
        width, 0,
    };
    intensity(0x4f);
    lines(4, vector_lines);
}

void display_flat_vectors(const int8_t *flat_vectors, uint16_t flat_vector_length) {
    /*
    ###
    ### SHOULD ADD A SCALE ARGUEMENT /256 HERE TO CHANGE VECTOR SIZES MORE EASILY
    ###
    . Displays a given 'flat-vector' list, which is a list as follows;
        {spacing, originY, originX          <-- Only first 3 terms, not repeating
        x1, i1, x2, i2, ..., xn, in, snake_offset, 0, 
        x1, i1, x2, i2, ..., xm, im, snake_offset, 0, 
        ...}
        . spacing = Distance betwen adjacent lines/planes
        . originY/X = Distance from top left corner to first vector start point
        . 0 marks a new line is about to be started
        . snake_offset = Offset in X direction (y offset always is spacing)
        . x...= X offset for this vector (from current beam position, move by xn)
        . i...= Intensity of the line to be drawn
    . flat_vector_length = Number of terms in this flat vector full-list
    . Starts from top left corner
    . This method of storage reduces space taken up by this shaded image
    . Vectors stored so the beam 'snakes' around for speed improvement e.g.
            --->--\/
            \/-<----
            --->----
            ...
    . Intensity on scale 1-8 (NOT 0-7 to avoid 0 being used for situations other than line end) => 1==NO Beam Intensity, 8==MAX Beam Intensity
    */
    const uint8_t spacing = flat_vectors[0];
    move(flat_vectors[1], flat_vectors[2]);
    for(uint16_t i=4; i<flat_vector_length; i+=2) {
        if(flat_vectors[i]==0) {    // Marks the end of a line => Move to new position
            move(spacing, flat_vectors[i-1]);   // <-- Move down + snake offset
        } else {        // For standard operation...
            switch(flat_vectors[i]) {
                case 2:
                    intensity(0x2f);
                    break;
                case 3:
                    intensity(0x3f);
                    break;
                case 4:
                    intensity(0x4f);
                    break;
                case 5:
                    intensity(0x5f);
                    break;
                case 6:
                    intensity(0x6f);
                    break;
                case 7:
                    intensity(0x7f);
                    break;
                default:
                    intensity(0x0f);
                    break;
            }
            line(0, flat_vectors[i-1]);
        }
    }
}

void display_battle_box(int8_t *box_height, uint8_t *staging, uint8_t *timer, uint8_t *hovered_index) {
    /*
    . Displays the boxes in the bottom-right corner which are shown when selecting and action/move

    . box_height = the height of the text box/action box set (2 rows within the single height)
    */
    if( (*staging==0) || (*staging==1) ) {
        // Draw rough border
        int8_t vector_lines[4] = {
            *box_height, 0,
            0, *box_height,
        };
        move(-120, (int8_t)(120- ( (*box_height) + (*box_height>>2) ))); // Move to centre-bottom of screen (assumes a beam_reset() before this)
        intensity(0x2f);
        lines(2, vector_lines);

        // Hover icon
        // ###
        // ### NOTE; Currently has a reset_beam(), which could be removed for better performance (would need to adjust origin pos after doing this)
        // ###
        int8_t yFactor = (*hovered_index<=1) ? 1 : -1;
        display_hovered_star(
            -120 +(*box_height>>1) +(*box_height>>2)*(yFactor), 
            120 -(*box_height>>1) +((*box_height>>1) + (*box_height>>3))*((*hovered_index%2)*2 -1),  //-1, 1, -1, 1
            // 120 -(*box_height>>1) +(*box_height>>1)*(*hovered_index%2),  //-1, 1, -1, 1
            3, 
            *timer
        );
    }
}
void display_battle_text(poke_details_flexible *friendly_poke_party, poke_details_flexible *enemy_poke_party, uint8_t *friendly_active_poke_index, uint8_t *enemy_active_poke_index, uint8_t *friendly_active_action, uint8_t *enemy_active_action, const uint8_t *poke_vertical_offset, int8_t *box_height, const uint8_t *width, uint8_t *staging, uint8_t *stage_timer, char *encounter_text) {
    /*
    . Displays the text for the battle boxes, showing action/move names
    . Handled separately since each requires the text to move to the origin each time so time will be wasted no matter what
    */
    display_poke_stats_text(friendly_poke_party, enemy_poke_party, friendly_active_poke_index, enemy_active_poke_index, poke_vertical_offset, width, staging);
    display_encounter_text(friendly_poke_party, enemy_poke_party, friendly_active_poke_index, enemy_active_poke_index, friendly_active_action, enemy_active_action, box_height, staging, stage_timer, encounter_text);
    display_action_text(friendly_poke_party, friendly_active_poke_index, box_height, staging);
}

void display_poke_stats_text(poke_details_flexible *friendly_poke_party, poke_details_flexible *enemy_poke_party, uint8_t *friendly_active_poke_index, uint8_t *enemy_active_poke_index, const uint8_t *poke_vertical_offset, const uint8_t *width, uint8_t *staging) {
    /*
    . Text for the poke stats of both friendly and enemy poke
    */

    char friendly_poke_level[7] = {'L', 'V', 'L', ':', '0'+(friendly_poke_party[*friendly_active_poke_index].level /10), '0'+(friendly_poke_party[*friendly_active_poke_index].level %10), '\0'};
    char enemy_poke_level[7] = {'L', 'V', 'L', ':', '0'+(enemy_poke_party[*enemy_active_poke_index].level /10), '0'+(enemy_poke_party[*enemy_active_poke_index].level %10), '\0'};
    reset_beam();
    set_text_size(-( (*width>>4) -(*width>>5) ), (*width>>1) -(*width>>5));
    intensity(0x5f);

    print_str_c((*poke_vertical_offset) +(*width>>3) +(*width>>4), -120 +(*width>>4)             , (char*)(friendly_poke_party[*friendly_active_poke_index].fixed_details->name) );
    print_str_c((*poke_vertical_offset) +(*width>>3) +(*width>>4), -120 +(*width>>4) +(*width>>1), friendly_poke_level );

    print_str_c( 2*(*poke_vertical_offset) +(*width>>3) +(*width>>4), 120 -*width +(*width>>4), (char*)(enemy_poke_party[*enemy_active_poke_index].fixed_details->name) );
    print_str_c( 2*(*poke_vertical_offset) +(*width>>3) +(*width>>4), 120 -*width +(*width>>1) +(*width>>4), enemy_poke_level );
}
void display_encounter_text(poke_details_flexible *friendly_poke_party, poke_details_flexible *enemy_poke_party, uint8_t *friendly_active_poke_index, uint8_t *enemy_active_poke_index, uint8_t *friendly_active_action, uint8_t *enemy_active_action, int8_t *box_height, uint8_t *staging, uint8_t *stage_timer, char *encounter_text) {
    /*
    . Displays the text for the combat description, E.g; XXX WAS SUPER EFFECTIVE!
    . Shown in the bottom left corner
    */
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

    if(*stage_timer > 5) {
        switch(*staging) {
            case 4+0:
                set_text(
                    encounter_text,
                    14,
                    (char*)"COMBAT BEGINS!"
                );
                break;

            // ###
            // ### THIS IS RECALCED EACH TURN, COULD MINIMISE THIS AND JUST DO ON FIRST FRAME THEN LEAVE ENCOUTNER_TEXT UNCHANGED
            // ###      ALTHOUGH THIS NEEDS TO BE RE-FOUND SINCE ENCOUNTER TEXT DEFINED INSIDE THIS FUNCTION
            // ###
            // First poke
            case 4+1:
                // ###
                // ### NEEDS TO BE FIRST, NOT FRIENDLY 
                // ###
                set_text(
                    encounter_text,
                    POKE_NAME_LENGTH+6+POKE_MOVE_NAME_LENGTH +1,
                    (char*)"XXXXXXX USED XXXXXXXY"
                );
                set_text_subset(
                    encounter_text,
                    0,
                    POKE_NAME_LENGTH-1,     // *NOTE; -1 ro remove the \0 on the last character
                    (char*)(poke_first->fixed_details->name)
                );
                set_text_subset(
                    encounter_text,
                    POKE_NAME_LENGTH+6,
                    POKE_NAME_LENGTH+6+POKE_MOVE_NAME_LENGTH +1,
                    (char*)(POKE_MOVE_CATALOGUE[poke_first->moves[poke_first_active_action-1]].name)
                );
                break;
            case 4+2:
                switch(GET_EFFECTIVENESS(poke_second->fixed_details->types, &POKE_MOVE_CATALOGUE[poke_first->moves[poke_first_active_action-1]])) {
                    case 0:     // No effect
                        set_text(
                            encounter_text,
                            20,
                            (char*)"IT HAD NO EFFECT..."
                        );
                        break;
                    case 1:     // Not very effective
                        set_text(
                            encounter_text,
                            24,
                            (char*)"IT WAS NOT EFFECTIVE..."
                        );
                        break;
                    case 2:     // Normal effectiveness
                        set_text(
                            encounter_text,
                            5,
                            (char*)"NULL"       //*NOTE; Null text since this should NOT be shown in normal effective case
                        );
                        break;
                    case 4:     // Super effective
                        set_text(
                            encounter_text,
                            24,
                            (char*)"IT WAS SUPER EFFECTIVE!"
                        );
                        break;
                    default:     // Fall back text
                        set_text(
                            encounter_text,
                            5,
                            (char*)"NULL"
                        );
                        break;
                }
                break;
            case 4+3:
                // *NOTE; Will be skipped if non-critical
                set_text(
                    encounter_text,
                    23,
                    (char*)"IT WAS A CRITICAL HIT!"
                );
                break;
            case 4+4:
                // *NOTE; Will be skipped if non-miss
                set_text(
                    encounter_text,
                    11,
                    (char*)"IT MISSED!"
                );
                break;
            case 4+5:
                // ######
                // ### STATUS EFFECT MSG -> Add later on
                // ######
                break;
            
            // Second poke
            case 4+6:
                // ###
                // ### NEEDS TO BE SECOND, NOT ENEMY 
                // ###
                set_text(
                    encounter_text,
                    POKE_NAME_LENGTH+6+POKE_MOVE_NAME_LENGTH +1,
                    (char*)"XXXXXXX USED XXXXXXXY"
                );
                set_text_subset(
                    encounter_text,
                    0,
                    POKE_NAME_LENGTH-1,     // *NOTE; -1 to remove the \0 on the last character of name
                    (char*)(poke_second->fixed_details->name)
                );
                set_text_subset(
                    encounter_text,
                    POKE_NAME_LENGTH+6,
                    POKE_NAME_LENGTH+6+POKE_MOVE_NAME_LENGTH +1,
                    (char*)(POKE_MOVE_CATALOGUE[poke_second->moves[poke_second_active_action-1]].name)
                    // (char*)(POKE_MOVE_CATALOGUE[poke_second->moves[0]].name)    // ### TEMPORARY -> JUST WHILE MOVES ARE NOT BEING AUTO SELECTED ###
                );
                break;
            case 4+7:
                switch(GET_EFFECTIVENESS(poke_first->fixed_details->types, &POKE_MOVE_CATALOGUE[poke_second->moves[poke_second_active_action-1]])) {
                    case 0:     // No effect
                        set_text(
                            encounter_text,
                            20,
                            (char*)"IT HAD NO EFFECT..."
                        );
                        break;
                    case 1:     // Not very effective
                        set_text(
                            encounter_text,
                            24,
                            (char*)"IT WAS NOT EFFECTIVE..."
                        );
                        break;
                    case 2:     // Normal effectiveness
                        set_text(
                            encounter_text,
                            5,
                            (char*)"NULL"       //*NOTE; Null text since this should NOT be shown in normal effective case
                        );
                        break;
                    case 4:     // Super effective
                        set_text(
                            encounter_text,
                            24,
                            (char*)"IT WAS SUPER EFFECTIVE!"
                        );
                        break;
                    default:     // Fall back text
                        set_text(
                            encounter_text,
                            5,
                            (char*)"NULL"
                        );
                        break;
                }
                break;
            case 4+8:
                set_text(
                    encounter_text,
                    23,
                    (char*)"IT WAS A CRITICAL HIT!"
                );
                break;
            case 4+9:
                set_text(
                    encounter_text,
                    11,
                    (char*)"IT MISSED!"
                );
                break;
            case 4+10:
                // ######
                // ### STATUS EFFECT MSG -> Add later on
                // ######
                break;

            case 15:   // EXPERIENCE

                if(friendly_poke_party[*friendly_active_poke_index].HP <= 0) {
                    set_text(
                        encounter_text,
                        16,
                        (char*)"XXXXXXX FAINTED"
                    );
                    set_text_subset(
                        encounter_text,
                        0,
                        POKE_NAME_LENGTH-1,     // *NOTE; -1 to remove the \0 on the last character
                        (char*)(friendly_poke_party[*friendly_active_poke_index].fixed_details->name)
                    );
                }
                if(enemy_poke_party[*enemy_active_poke_index].HP <= 0) {
                    set_text(
                        encounter_text,
                        16,
                        (char*)"XXXXXXX FAINTED"
                    );
                    set_text_subset(
                        encounter_text,
                        0,
                        POKE_NAME_LENGTH-1,     // *NOTE; -1 to remove the \0 on the last character
                        (char*)(enemy_poke_party[*enemy_active_poke_index].fixed_details->name)
                    );
                }

                break;
            case 16:   // FAINT
                //pass
                break;
            case 17:   // END
                //pass
                break;
            case 18:   // SWITCH
                //pass
                break;
            case 19:   // BAG
                //pass
                break;
            
            default:
                set_text(
                    encounter_text, 
                    ENCOUNTER_TEXT_LENGTH, 
                    (char*)"WHAT WILL YOU DECIDE?"
                );
                break;
        }
    } else {
        // For first few frames, set text to be blank --> This prevents skipped stages flashing their text
        set_text(
            encounter_text, 
            1, 
            (char*)""
        );
    }
    reset_beam();
    set_text_size(-((uint8_t)(*box_height)>>4), (uint8_t)(*box_height)>>1);
    intensity(0x5f);
    print_str_c(-120+((*box_height)>>1),-120,encounter_text);
}

void display_action_text(poke_details_flexible *friendly_poke_party, uint8_t *friendly_active_poke_index, int8_t *box_height, uint8_t *staging) {
    /*
    . Text for the action box, e.g. in the bottom-right corner

    A | B
    C | D
    */
    reset_beam();
    set_text_size(-((uint8_t)(*box_height)>>4), (uint8_t)(*box_height)>>1);
    intensity(0x5f);
    switch(*staging) {
        case 0:
            print_str_c(-120+(3*(*box_height)>>2), 120 -*box_height     , (char*)"FIGHT");
            print_str_c(-120+(3*(*box_height)>>2), 120 -(*box_height>>1), (char*)"POKE");
            print_str_c(-120+(  (*box_height)>>2), 120 -*box_height     , (char*)"BAG");
            print_str_c(-120+(  (*box_height)>>2), 120 -(*box_height>>1), (char*)"RUN");
            break;
        case 1:
            if(POKE_MOVE_CATALOGUE[ friendly_poke_party[*friendly_active_poke_index].moves[0] ].type != 0) { print_str_c(-120+(3*(*box_height)>>2), 120 -*box_height     , (char*)( POKE_MOVE_CATALOGUE[ friendly_poke_party[*friendly_active_poke_index].moves[0] ].name )); };
            if(POKE_MOVE_CATALOGUE[ friendly_poke_party[*friendly_active_poke_index].moves[1] ].type != 0) { print_str_c(-120+(3*(*box_height)>>2), 120 -(*box_height>>1), (char*)( POKE_MOVE_CATALOGUE[ friendly_poke_party[*friendly_active_poke_index].moves[1] ].name )); };
            if(POKE_MOVE_CATALOGUE[ friendly_poke_party[*friendly_active_poke_index].moves[2] ].type != 0) { print_str_c(-120+(  (*box_height)>>2), 120 -*box_height     , (char*)( POKE_MOVE_CATALOGUE[ friendly_poke_party[*friendly_active_poke_index].moves[2] ].name )); };
            if(POKE_MOVE_CATALOGUE[ friendly_poke_party[*friendly_active_poke_index].moves[3] ].type != 0) { print_str_c(-120+(  (*box_height)>>2), 120 -(*box_height>>1), (char*)( POKE_MOVE_CATALOGUE[ friendly_poke_party[*friendly_active_poke_index].moves[3] ].name )); };
            break;
        default:
            //pass
            break;
    }    
}

void bugfix_poke_details_flexible(poke_details_flexible *poke_details, int8_t yOrigin, int8_t xOrigin) {
    reset_beam();
    set_scale(128);
    set_text_size(-5, 40);

    uint8_t spacing = 10;
    // NAME
    print_str_c(spacing*2 +yOrigin, 0 +xOrigin, (char*)(poke_details->fixed_details->name));

    char value_buffer[4];
    // HP
    value_buffer[0] = '0'+(poke_details->HP /100);
    value_buffer[1] = '0'+(poke_details->HP /10);
    value_buffer[2] = '0'+(poke_details->HP %10);
    value_buffer[3] = '\0';
    print_str_c(-spacing*0 +yOrigin, 0 +xOrigin, value_buffer);

    // DEF
    value_buffer[0] = '0'+(poke_details->DEF /100);
    value_buffer[1] = '0'+(poke_details->DEF /10);
    value_buffer[2] = '0'+(poke_details->DEF %10);
    value_buffer[3] = '\0';
    print_str_c(-spacing*1 +yOrigin, 0 +xOrigin, value_buffer);

    // SP.DEF
    value_buffer[0] = '0'+(poke_details->SP_DEF /100);
    value_buffer[1] = '0'+(poke_details->SP_DEF /10);
    value_buffer[2] = '0'+(poke_details->SP_DEF %10);
    value_buffer[3] = '\0';
    print_str_c(-spacing*2 +yOrigin, 0 +xOrigin, value_buffer);

    // ATK
    value_buffer[0] = '0'+(poke_details->ATK /100);
    value_buffer[1] = '0'+(poke_details->ATK /10);
    value_buffer[2] = '0'+(poke_details->ATK %10);
    value_buffer[3] = '\0';
    print_str_c(-spacing*3 +yOrigin, 0 +xOrigin, value_buffer);

    // SP.ATK
    value_buffer[0] = '0'+(poke_details->SP_ATK /100);
    value_buffer[1] = '0'+(poke_details->SP_ATK /10);
    value_buffer[2] = '0'+(poke_details->SP_ATK %10);
    value_buffer[3] = '\0';
    print_str_c(-spacing*4 +yOrigin, 0 +xOrigin, value_buffer);

    // SPD
    value_buffer[0] = '0'+(poke_details->SPD /100);
    value_buffer[1] = '0'+(poke_details->SPD /10);
    value_buffer[2] = '0'+(poke_details->SPD %10);
    value_buffer[3] = '\0';
    print_str_c(-spacing*5 +yOrigin, 0 +xOrigin, value_buffer);

    // MOVE 0
    value_buffer[0] = '0'+(poke_details->moves[0] /100);
    value_buffer[1] = '0'+(poke_details->moves[0] /10);
    value_buffer[2] = '0'+(poke_details->moves[0] %10);
    value_buffer[3] = '\0';
    print_str_c(-spacing*7 +yOrigin, 0 +xOrigin, value_buffer);
    // MOVE 1
    value_buffer[0] = '0'+(poke_details->moves[1] /100);
    value_buffer[1] = '0'+(poke_details->moves[1] /10);
    value_buffer[2] = '0'+(poke_details->moves[1] %10);
    value_buffer[3] = '\0';
    print_str_c(-spacing*8 +yOrigin, 0 +xOrigin, value_buffer);
    // MOVE 2
    value_buffer[0] = '0'+(poke_details->moves[2] /100);
    value_buffer[1] = '0'+(poke_details->moves[2] /10);
    value_buffer[2] = '0'+(poke_details->moves[2] %10);
    value_buffer[3] = '\0';
    print_str_c(-spacing*9 +yOrigin, 0 +xOrigin, value_buffer);
    // MOVE 3
    value_buffer[0] = '0'+(poke_details->moves[3] /100);
    value_buffer[1] = '0'+(poke_details->moves[3] /10);
    value_buffer[2] = '0'+(poke_details->moves[3] %10);
    value_buffer[3] = '\0';
    print_str_c(-spacing*10 +yOrigin, 0 +xOrigin, value_buffer);

    // TYPE A
    value_buffer[0] = '0'+(poke_details->fixed_details->types[0] /100);
    value_buffer[1] = '0'+(poke_details->fixed_details->types[0] /10);
    value_buffer[2] = '0'+(poke_details->fixed_details->types[0] %10);
    value_buffer[3] = '\0';
    print_str_c(-spacing*9 +yOrigin, spacing*3 +xOrigin, value_buffer);
    // TYPE B
    value_buffer[0] = '0'+(poke_details->fixed_details->types[1] /100);
    value_buffer[1] = '0'+(poke_details->fixed_details->types[1] /10);
    value_buffer[2] = '0'+(poke_details->fixed_details->types[1] %10);
    value_buffer[3] = '\0';
    print_str_c(-spacing*10 +yOrigin, spacing*3 +xOrigin, value_buffer);
}

void bugfix_combat_info(uint8_t *staging, uint8_t *stage_timer, uint8_t *friendly_active_action, uint8_t *enemy_active_action, uint8_t *friendly_active_poke_index, uint8_t *enemy_active_poke_index, uint8_t *timer, uint8_t *t1_hover_var, int8_t yOrigin, int8_t xOrigin) {
    set_scale(128);
    set_text_size(-5, 40);

    uint8_t spacing = 10;

    char value_buffer[4];
    // STAGING
    value_buffer[0] = '0';
    value_buffer[1] = '0'+(*staging /10);
    value_buffer[2] = '0'+(*staging %10);
    value_buffer[3] = '\0';
    print_str_c(-spacing*0 +yOrigin, 0 +xOrigin, value_buffer);

    // STAGING TIMER
    value_buffer[0] = '0'+(*stage_timer /100);
    value_buffer[1] = '0'+(*stage_timer /10)%10;
    value_buffer[2] = '0'+(*stage_timer %10);
    value_buffer[3] = '\0';
    print_str_c(-spacing*0 +yOrigin, spacing*3 +xOrigin, value_buffer);

    // FRIENDLY ACTIVE ACTION
    value_buffer[0] = '0';
    value_buffer[1] = '0'+(*friendly_active_action /10);
    value_buffer[2] = '0'+(*friendly_active_action %10);
    value_buffer[3] = '\0';
    print_str_c(-spacing*1 +yOrigin, 0 +xOrigin, value_buffer);
    // ENEMY ACTIVE ACTION
    value_buffer[0] = '0';
    value_buffer[1] = '0'+(*enemy_active_action /10);
    value_buffer[2] = '0'+(*enemy_active_action %10);
    value_buffer[3] = '\0';
    print_str_c(-spacing*2 +yOrigin, 0 +xOrigin, value_buffer);

    // FRIENDLY ACTIVE POKE INDEX
    value_buffer[0] = '0';
    value_buffer[1] = '0'+(*friendly_active_poke_index /10);
    value_buffer[2] = '0'+(*friendly_active_poke_index %10);
    value_buffer[3] = '\0';
    print_str_c(-spacing*1 +yOrigin, spacing*3 +xOrigin, value_buffer);
    // ENEMY ACTIVE POKE INDEX
    value_buffer[0] = '0';
    value_buffer[1] = '0'+(*enemy_active_poke_index /10);
    value_buffer[2] = '0'+(*enemy_active_poke_index %10);
    value_buffer[3] = '\0';
    print_str_c(-spacing*2 +yOrigin, spacing*3 +xOrigin, value_buffer);

    // TIMER
    value_buffer[0] = '0'+(*timer /100);
    value_buffer[1] = '0'+(*timer /10)%10;
    value_buffer[2] = '0'+(*timer %10);
    value_buffer[3] = '\0';
    print_str_c(-spacing*3 +yOrigin, 0 +xOrigin, value_buffer);

    // T1 HOVER VAR
    value_buffer[0] = '0';
    value_buffer[1] = '0'+(*t1_hover_var /10);
    value_buffer[2] = '0'+(*t1_hover_var %10);
    value_buffer[3] = '\0';
    print_str_c(-spacing*4 +yOrigin, 0 +xOrigin, value_buffer);

    // ...
}

void set_text(char *target_text, uint8_t text_length, char *reference_text) {
    for(uint8_t i=0; i<text_length; i++) {
        target_text[i] = reference_text[i];
        if(i==text_length-1) {target_text[i] = '\0';}
    }
}
void set_text_subset(char *target_text, uint8_t start_index, uint8_t end_index, char *reference_text) {
    /*
    * NOTE; For encounter_text, DO NOT go beyond its maximum length
    */
    for(uint8_t i=start_index; i<end_index; i++) {
        target_text[i] = reference_text[i-start_index];
        // if(i==end_index-1) {target_text[i] = '\0';}
    }
}

void display_hovered_star(int8_t origin_y, int8_t origin_x, uint8_t radius, uint8_t phase) {
    int8_t base_vector_dots[8] = {
    radius, 0,          // Outer, starting Top going CW
    -radius, radius,
    -radius, -radius,
    radius, -radius,
    };
    int8_t rotated_vector_dots[8];
    rotate(phase, 4, base_vector_dots, rotated_vector_dots);
    reset_beam();
    move(origin_y, origin_x);
    for(uint8_t i=0; i<4; i++) {
    dot(rotated_vector_dots[2*i], rotated_vector_dots[2*i +1]);
    }
    // dots(..., rotated_vector_dots);  // ### Creates a ghost dot offset from others when called here, not sure why ###
}
int8_t linear_to_oscil(uint8_t x) {
  /*
  . Converts x in [0, 256] to y in [-128, 128]
  . Signal maps as follows;
  Input : 0->64,   64->128,  128->192,  192->256
  Output: 0->128, 128->0,     0->-128, -128->0
  */
  if(x<=64) {
    return 2*x -1;      // -1 for smoothing -> flickers at exactly
  } else if(x<=128) {
    return 128-2*(x-64);
  } else if(x<=192) {
    return -2*(x-128);
  } else {
    return -128+2*(x-192);
  }
}
