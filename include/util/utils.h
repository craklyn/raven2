/* ************************************************************************
*   File: utils.h                                       Part of CircleMUD *
*  Usage: header file: utility macros and prototypes of utility funcs     *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
#ifndef _UTILS_H_ /* Begin header file protection */
#define _UTILS_H_

#define ACMD(name)  \
   void name(struct char_data *ch, char *argument, int cmd, int subcmd)

/** direct all log() references to basic_mud_log() function. */
#define mlog			basic_mud_log

/** Standard line size, used for many string limits. */
#define READ_SIZE	256

/* external declarations and prototypes **********************************/

extern struct weather_data weather_info;
extern struct zone_data *zone_table;

/* public functions */
extern void   add_follower(CharData * ch, CharData * leader);
extern int    is_integer(const char *s1);
extern char  *str_dup(const char *source);
extern int attackers(CharData * ch);
/* Only provide our versions if one isn't in the C library. These macro names
 * will be defined by sysdep.h if a strcasecmp or stricmp exists. */
#ifndef str_cmp
extern int    str_cmp(char *arg1, char *arg2);
#endif
#ifndef strn_cmp
extern int    strn_cmp(char *arg1, char *arg2, int n);
#endif

extern void   basic_mud_log(const char *format, ...) __attribute__ ((format (printf, 1, 2)));
extern void   basic_mud_vlog(const char *format, va_list args);
extern int    touch(const char *path);
extern void   sendChar( CharData *ch, char *fmt, ... );
extern void   mudlog(int type, int level, int file, const char *str, ...) __attribute__ ((format (printf, 4, 5)));
extern void   jail_char(CharData *ch, int death_trap);
extern void   make_ghost(CharData *ch);
extern void   mob_talk(CharData *mob, CharData *target, int situation); 
extern int    number(int from, int to);
extern int    dice(int number, int size);
extern int    clamp( int value, int minimum, int maximum );
extern int    percentSuccess( int percentage );
extern void   sprintbit(u_int vektor, char *names[], char *result);
size_t	sprinttype(int type, const char *names[], char *result, size_t reslen);
void sprintbitarray(int bitvector[], char *names[], int maxar, char *result);
extern int    get_line(FILE *fl, char *buf);
extern int    get_filename(char *orig_name, char *filename, int mode);
extern struct time_info_data age(CharData *ch);
extern int    find_first_step(sh_int src, sh_int target);
extern int    replace_str(char **string, char *pattern, char *replacement, int rep_all, int max_size);
extern char  *stripcr(char *dest, const char *src);
char *strlower(char *string);
extern void   gecho(char *fmt, ...);
extern void  quest_echo(char *msg);
extern char  *ndth(int val);
extern CharData *random_victim(int roomnr);
extern int   in_same_group(CharData *ch, CharData *tch);
extern void core_dump_real(const char *who, int line);
int count_color_chars(char *string);
int levenshtein_distance(const char *s1, const char *s2);
extern int file_head( FILE *file, char *buf, size_t bufsize, int lines_to_read );
extern int file_tail( FILE *file, char *buf, size_t bufsize, int lines_to_read );
extern size_t file_sizeof( FILE *file );
extern int file_numlines( FILE *file );
void obj_corpse_save(ObjData *corpse, CharData *ch);

/** Creates a core dump for diagnostic purposes, but will keep (if it can)
 * the mud running after the core has been dumped. Call this in the place
 * of calling core_dump_real. */
#define core_dump() core_dump_real(__FILE__, __LINE__)


#ifndef MAX
int	MAX(int a, int b);
#endif

#ifndef MIN
int	MIN(int a, int b);
#endif

/* in magic.c */
bool    circle_follow(CharData *ch, CharData *victim);


/* Followers */
void	die_follower(CharData *ch);
void	add_follower(CharData *ch, CharData *leader);
void	stop_follower(CharData *ch);
bool	circle_follow(CharData *ch, CharData *victim);

/* in act.informative.c */
void	look_at_room(CharData *ch, int mode);

/* in act.movmement.c */
int	do_simple_move(CharData *ch, int dir, int following);
int	perform_move(CharData *ch, int dir, int following);

/* From mudlimits.c */
int     mana_limit(CharData *ch);
int     hit_limit(CharData *ch);
int     move_limit(CharData *ch);
int     mana_gain(CharData *ch);
int     hit_gain(CharData *ch);
int     move_gain(CharData *ch);
void	advance_level(CharData *ch);
void	set_title(CharData *ch, char *title);
void	gain_exp(CharData *ch, int gain);
void	gain_exp_regardless(CharData *ch, int gain);
void    gain_exp_unchecked(CharData *ch, int gain);
void	gain_condition(CharData *ch, int condition, int value);
void	check_idling(CharData *ch);
void	point_update(void);
void	update_pos(CharData *victim);
void    pulse_heal(void);
void    regen_update( void );
int calcHitBase(CharData *ch);
int calcHitMulti(CharData *ch);
int calcHitBonus(CharData *ch);
int calcManaBase(CharData *ch);
int calcManaMulti(CharData *ch);
int calcManaBonus(CharData *ch);
int calcMoveBase(CharData *ch);
int calcMoveMulti(CharData *ch);
int calcMoveBonus(CharData *ch);

int     GET_DAMROLL(CharData *ch);
void    SET_DAMROLL(CharData *ch, int dam);

/* various constants *****************************************************/
//
// Player/Mob stunning macros go here.
//
#define CLAMP_VALUE( min, val, max ) \
  (( val < min ) ? min : (( val > max ) ? max : val ))

#define STUNNED(ch)     ((ch)->player.stunned > 0 ? 1 : 0)
#define STUN_SET(ch,sc) ((ch)->player.stunned = PULSE_VIOLENCE * sc )
#define STUN_DEC(ch)    \
  { if((ch)->player.stunned > 0) (ch)->player.stunned -= 1; \
    if((ch)->player.stunned > 0 && IS_DARK_PRIEST((ch)) && GET_ADVANCE_LEVEL((ch)) >= 5 && percentSuccess(20)) (ch)->player.stunned -= 1;}

/*
** STUN durations for skills/spells
*/
#define STUN_USE_BASE              (PULSE_VIOLENCE*2)

#define ARENA_DLY(ch)              ((ch)->player.arenaDly)

/*
** Flee based attacks and other such stuff goes here
*/
#define FLEEING(ch)     ((ch)->flee_timer > 0 ? 1 : 0)
#define FLEEING_SET(ch,sc) ((ch)->flee_timer = PULSE_VIOLENCE * sc )
#define FLEEING_DEC(ch)    \
  { if((ch)->flee_timer > 0) (ch)->flee_timer -= 1; }

/*
 * Sacrifice macro for necromancers who sacrifice pet
 */
#define SACRIFICE(ch)   ((ch)->sacrifice)

/*
 * Herb count to keep track of how many poultices a ranger may use.  Most rangers
 * get 3.  Naturalists can have 4-7 depending on their advance level.
 */
#define HAS_HERBS(ch)   ((ch)->herbs)
#define MAX_HERBS(ch)   3 + (IS_NATURALIST(ch)? (1 + GET_ADVANCE_LEVEL(ch)/3):0)

/*
** Defines that are useful, in particular, for PvP fights
*/

#define IS_PVP(ch, vict)  \
(FIGHTING((ch)) ? \
(((IS_NPC((ch)) && (ch)->master && !IS_NPC((ch)->master) && !IS_NPC((vict))) || \
(!(IS_NPC((ch)) || IS_NPC((vict))) ) ) ? 1 : 0) : 0)
// Sorry for that shit you see above - Craklyn

/*
** Defines that are useful, in particular, for player stealing
*/

// Carrying a quest item also makes a player hunted
// Only players can be hunted
#define IS_HUNTED(ch)	(  (carrying_quest_item((ch)) ||                    \
                            IS_SET_AR(PLR_FLAGS((ch)), PLR_HUNTED) ||       \
                            IS_SET_AR(PLR_FLAGS((ch)), PLR_THIEF)     )  && \
                            !IS_NPC((ch)) )

