#include "vectrex.h"
#include "vectrex/bios.h"
#include "poketrex_main.h"

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


// -------------------
// -- BATTLE_SCREEN --
// -------------------
void display_battle_screen(uint8_t hovered_battle_option, uint8_t battle_mode, uint8_t timer) {
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

  display_battle_screen_pokemons();  // ### PARSE IN POKEMON HERE ### <-- POINTERS
  switch(battle_mode) {
    case 0:
      display_battle_screen_battle_options(hovered_battle_option, battle_log_extension, battle_options_hheight, timer);
      break;
    case 1:
      display_battle_screen_battle_options_fight(hovered_battle_option, battle_log_extension, battle_options_hheight, timer);
      break;
  }
  display_battle_screen_battle_log((char*)"BATTLE_LOG", battle_log_extension, battle_options_hheight);
};


void display_battle_screen_pokemons() {
  /*
  . Displays both pokemon involved in a battle
  . Position of their display is fixed
  */
  const uint8_t poke_radius = 20;
  const int8_t poke_friendly_position_y = -40;  // Lower half --> Will always display the sprite and poke-bar on opposite sides of the screen horizontally, only the height is required
  const int8_t poke_hostile_position_y = 80;    // Upper half
  // ### HAVE OFFSETS FOR ANIMATIONS REMEMVERED HERE ###

  const uint8_t poke_stats_dimensions[2] = {10, 40};   // (hheight, hwidth) format

  display_battle_screen_pokemon(poke_friendly_position_y, poke_radius, poke_stats_dimensions, 1);
  display_battle_screen_pokemon(poke_hostile_position_y, poke_radius, poke_stats_dimensions, 0);
};
void display_battle_screen_pokemon(const int8_t poke_position_y, uint8_t poke_radius, const uint8_t *poke_stats_dimensions, const uint8_t isLeftSide) {
  /*
  . Displays a specific pokemon in the battle screen
  . Displays relevant information about the pokemon alongside its sprite, such as;
    . Health
    . Experience
  */
  int8_t poke_position_x = (isLeftSide==1) ? (int8_t)(-128+(2*poke_radius)) : (int8_t)(poke_position_x = 128-2*poke_radius);  // L / R side

  display_cube( poke_position_y, poke_position_x, poke_radius );   // Poke sprite
  display_battle_screen_pokemon_stats(poke_position_y, isLeftSide, poke_stats_dimensions);  // Poke stats bar
};
void display_battle_screen_pokemon_stats(const int8_t bar_position_y, const uint8_t isLeftSide, const uint8_t *poke_stats_dimensions) {
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

  // ###
  // ### Should prob parse in from data outside
  // ###
  uint8_t health = 36;
  uint8_t health_max = 42;
  const uint8_t health_ratio = (uint8_t)( ((uint16_t)health << 8) /health_max );  // As a ratio of 256, truncated in division if fractional part remaining

  uint8_t experience = 12;
  uint8_t experience_max = 60;
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
  reset_beam();
  set_text_size( -(poke_stats_dimensions[0]>>1), poke_stats_dimensions[1] );
  print_str_c(bar_position_y -(poke_stats_dimensions[0]>>1), bar_position_x-(poke_stats_dimensions[1]) +(poke_stats_dimensions[1]>>2), (char*)"NAME");
  print_str_c(bar_position_y -(poke_stats_dimensions[0]>>1), bar_position_x, (char*)"LVL:XX");
  set_scale(128);
}


void display_battle_screen_battle_options(uint8_t hovered_option, const uint8_t battle_log_extension, const uint8_t battle_options_hheight, uint8_t timer) {
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
      timer
    );
    display_hovered_star(
      -128+buffer+2*battle_options_hheight +y_factor*battle_options_hheight +(battle_options_hheight>>1), 
      128-buffer-2*option_hwidth +x_factor*option_hwidth +(option_hwidth>>1) +(option_hwidth>>2), 
      2, 
      timer
    );
  }
};

void display_battle_screen_battle_options_fight(uint8_t hovered_option, const uint8_t battle_log_extension, const uint8_t battle_options_hheight, uint8_t timer) {
  /*
  . Displays the 4 moves the pokemon can use
    . Move1 . Move2
    . Move3 . Move4
  */
  uint8_t buffer = 2;
  uint8_t option_hwidth = (128-battle_log_extension+2*buffer)/4;  // Truncated float to uint8_t
  char* moves[4] = {
    (char*)"MOVE1",
    (char*)"MOVE2",
    (char*)"MOVE3",
    (char*)"MOVE4",
  };

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
  print_str_c(-128+buffer+2*battle_options_hheight +1*battle_options_hheight +(battle_options_hheight>>1), 128-buffer-2*option_hwidth -1*option_hwidth -(option_hwidth>>1), moves[0]);
  print_str_c(-128+buffer+2*battle_options_hheight +1*battle_options_hheight +(battle_options_hheight>>1), 128-buffer-2*option_hwidth +1*option_hwidth -(option_hwidth>>1), moves[1]);
  print_str_c(-128+buffer+2*battle_options_hheight -1*battle_options_hheight +(battle_options_hheight>>1), 128-buffer-2*option_hwidth +1*option_hwidth -(option_hwidth>>1), moves[2]);
  print_str_c(-128+buffer+2*battle_options_hheight -1*battle_options_hheight +(battle_options_hheight>>1), 128-buffer-2*option_hwidth -1*option_hwidth -(option_hwidth>>1), moves[3]);
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
      timer
    );
    display_hovered_star(
      -128+buffer+2*battle_options_hheight +y_factor*battle_options_hheight +(battle_options_hheight>>1), 
      128-buffer-2*option_hwidth +x_factor*option_hwidth +(option_hwidth>>1) +(option_hwidth>>2), 
      2, 
      timer
    );
  }
};


