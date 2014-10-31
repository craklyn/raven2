/* ************************************************************************
*   File: utility.c                                     Part of CircleMUD *
*  Usage: various internal functions of a utility nature                  *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "util/utils.h"
#include "general/comm.h"
#include "general/modify.h"
#include "general/color.h"
#include "general/class.h"
#include "magic/spells.h"
#include "general/handler.h"
#include "util/weather.h"
#include "actions/interpreter.h"
#include "magic/skills.h"
#include "magic/sing.h"
#include "magic/magic.h"
#include "actions/act.clan.h"
#include "actions/fight.h"


/* 128 bit */
void sprintbitarray(int bitvector[], char *names[], int maxar, char *result)
{
  int nr, teller, found = FALSE;

  *result = '\0';

  for(teller = 0; teller < maxar && !found; teller++)
    for (nr = 0; nr < 32 && !found; nr++) {
      if (IS_SET_AR(bitvector, (teller*32)+nr)) {
        if (*names[(teller*32)+nr] != '\n') {
          if (*names[(teller*32)+nr] != '\0') {
            strcat(result, names[(teller*32)+nr]);
            strcat(result, " ");
          }
        } else {
          strcat(result, "UNDEFINED ");
        }
      }
      if (*names[(teller*32)+nr] == '\n')
        found = TRUE;
    }

  if (!*result)
    strcpy(result, "NOBITS ");
}


/* Mortius :for creating a corpse file */
void obj_save_items(FILE *fp, ObjData *obj);
char *strlower(char *string);

extern void clear_char(CharData *); /* formake_ghost */
extern CharData *create_char(void);

int MIN(int a, int b)
{
  return a < b ? a : b;
}


int MAX(int a, int b)
{
  return a > b ? a : b;
}

int
clamp( int value, int minimum, int maximum )
{
    if( value < minimum )      return( minimum );
    else if( value > maximum ) return( maximum );
    else                       return( value );
}

/* creates a random number in interval [from;to] */
int number(int from, int to)
{
    if( from >= to )
        return( from );
    else
        return(( random() % (to - from + 1)) + from);
}

int
percentSuccess( int percentage )
{
    return( percentage >= number(1, 100) );
}

/* simulates dice roll */
int dice(int number, int size)
{
  int r;
  int sum = 0;

  if (size < 1)
  {
	mudlog(NRM, LVL_IMMORT, TRUE, "SYSERR: invalid dice size(%d) passed to dice in utils.c", size);
	size = 1;
  }

  for (r = 1; r <= number; r++)
    sum += ((random() % size) + 1);
  return (sum);
}


/* Determines if the string passed in is actually a valid integer value. Vex.*/
int is_integer(const char* s1)
{
    int i;
    char a1[MAX_INPUT_LENGTH];

    if (!*s1) return 0;
    strxfrm(a1, s1, strlen(s1));
    if (!(isdigit(a1[0]) || (a1[0] == '+') || (a1[0] == '-'))) return 0;
    for(i = 1; i < strlen(s1); i++)  if (!isdigit(a1[i])) return 0;
    return 1;
}

/* Create a duplicate of a string */
char *str_dup(const char *source)
{
  char *new;

  if (!source) return NULL;
  CREATE(new, char, strlen(source) + 1);
  return (strcpy(new, source));
}


#ifndef str_cmp
/* returns: 0 if equal, 1 if arg1 > arg2, -1 if arg1 < arg2  */
/* scan 'till found different or end of both                 */
int str_cmp(char *arg1, char *arg2)
{
  int chk, i;

  for (i = 0; *(arg1 + i) || *(arg2 + i); i++)
    if ((chk = LOWER(*(arg1 + i)) - LOWER(*(arg2 + i)))) {
      if (chk < 0)
	return (-1);
      else
	return (1);
    }
  return (0);
}
#endif

#ifndef strn_cmp
/* returns: 0 if equal, 1 if arg1 > arg2, -1 if arg1 < arg2  */
/* scan 'till found different, end of both, or n reached     */
int strn_cmp(char *arg1, char *arg2, int n)
{
  int chk, i;

  for (i = 0; (*(arg1 + i) || *(arg2 + i)) && (n > 0); i++, n--)
    if ((chk = LOWER(*(arg1 + i)) - LOWER(*(arg2 + i)))) {
      if (chk < 0)
	return (-1);
      else
	return (1);
    }

  return (0);
}
#endif

/* string manipulation fucntion originally by Darren Wilson */
/* (wilson@shark.cc.cc.ca.us) improved and bug fixed by Chris (zero@cnw.com) */
/* completely re-written again by M. Scott 10/15/96 (scottm@workcommn.net), */
/* substitute appearances of 'pattern' with 'replacement' in string */
/* and return the # of replacements */
int replace_str(char **string, char *pattern, char *replacement, int rep_all,
      int max_size) {
   char *replace_buffer = NULL;
   char *flow, *jetsam, temp;
   int len, i;

   if ((strlen(*string) - strlen(pattern)) + strlen(replacement) > max_size)
     return -1;

   CREATE(replace_buffer, char, max_size);
   i = 0;
   jetsam = *string;
   flow = *string;
   *replace_buffer = '\0';
   if (rep_all) {
      while ((flow = (char *)strstr(flow, pattern)) != NULL) {
   i++;
   temp = *flow;
   *flow = '\0';
   if ((strlen(replace_buffer) + strlen(jetsam) + strlen(replacement)) > max_size) {
      i = -1;
      break;
   }
   strcat(replace_buffer, jetsam);
   strcat(replace_buffer, replacement);
   *flow = temp;
   flow += strlen(pattern);
   jetsam = flow;
      }
      strcat(replace_buffer, jetsam);
   }
   else {
      if ((flow = (char *)strstr(*string, pattern)) != NULL) {
   i++;
   flow += strlen(pattern);
   len = ((char *)flow - (char *)*string) - strlen(pattern);

   strncpy(replace_buffer, *string, len);
   strcat(replace_buffer, replacement);
   strcat(replace_buffer, flow);
      }
   }
   if (i == 0) return 0;
   if (i > 0) {
      RECREATE(*string, char, strlen(replace_buffer) + 3);
      strcpy(*string, replace_buffer);
   }
   free(replace_buffer);
   return i;
}

void
sendChar( CharData *ch, char *fmt, ... )
{
  static char theBuffer[8192]; /* Potential problem if buf is too long */
  va_list args;

  va_start( args, fmt );
  vsprintf( theBuffer, fmt, args );
  va_end( args );

  send_to_char( theBuffer, ch );
}

void
quest_echo(char *msg)
{
  DescriptorData *questor;

  for( questor = descriptor_list; questor; questor = questor->next )
  {
    if( !questor->connected &&
         questor->character &&
         PRF_FLAGGED( questor->character, PRF_QUEST ))
      send_to_char( msg, questor->character);
  }
}

void
gecho(char *fmt, ...)
{
    static char theBuffer[8192]; /* Potential problem if buf is too long */
    DescriptorData *d;
    va_list args;

    va_start( args, fmt );
    vsprintf( theBuffer, fmt, args );
    va_end( args );

    for (d = descriptor_list; d; d = d->next)
        if (!d->connected && d->character)
            send_to_char(theBuffer, d->character);
}

/* This function removes flag game objects from the object list passed in. */
/* Its effect varies according to whether the death was due to a death trap */
/* or not. Based on seek_lootlist in seek.c */

#define IS_FLAG_STANDARD(o) ((GET_OBJ_VNUM(o) == GOLD_TEAM_STANDARD) || (GET_OBJ_VNUM(o) == BLACK_TEAM_STANDARD))
#define IS_FLAG_KEY(o) ((GET_OBJ_VNUM(o) == GOLD_TEAM_KEY) || (GET_OBJ_VNUM(o) == BLACK_TEAM_KEY))
#define IS_FLAG_ITEM(o) (IS_FLAG_STANDARD(o) ||  IS_FLAG_KEY(o))

void remove_flag_items(CharData *ch, ObjData *objlist, ObjData *cont, int death_trap)
{
	ObjData *nextobj, *obj;
	for (obj = objlist; obj; obj = nextobj) {
		nextobj = obj->next_content; /* save to avoid corruption */
		if (IS_FLAG_ITEM(obj)) {
		    /* Remove the item from where ever it is. */
		    if (!cont) {
			/* Get item from inventory. */
			if (!death_trap || IS_FLAG_STANDARD(obj))
			{
			    obj_from_char(obj);
			    if (death_trap) {
				if (GET_OBJ_VNUM(obj) == BLACK_TEAM_STANDARD)
				{
				    obj_to_room(obj, real_room(BLACK_STANDARD_START_ROOM));
                    quest_echo( "&08FLAG GAME: &00 &11The&00 &07Black&00 &11Flag has been returned.&00\r\n" );
				    sendChar(ch, "You have lost the black standard!\r\n");
				}
				else if (GET_OBJ_VNUM(obj) == GOLD_TEAM_STANDARD)
				{
				    obj_to_room(obj, real_room(GOLD_STANDARD_START_ROOM));
                    quest_echo( "&08FLAG GAME:&00 &11The&00 &10Gold&00 &11Flag has been returned.&00\r\n" );
				    sendChar(ch, "You have lost the gold standard!\r\n");
				}
				else
				{
				    obj_to_char(obj, ch);
				    sendChar(ch, "*remove_flag_items logic error!!*\r\n");
				    mudlog(NRM, LVL_IMMORT, TRUE, "remove_flag_items called with death_trap TRUE and invoked for item thats not a standard(on %s)!!!", GET_NAME(ch));
				}
			    }
			    else {
			        obj_to_room(obj, ch->in_room);
				act("$n drops $p to the ground.", TRUE, ch, obj, NULL, TO_ROOM);
		            }

			    mudlog(NRM, LVL_IMMORT, TRUE, "Removed flag item from %s.", GET_NAME(ch));
			} /* if (!death_trap || IS_FLAG_STANDARD) */
		    } /* if (!cont) */
		    else {
			/* Get item from container. */
			if (!death_trap || IS_FLAG_STANDARD(obj)) {
			    obj_from_obj(obj);
			    if (death_trap) {
				if (GET_OBJ_VNUM(obj) == BLACK_TEAM_STANDARD)
				{
				    obj_to_room(obj, real_room(BLACK_STANDARD_START_ROOM));
                    quest_echo( "&08FLAG GAME:&00 &11The&00 &07Black&00 &11Flag has been returned.&00\r\n" );
                    sendChar(ch, "You have lost the black standard!\r\n");
				}
				else if (GET_OBJ_VNUM(obj) == GOLD_TEAM_STANDARD)
				{
				    obj_to_room(obj, real_room(GOLD_STANDARD_START_ROOM));
                    quest_echo( "&08FLAG GAME:&00 &11The&00 &10Gold&00 &11Flag has been returned.&00\r\n" );
				    sendChar(ch, "You have lost the gold standard!\r\n");
				}
				else
				{
				    obj_to_char(obj, ch);
				    sendChar(ch, "*remove_flag_items logic error!!*\r\n");
				    mudlog(NRM, LVL_IMMORT, TRUE, "remove_flag_items called with death_trap TRUE and invoked for item thats not a standard(on %s)!!!", GET_NAME(ch));
				}
			    } /* if (death_trap) */
			    else {
				obj_to_room(obj, ch->in_room);
				act("$n drops $p to the ground.", TRUE, ch, obj, NULL, TO_ROOM);
			    }
			    mudlog(NRM, LVL_IMMORT, TRUE, "Removed flag item from container carried by %s.", GET_NAME(ch));
		        } /* if (!death_trap ... ) */
		    } /* else  */
		} /* if (IS_FLAG_ITEM) */
		else {
		    /* Maybe they hid it in a container. */
		    if (GET_OBJ_TYPE(obj) == ITEM_CONTAINER)
			remove_flag_items(ch, obj->contains, obj, death_trap);
		} /* else */
	}
}

