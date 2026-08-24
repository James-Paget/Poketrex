#include "game_constants.h"
#include "core_structs.h"

#ifndef ENCOUNTER_TEXT_LENGTH
#define ENCOUNTER_TEXT_LENGTH 25
#endif

#ifndef DISPLAY_COMBAT_H
#define DISPLAY_COMBAT_H

void display_combat_screen(poke_details_flexible *friendly_poke_party, poke_details_flexible *enemy_poke_party, uint8_t *staging, uint8_t *stage_timer, uint8_t *friendly_active_action, uint8_t *enemy_active_action, uint8_t *friendly_active_poke_index, uint8_t *enemy_active_poke_index, uint8_t *timer, uint8_t *t1);

void display_poke_bars(uint8_t *yOffset, uint8_t *xOffset, poke_details_flexible *poke, uint8_t width);
void display_poke_portrait(uint8_t width);
void display_battle_box(int8_t *box_height, uint8_t *staging, uint8_t *timer, uint8_t *hovered_index);

void display_battle_text(poke_details_flexible *friendly_poke_party, poke_details_flexible *enemy_poke_party, uint8_t *friendly_active_action, uint8_t *enemy_active_action, const uint8_t *poke_vertical_offset, int8_t *box_height, const uint8_t *width, uint8_t *staging, uint8_t *stage_timer, char *encounter_text);
void display_poke_stats_text(poke_details_flexible *friendly_poke_party, poke_details_flexible *enemy_poke_party, uint8_t *friendly_active_poke_index, uint8_t *enemy_active_poke_index, const uint8_t *poke_vertical_offset, const uint8_t *width, uint8_t *staging);
void display_encounter_text(int8_t *box_height, uint8_t *staging, uint8_t *stage_timer, char *encounter_text);
void display_action_text(poke_details_flexible *friendly_poke_party, uint8_t *friendly_active_poke_index, int8_t *box_height, uint8_t *staging);

void bugfix_poke_details_flexible(poke_details_flexible *poke_details, int8_t yOrigin, int8_t xOrigin);
void bugfix_combat_info(uint8_t *staging, uint8_t *stage_timer, uint8_t *friendly_active_action, uint8_t *enemy_active_action, uint8_t *friendly_active_poke_index, uint8_t *enemy_active_poke_index, uint8_t *timer, uint8_t *t1_hover_var, int8_t yOrigin, int8_t xOrigin);

void set_text(char *target_text, uint8_t text_length, char *reference_text);
void display_hovered_star(int8_t origin_y, int8_t origin_x, uint8_t radius, uint8_t phase);

#endif