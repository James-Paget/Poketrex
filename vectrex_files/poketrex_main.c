#include "vectrex.h"
#include "vectrex/bios.h"
#include "poketrex_main.h"
#include "cmoc.h"

void concat_strings(char *buffer, uint8_t *buffer_length, const uint8_t str_number, const char *str_additions[], const uint8_t str_addition_lengths[]) {
  *buffer_length = 0;
  for(uint8_t i=0; i<str_number; i++) {
    for(uint8_t j=0; j<str_addition_lengths[i]; j++) {
      buffer[*buffer_length +j] = (str_additions[i])[j];
    }
    *buffer_length = *buffer_length +str_addition_lengths[i];
  }
  buffer[*buffer_length] = '\0';
}

uint8_t sum_256(uint8_t *list, uint8_t list_length) {
  /*
  . Sums the integers in the list provided and returns the uint8_t total (hence 256 max total)
  */
  uint8_t total = 0;
  for(uint8_t i=0; i<list_length; i++) {
    total += list[i];
  }
  return total;
}

int8_t linear_to_oscil(uint8_t x) {
  /*
  . Converts x in [0, 256] to y in [-128, 128]
  . Signal maps as follows;
  Input : 0->64,   64->128,  128->192,  192->256
  Output: 0->128, 128->0,     0->-128, -128->0
  */
  if(x<=64) {
    return 2*x -1;      // -1 for smoothing -> flickers at exactly
  } else if(x<=128) {
    return 128-2*(x-64);
  } else if(x<=192) {
    return -2*(x-128);
  } else {
    return -128+2*(x-192);
  }
}

// Poke displays
void display_tree(int8_t origin_y, int8_t origin_x, uint8_t radius) {
  /*
  . Displays a tree within a cube of half-width 'radius', centered at the origin
  */
  int8_t vector_lines[8] = {    // ** Note; Non-const as lines requires just char[] argument
    -radius, radius,  // Center upper section of tree start, travelling roughly CW
    0, -2*radius,
    radius, radius,
    -2*radius, 0,
  };
  reset_beam();
  move(origin_y, origin_x-radius);
  intensity(0x7f);
  lines(4, vector_lines);
};
void display_grass(int8_t origin_y, int8_t origin_x, uint8_t radius) {
  /*
  . Displays a tree within a cube of half-width 'radius', centered at the origin
  */
  int8_t vector_lines[8] = {    // ** Note; Non-const as lines requires just char[] argument
    radius, radius>>1,     // Left bottom side of plant, travelling right
    -radius, radius>>1,
    radius, radius>>1,
    -radius, radius>>1,
  };
  reset_beam();
  move(origin_y, origin_x-radius);
  intensity(0x7f);
  lines(4, vector_lines);
};
void display_healstation(int8_t origin_y, int8_t origin_x, uint8_t radius) {
  /*
  . Displays a cube of half-width equal to radius, and centered at the origin
  */
  int8_t vector_lines[10] = {    // ** Note; Non-const as lines requires just char[] argument
    0, 2*radius,    // Top-left, travelling CW
    -2*radius, 0,
    0, -2*radius,
    2*radius, 0,
    -2*radius, 2*radius,
  };
  reset_beam();
  move(origin_y+radius, origin_x-radius);
  intensity(0x7f);
  lines(5, vector_lines);
};

// Generic displays
void display_cube(int8_t origin_y, int8_t origin_x, uint8_t radius) {
  /*
  . Displays a cube of half-width equal to radius, and centered at the origin
  */
  int8_t vector_lines[8] = {    // ** Note; Non-const as lines requires just char[] argument
    0, 2*radius,    // Top-left, travelling CW
    -2*radius, 0,
    0, -2*radius,
    2*radius, 0,
  };
  reset_beam();
  move(origin_y+radius, origin_x-radius);
  intensity(0x7f);
  lines(4, vector_lines);
};
void display_rect(int8_t origin_y, int8_t origin_x, uint8_t hheight, uint8_t hwidth) {
  /*
  . Displays a rectangle of half-width and height as specified, centered at the origin
  */
  int8_t vector_lines[8] = {    // ** Note; Non-const as lines requires just char[] argument
    0, 2*hwidth,    // Top-left, travelling CW
    -2*hheight, 0,
    0, -2*hwidth,
    2*hheight, 0,
  };
  reset_beam();
  move(origin_y+hheight, origin_x-hwidth);
  intensity(0x7f);
  lines(4, vector_lines);
};
void display_hovered_star(int8_t origin_y, int8_t origin_x, uint8_t radius, uint8_t phase) {
  int8_t base_vector_dots[8] = {
    radius, 0,          // Outer, starting Top going CW
    -radius, radius,
    -radius, -radius,
    radius, -radius,
  };
  int8_t rotated_vector_dots[8];
  rotate(phase, 4, base_vector_dots, rotated_vector_dots);
  reset_beam();
  move(origin_y, origin_x);
  for(uint8_t i=0; i<4; i++) {
    dot(rotated_vector_dots[2*i], rotated_vector_dots[2*i +1]);
  }
  // dots(..., rotated_vector_dots);  // ### Creates a ghost dot offset from others when called here, not sure why ###
}


uint8_t fetch_move_effectiveness(const uint8_t type_effectiveness_lookup[324], uint8_t move_type, const uint8_t recipient_types[2]) {
  /*
  . Considers the move type and recipient type to give an effectiveness score
  . Moves can only have 1 type, recipients can have up to 2 types

  0 = Not Effect
  1 = Not Very Effective
  2 = Normal Effectiveness
  4 = Super Effective
  */
 // ### NEEDS TO TAKE INTO ACCOUNT 2nd TYPE AS WELL --> SEE WHAT ACTUAL IMPLEMENTATION IS ###
  return type_effectiveness_lookup[18*(uint16_t)move_type +recipient_types[0]];
}

// -------------------
// -- BATTLE_SCREEN --
// -------------------
void display_battle_screen(poke_info ally_poke_party[6], uint8_t *active_ally_poke_index, poke_info hostile_poke_party[6], uint8_t *active_hostile_poke_index, const poke_move *poke_move_lookup, uint8_t *battle_item_index, char *battle_log_complete, uint8_t *battle_log_stage_length, uint8_t hovered_battle_option, uint8_t *battle_screen_variant, uint8_t *battle_outcome_stage, uint8_t *battle_outcome_timer, uint8_t battle_mode, uint8_t *timer) {
  /*
  . Displays the battle screen which consists of;
    . Sprites for both pokemon
      . +Health and experience for pokemon
      . +Battle arena floor (OPTIONAL)
    . Battle options
      . Fight, Run, Bag, Switch
    . Descriptive fight log
    . Animations for actions -> Calculate and alter sprite offsets by reference
  */
  const uint8_t battle_log_extension = 20;       // Half-width of the battle log that EXTENDS beyond the half-way section of the screen (1/2 way minimum); The battle options will fill the remainder of the width
  const uint8_t battle_options_hheight = 10;  // Half-height of each battle option box (fight, run, etc) 

  set_scale(128);

  display_battle_screen_pokemons(&(ally_poke_party[*active_ally_poke_index]), &(hostile_poke_party[*active_hostile_poke_index]), timer);  // ### PARSE IN POKEMON HERE ### <-- POINTERS
  switch(*battle_screen_variant) {
    case 0:
      display_battle_screen_battle_options(hovered_battle_option, battle_log_extension, battle_options_hheight, timer);
      break;
    case 1:
      display_battle_screen_battle_options_fight(&(ally_poke_party[*active_ally_poke_index]), poke_move_lookup, hovered_battle_option, battle_log_extension, battle_options_hheight, timer);
      break;
    // Case 5 (view battle after move selection) will just show everything but these options
  }
  display_battle_screen_battle_log(battle_log_complete, battle_log_stage_length, battle_outcome_stage, battle_outcome_timer, battle_log_extension, battle_options_hheight);

  // reset_beam();
  // set_scale(128);
  // set_text_size(-5, 50);
  // char int_char_stage[3];
  // int_char_stage[0] = '0' + ((*battle_outcome_stage) / 10);
  // int_char_stage[1] = '0' + ((*battle_outcome_stage) % 10);
  // int_char_stage[2] = '\0';
  // print_str_c(0, 0, int_char_stage);

  // char int_char_timer[4];
  // int_char_timer[0] = '0' + ((*battle_outcome_timer) / 100);
  // int_char_timer[1] = '0' + ((*battle_outcome_timer) / 10);
  // int_char_timer[2] = '0' + ((*battle_outcome_timer) % 10);
  // int_char_timer[3] = '\0';
  // print_str_c(20, 0, int_char_timer);
};


void display_battle_screen_pokemons(poke_info *poke_ally, poke_info *poke_hostile, uint8_t *timer) {
  /*
  . Displays both pokemon involved in a battle
  . Position of their display is fixed
  */
  const uint8_t poke_radius = 20;
  const int8_t poke_friendly_position_y = -40;  // Lower half --> Will always display the sprite and poke-bar on opposite sides of the screen horizontally, only the height is required
  const int8_t poke_hostile_position_y = 80;    // Upper half
  // ### HAVE OFFSETS FOR ANIMATIONS REMEMVERED HERE ###

  const uint8_t poke_stats_dimensions[2] = {10, 50};//{10, 40};   // (hheight, hwidth) format

  display_battle_screen_pokemon(poke_ally, poke_friendly_position_y, poke_radius, poke_stats_dimensions, 1, timer);
  display_battle_screen_pokemon(poke_hostile, poke_hostile_position_y, poke_radius, poke_stats_dimensions, 0, timer);
};
void display_battle_screen_pokemon(poke_info *poke, int8_t poke_position_y, uint8_t poke_radius, const uint8_t *poke_stats_dimensions, const uint8_t isLeftSide, uint8_t *timer) {
  /*
  . Displays a specific pokemon in the battle screen
  . Displays relevant information about the pokemon alongside its sprite, such as;
    . Health
    . Experience
  */
  int8_t poke_position_x = (isLeftSide==1) ? (int8_t)(-128+(2*poke_radius)) : (int8_t)(poke_position_x = 128-2*poke_radius);  // L / R side
  int8_t position_y_wobble = (int8_t)( ((int16_t)(linear_to_oscil(*timer +50*isLeftSide))*5) >> 7 );  // Note**; The left side addition in timer is used to offset the oscillation phase

  display_cube( poke_position_y+position_y_wobble, poke_position_x, poke_radius );   // Poke sprite
  display_battle_screen_pokemon_stats(poke, poke_position_y, isLeftSide, poke_stats_dimensions);  // Poke stats bar
};
void display_battle_screen_pokemon_stats(poke_info *poke, int8_t bar_position_y, const uint8_t isLeftSide, const uint8_t *poke_stats_dimensions) {
  /*
  . Displays the health and experience of a pokemon
  
  Note; 'isLeftSide' refers to the position of the pokemon the bar is linked to, hence if isLeftSide==1, then the bar is on the RIGHT, not the left
  */
  int8_t lean = 5; // Tilt on either end of the outer bar shape + experience bar width
  int8_t bar_position_x = (isLeftSide==1) ? (int8_t)(128-(poke_stats_dimensions[1]+5)) : (int8_t)(-128+(poke_stats_dimensions[1]+5));  // L / R side

  int8_t vector_lines[14] = {  // Start on left-middle (experience bar left edge), end on top of right-middle
    2*poke_stats_dimensions[0], lean,
    0, 2*poke_stats_dimensions[1], 
    -2*poke_stats_dimensions[0], -lean,
    0, -2*poke_stats_dimensions[1],
    -(2*poke_stats_dimensions[0])>>1, 0,
    0, 2*poke_stats_dimensions[1],
    (2*poke_stats_dimensions[0])>>1, 0,
  };

  uint8_t health = poke->health;
  uint8_t health_max = poke->health_max;
  const uint8_t health_ratio = (uint8_t)( ((uint16_t)health << 8) /health_max );  // As a ratio of 256, truncated in division if fractional part remaining

  uint8_t experience = poke->experience;
  uint8_t experience_max = poke->experience_max;
  const uint8_t experience_ratio = (uint8_t)( ((uint16_t)experience << 8) /experience_max );


  // ### Can collapse this to be done in fewer vectors if wobble is seen ###
  reset_beam();
  move(bar_position_y-2*poke_stats_dimensions[0], bar_position_x-poke_stats_dimensions[1]);
  lines(7, vector_lines); // Draw cage + experience bar, beam ends at bottom-right (above experience)

  const uint8_t bar_buffer = 2;
  // Draw health
  const uint8_t health_vector_distance = (uint8_t)( ((uint16_t)(health_ratio)*2*(poke_stats_dimensions[1]-bar_buffer)) >>8);
  move(-(2*poke_stats_dimensions[0]>>3), -2*poke_stats_dimensions[1] +bar_buffer);
  line(0, health_vector_distance);

  // Draw experience
  const uint8_t experience_vector_distance = (uint8_t)( ((uint16_t)(experience_ratio)*2*(poke_stats_dimensions[1]-bar_buffer)) >>8);
  move(-(2*poke_stats_dimensions[0]>>2) +1, -health_vector_distance);
  line(0, experience_vector_distance);

  // Draw name and level
  char poke_name[6] = {poke->name[0], poke->name[1], poke->name[2], poke->name[3], '.', '\0'};  // Shorten name to fit into stats box with clear lettering still
  char poke_level[6] = {'L','V','L', '0'+(poke->level)/10, '0'+(poke->level)%10, '\0'};
  reset_beam();
  set_text_size( -(poke_stats_dimensions[0]>>1), poke_stats_dimensions[1] );
  print_str_c(bar_position_y -(poke_stats_dimensions[0]>>1), bar_position_x-(poke_stats_dimensions[1]) +(poke_stats_dimensions[1]>>2), poke_name);
  print_str_c(bar_position_y -(poke_stats_dimensions[0]>>1), bar_position_x, poke_level);
  set_scale(128);
}