/* This function removes any flag items being worn or in containers worn by
   the character. */
void unequip_flag_items(CharData *ch, int death_trap)
{
	int i;

	for (i = 0; i < NUM_WEARS; i++) {
		if (ch->equipment[i]) {
		    if (IS_FLAG_ITEM(ch->equipment[i])) {
			/* unequip this item */
			if (!death_trap)
			{
			    act("$n drops $p to the ground.", TRUE, ch, ch->equipment[i], NULL, TO_ROOM);
			    obj_to_room(unequip_char(ch, i), ch->in_room);
			}
			else if (IS_FLAG_STANDARD(ch->equipment[i])) {
				if (GET_OBJ_VNUM(ch->equipment[i]) == BLACK_TEAM_STANDARD)
				{
				    obj_to_room(unequip_char(ch, i), real_room(BLACK_STANDARD_START_ROOM));
                    quest_echo( "&08FLAG GAME:&00 &11The&00 &07Black&00 &11Flag has been returned.&00\r\n" );
				    sendChar(ch, "You have lost the black standard!\r\n");
				}
				else if (GET_OBJ_VNUM(ch->equipment[i]) == GOLD_TEAM_STANDARD)
				{
				    obj_to_room(unequip_char(ch, i), real_room(GOLD_STANDARD_START_ROOM));
                    quest_echo( "&08FLAG GAME:&00 &11The&00 &10Gold&00 &11Flag has been returned.&00\r\n" );
				    sendChar(ch, "You have lost the gold standard!\r\n");
				}
				else
				{
				    sendChar(ch, "*remove_flag_items logic error!!*\r\n");
				    mudlog(NRM, LVL_IMMORT, TRUE, "remove_flag_items called with death_trap TRUE and invoked for item thats not a standard(on %s)!!!", GET_NAME(ch));
				}
			    }
		    }
		    else if (GET_OBJ_TYPE(ch->equipment[i]) == ITEM_CONTAINER) {
			/* Might be hiding it in something they are wearing. */
			remove_flag_items(ch, ch->equipment[i]->contains, ch->equipment[i], death_trap);
		    }
		}
	}
}


/* Place a character killed in the quest field in jail. */
/* The constants used here are defined in utils.h */
/* This function is called in act.movement.c and fight.c - Vex. */
void jail_char(CharData *ch, int death_trap)
{
    #define STUN_MIN 3
	int jail_room;

	if (IS_NPC(ch)) {
		mudlog(NRM, LVL_IMMORT, TRUE, "jail_char(utils.c) called for NPC %s!!", GET_NAME(ch));
		return;
	}
	if (!IN_QUEST_FIELD(ch)) {
		sendChar(ch, "FLAG GAME LOGIC ERROR!(1)\r\n");
		mudlog(NRM, LVL_IMMORT, TRUE, "jail_char(utils.c) called, but %s not in the quest field!", GET_NAME(ch));
		return;
	}
	if (!PRF_FLAGGED(ch, PRF_GOLD_TEAM) && !PRF_FLAGGED(ch, PRF_BLACK_TEAM) && !PRF_FLAGGED(ch, PRF_ROGUE_TEAM)) {
		sendChar(ch, "FLAG GAME LOGIC ERROR(2)!\r\n");
		mudlog(NRM, LVL_IMMORT, TRUE, "jail_char(utils.c) called for %s, but they are not in a team!", GET_NAME(ch));
		return;
	}
	if (death_trap) {
		sendChar(ch, "I love the smell of broiled adventurer in the morning.\r\n");
		remove_flag_items(ch, ch->carrying, NULL, TRUE);
		unequip_flag_items(ch, TRUE);
	}

	if (PRF_FLAGGED(ch, PRF_GOLD_TEAM))
		jail_room = real_room(GOLD_TEAM_JAIL);
	else if (PRF_FLAGGED(ch, PRF_BLACK_TEAM))
		jail_room = real_room(BLACK_TEAM_JAIL);
	else if (PRF_FLAGGED(ch, PRF_ROGUE_TEAM))
		jail_room = real_room(ROGUE_TEAM_JAIL);

        act( "$n is vaporized.", TRUE, ch, 0, 0, TO_ROOM );
	/* Transfer flag game items to the ground. */
	remove_flag_items(ch, ch->carrying, NULL, FALSE);
	unequip_flag_items(ch, FALSE);

	GET_HIT(ch) = GET_MAX_HIT(ch)/2;
	GET_MANA(ch) = 1;
	char_from_room(ch);
	if (FIGHTING(ch)) stop_fighting(ch);
	/* stop followers too */
	char_to_room(ch, jail_room);
        cast_spell( ch, ch, NULL, SPELL_CLEANSE);
	look_at_room(ch, 0);
	GET_POS(ch) = POS_RESTING;
	STUN_USER_MIN;
	if (!PLR_FLAGGED(ch, PLR_JAILED))
		SET_BIT_AR(PLR_FLAGS(ch), PLR_JAILED);
	(ch)->player_specials->saved.jail_timer = JAIL_TIME; /* Timer decremented in limits.c */
}

// Make a character into a ghost(kills the character)
// This is not really made for NPC's
//
void
make_ghost( CharData* ch )
{
  CharData* ghost = NULL;
  ObjData*  obj   = NULL;
  ObjData*  next_obj = NULL;

  char ghostDescr[80] = "";
  char ghostName[80]  = "";

  int j;

  // Don't ghost NPCs
  //
  if( IS_NPC(ch) ){ return; }

  // Don't create a ghost if this is an arena.
  //
  if( IN_ARENA(ch) )
  {
    int target_room = real_room( ARENA_RM(ch) );
    act( "$n is vaporized.", TRUE, ch, 0, 0, TO_ROOM );
    char_from_room( ch );
    if(FIGHTING(ch)) stop_fighting( ch );
    char_to_room( ch, target_room );
    GET_HIT ( ch ) = 1;
    GET_POS(ch) = POS_RESTING;
    act( "$n materializes before you.", TRUE, ch, 0, 0, TO_ROOM );
    look_at_room( ch, 0 );
    return;
  }

  // Create the  ghost and add it to the list.
  //
  ghost = create_char();

  ghost->nr              = 0;
  ghost->in_room         = ch->in_room;
  ghost->was_in_room     = NOWHERE;
  ghost->player_specials =  &dummy_mob;

  strcpy( ghostName, "PC_CORPSE " );
  strcat( ghostName, ch->player.name );
  ghost->player.name     = strdup( ghostName );

  strcpy( ghostDescr, ch->player.name );
  strcat( ghostDescr, "'s ghost" );
  ghost->player.short_descr = strdup( ghostDescr );
  ghost->player.long_descr  = NULL;
  ghost->player.title       = NULL;

  SET_BIT_AR(MOB_FLAGS(ghost), MOB_ISNPC);
  SET_BIT_AR(MOB_FLAGS(ghost), MOB_MEMORY);
  SET_BIT_AR(MOB_FLAGS(ghost), MOB_NOCHARM);
  SET_BIT_AR(AFF_FLAGS(ghost), AFF_FLY);
  GET_ALIGNMENT(ghost)  =  GET_ALIGNMENT(ch);
  GET_LEVEL(ghost)      = GET_LEVEL(ch);
  GET_EXP(ghost)        = GET_LEVEL(ch)*500;
  GET_MAX_HIT(ghost)    = GET_MAX_HIT(ch);
  GET_MAX_MANA(ghost)   = GET_MAX_MANA(ch);
  GET_MAX_MOVE(ghost)   =  GET_MAX_MOVE(ch);
  GET_HIT(ghost)        = GET_MAX_HIT(ghost);
  GET_MANA(ghost)       = GET_MAX_MANA(ghost);
  GET_MOVE(ghost)       = GET_MAX_MOVE(ghost);

  ghost->points.hitroll           = 0;
  ghost->points.armor             = 0;
  ghost->mob_specials.damnodice   = 1;
  ghost->mob_specials.damsizedice = GET_LEVEL(ch);
  SET_DAMROLL(ghost, 0);
  ghost->mob_specials.attack_type = 15;
  ghost->char_specials.position   = POS_STANDING;
  ghost->mob_specials.default_pos = POS_STANDING;

  GET_SEX(ghost)    = GET_SEX(ch);
  GET_CLASS(ghost)  = GET_CLASS(ch);
  GET_WEIGHT(ghost) = GET_WEIGHT(ch);
  GET_HEIGHT(ghost) = GET_HEIGHT(ch);
  SET_RACE(ghost, RACE_UNDEAD);

  for (j = 0;j < 3; j++) GET_COND(ghost, j) = -1;
  for (j = 0;j < 5; j++) GET_SAVE(ghost, j) = 0;

  roll_real_abils(ghost);

  // Transfer the inventory
  //
  for (obj = ch->carrying; obj; obj = next_obj)
  {
    next_obj = obj->next_content;
    obj_from_char(obj);
    obj_to_char(obj, ghost);
  }

  // transfer character's equipment to the ghost */
  for( j = 0; j < NUM_WEARS; j++ )
  {
    if( ch->equipment[j] )
    {
      equip_char(ghost, unequip_char(ch, j), j);
    }
  }

  if( GET_GOLD(ch) > 0 ){ GET_GOLD(ghost) = GET_GOLD(ch); }
  GET_GOLD(ch)      = 0;
  ch->carrying      = NULL;
  IS_CARRYING_N(ch) = 0;
  IS_CARRYING_W(ch) = 0;

  sendChar( ch, "&08You are DEAD!&00\r\n" );
  gain_exp( ch,  -MIN(1000000, (GET_LEVEL(ch) * GET_LEVEL(ch) * 500)));
  mudlog(NRM, LVL_IMMORT, TRUE, "(GHOST) %s turned into a ghost.", GET_NAME(ch) );
  extract_char(ch);
  char_to_room(ghost, ghost->in_room);
}


/* writes a string to the log
void mlog(char *str)
{
  time_t ct;
  char *tmstr;

  ct = time(0);
  tmstr = asctime(localtime(&ct));
  *(tmstr + strlen(tmstr) - 1) = '\0';
  fprintf(stderr, "%-19.19s :: %s\n", tmstr, str);
  fflush(stderr);
}
*/
/** New variable argument log() function; logs messages to disk.
 * Works the same as the old for previously written code but is very nice
 * if new code wishes to implment printf style log messages without the need
 * to make prior sprintf calls.
 * @param format The message to log. Standard printf formatting and variable
 * arguments are allowed.
 * @param args The comma delimited, variable substitutions to make in str. */
void basic_mud_vlog(const char *format, va_list args)
{
  time_t ct = time(0);
  char *time_s = asctime(localtime(&ct));

  if (logfile == NULL) {
    puts("SYSERR: Using log() before stream was initialized!");
    return;
  }

  if (format == NULL)
    format = "SYSERR: log() received a NULL format.";

  time_s[strlen(time_s) - 1] = '\0';

  fprintf(logfile, "%-15.15s :: ", time_s + 4);
  vfprintf(logfile, format, args);
  fputc('\n', logfile);
  fflush(logfile);
}

/** Log messages directly to syslog on disk, no display to in game immortals.
 * Supports variable string modification arguments, a la printf. Most likely
 * any calls to plain old log() have been redirected, via macro, to this
 * function.
 * @param format The message to log. Standard printf formatting and variable
 * arguments are allowed.
 * @param ... The comma delimited, variable substitutions to make in str. */
void basic_mud_log(const char *format, ...)
{
  va_list args;

  va_start(args, format);
  basic_mud_vlog(format, args);
  va_end(args);
}

