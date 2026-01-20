/* Automated flag game module */

/* Written for RavenMUD by Imhotep */

#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/handler.h"
#include "general/class.h"
#include "actions/act.h"          /* ACMDs located within the act*.c files */
#include "general/comm.h"
#include "actions/interpreter.h"
#include "util/utils.h"
#include "specials/special.h"
#include "specials/flag_game.h"

/* The current state of the flag game system */

#define GAME_IDLE       0
#define GAME_PICKING    1
#define GAME_PLANNING   2
#define GAME_FIRSTHALF  3
#define GAME_HALFTIME   4
#define GAME_SECONDHALF 5
#define GAME_TIMEOUT    6

/* Data stored about each player */
typedef struct flag_player {
    int caps, kills, returns;   /* scorekeeping */
    CharData *ch;               /* the character */
    struct flag_player *next;   /* make it a linked list */
} FlagPlayer;

/* The structure of the teams */
typedef struct flag_team {
    int size;                   /* # of players */
    int timeouts;               /* # of timeouts remaining */
    FlagPlayer *players;        /* List of players in this team */
} FlagTeam;

/* Information about the game itself */
typedef struct flag_game {
    int state, counter;         /* The state and a tick counter */
    FlagTeam gold, black;       /* The two teams */
    FlagTeam rogue;             /* The rogue team */
    FlagPlayer *players;        /* List of players not in a team yet */
} FlagGame;

static FlagGame theGame = {
    0, 0, {0, 0, NULL}, {0, 0, NULL}, {0, 0, NULL}, NULL
};

/* free all memory used in a FlagPlayer list */
static void flag_free_list(FlagPlayer *list)
{
    FlagPlayer *tmp;

    while (list) {
        tmp = list->next;
        free(list);
        list = tmp;
    }
}

void flag_init(void)
{
    theGame.state = GAME_IDLE;
    theGame.gold.size = 0;
    theGame.black.size = 0;
    theGame.rogue.size = 0;
    flag_free_list(theGame.gold.players);
    flag_free_list(theGame.black.players);
    flag_free_list(theGame.rogue.players);
    flag_free_list(theGame.players);
    theGame.gold.players = theGame.black.players = theGame.rogue.players = NULL;
    theGame.players = NULL;
}

/* Remove a player from a FlagPlayer ** list, if it's in there
 * Returns 1 if the players was removed, 0 if it was not found */
static int player_from_list(FlagPlayer **list, CharData *player)
{
    FlagPlayer *tmp, *last = NULL;

    for (tmp = *list; tmp && tmp->ch != player; last = tmp, tmp = tmp->next);
    if (tmp) {
        if (last) last->next = tmp->next; else *list = tmp->next;
        free(tmp);
        return 1;
    }
    return 0;
}

/* Add a player to a FlagPlayer **list */
static void player_to_list(FlagPlayer **list, CharData *player)
{
    FlagPlayer *tmp = (FlagPlayer *)malloc(sizeof(FlagPlayer));

    tmp->ch = player;
    tmp->caps = tmp->kills = tmp->returns = 0;
    tmp->next = *list;
    *list = tmp;
}

void flag_player_from_game(CharData *ch)
{
    if (player_from_list(&theGame.players, ch)) {
    } else if (player_from_list(&theGame.gold.players, ch)) {
        theGame.gold.size--;
    } else if (player_from_list(&theGame.black.players, ch)) {
        theGame.black.size--;
    } else if (player_from_list(&theGame.rogue.players, ch)) {
        theGame.rogue.size--;
    }
}

void flag_player_into_game(CharData *ch)
{
    player_to_list(&theGame.players, ch);
}

void flag_player_into_team(CharData *ch, int team)
{
    /* jic ... */
    flag_player_from_game(ch);

    switch (team) {
        case FLAG_TEAM_GOLD:
            player_to_list(&theGame.gold.players, ch);
            theGame.gold.size++;
            break;
        case FLAG_TEAM_BLACK:
            player_to_list(&theGame.black.players, ch);
            theGame.black.size++;
            break;
        case FLAG_TEAM_ROGUE:
            player_to_list(&theGame.rogue.players, ch);
            theGame.rogue.size++;
            break;
        default:
            mudlog( BRF, LVL_IMMORT, FALSE, "Attempt to add player %s into non-existent team %d",
                    GET_NAME(ch), team);
            break;
    }
}

FlagPlayer *flag_find_player_in_list(FlagPlayer *list, CharData *ch)
{
    while (list) {
        if (list->ch == ch) return list;
        list = list->next;
    }

    return NULL;
}

static FlagPlayer *flag_find_player(CharData *ch)
{
    FlagPlayer *p;

    p = flag_find_player_in_list(theGame.gold.players, ch);
    if (!p) p = flag_find_player_in_list(theGame.black.players, ch);
    if (!p) p = flag_find_player_in_list(theGame.rogue.players, ch);

    return p;
}

void flag_player_killed(CharData *killer, CharData *victim)
{
    /* find the player */
    FlagPlayer *p = flag_find_player(killer);

    if (p) p->kills++;
}

void flag_player_capture(CharData *player)
{
    /* find the player */
    FlagPlayer *p = flag_find_player(player);

    if (p) p->caps++;
}

void flag_player_return(CharData *player)
{
    /* find the player */
    FlagPlayer *p = flag_find_player(player);

    if (p) p->returns++;
}