void display_battle_screen_battle_options(uint8_t hovered_option, const uint8_t battle_log_extension, const uint8_t battle_options_hheight, uint8_t *timer) {
  /*
  . Displays the 4 battle options in the bottom-right corner of the screen;
    . Fight . Pokemon
    . Bag   . Run
  */
  uint8_t buffer = 2;
  uint8_t option_hwidth = (128-battle_log_extension+2*buffer)/4;  // Truncated float to uint8_t

  reset_beam();
  // Draw boxes
  // Top-Left, Going CW
  display_rect(-128+buffer+2*battle_options_hheight +1*battle_options_hheight, 128-buffer-2*option_hwidth -1*option_hwidth, battle_options_hheight, option_hwidth);
  display_rect(-128+buffer+2*battle_options_hheight +1*battle_options_hheight, 128-buffer-2*option_hwidth +1*option_hwidth, battle_options_hheight, option_hwidth);
  display_rect(-128+buffer+2*battle_options_hheight -1*battle_options_hheight, 128-buffer-2*option_hwidth +1*option_hwidth, battle_options_hheight, option_hwidth);
  display_rect(-128+buffer+2*battle_options_hheight -1*battle_options_hheight, 128-buffer-2*option_hwidth -1*option_hwidth, battle_options_hheight, option_hwidth);

  // Draw labels
  reset_beam();
  set_text_size(-battle_options_hheight>>1, 4*battle_options_hheight);
  print_str_c(-128+buffer+2*battle_options_hheight +1*battle_options_hheight +(battle_options_hheight>>1), 128-buffer-2*option_hwidth -1*option_hwidth -(option_hwidth>>1), (char*)"FIGHT");
  print_str_c(-128+buffer+2*battle_options_hheight +1*battle_options_hheight +(battle_options_hheight>>1), 128-buffer-2*option_hwidth +1*option_hwidth -(option_hwidth>>1), (char*)"BAG");
  print_str_c(-128+buffer+2*battle_options_hheight -1*battle_options_hheight +(battle_options_hheight>>1), 128-buffer-2*option_hwidth +1*option_hwidth -(option_hwidth>>1), (char*)"RUN");
  print_str_c(-128+buffer+2*battle_options_hheight -1*battle_options_hheight +(battle_options_hheight>>1), 128-buffer-2*option_hwidth -1*option_hwidth -(option_hwidth>>1), (char*)"POKE");
  set_scale(128);

  // Draw hovered icon
  reset_beam();
  if( (0 <= hovered_option) && (hovered_option < 4) ) {
    uint8_t y_factor = (hovered_option<2) ? 1 : -1;     // Chooses the box to hovered from the hovered index given
    uint8_t x_factor = (hovered_option%2==0) ? -1 : 1;  //
    display_hovered_star(
      -128+buffer+2*battle_options_hheight +y_factor*battle_options_hheight +(battle_options_hheight>>1), 
      128-buffer-2*option_hwidth +x_factor*option_hwidth -(option_hwidth>>1) -(option_hwidth>>2), 
      2, 
      *timer
    );
    display_hovered_star(
      -128+buffer+2*battle_options_hheight +y_factor*battle_options_hheight +(battle_options_hheight>>1), 
      128-buffer-2*option_hwidth +x_factor*option_hwidth +(option_hwidth>>1) +(option_hwidth>>2), 
      2, 
      *timer
    );
  }
};

void display_battle_screen_battle_options_fight(poke_info *poke, const poke_move *poke_move_lookup, uint8_t hovered_option, const uint8_t battle_log_extension, const uint8_t battle_options_hheight, uint8_t *timer) {
  /*
  . Displays the 4 moves the pokemon can use
    . Move1 . Move2
    . Move3 . Move4
  */
  uint8_t buffer = 2;
  uint8_t option_hwidth = (128-battle_log_extension+2*buffer)/4;  // Truncated float to uint8_t
  
  reset_beam();
  // Draw boxes
  // Top-Left, Going CW
  display_rect(-128+buffer+2*battle_options_hheight +1*battle_options_hheight, 128-buffer-2*option_hwidth -1*option_hwidth, battle_options_hheight, option_hwidth);
  display_rect(-128+buffer+2*battle_options_hheight +1*battle_options_hheight, 128-buffer-2*option_hwidth +1*option_hwidth, battle_options_hheight, option_hwidth);
  display_rect(-128+buffer+2*battle_options_hheight -1*battle_options_hheight, 128-buffer-2*option_hwidth +1*option_hwidth, battle_options_hheight, option_hwidth);
  display_rect(-128+buffer+2*battle_options_hheight -1*battle_options_hheight, 128-buffer-2*option_hwidth -1*option_hwidth, battle_options_hheight, option_hwidth);

  // Draw labels
  reset_beam();
  set_text_size(-battle_options_hheight>>1, 4*battle_options_hheight);
  print_str_c(-128+buffer+2*battle_options_hheight +1*battle_options_hheight +(battle_options_hheight>>1), 128-buffer-2*option_hwidth -1*option_hwidth -(option_hwidth>>1), (char*)(poke_move_lookup[ (poke->moves)[0] ].name) );
  print_str_c(-128+buffer+2*battle_options_hheight +1*battle_options_hheight +(battle_options_hheight>>1), 128-buffer-2*option_hwidth +1*option_hwidth -(option_hwidth>>1), (char*)poke_move_lookup[ (poke->moves)[1] ].name );
  print_str_c(-128+buffer+2*battle_options_hheight -1*battle_options_hheight +(battle_options_hheight>>1), 128-buffer-2*option_hwidth +1*option_hwidth -(option_hwidth>>1), (char*)poke_move_lookup[ (poke->moves)[2] ].name );
  print_str_c(-128+buffer+2*battle_options_hheight -1*battle_options_hheight +(battle_options_hheight>>1), 128-buffer-2*option_hwidth -1*option_hwidth -(option_hwidth>>1), (char*)poke_move_lookup[ (poke->moves)[3] ].name );
  set_scale(128);

  // Draw hovered icon
  reset_beam();
  if( (0 <= hovered_option) && (hovered_option < 4) ) {
    uint8_t y_factor = (hovered_option<2) ? 1 : -1;     // Chooses the box to hovered from the hovered index given
    uint8_t x_factor = (hovered_option%2==0) ? -1 : 1;  //
    display_hovered_star(
      -128+buffer+2*battle_options_hheight +y_factor*battle_options_hheight +(battle_options_hheight>>1), 
      128-buffer-2*option_hwidth +x_factor*option_hwidth -(option_hwidth>>1) -(option_hwidth>>2), 
      2, 
      *timer
    );
    display_hovered_star(
      -128+buffer+2*battle_options_hheight +y_factor*battle_options_hheight +(battle_options_hheight>>1), 
      128-buffer-2*option_hwidth +x_factor*option_hwidth +(option_hwidth>>1) +(option_hwidth>>2), 
      2, 
      *timer
    );
  }
};


