#ifndef POKE_INFO_H
#define POKE_INFO_H

typedef struct {   // All information about a specific stored pokemon
  const char name[5];
  const uint8_t name_length;
  const uint8_t types[2];

  uint8_t level;
  uint8_t experience;     // Experience gain so far this level
  uint8_t experience_max; // Experience required for the current level

  uint8_t health;
  uint8_t health_max;
  uint8_t speed;

  // NOTE**; PP not implemented yet
  uint8_t moves[4];     // Index of moves owned by this pokemon -> found via a lookup with the index
  uint8_t action;       // Decision of pokemon in combat; for [0,4)=Index of the move chosen to be used by the pokemon, [4, 10)=Switch with another poke (value-4th index of poke), 6=...
  uint8_t is_critical;  // 0 => not critical, 1 => is critical, for this turn
  uint8_t is_miss;      // 0 => has NOT missed, 1 => has missed, for this turn

  uint8_t attack;
  uint8_t special_attack;

  uint8_t defence;
  uint8_t special_defence;
} poke_info;

#endif

#ifndef POKE_MOVE_H
#define POKE_MOVE_H

typedef struct {   // All information about a specific stored pokemon
  const char *name;   // **NOTE; Lots of runtime errors when trying this as 'const char name[XX]' due to initialisation problems -> will only work like this on the stack in my experience
  const uint8_t name_length;

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
  const char *name;
  const uint8_t name_length;
  const uint8_t id;     // Used to identify items
  //pass -> Maybe store pointer to function for action of the item?
} poke_item;

#endif

void concat_strings(char *buffer, uint8_t *buffer_length, const uint8_t str_number, const char *str_additions[], const uint8_t str_addition_lengths[]);
uint8_t sum_256(uint8_t *list, uint8_t list_length);
int8_t linear_to_oscil(uint8_t x);

void display_tree(int8_t origin_y, int8_t origin_x, uint8_t radius);
void display_grass(int8_t origin_y, int8_t origin_x, uint8_t radius);
void display_healstation(int8_t origin_y, int8_t origin_x, uint8_t radius);

void display_cube(int8_t origin_y, int8_t origin_x, uint8_t radius);
void display_rect(int8_t origin_y, int8_t origin_x, uint8_t hheight, uint8_t hwidth);

uint8_t fetch_move_effectiveness(const uint8_t type_effectiveness_lookup[324], uint8_t move_type, const uint8_t recipient_types[2]);
uint8_t fetch_move_stab(uint8_t move_type, const uint8_t poke_types[2]);

void display_battle_screen(poke_info ally_poke_party[6], uint8_t *active_ally_poke_index, poke_info hostile_poke_party[6], uint8_t *active_hostile_poke_index, const poke_move *poke_move_lookup, char *battle_log_complete, uint8_t *battle_log_stage_length, uint8_t hovered_battle_option, uint8_t *battle_screen_variant, uint8_t *battle_outcome_stage, uint8_t *battle_outcome_timer, uint8_t battle_mode, uint8_t *timer);

void display_battle_screen_pokemons(poke_info *poke_ally, poke_info *poke_hostile, uint8_t *timer);
void display_battle_screen_pokemon(poke_info *poke, int8_t poke_position_y, uint8_t poke_radius, const uint8_t *poke_stats_dimensions, const uint8_t isLeftSide, uint8_t *timer);
void display_battle_screen_pokemon_stats(poke_info *poke, int8_t position_y, const uint8_t isLeftSide, const uint8_t *poke_stats_dimensions);

void display_battle_screen_battle_options(uint8_t hovered_option, const uint8_t battle_log_extension, const uint8_t battle_options_hheight, uint8_t *timer);
void display_battle_screen_battle_options_fight(poke_info *poke, const poke_move *poke_move_lookup, uint8_t hovered_option, const uint8_t battle_log_extension, const uint8_t battle_options_hheight, uint8_t *timer);