static void flag_insert_score(FlagPlayer **scores, int n, FlagPlayer *p)
{
    int i;

    /* locate insert position */
    for (i = 0; i < n && scores[i]->kills > p->kills; i++);

    /* see if the back end of the array needs to be moved */
    if (i < n) memmove(scores+i+1, scores+i, (n-i)*sizeof(FlagPlayer*));

    /* store the inserted player */
    scores[i] = p;
}

static char *flag_get_team_name(CharData *ch)
{
    static char *black = "&05(black)&00";
    static char *gold  = "&03(gold) &00";
    static char *rogue = "&08(rogue)&00";
    static char *none  = "             ";

    if (PRF_FLAGGED(ch, PRF_GOLD_TEAM)) return gold;
    if (PRF_FLAGGED(ch, PRF_BLACK_TEAM)) return black;
    if (PRF_FLAGGED(ch, PRF_ROGUE_TEAM)) return rogue;
    return none;
}

void flag_show_scores(void)
{
    FlagPlayer **scores, *p;
    int players = theGame.gold.size + theGame.black.size + theGame.rogue.size;
    int count = 0, gc = 0, bc = 0, gk = 0, bk = 0, gr = 0, br = 0;

    /* print out the headers */
    quest_echo("&14 Name                              ");
    quest_echo("&07|&14 Kills &07|&14 Caps  &07|&14 Rets&00\r\n");
    quest_echo("&07-----------------------------------");
    quest_echo("+-------+-------+-------&00\r\n");

    /* sort all players by #kills, via insert sort */
    /* (very small n suggests O(n^2) may be close to O(nlogn) here) */
    scores = (FlagPlayer **)malloc(players * sizeof(FlagPlayer *));

    for (p = theGame.gold.players; p; p = p->next)
        flag_insert_score(scores, count++, p);
    for (p = theGame.black.players; p; p = p->next)
        flag_insert_score(scores, count++, p);
    for (p = theGame.rogue.players; p; p = p->next)
        flag_insert_score(scores, count++, p);

    /* now dump that list of scores via quest_echo() */
    for (count = 0; count < players; count++) {
        char nbuf[MAX_NAME_LENGTH + 9];

        sprintf(nbuf, "%s %s", GET_NAME(scores[count]->ch),
                flag_get_team_name(scores[count]->ch));
        sprintf(buf, " %-40s&07|&00 %-3d   &07|&00 %-3d   &07|&00 %d\r\n",
                nbuf, scores[count]->kills, scores[count]->caps,
                scores[count]->returns);
        quest_echo(buf);
        if (PRF_FLAGGED(scores[count]->ch, PRF_GOLD_TEAM)) {
            gc += scores[count]->caps;
            gk += scores[count]->kills;
            gr += scores[count]->returns;
        } else if (PRF_FLAGGED(scores[count]->ch, PRF_BLACK_TEAM)) {
            bc += scores[count]->caps;
            bk += scores[count]->kills;
            br += scores[count]->returns;
        }
    }

    quest_echo("&07-----------------------------------");
    quest_echo("+-------+-------+-------&00\r\n");
    quest_echo("&03 Gold team total                   &07|&00 ");
    sprintf(buf, "%-3d   &07|&00 %-3d   &07|&00 %-3d\r\n", gk, gc, gr);
    quest_echo(buf);
    quest_echo("&05 Black team total                  &07|&00 ");
    sprintf(buf, "%-3d   &07|&00 %-3d   &07|&00 %-3d\r\n", bk, bc, br);
    quest_echo(buf);

    free(scores);
}

static void flag_award_qp(CharData *ch, FlagPlayer *team, int qp)
{
    char note[200];

    if (qp <= 0 || qp > 10) return;

    sprintf(note, "You are awarded %d quest points!\r\n", qp);
    while (team) {
        send_to_char(note, team->ch);
        sprintf(buf, "%s awarded %d quest points.\r\n", GET_NAME(team->ch), qp);
        send_to_char(buf, ch);
        GET_QP(team->ch) += qp;
        team = team->next;
    }
}

static void flag_show_usage(CharData *ch)
{
    send_to_char("Usage: flag { start | scores | award <team> <qp> }\r\n", ch);
}

ACMD(do_flag)
{
    char op[MAX_INPUT_LENGTH], team[MAX_INPUT_LENGTH], qp[MAX_INPUT_LENGTH];

    half_chop(argument, op, buf);
    if (is_abbrev(op, "start")) {
        flag_init();
        send_to_char("Okay.\r\n", ch);
    } else if (is_abbrev(op, "scores")) {
        send_to_char("Okay.\r\n", ch);
        flag_show_scores();
    } else if (is_abbrev(op, "award")) {
        half_chop(buf, team, buf);
        half_chop(buf, qp, buf);
        if (is_abbrev(team, "gold")) {
            flag_award_qp(ch, theGame.gold.players, atoi(qp));
        } else if (is_abbrev(team, "black")) {
            flag_award_qp(ch, theGame.black.players, atoi(qp));
        } else if (is_abbrev(team, "rogue")) {
            flag_award_qp(ch, theGame.rogue.players, atoi(qp));
        } else {
            flag_show_usage(ch);
            return;
        }
        send_to_char("Okay.\r\n", ch);
    } else {
        flag_show_usage(ch);
    }
}
 

// The following function is to test if a player is carrying
// a quest item, which will qualify them as "being hunted".

int carrying_quest_item(CharData *ch)
{
	ObjData *obj, *objnext = NULL;

	for( obj = ch->carrying; obj; obj = objnext ) {
		objnext = obj->next_content;
		if ( IS_SET_AR(GET_OBJ_EXTRA(obj), ITEM_ARENA) ) {
			return 1;
		}
	}  // for

	// No quest item, returning 0:
	return 0;
}
