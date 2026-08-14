#include "vectrex.h"
#include "vectrex/bios.h"
#include "cmoc.h"

#include "game_constants.h"
#include "core_structs.h"

#include "display_combat.h"

/*
-------------
BATTLE_ASSETS
-------------
. Holds all functions related the vector display within the battle mode


    // #####################
    // #####################
    // #####################
    // #####################
    reset_beam(); /// <--- very very slow when repeated

    Brightness slows program too

    Can manually lower refresh rate too

    Smarter asset linking

    CAN draw every other frame for low detail aspects, etc
*/

void display_combat_screen(poke_details_flexible *friendly_poke_party, poke_details_flexible *enemy_poke_party, uint8_t *staging, uint8_t *friendly_active_action, uint8_t *enemy_active_action, uint8_t *friendly_active_poke_index, uint8_t *enemy_active_poke_index, uint8_t *t1) {
    reset_beam();
    // display_poke(20);
    bugfix_poke_details_flexible(&(friendly_poke_party[0]), 0, -45);
    bugfix_poke_details_flexible(&(enemy_poke_party[0]), 0, 45);

    bugfix_combat_info(staging, friendly_active_action, enemy_active_action, friendly_active_poke_index, enemy_active_poke_index, t1, 60, 0);
}

void display_poke(uint8_t radius) {
    // Note** Draws the poke centered at the current beam position
    int8_t vector_lines[8] = {    // ** Note; Non-const as lines requires just char[] argument
        0, 2*radius,    // Top-left, travelling CW
        -2*radius, 0,
        0, -2*radius,
        2*radius, 0,
    };
    move(radius, -radius);
    intensity(0x7f);
    lines(4, vector_lines);
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

void bugfix_combat_info(uint8_t *staging, uint8_t *friendly_active_action, uint8_t *enemy_active_action, uint8_t *friendly_active_poke_index, uint8_t *enemy_active_poke_index, uint8_t *t1_hover_var, int8_t yOrigin, int8_t xOrigin) {
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

    // T1 HOVER VAR
    value_buffer[0] = '0';
    value_buffer[1] = '0'+(*t1_hover_var /10);
    value_buffer[2] = '0'+(*t1_hover_var %10);
    value_buffer[3] = '\0';
    print_str_c(-spacing*3 +yOrigin, 0 +xOrigin, value_buffer);

    // ...
}