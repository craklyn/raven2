/* Automated flag game module */

/* the teams */
#define FLAG_TEAM_BLACK         0
#define FLAG_TEAM_GOLD          1
#define FLAG_TEAM_ROGUE         2

/* reset the flag game status ready to play */
void flag_init(void);

/* Remove a player from the flag game */
void flag_player_from_game(CharData *ch);

/* Add a player into the game without a team */
void flag_player_into_game(CharData *ch);

/* Add a player into the game in a specific team */
void flag_player_into_team(CharData *ch, int team);

/* Note a player scoring a kill */
void flag_player_killed(CharData *killer, CharData *victim);

/* Note a player scoring a capture */
void flag_player_capture(CharData *player);

/* Note a player scoring a return */
void flag_player_return(CharData *player);

/* Note a player scoring a capture */
void flag_player_capture(CharData *player);

/* Show a table of current scores to all players with a QUEST flag */
void flag_show_scores(void);

extern int carrying_quest_item(CharData *ch);
