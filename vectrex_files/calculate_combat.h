#include "game_constants.h"
#include "core_structs.h"

#ifndef CALC_COMBAT_H
#define CALC_COMBAT_H

void calculate_combat_screen(poke_details_flexible *friendly_poke_party, poke_details_flexible *enemy_poke_party, uint8_t *staging, uint8_t *stage_timer, uint8_t *stage_speed, uint8_t *friendly_active_action, uint8_t *enemy_active_action, uint8_t *friendly_active_poke_index, uint8_t *enemy_active_poke_index, uint8_t *timer, uint8_t *t1);

void calculate_combat_buttons_actionSelection(uint8_t *friendly_hovered_action, uint8_t *staging);
void calculate_combat_buttons_moveSelection(poke_details_flexible *friendly_poke_party, uint8_t *friendly_active_action, uint8_t *friendly_active_poke_index, uint8_t *friendly_hovered_move, uint8_t *staging);

void calculate_combat_buttons(poke_details_flexible *friendly_poke_party, poke_details_flexible *enemy_poke_party);

#endif