/** Essentially the touch command. Create an empty file or update the modified
 * time of a file.
 * @param path The filepath to "touch." This filepath is relative to the /lib
 * directory relative to the root of the mud distribution.
 * @retval int 0 on a success, -1 on a failure; standard system call exit
 * values. */
int touch(const char *path)
{
  FILE *fl;

  if (!(fl = fopen(path, "a"))) {
    mlog("SYSERR: %s: %s", path, strerror(errno));
    return (-1);
  } else {
    fclose(fl);
    return (0);
  }
}

/** Log mud messages to a file & to online imm's syslogs.
 * @param type The minimum syslog level that needs be set to see this message.
 * OFF, BRF, NRM and CMP are the values from lowest to highest. Using mudlog
 * with type = OFF should be avoided as every imm will see the message even
 * if they have syslog turned off.
 * @param level Minimum character level needed to see this message.
 * @param file TRUE log this to the syslog file, FALSE do not log this to disk.
 * @param str The message to log. Standard printf formatting and variable
 * arguments are allowed.
 * @param ... The comma delimited, variable substitutions to make in str. */
void mudlog(int type, int level, int file, const char *str, ...)
{
  char buf[MAX_STRING_LENGTH];
  struct descriptor_data *i;
  va_list args;

  if (str == NULL)
    return;	/* eh, oh well. */

  if (file) {
    va_start(args, str);
    basic_mud_vlog(str, args);
    va_end(args);
  }

  if (level < 0)
    return;

  strcpy(buf, "[ ");	/* strcpy: OK */
  va_start(args, str);
  vsnprintf(buf + 2, sizeof(buf) - 6, str, args);
  va_end(args);
  strcat(buf, " ]\r\n");	/* strcat: OK */

  for (i = descriptor_list; i; i = i->next) {
    if (STATE(i) != CON_PLAYING || IS_NPC(i->character)) /* switch */
      continue;
    if (GET_LEVEL(i->character) < level)
      continue;
    if (PLR_FLAGGED(i->character, PLR_WRITING))
      continue;
    if (type > (PRF_FLAGGED(i->character, PRF_LOG1) ? 1 : 0) + (PRF_FLAGGED(i->character, PRF_LOG2) ? 2 : 0))
      continue;

    sendChar(i->character, "%s%s%s", CCGRN(i->character, C_NRM), buf, CCNRM(i->character, C_NRM));
      }
    }

/* End of Modification */



void
sprintbit( u_int vektor, char *names[], char *result)
{
  long nr;

  *result = '\0';

  if( vektor < 0 )
  {
    strcpy(result, "SPRINTBIT ERROR!");
    return;
  }
  for( nr = 0; vektor; vektor >>= 1 )
  {
    if( IS_SET(1, vektor))
    {
      if( *names[nr] != '\n' )
      {
	strcat(result, names[nr]);
	strcat(result, " ");
      }
      else
	strcat(result, "UNDEFINED ");
    }
    if( *names[nr] != '\n' )
      nr++;
  }

  if (!*result)
    strcat(result, "NOBITS ");
}



/** Return the human readable name of a defined type. This sprinttype has error
 * checking for buffer overflows and updated from a previous version.
 * @pre The final element in the names array must contain a one character
 * string consisting of a single newline character "\n". Caller of function is
 * responsible for creating the memory buffer for the result string.
 * @param[in] type The type number to be translated.
 * @param[in] names An array of human readable strings describing each possible
 * bit. The final element in this array must be a string made of a single
 * newline character (eg "\n").
 * @param[out] result Holds the translated name of the type.
 * Caller of sprintbit is responsible for creating the buffer for result.
 * Will be set to "UNDEFINED" if the type is greater than the number of names
 * available.
 * @param[in] reslen The length of the available memory in the result buffer.
 * @retval size_t The length of the string copied into result. */

size_t sprinttype(int type, const char *names[], char *result, size_t reslen)
{
  int nr = 0;

  while (type && *names[nr] != '\n') {
    type--;
    nr++;
  }

  return strlcpy(result, *names[nr] != '\n' ? names[nr] : "UNDEFINED", reslen);
}

/* Calculate the REAL time passed over the last t2-t1 centuries (secs) */
struct time_info_data real_time_passed(time_t t2, time_t t1)
{
  long secs;
  struct time_info_data now;

  secs = (long) (t2 - t1);

  now.hours = (secs / SECS_PER_REAL_HOUR) % 24;	/* 0..23 hours */
  secs -= SECS_PER_REAL_HOUR * now.hours;

  now.day = (secs / SECS_PER_REAL_DAY);	/* 0..34 days  */
  secs -= SECS_PER_REAL_DAY * now.day;

  now.month = -1;
  now.year = -1;

  return now;
}



/* Calculate the MUD time passed over the last t2-t1 centuries (secs) */
struct time_info_data mud_time_passed(time_t t2, time_t t1)
{
  long secs;
  struct time_info_data now;

  secs = (long) (t2 - t1);

  now.hours = (secs / SECS_PER_MUD_HOUR) % 24;	/* 0..23 hours */
  secs -= SECS_PER_MUD_HOUR * now.hours;

  now.day = (secs / SECS_PER_MUD_DAY) % 35;	/* 0..34 days  */
  secs -= SECS_PER_MUD_DAY * now.day;

  now.month = (secs / SECS_PER_MUD_MONTH) % 16;	/* 0..15 months */
  secs -= SECS_PER_MUD_MONTH * now.month;

  now.year = (secs / SECS_PER_MUD_YEAR);	/* 0..XX? years */

  return now;
}



struct time_info_data age(CharData * ch)
{
  struct time_info_data player_age;

  player_age = mud_time_passed(time(0), ch->player.time.birth);

  player_age.year += 17;	/* All players start at 17 */

  return player_age;
}

/* Check if making CH follow VICTIM will create an illegal */
/* Follow "Loop/circle"                                    */
bool circle_follow(CharData * ch, CharData * victim)
{
  CharData *k;

  for (k = victim; k; k = k->master) {
    if (k == ch)
      return TRUE;
  }

  return FALSE;
}


/*
** This function is called to see how many people are
** attacking a player.  Used for how easy it is to run away.
*/
int attackers(CharData * ch) {
	CharData *victim;
	int count = 0;

	victim = world[ch->in_room].people;

	while (victim) {
		if (FIGHTING(victim) && FIGHTING(victim) == ch && IS_PVP(victim, FIGHTING(victim)) )
			count++;
		victim = victim->next_in_room;
	}

	return count;
}



/*
** Called when stop following persons, or stopping charm
** This will NOT do if a character quits/dies!!
*/
void
stop_follower(CharData * ch)
{
    FollowType *j, *k;

  if( ch == NULL )
  {
    mudlog(NRM, LVL_IMMORT, TRUE, "SYSERR: stop_follower passed a NULL ch pointer" );
    return;
  }

  if( ch->master == NULL )
  {
    mudlog(NRM, LVL_IMMORT, TRUE, "SYSERR: stop_follower passed a NULL master for %s", GET_NAME(ch));
    return;
  }

    /* After losing control of a conjured creature for what ever reason,
     ** you will be unable to summon another one for a while.
     ** Death knights creatures are affected in a different way
     ** (see animate dead in magic.c)
     */
    if( IS_NPC(ch) && !IS_NPC(ch->master ) &&
            IS_SET_AR(MOB_FLAGS(ch), MOB_CONJURED) &&
            GET_CLASS(ch->master) != CLASS_DEATH_KNIGHT)
    {
        if (GET_CLASS(ch->master) != CLASS_MAGIC_USER)
            switch( GET_CLASS(ch->master) )
            {
                case CLASS_RANGER:
                    act("The creatures of the forest will no longer answer your call!", FALSE, 0, 0, ch->master, TO_VICT);
                    break;
                case CLASS_NECROMANCER:
                    act("The dead will no longer heed your summons!", FALSE, 0, 0,
                            ch->master, TO_VICT);
                    break;
                default:
                    act("Conjured creatures will no longer answer your summons!", FALSE, 0, 0, ch->master, TO_VICT);
                    break;
            }

            // Duration dependent on how powerful creature was to begin with.
            if(ch->timer < TIMER_MAXTIMER)
                GET_CONJ_CNT(ch->master,ch->timer) = GET_LEVEL(ch)/5;
    }

    if (affected_by_spell(ch, SKILL_BEFRIEND)) {
        act("$n looks at $N with contempt.", FALSE, ch, 0, ch->master,
                TO_NOTVICT);
        act("$n looks at you with contempt.", FALSE, ch, 0, ch->master,
                TO_VICT);
        affect_from_char(ch, SKILL_BEFRIEND);
    } else if (IS_AFFECTED(ch, AFF_CHARM)) {
        act("You realize that $N is a jerk!", FALSE, ch, 0, ch->master, TO_CHAR);
        act("$n realizes that $N is a jerk!", FALSE, ch, 0, ch->master, TO_NOTVICT);
        act("$n hates your guts!", FALSE, ch, 0, ch->master, TO_VICT);
        if (affected_by_spell(ch, SPELL_CHARM))
            affect_from_char(ch, SPELL_CHARM);
        else if (affected_by_spell(ch, SPELL_GATE))
            affect_from_char(ch, SPELL_GATE);
        else if (affected_by_spell(ch, SPELL_CONJURE_ELEMENTAL))
            affect_from_char(ch, SPELL_CONJURE_ELEMENTAL);
        else if (affected_by_spell(ch, SPELL_MONSTER_SUMMON))
            affect_from_char(ch, SPELL_MONSTER_SUMMON);
        else if (affected_by_spell(ch, SPELL_CALL_OF_THE_WILD))
            affect_from_char(ch, SPELL_CALL_OF_THE_WILD);
        else if (affected_by_spell(ch, SPELL_ANIMATE_DEAD))
            affect_from_char(ch, SPELL_ANIMATE_DEAD);
    } else {
        act( "You stop following $N.", FALSE, ch, 0, ch->master, TO_CHAR);
        if( GET_LEVEL(ch) < LVL_IMMORT )
        {
            act( "$n stops following $N.", TRUE, ch, 0, ch->master, TO_NOTVICT);
            if( CAN_SEE( ch->master, ch ))
            {
                act( "$n stops following you.", TRUE, ch, 0, ch->master, TO_VICT);
            }
        }
    }

    if( ch->master->followers->follower == ch )
    {
        k = ch->master->followers;
        ch->master->followers = k->next;
        free(k);
    }
    else
    {
        for (k = ch->master->followers; k->next->follower != ch; k = k->next);

        j = k->next;
        k->next = j->next;
        free(j);
    }

//128  REMOVE_BIT(AFF_FLAGS(ch), AFF_CHARM | AFF_GROUP);
    REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_CHARM);
    REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_GROUP);
    ch->master   = NULL;
    ch->guarding = NULL;

    /* if stalking, remove the affect */
    if (affected_by_spell(ch, SKILL_STALK))
        affect_from_char(ch, SKILL_STALK);

    if (affected_by_spell(ch, SPELL_CHARM_CORPSE)) {
        affect_from_char(ch, SPELL_CHARM_CORPSE);
    }

}



/*
** Called when a character that follows/is followed dies
*/
void
die_follower(CharData * ch)
{
  struct follow_type *j, *k;

  if (ch->master)
    stop_follower(ch);

  for (k = ch->followers; k; k = j) {
    j = k->next;
    stop_follower(k->follower);
  }
}



