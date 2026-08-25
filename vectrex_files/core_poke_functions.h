#ifndef CORE_POKE_FUNCS
#define CORE_POKE_FUNCS

uint8_t GET_POKE_DAMAGE(poke_details_flexible *p1, poke_details_flexible *p2, const poke_move *m, uint8_t *is_critical, uint8_t *is_miss);
uint8_t GET_MOVE_CRITICAL(const poke_move *m);
uint8_t GET_MOVE_MISS(const poke_move *m);
uint8_t GET_EFFECTIVENESS(const uint8_t *types, const poke_move *m);
uint8_t GET_EXPERIENCE(poke_details_flexible *fainted_poke);

#endif