void display_cube(int8_t origin_y, int8_t origin_x, uint8_t radius);
void display_rect(int8_t origin_y, int8_t origin_x, uint8_t hheight, uint8_t hwidth);

void display_battle_screen(uint8_t hovered_battle_option, uint8_t battle_mode, uint8_t timer);

void display_battle_screen_pokemons();
void display_battle_screen_pokemon(const int8_t poke_position_y, uint8_t poke_radius, const uint8_t *poke_stats_dimensions, const uint8_t isLeftSide);
void display_battle_screen_pokemon_stats(const int8_t position_y, const uint8_t isLeftSide, const uint8_t *poke_stats_dimensions);

void display_battle_screen_battle_options(uint8_t hovered_option, const uint8_t battle_log_extension, const uint8_t battle_options_hheight, uint8_t timer);
void display_battle_screen_battle_options_fight(uint8_t hovered_option, const uint8_t battle_log_extension, const uint8_t battle_options_hheight, uint8_t timer);

void display_battle_screen_battle_log(char* battle_text, const uint8_t battle_log_extension, const uint8_t battle_options_hheight);

void calculate_battle_screen(uint8_t *hovered_battle_option, uint8_t *battle_mode);
