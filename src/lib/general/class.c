
#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/class.h"
#include "general/handler.h"
#include "actions/interpreter.h"
#include "util/utils.h"
#include "magic/spells.h"
#include "actions/act.h"          /* ACMDs located within the act*.c files */

/** Help buffer the global variable definitions */
#define __CLASS_C__

/*
 * This file attempts to concentrate most of the code which must be changed
 * in order for new classes to be added.  If you're adding a new class,
 * you should go through this entire file from beginning to end and add
 * the appropriate new special cases for your new class.
*/
/* local file scope functions */
static int    elemental_aff(CharData *ch, int skill);
static u_char roll_the_dice( u_char size_dice, u_char no_dice, u_char extra_dice);
static void   roll_strength( u_char str_limit, u_char str_add_limit,
		    u_char extra_dice, int is_warrior, u_char strength[2]);

/* local global file scope variables */
static const int racemap[];
static const int expscale[][9];

/* The code to interpret a race letter -- used in interpreter.c when a new
 * character is selecting a race. Don't get this confused with the other race
 * parsing functions for 'set' and saving mobiles to file. */
int pc_parse_race(char arg)
{
    switch (LOWER(arg))
    {
    case 'b': return RACE_GNOME;
    case 'c': return RACE_ORC;
    case 'e': return RACE_ELF;
    case 'x': return RACE_DROW;
    case 'i': return RACE_DRACONIAN;
    case 'l': return RACE_HALFLING;
    case 'm': return RACE_MINOTAUR;
    case 'o': return RACE_OGRE;
    case 't': return RACE_TROLL;
    case 'w': return RACE_DWARF;
    case 'h': return RACE_HUMAN;
    default:
        return RACE_UNDEFINED;
    }
}

int all_parse_race(char arg)
{
    switch (LOWER(arg))
    {
        case 'a': return RACE_ANIMAL;
        case 'b': return RACE_GNOME;
        case 'c': return RACE_ORC;
        case 'd': return RACE_DRAGON;
        case 'e': return RACE_ELF;
        case 'x': return RACE_DROW;
        case 'f': return RACE_ELEMENTAL;
        case 'g': return RACE_GIANT;
        case 'h': return RACE_HUMAN;
        case 'i': return RACE_DRACONIAN;
        case 'j': return RACE_FAERIE;
        case 'k': return RACE_AMARA;
        case 'l': return RACE_HALFLING;
        case 'm': return RACE_MINOTAUR;
        case 'n': return RACE_DEMON;
        case 'o': return RACE_OGRE;
        case 'p': return RACE_PLANT;
        case 'r': return RACE_WEREWOLF;
        case 't': return RACE_TROLL;
        case 'u': return RACE_UNDEAD;
        case 'v': return RACE_VAMPIRE;
        case 'w': return RACE_DWARF;
        case 'z': return RACE_IZARTI;
        default: return RACE_UNDEFINED;
    }
}

/* get_race_char and get_class_char appears to be used exclusive to saving
 * mobiles in medit.c -Xiuh */
char get_race_char(int race)
{
    switch (race)
    {
    case RACE_ANIMAL: return 'A';
    case RACE_GNOME: return 'B';
    case RACE_ORC: return 'C';
    case RACE_DRAGON: return 'D';
    case RACE_ELF: return 'E';
    case RACE_ELEMENTAL: return 'F';
    case RACE_DROW: return 'X';
    case RACE_GIANT: return 'G';
    case RACE_HUMAN: return 'H';
    case RACE_DRACONIAN: return 'I';
    case RACE_FAERIE: return 'J';
    case RACE_HALFLING: return 'L';
    case RACE_MINOTAUR: return 'M';
    case RACE_DEMON: return 'N';
    case RACE_OGRE: return 'O';
    case RACE_PLANT: return 'P';
    case RACE_WEREWOLF: return 'R';
    case RACE_TROLL: return 'T';
    case RACE_UNDEAD: return 'U';
    case RACE_VAMPIRE: return 'V';
    case RACE_DWARF: return 'W';
    case RACE_IZARTI: return 'Z';
    case RACE_AMARA: return 'K';
    default:
        mudlog(NRM, LVL_IMMORT, TRUE, "class.c -> unknown race passed to get_race_char");
        return 'H';
    }
}

char get_class_char(int class)
{
  switch (class) {

  case CLASS_ASSASSIN: 		return 'A';
  case CLASS_CLERIC: 		return 'C';
  case CLASS_DEATH_KNIGHT: 	return 'D';
  case CLASS_SHADOW_DANCER: 	return 'H';
  case CLASS_MAGIC_USER: 	return 'M';
  case CLASS_SOLAMNIC_KNIGHT: 	return 'K';
  case CLASS_RANGER: 		return 'R';
  case CLASS_SHOU_LIN: 		return 'S';
  case CLASS_THIEF: 		return 'T';
  case CLASS_WARRIOR: 		return 'W';
  case CLASS_NECROMANCER:       return 'N';
  default:
	mudlog(NRM, LVL_IMMORT, TRUE, "class.c -> Unknown class passed to get_class_char");
	return 'W';
  }
}

/* The code to interpret a class letter -- used in interpreter.c when a new
 * character is selecting a class and by 'set class' in act.wizard.c. */
int parse_class(char arg)
{
    switch (LOWER(arg))
    {
    case 'a': return CLASS_ASSASSIN;
    case 'c': return CLASS_CLERIC;
    case 'd': return CLASS_DEATH_KNIGHT;
    case 'h': return CLASS_SHADOW_DANCER;
    case 'm': return CLASS_MAGIC_USER;
    case 'k': return CLASS_SOLAMNIC_KNIGHT;
    case 'p': return CLASS_SOLAMNIC_KNIGHT;
    case 'r': return CLASS_RANGER;
    case 's': return CLASS_SHOU_LIN;
    case 't': return CLASS_THIEF;
    case 'w': return CLASS_WARRIOR;
    case 'n': return CLASS_NECROMANCER;
    default: return CLASS_UNDEFINED;
    }
}

#include "newclass.c"

/* bitvectors (i.e., powers of two) for each class, mainly for use in do_who
 * and do_users.  Add new classes at the end so that all classes use sequential
 * powers of two (1 << 0, 1 << 1, 1 << 2, 1 << 3, 1 << 4, 1 << 5, etc.) up to
 * the limit of your bitvector_t, typically 0-31. The find_race_bitvector has
 * the exact same functionality */
bitvector_t find_class_bitvector(const char *arg)
{
  size_t rpos, ret = 0;

  for (rpos = 0; rpos < strlen(arg); rpos++)
    ret |= (1 << parse_class(arg[rpos]));

  return (ret);
}

bitvector_t find_race_bitvector(const char *arg)
{
    size_t rpos, ret = 0;

    for (rpos = 0; rpos < strlen(arg); rpos++)
        ret |= (1 << all_parse_race(arg[rpos]));

    return (ret);
}
/* This function determines whether or not a given class is valid for
 * the race passed in. Used in interpreter.c - Vex.
 */
int parse_pc_race_class(int race, char arg)
{
    int class;
    if ((race < 0) || (race >= NUM_RACES)) /* Oh oh... */
    {
        mudlog(NRM, LVL_IMMORT, TRUE, "SYSERR: Unknown race passed to parse_pc_race_class in class.c!!!");
        return CLASS_UNDEFINED;
    }
    class = parse_class(arg);
    if ((class == CLASS_UNDEFINED) || (classes_allowed[race][class] == 0))
        return CLASS_UNDEFINED;
    else
        return class;
}

/* prereq: GET_RACE(ch) == RACE_ELEMENTAL */
static int elemental_aff(CharData *ch, int skill)
{
    switch (GET_SUBRACE(ch)) {
        case FIRE_ELEMENTAL:
            return 0;
        case AIR_ELEMENTAL:
            return (skill == AFF_FLY);
        case EARTH_ELEMENTAL:
            return 0;
        case WATER_ELEMENTAL:
            return (skill == AFF_AIRSPHERE);
    }
    return 0;
}