void fetch_battle_screen_battle_log(const uint8_t type_effectiveness_lookup[324], char *battle_log_complete, uint8_t *battle_log_stage_length, const poke_move *poke_move_lookup, poke_info ally_poke_party[6], uint8_t *active_ally_poke_index, poke_info hostile_poke_party[6], uint8_t *active_hostile_poke_index, uint8_t *battle_outcome_stage, uint8_t *battle_outcome_timer);
uint8_t fetch_party_fainted_count();
char* fetch_battle_screen_battle_log_subset(char *battle_log_complete, uint8_t *battle_log_stage_length, uint8_t *battle_log_stage, uint8_t *battle_log_timer);
void display_battle_screen_battle_log(char *battle_log_complete, uint8_t *battle_log_stage_length, uint8_t *battle_outcome_stage, uint8_t *battle_outcome_timer, const uint8_t battle_log_extension, const uint8_t battle_options_hheight);

void fetch_battle_screen_counters(uint8_t *battle_outcome_stage, const poke_move *poke_move_lookup, poke_info ally_poke_party[6], uint8_t *active_ally_poke_index, poke_info hostile_poke_party[6], uint8_t *active_hostile_poke_index, int8_t *poke_ally_counter, int8_t *poke_hostile_counter, uint8_t *poke_counter_type, const uint8_t type_effectiveness_lookup[324]);
void fetch_poke_battle_damage(uint8_t is_A_attacking, const poke_move *poke_move_lookup, poke_info *poke_A, poke_info *poke_B, int8_t *poke_A_counter, int8_t *poke_B_counter, uint8_t *poke_counter_type, const uint8_t type_effectiveness_lookup[324]);
void fetch_poke_battle_experience(uint8_t is_A_fainting, poke_info *poke_A, poke_info *poke_B, int8_t *poke_A_counter, int8_t *poke_B_counter, uint8_t *poke_counter_type);
void set_battle_start_parameters(const uint8_t type_effectiveness_lookup[324], char *battle_log_complete, uint8_t *battle_log_stage_length, const poke_move *poke_move_lookup, poke_info ally_poke_party[6], uint8_t *active_ally_poke_index, poke_info hostile_poke_party[6], uint8_t *active_hostile_poke_index, uint8_t *hovered_battle_option, uint8_t *battle_screen_variant, uint8_t *battle_mode, uint8_t *battle_outcome_stage, uint8_t *battle_outcome_timer, int8_t *poke_ally_counter, int8_t *poke_hostile_counter, uint8_t *poke_counter_type);
uint8_t select_random_poke_move(uint8_t moves[4]);

void calculate_battle_screen(const uint8_t type_effectiveness_lookup[324], char *battle_log_complete, uint8_t *battle_log_stage_length, const poke_move *poke_move_lookup, poke_info ally_poke_party[6], uint8_t *active_ally_poke_index, poke_info hostile_poke_party[6], uint8_t *active_hostile_poke_index, uint8_t *hovered_battle_option, uint8_t *battle_screen_variant, uint8_t *battle_mode, uint8_t *battle_outcome_stage, uint8_t *battle_outcome_timer, int8_t *poke_ally_counter, int8_t *poke_hostile_counter, uint8_t *poke_counter_type);
void calculate_battle_screen_battle_timing(const uint8_t timer_speed, char *battle_log_complete, uint8_t *battle_log_stage_length, const poke_move *poke_move_lookup, poke_info ally_poke_party[6], uint8_t *active_ally_poke_index, poke_info hostile_poke_party[6], uint8_t *active_hostile_poke_index, uint8_t *hovered_battle_option, uint8_t *battle_mode, uint8_t *battle_screen_variant, uint8_t *battle_outcome_stage, uint8_t *battle_outcome_timer, int8_t *poke_ally_counter, int8_t *poke_hostile_counter, uint8_t *poke_counter_type, const uint8_t type_effectiveness_lookup[324]);
void calculate_battle_screen_buttons(const uint8_t type_effectiveness_lookup[324], char *battle_log_complete, uint8_t *battle_log_stage_length, const poke_move *poke_move_lookup, poke_info ally_poke_party[6], uint8_t *active_ally_poke_index, poke_info hostile_poke_party[6], uint8_t *active_hostile_poke_index, uint8_t *hovered_battle_option, uint8_t *battle_screen_variant, uint8_t *battle_mode, uint8_t *battle_outcome_stage, uint8_t *battle_outcome_timer, int8_t *poke_ally_counter, int8_t *poke_hostile_counter, uint8_t *poke_counter_type);

