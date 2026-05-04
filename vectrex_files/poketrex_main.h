#ifndef POKE_INFO_H
#define POKE_INFO_H

typedef struct {   // All information about a specific stored pokemon
  char* name;
  uint8_t name_length;
  const uint8_t types[2];

  uint8_t level;
  uint8_t experience;     // Experience gain so far this level
  uint8_t experience_max; // Experience required for the current level

  uint8_t health;
  uint8_t health_max;
  uint8_t speed;

  // NOTE**; PP not implemented yet
  uint8_t moves[4];     // Index of moves owned by this pokemon -> found via a lookup with the index
  uint8_t active_move;  // Index of the move chosen to be used by the pokemon -> must be in [0,4)

  uint8_t attack;
  uint8_t special_attack;

  uint8_t defence;
  uint8_t special_defence;
} poke_info;

#endif

#ifndef POKE_MOVE_H
#define POKE_MOVE_H

typedef struct {   // All information about a specific stored pokemon
  char* name;
  uint8_t name_length;

  const uint8_t type;   // e.g Grass, Water, Fire, ...
  const uint8_t attack;
  const uint8_t special_attack;
  const uint8_t probability;  // X/256 hit chance
  // pass -> For non-damaging moves may need to use function approach for the affect it has? -> OR lazily could include 'damage' for each stat as well -> 0 for null
} poke_move;

#endif

#ifndef POKE_ITEM_H
#define POKE_ITEM_H

typedef struct {
  char* name;
  uint8_t name_length;
  //pass -> Maybe store pointer to function for action of the item?
} poke_item;

#endif

void concat_strings(char* strings[], uint8_t string_number, uint8_t *lengths, uint8_t *concat_length, char* concat_string);
uint8_t sum_256(uint8_t *list, uint8_t list_length);
int8_t linear_to_oscil(uint8_t x);

void display_cube(int8_t origin_y, int8_t origin_x, uint8_t radius);
void display_rect(int8_t origin_y, int8_t origin_x, uint8_t hheight, uint8_t hwidth);

uint8_t fetch_move_effectiveness(uint8_t move_type, const uint8_t recipient_types[2]);

void display_battle_screen(poke_info *poke_ally, poke_info *poke_hostile, const poke_move *poke_move_lookup, char **battle_log_complete, uint8_t *battle_log_stage_length, uint8_t hovered_battle_option, uint8_t *battle_screen_variant, uint8_t *battle_outcome_stage, uint8_t *battle_outcome_timer, uint8_t battle_mode, uint8_t *timer);

void display_battle_screen_pokemons(poke_info *poke_ally, poke_info *poke_hostile, uint8_t *timer);
void display_battle_screen_pokemon(poke_info *poke, int8_t poke_position_y, uint8_t poke_radius, const uint8_t *poke_stats_dimensions, const uint8_t isLeftSide, uint8_t *timer);
void display_battle_screen_pokemon_stats(poke_info *poke, int8_t position_y, const uint8_t isLeftSide, const uint8_t *poke_stats_dimensions);

void display_battle_screen_battle_options(uint8_t hovered_option, const uint8_t battle_log_extension, const uint8_t battle_options_hheight, uint8_t *timer);
void display_battle_screen_battle_options_fight(poke_info *poke, const poke_move *poke_move_lookup, uint8_t hovered_option, const uint8_t battle_log_extension, const uint8_t battle_options_hheight, uint8_t *timer);

void fetch_battle_screen_battle_log(char **battle_log_complete, uint8_t *battle_log_stage_length, const poke_move *poke_move_lookup, poke_info *poke_ally, poke_info *poke_hostile, uint8_t *battle_outcome_stage, uint8_t *battle_outcome_timer);
uint8_t fetch_party_fainted_count();
char* fetch_battle_screen_battle_log_subset(char **battle_log_complete, uint8_t *battle_log_stage_length, uint8_t *battle_log_stage, uint8_t *battle_log_timer);
void display_battle_screen_battle_log(char **battle_log_complete, uint8_t *battle_log_stage_length, uint8_t *battle_outcome_stage, uint8_t *battle_outcome_timer, const uint8_t battle_log_extension, const uint8_t battle_options_hheight);