/* defines for mudlog() */
#define OFF     0
#define BRF     1
#define NRM     2
#define CMP     3

/* get_filename() */
#define CRASH_FILE      0
#define ETEXT_FILE      1
#define SCRIPT_VARS_FILE 2
#define QUESTS_FILE     3

/* breadth-first searching */
#define BFS_ERROR               -1
#define BFS_ALREADY_THERE       -2
#define BFS_NO_PATH             -3

/* mud-life time */
#define SECS_PER_MUD_HOUR       75
#define SECS_PER_MUD_DAY        (24*SECS_PER_MUD_HOUR)
#define SECS_PER_MUD_MONTH      (35*SECS_PER_MUD_DAY)
#define SECS_PER_MUD_YEAR       (16*SECS_PER_MUD_MONTH)

/* real-life time (remember Real Life?) */
#define SECS_PER_REAL_MIN       60
#define SECS_PER_REAL_HOUR      (60*SECS_PER_REAL_MIN)
#define SECS_PER_REAL_DAY       (24*SECS_PER_REAL_HOUR)
#define SECS_PER_REAL_YEAR      (365*SECS_PER_REAL_DAY)

#define TICKS_PER_MUD_DAY       24
#define DAYS_PER_MUD_MONTH          (7*5)  // 7 days per week, 5 weeks per month
#define TICKS_PER_MUD_MONTH     (DAYS_PER_MUD_MONTH * TICKS_PER_MUD_DAY)
#define MONTHS_PER_MUD_YEAR     16
#define TICKS_PER_MUD_YEAR      (MONTHS_PER_MUD_YEAR*TICKS_PER_MUD_MONTH)

#define TICKS_SO_FAR            (time_info.year*TICKS_PER_MUD_YEAR + \
                                 time_info.month*TICKS_PER_MUD_MONTH + \
                              time_info.day*TICKS_PER_MUD_DAY + time_info.hours)


/* string utils **********************************************************/

#define ATTR_STR( maxAttr, thisAttr, listAttr ) listAttr[( thisAttr > maxAttr ? maxAttr : thisAttr )]

#define YESNO(a) ((a) ? "YES" : "NO")
#define ONOFF(a) ((a) ? "ON" : "OFF")

#define LOWER(c)   (((c)>='A'  && (c) <= 'Z') ? ((c)+('a'-'A')) : (c))
#define UPPER(c)   (((c)>='a'  && (c) <= 'z') ? ((c)+('A'-'a')) : (c) )

#define ISNEWL(ch) ((ch) == '\n' || (ch) == '\r') 
#define IF_STR(st) ((st) ? (st) : "\0")
#define CAP(st)  (*(st) = UPPER(*(st)), st)
#define UNCAP(st)  (*(st) = LOWER(*(st)), st)

/** If string begins a vowel (upper or lower case), return "an"; else return
 * "a". */
#define AN(string) (strchr("aeiouAEIOU", *string) ? "an" : "a")

/** A calloc based memory allocation macro. Hoping this might point out a few
 * memory allocation problems we have here on RavenMUD.
 * @param result Pointer to created memory.
 * @param type The type of memory (int, struct char_data, etc.).
 * @param number How many of type to make. */
#define CREATE(result, type, number)  do {\
	if ((number) * sizeof(type) <= 0)	\
		mlog("SYSERR: Zero bytes or less requested at %s:%d.", __FILE__, __LINE__);	\
	if (!((result) = (type *) calloc ((number), sizeof(type))))	\
		{ perror("SYSERR: malloc failure"); abort(); } } while(0)

/** A realloc based memory reallocation macro. Reminder: realloc can reduce
 * the size of an array as well as increase it.
 * @param result Pointer to created memory.
 * @param type The type of memory (int, struct char_data, etc.).
 * @param number How many of type to make. */
#define RECREATE(result, type, number) do {\
  if (!((result) = (type *) realloc ((result), sizeof(type) * (number))))\
		{ perror("SYSERR: realloc failure"); abort(); } } while(0)

/** Remove an item from a linked list and reset the links.
 * If item is at the list head, change the head, else traverse the
 * list looking for the item before the one to be removed.
 * @pre Requires that a variable 'temp' be declared as the same type as the
 * list to be manipulated.
 * @post List pointers are correctly reset and item is no longer in the list.
 * item can now be changed, removed, etc independently from the list it was in.
 * @param item Pointer to item to remove from the list.
 * @param head Pointer to the head of the linked list.
 * @param next The variable name pointing to the next in the list.
 * */
#define REMOVE_FROM_LIST(item, head, next)	\
   if ((item) == (head))		\
      head = (item)->next;		\
   else {				\
      temp = head;			\
      while (temp && (temp->next != (item))) \
	 temp = temp->next;		\
      if (temp)				\
         temp->next = (item)->next;	\
   }					\

/* basic bitvector utils */
/** Return the bitarray field number x is in. */
#define Q_FIELD(x)  ((int) (x) / 32)
/** Return the bit to set in a bitarray field. */
#define Q_BIT(x)    (1 << ((x) % 32))
/** 1 if bit is set in the bitarray represented by var, 0 if not. */
#define IS_SET_AR(var, bit)       ((var)[Q_FIELD(bit)] & Q_BIT(bit))
/** Set a specific bit in the bitarray represented by var to 1. */
#define SET_BIT_AR(var, bit)      ((var)[Q_FIELD(bit)] |= Q_BIT(bit))
/** Unset a specific bit in the bitarray represented by var to 0. */
#define REMOVE_BIT_AR(var, bit)   ((var)[Q_FIELD(bit)] &= ~Q_BIT(bit))
/** If bit is on in bitarray var, turn it off; if it is off, turn it on. */
#define TOGGLE_BIT_AR(var, bit)   ((var)[Q_FIELD(bit)] = (var)[Q_FIELD(bit)] ^ Q_BIT(bit))

/* Accessing player specific data structures on a mobile is a very bad thing
 * to do.  Consider that changing these variables for a single mob will change
 * it for every other single mob in the game.  If we didn't specifically check
 * for it, 'wimpy' would be an extremely bad thing for a mob to do, as an
 * example.  If you really couldn't care less, change this to a '#if 0'. */
#if 1
/** Warn if accessing player_specials on a mob.
 * @todo Subtle bug in the var reporting, but works well for now. */