void fetch_battle_screen_battle_log(const uint8_t type_effectiveness_lookup[324], char *battle_log_complete, uint8_t *battle_log_stage_length, const poke_move *poke_move_lookup, poke_info ally_poke_party[6], uint8_t *active_ally_poke_index, poke_info hostile_poke_party[6], uint8_t *active_hostile_poke_index, int8_t *ally_poke_counter, int8_t *hostile_poke_counter, const poke_item *battle_item, uint8_t *battle_outcome_stage, uint8_t *battle_outcome_timer) {
  /*
  . Gets the entire battle log for both poke sides in one long string
  . Also updates the battle_log_stage_length with the number of characters in each subset of this entire string to get the log for each stage

  . The battle_log_stage_length states the index (from the BEGINNING) that the stage text ends at => the subset length is the difference in adjacent end indices
  . Note; If a stage has a subset length of 0 then the stage can be skipped immediately

  The battle log is constructed in the following order
  (1) Declare catching poke
  (2) Catching shake 1  -> Tracked with existing counters
  (3) Catching shake 2
  (4) Catching shake 3
  (5) Poke has been caught
  (6) Poke has escaped

  (7) Healed poke with potion

  (8) Switch ally pokemon
  (9) Switch hostile pokemon

    For Ally Poke
  (10) Move name statement (damage dealt in 2nd half)
  (11) Effectiveness
  (12) Critical hit
  (13) Missed hit
  (14) Status effects ### NOT IMPLEMENTED YET ###
  
  (15), (16), (17), (18), (19); Repeated above for hostile poke 

  (20) Death of either
  (21) Experience message on faint
  (22) Captured poke final message (leaves combat after this)
  (23) Continuing battle
  */
  poke_info *poke_first;
  poke_info *poke_second;
  if( (ally_poke_party[*active_ally_poke_index].speed) >= (hostile_poke_party[*active_hostile_poke_index].speed) ) {  // If you are faster than (or as fast as) the hostile, go first
    poke_first = &(ally_poke_party[*active_ally_poke_index]);
    poke_second = &(hostile_poke_party[*active_hostile_poke_index]);
  } else {
    poke_first = &(hostile_poke_party[*active_hostile_poke_index]);
    poke_second = &(ally_poke_party[*active_ally_poke_index]);
  }

  switch(*battle_outcome_stage) {
    // Use items (ally)
    case 1: {
      /*
      --> If a pokeball has been thrown
      E.G. A POKEBALL HAS BEEN THROWN!
      */
      if( (battle_item->category==1) ) {  // If an item has been used
        const uint8_t str_number = 5;
        const char *str_additions[5] = {
          "A ",
          (char*)(battle_item->name),
          " HAS ",
          "BEEN ",
          "THROWN!",
        };
        const uint8_t str_addition_lengths[5] = {
          2,
          (battle_item->name_length),
          5,
          5,
          7,
        };
        concat_strings(battle_log_complete, battle_log_stage_length, str_number, str_additions, str_addition_lengths);
      } else { *battle_log_stage_length = 0; }
      break;
    }
    case 2: {
      /*
      --> Shake 1 for pokeball catch
      E.G. ... (Shake 1)

      . Here, ally_poke_counter counts the number of shakes (where 3+ => catch)
      */
      if( (battle_item->category==1) && (hostile_poke_party[*active_hostile_poke_index].shake_state>=1) ) {
        const uint8_t str_number = 1;
        const char *str_additions[1] = {
          "...",
        };
        const uint8_t str_addition_lengths[1] = {
          3,
        };
        concat_strings(battle_log_complete, battle_log_stage_length, str_number, str_additions, str_addition_lengths);
      } else { *battle_log_stage_length = 0; }
      break;
    }
    case 3: {
      /*
      --> Shake 2 for pokeball catch
      E.G. ... (Shake 2)
      */
      if( (battle_item->category==1) && (hostile_poke_party[*active_hostile_poke_index].shake_state>=2) ) {
        const uint8_t str_number = 1;
        const char *str_additions[1] = {
          "...",
        };
        const uint8_t str_addition_lengths[1] = {
          3,
        };
        concat_strings(battle_log_complete, battle_log_stage_length, str_number, str_additions, str_addition_lengths);
      } else { *battle_log_stage_length = 0; }
      break;
    }
    case 4: {
      /*
      --> Shake 3 for pokeball catch
      E.G. ... (Shake 3)
      */
      if( (battle_item->category==1) && (hostile_poke_party[*active_hostile_poke_index].shake_state>=3) ) {
        const uint8_t str_number = 1;
        const char *str_additions[1] = {
          "...",
        };
        const uint8_t str_addition_lengths[1] = {
          3,
        };
        concat_strings(battle_log_complete, battle_log_stage_length, str_number, str_additions, str_addition_lengths);
      } else { *battle_log_stage_length = 0; }
      break;
    }
    case 5: {
      /*
      --> Caught poke in pokeball
      E.G. CHARMANDER HAS BEEN CAUGHT!
      */
      if( (battle_item->category==1) && (hostile_poke_party[*active_hostile_poke_index].shake_state>=3) ) {
        const uint8_t str_number = 4;
        const char *str_additions[4] = {
          (char*)(hostile_poke_party[*active_hostile_poke_index].name),
          " HAS ",
          "BEEN ",
          "CAUGHT!",
        };
        const uint8_t str_addition_lengths[4] = {
          (hostile_poke_party[*active_hostile_poke_index].name_length),
          5,
          5,
          7,
        };
        concat_strings(battle_log_complete, battle_log_stage_length, str_number, str_additions, str_addition_lengths);
      } else { *battle_log_stage_length = 0; }
      break;
    }
    case 6: {
      /*
      --> NOT caught poke in pokeball
      E.G. CHARMANDER ESCAPED!
      */
      if( (battle_item->category==1) && (hostile_poke_party[*active_hostile_poke_index].shake_state<3) ) {
        const uint8_t str_number = 2;
        const char *str_additions[2] = {
          (char*)(hostile_poke_party[*active_hostile_poke_index].name),
          " ESCAPED!",
        };
        const uint8_t str_addition_lengths[2] = {
          (hostile_poke_party[*active_hostile_poke_index].name_length),
          9,
        };
        concat_strings(battle_log_complete, battle_log_stage_length, str_number, str_additions, str_addition_lengths);
      } else { *battle_log_stage_length = 0; }
      break;
    }

    // Healing items
    case 7: {
      /*
      --> Potion of varying type is used
      E.G. CHARMANDER USED A POTION
      */
      if(battle_item->category==2) {
        const uint8_t str_number = 5;
        const char *str_additions[5] = {
          (char*)(ally_poke_party[*active_ally_poke_index].name),
          "USED ",
          "A ",
          (char*)(battle_item->name),
          "POTION",
        };
        const uint8_t str_addition_lengths[5] = {
          (ally_poke_party[*active_ally_poke_index].name_length),
          5,
          2,
          (battle_item->name_length),
          6,
        };
        concat_strings(battle_log_complete, battle_log_stage_length, str_number, str_additions, str_addition_lengths);
      } else { *battle_log_stage_length = 0; }
      break;
    }

    // Switching pokes (ally+hostile)
    case 8: {
      /*
      --> If swapping out an allied poke
      E.G. SWAPPING CHARMANDER WITH STARLY
      */
      if( (4 <= ally_poke_party[*active_ally_poke_index].action) && (ally_poke_party[*active_ally_poke_index].action < 10) && (battle_item->category==0) && (hostile_poke_party[*active_hostile_poke_index].shake_state < 3) ) {
        const uint8_t str_number = 4;
        const char *str_additions[4] = {
          "SWAPPING ",
          (char*)(ally_poke_party[*active_ally_poke_index].name),
          "WITH ",
          (char*)(ally_poke_party[ ally_poke_party[*active_ally_poke_index].action-4 ].name),
        };
        const uint8_t str_addition_lengths[4] = {
          9,
          (ally_poke_party[*active_ally_poke_index].name_length ),
          5,
          (ally_poke_party[  ally_poke_party[*active_ally_poke_index].action-4 ].name_length),
        };
        concat_strings(battle_log_complete, battle_log_stage_length, str_number, str_additions, str_addition_lengths);
      } else { *battle_log_stage_length = 0; }
      break;
    }
    case 9: {
      /*
      --> If swapping out a hostile poke
      E.G. SWAPPING CHARMANDER WITH STARLY
      */
      if( (4 <= hostile_poke_party[*active_hostile_poke_index].action) && (hostile_poke_party[*active_hostile_poke_index].action < 10) && (battle_item->category==0) && (hostile_poke_party[*active_hostile_poke_index].shake_state < 3) ) {
        const uint8_t str_number = 4;
        const char *str_additions[4] = {
          "SWAPPING ",
          (char*)(hostile_poke_party[*active_hostile_poke_index].name),
          "WITH ",
          (char*)(hostile_poke_party[ hostile_poke_party[*active_hostile_poke_index].action-4 ].name),
        };
        const uint8_t str_addition_lengths[4] = {
          9,
          (hostile_poke_party[*active_hostile_poke_index].name_length),
          5,
          (hostile_poke_party[ hostile_poke_party[*active_hostile_poke_index].action-4 ].name_length),
        };
        concat_strings(battle_log_complete, battle_log_stage_length, str_number, str_additions, str_addition_lengths);
      } else { *battle_log_stage_length = 0; }
      break;
    }


    // First poke
    case 10: {
      if((poke_first->health > 0) && (poke_first->action<4) && (battle_item->category==0) && (hostile_poke_party[*active_hostile_poke_index].shake_state < 3) ) {
        /*
        --> Move used by first (fastest) pokemon
        E.G. CHARMANDER USED EMBER
        */
        const uint8_t str_number = 3;
        const char *str_additions[3] = {
          (char*)(poke_first->name),
          (char*)" USED ",
          (char*)(poke_move_lookup[poke_first->moves[(poke_first->action)]].name),
        };
        const uint8_t str_addition_lengths[3] = {
          (poke_first->name_length),
          6,
          (poke_move_lookup[poke_first->moves[(poke_first->action)]].name_length),
        };
        concat_strings(battle_log_complete, battle_log_stage_length, str_number, str_additions, str_addition_lengths);
      } else { *battle_log_stage_length = 0; }
      break;
    }
    case 11: {
      /*
      --> Effectiveness of move of first poke on second poke
      E.G. IT WAS SUPER EFFECTIVE!
      */
      if((poke_first->health > 0) && (poke_first->action<4) && (battle_item->category==0) && (hostile_poke_party[*active_hostile_poke_index].shake_state < 3) ) {
        if(poke_first->is_miss != 1) {
          const uint8_t effectiveness = fetch_move_effectiveness(type_effectiveness_lookup, poke_move_lookup[poke_first->moves[(poke_first->action)]].type, poke_second->types);
          if(effectiveness == 0) {
            const uint8_t str_number = 4;
            const char *str_additions[4] = {
              "IT ",
              "HAS ",
              "NO ",
              "EFFECT...",
            };
            const uint8_t str_addition_lengths[4] = {
              3,
              4,
              3,
              9,
            };
            concat_strings(battle_log_complete, battle_log_stage_length, str_number, str_additions, str_addition_lengths);
          } else if(effectiveness == 1) {
            const uint8_t str_number = 5;
            const char *str_additions[5] = {
              "IT ",
              "IS ",
              "NOT ",
              "VERY ",
              "EFFECTIVE",
            };
            const uint8_t str_addition_lengths[5] = {
              3,
              3,
              4,
              5,
              9,
            };
            concat_strings(battle_log_complete, battle_log_stage_length, str_number, str_additions, str_addition_lengths);
          } else if(effectiveness == 2) {
            // No comment for normal effectiveness
            *battle_log_stage_length = 0;
          } else if(effectiveness == 4) {
            const uint8_t str_number = 4;
            const char *str_additions[4] = {
              "IT ",
              "IS ",
              "SUPER ",
              "EFFECTIVE!",
            };
            const uint8_t str_addition_lengths[4] = {
              3,
              3,
              6,
              10,
            };
            concat_strings(battle_log_complete, battle_log_stage_length, str_number, str_additions, str_addition_lengths);
          } else { *battle_log_stage_length = 0; }
        } else { *battle_log_stage_length = 0; }
      } else { *battle_log_stage_length = 0; }
      break;
    }
    case 12: {
      /*
      --> Was the move a critical hit
      E.G. IT WAS A CRITICAL HIT!
      */
      if((poke_first->health > 0) && (poke_first->action<4) && (battle_item->category==0) && (hostile_poke_party[*active_hostile_poke_index].shake_state < 3) ) {
        if(poke_first->is_miss != 1) {
          if(poke_first->is_critical == 1) {
            const uint8_t str_number = 5;
            const char *str_additions[5] = {
              "IT ",
              "WAS ",
              "A ",
              "CRITICAL ",
              "HIT",
            };
            const uint8_t str_addition_lengths[5] = {
              3,
              4,
              2,
              9,
              3,
            };
            concat_strings(battle_log_complete, battle_log_stage_length, str_number, str_additions, str_addition_lengths);
          } else { *battle_log_stage_length = 0; }
        } else { *battle_log_stage_length = 0; }
      } else { *battle_log_stage_length = 0; }
      break;
    }
    case 13: {
      /*
      --> If move missed
      E.G. IT MISSED!
      */
      if(poke_first->health > 0) {
        if( (poke_first->is_miss == 1) && (battle_item->category==0) && (hostile_poke_party[*active_hostile_poke_index].shake_state < 3) ) {
          const uint8_t str_number = 2;
          const char *str_additions[2] = {
            "IT ",
            "MISSED!",
          };
          const uint8_t str_addition_lengths[2] = {
            3,
            7,
          };
          concat_strings(battle_log_complete, battle_log_stage_length, str_number, str_additions, str_addition_lengths);
        } else { *battle_log_stage_length = 0; }
      } else { *battle_log_stage_length = 0; }
      break;
    }
    case 14: {
      if((poke_first->health > 0) && (poke_first->action<4) && (battle_item->category==0) && (hostile_poke_party[*active_hostile_poke_index].shake_state < 3) ) {

        // ###
        // ### NO BEHAVIOUR NEEDED HERE YET => IGNORE
        // ###    WILL LATER WANT TO ADD STATUS MESSAGES HERE E.G. POISONED, BURNED, ETC
        // ###
        *battle_log_stage_length = 0;

        // *battle_log_complete = (char*)"FIRST POKE, STG 5";
        // *battle_log_stage_length = 17;
      } else { *battle_log_stage_length = 0; }
      break;
    }


    // Second poke
    case 15: {
      if((poke_second->health > 0) && (poke_second->action<4) && (battle_item->category==0) && (hostile_poke_party[*active_hostile_poke_index].shake_state < 3) ) {
        /*
        --> Move used by second (slower) pokemon
        E.G. CHARMANDER USED EMBER
        */
        const uint8_t str_number = 3;
        const char *str_additions[3] = {
          (char*)(poke_second->name),
          (char*)" USED ",
          (char*)(poke_move_lookup[poke_second->moves[(poke_second->action)]].name),
        };
        const uint8_t str_addition_lengths[3] = {
          (poke_second->name_length),
          6,
          (poke_move_lookup[poke_second->moves[(poke_second->action)]].name_length),
        };
        concat_strings(battle_log_complete, battle_log_stage_length, str_number, str_additions, str_addition_lengths);
      } else { *battle_log_stage_length = 0; }
      break;
    }
    case 16: {
      /*
      --> Effectiveness of move of second poke on first poke
      E.G. IT WAS SUPER EFFECTIVE!
      */
      if((poke_second->health > 0) && (poke_second->action<4) && (battle_item->category==0) && (hostile_poke_party[*active_hostile_poke_index].shake_state < 3) ) {
        if(poke_second->is_miss != 1) {
          const uint8_t effectiveness = fetch_move_effectiveness(type_effectiveness_lookup, poke_move_lookup[poke_second->moves[(poke_second->action)]].type, poke_first->types);
          if(effectiveness == 0) {
            const uint8_t str_number = 4;
            const char *str_additions[4] = {
              "IT ",
              "HAS ",
              "NO ",
              "EFFECT...",
            };
            const uint8_t str_addition_lengths[4] = {
              3,
              4,
              3,
              9,
            };
            concat_strings(battle_log_complete, battle_log_stage_length, str_number, str_additions, str_addition_lengths);
          } else if(effectiveness == 1) {
            const uint8_t str_number = 5;
            const char *str_additions[5] = {
              "IT ",
              "IS ",
              "NOT ",
              "VERY ",
              "EFFECTIVE",
            };
            const uint8_t str_addition_lengths[5] = {
              3,
              3,
              4,
              5,
              9,
            };
            concat_strings(battle_log_complete, battle_log_stage_length, str_number, str_additions, str_addition_lengths);
          } else if(effectiveness == 2) {
            // No comment is made on normal effectiveness
            *battle_log_stage_length = 0;
          } else if(effectiveness == 4) {
            const uint8_t str_number = 4;
            const char *str_additions[4] = {
              "IT ",
              "IS ",
              "SUPER ",
              "EFFECTIVE!",
            };
            const uint8_t str_addition_lengths[4] = {
              3,
              3,
              6,
              10,
            };
            concat_strings(battle_log_complete, battle_log_stage_length, str_number, str_additions, str_addition_lengths);
          } else { *battle_log_stage_length = 0; }
        } else { *battle_log_stage_length = 0; }
      } else { *battle_log_stage_length = 0; }
      break;
    }
    case 17: {
      /*
      --> Was the move a critical hit
      E.G. IT WAS A CRITICAL HIT!
      */
      if((poke_second->health > 0) && (poke_second->action<4) && (hostile_poke_party[*active_hostile_poke_index].shake_state < 3) ) {
        if(poke_second->is_miss != 1) {
          if(poke_second->is_critical == 1) {
            const uint8_t str_number = 5;
            const char *str_additions[5] = {
              "IT ",
              "WAS ",
              "A ",
              "CRITICAL ",
              "HIT",
            };
            const uint8_t str_addition_lengths[5] = {
              3,
              4,
              2,
              9,
              3,
            };
            concat_strings(battle_log_complete, battle_log_stage_length, str_number, str_additions, str_addition_lengths);
          } else { *battle_log_stage_length = 0; }
        } else { *battle_log_stage_length = 0; }
      } else { *battle_log_stage_length = 0; }
      break;
    }
    case 18: {
      /*
      --> If move missed
      E.G. IT MISSED!
      */
      if((poke_second->health > 0) && (poke_second->action<4) && (hostile_poke_party[*active_hostile_poke_index].shake_state < 3) ) {
        if(poke_second->is_miss == 1) {
          const uint8_t str_number = 2;
          const char *str_additions[2] = {
            "IT ",
            "MISSED!",
          };
          const uint8_t str_addition_lengths[2] = {
            3,
            7,
          };
          concat_strings(battle_log_complete, battle_log_stage_length, str_number, str_additions, str_addition_lengths);
        } else { *battle_log_stage_length = 0; }
      } else { *battle_log_stage_length = 0; }
      break;
    }
    case 19: {
      if((poke_second->health > 0) && (poke_second->action<4) && (hostile_poke_party[*active_hostile_poke_index].shake_state < 3) ) {

        // ###
        // ### NO BEHAVIOUR NEEDED HERE YET => IGNORE
        // ###    WILL LATER WANT TO ADD STATUS MESSAGES HERE E.G. POISONED, BURNED, ETC
        // ###
        *battle_log_stage_length = 0;

        // *battle_log_complete = (char*)"SECOND POKE, STG 10";
        // *battle_log_stage_length = 19;
      } else { *battle_log_stage_length = 0; }
      break;
    }

    // When either poke is dead, new triggers
    case 20: {  // Death message
      *battle_log_stage_length = 0;   // Default to not occur
      if(poke_first->health <= 0) {
        const uint8_t str_number = 2;
        const char *str_additions[2] = {
          (char*)(poke_first->name),
          " FAINTED",
        };
        const uint8_t str_addition_lengths[2] = {
          (poke_first->name_length),
          8,
        };
        concat_strings(battle_log_complete, battle_log_stage_length, str_number, str_additions, str_addition_lengths);
      } else if(poke_second->health <= 0) {
        const uint8_t str_number = 2;
        const char *str_additions[2] = {
          (char*)(poke_second->name),
          " FAINTED",
        };
        const uint8_t str_addition_lengths[2] = {
          (poke_second->name_length),
          8,
        };
        concat_strings(battle_log_complete, battle_log_stage_length, str_number, str_additions, str_addition_lengths);
      } else { *battle_log_stage_length = 0; }
      break;
    }
    case 21: {  // Experience message
      *battle_log_stage_length = 0;   // Default to not occur
      if(poke_first->health <= 0) {
        const uint8_t str_number = 3;
        const char *str_additions[3] = {
          (char*)(poke_second->name),
          " GAINED ",
          "EXPERIENCE"
        };
        const uint8_t str_addition_lengths[3] = {
          (poke_second->name_length),
          8,
          10,
        };
        concat_strings(battle_log_complete, battle_log_stage_length, str_number, str_additions, str_addition_lengths);
      } else if(poke_second->health <= 0) {
        const uint8_t str_number = 3;
        const char *str_additions[3] = {
          (char*)(poke_first->name),
          " GAINED ",
          "EXPERIENCE"
        };
        const uint8_t str_addition_lengths[3] = {
          (poke_first->name_length),
          8,
          10,
        };
        concat_strings(battle_log_complete, battle_log_stage_length, str_number, str_additions, str_addition_lengths);
      } else { *battle_log_stage_length = 0; }
      break;
    }
    case 22: {  // Captured poke final message
      if( (hostile_poke_party[*active_hostile_poke_index].shake_state >= 3) ) {
        const uint8_t str_number = 2;
        const char *str_additions[2] = {
          "POKE ",
          "CAPTURED!"
        };
        const uint8_t str_addition_lengths[2] = {
          5,
          9,
        };
        concat_strings(battle_log_complete, battle_log_stage_length, str_number, str_additions, str_addition_lengths);
      } else { *battle_log_stage_length = 0; }
      break;
    }
    case 23: {  // Post-death trigger

      const uint8_t str_number = 2;
      const char *str_additions[2] = {
        "CONTINUING ",
        "BATTLE"
      };
      const uint8_t str_addition_lengths[2] = {
        11,
        6,
      };
      concat_strings(battle_log_complete, battle_log_stage_length, str_number, str_additions, str_addition_lengths);
      break;
    }
  }
}