void display_battle_screen_battle_log(char* battle_text, const uint8_t battle_log_extension, const uint8_t battle_options_hheight) {
  /*
  . Displays the battle log, which describes events occurring in the battle in a text box
  */
  //display_rect(-128+2*battle_options_hheight, -128+battle_log_hwidth, 2*battle_options_hheight, battle_log_hwidth);

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


void calculate_battle_screen(uint8_t *hovered_battle_option, uint8_t *battle_mode) {
  uint8_t buttons = read_buttons();
  if(buttons & JOY1_BTN1_MASK) {
    *hovered_battle_option = (*hovered_battle_option+1)%4;
    reset_beam();
    print_str_c(0, 0, (char*)"PRESSED 1");
    set_scale(128);
  }
  if(buttons & JOY1_BTN2_MASK) {
    *hovered_battle_option = (*hovered_battle_option-1);
    if(*hovered_battle_option > 3) {*hovered_battle_option=3;}  // Will loop to 256 when attempting to go negative
    reset_beam();
    print_str_c(0, 0, (char*)"PRESSED 2");
    set_scale(128);
  }
  if(buttons & JOY1_BTN3_MASK) {
    *battle_mode = (*battle_mode+1)%6;
    reset_beam();
    print_str_c(0, 0, (char*)"PRESSED 3");
    set_scale(128);
  }
  if( (buttons & JOY1_BTN4_MASK) && (hovered_battle_option==0) ) {  // Select button when hovering fight option
    *battle_mode = 1;
    reset_beam();
    print_str_c(0, 0, (char*)"PRESSED 3");
    set_scale(128);
  }
}

//-----------------------
//-- POKESWITCH_SCREEN --
//-----------------------

void display_battle_screen_pokeswitch_screen(uint8_t hovered_pokeswitch, uint8_t timer) {
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
      display_battle_screen_pokeswitch_poke_details(128-title_buffer-(switchbox_hheight>>2) -j*2*switchbox_hheight, -128+(64-switchbox_hwidth) +i*2*switchbox_hwidth, switchbox_hheight, switchbox_hwidth);
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

void display_battle_screen_pokeswitch_poke_details(int8_t origin_y, int8_t origin_x, uint8_t switchbox_hheight, uint8_t switchbox_hwidth) {
  // ###
  // ### PARSE IN POKE DETAILS ###
  // ###
  /*
  . Displays the name, health and sprite for a pokemon in a small box for the poke-switch screen
  */
  reset_beam();
  set_scale(128);
  set_text_size(-5, 40);  // ### MAKE THIS SCALE WITH BOX DIMS ###
  print_str_c(origin_y, origin_x, (char*)"POKE_NAME");
  print_str_c(origin_y -(switchbox_hheight>>1) -(switchbox_hheight>>2), origin_x, (char*)"HP: XX/YY");
}

void calculate_battle_screen_pokeswitch(uint8_t *hovered_pokeswitch, uint8_t *battle_mode) {
  uint8_t buttons = read_buttons();
  if(buttons & JOY1_BTN1_MASK) {
    *hovered_pokeswitch = (*hovered_pokeswitch+1)%6;
    reset_beam();
    print_str_c(0, 0, (char*)"PRESSED 1");
    set_scale(128);
  }
}

//----------
//-- MAIN --
//----------

int main()
{
  /*
  battle_mode = 
      0 => Choosing which option to pick initially (fight, bag, poke, run)  <-- Reset the hovered_option on change
      1 => Choosing which fight option to use (Atk1, Atk2, Atk3, Atk4)      <-- "" ""
      2 => Choosing which bag option to use [Screen Change]
      3 => Choosing which poke to use [Screen Change]
      4 => Exit battle screen [Screen Change]
      5 => Watch the battle occur [Remove options, leave battle log alone]
      ...
  */
  uint8_t timer = 0;  // Continually ticks -> used for animations

  uint8_t hovered_battle_option = 1;  // Which battle option is readied to be selected
  uint8_t hovered_pokeswitch = 0;     // Which pokemon index (in team) is being hovered when in the pokeswitch screen
  uint8_t battle_mode = 0;

  while(1)
  {
    wait_retrace();
    // display_battle_screen(hovered_battle_option, battle_mode, timer);   // ### SHOULD PROBABLY JUST PARSE POINTERS HERE TOO ###
    // calculate_battle_screen(&hovered_battle_option, &battle_mode);

    display_battle_screen_pokeswitch_screen(hovered_pokeswitch, timer);
    calculate_battle_screen_pokeswitch(&hovered_pokeswitch, &battle_mode);

    timer++;
  }
  return 0;
};