#define CHECK_PLAYER_SPECIAL(ch, var) \
	(*(((ch)->player_specials == &dummy_mob) ? (mlog("SYSERR: Mob using '"#var"' at %s:%d.", __FILE__, __LINE__), &(var)) : &(var)))
#else
#define CHECK_PLAYER_SPECIAL(ch, var)	(var)
#endif

#define IS_NPC(ch)  (IS_SET_AR(MOB_FLAGS(ch), MOB_ISNPC))
#define IS_CLONE(ch)	(IS_SET_AR(MOB_FLAGS(ch), MOB_CLONE))

#define MOB_FLAGGED(ch, flag) ( IS_NPC(ch) && IS_SET_AR(MOB_FLAGS(ch), (flag)))
#define PLR_FLAGGED(ch, flag) (!IS_NPC(ch) && IS_SET_AR(PLR_FLAGS(ch), (flag)))
#define AFF_FLAGGED(ch, flag) (IS_SET_AR(AFF_FLAGS(ch), (flag)))
#define PRF_FLAGGED(ch, flag) (IS_SET_AR(PRF_FLAGS(ch), (flag)))
#define ROOM_FLAGGED(loc, flag) (IS_SET_AR(ROOM_FLAGS(loc), (flag)))
#define ZONE_FLAGGED(loc, flag)  (IS_SET(ZONE_FLAGS(loc), (flag)))
/** 1 if flag is set in the exit, 0 if not. */
#define EXIT_FLAGGED(exit, flag) (IS_SET((exit)->exit_info, (flag)))
/** 1 if flag is set in the affects bitarray of obj, 0 if not. */
#define OBJAFF_FLAGGED(obj, flag) (IS_SET_AR(GET_OBJ_AFFECT(obj), (flag)))
/** 1 if flag is set in the element of obj value, 0 if not. */
#define OBJVAL_FLAGGED(obj, flag) (IS_SET(GET_OBJ_VAL((obj), 1), (flag)))
/** 1 if flag is set in the wear bits of obj, 0 if not. */
#define OBJWEAR_FLAGGED(obj, flag) (IS_SET_AR(GET_OBJ_WEAR(obj), (flag)))
/** 1 if flag is set in the extra bits of obj, 0 if not. */
#define OBJ_FLAGGED(obj, flag) (IS_SET_AR(GET_OBJ_EXTRA(obj), (flag)))

#define EXPLORE_LEVEL(ch)   ((ch)->player_specials->saved.chore_count)
#define PLR_IS_EXPLORER(ch) (EXPLORE_LEVEL((ch)) >= 5)
#define PLR_IS_VETERAN(ch)  (EXPLORE_LEVEL((ch)) >= 10)
#define PLR_IS_LEGEND(ch)   (EXPLORE_LEVEL((ch)) >= 15)

// Generic consolidated flag tests.
//
#define GET_TRAP(item)        ((item)->traplevel)
#define GET_CONTAINER(item)   ((item)->container)

#define MOB_IS_AGGR_EVIL(mob) (MOB_FLAGGED(mob,MOB_AGGR_EVIL))
#define MOB_IS_AGGR_GOOD(mob) (MOB_FLAGGED(mob,MOB_AGGR_GOOD))
#define MOB_IS_AGGR_NEUT(mob) (MOB_FLAGGED(mob,MOB_AGGR_NEUTRAL))
#define MOB_IS_AGGR(mob)      (MOB_FLAGGED(mob,MOB_AGGRESSIVE))

#define PLR_TOG_CHK(ch,flag) ((TOGGLE_BIT_AR(PLR_FLAGS(ch), (flag))) & Q_BIT(flag))
#define PRF_TOG_CHK(ch,flag) ((TOGGLE_BIT_AR(PRF_FLAGS(ch), (flag))) & Q_BIT(flag))

#define IS_OBJ_STAT(obj,stat)   (IS_SET_AR((obj)->obj_flags.extra_flags, \
                                 (stat)))

#define CAN_WEAR(obj, part) (IS_SET_AR((obj)->obj_flags.wear_flags, (part)))

#define GET_OBJ_EXTRA_AR(obj, i)  ((obj)->obj_flags.extra_flags[(i)])
#define GET_OBJ_WEAR_AR(obj, i)   ((obj)->obj_flags.wear_flags[(i)])

#define GET_OBJ_AFFECT(obj)    ((obj)->obj_flags.bitvector)
#define GET_OBJ_AFFECT_AR(obj, i) ((obj)->obj_flags.bitvector[(i)])

#define MOB_FLAGS_AR(ch, int) ((ch)->char_specials.saved.act[(int)])
#define AFF_FLAGS_AR(ch, int) ((ch)->char_specials.saved.affected_by[(int)])


#define IS_SET(flag,bit)  ((flag) & (bit))
#define BIT_IS_SET(bit,mask) ((1<<bit) & mask)
#define SET_BIT(var,bit)  ((var) |= (bit))
#define REMOVE_BIT(var,bit)  ((var) &= ~(bit))
#define TOGGLE_BIT(var,bit) ((var) = (var) ^ (bit))

#define MOB_FLAGS(ch)   ((ch)->char_specials.saved.act)
#define PLR_FLAGS(ch)   ((ch)->char_specials.saved.act)
#define PRF_FLAGS(ch)   ((ch)->player_specials->saved.pref)
#define AFF_FLAGS(ch)   ((ch)->char_specials.saved.affected_by)
#define ROOM_FLAGS(loc) (world[(loc)].room_flags)
#define ZONE_FLAGS(loc) (zone_table[(loc)].zone_flags)

#define IS_MOB(ch)   (IS_NPC(ch) && ((ch)->nr >-1))
#define PC_CONTROLLED(ch) (IS_NPC(ch) && \
			   IS_SET_AR(AFF_FLAGS(ch), AFF_CHARM) && \
			   ch->master && \
			   !IS_NPC(ch->master))
#define GET_EQ(ch, i)           ((ch)->equipment[i])
#define WATER_COUNTER(ch)	((ch)->char_specials.water_counter)
#define UNDERWATER(ch) 		(SECT(IN_ROOM(ch)) == SECT_UNDERWATER)
#define SECT(room)      	(world[(room)].sector_type)
#define IS_CONTAINER(obj)       (GET_OBJ_TYPE(obj) == ITEM_CONTAINER)
#define IS_LOCKER(obj)          (GET_OBJ_TYPE(obj) == ITEM_CONTAINER && \
                                 IS_SET(GET_OBJ_VAL(obj, 1), CONT_LOCKER))

#define WIELDING(ch) ( !affected_by_spell(ch, SKILL_DISARM) ? ch->equipment[WEAR_WIELD] : NULL)
#define SHIELDED(ch) (ch->equipment[WEAR_SHIELD])

#define IS_AFFECTED(ch, skill) (AFF_FLAGGED((ch), (skill)) || \
                                has_native_aff(ch, skill))

// Generic affect testing.
//
#define IS_FLYING(ch)         (AFF_FLAGGED(ch,AFF_FLY))
#define IS_NOT_FLYING(ch)     (!IS_FLYING(ch))
#define IS_CHARMED(ch)        (AFF_FLAGGED(ch,AFF_CHARM))
#define IS_NOT_CHARMED(ch)    (!IS_CHARMED(ch))

/* room utils ************************************************************/


#define IS_DARK(room)  ( !world[room].light && \
                         (ROOM_FLAGGED(room, ROOM_DARK) || \
                          ( ( world[room].sector_type != SECT_INSIDE && \
                              world[room].sector_type != SECT_CITY ) && \
                             (weather_info.sunlight == SUN_SET || \
                              weather_info.sunlight == SUN_DARK)) ) )

#define IS_SUNLIGHT(room)  ( !ROOM_FLAGGED(room, ROOM_DARK) && \
                             world[room].sector_type != SECT_INSIDE && \
                             !ROOM_FLAGGED(room, ROOM_INDOORS) && \
                             weather_info.sunlight != SUN_RISE && \
                             weather_info.sunlight != SUN_SET && \
                             weather_info.sunlight != SUN_DARK )

#define NOT_SUNLIGHT(room) (!IS_SUNLIGHT(room))

#define IS_LIGHT(room)  (!IS_DARK(room))

/** 1 if this is a valid room number, 0 if not. */
#define VALID_ROOM_RNUM(rnum)	((rnum) != NOWHERE && (rnum) <= top_of_world)
/** The room number if this is a valid room, NOWHERE if it is not */
#define GET_ROOM_VNUM(rnum) \
	((room_vnum)(VALID_ROOM_RNUM(rnum) ? world[(rnum)].number : NOWHERE))
/** Pointer to the room function, NULL if there is not one. */
#define GET_ROOM_SPEC(room) ((room) >= 0 ? world[(room)].func : NULL)

/* Flag Game utils *******************************************************/
#define IN_QUEST_FIELD(ch) (zone_table[ world[ (ch)->in_room ].zone ].pkill_room)
#define JAIL_TIME 2
#define GOLD_TEAM_JAIL 20198
#define GOLD_TEAM_START_ROOM 20075
#define GOLD_TEAM_STANDARD 20001
#define GOLD_STANDARD_START_ROOM 20095
#define GOLD_TEAM_KEY 20000
#define BLACK_TEAM_JAIL 20100
#define BLACK_TEAM_START_ROOM 20175
#define BLACK_TEAM_STANDARD 20101
#define BLACK_STANDARD_START_ROOM 20195
#define BLACK_TEAM_KEY 20100
#define ROGUE_TEAM_JAIL 20197
#define ROGUE_TEAM_START_ROOM 20098

