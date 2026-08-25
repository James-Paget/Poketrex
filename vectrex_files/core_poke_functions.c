#include "vectrex.h"
#include "vectrex/bios.h"
#include "cmoc.h"

#include "game_constants.h"
#include "core_structs.h"

#include "core_poke_functions.h"

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
    return 3*(fainted_poke->level);
}