int has_native_aff(CharData *ch, int skill)
{
    switch (GET_RACE(ch))
    {
    case RACE_HUMAN: 
    case RACE_SHUMAN:
        return 0;
    case RACE_PLANT: return 0;
    case RACE_ANIMAL: return 0;
    case RACE_DRAGON: return 0;
    case RACE_UNDEAD: return (skill == AFF_INFRAVISION);
    case RACE_VAMPIRE: return (skill == AFF_INFRAVISION);
    case RACE_HALFLING: 
    case RACE_SHALFLING:
        return 0;
    case RACE_ELF: 
    case RACE_SELF:
        return (skill == AFF_INFRAVISION);
    case RACE_DWARF: 
    case RACE_SDWARF:
        return 0;
    case RACE_GIANT: return 0;
    case RACE_MINOTAUR:
    case RACE_SMINOTAUR:
                return 0;
    case RACE_DEMON: return (skill == AFF_INFRAVISION);
    case RACE_OGRE: 
    case RACE_SOGRE:
        return 0;
    case RACE_TROLL:
    case RACE_STROLL:
        return (skill == AFF_REGENERATE);
    case RACE_WEREWOLF:
        return (skill == AFF_PULSE_HIT || skill == AFF_INFRAVISION ||
                skill == AFF_REGENERATE);
    case RACE_ELEMENTAL: return elemental_aff(ch, skill);
    case RACE_ORC: 
    case RACE_SORC:
        return (skill == AFF_INFRAVISION);
    case RACE_GNOME: 
    case RACE_SGNOME:
        return 0;
    case RACE_DRACONIAN:
    case RACE_SDRACONIAN:
        return 0;
    case RACE_FAERIE: return (skill == AFF_BLINK || skill == AFF_PULSE_MANA);
    case RACE_AMARA: return (skill == AFF_AIRSPHERE);
    case RACE_IZARTI: return 0;
    case RACE_DROW: 
    case RACE_SDROW:
        return (skill == AFF_INFRAVISION);
    default:
        return 0;
    }
}

/*
 * These are definitions which control the guildmasters for each class.
 *
 * The first field (top line) controls the highest percentage skill level
 * a character of the class is allowed to attain in any skill.  (After
 * this level, attempts to practice will say "You are already learned in
 * this area."
 * 
 * The second line controls the maximum percent gain in learnedness a
 * character is allowed per practice -- in other words, if the random
 * die throw comes out higher than this number, the gain will only be
 * this number instead.
 *
 * The third line controls the minimu percent gain in learnedness a
 * charcter is allowed per practice -- in other words, if the random
 * die throw comes out below this number, the gain will be set up to
 * this number.
 * 
 * The fourth line simply sets whether the character knows 'spells'
 * or 'skills'.  This does not affect anything except the message given
 * to the character when trying to practice (i.e. "You know of the
 * following spells" vs. "You know of the following skills"
 */

#define SPELL	0
#define SKILL	1

/* #define LEARNED_LEVEL	0  % known which is considered "learned" */
/* #define MAX_PER_PRAC		1  max percent gain in skill per practice */
/* #define MIN_PER_PRAC		2  min percent gain in skill per practice */
/* #define PRAC_TYPE		3  should it say 'spell' or 'skill'?	*/

int prac_params[4][NUM_CLASSES] = {
  /* MAG  CLE    THE    WAR    RAN    ASS    SHO    SOL    DEA    SHA   NEC */
  {95,    95,    85,    80,    85,    85,    85,    85,    85,    85,	95},  	/* learned level */
  {25,    25,    25,    25,    25,    25,    25,    25,    25,    25,	25},  	/* max per prac */
  {10,    10,    10,    10,    10,    10,    10,    10,    10,    10,	10},  	/* min per pac */
  {SPELL, SPELL, SKILL, SKILL, SKILL, SKILL, SKILL, SKILL, SKILL, SKILL, SPELL }/* prac name */
};

/*
** This function rolls a number of "dice" equal to no_dice + extra_dice
** then picks the best ones(equal to the no_dice), adds them together
** and returns that value. Oh ya, the size of the dice is simply size_dice
** duh. So if size_dice was 6, no_dice was 3 and extra_dice was 1, it would
** roll 4d6, pick the best 3, add them up and return that value.
** Vex.
*/
static u_char roll_the_dice( u_char size_dice, u_char no_dice, u_char extra_dice)
{
#   define MAX_DICE 6
    u_char dice_rolls[MAX_DICE];
    u_char i, j, tmp;

    for( i = 0; i < (no_dice + extra_dice); i++ )
	dice_rolls[i] = number(1,size_dice);

    for( i = 0; i < (no_dice + extra_dice - 1); i++ ) {
        for( j = i+1, tmp = 0; j < (no_dice + extra_dice); j++ )
            if( dice_rolls[i] < dice_rolls[j] ) {
                tmp = dice_rolls[i];
                dice_rolls[i] = dice_rolls[j];
                dice_rolls[j] = tmp;
            }
    }/* for */
    tmp = 0;
    for ( i = 0; i < no_dice; i++ )
	tmp += dice_rolls[i];
    return tmp;

}

/*
** I actually wanted this function to return a length 2 u_char array, but
** could'nt persuade the compiler to do it for me. Thus, we are left with
** this messier version, which requries a length 2  u_char array to be
** passed IN. The first value in the array is the "strength roll", the
** 2nd is the "strength_add, or percentile roll". I have simplified the
** percentile band so it only can be in increments of 10. i.e. 0, 10, 20 etc.
** This is because the extra digits have no meaningfull information anyway,
** and players dont even see it so won't know any better ;p.
** Vex.
*/
static void roll_strength( u_char str_limit, u_char str_add_limit,
		    u_char extra_dice, int is_warrior, u_char strength[2])
{
  u_char no_dice, size_dice, the_roll, remainder;

  if (str_limit > 12)
	no_dice = 3;
  else if (str_limit > 6)
	no_dice = 2;
  else
	no_dice = 1;

  if (is_warrior && (str_add_limit > 0))
  {
	size_dice = (int)(str_limit + 10) / no_dice;
	remainder = (int)(str_limit + 10) % no_dice;
  }
  else
  {
	size_dice = (int)(str_limit) / no_dice;
	remainder = (int)(str_limit) % no_dice;
  }

  the_roll = roll_the_dice(size_dice, no_dice, extra_dice) + remainder;
  if (the_roll < 19)
  {
	strength[0] = the_roll;
	strength[1] = 0;
  }
  else if (the_roll > 28)
  {
	strength[0] = the_roll - 10;
	strength[1] = 100;
  }
  else
  {
	strength[0] = 18;
	strength[1] = (the_roll - 18) * 10;
  }

  if (strength[1] > str_add_limit)
      strength[1] = str_add_limit;
}

/*
** ShouLin Equipment Restrictions  - DIGGER
*/
int objAntiShouLin( ObjData *obj )
{
  if( GET_OBJ_TYPE( obj ) == ITEM_ARMOR )
  {
    if( CAN_WEAR( obj, ITEM_WEAR_FEET) ||
        CAN_WEAR( obj, ITEM_WEAR_LEGS) ||
        CAN_WEAR( obj, ITEM_WEAR_BODY) ||
        CAN_WEAR( obj, ITEM_WEAR_ARMS) ||
        CAN_WEAR( obj, ITEM_WEAR_HANDS)||
        CAN_WEAR( obj, ITEM_WEAR_HEAD) ||
        CAN_WEAR( obj, ITEM_WEAR_SHIELD))
    {
      SET_BIT_AR(obj->obj_flags.extra_flags, ITEM_ANTI_SHOU_LIN);
      return(1);
    }
  }
  return(0);
}