#define IS_FLAG_STANDARD(o) ((GET_OBJ_VNUM(o) == GOLD_TEAM_STANDARD) || (GET_OBJ_VNUM(o) == BLACK_TEAM_STANDARD))
#define IS_FLAG_KEY(o) ((GET_OBJ_VNUM(o) == GOLD_TEAM_KEY) || (GET_OBJ_VNUM(o) == BLACK_TEAM_KEY))
#define IS_FLAG_ITEM(o) (IS_FLAG_STANDARD(o) ||  IS_FLAG_KEY(o))

/* char utils ************************************************************/

#define IN_ROOM(ch)	((ch)->in_room)
#define IN_ARENA(ch)    (zone_table[ world[ (ch)->in_room ].zone ].tournament_room != 0)
#define ZONE_NAME(ch)   (zone_table[ world[ (ch)->in_room ].zone ].name)
#define ARENA_RM(ch)    (zone_table[ world[ (ch)->in_room ].zone ].tournament_room)
#define ZKILL_CNT(ch)   (zone_table[ world[ (ch)->in_room ].zone ].kill_count+=1)
#define ZMIN_LVL(ch)    (zone_table[ world[ (ch)->in_room ].zone ].min_level)
#define ZMAX_LVL(ch)    (zone_table[ world[ (ch)->in_room ].zone ].max_level)
#define GET_WAS_IN(ch)	((ch)->was_in_room)
#define GET_AGE(ch)     (age(ch).year)

// IN_NECROPOLIS if they're in zone 439, Necropolis
#define IN_NECROPOLIS(ch) ((ch)->in_room != -1 && world[(ch)->in_room].number/100 == 439)

/** Name of PC. */
#define GET_PC_NAME(ch)	((ch)->player.name)
/** Name of PC or short_descr of NPC. */
#define GET_NAME(ch)            (IS_NPC(ch) ? \
                                (ch)->player.short_descr : GET_PC_NAME(ch))

//macro used in GET_SEE(). Recalcs switched character lvl.
#define GET_REAL_LEVEL(ch) \
   (ch->desc && ch->desc->original ? GET_LEVEL(ch->desc->original) : \
    GET_LEVEL(ch))

#define GET_TITLE(ch)           ((ch)->player.title)
#define GET_LEVEL(ch)           ((ch)->player.level)
#define GET_LOST_LEVELS(ch)     ((ch)->player.lostlevels)
#define GET_HEIGHT(ch)	        ((ch)->player.height)
#define GET_WEIGHT(ch)	        ((ch)->player.weight)
#define GET_SEX(ch)             ((ch)->player.sex)
#define GET_ORCS(ch)            ((ch)->player.orcs)

#define GET_STR(ch)     ((ch)->aff_abils.str)
#define GET_ADD(ch)     ((ch)->aff_abils.str_add)
#define GET_DEX(ch)     ((ch)->aff_abils.dex)
#define GET_INT(ch)     ((ch)->aff_abils.intel)
#define GET_WIS(ch)     ((ch)->aff_abils.wis)
#define GET_CON(ch)     ((ch)->aff_abils.con)
#define GET_CHA(ch)     ((ch)->aff_abils.cha)

#define GET_EXP(ch)       ((ch)->points.exp)
#define GET_AC(ch)        ((ch)->points.armor)
#define GET_HIT(ch)       ((ch)->points.hit)
#define GET_MAX_HIT(ch)	  ((ch)->points.max_hit)
#define GET_MOVE(ch)	  ((ch)->points.move)
#define GET_MAX_MOVE(ch)  ((ch)->points.max_move)
#define GET_MANA(ch)	  ((ch)->points.mana)
#define GET_MAX_MANA(ch)  ((ch)->points.max_mana)
#define GET_GOLD(ch)	  ((ch)->points.gold)
#define GET_BANK_GOLD(ch) ((ch)->points.bank_gold)
#define GET_HITROLL(ch)	  ((ch)->points.hitroll)

#define GET_PERCENT_HIT(ch) = GET_HIT(ch)/GET_MAX_HIT(ch)

#define GET_POS(ch)       ((ch)->char_specials.position)
#define GET_IDNUM(ch)	  ((ch)->char_specials.saved.idnum)
#define GET_ID(ch)        ((ch)->id)
#define IS_CARRYING_W(ch) ((ch)->char_specials.carry_weight)
#define IS_CARRYING_N(ch) ((ch)->char_specials.carry_items)
#define FIGHTING(ch)	  ((ch)->char_specials.fighting)
#define HUNTING(ch)       ((ch)->char_specials.hunting)
#define HAS_FEIGNED(ch)   ((ch)->char_specials.feign_flag)
#define GET_SAVE(ch, i)	  ((ch)->char_specials.saved.apply_saving_throw[i])
#define GET_ALIGNMENT(ch) ((ch)->char_specials.saved.alignment)
#define GET_RECALL(ch)    ((ch)->player_specials->saved.recall)

#define SINGING(ch)       ((ch)->singing)
#define OFF_BALANCE(ch)   ((ch)->offbalance)
#define GET_VOTE(ch)	  ((ch)->vote)
#define SONG_FLAGS(ch, f) (spell_info[(ch)->singing].routines & f)

#define GET_MOB_SKILL_SUC(ch)   ((ch)->mobskill_suc)
#define GET_MOB_SPELL_DAM(ch)   ((ch)->mobspell_dam)

#define GET_COND(ch, i)		((ch)->player_specials->saved.conditions[(i)])
#define GET_LOADROOM(ch)	((ch)->player_specials->saved.load_room)
#define GET_PRACTICES(ch)	((ch)->player_specials->saved.spells_to_learn)
#define GET_INVIS_LEV(ch)	((ch)->player_specials->saved.invis_level)
#define GET_WIMP_LEV(ch)	((ch)->player_specials->saved.wimp_level)
#define GET_FREEZE_LEV(ch)	((ch)->player_specials->saved.freeze_level)
#define GET_BAD_PWS(ch)		((ch)->player_specials->saved.bad_pws)
#define GET_CLAN(ch)            ((ch)->player_specials->saved.clan_id)
#define GET_RECALL(ch)          ((ch)->player_specials->saved.recall)
#define GET_CLAN_RANK(ch)       ((ch)->player_specials->saved.clan_rank)
#define GET_PKILL_CNT(ch)       ((ch)->player_specials->saved.pkill_countdown)
#define GET_PTHIEF_CNT(ch)      ((ch)->player_specials->saved.pthief_countdown)
#define GET_PCOMBAT_CNT(ch)     ((ch)->player_specials->saved.combat_delay)
#define GET_CONJ_CNT(ch, n)     ((ch)->player_specials->saved.conj_countdown[n])
#define SET_CONJ_CNT(ch, n)     (ch)->player_specials->saved.conj_countdown[n]
#define GET_ARENA_RANK(ch)      ((ch)->player_specials->saved.arenarank)
#define POOFIN(ch)              ((ch)->player_specials->poofin)
#define POOFOUT(ch)             ((ch)->player_specials->poofout)
#define GET_ALIASES(ch)		((ch)->player_specials->aliases)
#define GET_LAST_TELL(ch)	((ch)->player_specials->last_tell)
#define GET_QP(ch)              ((ch)->player_specials->saved.quest_pts)
#define COOLDOWN(ch, slot)      ((ch)->player_specials->saved.cooldown[(slot)])
#define GET_LOCKER(ch)          ((ch)->player_specials->saved.locker_num)
#define GET_ASSIST_HP(ch)       ((ch)->player_specials->saved.assistant)
#define GET_MASTERY(ch)         ((ch)->player_specials->saved.mastery)

#define UNDEFINED_MASTERY           0
#define SKILL_EDGE_MASTERY          1
#define SKILL_POINT_MASTERY         2
#define SKILL_BLUNT_MASTERY         3
#define SKILL_EXOTIC_MASTERY        4