uint8_t fetch_party_fainted_count() {
  /*
  . Returns the number of fainted members of the poke party given (e.g. collection of 6 pokemon/nulls)
  . This is used for black out checks
  */
  uint8_t fainted_total = 0;
  // for(uint8_t i=0; i<6; i++) {
  //   if(poke_party[i]->health <= 0) {
  //     fainted_total++;
  //   }
  // }
  return fainted_total;
}

char* fetch_battle_screen_battle_log_subset(char *battle_log_complete, uint8_t *battle_log_stage_length, uint8_t *battle_log_stage, uint8_t *battle_log_timer) {
  char *battle_log_subset = (char*)("");
  uint8_t subset_end = (uint8_t)( ((uint16_t)(*battle_log_stage_length)*(*battle_log_timer))>>7 ); // Note**; >>7 => ratio of 128 (half time), >>8 for 256 ratio (full time)
  for(uint8_t i=0; i<subset_end; i++) {
    battle_log_subset[i] = battle_log_complete[i];
  }
  battle_log_subset[subset_end+1] = '\0';
  return battle_log_subset;
}


void display_battle_screen_battle_log(char *battle_log_complete, uint8_t *battle_log_stage_length, uint8_t *battle_outcome_stage, uint8_t *battle_outcome_timer, const uint8_t battle_log_extension, const uint8_t battle_options_hheight) {
  /*
  . Displays the battle log, which describes events occurring in the battle in a text box
  */
  //display_rect(-128+2*battle_options_hheight, -128+battle_log_hwidth, 2*battle_options_hheight, battle_log_hwidth);

  char *battle_text = fetch_battle_screen_battle_log_subset(battle_log_complete, battle_log_stage_length, battle_outcome_stage, battle_outcome_timer);
  uint8_t buffer = 2; // Give some space at the left/bottom edge of the battle log
  int8_t vector_lines[12] = {
    0, 120,
    0, battle_log_extension,
    -4*battle_options_hheight, 0,
    0, -battle_log_extension,
    0, -120,
    4*battle_options_hheight, 0,
  };
  reset_beam();
  set_scale(128);
  // move(0,0);
  move(-128+4*battle_options_hheight+buffer, -128+buffer);
  lines(6, vector_lines);

  reset_beam();
  set_text_size(-battle_options_hheight>>1, 4*battle_options_hheight);
  print_str_c(-128+4*battle_options_hheight -(battle_options_hheight>>1)+buffer, -128+buffer, battle_text);
  set_scale(128);
};


void fetch_battle_screen_counters(uint8_t *battle_outcome_stage, const poke_move *poke_move_lookup, poke_info ally_poke_party[6], uint8_t *active_ally_poke_index, poke_info hostile_poke_party[6], uint8_t *active_hostile_poke_index, int8_t *ally_poke_counter, int8_t *hostile_poke_counter, uint8_t *poke_counter_type, const uint8_t type_effectiveness_lookup[324], uint8_t *battle_item_index) {
  /*
  . Returns the counter type and values for counters required on this stage of the battle animation
  . This includes damage, experience, etc recieved by both parties

  poke_counter_type;
  0 = No counter
  1 = HP change
  2 = EXP change
  ... (stats in future)

  . Ally/hostile counter values may show a positive or negative change
  */
  uint8_t is_ally_fainting = 0;
  poke_info *poke_first;
  poke_info *poke_second;
  int8_t *poke_first_counter;
  int8_t *poke_second_counter;
  if( ally_poke_party[*active_ally_poke_index].speed >= hostile_poke_party[*active_hostile_poke_index].speed ) {
    poke_first = &(ally_poke_party[*active_ally_poke_index]);
    poke_second = &(hostile_poke_party[*active_hostile_poke_index]);
    poke_first_counter = ally_poke_counter;
    poke_second_counter = hostile_poke_counter;
  } else {
    poke_first = &(hostile_poke_party[*active_hostile_poke_index]);
    poke_second = &(ally_poke_party[*active_ally_poke_index]);
    poke_first_counter = hostile_poke_counter;
    poke_second_counter = ally_poke_counter;
  }

  switch(*battle_outcome_stage) {
    // ### LEGACY -> HANDLED THROUGH SHAKE STATE DIRECTLY NOW NOT USING COUNTERS ###
    // **NOTE; This capture check only occurs for PLAYER capturing the HOSTILE; never vice versa -> May want to generalise in future
    // case 2:   // Shake 1, roll for pokeball escape
    //   *ally_poke_counter = hostile_poke_party[*active_hostile_poke_index].shake_state;
    //   break;
    // case 3:   // Shake 2, roll for pokeball escape
    //   *ally_poke_counter = hostile_poke_party[*active_hostile_poke_index].shake_state;
    //   break;
    // case 4:   // Shake 3, roll for pokeball escape
    //   *ally_poke_counter = hostile_poke_party[*active_hostile_poke_index].shake_state;
    //   break;
    // case 5:   // Capture
    //   *ally_poke_counter = hostile_poke_party[*active_hostile_poke_index].shake_state;
    //   break;
    // case 6:   // Escape
    //   *ally_poke_counter = hostile_poke_party[*active_hostile_poke_index].shake_state;
    //   break;

    case 10:   // HP (first poke move reaction)
      if(poke_first->is_miss != 1) {
        fetch_poke_battle_damage(1, poke_move_lookup, poke_first, poke_second, poke_first_counter, poke_second_counter, poke_counter_type, type_effectiveness_lookup);
      }
      break;
    case 15:   // HP (second poke move reaction)
      if(poke_second->is_miss != 1) {
        fetch_poke_battle_damage(0, poke_move_lookup, poke_first, poke_second, poke_first_counter, poke_second_counter, poke_counter_type, type_effectiveness_lookup);
      }
      break;
    case 21:   // EXP
      if(ally_poke_party[*active_ally_poke_index].health <= 0) { is_ally_fainting = 1; }
      fetch_poke_battle_experience(is_ally_fainting, &(ally_poke_party[*active_ally_poke_index]), &(hostile_poke_party[*active_hostile_poke_index]), ally_poke_counter, hostile_poke_counter, poke_counter_type);
      break;
    default:     // Set to nulls
      *poke_counter_type = 0;
      *ally_poke_counter = 0;
      *hostile_poke_counter = 0;
      break;
  }
}
void fetch_poke_battle_experience(uint8_t is_A_fainting, poke_info *poke_A, poke_info *poke_B, int8_t *poke_A_counter, int8_t *poke_B_counter, uint8_t *poke_counter_type) {
  /*
  . Consider who is fainting currently
  . Calculate how much experience is to be gained
  . Set buffer counter values
  */
  if(is_A_fainting == 1) {
    uint8_t experience = poke_A->level*5;
    *poke_A_counter = 0;
    *poke_B_counter = experience;
    *poke_counter_type = 2;
  } else {
    uint8_t experience = poke_B->level*5;
    *poke_A_counter = experience;
    *poke_B_counter = 0;
    *poke_counter_type = 2;
  }
}
void fetch_poke_battle_damage(uint8_t is_A_attacking, const poke_move *poke_move_lookup, poke_info *poke_A, poke_info *poke_B, int8_t *poke_A_counter, int8_t *poke_B_counter, uint8_t *poke_counter_type, const uint8_t type_effectiveness_lookup[324]) {
  /*
  . Consider who is attacking currently
  . Calculate how much damage is to be taken
  . Set buffer counter values
  */
  if(is_A_attacking == 1) {
    uint8_t effectiveness = fetch_move_effectiveness(type_effectiveness_lookup, poke_move_lookup[ poke_A->moves[poke_A->action] ].type, poke_B->types );
    uint8_t STAB = fetch_move_stab(poke_move_lookup[ poke_A->moves[poke_A->action] ].type, poke_A->types);
    uint8_t damage = poke_move_lookup[ poke_A->moves[poke_A->action] ].attack;
      // (
      //   (poke_move_lookup[ poke_A->moves[poke_A->action] ].attack)*(1- (poke_B->defence/100)) +
      //   (poke_move_lookup[ poke_A->moves[poke_A->action] ].special_attack)*(1- (poke_B->special_defence/100))
      // )*(STAB)*(effectiveness/2);

    *poke_A_counter = 0;
    *poke_B_counter = -damage;
    *poke_counter_type = 1;
  } else {
    uint8_t effectiveness = fetch_move_effectiveness(type_effectiveness_lookup, poke_move_lookup[ poke_B->moves[poke_B->action] ].type, poke_A->types );
    uint8_t STAB = fetch_move_stab(poke_move_lookup[ poke_B->moves[poke_B->action] ].type, poke_B->types);
    uint8_t damage = poke_move_lookup[ poke_B->moves[poke_B->action] ].attack;
      // (
      //   (poke_move_lookup[ poke_B->moves[poke_B->action] ].attack)*(1- (poke_A->defence/100)) +
      //   (poke_move_lookup[ poke_B->moves[poke_B->action] ].special_attack)*(1- (poke_A->special_defence/100))
      // )*(STAB)*(effectiveness/2);
    *poke_A_counter = -damage;
    *poke_B_counter = 0;
    *poke_counter_type = 1;
  }
}
uint8_t fetch_move_stab(uint8_t move_type, const uint8_t poke_types[2]) {
  return 0;//( (move_type==poke_types[0])||(move_type==poke_types[1]) ) ? 1 : 0;
}
void set_battle_start_parameters(const poke_item poke_item_lookup[], const uint8_t type_effectiveness_lookup[324], uint8_t *battle_item_index, char *battle_log_complete, uint8_t *battle_log_stage_length, const poke_move *poke_move_lookup, poke_info ally_poke_party[6], uint8_t *active_ally_poke_index, poke_info hostile_poke_party[6], uint8_t *active_hostile_poke_index, uint8_t *hovered_battle_option, uint8_t *battle_screen_variant, uint8_t *battle_mode, uint8_t *battle_outcome_stage, uint8_t *battle_outcome_timer, int8_t *ally_poke_counter, int8_t *hostile_poke_counter, uint8_t *poke_counter_type) {
  /*
  . Set required info for a battle combat round to start
  . This is triggered one time, once the player makes a move selection

  . Sets pokemon info as well as game state info
  */
  poke_info *poke_ally = &(ally_poke_party[*active_ally_poke_index]);
  poke_info *poke_hostile = &(hostile_poke_party[*active_hostile_poke_index]);
  // Queue up move for ally
  poke_ally->action = *hovered_battle_option;
  if(rand() < 1638) { poke_ally->is_critical = 1; } else { poke_ally->is_critical = 0; }  //~5% chance
  if(rand() > 128*(uint16_t)poke_move_lookup[ poke_ally->moves[poke_ally->action] ].probability) { poke_ally->is_miss = 1; } else { poke_ally->is_miss = 0; }

  // Queue up move for hostile
  poke_hostile->action = select_random_poke_move(poke_hostile->moves);
  if(rand() < 1638) { poke_hostile->is_critical = 1; } else { poke_hostile->is_critical = 0; }  //~5% chance
  if(rand() > 128*(uint16_t)poke_move_lookup[ poke_hostile->moves[poke_hostile->action] ].probability) { poke_hostile->is_miss = 1; } else { poke_hostile->is_miss = 0; }
  
  // Queue up *hovered_battle_option index move IF IS VALID
  *battle_screen_variant = 2;   // So the options are removed (since 1 is the max)
  *battle_outcome_stage = 1;
  *battle_outcome_timer = 0;
  fetch_battle_screen_battle_log(type_effectiveness_lookup, battle_log_complete, battle_log_stage_length, poke_move_lookup, ally_poke_party, active_ally_poke_index, hostile_poke_party, active_hostile_poke_index, ally_poke_counter, hostile_poke_counter, &(poke_item_lookup[*battle_item_index]), battle_outcome_stage, battle_outcome_timer);
  fetch_battle_screen_counters(battle_outcome_stage, poke_move_lookup, ally_poke_party, active_ally_poke_index, hostile_poke_party, active_hostile_poke_index, ally_poke_counter, hostile_poke_counter, poke_counter_type, type_effectiveness_lookup, battle_item_index);
}
uint8_t select_random_poke_move(uint8_t moves[4]) {
  uint8_t valid_moves[4] = {9,9,9,9};   // Changes to the index of valid moves as they are found e.g. {0,2,3,9}, 9 means no valid move has been found (only 0-3 valid)
  int8_t valid_moves_number = 0;
  for(uint8_t i=0; i<4; i++) {
    // ###
    // ### ADD A PP CHECK IN THE FUTURE TOO
    // ###
    if(moves[i] != 0) {
      valid_moves[valid_moves_number] = i;
      valid_moves_number = valid_moves_number+1;
    }
  }
  uint8_t chosen_move_index = 0;
  uint8_t random_index = (uint8_t)(((uint16_t)valid_moves_number*rand())>>15); // rand()>>15 to get (0, 1) multiplier, *valid moves to select int in range (0, valid_number) to pick out a valid index
  if(valid_moves[random_index] < 4) {   // e.g. not 9 hence not default
    chosen_move_index = valid_moves[random_index];
  }
  return chosen_move_index;
}