/*
** Do NOT call this before having checked if a circle of followers
** will arise. CH will follow leader
*/
void
add_follower(CharData * ch, CharData * leader)
{
  FollowType *k;

  if( ch == NULL )
  {
    mudlog(NRM, LVL_IMMORT, TRUE, "SYSERR: add_follower passed a NULL ch pointer" );
    return;
  }

  if( leader == NULL )
  {
    mudlog(NRM, LVL_IMMORT, TRUE, "SYSERR: add_follower passed a NULL leader pointer for %s", GET_NAME(ch) );
    return;
  }

  if( ch->master )
  {
    /* OLD WAY: assert(!ch->master); */
    mudlog(NRM, LVL_IMMORT, TRUE, "SYSERR: add_follower called with a master already set ?" );
    mudlog(NRM, LVL_IMMORT, TRUE, "SYSERR: ch is %s, leader is %s", GET_NAME(ch), GET_NAME(leader));
    return;
  }

  ch->master = leader;

  CREATE(k, struct follow_type, 1);

  k->follower = ch;
  k->next = leader->followers;
  leader->followers = k;

  act("You now follow $N.", FALSE, ch, 0, leader, TO_CHAR);
  if (CAN_SEE(leader, ch))
    act("$n starts following you.", TRUE, ch, 0, leader, TO_VICT);
  act("$n starts to follow $N.", TRUE, ch, 0, leader, TO_NOTVICT);
}

/*
 * get_line reads the next non-blank line off of the input stream.
 * The newline character is removed from the input.  Lines which begin
 * with '*' are considered to be comments.
 *
 * Returns the number of lines advanced in the file.
 */
int get_line(FILE * fl, char *buf)
{
  char temp[256];
  int lines = 0;

  /* initialise temp */
  temp[0] = '\0';

  do {
    lines++;
    fgets(temp, 256, fl);
    if (*temp)
      temp[strlen(temp) - 1] = '\0';
  } while (!feof(fl) && (*temp == '*' || !*temp));

  if (feof(fl))
    return 0;
  else {
    strcpy(buf, temp);
    return lines;
  }
}


int get_filename(char *orig_name, char *filename, int mode)
{
  char *prefix, *middle, *suffix, *ptr, name[64];

  switch (mode) {
  case CRASH_FILE:
    prefix = "plrobjs";
    suffix = "objs";
    break;
  case ETEXT_FILE:
    prefix = "plrtext";
    suffix = "text";
    break;
  case SCRIPT_VARS_FILE:
    prefix = "plrvars";
    suffix = "mem";
    break;
  case QUESTS_FILE:
    prefix = "quests";
    suffix = "qsts";
    break;
  default:
    return 0;
    break;
  }

  if (!*orig_name)
    return 0;

  strcpy(name, orig_name);
  for (ptr = name; *ptr; ptr++)
    *ptr = LOWER(*ptr);

  switch (LOWER(*name)) {
  case 'a':  case 'b':  case 'c':  case 'd':  case 'e':
    middle = "A-E";
    break;
  case 'f':  case 'g':  case 'h':  case 'i':  case 'j':
    middle = "F-J";
    break;
  case 'k':  case 'l':  case 'm':  case 'n':  case 'o':
    middle = "K-O";
    break;
  case 'p':  case 'q':  case 'r':  case 's':  case 't':
    middle = "P-T";
    break;
  case 'u':  case 'v':  case 'w':  case 'x':  case 'y':  case 'z':
    middle = "U-Z";
    break;
  default:
    middle = "ZZZ";
    break;
  }

  sprintf(filename, "%s/%s/%s.%s", prefix, middle, name, suffix);
  return 1;
}

/* strips \r's from line */
char *stripcr(char *dest, const char *src) {
   int i, length;
   char *temp;

   if (!dest || !src) return NULL;
   temp = &dest[0];
   length = strlen(src);
   for (i = 0; *src && (i < length); i++, src++)
     if (*src != '\r') *(temp++) = *src;
   *temp = '\0';
   return dest;
}

/* Returns the plural of a races name, e.g. elves instead of elfs */
void race_plural_string(int race, char *the_name)
{
    switch(race) {
    case RACE_HUMAN: sprintf(the_name, "humans"); break;
    case RACE_PLANT: sprintf(the_name, "plants"); break;
    case RACE_ANIMAL: sprintf(the_name, "animals"); break;
    case RACE_DRAGON: sprintf(the_name, "dragons"); break;
    case RACE_UNDEAD: sprintf(the_name, "undead"); break;
    case RACE_VAMPIRE: sprintf(the_name, "vampires"); break;
    case RACE_HALFLING: sprintf(the_name, "halflings"); break;
    case RACE_ELF: sprintf(the_name, "elves"); break;
    case RACE_DROW: sprintf(the_name, "drow"); break;
    case RACE_DWARF: sprintf(the_name, "dwarves"); break;
    case RACE_GIANT: sprintf(the_name, "giants"); break;
    case RACE_MINOTAUR: sprintf(the_name, "minotaurs"); break;
    case RACE_DEMON: sprintf(the_name, "demons"); break;
    case RACE_OGRE: sprintf(the_name, "ogres"); break;
    case RACE_TROLL: sprintf(the_name, "trolls"); break;
    case RACE_WEREWOLF: sprintf(the_name, "werewolves"); break;
    case RACE_ELEMENTAL: sprintf(the_name, "elementals"); break;
    case RACE_ORC: sprintf(the_name, "orcs"); break;
    case RACE_GNOME: sprintf(the_name, "gnomes"); break;
    case RACE_DRACONIAN: sprintf(the_name, "draconians"); break;
    case RACE_FAERIE: sprintf(the_name, "faeries"); break;
    case RACE_IZARTI: sprintf(the_name, "izarti"); break;
    case RACE_AMARA: sprintf(the_name, "amara"); break;
    case RACE_SHUMAN: sprintf(the_name, "shumans"); break;
    case RACE_SHALFLING: sprintf(the_name, "shalflings"); break;
    case RACE_SELF: sprintf(the_name, "selves"); break;
    case RACE_SDROW: sprintf(the_name, "sdrow"); break;
    case RACE_SDWARF: sprintf(the_name, "sdwarves"); break;
    case RACE_SMINOTAUR: sprintf(the_name, "sminotaurs"); break;
    case RACE_SOGRE: sprintf(the_name, "sogres"); break;
    case RACE_STROLL: sprintf(the_name, "strolls"); break;
    case RACE_SGNOME: sprintf(the_name, "sgnomes"); break;
    case RACE_SDRACONIAN: sprintf(the_name, "sdraconians"); break;
    case RACE_SORC: sprintf(the_name, "sorcs"); break;
    default:
        sprintf(the_name, "ERROR!(race_plural_string)");
    }
}

/*
** Tries to come up with an appropriate insulting name based on the race
** passed in. Vex.
*/
void race_insult_string(int race, char *the_name)
{
    int insult;

    switch(race) {
    case RACE_HUMAN:
    case RACE_SHUMAN:
        insult = number(1, 5);
        switch(insult) {
        case 1: sprintf(the_name, "goat herder"); break;
        case 2: sprintf(the_name, "sheep lover"); break;
        case 3: sprintf(the_name, "scum"); break;
        case 4:
        case 5: sprintf(the_name, "human"); break;
        default: sprintf(the_name, "DUMB CODER(race_insult_string)");
        }
        break;
    case RACE_PLANT:
        insult = number(1, 5);
        switch(insult) {
        case 1: sprintf(the_name, "veggie"); break;
        case 2: sprintf(the_name, "greenie"); break;
        case 3: sprintf(the_name, "woody"); break;
        case 4: sprintf(the_name, "asparagus"); break;
        case 5: sprintf(the_name, "barky"); break;
        default: sprintf(the_name, "DUMB CODER(race_insult_string)");
        }
        break;
    case RACE_ANIMAL:
        insult = number(1, 5);
        switch(insult) {
        case 1: sprintf(the_name, "furry"); break;
        case 2: sprintf(the_name, "smelly"); break;
        case 3:
        case 4:
        case 5: sprintf(the_name, "animal"); break;
        default: sprintf(the_name, "DUMB CODER(race_insult_string)");
        }
        break;
    case RACE_DRAGON:
        insult = number(1, 5);
        switch(insult) {
        case 1: sprintf(the_name, "scaly"); break;
        case 2: sprintf(the_name, "miser"); break;
        case 3:
        case 4:
        case 5: sprintf(the_name, "dragon"); break;
        default: sprintf(the_name, "DUMB CODER(race_insult_string)");
        }
        break;
    case RACE_UNDEAD:
        insult = number(1, 5);
        switch(insult) {
        case 1: sprintf(the_name, "dead dude"); break;
        case 2: sprintf(the_name, "rotten"); break;
        case 3: sprintf(the_name, "bones"); break;
        case 4: sprintf(the_name, "stinky"); break;
        case 5: sprintf(the_name, "foul beast"); break;
        default: sprintf(the_name, "DUMB CODER(race_insult_string)");
        }
        break;
    case RACE_VAMPIRE:
        insult = number(1, 5);
        switch(insult) {
        case 1: sprintf(the_name, "toothy"); break;
        case 2: sprintf(the_name, "pale skin"); break;
        case 3: sprintf(the_name, "sucker"); break;
        case 4: sprintf(the_name, "leech"); break;
        case 5: sprintf(the_name, "vampire"); break;
        default: sprintf(the_name, "DUMB CODER(race_insult_string)");
        }
        break;
    case RACE_HALFLING:
    case RACE_SHALFLING:
        insult = number(1, 5);
        switch(insult) {
        case 1: sprintf(the_name, "little guy"); break;
        case 2:
        case 3:
        case 4:
        case 5: sprintf(the_name, "shorty"); break;
        default: sprintf(the_name, "DUMB CODER(race_insult_string)");
        }
        break;
    case RACE_ELF:
    case RACE_SELF:
        insult = number(1, 6);
        switch(insult) {
        case 1: sprintf(the_name, "faery"); break;
        case 2: sprintf(the_name, "pansie"); break;
        case 3: sprintf(the_name, "Janthalanus"); break;
        case 4: sprintf(the_name, "goldie-locks"); break;
        case 5: sprintf(the_name, "elf"); break;
        case 6: sprintf(the_name, "tree hugger"); break;
        default: sprintf(the_name, "DUMB CODER(race_insult_string)");
        }
        break;
    case RACE_DROW:
    case RACE_SDROW:
        insult = number(1, 4);
        switch(insult) {
        case 1: sprintf(the_name, "inky"); break;
        case 2: sprintf(the_name, "spider-lover"); break;
        case 3: sprintf(the_name, "drow"); break;
        case 4: sprintf(the_name, "arachnid"); break;
        default: sprintf(the_name, "DUMB CODER(race_insult_string)");
        }
        break;
    case RACE_DWARF:
    case RACE_SDWARF:
        insult = number(1, 5);
        switch(insult) {
        case 1: sprintf(the_name, "stumpy-legs"); break;
        case 2: sprintf(the_name, "beardless"); break;
        case 3: sprintf(the_name, "hairy"); break;
        case 4: sprintf(the_name, "shorty"); break;
        case 5: sprintf(the_name, "dwarf"); break;
        default: sprintf(the_name, "DUMB CODER(race_insult_string)");
        }
        break;
    case RACE_GIANT:
        insult = number(1, 5);
        switch(insult) {
        case 1: sprintf(the_name, "boulder brain"); break;
        case 2:
        case 3:
        case 4:
        case 5: sprintf(the_name, "giant"); break;
        default: sprintf(the_name, "DUMB CODER(race_insult_string)");
        }
        break;
    case RACE_MINOTAUR:
    case RACE_SMINOTAUR:
        insult = number(1, 5);
        switch(insult) {
        case 1: sprintf(the_name, "moo"); break;
        case 2: sprintf(the_name, "cow"); break;
        case 3: sprintf(the_name, "calf"); break;
        case 4: sprintf(the_name, "milky"); break;
        case 5: sprintf(the_name, "horny"); break;
        default: sprintf(the_name, "DUMB CODER(race_insult_string)");
        }
        break;
    case RACE_DEMON:
        insult = number(1, 5);
        switch(insult) {
        case 1: sprintf(the_name, "sir"); break;
        case 2:
        case 3:
        case 4:
        case 5: sprintf(the_name, "demon"); break;
        default: sprintf(the_name, "DUMB CODER(race_insult_string)");
        }
        break;
    case RACE_OGRE:
    case RACE_SOGRE:
        insult = number(1, 5);
        switch(insult) {
        case 1: sprintf(the_name, "dunder head"); break;
        case 2: sprintf(the_name, "meat head"); break;
        case 3: sprintf(the_name, "muscles"); break;
        case 4:
        case 5: sprintf(the_name, "ogre"); break;
        default: sprintf(the_name, "DUMB CODER(race_insult_string)");
        }
        break;
    case RACE_TROLL:
    case RACE_STROLL:
        insult = number(1, 5);
        switch(insult) {
        case 1: sprintf(the_name, "pea brain"); break;
        case 2: sprintf(the_name, "butter fingers"); break;
        case 3: sprintf(the_name, "warty"); break;
        case 4:
        case 5: sprintf(the_name, "troll"); break;
        default: sprintf(the_name, "DUMB CODER(race_insult_string)");
        }
        break;
    case RACE_WEREWOLF:
        insult = number(1, 5);
        switch(insult) {
        case 1: sprintf(the_name, "moon gazer"); break;
        case 2: sprintf(the_name, "lupine"); break;
        case 3: sprintf(the_name, "canine"); break;
        case 4: sprintf(the_name, "garou"); break;
        case 5: sprintf(the_name, "were wolf"); break;
        default: sprintf(the_name, "DUMB CODER(race_insult_string)");
        }
        break;
    case RACE_ELEMENTAL:
        insult = number(1, 5);
        switch(insult) {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5: sprintf(the_name, "elemental"); break;
        default: sprintf(the_name, "DUMB CODER(race_insult_string)");
        }
        break;
    case RACE_ORC:
    case RACE_SORC:
        insult = number(1, 5);
        switch(insult) {
        case 1: sprintf(the_name, "snout"); break;
        case 2: sprintf(the_name, "pig"); break;
        case 3:
        case 4:
        case 5: sprintf(the_name, "orc"); break;
        default: sprintf(the_name, "DUMB CODER(race_insult_string)");
        }
        break;
    case RACE_GNOME:
    case RACE_SGNOME:
        insult = number(1, 5);
        switch(insult) {
        case 1: sprintf(the_name, "scatter brain"); break;
        case 2: sprintf(the_name, "Aristotle"); break;
        case 3:
        case 4:
        case 5: sprintf(the_name, "gnome"); break;
        default: sprintf(the_name, "DUMB CODER(race_insult_string)");
        }
        break;
    case RACE_DRACONIAN:
    case RACE_SDRACONIAN:
        insult = number(1, 5);
        switch(insult) {
        case 1: sprintf(the_name, "lizard"); break;
        case 2: sprintf(the_name, "scaly"); break;
        case 3: sprintf(the_name, "badus breathus"); break;
 	case 4:
        case 5: sprintf(the_name, "gekko"); break;

        default: sprintf(the_name, "DUMB CODER(race_insult_string)");
        }
        break;
    case RACE_FAERIE:
        insult = number(1, 5);
        switch(insult) {
        case 1: sprintf(the_name, "flutter"); break;
        case 2: sprintf(the_name, "breakfast"); break;
        case 3: sprintf(the_name, "juice"); break;
        case 4:
        case 5: sprintf(the_name, "faerie"); break;
        default: sprintf(the_name, "DUMB CODER(race_insult_string)");
        }
        break;
    case RACE_IZARTI:
        insult = number(1, 5);
        switch(insult) {
        case 1: sprintf(the_name, "pussy cat"); break;
        case 2: sprintf(the_name, "tigger"); break;
        case 3: sprintf(the_name, "furball"); break;
        case 4:
        case 5: sprintf(the_name, "izarti"); break;
        default: sprintf(the_name, "DUMB CODER(race_insult_string)");
        }
        break;
    case RACE_AMARA:
        insult = number(1, 5);
        switch(insult) {
        case 1: sprintf(the_name, "fishstick"); break;
        case 2: sprintf(the_name, "soggy"); break;
        case 3: sprintf(the_name, "seafood"); break;
        case 4:
        case 5: sprintf(the_name, "amara"); break;
        default: sprintf(the_name, "DUMB CODER(race_insult_string)");
        }
        break;
    default:
        sprintf(the_name, "ERROR!(race_insult_string");
    }
}

