#include "game_constants.h"
#include "core_structs.h"

#ifndef POKETREX_H
#define POKETREX_H

void initialise_poke_parties(poke_details_flexible friendly_poke_party[POKE_PARTY_LENGTH], poke_details_flexible enemy_poke_party[POKE_PARTY_LENGTH]);

int main();

void process_combat_screen(poke_details_flexible *friendly_poke_party, poke_details_flexible *enemy_poke_party, uint8_t *staging, uint8_t *stage_timer, uint8_t *stage_speed, uint8_t *friendly_active_action, uint8_t *enemy_active_action, uint8_t *friendly_active_poke_index, uint8_t *enemy_active_poke_index, uint8_t *timer, uint8_t *t1);
// ...


#endif