void calculate_battle_screen(const poke_item poke_item_lookup[], const uint8_t type_effectiveness_lookup[324], char *battle_log_complete, uint8_t *battle_log_stage_length, const poke_move *poke_move_lookup, poke_info ally_poke_party[6], uint8_t *active_ally_poke_index, poke_info hostile_poke_party[6], uint8_t *active_hostile_poke_index, uint8_t *battle_item_index, uint8_t *hovered_battle_option, uint8_t *battle_screen_variant, uint8_t *battle_mode, uint8_t *battle_outcome_stage, uint8_t *battle_outcome_timer, int8_t *ally_poke_counter, int8_t *hostile_poke_counter, uint8_t *poke_counter_type) {
  if(*battle_outcome_stage > 0) {     // If beyond stage 0 (e.g. in automatic animation), progress the stages periodically
    // ** NOTE; Set the text speed here through 'timer_speed'
    calculate_battle_screen_battle_timing(4, battle_log_complete, battle_log_stage_length, poke_move_lookup, ally_poke_party, active_ally_poke_index, hostile_poke_party, active_hostile_poke_index, battle_item_index, hovered_battle_option, battle_mode, battle_screen_variant, battle_outcome_stage, battle_outcome_timer, ally_poke_counter, hostile_poke_counter, poke_counter_type, poke_item_lookup, type_effectiveness_lookup);
  } else if(*battle_outcome_stage == 0) {
    calculate_battle_screen_buttons(poke_item_lookup, type_effectiveness_lookup, battle_item_index, battle_log_complete, battle_log_stage_length, poke_move_lookup, ally_poke_party, active_ally_poke_index, hostile_poke_party, active_hostile_poke_index, hovered_battle_option, battle_screen_variant, battle_mode, battle_outcome_stage, battle_outcome_timer, ally_poke_counter, hostile_poke_counter, poke_counter_type);
  }   // ### COULD ADD A SKIP TEXT BUTTON ###
}

void calculate_battle_screen_battle_timing(const uint8_t timer_speed, char *battle_log_complete, uint8_t *battle_log_stage_length, const poke_move *poke_move_lookup, poke_info ally_poke_party[6], uint8_t *active_ally_poke_index, poke_info hostile_poke_party[6], uint8_t *active_hostile_poke_index, uint8_t *battle_item_index, uint8_t *hovered_battle_option, uint8_t *battle_mode, uint8_t *battle_screen_variant, uint8_t *battle_outcome_stage, uint8_t *battle_outcome_timer, int8_t *ally_poke_counter, int8_t *hostile_poke_counter, uint8_t *poke_counter_type, const poke_item poke_item_lookup[], const uint8_t type_effectiveness_lookup[324]) {
  /*
  . Calculates the battle log to be shown and any changes in state while the battle animation is playing
  . This includes damage taken, experience gained, end of combat, etc while fight takes place
  */
  // Progress the battle stages
  if( (*battle_outcome_timer < 255) && (*battle_log_stage_length!=0) ) { // If timer has not finished, continue it
    if(*battle_outcome_timer+timer_speed < *battle_outcome_timer) { // If overflows, prevent this to move onto next stage -> not an issue when timer_speed=1
      *battle_outcome_timer = 255;
    } else {
      *battle_outcome_timer = *battle_outcome_timer+timer_speed;  // *battle_outcome_timer+1
    }
  } else {                          // If timer has finished, change stage and reset it
    // One-time calcs as new stage begins
    *battle_outcome_stage = *battle_outcome_stage+1;
    *battle_outcome_timer = 0;
    fetch_battle_screen_battle_log(type_effectiveness_lookup, battle_log_complete, battle_log_stage_length, poke_move_lookup, ally_poke_party, active_ally_poke_index, hostile_poke_party, active_hostile_poke_index, ally_poke_counter, hostile_poke_counter, &(poke_item_lookup[*battle_item_index]), battle_outcome_stage, battle_outcome_timer);
    fetch_battle_screen_counters(battle_outcome_stage, poke_move_lookup, ally_poke_party, active_ally_poke_index, hostile_poke_party, active_hostile_poke_index, ally_poke_counter, hostile_poke_counter, poke_counter_type, type_effectiveness_lookup, battle_item_index);
  }

  // Reset battle stages/timers once finished showing
  if(*battle_outcome_stage > 23) {
    *battle_outcome_stage = 0;    // <-- This officially ends the animation
    *battle_outcome_timer = 0;
    *battle_screen_variant = 0;   // <-- This ensures animation is not shown to the player (controls the hiding of the options)
  }

  // Apply effects in the 2nd half of stages
  poke_info *poke_ally = &(ally_poke_party[*active_ally_poke_index]);
  poke_info *poke_hostile = &(hostile_poke_party[*active_hostile_poke_index]);
  if(*battle_outcome_timer>128) {
    // ### LEGACY ###
    // int8_t ally_counter_step    = (uint8_t)(*ally_poke_counter /(256 -(uint16_t)*battle_outcome_timer)) +ally_counter_step_offset;         // The increment change each frame
    // int8_t hostile_counter_step = (uint8_t)(*hostile_poke_counter /(256 -(uint16_t)*battle_outcome_timer)) +hostile_counter_step_offset;   // "" ""
    
    // Min of 1 tick per frame, overwise scale with amount of counters to be applied (which is then limited to 0 later)
    int8_t ally_counter_step_offset    = (*ally_poke_counter > 0) ? 1 : -1;
    int8_t hostile_counter_step_offset = (*hostile_poke_counter > 0) ? 1 : -1;
    int8_t ally_counter_step    = (int8_t)( (int16_t)(*ally_poke_counter)*timer_speed/128 ) +ally_counter_step_offset;
    int8_t hostile_counter_step = (int8_t)( (int16_t)(*ally_poke_counter)*timer_speed/128 ) +hostile_counter_step_offset;

    switch(*poke_counter_type) {
      case 1:   // HP
        // Ensure the step is bounded correctly
        // Ally below/above
        if( (int16_t)(poke_ally->health) +ally_counter_step < 0 ) { ally_counter_step = -poke_ally->health; }
        else if( (int16_t)(poke_ally->health) +ally_counter_step > poke_ally->health_max) { ally_counter_step = poke_ally->health_max -poke_ally->health; }
        // Hostile below/above
        if( (int16_t)(poke_hostile->health) +hostile_counter_step < 0 ) { hostile_counter_step = -poke_hostile->health; }
        else if( (int16_t)(poke_hostile->health) +hostile_counter_step > poke_hostile->health_max-1) { hostile_counter_step = poke_hostile->health_max-1 -poke_hostile->health; }

        poke_ally->health = poke_ally->health +ally_counter_step;
        poke_hostile->health = poke_hostile->health +hostile_counter_step;
        break;
      case 2:   // EXP
        // Ensure the step is bounded correctly
        // Ally below/above
        if( poke_ally->experience +ally_counter_step < 0 ) { ally_counter_step = -poke_ally->experience; }
        if( poke_ally->experience +ally_counter_step > poke_ally->experience_max-1) { ally_counter_step = poke_ally->experience_max-1 -poke_ally->experience; }
        poke_ally->experience = poke_ally->experience +ally_counter_step;

        // Handle ally level up
        if(poke_ally->experience >= poke_ally->experience_max-1) {  // If you have enough experience to level up
          poke_ally->level = poke_ally->level+1;                                                      // Level up poke
          poke_ally->attack = poke_ally->attack +(poke_ally->attack)>>3;                              // Increase stats
          poke_ally->special_attack = poke_ally->special_attack +2;                                   //
          poke_ally->defence = poke_ally->defence +2;                                                 //
          poke_ally->special_defence = poke_ally->special_defence +2;                                 //
          poke_ally->health = poke_ally->health +5;                                                   // **NOTE; Health and MaxHealth increased together -> optional choice
          poke_ally->health_max = poke_ally->health_max +5;                                           //
          poke_ally->experience_max = poke_ally->experience_max +2;                                   //
          poke_ally->experience = 0;                                                                  // Reset experience
        }

        // poke_hostile->experience = poke_hostile->experience +hostile_counter_step;   // ** Note; For now, only ally experience matters
        break;
    }
    *ally_poke_counter = *ally_poke_counter-ally_counter_step;
    *hostile_poke_counter = *hostile_poke_counter-hostile_counter_step;
  }

  // Apply separate fixed events
  if(*battle_outcome_timer >= 250) {  // E.G trigger at the end of stage that is being shown
    switch(*battle_outcome_stage) {
      case 8: // Switching ally poke
        *active_ally_poke_index = ally_poke_party[*active_ally_poke_index].action-4;    // Set the active poke to the one being switched out
        ally_poke_party[*active_ally_poke_index].action = *active_ally_poke_index+4;    // Make the new poke have no active action (so it won't immediately start performing attacks once switched in -> this is done by defaulting to switching to itself, hence not in [0,4) attack range and past its own switch trigger)
        break;
      case 9: // switching hostile poke
        *active_hostile_poke_index = hostile_poke_party[*active_hostile_poke_index].action-4; // "" ""
        hostile_poke_party[*active_hostile_poke_index].action = *active_hostile_poke_index+4; // "" ""
        break;
      case 22:  // When a poke is captured, once stage ends also end the battle
        /*
        . Add poke to party (OR box if party is full)
        . GO back to roam screen
        */
        capture_pokemon(ally_poke_party, hostile_poke_party[*active_hostile_poke_index]);
        switch_to_roam(battle_log_stage_length, battle_outcome_stage, battle_outcome_timer, hovered_battle_option, battle_screen_variant, battle_mode);
        break;
      case 23:
        if(poke_hostile->health <= 0) {   // When the hostile dies, will always result in a successful battle + exit
          // ###
          // ### NOTE; THIS WILL NEED CHANGING ONCE NON-WILD BATTLES INTRODUCED
          // ###
          switch_to_roam(battle_log_stage_length, battle_outcome_stage, battle_outcome_timer, hovered_battle_option, battle_screen_variant, battle_mode);
        }
        if(poke_ally->health <= 0) {   // When the your poke dies, will either (1) require a switch or (2) end in black out
          uint8_t fainted_allies = fetch_party_fainted_count(); // ### NEED TO INCLUDE PARTY IN THIS FUNC ###
          if(fainted_allies >= 6) { // If all pokes are fainted, then black out
            switch_to_roam(battle_log_stage_length, battle_outcome_stage, battle_outcome_timer, hovered_battle_option, battle_screen_variant, battle_mode);
          } else {                  // If any pokes remain, allow option to switch
            // switch_to_pokeswitch();
            // ###
            // ### USE VARIANTS TO CONTROL WHETHER USER MUST OR CAN SWITCH (DEAD POKE vs SELECTED SWITCH MANUALLY)
            // ###
          }
        }
        break;
    }
  }
}