/* THIS FUNCTION IS EXTREMELY LONG! LEAVE IT AT END OF FILE. */
/* Decided to place anything a mob says(apart from special procedures) in
** the same function(mostly so you can easily see what they are already
** saying). The function has three arguments: mob is the structure for the
** mob thats going to be doing the talking, situation is an integer
** that reflects what caused the mob to say something in the first place.
** target is usually the person the mob is talking about.
** The set up here is preety simple:
** There is a situation switch statement at the top level. Each of the cases
** in this switch is followed by a race switch statement. Most of the time,
** this will simply place text into a buffer, the buffer is then passed to
** the act function at the end of the switch, if something was placed into the
** buffer. If nothing was placed into the buffer, the variable "buf_empty" is
** set to true(other wise its false).
** Vex is responsible for this mess :p
*/
/* Situation constants. */
#define RESCUE_MASTER 1
#define CHARMIE_ATTACKED 2
#define CHARMIE_FIGHTING_ALONE 3
#define AGGRO_ATTACK 4
#define HELPER_ATTACK 5
#define WIMPY_ATTACK 6
#define BACKSTAB_ATTACK 7
void
mob_talk(CharData *mob, CharData *target, int situation)
{
    char mob_race[80], target_race[80], mob_plural[80], target_plural[80], target_insult[80];
    int buf_empty = 0; /* Something assumed to be in the buffer. */
    int the_action = number(1, 20); /* Used _EVERYWHERE_ */

    /* Try and find any really vague errors. */
    if (mob)
        sprinttype(GET_RACE(mob), race_types, mob_race, sizeof(mob_race));
    else {
        mudlog(NRM, LVL_IMMORT, TRUE, "(UTIL): NULL mob passed to mob_talk!");
        return;
    }
    if (target)
        sprinttype(GET_RACE(target), race_types, target_race, sizeof(target_race));
    else {
        mudlog(NRM, LVL_IMMORT, TRUE, "(UTIL): NULL target passed to mob_talk!");
        return;
    }

    race_plural_string(GET_RACE(mob), mob_plural);
    race_plural_string(GET_RACE(target), target_plural);
    race_insult_string(GET_RACE(target), target_insult);

    switch(situation) {
    /* Invoked from act.combat.c */
	case RESCUE_MASTER:
	    switch (the_action) {
		case 1:
            act("$n says, 'Prepare to meet thine maker $N!'", FALSE, mob, 0, target, TO_ROOM);
			break;
		case 2:
            act("$n screams, '&08MORE BLOOD!!!&00'", FALSE, mob, 0, target, TO_ROOM);
			break;
		case 3:
            act("$n says, 'This is another fine mess you have gotten me into $N.'", FALSE, mob, 0, mob->master, TO_ROOM);
			break;
		case 4:
            act("$n says, '...and now, $N, you must deal with me!!'", FALSE, mob, 0, target, TO_ROOM);
			break;
		case 5:
            act("$n says, 'Your heart will make a nice after dinner treat, $N!'", FALSE, mob, 0, target, TO_ROOM);
			break;
		default: /* Don't bother doing anything. */
			break;
	    }
	    break; /* charmie trying to rescue master messages. */
    /* Invoked from mobact.c */
	case CHARMIE_ATTACKED:
	    break; /* charmie attacked by aggro messages. */
	case CHARMIE_FIGHTING_ALONE:
	    break; /* charmie alone messages */
    /* Invoked from mobact.c */
    case AGGRO_ATTACK:
        switch(GET_RACE(mob)) {
        case RACE_HUMAN:
        case RACE_SHUMAN:
           switch(the_action) {
           case 1:
                sprintf(buf, "%s says, 'I don't like %s!'", GET_NAME(mob), target_plural);
                break;
           case 2:
                sprintf(buf, "%s screams, 'Prepare to die %s!'", GET_NAME(mob), target_insult);
                break;
           case 3:
                sprintf(buf, "%s says, 'Sorry, %s, but I must kill you now.'", GET_NAME(mob), target_insult);
                break;
           case 4:
                sprintf(buf, "%s says, 'Its nothing personal... I just haven't killed anyone for days!", GET_NAME(mob));
                break;
           case 5:
                sprintf(buf, "%s shouts, 'Death to all %s!'", GET_NAME(mob), target_plural);
                break;
           case 6:
                sprintf(buf, "%s screams, 'Get the hell out %s!'", GET_NAME(mob), GET_NAME(target));
                break;
           case 7:
                sprintf(buf, "%s says, 'How DARE you come in here?'", GET_NAME(mob));
                break;
           case 8:
                sprintf(buf, "%s says, 'What are YOU doing here?", GET_NAME(mob));
                break;
           case 9:
                sprintf(buf, "%s glares icily at %s.", GET_NAME(mob), GET_NAME(target));
                break;
           case 10:
                sprintf(buf, "%s screams, 'BANZAI!! CHARGE!!! KILLLLLL!!!!!!!!'", GET_NAME(mob));
                break;
           default:
              buf_empty = 1;
           }
           break;
        case RACE_HALFLING:
        case RACE_SHALFLING:
           switch(the_action) {
           case 1:
                sprintf(buf, "%s glares up at %s.", GET_NAME(mob), GET_NAME(target));
                break;
           case 2:
                sprintf(buf, "%s says, 'I can't stand you stuck up %s!'", GET_NAME(mob), target_plural);
                break;
           case 3:
                sprintf(buf, "%s screams, '%s killer, prepare to die!'", GET_NAME(mob), mob_race);
                break;
           case 4:
                sprintf(buf, "%s says, 'Hey, %s, chew on THIS!'", GET_NAME(mob), target_insult);
                break;
           case 5:
                sprintf(buf, "%s says, 'I'm tired of you %s bullying us %s!'", GET_NAME(mob), target_plural, mob_plural);
                break;
           case 6:
                sprintf(buf, "%s says, 'I've always wanted to put one of you %s in your place!'", GET_NAME(mob), target_plural);
                break;
           case 7:
                sprintf(buf, "%s says, 'It was a lovely day... then YOU showed up.'", GET_NAME(mob));
                break;
           case 8:
                sprintf(buf, "%s shouts, 'AHHHH!!! It's a %s!'", GET_NAME(mob), target_race);
                break;
           case 9:
                sprintf(buf, "%s says, 'I've always wanted to be an adventurer!'", GET_NAME(mob));
                break;
           case 10:
                sprintf(buf, "%s says, 'NEVER bother me when I'm smoking pipe weed %s!'", GET_NAME(mob), GET_NAME(target));
                break;
           default:
                buf_empty = 1;
           }
          break;
        case RACE_ELF:
        case RACE_SELF:
           switch(the_action) {
           case 1:
                sprintf(buf, "%s gasps and says, 'Look out %s! Behind you!'", GET_NAME(mob), GET_NAME(target));
                break;
           case 2:
                sprintf(buf, "%s glares at %s with eyes like chips of glass.", GET_NAME(mob), GET_NAME(target));
                break;
           case 3:
                sprintf(buf, "%s says, 'Oh GREAT. ANOTHER %s.'", GET_NAME(mob), target_race);
                break;
           case 4:
                sprintf(buf, "%s screams, 'TREE KILLER! RAPER OF THE FOREST! DIE!!!!!!!'", GET_NAME(mob));
                break;
           case 5:
                sprintf(buf, "%s says, 'You shouldn't have come here %s!'", GET_NAME(mob), GET_NAME(target));
                break;
           case 6:
                sprintf(buf, "%s says, 'Hi %s, you're just in time for the lesson of the day.", GET_NAME(mob), target_insult);
                break;
           case 7:
                sprintf(buf, "%s frowns.", GET_NAME(mob));
                break;
           case 8:
                sprintf(buf, "%s says, 'FINALLY! A chance to kill a %s!", GET_NAME(mob), target_race);
                break;
           case 9:
                sprintf(buf, "%s says, '%s your time on this earth is about to end.'", GET_NAME(mob), target_insult);
                break;
           case 10:
                sprintf(buf, "%s look at %s and smirks.", GET_NAME(mob), GET_NAME(target));
                break;
           default:
                buf_empty = 1;
           }
           break;
        case RACE_DROW:
        case RACE_SDROW:
           switch(the_action) {
           case 1:
                sprintf(buf, "%s gasps and says, 'Look out %s! Behind you!'", GET_NAME(mob), GET_NAME(target));
                break;
           case 2:
                sprintf(buf, "%s glares at %s with eyes like chips of glass.", GET_NAME(mob), GET_NAME(target));
                break;
           case 3:
                sprintf(buf, "%s says, 'Oh GREAT. ANOTHER %s.'", GET_NAME(mob), target_race);
                break;
           case 4:
                sprintf(buf, "%s screams, 'TREE KILLER! RAPER OF THE FOREST! DIE!!!!!!!'", GET_NAME(mob));
                break;
           case 5:
                sprintf(buf, "%s says, 'You shouldn't have come here %s!'", GET_NAME(mob), GET_NAME(target));
                break;
           case 6:
                sprintf(buf, "%s says, 'Hi %s, you're just in time for the lesson of the day.", GET_NAME(mob), target_insult);
                break;
           case 7:
                sprintf(buf, "%s frowns.", GET_NAME(mob));
                break;
           case 8:
                sprintf(buf, "%s says, 'FINALLY! A chance to kill a %s!", GET_NAME(mob), target_race);
                break;
           case 9:
                sprintf(buf, "%s says, '%s your time on this earth is about to end.'", GET_NAME(mob), target_insult);
                break;
           case 10:
                sprintf(buf, "%s look at %s and smirks.", GET_NAME(mob), GET_NAME(target));
                break;
           default:
                buf_empty = 1;
           }
           break;
        case RACE_DWARF:
        case RACE_SDWARF:
           switch(the_action) {
           case 1:
                sprintf(buf, "%s says, 'By Moradins left arm pit! A bloody %s!'", GET_NAME(mob), target_race);
                break;
           case 2:
                sprintf(buf, "%s says, 'I've always thought you %s would look much better... chopped off at the knees.", GET_NAME(mob), target_plural);
                break;
           case 3:
                sprintf(buf, "%s shouts, 'Wait till I tell Dad I killed a filthy %s!'", GET_NAME(mob), target_race);
                break;
           case 4:
                sprintf(buf, "%s shouts, 'CLANGGEDIN!'", GET_NAME(mob));
                break;
           case 5:
                sprintf(buf, "%s looks up at %s and says, 'A %s. I guess it will just have to do.'", GET_NAME(mob), GET_NAME(target), target_race);
                break;
           case 6:
                sprintf(buf, "%s says, 'I'm not racist %s, I hate all %s equally!'", GET_NAME(mob), target_insult, target_plural);
                break;
           case 7:
                sprintf(buf, "%s says, 'Look on the bright side %s, you'll be the hundreth %s I've killed, its a mile stone!", GET_NAME(mob), target_insult, target_race);
                break;
           case 8:
                sprintf(buf, "%s says, 'What'd you say about my beard %s?'", GET_NAME(mob), target_insult);
                break;
           case 9:
                sprintf(buf, "%s says, 'I love the smell of %s brains in the morning'", GET_NAME(mob), target_race);
                break;
           case 10:
                sprintf(buf, "%s glares coldly at %s.", GET_NAME(mob), GET_NAME(target));
                break;
           default:
                buf_empty = 1;
           }
           break;
        case RACE_GNOME:
        case RACE_SGNOME:
           switch(the_action) {
           case 1:
                sprintf(buf, "%s says, 'Have you seen my widget? No? Oh well, die then.'", GET_NAME(mob));
                break;
           case 2:
                sprintf(buf, "%s shouts, 'HELP! I'm being attacked by a %s! SAVE MEEEE!!!!'", GET_NAME(mob), target_race);
                break;
           case 3:
                sprintf(buf, "%s looks carefully at %s and says, %s tongue is the ideal potion ingredient!", GET_NAME(mob), GET_NAME(target), target_race);
                break;
           case 4:
                sprintf(buf, "%s screams, 'ARRGHHH! THE %s IS MINE!! MORE BLOOD!!!!'", GET_NAME(mob), target_race);
                break;
           case 5:
                sprintf(buf, "%s frowns disapprovingly at %s.", GET_NAME(mob), GET_NAME(target));
                break;
           case 6:
		sprintf(buf, "%s says, 'Ack! A %s!", GET_NAME(mob), target_race);
		break;
           case 7:
                sprintf(buf, "%s says, 'Oh dear! Oh dear oh dear!! I'm SO sorry, but I must kill you %s!'", GET_NAME(mob), GET_NAME(target));
                break;
           case 8:
                sprintf(buf, "%s says, 'SLAYER OF GNOME CHILDREN! DEFILER OF THE GNOME VILLAGE!! DIE!!!!!'", GET_NAME(mob));
                break;
           case 9:
                sprintf(buf, "%s says, 'Let me see (x + y) = z. z * a = b, AND b + c = a... wait! DAMNIT! I lost count you stupid %s!'", GET_NAME(mob), target_race);
                break;
           case 10:
                sprintf(buf, "%s says, 'Swell, why do all the damn %s come here?'", GET_NAME(mob), target_plural);
                break;
           default:
                buf_empty = 1;
           }
           break;
        case RACE_PLANT:
           switch(the_action) {
           case 1:
                sprintf(buf, "%s viciously rustles $m leaves at %s!", GET_NAME(mob), GET_NAME(target));
                break;
           case 2:
                sprintf(buf, "%s says, 'Hey %s, whats your problem? Never seen a talking plant before?'", GET_NAME(mob), target_insult);
                break;
           case 3:
                sprintf(buf, "%s seems to be moving towards %s... FAST!", GET_NAME(mob), GET_NAME(target));
                break;
           case 4:
                sprintf(buf, "%s seems to be looking at %s hungrily...", GET_NAME(mob), GET_NAME(target));
                break;
           case 5:
                sprintf(buf, "%s manages to frown barkily at %s", GET_NAME(mob), GET_NAME(target));
                break;
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
                buf_empty = 1;
           }
           break;
        case RACE_ANIMAL:
           switch(the_action) {
           case 1:
                sprintf(buf, "%s growls fiercely!", GET_NAME(mob));
                break;
           case 2:
                sprintf(buf, "%s ROARS!!", GET_NAME(mob));
                break;
           case 3:
                sprintf(buf, "%s looks at %s and starts to slobber.", GET_NAME(mob), GET_NAME(target));
                break;
           case 4:
                sprintf(buf, "You have the distinct impression %s is on %s's menu.", target_race, GET_NAME(mob));
                break;
           case 5:
                sprintf(buf, "%s growls at %s.", GET_NAME(mob), GET_NAME(target));
                break;
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
                buf_empty = 1;
           }
           break;
        case RACE_DRAGON:
           switch(the_action) {
           case 1:
                sprintf(buf, "%s hisses, 'Oh YESSS! I have been looking forward to ssslaying a %s all week!'", GET_NAME(mob), target_race);
                break;
           case 2:
                sprintf(buf, "%s hisses, 'Welcome %s, you've arrived just in time to die!'", GET_NAME(mob), GET_NAME(target));
                break;
           case 3:
                sprintf(buf, "%s roars, 'How DARE you disturb me %s!'", GET_NAME(mob), target_race);
                break;
           case 4:
                sprintf(buf, "%s hisses, 'Ahhhh so nisse of you to bring me sssome %s sssteak!'", GET_NAME(mob), target_race);
                break;
           case 5:
                sprintf(buf, "%s hisses, 'Greetingsss adventurerssss, allow me to extend you the hossspitality you dessserve!'", GET_NAME(mob));
                break;
           case 6:
                sprintf(buf, "%s roars, 'FOOLSS! No mortal vissitsss %s and survivesss!'", GET_NAME(mob), GET_NAME(mob));
                break;
           case 7:
                sprintf(buf, "%s hisses, 'Ahhhhh a %s. The catch of the day.'", GET_NAME(mob), target_race);
                break;
           case 8:
                sprintf(buf, "%s roars, 'THIEVESSS! RRRRRRROOOOOOOOOOOAAAAARRRRRRRR!!!!'", GET_NAME(mob));
                break;
           case 9:
                sprintf(buf, "%s hisses, 'Mmmmmmm, it'sss been a while sssince I lasst tasssted %s flessh!'", GET_NAME(mob), target_race);
                break;
           case 10:
                sprintf(buf, "%s hisses, 'You're a bit ssslow off the mark, %s!'", GET_NAME(mob), target_race);
                break;
           default:
                buf_empty = 1;
           }
           break;
        case RACE_UNDEAD:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
                buf_empty = 1;
           }
           break;
        case RACE_VAMPIRE:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
                buf_empty = 1;
           }
           break;
        case RACE_GIANT:
           switch(the_action) {
           case 1:
                sprintf(buf, "%s thunders, 'HELLO!'", GET_NAME(mob));
                break;
           case 2:
                sprintf(buf, "%s bellows, 'LOOK! Little %s to squish!'", GET_NAME(mob), target_race);
                break;
           case 3:
                sprintf(buf, "%s thunders, 'I like playing with %s!'", GET_NAME(mob), target_plural);
                break;
           case 4:
                sprintf(buf, "%s bellows, 'Good bye little %s!'", GET_NAME(mob), target_race);
                break;
           case 5:
                sprintf(buf, "%s thunders, 'OH GOODY! LITTLE JELLY TUBES!!'", GET_NAME(mob));
                break;
           case 6:
                sprintf(buf, "%s bellows, 'I like %s pan cakes!'", GET_NAME(mob), target_race);
                break;
           case 7:
                sprintf(buf, "%s laughs heartily.", GET_NAME(mob));
                break;
           case 8:
                sprintf(buf, "%s booms, '%s!'", GET_NAME(mob), target_race);
                break;
           case 9:
                sprintf(buf, "%s thunders, 'Breakfast!'", GET_NAME(mob));
                break;
           case 10:
                sprintf(buf, "%s looks down at %s and frowns.", GET_NAME(mob), GET_NAME(target));
                break;
           default:
                buf_empty = 1;
           }
           break;
        case RACE_MINOTAUR:
        case RACE_SMINOTAUR:
           switch(the_action) {
           case 1:
                sprintf(buf, "%s charges %s!", GET_NAME(mob), GET_NAME(target));
                break;
           case 2:
                sprintf(buf, "%s paws the dirt with $m hoof.", GET_NAME(mob));
                break;
           case 3:
                sprintf(buf, "%s snorts angrily!", GET_NAME(mob));
                break;
           case 4:
                sprintf(buf, "%s shouts, 'Invaders!'", GET_NAME(mob));
                break;
           case 5:
                sprintf(buf, "%s says, 'You %s grill nicely on a spit!'", GET_NAME(mob), target_plural);
                break;
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
                buf_empty = 1;
           }
           break;
        case RACE_DEMON:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
                buf_empty = 1;
           }
           break;
        case RACE_OGRE:
        case RACE_SOGRE:
           switch(the_action) {
           case 1:
                sprintf(buf, "%s shouts, 'Leave that %s! They are mine!'", GET_NAME(mob), target_race);
                break;
           case 2:
                sprintf(buf, "%s says, 'It's %s mash time!'", GET_NAME(mob), target_race);
                break;
           case 3:
                sprintf(buf, "%s glares at %s and attacks!", GET_NAME(mob), GET_NAME(target));
                break;
           case 4:
                sprintf(buf, "%s says, 'YOU'RE not supposed to be here!'", GET_NAME(mob));
                break;
           case 5:
                sprintf(buf, "%s screams, 'Death to all %s!'", GET_NAME(mob), target_plural);
                break;
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
                buf_empty = 1;
           }
           break;
        case RACE_TROLL:
        case RACE_STROLL:
           switch(the_action) {
           case 1:
                sprintf(buf, "%s says, 'Me no like you %s!", GET_NAME(mob), target_plural);
                break;
           case 2:
                sprintf(buf, "%s says, 'FOOD!'", GET_NAME(mob));
                break;
           case 3:
                sprintf(buf, "%s looks at %s and grins toothily.", GET_NAME(mob), GET_NAME(target));
                break;
           case 4:
                sprintf(buf, "%s says, 'It's dying time %s!'", GET_NAME(mob), target_insult);
                break;
           case 5:
                sprintf(buf, "%s screams in rage!", GET_NAME(mob));
                break;
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
                buf_empty = 1;
           }
           break;
        case RACE_WEREWOLF:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
                buf_empty = 1;
           }
           break;
        case RACE_ELEMENTAL:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
                buf_empty = 1;
           }
           break;
        case RACE_ORC:
        case RACE_SORC:
           switch(the_action) {
           case 1:
                sprintf(buf, "%s says, 'Lookee ere boys, we got ourselves a %s!'", GET_NAME(mob), target_race);
                break;
           case 2:
                sprintf(buf, "%s shouts, 'BLOOD!!!!'", GET_NAME(mob));
                break;
           case 3:
                sprintf(buf, "%s says, 'You %s make good eating!'", GET_NAME(mob), target_plural);
                break;
           case 4:
                sprintf(buf, "%s looks at %s scornfully.", GET_NAME(mob), GET_NAME(target));
                break;
           case 5:
                sprintf(buf, "%s says, 'Time to die %s!'",  GET_NAME(mob), target_insult);
                break;
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
                buf_empty = 1;
           }
           break;
        case RACE_DRACONIAN:
        case RACE_SDRACONIAN:
           switch(the_action) {
           case 1:
                sprintf(buf, "%s hisses loudly!", GET_NAME(mob));
                break;
           case 2:
                sprintf(buf, "%s hisses, 'Die %s!'", GET_NAME(mob), target_race);
                break;
           case 3:
                sprintf(buf, "%s glares at %s with slitted eyes.", GET_NAME(mob), GET_NAME(target));
                break;
           case 4:
                sprintf(buf, "%s hisses, 'Intrudersss!'", GET_NAME(mob));
                break;
           case 5:
                sprintf(buf, "%s hisses, 'It'sss been too long sssince I lassst sslew a %s!'", GET_NAME(mob), target_race);
                break;
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
                buf_empty = 1;
           }
           break;
        case RACE_FAERIE:
           switch(the_action) {
           case 1:
                sprintf(buf, "%s flutters around %s.", GET_NAME(mob), GET_NAME(target));
                break;
           case 2:
                sprintf(buf, "%s giggles.", GET_NAME(mob));
                break;
           case 3:
                sprintf(buf, "%s tinkles, 'Te-he! Time to die %s!'", GET_NAME(mob), target_insult);
                break;
           case 4:
                sprintf(buf, "%s tinkles, 'Allow me to help you reach the next life %s!", GET_NAME(mob), target_insult);
                break;
           case 5:
                sprintf(buf, "%s glares angrily at %s!", GET_NAME(mob), GET_NAME(target));
                break;
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
                buf_empty = 1;
           }
           break;
        case RACE_IZARTI:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
                buf_empty = 1;
           }
           break;
        case RACE_AMARA:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
                buf_empty = 1;
           }
           break;
        default:
            mudlog(NRM, LVL_IMMORT, TRUE, "(UTIL): Unknown race(%d) passed to mob_talk for %s.", GET_RACE(mob), GET_NAME(mob));
            break;
        } /* race switch */
        if (!buf_empty) act(buf, FALSE, mob, 0, target, TO_ROOM);
        break; /* aggro messages */
    /* Invoked from mobact.c */
	case HELPER_ATTACK:
        switch(GET_RACE(mob)) {
        case RACE_HUMAN:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_PLANT:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_ANIMAL:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_DRAGON:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_UNDEAD:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_VAMPIRE:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_HALFLING:
        case RACE_SHALFLING:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_ELF:
        case RACE_SELF:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_DROW:
        case RACE_SDROW:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_DWARF:
        case RACE_SDWARF:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_GIANT:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_MINOTAUR:
        case RACE_SMINOTAUR:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_DEMON:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_OGRE:
        case RACE_SOGRE:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_TROLL:
        case RACE_STROLL:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_WEREWOLF:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_ELEMENTAL:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_ORC:
        case RACE_SORC:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_GNOME:
        case RACE_SGNOME:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_DRACONIAN:
        case RACE_SDRACONIAN:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_FAERIE:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_IZARTI:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_AMARA:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        default:
            mudlog(NRM, LVL_IMMORT, TRUE, "(UTIL): Unknown race(%d) passed to mob_talk for %s.", GET_RACE(mob), GET_NAME(mob));
            break;
        } /* race switch */
	    break; /* helper messages */
    /* Invoked from mobact.c */
	case WIMPY_ATTACK:
        switch(GET_RACE(mob)) {
        case RACE_HUMAN:
        case RACE_SHUMAN:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_PLANT:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_ANIMAL:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_DRAGON:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_UNDEAD:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_VAMPIRE:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_HALFLING:
        case RACE_SHALFLING:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_ELF:
        case RACE_SELF:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_DROW:
        case RACE_SDROW:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_DWARF:
        case RACE_SDWARF:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_GIANT:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_MINOTAUR:
        case RACE_SMINOTAUR:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_DEMON:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_OGRE:
        case RACE_SOGRE:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_TROLL:
        case RACE_STROLL:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_WEREWOLF:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_ELEMENTAL:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_ORC:
        case RACE_SORC:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_GNOME:
        case RACE_SGNOME:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_DRACONIAN:
        case RACE_SDRACONIAN:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_FAERIE:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_IZARTI:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_AMARA:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        default:
            mudlog(NRM, LVL_IMMORT, TRUE, "(UTIL): Unknown race(%d) passed to mob_talk for %s.", GET_RACE(mob), GET_NAME(mob));
            break;
        } /* race switch */
        break; /* wimpy messages */
    /* Invoked from mobact.c */
	case BACKSTAB_ATTACK:
        switch(GET_RACE(mob)) {
        case RACE_HUMAN:
        case RACE_SHUMAN:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_PLANT:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_ANIMAL:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_DRAGON:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_UNDEAD:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_VAMPIRE:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_HALFLING:
        case RACE_SHALFLING:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_ELF:
        case RACE_SELF:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_DROW:
        case RACE_SDROW:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_DWARF:
        case RACE_SDWARF:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_GIANT:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_MINOTAUR:
        case RACE_SMINOTAUR:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_DEMON:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_OGRE:
        case RACE_SOGRE:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_TROLL:
        case RACE_STROLL:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_WEREWOLF:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_ELEMENTAL:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_ORC:
        case RACE_SORC:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_GNOME:
        case RACE_SGNOME:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_DRACONIAN:
        case RACE_SDRACONIAN:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_FAERIE:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_IZARTI:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        case RACE_AMARA:
           switch(the_action) {
           case 1:
           case 2:
           case 3:
           case 4:
           case 5:
           case 6:
           case 7:
           case 8:
           case 9:
           case 10:
           default:
			   break;
           }
           break;
        default:
           mudlog(NRM, LVL_IMMORT, TRUE, "(UTIL): Unknown race(%d) passed to mob_talk for %s.", GET_RACE(mob), GET_NAME(mob));
           break;
        } /* race switch */
	    break; /* backstab messages. */
	default:
	    mudlog(NRM, LVL_IMMORT, TRUE, "(UTIL)Unknown situation(%d) passed to mob_talk for %s.", situation, GET_NAME(mob));
	    break;
    } /* situation switch */
} /* end mob_talk */