void fetch_battle_screen_counters(uint8_t *battle_outcome_stage, poke_info *poke_ally, poke_info *poke_hostile, int8_t *poke_ally_counter, int8_t *poke_hostile_counter, uint8_t *poke_counter_type);

void calculate_battle_screen(char **battle_log_complete, uint8_t *battle_log_stage_length, const poke_move *poke_move_lookup, poke_info *poke_ally, poke_info *poke_hostile, uint8_t *hovered_battle_option, uint8_t *battle_screen_variant, uint8_t *battle_mode, uint8_t *battle_outcome_stage, uint8_t *battle_outcome_timer, int8_t *poke_ally_counter, int8_t *poke_hostile_counter, uint8_t *poke_counter_type);
void calculate_battle_screen_battle_timing(char **battle_log_complete, uint8_t *battle_log_stage_length, const poke_move *poke_move_lookup, poke_info *poke_ally, poke_info *poke_hostile, uint8_t *hovered_battle_option, uint8_t *battle_mode, uint8_t *battle_screen_variant, uint8_t *battle_outcome_stage, uint8_t *battle_outcome_timer, int8_t *poke_ally_counter, int8_t *poke_hostile_counter, uint8_t *poke_counter_type);
void calculate_battle_screen_buttons(char **battle_log_complete, uint8_t *battle_log_stage_length, const poke_move *poke_move_lookup, poke_info *poke_ally, poke_info *poke_hostile, uint8_t *hovered_battle_option, uint8_t *battle_screen_variant, uint8_t *battle_mode, uint8_t *battle_outcome_stage, uint8_t *battle_outcome_timer, int8_t *poke_ally_counter, int8_t *poke_hostile_counter, uint8_t *poke_counter_type);

void display_battle_screen_bag();
void calculate_battle_screen_bag(uint8_t *battle_mode);

void display_battle_screen_pokeswitch_screen(uint8_t hovered_pokeswitch, uint8_t timer);
void display_battle_screen_pokeswitch_poke_details(int8_t origin_y, int8_t origin_x, uint8_t screen_hheight, uint8_t screen_hwidth);
void calculate_battle_screen_pokeswitch(uint8_t *hovered_pokeswitch, uint8_t *battle_mode);

void display_roam_screen(uint8_t *timer, uint8_t *terrain, uint8_t terrain_width, uint8_t terrain_height, uint8_t tile_hwidth, uint8_t player_coordinates[2]);
void display_roam_screen_terrain(uint8_t *terrain, uint8_t terrain_width, uint8_t terrain_height, uint8_t tile_hwidth, uint8_t player_coordinates[2]);
void display_roam_screen_player(uint8_t tile_hwidth);
uint8_t resolve_terrain_action(uint8_t *terrain, uint8_t terrain_width, uint8_t terrain_height, uint8_t position[2]);
void calculate_roam_screen(uint8_t *terrain, uint8_t terrain_width, uint8_t terrain_height, uint8_t player_coordinates[2], uint8_t *battle_log_stage_length, uint8_t *battle_outcome_stage, uint8_t *battle_outcome_timer, uint8_t *hovered_battle_option, uint8_t *battle_screen_variant, uint8_t *battle_mode);
void switch_to_roam(uint8_t *battle_log_stage_length, uint8_t *battle_outcome_stage, uint8_t *battle_outcome_timer, uint8_t *hovered_battle_option, uint8_t *battle_screen_variant, uint8_t *battle_mode);
void switch_to_battle(uint8_t *battle_log_stage_length, uint8_t *battle_outcome_stage, uint8_t *battle_outcome_timer, uint8_t *hovered_battle_option, uint8_t *battle_screen_variant, uint8_t *battle_mode);
