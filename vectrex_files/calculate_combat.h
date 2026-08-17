#include "game_constants.h"
#include "core_structs.h"

#ifndef CALC_COMBAT_H
#define CALC_COMBAT_H

void calculate_combat_screen(poke_details_flexible *friendly_poke_party, poke_details_flexible *enemy_poke_party, uint8_t *staging, uint8_t *stage_timer, uint8_t *stage_speed, uint8_t *friendly_active_action, uint8_t *enemy_active_action, uint8_t *friendly_active_poke_index, uint8_t *enemy_active_poke_index, int8_t *poke_first_counter, int8_t *poke_second_counter, uint8_t *poke_first_counter_type, uint8_t *poke_second_counter_type, uint8_t *is_critical, uint8_t *is_miss, uint8_t *timer, uint8_t *t1);

void calculate_combat_buttons_actionSelection(uint8_t *friendly_hovered_action, uint8_t *staging);
void calculate_combat_buttons_moveSelection(poke_details_flexible *friendly_poke_party, uint8_t *friendly_active_action, uint8_t *friendly_active_poke_index, uint8_t *friendly_hovered_move, uint8_t *staging);

void calculate_combat_buttons(poke_details_flexible *friendly_poke_party, poke_details_flexible *enemy_poke_party);

uint8_t GET_POKE_DAMAGE(poke_details_flexible *p1, poke_details_flexible *p2, const poke_move *m, uint8_t *is_critical, uint8_t *is_miss);
uint8_t GET_MOVE_CRITICAL(const poke_move *m);
uint8_t GET_MOVE_MISS(const poke_move *m);
uint8_t GET_EFFECTIVENESS(uint8_t *types, const poke_move *m);

#endif