void calculate_battle_screen_buttons(const poke_item poke_item_lookup[], const uint8_t type_effectiveness_lookup[324], uint8_t *battle_item_index, char *battle_log_complete, uint8_t *battle_log_stage_length, const poke_move *poke_move_lookup, poke_info ally_poke_party[6], uint8_t *active_ally_poke_index, poke_info hostile_poke_party[6], uint8_t *active_hostile_poke_index, uint8_t *hovered_battle_option, uint8_t *battle_screen_variant, uint8_t *battle_mode, uint8_t *battle_outcome_stage, uint8_t *battle_outcome_timer, int8_t *ally_poke_counter, int8_t *hostile_poke_counter, uint8_t *poke_counter_type) {
  /*
  . Button interactions with the battle_screen
  . Currently only takes button input while battle_outcome_stage==0, e.g. When animation for outcome is NOT playing
  */
  uint8_t buttons = read_buttons();
  if(buttons & JOY1_BTN1_MASK) {  // Cycle selected battle option
    *hovered_battle_option = (*hovered_battle_option+1)%4;
    reset_beam();
    print_str_c(0, 0, (char*)"PRESSED 1");
    set_scale(128);
  }

  if( (buttons & JOY1_BTN2_MASK) && (*battle_mode==0) ) {  // Select button when hovering fight option
    if(*battle_screen_variant == 0) {   // If in SelectOptions variant
      switch(*hovered_battle_option) {  // Change mode
        case 0:
          *battle_screen_variant = 1;
          break;
        case 1:
          *battle_mode = 1;
          break;
        case 2:
          *battle_mode = 2;
          break;
        case 3:
          *battle_mode = 3;
          break;
      }
    } else if(*battle_screen_variant == 1) {  // If in MoveOptions variant
      if( poke_move_lookup[ ally_poke_party[*active_ally_poke_index].moves[*hovered_battle_option] ].probability > 0 ) {  // If is a valid move, allow it to be queued
        set_battle_start_parameters(poke_item_lookup, type_effectiveness_lookup, battle_item_index, battle_log_complete, battle_log_stage_length, poke_move_lookup, ally_poke_party, active_ally_poke_index, hostile_poke_party, active_hostile_poke_index, hovered_battle_option, battle_screen_variant, battle_mode, battle_outcome_stage, battle_outcome_timer, ally_poke_counter, hostile_poke_counter, poke_counter_type);
      } else {  // If the move is invalid, make a battle log message visable
        //pass
      }
    }
  
    *hovered_battle_option = 0;               // Reset index
    reset_beam();
    print_str_c(0, 0, (char*)"PRESSED 2 BASE");
    set_scale(128);
  }

  if( (buttons & JOY1_BTN3_MASK) && (*battle_mode==1) ) {  // Back
    *battle_mode = 0;
    reset_beam();
    print_str_c(0, 0, (char*)"PRESSED 3");
    set_scale(128);
  }
}

//-----------------------
//-- POKESWITCH_SCREEN --
//-----------------------

void display_battle_screen_pokeswitch_screen(poke_info ally_poke_party[6], uint8_t *active_ally_poke_index, uint8_t hovered_pokeswitch, uint8_t timer) {
  /*
  . Displays the screen to switch pokemon mid battle
  . This screen can only appear when in battle; a different screen will be used when viewing pokemon outside of battle
  */
  // ** Note; At least 2x3 (x X y) must be shown, hence switchbox_hwidth in [0, 42-delta], and switchbox_hheight in [0, 64-delta]
  uint8_t switchbox_hheight = 20;
  uint8_t switchbox_hwidth = 60;
  uint8_t title_buffer = 20;      // Space given for title to be shown
  
  int8_t vector_lines[46] = {
    // Outer edge + center line
    0, 2*switchbox_hwidth, 
    0, 2*switchbox_hwidth, 
    -2*switchbox_hheight, 0,
    -2*switchbox_hheight, 0,
    -2*switchbox_hheight, 0,
    0, -2*switchbox_hwidth,
    0, -2*switchbox_hwidth,
    2*switchbox_hheight, 0,
    2*switchbox_hheight, 0,
    2*switchbox_hheight, 0,
    0, 2*switchbox_hwidth, 
    -2*switchbox_hheight, 0,
    -2*switchbox_hheight, 0,
    -2*switchbox_hheight, 0,

    // Horizontal lines
    2*switchbox_hheight, 0,
    0, 2*switchbox_hwidth,
    0, -2*switchbox_hwidth,
    0, -2*switchbox_hwidth,
    0, 2*switchbox_hwidth,

    2*switchbox_hheight, 0,
    0, 2*switchbox_hwidth,
    0, -2*switchbox_hwidth,
    0, -2*switchbox_hwidth, 
  };

  // Draw poke switchboxes frame
  set_scale(128);
  move(128-title_buffer, -128+(64-switchbox_hwidth));
  lines(23, vector_lines);

  // Draw switchbox contents
  // (1) Small sprite, (2) Health, (3) Name
  for(uint8_t j=0; j<3; j++) {
    for(uint8_t i=0; i<2; i++) {
      // ### GET POKE DETAILS FROM Ith INDEX ###
      uint8_t pokeswitch_index = i +2*j;  // Which index in the list the considered pokemon is at
      display_battle_screen_pokeswitch_poke_details(&(ally_poke_party[i+2*j]), 128-title_buffer-(switchbox_hheight>>2) -j*2*switchbox_hheight, -128+(64-switchbox_hwidth) +i*2*switchbox_hwidth, switchbox_hheight, switchbox_hwidth);
      display_cube(128-title_buffer-switchbox_hheight -j*2*switchbox_hheight, -64 +(switchbox_hwidth>>1) +i*2*switchbox_hwidth, switchbox_hheight>>1);
      if(hovered_pokeswitch == pokeswitch_index) {
        display_hovered_star(128-title_buffer -j*2*switchbox_hheight -10, -128+(64-switchbox_hwidth) +2*switchbox_hwidth +i*2*switchbox_hwidth -10, 2, timer);
      }
    }
  }

  reset_beam();
  set_scale(128);
  set_text_size(-5, 40);
  // Draw title
  print_str_c(128 -(title_buffer>>2), -40, (char*)"SWITCH POKEMON");
  // Draw hotkey instructions below
  print_str_c(-64, -128, (char*)"BUTTON 1 = SELECT POKEMON");
  print_str_c(-64 -title_buffer, -128, (char*)"BUTTON 2 = CYLE POKEMON");
  print_str_c(-64 -2*title_buffer, -128, (char*)"BUTTON 3 = SUMMARY");
  print_str_c(-64 -3*title_buffer, -128, (char*)"BUTTON 4 = BACK");
  
  set_scale(128);
}

void display_battle_screen_pokeswitch_poke_details(poke_info *poke, int8_t origin_y, int8_t origin_x, uint8_t switchbox_hheight, uint8_t switchbox_hwidth) {
  /*
  . Displays the name, health and sprite for a pokemon in a small box for the poke-switch screen
  */
  if(poke->name_length > 0) {
    char hp_text_buffer[11] = {'H','P',':','0','0','0','/','0','0','0','\0'};
    hp_text_buffer[3] = poke->health/100 +'0';   // Current HP
    hp_text_buffer[4] = poke->health/10 +'0';    //
    hp_text_buffer[5] = poke->health % 10 +'0';  //
    hp_text_buffer[7] = poke->health_max/100 +'0';   // Max HP
    hp_text_buffer[8] = poke->health_max/10 +'0';    //
    hp_text_buffer[9] = poke->health_max % 10 +'0';  //

    reset_beam();
    set_scale(128);
    set_text_size(-5, 40);  // ### MAKE THIS SCALE WITH BOX DIMS ###
    print_str_c(origin_y, origin_x, (char*)(poke->name));
    print_str_c(origin_y -(switchbox_hheight>>1) -(switchbox_hheight>>2), origin_x, (char*)(hp_text_buffer));
  }
}

void calculate_battle_screen_pokeswitch(uint8_t *hovered_pokeswitch, const poke_move *poke_move_lookup, const poke_item poke_item_lookup[], const uint8_t type_effectiveness_lookup[324], int8_t *ally_poke_counter, int8_t *hostile_poke_counter, uint8_t *poke_counter_type, poke_info ally_poke_party[6], uint8_t *active_ally_poke_index, poke_info hostile_poke_party[6], uint8_t *active_hostile_poke_index, uint8_t *battle_item_index, char *battle_log_complete, uint8_t *battle_log_stage_length, uint8_t *battle_mode, uint8_t *battle_screen_variant, uint8_t *battle_outcome_stage, uint8_t *battle_outcome_timer) {
  uint8_t buttons = read_buttons();
  if(buttons & JOY1_BTN1_MASK) {    // Cycle pokemon
    *hovered_pokeswitch = (*hovered_pokeswitch+1)%6;
    reset_beam();
    print_str_c(0, 0, (char*)"PRESSED 1");
    set_scale(128);
  }
  if(buttons & JOY1_BTN2_MASK) {    // Select pokemon
    if( fetch_pokeswitch_valid(&(ally_poke_party[*hovered_pokeswitch])) > 0 ) {  // If you are hovering a valid poke
      ally_poke_party[*active_ally_poke_index].action = 4+*hovered_pokeswitch;  // Set the active pokemon to switch with hovered pokemon
      // ###
      // ### COMBINE INTO FUNC WITH SET_BATTLE_PARAMS
      // ###    ONE FUNC FOR GEN WILD POKE, ONE FOR SET PARAMETER START
      // ###
      *battle_screen_variant = 2;   // So the options are removed (since 1 is the max)
      *battle_outcome_stage = 1;
      *battle_outcome_timer = 0;
      fetch_battle_screen_battle_log(type_effectiveness_lookup, battle_log_complete, battle_log_stage_length, poke_move_lookup, ally_poke_party, active_ally_poke_index, hostile_poke_party, active_hostile_poke_index, ally_poke_counter, hostile_poke_counter, &(poke_item_lookup[*battle_item_index]), battle_outcome_stage, battle_outcome_timer);
      fetch_battle_screen_counters(battle_outcome_stage, poke_move_lookup, ally_poke_party, active_ally_poke_index, hostile_poke_party, active_hostile_poke_index, ally_poke_counter, hostile_poke_counter, poke_counter_type, type_effectiveness_lookup, battle_item_index);
    }

    reset_beam();
    print_str_c(0, 0, (char*)"PRESSED 2");
    set_scale(128);
  }
  if(buttons & JOY1_BTN3_MASK) {    // Back
    *battle_mode = 0;
    reset_beam();
    print_str_c(0, 0, (char*)"PRESSED 3");
    set_scale(128);
  }
}
uint8_t fetch_pokeswitch_valid(poke_info *poke_switch_target) {
  if( (poke_switch_target->name_length > 0) && (poke_switch_target->health > 0) ) { // If not a null poke, and not a dead poke
    return 1;
  } else {
    return 0;
  }
}


//---------
//-- BAG --
//---------
void display_battle_screen_bag(const poke_item *poke_item_lookup, uint8_t *poke_bag, uint8_t *poke_bag_size, uint8_t *hovered_bag_index, uint8_t *timer) {
  /*
  . Displays the name, health and sprite for a pokemon in a small box for the poke-switch screen
  */
  const uint8_t element_hheight = 5;
  const uint8_t element_hwidth  = 20;

  set_scale(128);

  for(uint8_t i=0; i<*poke_bag_size; i++) {
    if(i==*hovered_bag_index) {
      display_battle_screen_bag_element( &(poke_item_lookup[poke_bag[2*i]]), i, hovered_bag_index, element_hheight, 2*element_hwidth );
    } else {
      display_battle_screen_bag_element( &(poke_item_lookup[poke_bag[2*i]]), i, hovered_bag_index, element_hheight, element_hwidth );
    }
  }
  display_battle_screen_bag_element_closeup( &(poke_item_lookup[poke_bag[2*(*hovered_bag_index)]]), element_hwidth, element_hwidth );

  // Text at top of screen
  reset_beam();
  set_text_size(-10, 60);
  print_str_c(96, 0, (char*)"BAG");
  set_scale(128);
}
void display_battle_screen_bag_element(const poke_item *item, uint8_t poke_item_position_index, uint8_t *hovered_bag_index, uint8_t hheight, uint8_t hwidth) {
  /*
  . Info on LHS for a poke bag item
  */
  reset_beam();
  // Base for item box
  display_rect(-3*hheight*poke_item_position_index +(*hovered_bag_index)*2*hheight, -50, hheight, hwidth);

  // Name on box
  reset_beam();
  set_text_size(-hheight, 2*hwidth);
  print_str_c(0-3*hheight*poke_item_position_index +(*hovered_bag_index)*2*hheight, -50-hwidth, (char*)(item->name));
  set_scale(128);
}
void display_battle_screen_bag_element_closeup(const poke_item *hovered_item, uint8_t hheight, uint8_t hwidth) {
  /*
  . Image on RHS for the item being hovered
  */
  //Display the item on the right
  reset_beam();
  switch(hovered_item->id) {
    case 1:
      display_tree(0, 50, hwidth);
      //pass
      break;
    case 2:
      display_healstation(0, 50, hwidth);
      //pass
      break;
    case 3:
      display_grass(0, 50, hwidth);
      //pass
      break;
    default:
      display_rect(0, 50, hheight, hwidth);
      break;
  }
}