void roll_real_abils(CharData * ch)
{
#   define MAX_STATS 6
/*
**  This is an array of the stats that can be calculated "normally"
**  This means all the stats other than strength which needs special
**  special processing due the "str_add" part. The rows of the array
**  are the stats(int, wis, dex, con, cha) while the columns of the
**  array are no_dice, size_of_dice and remainder(needed since the
**  dice might not fit into the stat). The good thing about doing the
**  stat generation for the races this way is it makes it preety easy
**  to add a new race - you won't need to touch this function at all
**  unless you want to add extra abilities(like luck or something)
**  Vex.
*/
     u_char norm_stats[MAX_STATS - 1][3];
     u_char strength[2];
     u_char i, the_limit, no_dice;


  if (IS_NPC(ch)) {
    if (GET_LEVEL(ch) > 45)  /* Just max all stats */
    {
	ch->real_abils.str = race_stat_limits[(int)GET_RACE(ch)][STRENGTH_INDEX];
	ch->real_abils.str_add = race_stat_limits[(int)GET_RACE(ch)][STRENGTH_ADD_INDEX];

	ch->real_abils.intel = race_stat_limits[(int)GET_RACE(ch)][INTELLIGENCE_INDEX];
	ch->real_abils.wis = race_stat_limits[(int)GET_RACE(ch)][WISDOM_INDEX];
	ch->real_abils.dex = race_stat_limits[(int)GET_RACE(ch)][DEXTERITY_INDEX];
	ch->real_abils.con = race_stat_limits[(int)GET_RACE(ch)][CONSTITUTION_INDEX];
	ch->real_abils.cha = race_stat_limits[(int)GET_RACE(ch)][CHARISMA_INDEX];

	ch->aff_abils = ch->real_abils;
	return;
    }
    else
    {
	ch->real_abils.str = number(race_stat_limits[(int)GET_RACE(ch)][STRENGTH_INDEX]/2, race_stat_limits[(int)GET_RACE(ch)][STRENGTH_INDEX]);
	if (GET_STR(ch) > 18)
		ch->real_abils.str_add = race_stat_limits[(int)GET_RACE(ch)][STRENGTH_ADD_INDEX];
	else if (GET_STR(ch) < 18)
		ch->real_abils.str_add = 0;
        else
		ch->real_abils.str_add = number(1, race_stat_limits[(int)GET_RACE(ch)][STRENGTH_ADD_INDEX]);

	ch->real_abils.intel = number(race_stat_limits[(int)GET_RACE(ch)][INTELLIGENCE_INDEX]/2, race_stat_limits[(int)GET_RACE(ch)][INTELLIGENCE_INDEX]);
	ch->real_abils.wis = number(race_stat_limits[(int)GET_RACE(ch)][WISDOM_INDEX]/2, race_stat_limits[(int)GET_RACE(ch)][WISDOM_INDEX]);
	ch->real_abils.dex = number(race_stat_limits[(int)GET_RACE(ch)][DEXTERITY_INDEX]/2, race_stat_limits[(int)GET_RACE(ch)][DEXTERITY_INDEX]);
	ch->real_abils.con = number(race_stat_limits[(int)GET_RACE(ch)][CONSTITUTION_INDEX]/2, race_stat_limits[(int)GET_RACE(ch)][CONSTITUTION_INDEX]);
	ch->real_abils.cha = number(race_stat_limits[(int)GET_RACE(ch)][CHARISMA_INDEX]/2, race_stat_limits[(int)GET_RACE(ch)][CHARISMA_INDEX]);

	ch->aff_abils = ch->real_abils;
	return;
    }
  } /* Finished NPC stats */

  for( i = 0; i < (MAX_STATS - 1); i++) 
  {
	the_limit = race_stat_limits[(int)GET_RACE(ch)][i+2];
	if (the_limit > 12)
		no_dice = 3;
	else if (the_limit > 6)
		no_dice = 2;
	else
		no_dice = 1;

	norm_stats[i][0] = (int)(the_limit / no_dice); /* size of the dice */
	norm_stats[i][1] = no_dice;	/* number of dice to roll */
	norm_stats[i][2] = (int)(the_limit % no_dice); /* remainder */
  } /* for loop */


    switch (GET_CLASS(ch)) {
        case CLASS_MAGIC_USER:
        case CLASS_NECROMANCER:
            roll_strength(race_stat_limits[(int)GET_RACE(ch)][STRENGTH_INDEX],
                    race_stat_limits[(int)GET_RACE(ch)][STRENGTH_ADD_INDEX],
                    0, FALSE, strength);
            ch->real_abils.str = strength[0];
            ch->real_abils.str_add = strength[1];
            ch->real_abils.intel = roll_the_dice(
                    norm_stats[INTELLIGENCE_INDEX -2][0],
                    norm_stats[INTELLIGENCE_INDEX -2][1], 2
                ) + norm_stats[INTELLIGENCE_INDEX -2][2];
            ch->real_abils.wis   = roll_the_dice(
                    norm_stats[WISDOM_INDEX -2][0],
                    norm_stats[WISDOM_INDEX -2][1], 1
                ) + norm_stats[WISDOM_INDEX -2][2];
            ch->real_abils.dex   = roll_the_dice(
                    norm_stats[DEXTERITY_INDEX - 2][0],
                    norm_stats[DEXTERITY_INDEX - 2][1], 0
                ) + norm_stats[DEXTERITY_INDEX - 2][2];
            ch->real_abils.con   = roll_the_dice(
                    norm_stats[CONSTITUTION_INDEX - 2][0],
                    norm_stats[CONSTITUTION_INDEX - 2][1], 0
                ) + norm_stats[CONSTITUTION_INDEX - 2][2];
            ch->real_abils.cha   = roll_the_dice(
                    norm_stats[CHARISMA_INDEX -2][0],
                    norm_stats[CHARISMA_INDEX -2][1], 1
                ) + norm_stats[CHARISMA_INDEX -2][2];
        break;
  case CLASS_CLERIC:
    roll_strength(race_stat_limits[(int)GET_RACE(ch)][STRENGTH_INDEX],
		  race_stat_limits[(int)GET_RACE(ch)][STRENGTH_ADD_INDEX],
		  0, FALSE, strength);
    ch->real_abils.str = strength[0];
    ch->real_abils.str_add = strength[1];
    ch->real_abils.intel = roll_the_dice(norm_stats[INTELLIGENCE_INDEX -2][0],
					 norm_stats[INTELLIGENCE_INDEX -2][1],
					 1) + 
					 norm_stats[INTELLIGENCE_INDEX -2][2];
    ch->real_abils.wis   = roll_the_dice(norm_stats[WISDOM_INDEX -2][0],
					 norm_stats[WISDOM_INDEX -2][1],
					 2) +
					 norm_stats[WISDOM_INDEX -2][2];
    ch->real_abils.dex   = roll_the_dice(norm_stats[DEXTERITY_INDEX - 2][0],
					 norm_stats[DEXTERITY_INDEX - 2][1],
					 0) +
					 norm_stats[DEXTERITY_INDEX - 2][2];
    ch->real_abils.con   = roll_the_dice(norm_stats[CONSTITUTION_INDEX - 2][0],
					 norm_stats[CONSTITUTION_INDEX - 2][1],
					 0) +
					 norm_stats[CONSTITUTION_INDEX - 2][2];
    ch->real_abils.cha	 = roll_the_dice(norm_stats[CHARISMA_INDEX -2][0],
					 norm_stats[CHARISMA_INDEX -2][1],
					 1) +
					 norm_stats[CHARISMA_INDEX -2][2];
    break;
  case CLASS_THIEF:
  case CLASS_ASSASSIN:
    roll_strength(race_stat_limits[(int)GET_RACE(ch)][STRENGTH_INDEX],
		  race_stat_limits[(int)GET_RACE(ch)][STRENGTH_ADD_INDEX],
		  0, FALSE, strength);
    ch->real_abils.str = strength[0];
    ch->real_abils.str_add = strength[1];
    ch->real_abils.intel = roll_the_dice(norm_stats[INTELLIGENCE_INDEX -2][0],
					 norm_stats[INTELLIGENCE_INDEX -2][1],
					 0) + 
					 norm_stats[INTELLIGENCE_INDEX -2][2];
    ch->real_abils.wis   = roll_the_dice(norm_stats[WISDOM_INDEX -2][0],
					 norm_stats[WISDOM_INDEX -2][1],
					 0) +
					 norm_stats[WISDOM_INDEX -2][2];
    ch->real_abils.dex   = roll_the_dice(norm_stats[DEXTERITY_INDEX - 2][0],
					 norm_stats[DEXTERITY_INDEX - 2][1],
					 2) +
					 norm_stats[DEXTERITY_INDEX - 2][2];
    ch->real_abils.con   = roll_the_dice(norm_stats[CONSTITUTION_INDEX - 2][0],
					 norm_stats[CONSTITUTION_INDEX - 2][1],
					 1) +
					 norm_stats[CONSTITUTION_INDEX - 2][2];
    ch->real_abils.cha	 = roll_the_dice(norm_stats[CHARISMA_INDEX -2][0],
					 norm_stats[CHARISMA_INDEX -2][1],
					 0) +
					 norm_stats[CHARISMA_INDEX -2][2];
    break;
  case CLASS_WARRIOR:
    roll_strength(race_stat_limits[(int)GET_RACE(ch)][STRENGTH_INDEX],
		  race_stat_limits[(int)GET_RACE(ch)][STRENGTH_ADD_INDEX],
		  2, TRUE, strength);
    ch->real_abils.str = strength[0];
    ch->real_abils.str_add = strength[1];
    ch->real_abils.intel = roll_the_dice(norm_stats[INTELLIGENCE_INDEX -2][0],
					 norm_stats[INTELLIGENCE_INDEX -2][1],
					 0) + 
					 norm_stats[INTELLIGENCE_INDEX -2][2];
    ch->real_abils.wis   = roll_the_dice(norm_stats[WISDOM_INDEX -2][0],
					 norm_stats[WISDOM_INDEX -2][1],
					 0) +
					 norm_stats[WISDOM_INDEX -2][2];
    ch->real_abils.dex   = roll_the_dice(norm_stats[DEXTERITY_INDEX - 2][0],
					 norm_stats[DEXTERITY_INDEX - 2][1],
					 1) +
					 norm_stats[DEXTERITY_INDEX - 2][2];
    ch->real_abils.con   = roll_the_dice(norm_stats[CONSTITUTION_INDEX - 2][0],
					 norm_stats[CONSTITUTION_INDEX - 2][1],
					 1) +
					 norm_stats[CONSTITUTION_INDEX - 2][2];
    ch->real_abils.cha	 = roll_the_dice(norm_stats[CHARISMA_INDEX -2][0],
					 norm_stats[CHARISMA_INDEX -2][1],
					 0) +
					 norm_stats[CHARISMA_INDEX -2][2];
    break;
  case CLASS_RANGER:
    roll_strength(race_stat_limits[(int)GET_RACE(ch)][STRENGTH_INDEX],
		  race_stat_limits[(int)GET_RACE(ch)][STRENGTH_ADD_INDEX],
		  1, TRUE, strength);
    ch->real_abils.str = strength[0];
    ch->real_abils.str_add = strength[1];
    ch->real_abils.intel = roll_the_dice(norm_stats[INTELLIGENCE_INDEX -2][0],
					 norm_stats[INTELLIGENCE_INDEX -2][1],
					 0) + 
					 norm_stats[INTELLIGENCE_INDEX -2][2];
    ch->real_abils.wis   = roll_the_dice(norm_stats[WISDOM_INDEX -2][0],
					 norm_stats[WISDOM_INDEX -2][1],
					 0) +
					 norm_stats[WISDOM_INDEX -2][2];
    ch->real_abils.dex   = roll_the_dice(norm_stats[DEXTERITY_INDEX - 2][0],
					 norm_stats[DEXTERITY_INDEX - 2][1],
					 1) +
					 norm_stats[DEXTERITY_INDEX - 2][2];
    ch->real_abils.con   = roll_the_dice(norm_stats[CONSTITUTION_INDEX - 2][0],
					 norm_stats[CONSTITUTION_INDEX - 2][1],
					 2) +
					 norm_stats[CONSTITUTION_INDEX - 2][2];
    ch->real_abils.cha	 = roll_the_dice(norm_stats[CHARISMA_INDEX -2][0],
					 norm_stats[CHARISMA_INDEX -2][1],
					 0) +
					 norm_stats[CHARISMA_INDEX -2][2];
    break;
  case CLASS_SHADOW_DANCER:
    roll_strength(race_stat_limits[(int)GET_RACE(ch)][STRENGTH_INDEX],
		  race_stat_limits[(int)GET_RACE(ch)][STRENGTH_ADD_INDEX],
		  0, FALSE, strength);
    ch->real_abils.str = strength[0];
    ch->real_abils.str_add = strength[1];
    ch->real_abils.intel = roll_the_dice(norm_stats[INTELLIGENCE_INDEX -2][0],
					 norm_stats[INTELLIGENCE_INDEX -2][1],
					 2) + 
					 norm_stats[INTELLIGENCE_INDEX -2][2];
    ch->real_abils.wis   = roll_the_dice(norm_stats[WISDOM_INDEX -2][0],
					 norm_stats[WISDOM_INDEX -2][1],
					 0) +
					 norm_stats[WISDOM_INDEX -2][2];
    ch->real_abils.dex   = roll_the_dice(norm_stats[DEXTERITY_INDEX - 2][0],
					 norm_stats[DEXTERITY_INDEX - 2][1],
					 2) +
					 norm_stats[DEXTERITY_INDEX - 2][2];
    ch->real_abils.con   = roll_the_dice(norm_stats[CONSTITUTION_INDEX - 2][0],
					 norm_stats[CONSTITUTION_INDEX - 2][1],
					 0) +
					 norm_stats[CONSTITUTION_INDEX - 2][2];
    ch->real_abils.cha	 = roll_the_dice(norm_stats[CHARISMA_INDEX -2][0],
					 norm_stats[CHARISMA_INDEX -2][1],
					 0) +
					 norm_stats[CHARISMA_INDEX -2][2];
    break;
  case CLASS_SHOU_LIN:
    roll_strength(race_stat_limits[(int)GET_RACE(ch)][STRENGTH_INDEX],
		  race_stat_limits[(int)GET_RACE(ch)][STRENGTH_ADD_INDEX],
		  0, FALSE, strength);
    ch->real_abils.str = strength[0];
    ch->real_abils.str_add = strength[1];
    ch->real_abils.intel = roll_the_dice(norm_stats[INTELLIGENCE_INDEX -2][0],
					 norm_stats[INTELLIGENCE_INDEX -2][1],
					 0) + 
					 norm_stats[INTELLIGENCE_INDEX -2][2];
    ch->real_abils.wis   = roll_the_dice(norm_stats[WISDOM_INDEX -2][0],
					 norm_stats[WISDOM_INDEX -2][1],
					 2) +
					 norm_stats[WISDOM_INDEX -2][2];
    ch->real_abils.dex   = roll_the_dice(norm_stats[DEXTERITY_INDEX - 2][0],
					 norm_stats[DEXTERITY_INDEX - 2][1],
					 2) +
					 norm_stats[DEXTERITY_INDEX - 2][2];
    ch->real_abils.con   = roll_the_dice(norm_stats[CONSTITUTION_INDEX - 2][0],
					 norm_stats[CONSTITUTION_INDEX - 2][1],
					 0) +
					 norm_stats[CONSTITUTION_INDEX - 2][2];
    ch->real_abils.cha	 = roll_the_dice(norm_stats[CHARISMA_INDEX -2][0],
					 norm_stats[CHARISMA_INDEX -2][1],
					 0) +
					 norm_stats[CHARISMA_INDEX -2][2];
    break;
  case CLASS_SOLAMNIC_KNIGHT:
  case CLASS_DEATH_KNIGHT:
    roll_strength(race_stat_limits[(int)GET_RACE(ch)][STRENGTH_INDEX],
		  race_stat_limits[(int)GET_RACE(ch)][STRENGTH_ADD_INDEX],
		  1, TRUE, strength);
    ch->real_abils.str = strength[0];
    ch->real_abils.str_add = strength[1];
    ch->real_abils.intel = roll_the_dice(norm_stats[INTELLIGENCE_INDEX -2][0],
					 norm_stats[INTELLIGENCE_INDEX -2][1],
					 0) + 
					 norm_stats[INTELLIGENCE_INDEX -2][2];
    ch->real_abils.wis   = roll_the_dice(norm_stats[WISDOM_INDEX -2][0],
					 norm_stats[WISDOM_INDEX -2][1],
					 1) +
					 norm_stats[WISDOM_INDEX -2][2];
    ch->real_abils.dex   = roll_the_dice(norm_stats[DEXTERITY_INDEX - 2][0],
					 norm_stats[DEXTERITY_INDEX - 2][1],
					 0) +
					 norm_stats[DEXTERITY_INDEX - 2][2];
    ch->real_abils.con   = roll_the_dice(norm_stats[CONSTITUTION_INDEX - 2][0],
					 norm_stats[CONSTITUTION_INDEX - 2][1],
					 1) +
					 norm_stats[CONSTITUTION_INDEX - 2][2];
    ch->real_abils.cha	 = roll_the_dice(norm_stats[CHARISMA_INDEX -2][0],
					 norm_stats[CHARISMA_INDEX -2][1],
					 0) +
					 norm_stats[CHARISMA_INDEX -2][2];
    break;
  }
  ch->aff_abils = ch->real_abils;
}

