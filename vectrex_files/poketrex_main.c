#include "vectrex.h"
#include "vectrex/bios.h"
#include "poketrex_main.h"

#include "game_constants.h"
#include "core_structs.h"

#include "poke_fixed_catalogue.h"

#include "display_roam.h"
#include "calculate_roam.h"
#include "display_combat.h"
#include "calculate_combat.h"

//...

#include "cmoc.h"

void initialise_poke_parties(poke_details_flexible friendly_poke_party[POKE_PARTY_LENGTH], poke_details_flexible enemy_poke_party[POKE_PARTY_LENGTH]) {
  /*
  . Initialises the friendly and enemy poke parties with blank(/starter) spaces
  */
  poke_details_flexible poke_flexible_null = {
    0,

    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,

    {1,2,0,0},

    0,

    &(POKE_FIXED_CATALOGUE[0]),
  };

  poke_details_flexible poke_flexible_charmander = {
    13,

    32,
    30,
    2,
    -1,
    4,
    6,
    2,
    5,
    20,

    {2,1,0,0},

    0,

    &(POKE_FIXED_CATALOGUE[1]),
  };

  poke_details_flexible poke_flexible_starly = {
    5,

    30,
    11,
    1,
    0,
    2,
    8,
    1,
    5,
    24,

    {1,2,0,0},

    0,

    &(POKE_FIXED_CATALOGUE[2]),
  };

  friendly_poke_party[0] = poke_flexible_charmander;
  friendly_poke_party[1] = poke_flexible_null;
  friendly_poke_party[2] = poke_flexible_null;
  friendly_poke_party[3] = poke_flexible_null;
  friendly_poke_party[4] = poke_flexible_null;
  friendly_poke_party[5] = poke_flexible_null;

  enemy_poke_party[0] = poke_flexible_starly;
  enemy_poke_party[1] = poke_flexible_null;
  enemy_poke_party[2] = poke_flexible_null;
  enemy_poke_party[3] = poke_flexible_null;
  enemy_poke_party[4] = poke_flexible_null;
  enemy_poke_party[5] = poke_flexible_null;
};

int main() {
  /*
  . screen_mode = 
    0 = Roaming screen
    1 = Combat screen
    2 = Poke summary screen (OUT of combat)
    3 = Poke switch screen (IN combat)
    4 = Bag summary screen (OUT of combat)
    5 = Bag select screen (IN combat)
  */
  // Global variables
  uint8_t screen_mode = 0;        // Which type of screen to display and calculate for

  uint8_t timer = 0;  // Ticks throughout the entire program - Used for animations

  poke_details_flexible friendly_poke_party[POKE_PARTY_LENGTH];
  poke_details_flexible enemy_poke_party[POKE_PARTY_LENGTH];

  // Combat variables
  uint8_t staging = 0;      // Tracks which part of the combat sequence you are currently in - Used for button pressing sequences as well as automatic fight displays
  uint8_t stage_timer = 0;  // Tracks how far through current stage you are (used for text animations and progression to next timer at its max value)
  uint8_t stage_speed = 1;  // Tracks how many timer ticks before stage_timer is increased e.g. ==N => stage_timer 1/N speed of base timer clock

  uint8_t t1 = 0;   // Used for hovered indices, such as action hovered, move hovered, etc

  uint8_t friendly_active_poke_index = 0;   // Index of poke in either party which is currently in play
  uint8_t enemy_active_poke_index = 0;      //

  uint8_t friendly_active_action = 0;       // Int used to determine which action (move, item, etc) is being used; Action indices are defined in the calulate_combat.c file
  uint8_t enemy_active_action = 0;          //

  int8_t poke_first_counter = 0;            // Value of the counter currently applied to the poke -> This is specified in each stage where appropriate
  int8_t poke_second_counter = 0;           //
  uint8_t poke_first_counter_type = 0;      // Type of counter for the poke -> Specified in the stage where appropriate (e.g. 0=None, 1=HP, ..., N=SPD, ...)
  uint8_t poke_second_counter_type = 0;     //

  uint8_t is_critical = 0;  // Tracks whether an attack is critical - considers the poke being considered currently in staging only (0=NoCrit, 1=Crit)
  uint8_t is_miss = 0;      // "" "" --> ### NOTE; THIS COULD BE REDUCED BY HAVING A SINGLE VARIABLE AND READING THE FIRST AND LAST 4 BITS INSTEAD -> TWICE THE DATA STORAGE ###

  // Initialisation
  initialise_poke_parties(friendly_poke_party, enemy_poke_party);

  while(1)
  {
    wait_retrace();
    // Display & Calculation
    switch(screen_mode) {
      case 0:
        process_roam_screen( &screen_mode, &(enemy_poke_party[0]) );
        break;
      case 1:
        process_combat_screen( &screen_mode, &(friendly_poke_party[0]), &(enemy_poke_party[0]), &staging, &stage_timer, &stage_speed, &friendly_active_action, &enemy_active_action, &friendly_active_poke_index, &enemy_active_poke_index, &poke_first_counter, &poke_second_counter, &poke_first_counter_type, &poke_second_counter_type, &is_critical, &is_miss, &timer, &t1 );
        break;
      // ...
      default:
        //pass
        break;
    }
    timer = timer +1; // Progress timer always (ONLY PERFORMED HERE) <- Can be adjusted to change game animation speeds (and hence combat stage durations too)
  }
  return 0;
};

void process_roam_screen(uint8_t *screen_mode, poke_details_flexible *enemy_poke_party) {
  /*
  */
  // Display and calculate for this screen
  display_roam_screen();
  calculate_roam_screen(screen_mode, enemy_poke_party);
}

void process_combat_screen(uint8_t *screen_mode, poke_details_flexible *friendly_poke_party, poke_details_flexible *enemy_poke_party, uint8_t *staging, uint8_t *stage_timer, uint8_t *stage_speed, uint8_t *friendly_active_action, uint8_t *enemy_active_action, uint8_t *friendly_active_poke_index, uint8_t *enemy_active_poke_index, int8_t *poke_first_counter, int8_t *poke_second_counter, uint8_t *poke_first_counter_type, uint8_t *poke_second_counter_type, uint8_t *is_critical, uint8_t *is_miss, uint8_t *timer, uint8_t *t1) {
  /*
  . staging = Which stage of combat you are in, e.g.;
    0 = Select action: moves, items, switch, run
    1 = Select specific move action
    2 = Action chosen, play out combat
    ...

  . XXX_active_poke_index = Which index in the poke party is the currently active poke for friendly/enemy pokes

  . XXX_active_action = The action to be taken by friendly/enemy poke, e.g.;
    0       = Unselected action
    1-4     = Move index to be used (N-1th move index)
    5-10    = Poke switch (N-5th poke index)
    11      = Run attempt
    11-19     = [UNASSIGNED]
    20-...  = Item index from bag to be used (N-20th bag index)   <-- ### Overflow issue if too many items (very unlikely currently) ###
  */
  // Display and calculate for this screen
  display_combat_screen(friendly_poke_party, enemy_poke_party, staging, stage_timer, friendly_active_action, enemy_active_action, friendly_active_poke_index, enemy_active_poke_index, timer, t1);
  calculate_combat_screen(screen_mode, friendly_poke_party, enemy_poke_party, staging, stage_timer, stage_speed, friendly_active_action, enemy_active_action, friendly_active_poke_index, enemy_active_poke_index, poke_first_counter, poke_second_counter, poke_first_counter_type, poke_second_counter_type, is_critical, is_miss, timer, t1);
}