// Advance & specialization shit.
#define GET_ADVANCE_LEVEL(ch)   ((ch)->player_specials->saved.advance_level)
#define GET_SPEC(ch)            ((ch)->player_specials->saved.specialization)
// DK
#define IS_KNIGHT_ERRANT(ch)    (GET_CLASS(ch) == CLASS_DEATH_KNIGHT && GET_SPEC(ch) == SPEC_KNIGHT_ERRANT)
#define IS_DEFILER(ch)          (GET_CLASS(ch) == CLASS_DEATH_KNIGHT && GET_SPEC(ch) == SPEC_DEFILER)
// SK
#define IS_DRAGOON(ch)		(GET_CLASS(ch) == CLASS_SOLAMNIC_KNIGHT && GET_SPEC(ch) == SPEC_DRAGOON)
#define IS_KNIGHT_TEMPLAR(ch)	(GET_CLASS(ch) == CLASS_SOLAMNIC_KNIGHT && GET_SPEC(ch) == SPEC_KNIGHT_TEMPLAR)
// Mage
#define IS_ARCANIST(ch)		(GET_CLASS(ch) == CLASS_MAGIC_USER && GET_SPEC(ch) == SPEC_ARCANIST)
#define IS_ENCHANTER(ch)	(GET_CLASS(ch) == CLASS_MAGIC_USER && GET_SPEC(ch) == SPEC_ENCHANTER)
// SD
#define IS_PRESTIDIGITATOR(ch)  (GET_CLASS(ch) == CLASS_SHADOW_DANCER && GET_SPEC(ch) == SPEC_PRESTIDIGITATOR)
#define IS_SHADE(ch)            (GET_CLASS(ch) == CLASS_SHADOW_DANCER && GET_SPEC(ch) == SPEC_SHADE)
// Assassin
#define IS_BOUNTY_HUNTER(ch)    (GET_CLASS(ch) == CLASS_ASSASSIN && GET_SPEC(ch) == SPEC_BOUNTY_HUNTER)
#define IS_BUTCHER(ch)		(GET_CLASS(ch) == CLASS_ASSASSIN && GET_SPEC(ch) == SPEC_BUTCHER)
// Thief
#define IS_TRICKSTER(ch)	(GET_CLASS(ch) == CLASS_THIEF && GET_SPEC(ch) == SPEC_TRICKSTER)
#define IS_BRIGAND(ch)	        (GET_CLASS(ch) == CLASS_THIEF && GET_SPEC(ch) == SPEC_BRIGAND)
// Cleric
#define IS_DARK_PRIEST(ch)	(GET_CLASS(ch) == CLASS_CLERIC && GET_SPEC(ch) == SPEC_DARK_PRIEST)
#define IS_HOLY_PRIEST(ch)      (GET_CLASS(ch) == CLASS_CLERIC && GET_SPEC(ch) == SPEC_HOLY_PRIEST)
// Warrior
#define IS_DEFENDER(ch)		(GET_CLASS(ch) == CLASS_WARRIOR && GET_SPEC(ch) == SPEC_DEFENDER)
#define IS_DRAGONSLAYER(ch)	(GET_CLASS(ch) == CLASS_WARRIOR && GET_SPEC(ch) == SPEC_DRAGON_SLAYER)
// Shou Lin
#define IS_CHI_WARRIOR(ch)	(GET_CLASS(ch) == CLASS_SHOU_LIN && GET_SPEC(ch) == SPEC_CHI_WARRIOR)
#define IS_ANCIENT_DRAGON(ch)	(GET_CLASS(ch) == CLASS_SHOU_LIN && GET_SPEC(ch) == SPEC_ANCIENT_DRAGON)
// Ranger
#define IS_NATURALIST(ch)	(GET_CLASS(ch) == CLASS_RANGER && GET_SPEC(ch) == SPEC_NATURALIST)
#define IS_HUNTER(ch)		(GET_CLASS(ch) == CLASS_RANGER && GET_SPEC(ch) == SPEC_HUNTER)
// Necromancer
#define IS_WITCH_DOCTOR(ch)	(GET_CLASS(ch) == CLASS_NECROMANCER && GET_SPEC(ch) == SPEC_WITCH_DOCTOR)
#define IS_REVENANT(ch)		(GET_CLASS(ch) == CLASS_NECROMANCER && GET_SPEC(ch) == SPEC_REVENANT)

#define SPEC_NONE            0
#define SPEC_KNIGHT_ERRANT   1
#define SPEC_DEFILER         2
#define SPEC_DRAGOON         1
#define SPEC_KNIGHT_TEMPLAR  2
#define SPEC_ARCANIST        1
#define SPEC_ENCHANTER       2
#define SPEC_PRESTIDIGITATOR 1
#define SPEC_SHADE           2
#define SPEC_BOUNTY_HUNTER   1
#define SPEC_BUTCHER         2
#define SPEC_TRICKSTER       1
#define SPEC_BRIGAND         2
#define SPEC_DARK_PRIEST     1
#define SPEC_HOLY_PRIEST     2
#define SPEC_DEFENDER        1
#define SPEC_DRAGON_SLAYER   2
#define SPEC_CHI_WARRIOR     1
#define SPEC_ANCIENT_DRAGON  2
#define SPEC_NATURALIST      1
#define SPEC_HUNTER          2
#define SPEC_WITCH_DOCTOR    1
#define SPEC_REVENANT        2

#define FIRST_ADVANCE_SKILL  1
#define SECOND_ADVANCE_SKILL 3
#define THIRD_ADVANCE_SKILL  5

/*
#define SPEC_DRAGOON 3
#define SPEC_KNIGHT_TEMPLAR 4
#define SPEC_ARCANIST 5
#define SPEC_ENCHANTER 6
#define SPEC_BOUNTY_HUNTER 7
#define SPEC_BUTCHER 8
#define SPEC_TRICKSTER 9
#define SPEC_THIEFSPECTWO 10
#define SPEC_DARK_PRIEST 11
#define SPEC_HOLY_PRIEST 12
#define SPEC_DEFENDER 13
#define SPEC_DRAGON_SLAYER 14
#define SPEC_CHI_WARRIOR 15
#define SPEC_ANCIENT_DRAGON 16
#define SPEC_NATURALIST 17
#define SPEC_HUNTER 18
#define SPEC_WITCH_DOCTOR 19
#define SPEC_REVENANT 20
*/

// DK
// SK
// Mage
// SD
#define SLOT_SLIPPERY_MIND  1
#define SLOT_NODESHIFT      1
// Assassin
#define SLOT_DETERRENCE     1
// Thief
//#define SLOT_DETERRENCE 1
#define SLOT_BLACKJACK      2
// Cleric
#define SLOT_SHADOW_FORM    1
// Warrior
#define SLOT_REDOUBT        1
#define SLOT_COMMANDING_SHOUT 2
// Shou Lin
// Ranger
// Necromancer
#define SLOT_QUICKEN 1
#define SLOT_METAMORPHOSIS 2


#define GET_QUEST(ch)           ((ch)->player_specials->quest.quest)
#define GET_QUEST_TIME(ch)      ((ch)->player_specials->quest.time)
#define GET_QUEST_WAIT(ch)      ((ch)->player_specials->quest.saved.next_quest)
#define GET_QUEST_TASKS(ch)     ((ch)->player_specials->quest.tasks)
#define GET_QUEST_COUNT(ch)     ((ch)->player_specials->quest.saved.quest_count)
#define GET_QUEST_DONE(ch, i)   ((ch)->player_specials->quest.saved.quests[i])
#define GET_QUEST_GIVER(ch)     ((ch)->player_specials->quest.mobrnum)

#define GET_SKILL_LVL(ch, i)    (spell_info[i].min_level[GET_CLASS(ch)])
#define GET_SKILL_LVL_MAX(ch, i) (GET_SKILL_LVL(ch, i) < LVL_IMMORT ? \
                                 GET_SKILL_LVL(ch, i) : 0)
#define GET_SKILL(ch, i)	(GET_LEVEL(ch) >= GET_SKILL_LVL_MAX(ch, i) ? \
                                 (ch)->player_specials->saved.skills[i] : 0)
#define SET_SKILL(ch, i, pct)	{ (ch)->player_specials->saved.skills[i] = pct; }

#define GET_ASPECT(ch)   ((ch)->aspect)

#define GET_USAGE(ch, i)	((ch)->player_specials->saved.skill_usage[i])
#define SET_USAGE(ch, i, u)	{ (ch)->player_specials->saved.skill_usage[i] = u; }