int find_initial_skills(CharData *ch, int skill_num) {
    switch (GET_CLASS(ch)) {
        case CLASS_THIEF:
            if(skill_num == SKILL_HIDE)
                return 10;
            if(skill_num == SKILL_STEAL)
                return 15;
            if(skill_num == SKILL_PICK_LOCK)
                return 30;
            break;
        case CLASS_RANGER:
            if(skill_num == SKILL_SNEAK)
                return 10;
            if(skill_num == SKILL_HIDE)
                return 15;
            if(skill_num == SKILL_TRACK)
                return 20;
            break;
        case CLASS_MAGIC_USER:
            if(skill_num == SPELL_MAGIC_MISSILE)
                return 30;
            break;
        case CLASS_CLERIC:
            if(skill_num == SPELL_CURE_CRITIC)
                return 30;
            break;
        case CLASS_WARRIOR:
            if(skill_num == SKILL_INVIGORATE)
                return 25;
            break;
        case CLASS_ASSASSIN:
            if(skill_num == SKILL_BACKSTAB)
                return 25;
            break;
        case CLASS_SHOU_LIN:
            if(skill_num == SKILL_SWEEP)
                return 10;
            if(skill_num == SKILL_LAY_HANDS)
                return 10;
            break;
        case CLASS_SOLAMNIC_KNIGHT:
            if(skill_num == SPELL_CURE_LIGHT)
                return 25;
            break;
        case CLASS_DEATH_KNIGHT:
            if(skill_num == SPELL_CAUSE_WOUND)
                return 30;
            break;
        case CLASS_SHADOW_DANCER:
            if(skill_num == SPELL_SHADOW_BLADES)
                return 15;
            if(skill_num == SPELL_SHADOW_SPHERE)
                return 10;
            break;
        case CLASS_NECROMANCER:
            if(skill_num == SPELL_LIFE_DRAIN)
                return 15;
            if(skill_num == SPELL_BLINDNESS)
                return 10;
            break;
        default:
            mudlog(BRF, LVL_IMMORT, TRUE, "SYS ERR: %s has unknown class %d", GET_NAME(ch), GET_CLASS(ch));
            break;
    }

    switch (GET_RACE(ch)) {
        case RACE_MINOTAUR:
        case RACE_SMINOTAUR:
            if(skill_num == SKILL_GORE)
                return 60;
            break;
        case RACE_DRACONIAN:
        case RACE_SDRACONIAN:
            if(skill_num == SKILL_BREATHE)
                return 60;
            break;
        case RACE_DEMON:
            if(skill_num == SKILL_MIST)
                return 60;
            break;
        case RACE_IZARTI:
            if(skill_num == SKILL_CALM)
                return 60;
            break;
        case RACE_VAMPIRE:
            if(skill_num == SKILL_FEED)
                return 60;
            break;
        case RACE_AMARA:
            if(skill_num == SKILL_STING)
                return 60;
            break;
        case RACE_ELEMENTAL:
            switch (GET_SUBRACE(ch)) {
                case FIRE_ELEMENTAL:
                    if(skill_num == SPELL_WALL_OF_FIRE)
                        return 60;
                    break;
                case AIR_ELEMENTAL:
                    if(skill_num == SPELL_TYPHOON)
                        return 60;
                    break;
                case EARTH_ELEMENTAL:
                    if(skill_num == SPELL_TREMOR)
                        return 60;
                    break;
                case WATER_ELEMENTAL:
                    if(skill_num == SPELL_TSUNAMI)
                        return 60;
                    break;
            }
            break;
        case RACE_UNDEAD:
            if(skill_num == SPELL_TERROR)
                return 60;
            break;
        default:
            break;
    }

    return 0;
}

