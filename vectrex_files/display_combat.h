#include "game_constants.h"
#include "core_structs.h"

#ifndef DISPLAY_COMBAT_H
#define DISPLAY_COMBAT_H

void display_combat_screen(poke_details_flexible *friendly_poke_party, poke_details_flexible *enemy_poke_party, uint8_t *staging, uint8_t *stage_timer, uint8_t *friendly_active_action, uint8_t *enemy_active_action, uint8_t *friendly_active_poke_index, uint8_t *enemy_active_poke_index, uint8_t *timer, uint8_t *t1);

void display_poke(uint8_t radius);

void bugfix_poke_details_flexible(poke_details_flexible *poke_details, int8_t yOrigin, int8_t xOrigin);
void bugfix_combat_info(uint8_t *staging, uint8_t *stage_timer, uint8_t *friendly_active_action, uint8_t *enemy_active_action, uint8_t *friendly_active_poke_index, uint8_t *enemy_active_poke_index, uint8_t *timer, uint8_t *t1_hover_var, int8_t yOrigin, int8_t xOrigin);

#endif