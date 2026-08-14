#include "vectrex.h"
#include "vectrex/bios.h"

#include "poke_fixed_catalogue.h"

#include "game_constants.h"
#include "core_structs.h"

const poke_details_fixed POKE_FIXED_CATALOGUE[POKE_FIXED_CATALOGUE_NUMBER] = {
    {   // Null
        {'N','U','L','L','','','\0'},
        // [],
        {0,0},  
        // [0,0],
    },
    {   // Charmander
        {'C','H','A','R','M','A','\0'},
        // [1,6, 2,8, 4,11],
        {1,0},
        // [15,...],
    },
    {   // Starly
        {'S','T','A','R','L','Y','\0'},
        // [1,6, 2,8, 4,11],
        {2,3},
        // [15,...],
    },
};