void calculate_battle_screen_bag(const poke_item poke_item_lookup[], const uint8_t type_effectiveness_lookup[324], const poke_move *poke_move_lookup, uint8_t *battle_mode, uint8_t *hovered_bag_index, uint8_t *poke_bag, uint8_t *poke_bag_size, uint8_t *battle_item_index, char *battle_log_complete, uint8_t *battle_log_stage_length, poke_info ally_poke_party[6], uint8_t *active_ally_poke_index, poke_info hostile_poke_party[6], uint8_t *active_hostile_poke_index, uint8_t *battle_screen_variant, uint8_t *battle_outcome_stage, uint8_t *battle_outcome_timer, int8_t *ally_poke_counter, int8_t *hostile_poke_counter, uint8_t *poke_counter_type) {
  uint8_t buttons = read_buttons();
  if(buttons & JOY1_BTN1_MASK) {    // Cycle bag items
    *hovered_bag_index = (*hovered_bag_index +1)%(*poke_bag_size);
    reset_beam();
    print_str_c(0, 0, (char*)"PRESSED 1");
    set_scale(128);
  }
  if(buttons & JOY1_BTN2_MASK) {    // Select bag item
    *battle_item_index = poke_bag[ 2*(*hovered_bag_index) ]; // Get id from poke bag
    *hovered_bag_index = 0;                                   // Reset hover start (shared with move hover index, hence important to reset each time)
    
    // One-time item triggers at each round start
    switch(poke_item_lookup[poke_bag[2*(*battle_item_index)]].category) {
      case(1):  // If is a ball item ...
        // ###
        // ### ADD RAND IN HERE
        // ###
        hostile_poke_party[*active_hostile_poke_index].shake_state = 3;     // Once decision to capture has been made, set a shake state for hostile poke attempting to be captured
        break;
      case(2):  // If is a heal item...
        //pass
        break;
    }

    *battle_mode = 0;
    *battle_screen_variant = 2;   // So the options are removed (since 1 is the max)
    *battle_outcome_stage = 1;
    *battle_outcome_timer = 0;
    fetch_battle_screen_battle_log(type_effectiveness_lookup, battle_log_complete, battle_log_stage_length, poke_move_lookup, ally_poke_party, active_ally_poke_index, hostile_poke_party, active_hostile_poke_index, ally_poke_counter, hostile_poke_counter, &(poke_item_lookup[*battle_item_index]), battle_outcome_stage, battle_outcome_timer);
    fetch_battle_screen_counters(battle_outcome_stage, poke_move_lookup, ally_poke_party, active_ally_poke_index, hostile_poke_party, active_hostile_poke_index, ally_poke_counter, hostile_poke_counter, poke_counter_type, type_effectiveness_lookup, battle_item_index);

    reset_beam();
    print_str_c(0, 0, (char*)"PRESSED 2");
    set_scale(128);
  }
  if(buttons & JOY1_BTN3_MASK) {    // Back
    *battle_mode = 0;
    *hovered_bag_index = 0;
    reset_beam();
    print_str_c(0, 0, (char*)"PRESSED 3");
    set_scale(128);
  }
}

// ###
// ### GET THE TYPES RIGHT
// ###
void display_roam_screen(uint8_t *timer, uint8_t *terrain, uint8_t terrain_width, uint8_t terrain_height, uint8_t tile_hwidth, uint8_t player_coordinates[2]) {
  /*
  . Displays the screen where the player is allowed to freely roam about the map
  . The player can encounter wild battles here which trigger a battle_screen transition
  */
 display_roam_screen_terrain(terrain, terrain_width, terrain_height, tile_hwidth, player_coordinates);
  display_roam_screen_player(tile_hwidth);
}
void display_roam_screen_terrain(uint8_t *terrain, uint8_t terrain_width, uint8_t terrain_height, uint8_t tile_hwidth, uint8_t player_coordinates[2]) {
  /*
  . Note; Terrain is shifted so the player is at the centre, and terrain outside the screen width is not displayed
  */
  for(uint8_t j=0; j<terrain_height; j++) {
    for(uint8_t i=0; i<terrain_width; i++) {
      if(terrain[i+ j*terrain_width] != 0) {
        int8_t shifted_position_y = -j*2*tile_hwidth +player_coordinates[0]*2*tile_hwidth;
        int8_t shifted_position_x = i*2*tile_hwidth -player_coordinates[1]*2*tile_hwidth;

        if( ((-64+(int16_t)(tile_hwidth)<(int16_t)(shifted_position_y))&&((int16_t)(shifted_position_y)<64-(int16_t)(tile_hwidth))) && ((-64+(int16_t)(tile_hwidth)<(int16_t)(shifted_position_x))&&((int16_t)(shifted_position_x)<64-(int16_t)(tile_hwidth))) ) {
          uint8_t tile_type = terrain[i +terrain_width*j];
          resolve_terrain_display(tile_type, shifted_position_y, shifted_position_x, tile_hwidth);
        }
      }
    }
  }
}
void display_roam_screen_player(uint8_t tile_hwidth) {
  /*
  . Always centered in middle of screen
  . Rotates according to direction moved
  */
  // display_cube(-2*tile_hwidth*player_coordinates[0], 2*tile_hwidth*player_coordinates[1], tile_hwidth>>2);
  display_cube(0, 0, tile_hwidth>>2);
  // reset_beam();
  // set_scale(128);
  // set_text_size(-5, 40);
  // print_str_c(0,0, (char*)"ROAM SCREEN");
  // set_scale(128);
}
uint8_t resolve_terrain_action(uint8_t *terrain, uint8_t terrain_width, uint8_t terrain_height, uint8_t position[2]) {
  /*
  . Calculates what should happen at the given tile type
  . Returned value indicates action;
    0 => Nothing occurs
    1 => Blocked movement
    2 => Encounter chance
    3 => Heal party
    ...
  */
  uint8_t action = 0;
  if( ( (0 <= position[0])||(position[0] < terrain_height) ) && ( (0 <= position[1])||(position[1] < terrain_width) ) ) {
    if( terrain[position[1] +terrain_width*position[0]] == 1 || terrain[position[1] +terrain_width*position[0]] == 3 ) {
      action = 1;
    } else if(terrain[position[1] +terrain_width*position[0]] == 2) {
      action = 2;
    } else {
      action = 0;
    }
  }
  return action;
}
void resolve_terrain_display(uint8_t tile_type, int8_t draw_position_y, int8_t draw_position_x, uint8_t tile_hwidth) {
  /*
  . Calculates what should happen at the given tile type
  . Returned value indicates action;
    0 => Empty
    1 => Tree
    2 => Grass
    3 => Heal station
    ...
  */
  // If 0 (empty), draw nothing
  if(tile_type == 1) {
    display_tree(draw_position_y, draw_position_x, tile_hwidth);
  } else if(tile_type == 2) {
    display_grass(draw_position_y, draw_position_x, tile_hwidth);
  } else if(tile_type == 3) {
    display_healstation(draw_position_y, draw_position_x, tile_hwidth);
  } else {  // If unknown, just draw a cube
    display_cube(draw_position_y, draw_position_x, tile_hwidth);
  }
}
void calculate_roam_screen(uint8_t *terrain, uint8_t terrain_width, uint8_t terrain_height, uint8_t player_coordinates[2], poke_info hostile_poke_party[6], uint8_t *active_hostile_poke_index, uint8_t *battle_log_stage_length, uint8_t *battle_outcome_stage, uint8_t *battle_outcome_timer, uint8_t *hovered_battle_option, uint8_t *battle_screen_variant, uint8_t *battle_mode) {
  uint8_t buttons = read_buttons();
  if(buttons & JOY1_BTN1_MASK) {    // Travel up

    uint8_t following_position[2] = {player_coordinates[0]-1, player_coordinates[1]};
    uint8_t action_resolution = resolve_terrain_action(terrain, terrain_width, terrain_height, following_position);
    if( action_resolution != 1 ) {
      player_coordinates[0] -= 1;
    }
    if( action_resolution == 2 ) {
      if(rand() < 5000) {
        switch_to_battle(hostile_poke_party, active_hostile_poke_index, battle_log_stage_length, battle_outcome_stage, battle_outcome_timer, hovered_battle_option, battle_screen_variant, battle_mode);
      }
    }

    reset_beam();
    print_str_c(0, 0, (char*)"PRESSED 1");
    set_scale(128);
  }
  if(buttons & JOY1_BTN2_MASK) {    // Travel down
    
    uint8_t following_position[2] = {player_coordinates[0]+1, player_coordinates[1]};
    uint8_t action_resolution = resolve_terrain_action(terrain, terrain_width, terrain_height, following_position);
    if( action_resolution != 1 ) {
      player_coordinates[0] += 1;
    }
    if( action_resolution == 2 ) {
      if(rand() < 5000) {
        switch_to_battle(hostile_poke_party, active_hostile_poke_index, battle_log_stage_length, battle_outcome_stage, battle_outcome_timer, hovered_battle_option, battle_screen_variant, battle_mode);
      }
    }

    reset_beam();
    print_str_c(0, 0, (char*)"PRESSED 2");
    set_scale(128);
  }
  if(buttons & JOY1_BTN3_MASK) {    // Travel right
    
    uint8_t following_position[2] = {player_coordinates[0], player_coordinates[1]+1};
    uint8_t action_resolution = resolve_terrain_action(terrain, terrain_width, terrain_height, following_position);
    if( action_resolution != 1 ) {
      player_coordinates[1] += 1;
    }
    if( action_resolution == 2 ) {
      if(rand() < 5000) {
        switch_to_battle(hostile_poke_party, active_hostile_poke_index, battle_log_stage_length, battle_outcome_stage, battle_outcome_timer, hovered_battle_option, battle_screen_variant, battle_mode);
      }
    }

    reset_beam();
    print_str_c(0, 0, (char*)"PRESSED 3");
    set_scale(128);
  }
}

// ###
// ### NOW NEEDS TO CALL THIS FUNC AGAIN WHEN BATTLE IS STARTED -> RESET OPPONENT
// ###

void generate_pokemon(poke_info poke_party[6], uint8_t poke_index, uint8_t preset) {
  if(preset==1) {
    poke_info generated_poke = {
      "CHAR", 4,
      {2,0},
      5, 4, 20,
      20, 28,
      4,
      {4,2,0,0}, 0, 0, 0,
      3, 7,
      6, 2,
      0
    };
    poke_party[poke_index] = generated_poke;
  } else if(preset==2) {
    poke_info generated_poke = {
      "STAR", 4,
      {1,0},
      28, 98, 132,
      17, 30,
      3,
      {2,3,1,0}, 0, 0, 0,
      3, 7,
      6, 2,
      0
    };
    poke_party[poke_index] = generated_poke;
  } else if(preset==3) {
    poke_info generated_poke = {
      "SPEE", 4,
      {1,0},
      28, 98, 132,
      17, 30,
      6,
      {3,2,1,0}, 0, 0, 0,
      3, 7,
      6, 2,
      0
    };
    poke_party[poke_index] = generated_poke;
  } else {
    // If preset not recognised, default to null pokemon
    poke_info null_poke = {
      "", 0,   // <-- This 0 length name indicates a NULL poke (empty space)
      {0,0},
      0, 0, 0,
      0, 0,
      0,
      {0,0,0,0}, 0, 0, 0,
      0, 0,
      0, 0,
      0
    };
    poke_party[poke_index] = null_poke;
  }
  
}
void capture_pokemon(poke_info target_poke_party[6], poke_info captured_poke) {
  /*
  . Adds the captured poke to the target_poke_party if there is space
  . Adds the captured poke to the pokebox if there is no space in the party ### NOT IMPLEMENTED YET ###
  */
  for(uint8_t i=0; i<6; i++) {
    if(target_poke_party[i].name_length == 0) { // If is a NULL space, e.g. empty, then add the captured_poke here
      target_poke_party[i] = captured_poke;
      break;
    }
    // Ignore if no space is found FOR NOW
  }
  // ### If space is never found, add logic for adding to box instead ###
}

void switch_to_roam(uint8_t *battle_log_stage_length, uint8_t *battle_outcome_stage, uint8_t *battle_outcome_timer, uint8_t *hovered_battle_option, uint8_t *battle_screen_variant, uint8_t *battle_mode) {
  /*
  . Resets all the battle parameters and chanegs the game state to work for roaming mode
  */
  // Reset params
  *battle_log_stage_length = 0;
  *battle_outcome_stage = 0;
  *battle_outcome_timer = 0;
  *hovered_battle_option = 0;
  *battle_screen_variant = 0;
  // Roam mode
  *battle_mode = 3;
}
void switch_to_battle(poke_info hostile_poke_party[6], uint8_t *active_hostile_poke_index, uint8_t *battle_log_stage_length, uint8_t *battle_outcome_stage, uint8_t *battle_outcome_timer, uint8_t *hovered_battle_option, uint8_t *battle_screen_variant, uint8_t *battle_mode) {
  /*
  . Resets all the battle parameters and chanegs the game state to work for battling mode
  */
  // Reset params
  *battle_log_stage_length = 0;
  *battle_outcome_stage = 0;
  *battle_outcome_timer = 0;
  *hovered_battle_option = 0;
  *battle_screen_variant = 0;
  // Battle mode
  *battle_mode = 0;
  // Setup opponent
  generate_pokemon(hostile_poke_party, 0, 2);
  *active_hostile_poke_index = 0; // Reset to deal with first poke when starting a battle
}