#define GET_MOB_SPEC(ch)    (IS_MOB(ch) ? (mob_index[(ch->nr)].func) : NULL)
#define GET_MOB_RNUM(mob)	((mob)->nr)
#define GET_MOB_VNUM(mob)	(IS_MOB(mob) ? \
                             mob_index[GET_MOB_RNUM(mob)].virtual : -1)

#define MEMORY(ch)          ((ch)->mob_specials.memory)
#define GET_DEFAULT_POS(ch)	((ch)->mob_specials.default_pos)
#define GET_LOAD_POS(ch)    ((ch)->mob_specials.load_pos)

#define STRENGTH_APPLY_INDEX(ch) \
        ( ((GET_ADD(ch)==0) || (GET_STR(ch) < 18)) ? GET_STR(ch) :\
          ((GET_ADD(ch) <= 40) && (GET_STR(ch) == 18)) ? 19 :( \
          ((GET_ADD(ch) <= 60) && (GET_STR(ch) == 18)) ? 20 :( \
          ((GET_ADD(ch) <= 80) && (GET_STR(ch) == 18)) ? 21 :( \
          ((GET_ADD(ch) <= 90) && (GET_STR(ch) == 18)) ? 22 :( \
          ((GET_ADD(ch) == 100) && (GET_STR(ch) == 18)) ? 23 : \
            GET_STR(ch) + 5 ) ) ) )     \
        )

#define CAN_CARRY_W(ch) (str_app[STRENGTH_APPLY_INDEX(ch)].carry_w)
#define CAN_CARRY_N(ch) (5 + (GET_DEX(ch) >> 1) + (GET_LEVEL(ch) >> 1))
#define AWAKE(ch) (GET_POS(ch) > POS_SLEEPING)

#define IS_HUMAN(ch) (GET_RACE(ch) == RACE_HUMAN || GET_RACE(ch) == RACE_SHUMAN)
#define IS_HALFLING(ch) (GET_RACE(ch) == RACE_HALFLING || GET_RACE(ch) == RACE_SHALFLING)
#define IS_ELF(ch)  (GET_RACE(ch) == RACE_ELF || GET_RACE(ch) == RACE_SELF)
#define IS_DROW(ch) (GET_RACE(ch) == RACE_DROW || GET_RACE(ch) == RACE_SDROW)
#define NOT_DROW(ch) (!IS_DROW(ch))
#define IS_DWARF(ch) (GET_RACE(ch) == RACE_DWARF || GET_RACE(ch) == RACE_SDWARF)
#define IS_MINOTAUR(ch) (GET_RACE(ch) == RACE_MINOTAUR || GET_RACE(ch) == RACE_SMINOTAUR)
#define IS_OGRE(ch) (GET_RACE(ch) == RACE_OGRE || GET_RACE(ch) == RACE_SOGRE)
#define IS_TROLL(ch) (GET_RACE(ch) == RACE_TROLL || GET_RACE(ch) == RACE_STROLL)
#define IS_DRACONIAN(ch) (GET_RACE(ch) == RACE_DRACONIAN || GET_RACE(ch) == RACE_SDRACONIAN)
#define IS_GNOME(ch) (GET_RACE(ch) == RACE_GNOME || GET_RACE(ch) == RACE_SGNOME)
#define IS_ORC(ch) (GET_RACE(ch) == RACE_ORC || GET_RACE(ch) == RACE_SORC)

#define IS_VAMPIRE(ch) (GET_RACE(ch) == RACE_VAMPIRE)
#define IS_ELEMENTAL(ch) (GET_RACE(ch) == RACE_ELEMENTAL)
#define IS_AMARA(ch) (GET_RACE(ch) == RACE_AMARA)
#define IS_IZARTI(ch) (GET_RACE(ch) == RACE_IZARTI)
#define IS_ELEMENTAL(ch) (GET_RACE(ch) == RACE_ELEMENTAL)
#define IS_FAERIE(ch) (GET_RACE(ch) == RACE_FAERIE)
#define IS_UNDEAD(ch) (GET_RACE(ch) == RACE_UNDEAD)
#define IS_DEMON(ch) (GET_RACE(ch) == RACE_DEMON)
#define IS_DRAGON(ch) (GET_RACE(ch) == RACE_DRAGON)
#define IS_GIANT(ch) (GET_RACE(ch) == RACE_GIANT)
#define IS_WEREWOLF(ch) (GET_RACE(ch) == RACE_WEREWOLF)

#define IS_SUNBLIND(ch) (GET_RACE(ch) == RACE_DROW && IS_SUNLIGHT((ch)->in_room) && \
        !PRF_FLAGGED(ch, PRF_HOLYLIGHT))
                            
#define CAN_SEE_IN_DARK(ch) \
        (PRF_FLAGGED(ch, PRF_HOLYLIGHT) || aff_level(ch, AFF_INFRAVISION) >= 0)
#define CAN_SEE_IN_DARK_OLD(ch) \
   (AFF_FLAGGED(ch, AFF_INFRAVISION) || PRF_FLAGGED(ch, PRF_HOLYLIGHT) \
   || (GET_RACE(ch) == RACE_UNDEAD) || (GET_RACE(ch) == RACE_VAMPIRE) \
   || (GET_RACE(ch) == RACE_DEMON) || (GET_RACE(ch) == RACE_ELF) \
   || IS_DROW \
   || (GET_RACE(ch) == RACE_WEREWOLF) || (GET_RACE(ch) == RACE_ORC)) 

#define HAS_WINGS(ch) ( !ch->equipment[WEAR_ABOUT] && \
			((GET_RACE(ch) == RACE_DRACONIAN) || \
			 (GET_RACE(ch) == RACE_DRAGON) || \
			 (GET_RACE(ch) == RACE_FAERIE)) )

#define IS_IMMORTAL(ch) (GET_LEVEL(ch) >= LVL_IMMORT)
#define IS_MORTAL(ch)   (GET_LEVEL(ch) <  LVL_IMMORT)

#define CAN_FLY(ch) (IS_SET_AR(AFF_FLAGS(ch), AFF_FLY) \
		     || (GET_LEVEL(ch) >= LVL_IMMORT) \
		     || HAS_WINGS(ch) \
                     || (GET_RACE(ch) == RACE_ELEMENTAL && AIR_ELEMENTAL == GET_SUBRACE(ch))  )

#define CAN_SWIM(ch) (CAN_FLY(ch))
#define IS_GOOD(ch)    (GET_ALIGNMENT(ch) >= 350)
#define IS_EVIL(ch)    (GET_ALIGNMENT(ch) <= -350)
#define IS_NEUTRAL(ch) (!IS_GOOD(ch) && !IS_EVIL(ch))

/* KDM(Ynnek) */
#define SEEKING(ch)           ((ch)->char_specials.seeking.hunting)
#define SEEK_STATUS(ch)       ((ch)->char_specials.seeking.status)
#define SEEK_TARGETSTR(ch)    ((ch)->char_specials.seeking.targetstr)

/* descriptor-based utils ************************************************/


#ifdef OLD_WAITS
#define WAIT_STATE(ch, cycle) { if ((ch)->desc) (ch)->desc->wait = (cycle); }
#define CHECK_WAIT(ch)	(((ch)->desc) ? ((ch)->desc->wait > 1) : 0)
#else
#define WAIT_STATE(ch, cycle) ( (ch)->player.stunned = (cycle) )
#define CHECK_WAIT(ch)        ((ch)->player.stunned > 1 ? 1 : 0 )
#endif

/* Descriptor-based utils. */
#define STATE(d)	((d)->connected)

/** Defines whether d is using OLC. */
#define IS_IN_OLC(d)   ((STATE(d) >= FIRST_OLC_STATE) && (STATE(d) <= LAST_OLC_STATE))

/** Defines whether d is playing or not. */
#define IS_PLAYING(d)   (IS_IN_OLC(d) || STATE(d) == CON_PLAYING)

