#ifndef POKE_INFO_H
#define POKE_INFO_H

typedef struct {   // All information about a specific stored pokemon
  char* name;             // ### LIST OF CHAR, BUT HOW LONG? ### <- WANT TO SPECIFY ON CREATION, COULD JUST HAVE A STANDARD AMOUNT THAT EACH NAME IS FIT TOO

  uint8_t health;
  uint8_t max_health;
  uint8_t speed;

  uint8_t moves[4];     // Index of moves owned by this pokemon -> found via a lookup with the index

  uint8_t attack;
  uint8_t special_attack;

  uint8_t defence;
  uint8_t special_defence;
} poke_info;

#endif

#ifndef POKE_ITEM_H
#define POKE_ITEM_H

typedef struct {
  char* name;
  uint8_t type;   // E.g Fire, Grass, Water, ...
  uint8_t attack;         // 0 if does no damage of this type
  uint8_t special_attack; //
  // ### Use POINTER to functions to apply buffs to a pokemon ###
} poke_move;
typedef struct {
  char* name;
  //pass
} poke_item;

#endif

void display_cube(int8_t origin_y, int8_t origin_x, uint8_t radius);
void display_rect(int8_t origin_y, int8_t origin_x, uint8_t hheight, uint8_t hwidth);

void display_battle_screen(char **battle_log_complete, uint8_t hovered_battle_option, uint8_t *battle_screen_variant, uint8_t *battle_outcome_stage, uint8_t *battle_outcome_timer, uint8_t battle_mode, uint8_t timer);

void display_battle_screen_pokemons();
void display_battle_screen_pokemon(const int8_t poke_position_y, uint8_t poke_radius, const uint8_t *poke_stats_dimensions, const uint8_t isLeftSide);
void display_battle_screen_pokemon_stats(const int8_t position_y, const uint8_t isLeftSide, const uint8_t *poke_stats_dimensions);

void display_battle_screen_battle_options(uint8_t hovered_option, const uint8_t battle_log_extension, const uint8_t battle_options_hheight, uint8_t timer);
void display_battle_screen_battle_options_fight(uint8_t hovered_option, const uint8_t battle_log_extension, const uint8_t battle_options_hheight, uint8_t timer);

char *fetch_battle_screen_battle_log(char **battle_log_complete, uint8_t *battle_outcome_stage, uint8_t *battle_outcome_timer);
void display_battle_screen_battle_log(char **battle_log_complete, uint8_t *battle_outcome_stage, uint8_t *battle_outcome_timer, const uint8_t battle_log_extension, const uint8_t battle_options_hheight);

void calculate_battle_screen(uint8_t *hovered_battle_option, uint8_t *battle_screen_variant, uint8_t *battle_mode, uint8_t *battle_outcome_stage, uint8_t *battle_outcome_timer);

void display_battle_screen_bag();
void calculate_battle_screen_bag(uint8_t *battle_mode);

void display_battle_screen_pokeswitch_screen(uint8_t hovered_pokeswitch, uint8_t timer);
void display_battle_screen_pokeswitch_poke_details(int8_t origin_y, int8_t origin_x, uint8_t screen_hheight, uint8_t screen_hwidth);
void calculate_battle_screen_pokeswitch(uint8_t *hovered_pokeswitch, uint8_t *battle_mode);
