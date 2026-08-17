#ifndef POKE_NAME_LENGTH        // Replaces variable with the value directly, hence has no direct data type; hence in program use may want to create a typed variable with this FIRST before usage otherwise

#define POKE_NAME_LENGTH 7      // Max length of the name of any poke --> Note** This length INCLUDES the final endline character; hence real length is THIS-1
#define POKE_MOVE_NAME_LENGTH 7 // Max length of the name of any poke move name -> final char endline inclusive
#define POKE_PARTY_LENGTH 6     // Number of poke allowed in each poke party (friendly and enemy)
//...

#endif