void give_initial_skills(CharData *ch) {
    int skill_num, skill_level;

    for(skill_num = 0; skill_num <= MAX_SKILLS; skill_num++) {
        if(skill_level = find_initial_skills(ch, skill_num))
            SET_SKILL(ch, skill_num, skill_level);
    }
}

/* Initialisations for chars shared by remorting chars and new chars */
void do_initialise(CharData *ch)
{
    void level_up(CharData * ch);

    ch->points.max_hit = 10;
    ch->points.max_move = 60;
    ch->points.max_mana = 100;

    give_initial_skills(ch);

    GET_LEVEL(ch) = 1;
    set_title(ch, NULL);
    level_up(ch);
    if( GET_PRACTICES(ch) <= 1 ) GET_PRACTICES(ch) = 2;

    restore(ch, ch);

    ch->player.time.logon = time(0);

    /* Liam Jan 28, 1995 -- Level 1 chars don't start with 0 xp */

    GET_EXP(ch) = titles[(int)GET_CLASS(ch)][1].exp;
}

/* Some initializations for characters, including initial skills */
void do_start(CharData * ch)
{
    do_initialise(ch);

    GET_CLAN(ch) = 0;

    if( GET_CLASS(ch) == CLASS_SOLAMNIC_KNIGHT )
        GET_ALIGNMENT(ch) =  500;
    else if( GET_CLASS(ch) == CLASS_DEATH_KNIGHT )
        GET_ALIGNMENT(ch) = -500;
    else
        GET_ALIGNMENT(ch) = 0;

    SET_BIT_AR(PRF_FLAGS(ch), PRF_AUTOEXIT);
    SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPHP);
    SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPMANA);
    SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPMOVE);
    SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPEXP);

    // Toggle "color complete" on
    SET_BIT_AR(PRF_FLAGS(ch), PRF_COLOR_1);
    SET_BIT_AR(PRF_FLAGS(ch), PRF_COLOR_2);

    ch->player.time.played = 0;

    /* Liam Jan 28, 1995 -- Added "newbie pack", eq done by Tyr */
    {
        ObjData *obj;
	int    i;
	
	/* Objs 1220 to 1232 are the "newbie" pack */
	for (i=0;i<13;i++)
 	{
	    obj = read_perfect_object ( 1220 + i, VIRTUAL );
	    SET_BIT_AR(obj->obj_flags.extra_flags, ITEM_EXPLODES);
	    obj_to_char (obj, ch);
	}

	/* Give 'em some gold too */
	GET_GOLD(ch) = GET_CHA(ch) * number(8,12) * 10;
    }
}

#define LOSS_HP 0
#define LOSS_MP 0
#define LOSS_MV 0

/* HP/MP/MV loss table */
const short int losses[3][NUM_CLASSES] = {
/*   Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm*/
    { 8, 10, 13, 16, 15, 13, 13,  0,  0, 13,  8},  /* MAXHIT */
    { 9,  7,  0,  0,  3,  3,  3,  4,  4,  5,  8},  /* MAXMANA */
    { 3,  3,  3,  3,  4,  3,  3,  3,  3,  3,  3},  /* MAXMOVE */
};

void retreat_level(CharData *ch)
{
    ch->points.max_hit  -= losses[LOSS_HP][GET_CLASS(ch)];
    ch->points.max_mana -= losses[LOSS_MP][GET_CLASS(ch)];
    ch->points.max_move -= losses[LOSS_MV][GET_CLASS(ch)];
    GET_LOST_LEVELS(ch) += 1;

    save_char(ch, NOWHERE);

    mudlog( BRF, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE, "%s retreated to level %d", GET_NAME(ch), GET_LEVEL(ch));
}

/* Gain maximum in various points */
void level_up(CharData * ch)
{
  short add_hp   = 0;
  short add_mana = 0;
  short add_move = 0;
  short practices_gained = 0;
  short i        = 0;

  add_hp = con_app[(int)GET_CON(ch)].hitp;
  if (con_app[(int)GET_CON(ch)].extra >= number(1, 3))
	add_hp += 1;
  add_mana = int_app[(int)GET_INT(ch)].manap;
  if (int_app[(int)GET_INT(ch)].extra >= number(1,3))
	add_mana += 1;

  switch (GET_CLASS(ch)) {

  case CLASS_MAGIC_USER:
    add_hp  += number(4, 8);
    add_mana += number(4, 9);
    add_move += number(1, 3);
    break;

  case CLASS_CLERIC:
    add_hp  += number(5, 10);
    add_mana += number(3, 7);
    add_move += number(1, 3);
    break;

  case CLASS_THIEF:
    add_hp  += number(7, 13);
    add_mana = 0;
    add_move += number(2, 3);
    break;

  case CLASS_WARRIOR:
    add_hp  += number(10, 16);
    add_mana = 0;
    add_move += number(2, 3);
    break;

  case CLASS_RANGER:
    add_hp  += number(10, 15);
    add_mana += number(1, 3);
    add_move = number(2, 4);
    break;

  case CLASS_ASSASSIN:
    add_hp  += number(7, 13);
    add_mana = 0;
    add_move = number(2, 3);
    break;

  case CLASS_SHOU_LIN:
    add_hp  += number(7, 13);
    add_mana += number(1, 3);
    add_move += number(1, 3);
    break;

  case CLASS_SOLAMNIC_KNIGHT:
    add_hp  += number(10, 15);
    add_mana += number(1, 4);
    add_move += number(1, 3);
    break;

  case CLASS_DEATH_KNIGHT:
    add_hp  += number(10, 15);
    add_mana += number(1, 4);
    add_move += number(1, 3);
    break;

  case CLASS_SHADOW_DANCER:
    add_hp  += number(7, 13);
    add_mana += number(2, 5);
    add_move += number(1,3);
    break;

  case CLASS_NECROMANCER:
    add_hp  += number(4, 8);
    add_mana += number(4, 8);
    add_move += number(1, 3);
    break;

  }

  // drow get a little more mana : Sanji
  if(IS_DROW(ch)) add_mana += number(1,2);

  /* whew! glad we did all that work!  but, if they lost levels ... */
  if (GET_LOST_LEVELS(ch) > 0) {
    ch->points.max_hit  += losses[LOSS_HP][GET_CLASS(ch)];
    ch->points.max_mana += losses[LOSS_MP][GET_CLASS(ch)];
    ch->points.max_move += losses[LOSS_MV][GET_CLASS(ch)];
    GET_LOST_LEVELS(ch) -= 1;
  } else {
    ch->points.max_hit  += MAX(1, add_hp);
    ch->points.max_move += MAX(1, add_move);

    if (GET_LEVEL(ch) > 1)
      ch->points.max_mana += MAX(0, add_mana);

    practices_gained += wis_app[(int)GET_WIS(ch)].bonus;
    if (wis_app[(int)GET_WIS(ch)].extra >= number(1, 5))
	practices_gained += 1;
    GET_PRACTICES(ch) += practices_gained;

    if (GET_LEVEL(ch) >= LVL_IMMORT) {
      for (i = 0; i < 3; i++) GET_COND(ch, i) = (char) -1;
      SET_BIT_AR(PRF_FLAGS(ch), PRF_HOLYLIGHT);
    }
  }

  save_char(ch, NOWHERE);

  if(GET_LEVEL(ch) > 1) {
    sendChar(ch, "You have gained %d hit points.\r\n", MAX(1, add_hp));
    if( MAX(0, add_mana) )
       sendChar(ch, "You have gained %d mana points.\r\n", MAX(0, add_mana));
    sendChar(ch, "You have gained %d vigor points.\r\n", MAX(1, add_move));
    sendChar(ch, "You are able to practice with your guild master %d more times.\r\n", practices_gained);
  }
  restore(ch, ch);

  mudlog( BRF, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE, "%s advanced to level %d", GET_NAME(ch), GET_LEVEL(ch));
}