#define SENDOK(ch)     (((ch)->desc || SCRIPT_CHECK((ch), MTRIG_ACT)) && \
                       (to_sleeping || AWAKE(ch)) && \
                       !PLR_FLAGGED((ch), PLR_WRITING))

#define SET_DCPENDING(d)  d->dcPending = 1
#define CLR_DCPENDING(d)  d->dcPending = 0
#define NOISE_LEVEL(ch) ((ch)->desc->noisy)
#define SPAM_LEVEL(ch)  ((ch)->desc->spammy)

/* object utils **********************************************************/


#define GET_OBJ_TYPE(obj)       ((obj)->obj_flags.type_flag)
#define GET_OBJ_COST(obj)       ((obj)->obj_flags.cost)
#define GET_OBJ_RENT(obj)       ((obj)->obj_flags.cost_per_day)
#define GET_OBJ_EXTRA(obj)      ((obj)->obj_flags.extra_flags)
#define GET_OBJ_WEAR(obj)       ((obj)->obj_flags.wear_flags)
/** Return value val for obj. */
#define GET_OBJ_VAL(obj, val)	((obj)->obj_flags.value[(val)])
#define GET_OBJ_WEIGHT(obj)     ((obj)->obj_flags.weight)
#define GET_OBJ_TIMER(obj)      ((obj)->obj_flags.timer)
#define GET_OBJ_POSITION(obj)	((obj)->obj_flags.position)
#define GET_OBJ_RNUM(obj)       ((obj)->item_number)
#define GET_OBJ_VNUM(obj)       (GET_OBJ_RNUM(obj) >= 0 ? \
                                 obj_index[GET_OBJ_RNUM(obj)].virtual : -1)
#define GET_OBJ_RENAME(ob)      ((obj)->rename_slot)
// 128 #define IS_OBJ_STAT(obj,stat)	(IS_SET((obj)->obj_flags.extra_flags,stat))

#define GET_OBJ_SPEC(obj) ((obj)->item_number >= 0 ? \
                           (obj_index[(obj)->item_number].func) : NULL)

// 128 #define CAN_WEAR(obj, part) (IS_SET((obj)->obj_flags.wear_flags, (part)))


/* compound utilities and other macros **********************************/


#define HSHR(ch) (GET_SEX(ch) ? (GET_SEX(ch)==SEX_MALE ? "his":"her") :"its")
#define HSSH(ch) (GET_SEX(ch) ? (GET_SEX(ch)==SEX_MALE ? "he" :"she") : "it")
#define capsHSSH(ch) (GET_SEX(ch) ? (GET_SEX(ch)==SEX_MALE ? "He":"She"):"It")
#define HMHR(ch) (GET_SEX(ch) ? (GET_SEX(ch)==SEX_MALE ? "him":"her") : "it")
#define MANWOMAN(ch) (GET_SEX(ch) ? (GET_SEX(ch) == SEX_MALE ? "man":"woman"): "man")
#define SINGPLUR(var)	(((var) > 1) ? "s" : "")

#define ANA(obj) (strchr("aeiouyAEIOUY", *(obj)->name) ? "An" : "A")
#define SANA(obj) (strchr("aeiouyAEIOUY", *(obj)->name) ? "an" : "a")


/* Various macros building up to CAN_SEE */

#define SHADOWED(sub) ((FIGHTING(sub) &&  \
   IS_AFFECTED(FIGHTING(sub), AFF_SHADOW_SPHERE)) || \
   IS_AFFECTED(sub, AFF_SHADOW_SPHERE))

#define LIGHT_OK(sub) (!IS_AFFECTED(sub, AFF_BLIND) && \
  ((IS_LIGHT((sub)->in_room) && !SHADOWED(sub)) \
  || CAN_SEE_IN_DARK(sub)))

#define SHADOW_NOT_OK(sub)  (SHADOWED(sub) && !CAN_SEE_IN_DARK(sub))

#define SHADOW_OK(sub, obj) \
  (aff_level(sub, AFF_INFRAVISION) + 15 >= aff_level(obj, AFF_SHADOW_SPHERE))

#define SHADOW_OK_OLD(sub, obj) \
 ((!IS_AFFECTED((obj), AFF_SHADOW_SPHERE)) || CAN_SEE_IN_DARK(sub))

#define INVIS_OK_OLD(sub, obj) \
 ((!IS_AFFECTED((obj),AFF_INVISIBLE) || IS_AFFECTED(sub,AFF_DETECT_INVIS)) && \
 (!IS_AFFECTED((obj), AFF_HIDE) || IS_AFFECTED(sub, AFF_SENSE_LIFE)))

#define INVIS_OK(sub, obj) \
  ((aff_level(sub, AFF_DETECT_INVIS) + 15 >= aff_level(obj, AFF_INVISIBLE)) && \
   (aff_level(sub, AFF_SENSE_LIFE) + 15 >= aff_level(obj, AFF_HIDE)))

#define IS_SUPER_HIDDEN(ch) \
  ((SINGING(ch) == SPELL_DANCE_SHADOWS) && ((ch)->state) >= DANCE_SHADOWS1 && \
	  ((ch)->state) <= DANCE_SHADOWS6)

#define MORT_CAN_SEE(sub, obj) (LIGHT_OK(sub) && SHADOW_OK(sub, obj) && \
   INVIS_OK(sub, obj) && !IS_AFFECTED(sub, AFF_BLIND) && !IS_SUPER_HIDDEN(obj) )

#define IMM_CAN_SEE(sub, obj) \
   (MORT_CAN_SEE(sub, obj) || PRF_FLAGGED(sub, PRF_HOLYLIGHT))

#define SELF(sub, obj)  ((sub) == (obj))

/* Can subject see character "obj"? */
#define CAN_SEE(sub, obj) (SELF(sub, obj) || \
   ((GET_REAL_LEVEL(sub) >= (IS_NPC(obj) ? 0 : GET_INVIS_LEV(obj))) && \
   IMM_CAN_SEE(sub, obj)))

/* End of CAN_SEE */


#define INVIS_OK_OBJ(sub, obj) \
  (!IS_OBJ_STAT((obj), ITEM_INVISIBLE) || IS_AFFECTED((sub), AFF_DETECT_INVIS))

#define MORT_CAN_SEE_OBJ(sub, obj) \
 (LIGHT_OK(sub) && (!SHADOW_NOT_OK(sub)) && INVIS_OK_OBJ(sub, obj))

#define CAN_SEE_OBJ(sub, obj) \
   (MORT_CAN_SEE_OBJ(sub, obj) || PRF_FLAGGED((sub), PRF_HOLYLIGHT))

#define CAN_CARRY_OBJ(ch,obj)  \
   (((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) <= CAN_CARRY_W(ch)) &&   \
    ((IS_CARRYING_N(ch) + 1) <= CAN_CARRY_N(ch)))

#define CAN_GET_OBJ(ch, obj)   \
   (CAN_WEAR((obj), ITEM_WEAR_TAKE) && CAN_CARRY_OBJ((ch),(obj)) && \
    CAN_SEE_OBJ((ch),(obj)))

/* Updated macro to add consideration for immortals */
#define PERS(ch, vict)   (CAN_SEE(vict, ch) ? GET_NAME(ch) : (GET_LEVEL(ch) > LVL_IMMORT ? "an immortal" : "someone"))

#define OBJS(obj, vict) (obj && CAN_SEE_OBJ((vict), (obj)) ? \
	(obj)->short_description  : "something")

#define OBJN(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
	fname((obj)->name) : "something")


#define EXIT(ch, door)  (world[(ch)->in_room].dir_option[door])

#define CAN_GO(ch, door) (EXIT(ch,door) && \
			 (EXIT(ch,door)->to_room != NOWHERE) && \
			 !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))

#define OUTSIDE(ch) (!ROOM_FLAGGED((ch)->in_room, ROOM_INDOORS))

#define IS_AFK(ch) ( PRF_FLAGGED(ch, PRF_AFK) || \
       ((ch->char_specials.timer * SECS_PER_MUD_HOUR / SECS_PER_REAL_MIN) > 5) )

/* OS compatibility ******************************************************/