void display_battle_screen_bag(const poke_item *poke_item_lookup, uint8_t *poke_bag, uint8_t *poke_bag_size, uint8_t *hovered_bag_index, uint8_t *timer);
void display_battle_screen_bag_element(const poke_item *item, uint8_t poke_item_position_index, uint8_t *hovered_bag_index, uint8_t hheight, uint8_t hwidth);
void display_battle_screen_bag_element_closeup(const poke_item *hovered_item, uint8_t hheight, uint8_t hwidth);
void calculate_battle_screen_bag(const uint8_t type_effectiveness_lookup[324], const poke_move *poke_move_lookup, uint8_t *battle_mode, uint8_t *hovered_bag_index, uint8_t *poke_bag_size, uint8_t *battle_item_index, char *battle_log_complete, uint8_t *battle_log_stage_length, poke_info ally_poke_party[6], uint8_t *active_ally_poke_index, poke_info hostile_poke_party[6], uint8_t *active_hostile_poke_index, uint8_t *battle_screen_variant, uint8_t *battle_outcome_stage, uint8_t *battle_outcome_timer, int8_t *poke_ally_counter, int8_t *poke_hostile_counter, uint8_t *poke_counter_type);

void display_battle_screen_pokeswitch_screen(poke_info ally_poke_party[6], uint8_t *active_ally_poke_index, uint8_t hovered_pokeswitch, uint8_t timer);
void display_battle_screen_pokeswitch_poke_details(poke_info *poke, int8_t origin_y, int8_t origin_x, uint8_t screen_hheight, uint8_t screen_hwidth);
void calculate_battle_screen_pokeswitch(uint8_t *hovered_pokeswitch, const poke_move *poke_move_lookup, const uint8_t type_effectiveness_lookup[324], int8_t *poke_ally_counter, int8_t *poke_hostile_counter, uint8_t *poke_counter_type, poke_info ally_poke_party[6], uint8_t *active_ally_poke_index, poke_info hostile_poke_party[6], uint8_t *active_hostile_poke_index, char *battle_log_complete, uint8_t *battle_log_stage_length, uint8_t *battle_mode, uint8_t *battle_screen_variant, uint8_t *battle_outcome_stage, uint8_t *battle_outcome_timer);
uint8_t fetch_pokeswitch_valid(poke_info *poke_switch_target);

void display_roam_screen(uint8_t *timer, uint8_t *terrain, uint8_t terrain_width, uint8_t terrain_height, uint8_t tile_hwidth, uint8_t player_coordinates[2]);
void display_roam_screen_terrain(uint8_t *terrain, uint8_t terrain_width, uint8_t terrain_height, uint8_t tile_hwidth, uint8_t player_coordinates[2]);
void display_roam_screen_player(uint8_t tile_hwidth);
uint8_t resolve_terrain_action(uint8_t *terrain, uint8_t terrain_width, uint8_t terrain_height, uint8_t position[2]);
void resolve_terrain_display(uint8_t tile_type, int8_t draw_position_y, int8_t draw_position_x, uint8_t tile_hwidth);
void calculate_roam_screen(uint8_t *terrain, uint8_t terrain_width, uint8_t terrain_height, uint8_t player_coordinates[2], poke_info hostile_poke_party[6], uint8_t *active_hostile_poke_index, uint8_t *battle_log_stage_length, uint8_t *battle_outcome_stage, uint8_t *battle_outcome_timer, uint8_t *hovered_battle_option, uint8_t *battle_screen_variant, uint8_t *battle_mode);

void generate_pokemon(poke_info poke_party[6], uint8_t poke_index, uint8_t preset);

void switch_to_roam(uint8_t *battle_log_stage_length, uint8_t *battle_outcome_stage, uint8_t *battle_outcome_timer, uint8_t *hovered_battle_option, uint8_t *battle_screen_variant, uint8_t *battle_mode);
void switch_to_battle(poke_info hostile_poke_party[6], uint8_t *active_hostile_poke_index, uint8_t *battle_log_stage_length, uint8_t *battle_outcome_stage, uint8_t *battle_outcome_timer, uint8_t *hovered_battle_option, uint8_t *battle_screen_variant, uint8_t *battle_mode);