/* Mortius : When a player dies this will create a txt file containing what
             items the had when they died */
void obj_corpse_save(ObjData *corpse, CharData *ch)
{
  FILE *fl;
  char buf[255], tmstr[30];
  struct tm *ct;
  time_t tmp;
  char lname[255];

  if(!corpse || !corpse->contains)
    return;

  /* Convert name to lower case */
  strcpy(lname, GET_NAME(ch));
  strlower(lname);
  sprintf(buf, "corpse/%s", lname);

  if (!(fl = fopen(buf, "w"))) {
    mlog("OPEN ERROR: %s", buf);
    return;
  }

  tmp = time(0);
  ct = localtime(&tmp);
  strftime(tmstr, 20, "%b %d %T", ct);
  fprintf(fl, "%s\n", tmstr);

  fprintf(fl, "W %d %s\n",
    world[ch->in_room].number, world[ch->in_room].name);

  obj_save_items(fl, corpse);
  fprintf(fl, "E");

  fclose(fl);
}

void obj_save_items(FILE *fp, ObjData *obj)
{
    /* just in case */
    if(!obj) return;

    if(obj->obj_flags.type_flag == ITEM_MONEY) {
        fprintf(fp, "M %d\n", obj->obj_flags.value[0]);
    }
    /* can't record corpses, mail, etc in the corpse */
    else if(obj->item_number != -1) {
        if(obj_index[obj->item_number].virtual > 0)
            fprintf(fp, "O %d %d    ; %s\n",
              obj_index[obj->item_number].virtual, obj->obj_flags.timer,
              obj->short_description);
    }
    if(obj->obj_flags.type_flag == ITEM_CONTAINER) {
        ObjData *p;
        for(p = obj->contains; p; p = p->next_content)
            obj_save_items(fp, p);
    }
}