int invalid_class(CharData *ch, ObjData *obj) {
  if ((IS_OBJ_STAT(obj, ITEM_ANTI_MAGIC_USER) && IS_MAGIC_USER(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_CLERIC) && IS_CLERIC(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_WARRIOR) && IS_WARRIOR(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_ASSASSIN) && IS_ASSASSIN(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_SHOU_LIN) && IS_SHOU_LIN(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_SOLAMNIC_KNIGHT) && IS_SOLAMNIC_KNIGHT(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_DEATH_KNIGHT) && IS_DEATH_KNIGHT(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_SHADOW_DANCER) && IS_SHADOW_DANCER(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_RANGER) && IS_RANGER(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_THIEF) && IS_THIEF(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_NECROMANCER) && IS_NECROMANCER(ch)))
       return 1;
  else
       return 0;
}

int invalid_race(CharData *ch, ObjData *obj) 
{
    if ((IS_OBJ_STAT(obj, ITEM_ANTI_MINOTAUR) && IS_MINOTAUR(ch)) ||
        (IS_OBJ_STAT(obj, ITEM_ANTI_GNOME) && IS_GNOME(ch)) ||
        (IS_OBJ_STAT(obj, ITEM_ANTI_ORC) && IS_ORC(ch)) ||
        (IS_OBJ_STAT(obj, ITEM_ANTI_ELF) && IS_ELF(ch)) ||
        (IS_OBJ_STAT(obj, ITEM_ANTI_DROW) && IS_DROW(ch)) ||
        (IS_OBJ_STAT(obj, ITEM_ANTI_DRACONIAN) && IS_DRACONIAN(ch)) ||
        (IS_OBJ_STAT(obj, ITEM_ANTI_HALFLING) && IS_HALFLING(ch)) ||
        (IS_OBJ_STAT(obj, ITEM_ANTI_OGRE) && IS_OGRE(ch))	||
        (IS_OBJ_STAT(obj, ITEM_ANTI_TROLL) && IS_TROLL(ch)) ||
        (IS_OBJ_STAT(obj, ITEM_ANTI_DWARF) && IS_DWARF(ch)) ||
        (IS_OBJ_STAT(obj, ITEM_ANTI_DEMON) && IS_DEMON(ch)) ||
        (IS_OBJ_STAT(obj, ITEM_ANTI_IZARTI) && IS_IZARTI(ch)) ||
        (IS_OBJ_STAT(obj, ITEM_ANTI_VAMPIRE) && IS_VAMPIRE(ch)) ||
        (IS_OBJ_STAT(obj, ITEM_ANTI_WEREWOLF) && IS_WEREWOLF(ch)) ||
        (IS_OBJ_STAT(obj, ITEM_ANTI_ELEMENTAL) && IS_ELEMENTAL(ch)) ||
        (IS_OBJ_STAT(obj, ITEM_ANTI_GIANT) && IS_GIANT(ch)) ||
        (IS_OBJ_STAT(obj, ITEM_ANTI_FAERIE) && IS_FAERIE(ch)) ||
        (IS_OBJ_STAT(obj, ITEM_ANTI_AMARA) && IS_AMARA(ch)) ||
        (IS_OBJ_STAT(obj, ITEM_ANTI_UNDEAD) && IS_UNDEAD(ch)) ||
        (IS_OBJ_STAT(obj, ITEM_ANTI_HUMAN) && IS_HUMAN(ch))  ||
        (IS_OBJ_STAT(obj, ITEM_ANTI_REMORT) && IS_REMORT(ch)) ||
        (IS_OBJ_STAT(obj, ITEM_ANTI_PREMORT) && !IS_REMORT(ch)))
        return 1;
    else
	return 0;
}

/* ============================================================================
Constants used by races and classes.
============================================================================ */

const char *pc_class_types[] = {
    "Magic User",
    "Cleric",
    "Thief",
    "Warrior",
    "Ranger",
    "Assassin",
    "Shou-Lin",
    "Solamnic Knight",
    "Death Knight",
    "Shadow Dancer",
    "Necromancer",
    "\n"
};

const char *race_types[] = {
    "Human",
    "Plant",
    "Animal",
    "Dragon",
    "Undead",
    "Vampire",
    "Halfling",
    "Elf",
    "Dwarf",
    "Giant",
    "Minotaur",
    "Demon",
    "Ogre",
    "Troll",
    "Werewolf",
    "Elemental",
    "Orc",
    "Gnome",
    "Draconian",
    "Faerie",
    "Amara",
    "Izarti",
    "Drow",
    "Ascended Human",
    "Halfling Sneak",
    "Elf Ancient",
    "Dark Drow",
    "Firstborn Dwarf",
    "Minotaur Warchampion",
    "Ogre Magi",
    "Rootfellow Troll",
    "Dragonspawn",
    "Feyborn Gnome",
    "Orc Blooddrinker",
    "Terran",
    "Zerg",
    "Protoss",
    "\n"
};

const char *class_abbrevs[NUM_CLASSES + 1] = {
    "Mu",         /* (0) - MAGE            */
    "Cl",         /* (1) - CLERIC          */
    "Th",         /* (2) - THIEF           */
    "Wa",         /* (3) - WARRIOR         */
    "Ra",         /* (4) - RANGER          */
    "As",         /* (5) - ASSASSIN        */
    "Sl",         /* (6) - SHOU_LIN        */
    "Kn",         /* (7) - SOLAMNIC_KNIGHT */
    "De",         /* (8) - DEATH_KNIGHT    */
    "Sd",         /* (9) - SHADOW_DANCER   */
    "Nm",	  /* (10) - NECROMANCER	   */
    "\n"
};

/* Names first */
const char *race_abbrevs[NUM_RACES + 1] = {
    "Hum",		/* (0) - HUMAN		*/
    "&09Plt&00",	/* (1) - PLANT		*/
    "Ani",		/* (2) - ANIMAL		*/
    "&10&15Drg&00",	/* (3) - DRAGON		*/
    "&00&07Und&00",	/* (4) - UNDEAD		*/
    "&08Vam&00",	/* (5) - VAMPIRE	*/
    "Hlf",		/* (6) - HALFLING	*/
    "Elf",		/* (7) - ELF		*/
    "Dwf",		/* (8) - DWARF		*/
    "&00&05Gia&00",	/* (9) - GIANT		*/
    "Min",		/* (10) - MINOTAUR	*/
    "&00&06Dem&00",	/* (11) - DEMON		*/
    "Ogr",		/* (12) - OGRE		*/
    "Tro",		/* (13) - TROLL		*/
    "&00&03Wer&00",	/* (14) - WEREWOLF	*/
    "&14Ele&00",	/* (15) - ELEMENTAL	*/
    "Orc",		/* (16) - ORC		*/
    "Gnm",		/* (17) - GNOME		*/
    "Drc",		/* (18) - DRACONIAN	*/
    "&13Fae&00",	/* (19) - FAERIE	*/
    "&11Ama&00",	/* (20) - AMARA 	*/
    "&10Iza&00",	/* (21) - IZARTI	*/
    "Drw",              /* (22) - DROW		*/
    "&11Hum&00",        /* (23) - SHUMAN	*/
    "&09&18Hlf&00",     /* (24) - SHALFLING	*/
    "&09Elf&00",        /* (25) - SELF		*/
    "&07Drw&00",        /* (26) - SDROW		*/
    "&00&03Dwf&00",        /* (27) - SDWARF	*/
    "&14&15Min&00",     /* (28) - SMINOTAUR	*/
    "&14Ogr&00",        /* (29) - SOGRE		*/
    "&09Tro&00",        /* (30) - STROLL	*/
    "&08Drc&00",        /* (31) - SDRACONIAN	*/
    "&00&04Gnm&00",        /* (32) - SGNOME	*/
    "&00&01Orc&00",        /* (33) - SORC		*/
    "&18&01Trn&00",     /*  34    TERRAN        */
    "&16&14Zrg&00",
    "&21&07Pro&00",
    "\n"
};