//----------
//-- MAIN --
//----------
// Fixed Info
const poke_move poke_move_lookup[] = {   // Retrieve moves from the index held by pokemon; Stored in ROM for space
  {
    "",   // Name --> Placeholder move to give empty move slot e.g. cannot be used, empty move slot
    0,    // Name Length
    0,    // Type
    0,    // Attack
    0,    // Special Attack
    0   // Probability
  },
  {
    "FLAIL",   // Name
    5,    // Name Length
    3,    // Type
    0,    // Attack
    0,    // Special Attack
    255   // Probability
  },
  {
    "TACKLE",  // Name
    6,    // Name Length
    1,    // Type
    5,    // Attack
    0,    // Special Attack
    255   // Probability
  },
  {
    "LEER",   // Name
    4,    // Name Length
    1,    // Type
    0,    // Attack
    0,    // Special Attack
    255   // Probability
  },
  {
    "EMBER",   // Name
    5,    // Name Length
    2,    // Type
    10,    // Attack
    7,    // Special Attack
    255   // Probability
  },
};
const poke_item poke_item_lookup[] = {   // Retrieve items from an index stored elsewhere; Stored in ROM for space
  {
    "",   // Name --> Placeholder move to give empty move slot e.g. cannot be used, empty item slot
    0,    // Name Length
    0,    // ID
    0,    // Category
  },
  {
    "POKEBALL",   // Name
    8,    // Name Length
    1,    // ID
    1,    // Category
  },
  {
    "GREATBALL",   // Name
    9,    // Name Length
    2,    // ID
    1,    // Category
  },
  {
    "POTION",   // Name
    6,    // Name Length
    3,    // ID
    2,    // Category
  },
};
// ### MAY NEED LENGTHS FOR EACH OF THESE AS WELL -> STORE IN ADJACENT ARRAY ###
const char* poke_type_lookup[] = {   // Retrieve moves from the index held by pokemon; Stored in ROM for space
  "NON",
  "NORMAL",
  "FIRE",
  "WATER",
  "GRASS",
  "ELECTRIC",
  "ICE",
  "FIGHTING",
  "POISON",
  "GROUND",
  "FLYING",
  "PSYCHIC",
  "BUG",
  "ROCK",
  "GHOST",
  "DRAGON",
  "DARK",
  "STEEL",
  "FAIRY",
};

// 0=NoEffect, 1=NotVery, 2=Normal, 4=Super
// E.G. Damage multiplier = Value/2
const uint8_t type_effectiveness_lookup[324] = {
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 2, 2, 1, 2,
  2, 1, 1, 4, 2, 4, 2, 2, 2, 2, 2, 4, 1, 2, 1, 2, 4, 2,
  2, 4, 1, 1, 2, 2, 2, 2, 4, 2, 2, 2, 4, 2, 1, 2, 2, 2,
  2, 1, 4, 1, 2, 2, 2, 1, 4, 1, 2, 1, 4, 2, 1, 2, 1, 2,
  2, 2, 4, 1, 1, 2, 2, 2, 0, 4, 2, 2, 2, 2, 1, 2, 2, 2,
  2, 1, 1, 4, 2, 1, 2, 2, 4, 4, 2, 2, 2, 2, 4, 2, 1, 2,
  4, 2, 2, 2, 2, 4, 2, 1, 2, 1, 1, 1, 4, 0, 2, 4, 4, 1,
  2, 2, 2, 4, 2, 2, 2, 1, 1, 2, 2, 2, 1, 1, 2, 2, 0, 4,
  2, 4, 2, 1, 4, 2, 2, 4, 2, 0, 2, 1, 4, 2, 2, 2, 4, 2,
  2, 2, 2, 4, 1, 2, 4, 2, 2, 2, 2, 4, 1, 2, 2, 2, 1, 2,
  2, 2, 2, 2, 2, 2, 4, 4, 2, 2, 1, 2, 2, 2, 2, 0, 1, 2,
  2, 1, 2, 4, 2, 2, 1, 1, 2, 1, 4, 2, 2, 1, 2, 4, 1, 1,
  2, 4, 2, 2, 2, 4, 1, 2, 1, 4, 2, 4, 2, 2, 2, 2, 1, 2,
  0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 4, 2, 1, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 2, 1, 0,
  2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 4, 2, 2, 4, 2, 1, 2, 1,
  2, 1, 1, 2, 1, 4, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 1, 4,
  2, 1, 2, 2, 2, 2, 4, 1, 2, 2, 2, 2, 2, 2, 4, 4, 1, 2,
};

const uint8_t terrain_width  = 10;
const uint8_t terrain_height = 10;
const uint8_t tile_hwidth = 4;    // How wide each tile is in vectrex-screen units (128 half screen width)
  

int main()
{
  /*
  battle_mode = 
      0 => Choosing which option to pick initially (fight, bag, poke, run)  <-- Reset the hovered_option on change
           Choosing which fight option to use (Atk1, Atk2, Atk3, Atk4)      <-- "" ""
      1 => Choosing which bag option to use [Screen Change]
      2 => Choosing which poke to use [Screen Change]
      3 => Exit battle screen [Screen Change]
      ...

  - TERRAIN LOOKUP -
  0 = Empty space
  1 = Tree (wall)
  2 = Grass (poke encounter)
  3 = Heal station
  ...
  */
  uint8_t terrain[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 2, 2, 2, 0, 0, 3, 0, 1,
    1, 0, 2, 2, 0, 0, 0, 0, 0, 1,
    1, 0, 2, 0, 2, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  };

  // Player setup
  uint8_t player_coordinates[2] = {1,1};
  
  // Poke item setup -> Formatted as [id, quantity,  id, quantity,  ...]
  uint8_t poke_bag[10] = {1,5, 3,1, 2,1, 0,0, 0,0,};
  uint8_t poke_bag_size = 5;  // Size of N => 2*N spaces in list (to store quantity too)

  // Poke setup
  // ###
  // ### POSSIBLY CAN INCREASE POKE NAME SIZE -> SEE HOW MUCH OF AN ISSUE MEMORY IS
  // ###
  poke_info ally_poke_party[6];
  poke_info hostile_poke_party[6];
  uint8_t active_ally_poke_index = 0;
  uint8_t active_hostile_poke_index = 0;
  generate_pokemon(ally_poke_party, 0, 1);
  generate_pokemon(ally_poke_party, 1, 2);
  generate_pokemon(ally_poke_party, 2, 0);
  generate_pokemon(ally_poke_party, 3, 0);
  generate_pokemon(ally_poke_party, 4, 0);
  generate_pokemon(ally_poke_party, 5, 0);

  generate_pokemon(hostile_poke_party, 0, 0);
  generate_pokemon(hostile_poke_party, 1, 0);
  generate_pokemon(hostile_poke_party, 2, 0);
  generate_pokemon(hostile_poke_party, 3, 0);
  generate_pokemon(hostile_poke_party, 4, 0);
  generate_pokemon(hostile_poke_party, 5, 0);


  char test_battle_log[100];

  // **Note; Set to large initial value like 100 to leave space for possible large strings in the future + empty space
  char battle_log_complete[100];                // Update only when needed; stores the full battle log resultant text and is spliced later; DO NOT fetch raw each time; battle-log will lerp off this stored result
  uint8_t battle_log_stage_length = 0;          // Holds the number of characters used for each stage of the battle log, Note**; 12 since 6 stages per side of the fight <-- This NEEDS increasing if more stages are added e.g. for status effects

  uint8_t timer = 0;  // Continually ticks -> used for animations
  uint8_t battle_outcome_stage = 0;   // Which 'stage' of the battle outcome process you are in e.g. (0) Read attack, (1) Show attack anim, (2) Show crits, etc
  uint8_t battle_outcome_timer = 0;   // A timer for after a battle move/action has been chosen and the resulting outcome is shown to the player
  
  int8_t temp_counter_1 = 0;    // Used to count through values, but is multi-purpose; Used for counting HP, EXP, etc in battle outcome animation
  int8_t temp_counter_2 = 0;    // "" ""
  uint8_t temp_counter_3 = 0;   // "" ""; Used to track the type of resource being changed in the battle animation

  uint8_t hovered_battle_option = 0;  // Which battle option is readied to be selected
  uint8_t battle_screen_variant = 0;  // Which variant of the battle screen to show (0=BattleOptions, 1=MoveOptions, ...)
  uint8_t battle_item_index = 0;      // The index of the battle item to be used this turn of the combat -> 0 => no item, >0 => consider lookup index

  // ###
  // ### RENAME THIS TO MAKE MORE SENSE
  // ###  --> WILL NEED TO BE RESET TO 0 EACH TIME FOR SAFETY
  // ###
  uint8_t hovered_pokeswitch = 0;     // Which pokemon index (in team) is being hovered when in the pokeswitch screen OR the hovered bag item index
  
  uint8_t battle_mode = 3;

  while(1)
  {
    wait_retrace();

    if(battle_mode==0) {    // Fight Screen
      display_battle_screen(&(ally_poke_party[0]), &active_ally_poke_index, &(hostile_poke_party[0]), &active_hostile_poke_index, poke_move_lookup, &battle_item_index, battle_log_complete, &battle_log_stage_length, hovered_battle_option, &battle_screen_variant, &battle_outcome_stage, &battle_outcome_timer, battle_mode, &timer); // ### SHOULD PROBABLY JUST PARSE POINTERS HERE TOO ###
      calculate_battle_screen(poke_item_lookup, type_effectiveness_lookup, battle_log_complete, &battle_log_stage_length, poke_move_lookup, ally_poke_party, &active_ally_poke_index, hostile_poke_party, &active_hostile_poke_index, &battle_item_index, &hovered_battle_option, &battle_screen_variant, &battle_mode, &battle_outcome_stage, &battle_outcome_timer, &temp_counter_1, &temp_counter_2, &temp_counter_3);
    }
    if(battle_mode==1) {    // Battle Bag Screen
      display_battle_screen_bag(poke_item_lookup, poke_bag, &poke_bag_size, &hovered_pokeswitch, &timer);
      calculate_battle_screen_bag(poke_item_lookup, type_effectiveness_lookup, poke_move_lookup, &battle_mode, &hovered_pokeswitch, poke_bag, &poke_bag_size, &battle_item_index, battle_log_complete, &battle_log_stage_length, ally_poke_party, &active_ally_poke_index, hostile_poke_party, &active_hostile_poke_index, &battle_screen_variant, &battle_outcome_stage, &battle_outcome_timer, &temp_counter_1, &temp_counter_2, &temp_counter_3);
    }
    if(battle_mode==2) {    // Poke-switch Screen
      display_battle_screen_pokeswitch_screen(ally_poke_party, &active_ally_poke_index, hovered_pokeswitch, timer);
      calculate_battle_screen_pokeswitch(&hovered_pokeswitch, poke_move_lookup, poke_item_lookup, type_effectiveness_lookup, &temp_counter_1, &temp_counter_2, &temp_counter_3, ally_poke_party, &active_ally_poke_index, hostile_poke_party, &active_hostile_poke_index, &battle_item_index, battle_log_complete, &battle_log_stage_length, &battle_mode, &battle_screen_variant, &battle_outcome_stage, &battle_outcome_timer);
    }
    if(battle_mode==3) {    // Roam Screen
      display_roam_screen(&timer, terrain, terrain_width, terrain_height, tile_hwidth, player_coordinates);
      calculate_roam_screen(terrain, terrain_width, terrain_height, player_coordinates, hostile_poke_party, &active_hostile_poke_index, &battle_log_stage_length, &battle_outcome_stage, &battle_outcome_timer, &hovered_battle_option, &battle_screen_variant, &battle_mode);
    }
    timer++;


    reset_beam();
    set_scale(128);
    set_text_size(-5, 40);

    // char debug[20];
    // sprintf(debug, "%u", rand());    // (0, 2^15-1]
    // print_str_c(0, 0, debug);
    // if(rand() < 10000){
    //   print_str_c(-20, 0, (char*)"TRUE");
    // } else {
    //   print_str_c(-20, 0, (char*)"FALSE");
    // }
    // set_scale(128);

    // char int_char_timer[4];
    // int_char_timer[0] = '0' +(battle_item_index);
    // int_char_timer[1] = '0' +(hovered_pokeswitch);
    // int_char_timer[2] = '2';
    // int_char_timer[3] = '\0';
    // print_str_c(20, 0, int_char_timer);
  }
  return 0;
};

/*
######
###### ]Currently all works EXCEPT new bag stuff which appears to break memory afterwards
######    Make it perform subsets of this process to find where it breaks
######

###
### Swapping works EXCEPT for the first turn
###     -> Is actually that you have to hit button 2 then 3???
### Text is also broken currently -> Probably related to seg fault when switching poke_info name from any length to fixed length
###

###
### ITEM -> MOVE LEADS TO NAMEING FAULTS --> LIKELY SOME USED VALUE HAS NOT RESET
###       Other way around works fine => POKE BAG ACTS RESULT IN DISRUPTION
###

###
### Need to get cycles to be around 30k for good framerate/minimal flickering
###   -> Currently between (100-180)k
###

====
TODO
====
(1) Switching + catching pokemon
(2) Items / bag working

(1) Learn moves with levels
(2) Fainting mechanics / black out / related
(3) Poke visuals add

(1) Temp stat changing moves need to be added
(2) Status effects on moves
*/