/* Mortius : change string to lower case */

char *strlower(char *string)
{
   char *p;
   for(p = string; *p; p++) *p = LOWER(*p);
   return string;
}

/* Mortius : this function is for traps/portals mainly.  Checks to make sure
             the room is there before trying to send the player to it.  If
	     the room isn't there then we remove the object from the game to
	     stop any problems.
*/

int check_room(CharData *ch, int to_room, ObjData *obj)
{
  if (!ch || !to_room || !obj)          return 0;

  if (world[to_room].name != NULL)      return 1;

  mudlog( BRF, LVL_IMMORT, TRUE, "SYSERR: problem in check_room : Obj = %s, objnum = %d, room = %d",
          obj->name, obj->item_number, to_room);

  sprintf(buf, "There was a problem using %s, please inform an immortal\r\n",
          obj->short_description);
  send_to_char(buf, ch);

  /* We know there is a problem so lets take the item from the game */

  extract_obj(obj);

  return 0;
}

/* the nst, nnd or nth ? */
char *ndth(int val) {
    // Special case numbers 11, 12, 13 - Arbaces
    if (val == 11 || val == 12 || val == 13) return "th";

    if ((val % 10) == 1) return "st";
    if ((val % 10) == 2) return "nd";
    if ((val % 10) == 3) return "rd";
    return "th";
}

