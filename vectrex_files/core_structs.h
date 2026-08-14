#include "game_constants.h"

#ifndef POKE_DETAILS_FIX_H
#define POKE_DETAILS_FIX_H
/*
. Stores the const details about a poke, such as;
    - Name
    - Learnable moves /w levels
    - Types
    - Evolutions /w levels
*/
typedef struct {
    const char name[POKE_NAME_LENGTH];

    // const uint8_t learnable_move_number;
    // const uint8_t[2*learnable_move_number];     // [move_index, level_learned_at, ..., ...] repeated

    const uint8_t types[2];

    // const uint8_t evolution_level;  // Minimum level requirement for the poke to evolve (can only evolve into one other poke)
    // const poke_details_fixed* evolution_poke;   // Fixed details for the poke this poke will evolve into
} poke_details_fixed;

#endif

#ifndef POKE_DETAILS_FLEX_H
#define POKE_DETAILS_FLEX_H
/*
. Stores variable details about a poke, such as;
    - Level
    - Stats (HP_MAX, HP, DEF, SP.DEF, ATK, SP.ATK, SPD)
    - Current moves
    - The fixed poke details relating to this poke
*/
typedef struct {
    uint8_t level;

    uint8_t HP_MAX;     // >=0 only
    uint8_t HP;         
    int8_t DEF;         // Negative allowed
    int8_t SP_DEF;
    int8_t ATK;
    int8_t SP_ATK;
    int8_t SPD;

    uint8_t moves[4];

    uint8_t status_effect;  // 0=None, 1=Poison, 2= ...

    const poke_details_fixed* fixed_details;   // Pointer to the fixed details about this poke (swapped out on evolution)
} poke_details_flexible;

#endif