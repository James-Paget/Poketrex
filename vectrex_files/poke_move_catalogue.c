#include "vectrex.h"
#include "vectrex/bios.h"

#include "poke_move_catalogue.h"

#include "game_constants.h"
#include "core_structs.h"

const poke_move POKE_MOVE_CATALOGUE[POKE_MOVE_CATALOGUE_NUMBER] = {
    {   // Null
        {'N','U','L','L','','','\0'},
        0,
        0,
        0,
        0,
        0,
    },
    {   // Tackle
        {'T','A','C','K','L','E','\0'},
        1,
        3,
        1,
        100,
        100,
    },
    {   // Ember
        {'E','M','B','E','R','','\0'},
        2,
        20,
        20,
        100,
        100,
    },
    // ...
};