/* OS compatibility */
#ifndef NULL
/** Just in case NULL is not defined. */
#define NULL (void *)0
#endif

#if !defined(FALSE)
/** Just in case FALSE is not defined. */
#define FALSE 0
#endif

#if !defined(TRUE)
/** Just in case TRUE is not defined. */
#define TRUE  (!FALSE)
#endif

#if !defined(YES)
/** In case YES is not defined. */
#define YES 1
#endif

#if !defined(NO)
/** In case NO is not defined. */
#define NO 0
#endif

/* defines for fseek */
#ifndef SEEK_SET
#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2
#endif

/*
 * Some systems such as Sun's don't have prototyping in their header files.
 * Thus, we try to compensate for them.
 *
 * Much of this is from Merc 2.2, used with permission.
 */
#if defined(linux)
char	*crypt( const char *key, const char *salt);
#endif

/*
 * The crypt(3) function is not available on some operating systems.
 * In particular, the U.S. Government prohibits its export from the
 * United States to foreign countries.
 * Turn on NOCRYPT to keep passwords in plain text.
 */
#ifdef NOCRYPT
#define CRYPT(a,b) (a)
#else
#define CRYPT(a,b) ((char *) crypt((a),(b)))
#endif

/** Pointer to the config file. */
#define CONFIG_CONFFILE         config_info.CONFFILE
/** Player killing allowed or not? */
#define CONFIG_PK_ALLOWED       config_info.play.pk_allowed
/** Player thieving allowed or not? */
#define CONFIG_PT_ALLOWED       config_info.play.pt_allowed
/** What level to use the shout command? */
#define CONFIG_LEVEL_CAN_SHOUT  config_info.play.level_can_shout
/** How many move points does holler cost? */
#define CONFIG_HOLLER_MOVE_COST config_info.play.holler_move_cost
/** How many characters can fit in a room marked as tunnel? */
#define CONFIG_TUNNEL_SIZE      config_info.play.tunnel_size
/** What is the max experience that can be gained at once? */
#define CONFIG_MAX_EXP_GAIN     config_info.play.max_exp_gain
/** What is the max experience that can be lost at once? */
#define CONFIG_MAX_EXP_LOSS     config_info.play.max_exp_loss
/** How long will npc corpses last before decomposing? */
#define CONFIG_MAX_NPC_CORPSE_TIME config_info.play.max_npc_corpse_time
/** How long will pc corpses last before decomposing? */
#define CONFIG_MAX_PC_CORPSE_TIME  config_info.play.max_pc_corpse_time
/** How long can a pc be idled before being pulled into the void? */
#define CONFIG_IDLE_VOID        config_info.play.idle_void
/** How long until the idle pc is force rented? */
#define CONFIG_IDLE_RENT_TIME   config_info.play.idle_rent_time
/** What level and above is immune to idle outs? */
#define CONFIG_IDLE_MAX_LEVEL   config_info.play.idle_max_level
/** Are death traps dumps? */
#define CONFIG_DTS_ARE_DUMPS    config_info.play.dts_are_dumps
/** Should items crated with the load command be placed on ground or
 * in the creator's inventory? */
#define CONFIG_LOAD_INVENTORY   config_info.play.load_into_inventory
/** Get the track through doors setting. */
#define CONFIG_TRACK_T_DOORS    config_info.play.track_through_doors
/** Get the permission to level up from mortal to immortal. */
#define CONFIG_NO_MORT_TO_IMMORT config_info.play.no_mort_to_immort
/** Get the 'OK' message. */
#define CONFIG_OK               config_info.play.OK
/** Get the NOPERSON message. */
#define CONFIG_NOPERSON         config_info.play.NOPERSON
/** Get the NOEFFECT message. */
#define CONFIG_NOEFFECT         config_info.play.NOEFFECT
/** Get the display closed doors setting. */
#define CONFIG_DISP_CLOSED_DOORS config_info.play.disp_closed_doors
/** Is plague contagious? */
#define CONFIG_PLAGUE_IS_CONTAGIOUS       config_info.play.plague_is_contagious
/** Are quests currently active? */
#define CONFIG_QUEST_ACTIVE    config_info.play.quest_active

/* DG Script Options */
#define CONFIG_SCRIPT_PLAYERS  config_info.play.script_players

/* Crash Saves */
/** Get free rent setting. */
#define CONFIG_FREE_RENT        config_info.csd.free_rent
/** Get max number of objects to save. */
#define CONFIG_MAX_OBJ_SAVE     config_info.csd.max_obj_save
/** Get minimum cost to rent. */
#define CONFIG_MIN_RENT_COST    config_info.csd.min_rent_cost
/** Get the auto save setting. */
#define CONFIG_AUTO_SAVE        config_info.csd.auto_save
/** Get the auto save frequency. */
#define CONFIG_AUTOSAVE_TIME    config_info.csd.autosave_time
/** Get the length of time to hold crash files. */
#define CONFIG_CRASH_TIMEOUT    config_info.csd.crash_file_timeout
/** Get legnth of time to hold rent files. */
#define CONFIG_RENT_TIMEOUT     config_info.csd.rent_file_timeout

/* Room Numbers */
/** Get the mortal start room. */
#define CONFIG_MORTAL_START     config_info.room_nums.mortal_start_room
/** Get the immortal start room. */
#define CONFIG_IMMORTAL_START   config_info.room_nums.immort_start_room
/** Get the frozen character start room. */
#define CONFIG_FROZEN_START     config_info.room_nums.frozen_start_room
/** Get the 1st donation room. */
#define CONFIG_DON_ROOM_1       config_info.room_nums.donation_room_1
/** Get the second donation room. */
#define CONFIG_DON_ROOM_2       config_info.room_nums.donation_room_2
/** Ge the third dontation room. */
#define CONFIG_DON_ROOM_3       config_info.room_nums.donation_room_3

/* Game Operation */
/** Get the default mud connection port. */
#define CONFIG_DFLT_PORT        config_info.operation.DFLT_PORT
/** Get the default mud ip address. */
#define CONFIG_DFLT_IP          config_info.operation.DFLT_IP
/** Get the max number of players allowed. */
#define CONFIG_MAX_PLAYING      config_info.operation.max_playing
/** Get the max filesize allowed. */
#define CONFIG_MAX_FILESIZE     config_info.operation.max_filesize
/** Get the max bad password attempts. */
#define CONFIG_MAX_BAD_PWS      config_info.operation.max_bad_pws
/** Get the siteok setting. */
#define CONFIG_SITEOK_ALL       config_info.operation.siteok_everyone
/** Get the auto-save-to-disk settings for OLC. */
#define CONFIG_OLC_SAVE         config_info.operation.auto_save_olc
/** Get the ability to use aedit or not. */
#define CONFIG_NEW_SOCIALS      config_info.operation.use_new_socials
/** Get the setting to resolve IPs or not. */
#define CONFIG_NS_IS_SLOW       config_info.operation.nameserver_is_slow
/** Default data directory. */
#define CONFIG_DFLT_DIR         config_info.operation.DFLT_DIR
/** Where is the default log file? */
#define CONFIG_LOGNAME          config_info.operation.LOGNAME
/** Get the text displayed in the opening menu. */
#define CONFIG_MENU             config_info.operation.MENU
/** Get the text displayed in the greeting. */
#define CONFIG_GREETINGS            config_info.operation.GREETINGS
/** Get the standard welcome message. */
#define CONFIG_WELC_MESSG       config_info.operation.WELC_MESSG
/** Get the standard new character message. */
#define CONFIG_START_MESSG      config_info.operation.START_MESSG
/** Should medit show the advnaced stats menu? */
#define CONFIG_MEDIT_ADVANCED   config_info.operation.medit_advanced

/* Autowiz */
/** Use autowiz or not? */
#define CONFIG_USE_AUTOWIZ      config_info.autowiz.use_autowiz
/** What is the minimum level character to put on the wizlist? */
#define CONFIG_MIN_WIZLIST_LEV  config_info.autowiz.min_wizlist_lev

#endif /* _UTILS_H_ */