CharData *random_victim(int room)
{
    CharData *ch = NULL, *vict;
    int people = 0;

    for (vict = world[room].people; vict; vict = vict->next_in_room) {
        if (number(1, ++people) == 1) ch = vict;
    }

    return ch;
}

int in_same_group(CharData *ch, CharData *tch)
{
    if (ch == tch)                              return 1;
    if (!ch->master && !tch->master)            return 0;
    if (!IS_AFFECTED(ch, AFF_GROUP))            return 0;
    if (!IS_AFFECTED(tch, AFF_GROUP))           return 0;
    if (ch == tch->master || tch == ch->master) return 1;
    if (ch->master == tch->master)              return 1;
    return 0;
}

/** This function (derived from basic fork() abort() idea by Erwin S Andreasen)
 * causes your MUD to dump core (assuming you can) but continue running. The
 * core dump will allow post-mortem debugging that is less severe than assert();
 * Don't call this directly as core_dump_unix() but as simply 'core_dump()' so
 * that it will be excluded from systems not supporting them. You still want to
 * call abort() or exit(1) for non-recoverable errors, of course. Wonder if
 * flushing streams includes sockets?
 * @param who The file in which this call was made.
 * @param line The line at which this call was made. */
void core_dump_real(const char *who, int line)
{
  mlog("SYSERR: Assertion failed at %s:%d!", who, line);

#if 1	/* By default, let's not litter. */
#if defined(CIRCLE_UNIX)
  /* These would be duplicated otherwise...make very sure. */
  fflush(stdout);
  fflush(stderr);
  fflush(logfile);
  /* Everything, just in case, for the systems that support it. */
  fflush(NULL);

  /* Kill the child so the debugger or script doesn't think the MUD crashed.
   * The 'autorun' script would otherwise run it again. */
  if (fork() == 0)
    abort();
#endif
#endif
}

/** Count the number bytes taken up by color codes in a string that will be
 * empty space once the color codes are converted and made non-printable.
 * @param string The string in which to check for color codes.
 * @retval int the number of color codes found. */
int count_color_chars(char *string)
{
  int i, len;
  int num = 0;

	if (!string || !*string)
		return 0;

	len = strlen(string);
  for (i = 0; i < len; i++) {
    while (string[i] == '&') {
      if (string[i + 2] == '&')
        num++;
      else
        num += 3;
      i += 2;
    }
  }
  return num;
}

/** Calculates the Levenshtein distance between two strings. Currently used
 * by the mud to make suggestions to the player when commands are mistyped.
 * This function is most useful when an index of possible choices are available
 * and the results of this function are constrained and used to help narrow
 * down the possible choices. For more information about Levenshtein distance,
 * recommend doing an internet or wikipedia search.
 * @param s1 The input string.
 * @param s2 The string to be compared to.
 * @retval int The Levenshtein distance between s1 and s2. */
int levenshtein_distance(const char *s1, const char *s2)
{
  int **d, i, j;
  int s1_len = strlen(s1), s2_len = strlen(s2);

  CREATE(d, int *, s1_len + 1);

  for (i = 0; i <= s1_len; i++) {
    CREATE(d[i], int, s2_len + 1);
    d[i][0] = i;
  }

  for (j = 0; j <= s2_len; j++)
    d[0][j] = j;
  for (i = 1; i <= s1_len; i++)
    for (j = 1; j <= s2_len; j++)
      d[i][j] = MIN(d[i - 1][j] + 1, MIN(d[i][j - 1] + 1,
      d[i - 1][j - 1] + ((s1[i - 1] == s2[j - 1]) ? 0 : 1)));

  i = d[s1_len][s2_len];

  for (j = 0; j <= s1_len; j++)
    free(d[j]);
  free(d);

  return i;
}
/**
 * Reads a certain number of lines from the begining of a file, like performing
 * a 'head'.
 * @pre Expects an already open file and the user to supply enough memory
 * in the output buffer to hold the lines read from the file. Assumes the
 * file is a text file. Expects buf to be nulled out if the entire buf is
 * to be used, otherwise, appends file information beyond the first null
 * character. lines_to_read is assumed to be a positive number.
 * @post Rewinds the file pointer to the beginning of the file. If buf is
 * too small to handle the requested output, **OVERFLOW** is appended to the
 * buffer.
 * @param[in] file A pointer to an already successfully opened file.
 * @param[out] buf Buffer to hold the data read from the file. Will not
 * overwrite preexisting information in a non-null string.
 * @param[in] bufsize The total size of the buffer.
 * @param[in] lines_to_read The number of lines to be read from the front of
 * the file.
 * @retval int The number of lines actually read from the file. Can be used
 * the compare with the number of lines requested to be read to determine if the
 * entire file was read. If lines_to_read is <= 0, no processing occurs
 * and lines_to_read is returned.
 */
int file_head( FILE *file, char *buf, size_t bufsize, int lines_to_read )
{
  /* Local variables */
  int lines_read = 0;   /* The number of lines read so far. */
  char line[READ_SIZE]; /* Retrieval buffer for file. */
  size_t buflen;        /* Amount of previous existing data in buffer. */
  int readstatus = 1;   /* Are we at the end of the file? */
  int n = 0;            /* Return value from snprintf. */
  const char *overflow = "\r\n**OVERFLOW**\r\n"; /* Appended if overflow. */

  /* Quick check for bad arguments. */
  if (lines_to_read <= 0)
  {
    return lines_to_read;
  }

  /* Initialize local variables not already initialized. */
  buflen  = strlen(buf);

  /* Read from the front of the file. */
  rewind(file);

  while ( (lines_read < lines_to_read) &&
      (readstatus > 0) && (buflen < bufsize) )
  {
    /* Don't use get_line to set lines_read because get_line will return
     * the number of comments skipped during reading. */
    readstatus = get_line( file, line );

    if (readstatus > 0)
    {
      n = snprintf( buf + buflen, bufsize - buflen, "%s\r\n", line);
      buflen += n;
      lines_read++;
    }
  }

  /* Check to see if we had a potential buffer overflow. */
  if (buflen >= bufsize)
  {
    /* We should never see this case, but... */
    if ( (strlen(overflow) + 1) >= bufsize )
    {
      core_dump();
      snprintf( buf, bufsize, "%s", overflow);
    }
    else
    {
      /* Append the overflow statement to the buffer. */
      snprintf( buf + buflen - strlen(overflow) - 1, strlen(overflow) + 1, "%s", overflow);
    }
  }

  rewind(file);

  /* Return the number of lines. */
  return lines_read;
}

/**
 * Reads a certain number of lines from the end of the file, like performing
 * a 'tail'.
 * @pre Expects an already open file and the user to supply enough memory
 * in the output buffer to hold the lines read from the file. Assumes the
 * file is a text file. Expects buf to be nulled out if the entire buf is
 * to be used, otherwise, appends file information beyond the first null
 * character in buf. lines_to_read is assumed to be a positive number.
 * @post Rewinds the file pointer to the beginning of the file. If buf is
 * too small to handle the requested output, **OVERFLOW** is appended to the
 * buffer.
 * @param[in] file A pointer to an already successfully opened file.
 * @param[out] buf Buffer to hold the data read from the file. Will not
 * overwrite preexisting information in a non-null string.
 * @param[in] bufsize The total size of the buffer.
 * @param[in] lines_to_read The number of lines to be read from the back of
 * the file.
 * @retval int The number of lines actually read from the file. Can be used
 * the compare with the number of lines requested to be read to determine if the
 * entire file was read. If lines_to_read is <= 0, no processing occurs
 * and lines_to_read is returned.
 */
int file_tail( FILE *file, char *buf, size_t bufsize, int lines_to_read )
{
  /* Local variables */
  int lines_read = 0;   /* The number of lines read so far. */
  int total_lines = 0;  /* The total number of lines in the file. */
  char c;               /* Used to fast forward the file. */
  char line[READ_SIZE]; /* Retrieval buffer for file. */
  size_t buflen;        /* Amount of previous existing data in buffer. */
  int readstatus = 1;   /* Are we at the end of the file? */
  int n = 0;            /* Return value from snprintf. */
  const char *overflow = "\r\n**OVERFLOW**\r\n"; /* Appended if overflow. */

  /* Quick check for bad arguments. */
  if (lines_to_read <= 0)
  {
    return lines_to_read;
  }

  /* Initialize local variables not already initialized. */
  buflen  = strlen(buf);
  total_lines = file_numlines(file); /* Side effect: file is rewound. */

  /* Fast forward to the location we should start reading from */
  while (((lines_to_read + lines_read) < total_lines))
  {
    do {
      c = fgetc(file);
    } while(c != '\n');

    lines_read++;
  }

  /* We reuse the lines_read counter. */
  lines_read = 0;

  /** From here on, we perform just like file_head */
  while ( (lines_read < lines_to_read) &&
      (readstatus > 0) && (buflen < bufsize) )
  {
    /* Don't use get_line to set lines_read because get_line will return
     * the number of comments skipped during reading. */
    readstatus = get_line( file, line );

    if (readstatus > 0)
    {
      n = snprintf( buf + buflen, bufsize - buflen, "%s\r\n", line);
      buflen += n;
      lines_read++;
    }
  }

  /* Check to see if we had a potential buffer overflow. */
  if (buflen >= bufsize)
  {
    /* We should never see this case, but... */
    if ( (strlen(overflow) + 1) >= bufsize )
    {
      core_dump();
      snprintf( buf, bufsize, "%s", overflow);
    }
    else
    {
      /* Append the overflow statement to the buffer. */
      snprintf( buf + buflen - strlen(overflow) - 1, strlen(overflow) + 1, "%s", overflow);
    }
  }

  rewind(file);

  /* Return the number of lines read. */
  return lines_read;

}

/** Returns the byte size of a file. We assume size_t to be a large enough type
 * to handle all of the file sizes in the mud, and so do not make SIZE_MAX
 * checks.
 * @pre file parameter must already be opened.
 * @post file will be rewound.
 * @param file The file to determine the size of.
 * @retval size_t The byte size of the file (we assume no errors will be
 * encountered in this function).
 */
size_t file_sizeof( FILE *file )
{
  size_t numbytes = 0;

  rewind(file);

  /* It would be so much easier to do a byte count if an fseek SEEK_END and
   * ftell pair of calls was portable for text files, but all information
   * I've found says that getting a file size from ftell for text files is
   * not portable. Oh well, this method should be extremely fast for the
   * relatively small filesizes in the mud, and portable, too. */
  while (!feof(file))
  {
    fgetc(file);
    numbytes++;
  }

  rewind(file);

  return numbytes;
}

/** Returns the number of newlines '\n' in a file, which we equate to number of
 * lines. We assume the int type more than adequate to count the number of lines
 * and do not make checks for overrunning INT_MAX.
 * @pre file parameter must already be opened.
 * @post file will be rewound.
 * @param file The file to determine the size of.
 * @retval size_t The byte size of the file (we assume no errors will be
 * encountered in this function).
 */
int file_numlines( FILE *file )
{
  int numlines = 0;
  char c;

  rewind(file);

  while (!feof(file))
  {
    c = fgetc(file);
    if (c == '\n')
    {
      numlines++;
    }
  }

  rewind(file);

  return numlines;
}


int GET_DAMROLL(CharData *ch) {
    if (IS_NPC(ch))
        return (ch)->points.damroll;
    else
        return (ch)->playerDamroll;
}

void SET_DAMROLL(CharData *ch, int dam) {
    if(IS_NPC(ch))
        ch->points.damroll = dam;
    else
        ch->playerDamroll = dam;
    
}