const char *ele_subrace_abbrevs[] = {
        "Ele", /* (0) - UNDEFINED!?! */
        "&01Ele&00", /* (1) - Fire         */
        "&14Ele&00", /* (2) - Air          */
        "&11Ele&00", /* (3) - Water        */
        "&03Ele&00", /* (4) - Earth        */
};

const char *drc_subrace_abbrevs[] = {
    "Drc",           /* (0) - UNDEFINED!?! */
    "&16&08Drc&00", /* (1) - red_dragon */
    "&16&10Drc&00", /* (2) - green_dragon */
    "&16&11Drc&00", /* (3) - white_dragon */
    "&16&05Drc&00", /* (4) - black_dragon */
    "&09&22Drc&00", /* (5) - blue_dragon */
};

/* ============================================================================
Constants used to contruct menu in interpreter.c
============================================================================ */

/* The menu for choosing a class in interpreter.c: */
const char *class_menu[NUM_CLASSES] = {
"  [M] - Magic User\r\n",     /* Mage */
"  [C] - Cleric\r\n",	      /* Cleric */
"  [T] - Thief\r\n",	      /* Thief */
"  [W] - Warrior\r\n",	      /* Warrior */
"  [R] - Ranger\r\n",         /* Ranger */
"  [A] - Assassin\r\n",       /* Assassin */
"  [S] - Shou Lin\r\n",       /* Shou Lin */
"  [P] - Solamnic Knight\r\n",/* Solamnic Knight */
"  [D] - Death Knight\r\n",   /* Death Knight */
"  [H] - Shadow Dancer\r\n",  /* Shadow Dancer */
"  [N] - Necromancer\r\n"     /* Necromancer */
};

const char *race_menu =
"\r\n"
"Your choice of race will restrict which classes you can pick.  A character who\r\n"
"has advanced sufficiently can be reborn as any of these races or a variety of\r\n"
"more powerful races.\r\n"
"\r\n"
"Select a race : \r\n"
"  [H] - Human\r\n"
"  [B] - Gnome\r\n"
"  [C] - Orc\r\n"
"  [E] - Elf\r\n"
"  [I] - Draconian\r\n"
"  [L] - Halfling\r\n"
"  [M] - Minotaur\r\n"
"  [O] - Ogre\r\n"
"  [T] - Troll\r\n"
"  [W] - Dwarf\r\n"
"  [X] - Drow\r\n"
;

const char *breath_menu =
"\r\n"
"Select a breath weapon : \r\n"
"  [1] - Fire\r\n"
"  [2] - Gas\r\n"
"  [3] - Frost\r\n"
"  [4] - Acid\r\n"
"  [5] - Lightning\r\n"
;

/* ============================================================================
Constants defining race stat and class limitations.
============================================================================ */

/* Racial stat limitations - Vex. */
/*
** Vex - This array determines the racial maximums for each stat for every
** race. I did'nt bother defining any minimums - that can be taken care of
** in stat generation, items that are minus to stats will be able to lower
** a stat down to what ever value the mud will accept(same for every race thou)
** This array will control the maximums thou both for character generation
** AND equipment and/or spells.
*/
const char race_stat_limits[NUM_RACES][7] = {
/*	 S   %    I   W   D   C   Ch				*/
	{18, 100, 18, 18, 18, 18, 18}, /* 0 - RACE_HUMAN	*/
	{20, 100, 16, 16, 18, 20, 16}, /* 1 - RACE_PLANT 	*/
	{19, 100, 12, 12, 19, 19, 18}, /* 2 - RACE_ANIMAL	*/
	{25, 100, 20, 20, 20, 25, 20}, /* 3 - RACE_DRAGON	*/
	{20, 100, 16, 16, 16, 20,  3}, /* 4 - RACE_UNDEAD	*/
	{21, 100, 18, 18, 22, 20, 23}, /* 5 - RACE_VAMPIRE	*/
	{18,  50, 18, 17, 20, 18, 18}, /* 6 - RACE_HALFLING	*/
	{18,  70, 19, 20, 19, 16, 19}, /* 7 - RACE_ELF		*/
	{18, 100, 17, 19, 16, 21, 16}, /* 8 - RACE_DWARF	*/
	{24, 100, 10, 10, 12, 24, 14}, /* 9 - RACE_GIANT	*/
	{20, 100, 13, 12, 16, 20,  4}, /*10 - RACE_MINOTAUR	*/
	{22, 100, 19, 19, 20, 21, 20}, /*11 - RACE_DEMON	*/
	{21, 100, 12, 12, 15, 20,  5}, /*12 - RACE_OGRE		*/
	{20, 100,  6,  6, 12, 22,  3}, /*13 - RACE_TROLL	*/
	{21, 100, 18, 18, 20, 22, 14}, /*14 - RACE_WEREWOLF	*/
	{20, 100, 20, 20, 18, 20, 19}, /*15 - RACE_ELEMENTAL	*/
	{19, 100, 16, 16, 17, 19, 12}, /*16 - RACE_ORC		*/
	{18,  50, 20, 16, 18, 17, 18}, /*17 - RACE_GNOME	*/
	{19, 100, 17, 16, 16, 19,  8}, /*18 - RACE_DRACONIAN	*/
        {13,   0, 24, 24, 22, 16, 24}, /*19 - RACE_FAERIE       */
        {18, 100, 23, 23, 22, 18, 12}, /*20 - RACE_AMARA        */
        {19, 100, 21, 21, 21, 19, 22}, /*21 - RACE_IZARTI       */
        {18, 100, 19, 17, 21, 14, 18}, /*22 - RACE_DROW         */

        {20, 100, 20, 20, 20, 20, 20}, /*23 - RACE_SHUMAN       */
        {18,  90, 20, 18, 22, 20, 20}, /*24 - RACE_SHALFLING    */
        {19, 100, 21, 22, 21, 18, 21}, /*25 - RACE_SELF         */
        {20, 100, 20, 18, 23, 15, 20}, /*26 - RACE_SDROW        */
        {20, 100, 18, 20, 18, 23, 18}, /*27 - RACE_SDWARF       */
        {22, 100, 15, 14, 18, 22,  6}, /*28 - RACE_SMINOTAUR     */
        {24, 100, 14, 14, 16, 21,  7}, /*29 - RACE_SOGRE         */
        {22, 100,  8,  8, 14, 24,  5}, /*30 - RACE_STROLL       */
        {21, 100, 19, 17, 18, 21, 10}, /*31 - RACE_SDRACONIAN   */
        {18,  80, 22, 17, 20, 19, 20}, /*32 - RACE_SGNOME       */
        {21, 100, 17, 17, 19, 21, 14}, /*33 - RACE_SORC         */
        {18, 100, 18, 18, 18, 18, 18}, /* 0 - RACE_TERRAN	*/
        {18, 100, 18, 18, 18, 15, 18}, /* 0 - RACE_ZERG 	*/
        {20, 100, 18, 18, 14, 19, 18}, /* 0 - RACE_PROTOSS	*/
};

/* Classes allowed for PC races.*/
const signed short int classes_allowed[NUM_RACES][NUM_CLASSES] = {
/* 	 Ma Cl Th Wa Ra As Sl Kn Dk Sd Nm*/
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},	/* 0 - RACE_HUMAN */
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	/* 1 - RACE_PLANT */
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	/* 2 - RACE_ANIMAL */
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	/* 3 - RACE_DRAGON */
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	/* 4 - RACE_UNDEAD */
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	/* 5 - RACE_VAMPIRE */
	{0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0},	/* 6 - RACE_HALFLING */
	{1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0},	/* 7 - RACE_ELF */
	{0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0},	/* 8 - RACE_DWARF */
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	/* 9 - RACE_GIANT */
	{0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 1},	/* 10 - RACE_MINOTAUR */
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	/* 11 - RACE_DEMON */
	{1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0},	/* 12 - RACE_OGRE */
	{0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0},	/* 13 - RACE_TROLL */
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	/* 14 - RACE_WEREWOLF */
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	/* 15 - RACE_ELEMENTAL */
	{1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1},	/* 16 - RACE_ORC */
	{1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1},	/* 17 - RACE_GNOME */
	{1, 1, 0, 1, 0, 1, 1, 0, 0, 0, 1},	/* 18 - RACE_DRACONIAN */
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},      /* 19 - RACE_FAERIE */
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},      /* 20 - RACE_AMARA */
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},      /* 21 - RACE_IZARTI */
        {1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1},      /* 22 - RACE_DROW */
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},      /* 23 - RACE_SHUMAN */
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},      /* 24 - RACE_SHALFLING   */
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},      /* 25 - RACE_SELF */
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},      /* 26 - RACE_SDROW */
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},      /* 27 - RACE_SDWARF */
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},      /* 28 - RACE_SMINOTAUR */
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},      /* 29 - RACE_SOGRE */
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},      /* 30 - RACE_STROLL */
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},      /* 31 - RACE_SDRACONIAN */
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},      /* 32 - RACE_SGNOME */
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},      /* 33 - RACE_SORC */
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},      /* 33 - RACE_TERRAN */
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},      /* 33 - RACE_ZERG */
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},      /* 33 - RACE_PROTOSS */
};

/* This is the % of exp that they actually get WHILE on this level.  If a
 * level 1 amara kills a 100 exp mob, they get 13 exp earned.
 */

#define SCALE_AMARA     0
#define SCALE_DEMON     1
#define SCALE_IZARTI    2
#define SCALE_WEREWOLF  3
#define SCALE_VAMPIRE   4
#define SCALE_FAERIE    5
#define SCALE_GIANT     6
#define SCALE_ELEMENTAL 7
#define SCALE_UNDEAD    8
#define SCALE_SHUMAN    9
#define SCALE_SHALFLING 10
#define SCALE_SELF      11
#define SCALE_SDROW     12
#define SCALE_SDWARF    13
#define SCALE_SMINOTAUR 14
#define SCALE_SOGRE     15
#define SCALE_STROLL    16
#define SCALE_SGNOME    17
#define SCALE_SDRACONIAN 18
#define SCALE_SORC      19

/*
static const int racemap[NUM_RACES] = {
    -1,                 // human
    -1,                 // plant
    -1,                 // animal
    -1,                 // dragon
    SCALE_UNDEAD,       // undead
    SCALE_VAMPIRE,      // vampire
    -1,                 // halfling
    -1,                 // elf
    -1,                 // dwarf
    SCALE_GIANT,        // giant
    -1,                 // minotaur
    SCALE_DEMON,        // demon
    -1,                 // ogre
    -1,                 // troll
    SCALE_WEREWOLF,     // werewolf
    SCALE_ELEMENTAL,    // elemental
    -1,                 // orc
    -1,                 // gnome
    -1,                 // draconian
    SCALE_FAERIE,       // faerie
    SCALE_AMARA,        // amara
    SCALE_IZARTI,       // izarti
    -1,                 // drow
    SCALE_SHUMAN,       // super human
    SCALE_SHALFLING,    // etc.
    SCALE_SELF,
    SCALE_SDROW,
    SCALE_SDWARF,
    SCALE_SMINOTAUR,
    SCALE_SOGRE,
    SCALE_STROLL,
    SCALE_SGNOME,
    SCALE_SDRACONIAN,
    SCALE_SORC
 *  SCALE TERERAN
 *  SCALE ZERG
 *  SCALE PROTOSS
};
*/

static const int expScale[51] = {
    100,                // 0
    18, 18, 18, 18, 18, // 5
    18, 18, 19, 19, 19, // 10
    20, 20, 20, 20, 21, // 15
    21, 21, 21, 21, 22, // 20
    22, 22, 23, 23, 23, // 25
    23, 24, 24, 24, 25, // 30
    26, 27, 28, 29, 30, // 35
    35, 40, 45, 50, 55, // 40
    60, 65, 70, 75, 80, // 45
    85, 90, 95, 100, 100
};

/*  Old experience scaling system.  No real reason to give some races worse
 *  experience scales than others, especially since the most powerful races
 *  tended to have the easiest scales.
//   Ama  Dem  Iza  Wer  Vam  Fae  Gia  Ele  Und
static const int expscale[51][9] = {
    {100, 100, 100, 100, 100, 100, 100, 100, 100}, // 0
    {13,  15,  15,  18,  18,  18,  23,  25,  25},  // 1
    {13,  15,  15,  18,  18,  18,  23,  25,  25},  // 2
    {13,  15,  15,  18,  18,  18,  23,  25,  25},  // 3
    {13,  15,  15,  18,  18,  18,  23,  25,  25},  // 4
    {13,  15,  15,  18,  18,  18,  23,  25,  25},  // 5
    {13,  15,  15,  18,  18,  18,  23,  25,  25},  // 6
    {13,  15,  15,  18,  18,  18,  23,  25,  25},  // 7
    {13,  15,  15,  18,  18,  18,  23,  25,  25},  // 8
    {13,  15,  15,  18,  18,  18,  23,  25,  25},  // 9
    {14,  15,  15,  18,  18,  18,  23,  25,  25},  // 10
    {14,  15,  15,  18,  18,  18,  23,  25,  25},  // 11
    {14,  15,  15,  18,  18,  18,  23,  25,  25},  // 12
    {14,  15,  15,  18,  18,  18,  23,  25,  25},  // 13
    {14,  15,  15,  18,  18,  18,  23,  25,  25},  // 14
    {14,  15,  15,  18,  18,  18,  23,  25,  25},  // 15
    {14,  15,  15,  18,  18,  18,  23,  25,  25},  // 16
    {14,  15,  15,  18,  18,  18,  23,  25,  25},  // 17
    {14,  15,  15,  18,  18,  18,  23,  25,  25},  // 18
    {14,  15,  15,  18,  18,  18,  23,  25,  25},  // 19
    {14,  15,  15,  18,  18,  18,  23,  30,  30},  // 20
    {19,  20,  20,  23,  23,  23,  28,  30,  30},  // 21
    {19,  20,  20,  23,  23,  23,  28,  30,  30},  // 22
    {19,  20,  20,  23,  23,  23,  28,  30,  30},  // 23
    {19,  20,  20,  23,  23,  23,  28,  30,  30},  // 24
    {19,  20,  20,  23,  23,  23,  28,  30,  30},  // 25
    {19,  20,  20,  23,  23,  23,  28,  30,  30},  // 26
    {19,  20,  20,  23,  23,  23,  28,  30,  30},  // 27
    {19,  20,  20,  23,  23,  23,  28,  30,  30},  // 28
    {19,  20,  20,  23,  23,  23,  28,  30,  30},  // 29
    {20,  20,  20,  23,  23,  23,  28,  31,  31},  // 30
    {21,  25,  25,  24,  24,  24,  29,  32,  32},  // 31
    {22,  26,  26,  25,  25,  25,  30,  33,  33},  // 32
    {23,  27,  27,  26,  26,  26,  31,  34,  34},  // 33
    {24,  28,  28,  27,  27,  27,  32,  35,  35},  // 34
    {25,  29,  29,  30,  30,  30,  33,  36,  36},  // 35
    {30,  35,  35,  35,  35,  35,  34,  37,  37},  // 36
    {40,  40,  40,  40,  40,  40,  35,  38,  38},  // 37
    {45,  45,  45,  45,  45,  45,  45,  39,  39},  // 38
    {50,  50,  50,  50,  50,  50,  50,  50,  50},  // 39
    {55,  55,  55,  55,  55,  55,  55,  55,  55},  // 40
    {60,  60,  60,  60,  60,  60,  60,  60,  60},  // 41
    {65,  65,  65,  65,  65,  65,  65,  65,  65},  // 42
    {70,  70,  70,  70,  70,  70,  70,  70,  70},  // 43
    {75,  75,  75,  75,  75,  75,  75,  75,  75},  // 44
    {80,  80,  80,  80,  80,  80,  80,  80,  80},  // 45
    {85,  85,  85,  85,  85,  85,  85,  85,  85},  // 46
    {90,  90,  90,  90,  90,  90,  90,  90,  90},  // 47
    {95,  95,  95,  95,  95,  95,  95,  95,  95},  // 48
    {100, 100, 100, 100, 100, 100, 100, 100, 100}, // 49
    {100, 100, 100, 100, 100, 100, 100, 100, 100}, // 50
};
*/

int expfactor(CharData *ch)
{
    int percent;

    // Multiply the player's experience by a percent of experience that they will
    // actually receive.  This is a penalty that remort races experience.
    if (!IS_REMORT(ch) || GET_LEVEL(ch) > 50)
        percent = 100;
    else
        percent = expScale[GET_LEVEL(ch)];

    // Undead experience a small experience penalty since they are experience machines
    if (GET_RACE(ch) == RACE_UNDEAD)
        percent = MAX(1, percent - (GET_LEVEL(ch) / 3));
    if (GET_RACE(ch) == RACE_GIANT)
        percent = MIN(100, percent + 5);

    return percent;
}
 
void SET_RACE(CharData *ch, int race) {
    ch->player.race = race;
}

int GET_RACE(CharData *ch) {
    return ch->player.race;
}

/** sub_race stored in player_specials of PC or mob_specials of NPC. */
int GET_SUBRACE(CharData *ch) {
    if(IS_NPC(ch))
        return (ch)->mob_specials.sub_race;
    else
        return (ch)->player_specials->saved.sub_race;
}

void SET_SUBRACE(CharData *ch, int srace) {
    if(IS_NPC(ch))
        (ch)->mob_specials.sub_race = srace;
    else
        (ch)->player_specials->saved.sub_race = srace;
}





