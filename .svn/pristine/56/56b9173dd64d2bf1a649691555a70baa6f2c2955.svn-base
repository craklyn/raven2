/* ************************************************************************
*   File: interpreter.c                                 Part of CircleMUD *
*  Usage: parse user commands, search for specials, call ACMD functions   *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#define __INTERPRETER_C__

#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "util/utils.h"
#include "general/comm.h"
#include "actions/interpreter.h"
#include "magic/spells.h"
#include "general/handler.h"
#include "actions/act.clan.h"
#include "general/class.h"
#include "specials/mail.h"
#include "general/color.h"
#include "magic/sing.h"
#include "general/objsave.h"
#include "actions/ban.h"
#include "olc/olc.h"
#include "olc/oedit.h"
#include "olc/redit.h"
#include "olc/zedit.h"
#include "olc/medit.h"
#include "olc/sedit.h"
#include "olc/qedit.h"
#include "olc/hedit.h"
#include "general/modify.h"
#include "scripts/dg_scripts.h"
#include "actions/act.h"              /* ACMDs located within the act*.c files */
#include "general/chores.h"

/* external functions */
void echo_on(DescriptorData * d);
void echo_off(DescriptorData * d);
void init_char(CharData * ch);
int create_entry(char *name);
int special(CharData * ch, int cmd, char *arg);
int isbanned(char *hostname);
int Valid_Name(char *newname);
//char last_command_entered[MAX_STRING_LENGTH+100];

//struct command_info *complete_cmd_info;
/* Note! Below includes contains all the mud commands. */
#include "actions/command_list.c"

const char *fill[]     = { "in", "from", "with", "the", "on", "at", "to", "\n" };
const char *reserved[] = { "a", "an", "self", "me", "all", "room", "someone", "something", "\n" };

/*
 * This is the actual command interpreter called from game_loop() in comm.c
 * It makes sure you are the proper level and position to execute the command,
 * then calls the appropriate function.
 */
void command_interpreter(CharData * ch, char *argument)
{
    int cmd, length;
    extern int no_specials;
    char *line;
    CharData *tmp_ch;

  /* Mortius: Lets log the very last thing typed
   *
   * I've never once seen this log anything. Waste of space. Maybe we can reuse
   * it down the road. - Xiuh 09.04.09
  if (ch->desc) {
      if (ch->desc->connected == CON_PLAYING) {
          if (ch->in_room == NOWHERE)
              strcat(last_buf, " (in NOWHERE!)");
          else if (ch->in_room < 0 || ch->in_room > top_of_world)
                   strcat(last_buf, " (illegal ch->in_room!)");
          else {
                sprintf(last_buf2, "room %d", world[ch->in_room].number);
                strcat(last_buf, last_buf2);
          }
      }
   } else
        strcpy(last_buf, "?mob?");

   sprintf(last_command_entered, "LAST COMMAND: %s [ %s ] %s",
           GET_NAME(ch), argument, last_buf);

   * end of commited out last_command code.
   */

    line = any_one_arg(argument, arg);

    if( !*arg ) return;

    if( PLR_FLAGGED( ch, PLR_SHUNNED )){
        mlog( "Shunned I/O: %s [%s]", ch->player.name, argument );
        WAIT_STATE(ch, PULSE_VIOLENCE * number(1, 4));
    }

    /*
    ** A stunned NPC should drop the command it has been
    ** ordered to perform - Digger
    */
    if(IS_MOB(ch) && STUNNED(ch)) return;

    /* Let's stop switched gods from cheating. */
    if( ch->desc != NULL && ch->desc->original )
        tmp_ch = ch->desc->original;
    else
        tmp_ch = ch;

    if (GET_LEVEL(ch) < LVL_IMMORT) {
      int cont;
      cont = command_wtrigger(ch, arg, line);
      if (!cont) cont = command_mtrigger(ch, arg, line);
      if (!cont) cont = command_otrigger(ch, arg, line);
      if (cont) return; // command trigger took over
    }

    for( length = strlen(arg), cmd = 0; *cmd_info[cmd].command != '\n'; cmd++ )
        if( !strncmp( cmd_info[cmd].command, arg, length ))
            if( GET_LEVEL(tmp_ch) >= cmd_info[cmd].minimum_level )
                break;

    if( *cmd_info[cmd].command == '\n' ) send_to_char("Huh?!?\r\n", ch);

    else if( PLR_FLAGGED(ch, PLR_FROZEN) && GET_LEVEL(ch) < LVL_IMPL ) {
        send_to_char("You try, but the mind-numbing cold prevents you...\r\n", ch);
    }

    else if((IS_AFFECTED(ch, AFF_PARALYZE) && GET_LEVEL(ch) < LVL_IMPL)
          && cmd_info[cmd].minimum_position != POS_DEAD ){
        send_to_char("You are paralyzed and unable to move.\n\r", ch);
    }

    else if (cmd_info[cmd].command_pointer == NULL) {
        send_to_char("Sorry, that command hasn't been implemented yet.\r\n", ch);
    }

    else if( GET_POS(ch) < cmd_info[cmd].minimum_position ) {
        static char *positStr[] = {
            "Lie still; you are DEAD!!",
            "You are in a pretty bad shape, unable to do anything!",
            "You are in a pretty bad shape, unable to do anything!",
            "All you can do right now is think about the stars!",
            "In your dreams, or what?",
            "Nah... You feel too relaxed to do that..",
            "Nah... You feel too relaxed to do that..",
            "Maybe you should get on your feet first?",
            "No way!  You're fighting for your life!",
            "You're just standing around with nothing to do."
        };
        sendChar( ch, "%s\r\n", positStr[ (int)GET_POS(ch) ] );
    }

    else if( no_specials || !special(ch, cmd, line) ){
        int unaffFlags = cmd_info[cmd].unaffFlags;
        /*
        ** Check for any affects that may need to be removed.
        */

        // As a special case, AFF_HIDE is not removed if SPELL_DANCE_SHADOWS
        // is in effect
        if (affected_by_spell(ch, SPELL_DANCE_SHADOWS))
          unaffFlags &= ~AFF_HIDE;

        REMOVE_BIT_AR( AFF_FLAGS(ch), unaffFlags );

        /* any command at all breaks an ambush */
        if (ch->ambushing) free(ch->ambushing);
        ch->ambushing = NULL;

        /* any command at all stops you singing */
        if(cmd_info[cmd].minimum_position >= POS_SITTING) {
            if (SINGING(ch)) stop_singing(ch);
            REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_AFK);
        }

        ((*cmd_info[cmd].command_pointer) (ch, line, cmd, cmd_info[cmd].subcmd));
    }
}

/***************************************************************************
 * Various other parsing utilities                                         *
 **************************************************************************/

/* Searches an array of strings for a target string.  "exact" can be
 * 0 or non-0, depending on whether or not the match must be exact for
 * it to be returned.  Returns -1 if not found; 0..n otherwise.  Array
 * must be terminated with a '\n' so it knows to stop searching. */
int search_block(char *arg, const char **list, int exact)
{
  int i, l;

  /*  We used to have \r as the first character on certain array items to
   *  prevent the explicit choice of that point.  It seems a bit silly to
   *  dump control characters into arrays to prevent that, so we'll just
   *  check in here to see if the first character of the argument is '!',
   *  and if so, just blindly return a '-1' for not found. - ae. */
  if (*arg == '!')
    return (-1);

  /* Make into lower case, and get length of string */
  for (l = 0; *(arg + l); l++)
    *(arg + l) = LOWER(*(arg + l));

  if (exact) {
    for (i = 0; **(list + i) != '\n'; i++)
      if (!strcmp(arg, *(list + i)))
	return (i);
  } else {
    if (!l)
      l = 1;			/* Avoid "" to match the first available
				 * string */
    for (i = 0; **(list + i) != '\n'; i++)
      if (!strncmp(arg, *(list + i), l))
	return (i);
  }

  return (-1);
}

int is_number(char *str)
{
  while (*str)
    if (!isdigit(*(str++)))
      return (0);

  return (1);
}

/* Function to skip over the leading spaces of a string. */
void skip_spaces(char **string)
{
  for (; **string && isspace(**string); (*string)++);
}

/* Given a string, change all instances of double dollar signs ($$) to single
 * dollar signs ($).  When strings come in, all $'s are changed to $$'s to
 * avoid having users be able to crash the system if the inputted string is
 * eventually sent to act().  If you are using user input to produce screen
 * output AND YOU ARE SURE IT WILL NOT BE SENT THROUGH THE act() FUNCTION
 * (i.e., do_gecho, do_title, but NOT do_say), you can call
 * delete_doubledollar() to make the output look correct.
 * Modifies the string in-place. */
char *delete_doubledollar(char *string)
{
  char *ddread, *ddwrite;

  /* If the string has no dollar signs, return immediately */
  if ((ddwrite = strchr(string, '$')) == NULL)
    return (string);

  /* Start from the location of the first dollar sign */
  ddread = ddwrite;


  while (*ddread)   /* Until we reach the end of the string... */
    if ((*(ddwrite++) = *(ddread++)) == '$') /* copy one char */
      if (*ddread == '$')
	ddread++; /* skip if we saw 2 $'s in a row */

  *ddwrite = '\0';

  return (string);
}

int fill_word(char *argument)
{
  return (search_block(argument, fill, TRUE) >= 0);
}

int reserved_word(char *argument)
{
  return (search_block(argument, reserved, TRUE) >= 0);
}

/* Copy the first non-fill-word, space-delimited argument of 'argument'
 * to 'first_arg'; return a pointer to the remainder of the string. */
char *one_argument(char *argument, char *first_arg)
{
# define MAXARG 30
  int   arglen = 0;
  char *begin  = first_arg;

  if (!argument) {
    mlog("SYSERR: one_argument received a NULL pointer!");
    *first_arg = '\0';
    return (NULL);
  }

  do {
    skip_spaces(&argument);

    first_arg = begin;
    while( *argument && !isspace( *argument )){
      /*
      ** Let an underline serve as a sep that is
      ** converted to a space on the fly. Digger 9/2/97
      */
      if( *argument == '_' )
        *(first_arg++) = ' ';
      else
        *(first_arg++) = LOWER(*argument);
      argument++;
      arglen++;
      if( arglen > MAXARG ){
          mudlog( NRM, LVL_LRGOD, TRUE, "CRASH: Max arg length reached in one_argument function." );
          mudlog( NRM, LVL_LRGOD, TRUE, begin, 80 );
          *argument = ' ';
      }
    }

    *first_arg = '\0';
  } while (fill_word(begin));

  return (argument);
}

/* one_word is like any_one_arg, except that words in quotes ("") are
 * considered one word. No longer ignores fill words.  -dak */
char *one_word(char *argument, char *first_arg)
{
    skip_spaces(&argument);

    if (*argument == '\"') {
      argument++;
      while (*argument && *argument != '\"') {
        *(first_arg++) = LOWER(*argument);
        argument++;
      }
      argument++;
    } else {
      while (*argument && !isspace(*argument)) {
        *(first_arg++) = LOWER(*argument);
        argument++;
      }
    }

    *first_arg = '\0';
  return (argument);
}

/* same as one_argument except that it doesn't ignore fill words */
char*
any_one_arg( char *argument, char *first_arg )
{
  skip_spaces( &argument );

  while( *argument && !isspace(*argument) )
  {
    *(first_arg++) = LOWER(*argument);
    argument++;
  }

  *first_arg = '\0';

  return argument;
}

/* same as any_one_argument except that it preserves case - Vex. */
char*
case_one_arg(char *argument, char *first_arg)
{
  skip_spaces(&argument);

  while (*argument && !isspace(*argument)) {
    *(first_arg++) = (*argument);
    argument++;
  }

  *first_arg = '\0';

  return argument;
}

/*
 * Same as one_argument except that it takes two args and returns the rest;
 * ignores fill words
 */
char *two_arguments(char *argument, char *first_arg, char *second_arg)
{
  return one_argument(one_argument(argument, first_arg), second_arg);	/* :-) */
}

/* Determine if a given string is an abbreviation of another.
 * Returns 1 if arg1 is an abbreviation of arg2. */
int is_abbrev(const char *arg1, const char *arg2)
{
  if (!*arg1)
    return (0);

  for (; *arg1 && *arg2; arg1++, arg2++)
    if (LOWER(*arg1) != LOWER(*arg2))
      return (0);

  if (!*arg1)
    return (1);
  else
    return (0);
}

/* return first space-delimited token in arg1; remainder of string in arg2 */
void
half_chop( char *string, char *arg1, char *arg2 )
{
  char *temp = any_one_arg( string, arg1 );

  skip_spaces( &temp );
  strcpy( arg2, temp );
}

/* same as half_chop except preserves case - vex */
void
case_chop(char *string, char *arg1, char *arg2)
{
  char *temp;

  temp = case_one_arg(string, arg1);
  skip_spaces(&temp);
  strcpy(arg2, temp);
}


/* Used in specprocs, mostly.  (Exactly) matches "command" to cmd number */
int find_command(char *command)
{
  int cmd;

  for (cmd = 0; *cmd_info[cmd].command != '\n'; cmd++)
    if (!strcmp(cmd_info[cmd].command, command))
      return cmd;

  return -1;
}


int
special( CharData * ch, int cmd, char *arg)
{
  register ObjData *i;
  register CharData *k;
  int j;

  /* special in room? */
  if (GET_ROOM_SPEC(ch->in_room) != NULL)
    if (GET_ROOM_SPEC(ch->in_room) (ch, world + ch->in_room, cmd, arg))
      return 1;

  /* special in equipment list? */
  for (j = 0; j < NUM_WEARS; j++)
    if (ch->equipment[j] && GET_OBJ_SPEC(ch->equipment[j]) != NULL)
      if (GET_OBJ_SPEC(ch->equipment[j]) (ch, ch->equipment[j], cmd, arg))
	return 1;

  /* special in inventory? */
  for (i = ch->carrying; i; i = i->next_content)
    if (GET_OBJ_SPEC(i) != NULL)
      if (GET_OBJ_SPEC(i) (ch, i, cmd, arg))
	return 1;

  /* special in mobile present? */
  for (k = world[ch->in_room].people; k; k = k->next_in_room)
    if (GET_MOB_SPEC(k) != NULL)
      if (GET_MOB_SPEC(k) (ch, k, cmd, arg))
	return 1;

  /* special in object present? */
  for (i = world[ch->in_room].contents; i; i = i->next_content)
    if (GET_OBJ_SPEC(i) != NULL)
      if (GET_OBJ_SPEC(i) (ch, i, cmd, arg))
	return 1;

  return 0;
}



/* *************************************************************************
*  Stuff for controlling the non-playing sockets (get name, pwd etc)       *
************************************************************************* */


/* locate entry in p_table with entry->name == name. -1 mrks failed search */
int find_name(char *name)
{
  int i;

  for (i = 0; i <= top_of_p_table; i++) {
    if (!str_cmp((player_table + i)->name, name))
      return i;
  }

  return -1;
}


int _parse_name(char *arg, char *name)
{
  int i;

  /* skip whitespaces */
  for (; isspace(*arg); arg++);

  for (i = 0; (*name = LOWER(*arg)); arg++, i++, name++)
    if (!isalpha(*arg))
      return 1;

  if (!i)
    return 1;

  return 0;
}

static int multi_ok(CharData *ch1, CharData *ch2, char *host)
{
  // Gods can break the rules
  if (GET_LEVEL(ch1) >= LVL_IMMORT || GET_LEVEL(ch2) >= LVL_IMMORT) return 1;

  return 0; // 0
}

/* deal with newcomers and other non-playing sockets */
void nanny(DescriptorData * d, char *arg)
{
  int load_result;	/* Overloaded variable */
  int player_i;
  int i;
  struct char_file_u tmp_store;
  CharData *tmp_ch;
  DescriptorData *k, *next;
  char tmp_name[MAX_INPUT_LENGTH];
  int chooseAge;

  int load_char(char *name, struct char_file_u * char_element);
  int parse_pc_race_class(int race, char arg);

  /* OasisOLC states */
  struct {
    int state;
    void (*func)(DescriptorData *, char *);
  } olc_functions[] = {
  /* OLC states */
  { CON_OEDIT, oParse },
  { CON_REDIT, rParse },
  { CON_ZEDIT, zParse },
  { CON_MEDIT, mParse },
  { CON_SEDIT, sParse },
  { CON_TRIGEDIT, trigedit_parse },
  { CON_QEDIT, qParse },
  { CON_CEDIT, cedit_parse },
  { CON_HEDIT, hedit_parse },
  { -1, NULL }
  };

    skip_spaces(&arg);

  /* Quick check for the OLC states. */
  for (player_i = 0; olc_functions[player_i].state >= 0; player_i++)
    if (STATE(d) == olc_functions[player_i].state) {
      (*olc_functions[player_i].func)(d, arg);
      return;
    }

  /* Not in OLC. */
  switch (STATE(d)) {
  case CON_GET_NAME:		/* wait for input of name */
    if (d->character == NULL) {
      CREATE(d->character, CharData, 1);
      clear_char(d->character);
      CREATE(d->character->player_specials, struct player_special_data, 1);
      d->character->player_specials->pardons = NULL;
      d->character->desc = d;
    }
    if (!*arg)
      SET_DCPENDING(d);
    else {
      if ((_parse_name(arg, tmp_name)) || strlen(tmp_name) < 2 ||
          strlen(tmp_name) > MAX_NAME_LENGTH ||
          fill_word(strcpy(buf, tmp_name)) || reserved_word(buf)) {
        write_to_output(d, "Invalid name, please try another.\r\nName: ");
	return;
      }

      if ((player_i = load_char(tmp_name, &tmp_store)) > -1) {
	d->pos = player_i;
	store_to_char(&tmp_store, d->character);

	if (PLR_FLAGGED(d->character, PLR_DELETED)) {
            if(GET_LEVEL(d->character) >= 40) {
                write_to_output(d, "That character name has been retired, please try another.\r\n");
                setConnectState(d, CON_CLOSE );
                return;
            }

          free_char(d->character);
	  CREATE(d->character, CharData, 1);
	  clear_char(d->character);
	  /* Initialize player specials so they don't crash mud later. Vex. */
	  CREATE(d->character->player_specials, struct player_special_data, 1);
          d->character->player_specials->pardons = NULL;
	  d->character->desc = d;
	  CREATE(d->character->player.name, char, strlen(tmp_name) + 1);
	  strcpy(d->character->player.name, CAP(tmp_name));
	  write_to_output(d, "Did I get that right, %s (Y/N)? ", tmp_name);
	  setConnectState(d, CON_NAME_CNFRM );
	} else {
	  strcpy(d->pwd, tmp_store.pwd);
          // load the quest status
          load_char_quests(d->character);
          GET_QUEST(d->character) = NULL;

	  /* undo it just in case they are set */
	  REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_WRITING);
	  REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_MAILING);
	  REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_CRYO);
          REMOVE_BIT_AR(AFF_FLAGS(d->character), AFF_GROUP);
          REMOVE_BIT_AR(PRF_FLAGS(d->character), PRF_QUEST);

	  write_to_output(d, "Password: ");
	  echo_off(d);

	  setConnectState(d, CON_PASSWORD );
	}
      } else {
	/* player unknown -- make new character */

        /* Check for multiple creations of a character. */
	if (!Valid_Name(tmp_name)) {
          write_to_output(d, "Invalid name, please try another.\r\nName: ");
	  return;
	}
	CREATE(d->character->player.name, char, strlen(tmp_name) + 1);
        strcpy(d->character->player.name, CAP(tmp_name));	/* strcpy: OK (size checked above) */

	write_to_output(d, "Did I get that right, %s (Y/N)? ", tmp_name);
	setConnectState(d, CON_NAME_CNFRM );
      }
    }
    break;

  case CON_NAME_CNFRM:		/* wait for conf. of new name	 */
    if (UPPER(*arg) == 'Y') {
      if (isbanned(d->host) >= BAN_NEW) {
	mudlog( NRM, LVL_LRGOD, TRUE, "Request for new char %s denied from [%s] (siteban)",
		GET_NAME(d->character), d->host);
	write_to_output(d, "Sorry, new characters are not allowed from your site!\r\n");
	setConnectState(d, CON_CLOSE );
	return;
      }
      if (circle_restrict) {
	write_to_output(d, "Sorry, new players can't be created at the moment.\r\n");
	mudlog( NRM, LVL_LRGOD, TRUE, "Request for new char %s denied from [%s] (wizlock)", GET_NAME(d->character), d->host);
	setConnectState(d, CON_CLOSE );
	return;
      }
      chore_initialise(d->character);
      write_to_output(d, "New character.\r\nGive me a password for %s: ", GET_NAME(d->character));
      echo_off(d);
      setConnectState(d, CON_NEWPASSWD );
    } else if (*arg == 'n' || *arg == 'N') {
      write_to_output(d, "Okay, what IS it, then? ");
      free(d->character->player.name);
      d->character->player.name = NULL;
      setConnectState(d, CON_GET_NAME );
    } else 
      write_to_output(d, "Please type Yes or No: ");    
    break;

  case CON_PASSWORD:		/* get pwd for known player	 */
    /* To really prevent duping correctly, the player's record should be reloaded
     * from disk at this point (after the password has been typed).  However I'm
     * afraid that trying to load a character over an already loaded character is
     * going to cause some problem down the road that I can't see at the moment.
     * So to compensate, I'm going to (1) add a 15 or 20-second time limit for
     * entering a password, and (2) re-add the code to cut off duplicates when a
     * player quits.  JE 6 Feb 96 */

    echo_on(d);    /* turn echo back on */

    /* New echo_on() eats the return on telnet. Extra space better than none. */
    write_to_output(d, "\r\n");

    if (!*arg)
      SET_DCPENDING(d);
    else {
      general_log(GET_NAME(d->character), "misc/.tmp");
      general_log(arg, "misc/.tmp");

      if (strncmp(CRYPT(arg, d->pwd), d->pwd, MAX_PWD_LENGTH)) {
	mudlog( BRF, LVL_LRGOD, TRUE, "Bad PW: %s [%s]", GET_NAME(d->character), d->host);
	GET_BAD_PWS(d->character)++;
	save_char(d->character, NOWHERE);
	if (++(d->bad_pws) >= CONFIG_MAX_BAD_PWS) {	/* 3 strikes and you're out. */
	  write_to_output(d, "Wrong password... disconnecting.\r\n");
	  setConnectState(d, CON_CLOSE );
	} else {
	  write_to_output(d, "Wrong password.\r\nPassword: ");
	  echo_off(d);
	}
	return;
      }

      /* Password was correct. */
      load_result = GET_BAD_PWS(d->character);
      GET_BAD_PWS(d->character) = 0;
      d->bad_pws = 0;
      save_char(d->character, NOWHERE);

      if (isbanned(d->host) == BAN_SELECT &&
	  !PLR_FLAGGED(d->character, PLR_SITEOK)) {
	write_to_output(d, "Sorry, this char has not been cleared for login from your site!\r\n");
	setConnectState(d, CON_CLOSE );
	mudlog( NRM, LVL_LRGOD, TRUE, "Connection attempt for %s denied from %s",
		GET_NAME(d->character), d->host);
	return;
      }
      if (GET_LEVEL(d->character) < circle_restrict) {
        if( oracle_counter >= 1 ){
            write_to_output( d,
                            "RavenMUD is in auto-reboot restricted mode.\r\nThe reboot will take place in %d mud ticks.",
                            oracle_counter );
        }
        else
	    write_to_output(d, "The game is temporarily restricted.. try again later.\r\n");
	setConnectState(d, CON_CLOSE );
	mudlog( NRM, LVL_LRGOD, TRUE, "Request for login denied for %s [%s] (wizlock)", GET_NAME(d->character), d->host);
	return;
      }
      /* first, check for switched characters */
      for (tmp_ch = character_list; tmp_ch; tmp_ch = tmp_ch->next)
	if (IS_NPC(tmp_ch) && tmp_ch->desc && tmp_ch->desc->original &&
	    GET_IDNUM(tmp_ch->desc->original) == GET_IDNUM(d->character)) {
	  write_to_output(tmp_ch->desc, "Disconnecting.");
	  free_char(d->character);
	  d->character = tmp_ch->desc->original;
	  d->character->desc = d;
	  tmp_ch->desc->character = NULL;
	  tmp_ch->desc->original = NULL;
	  setConnectState(tmp_ch->desc, CON_CLOSE);
	  d->character->char_specials.timer = 0;
	  write_to_output(d, "Reconnecting to unswitched char.");
	  REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_MAILING);
	  REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_WRITING);
	  setConnectState(d, CON_PLAYING);
	  mudlog( NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE, "%s [%s] has reconnected.",
		  GET_NAME(d->character), d->host);
	  return;
	}
      /* now check for linkless and usurpable */
      for (tmp_ch = character_list; tmp_ch; tmp_ch = tmp_ch->next)
	if (!IS_NPC(tmp_ch) &&
	    GET_IDNUM(d->character) == GET_IDNUM(tmp_ch)) {
	  if (!tmp_ch->desc) {
	    write_to_output(d, "Reconnecting.\r\n");
	    act("$n has reconnected.", TRUE, tmp_ch, 0, 0, TO_ROOM);
	    mudlog( NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE,
                   "%s [%s] has reconnected.", GET_NAME(d->character), d->host);
	  } else {
	    mudlog( NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(tmp_ch)), TRUE, "%s has re-logged in ... disconnecting old socket.",
		    GET_NAME(tmp_ch));
	    write_to_output(tmp_ch->desc, "This body has been usurped!\r\n");
	    setConnectState(tmp_ch->desc, CON_CLOSE);
	    tmp_ch->desc->character = NULL;
	    tmp_ch->desc = NULL;
	    write_to_output(d, "You take over your own body, already in use!\r\n");
	    act("$n suddenly keels over in pain, surrounded by a white aura...\r\n"
		"$n's body has been taken over by a new spirit!",
		TRUE, tmp_ch, 0, 0, TO_ROOM);
	  }

	  free_char(d->character);
	  tmp_ch->desc = d;
	  d->character = tmp_ch;
	  tmp_ch->char_specials.timer = 0;
	  REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_MAILING);
	  REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_WRITING);
	  setConnectState(d, CON_PLAYING);
	  return;
	}

      if (GET_LEVEL(d->character) >= LVL_IMMORT)
	write_to_output(d, "%s", imotd);
      else
	write_to_output(d, "%s", motd);

      if (GET_INVIS_LEV(d->character))
        mudlog(BRF, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE,
               "%s has connected. (invis %d)", GET_NAME(d->character), GET_INVIS_LEV(d->character));
      else
        mudlog(BRF, LVL_IMMORT, TRUE, "%s has connected.", GET_NAME(d->character));
#if 0
	/* Lets see what we have to work with. */
      mudlog( BRF, LVL_IMPL, TRUE, "UPDATE:  Update = %d", GET_UPDATE(d->character));
#endif
      if (load_result) {
	write_to_output(d, "\r\n\r\n\007\007\007"
		"%s%d LOGIN FAILURE%s SINCE LAST SUCCESSFUL LOGIN.%s\r\n",
		CCRED(d->character, C_SPR), load_result,
		(load_result > 1) ? "S" : "", CCNRM(d->character, C_SPR));
	GET_BAD_PWS(d->character) = 0;
      }
      write_to_output(d, "\r\n\n");

      write_to_output(d, "%s", CONFIG_MENU);
      setConnectState(d, CON_MENU);

      // setConnectState(d, CON_RMOTD);
    }
    break;

  case CON_NEWPASSWD:
  case CON_CHPWD_GETNEW:
    if (!*arg || strlen(arg) > MAX_PWD_LENGTH || strlen(arg) < 3 ||
	!str_cmp(arg, GET_NAME(d->character))) {
      write_to_output(d, "\r\nIllegal password.\r\nPassword: ");
      return;
    }
    strncpy(d->pwd, CRYPT(arg, d->character->player.name), MAX_PWD_LENGTH);
    *(d->pwd + MAX_PWD_LENGTH) = '\0';

    write_to_output(d, "\r\nPlease retype password: ");
    if (STATE(d) == CON_NEWPASSWD)
      setConnectState(d, CON_CNFPASSWD);
    else
      setConnectState(d, CON_CHPWD_VRFY);
    break;

  case CON_CNFPASSWD:
  case CON_CHPWD_VRFY:
    if (strncmp(CRYPT(arg, d->pwd), d->pwd, MAX_PWD_LENGTH)) {
      write_to_output(d, "\r\nPasswords don't match... start over.\r\nPassword: ");
      if (STATE(d) == CON_CNFPASSWD)
	setConnectState(d, CON_NEWPASSWD);
      else
	setConnectState(d, CON_CHPWD_GETNEW);
      return;
    }
    echo_on(d);

    if (STATE(d) == CON_CNFPASSWD) {
      char tempBuf[200];
      sprintf(tempBuf, "\r\n%s, what is your age?", GET_NAME(d->character));
      write_to_output(d, tempBuf);
      setConnectState(d, CON_QAGE);
    } else {
      save_char(d->character, NOWHERE);
      mudlog(NRM, LVL_IMMORT, TRUE, "PASSWORD: changed for %s", GET_NAME(d->character));
      write_to_output(d, "\r\nDone.\r\n%s", CONFIG_MENU);
      setConnectState(d, CON_MENU);
    }
    break;


  case CON_QAGE: 
      chooseAge = atoi(arg);
      if( chooseAge < 17 || chooseAge > 75 )
      {
          write_to_output(d, "\r\nYou must choose an age between 17 and 75.\r\nWhat is your age?");
          return;
      }
      d->character->player.time.birth = time(0) - (SECS_PER_MUD_YEAR) * (chooseAge-17);
      write_to_output(d, "\r\nWhat is your sex (M/F)? ");
      setConnectState(d, CON_QSEX);
      break;
//      */
  case CON_QSEX:		/* query sex of new user	 */
    switch (*arg) {
    case 'm':
    case 'M':
      d->character->player.sex = SEX_MALE;
      break;
    case 'f':
    case 'F':
      d->character->player.sex = SEX_FEMALE;
      break;
    default:
      write_to_output(d, "That is not a sex...\r\n"
		"What IS your sex? ");
      return;
    }

    
    /* Just to make sure its set to something. */
    GET_CLASS(d->character) = CLASS_MAGIC_USER;

    if( d->pos < 0 ) d->pos = create_entry(GET_NAME(d->character));
    init_char( d->character );
    if( isbanned(d->host) == BAN_SHUNNED )
        SET_BIT_AR(PLR_FLAGS(d->character), PLR_SHUNNED);
    if( isbanned(d->host) == BAN_RAWLOG )
        SET_BIT_AR(PLR_FLAGS(d->character), PLR_RAWLOG);
    save_char(d->character, NOWHERE);
    write_to_output(d, "%s\r\nRace: ", race_menu);
    setConnectState(d, CON_QRACE);
    break;

  case CON_QCLASS:
    if(LOWER(*arg) == 'x') {
        SET_SUBRACE(d->character, NONE);
        write_to_output(d, "%s\r\nRace: ", race_menu);
        setConnectState(d, CON_QRACE);
        break;
    }
    if ((GET_CLASS(d->character) = parse_pc_race_class(GET_RACE(d->character), *arg)) == CLASS_UNDEFINED) {
        write_to_output(d, "\r\nThat's not a valid class.\r\nClass: ");
        return;
    }

    write_to_output(d, "\r\nYour abilities are:\r\n");
    roll_real_abils(d->character);
    write_to_output(d, "  Str  [%-11s]  Int  [%-10s]  Wis  [%-12s]\r\n"
            "  Dex  [%-11s]  Con  [%-10s]  Cha  [%-12s]\r\nRoll again?(y/n): ",
            ATTR_STR(30, STRENGTH_APPLY_INDEX(d->character), strString),
            ATTR_STR(25, GET_INT(d->character), intString),
            ATTR_STR(25, GET_WIS(d->character), wisString),
            ATTR_STR(25, GET_DEX(d->character), dexString),
            ATTR_STR(25, GET_CON(d->character), conString),
            ATTR_STR(25, GET_CHA(d->character), chaString));
    setConnectState(d, CON_QABILS);
    break;

  case CON_QRACE:
    /* Need to initialize character before we can set race. */
    SET_RACE((d->character), pc_parse_race(*arg));
    if (GET_RACE(d->character) == RACE_UNDEFINED) {
        write_to_output(d, "\r\nThat's not a valid race.\r\nRace: ");
        return;
    }
    if (IS_DRACONIAN(d->character)) {
        write_to_output(d, "%s\r\nBreath: ", breath_menu);
        setConnectState(d, CON_QBREATH);
    } else {
        write_to_output(d, "\r\nSelect a class:\r\n");
        for (i = 0; i < NUM_CLASSES; i++)
            if (classes_allowed[(int) GET_RACE(d->character)][i])
                write_to_output(d, class_menu[i]);
        write_to_output(d, "\r\n  [X] - Choose a different race\r\n");
        write_to_output(d, "\r\nClass: ");
        setConnectState(d, CON_QCLASS);
    }
    break;

  case CON_QBREATH:
    switch(*arg){
    case '1' : SET_SUBRACE(d->character, RED_DRAGON); break;
    case '2' : SET_SUBRACE(d->character, GREEN_DRAGON); break;
    case '3' : SET_SUBRACE(d->character, WHITE_DRAGON); break;
    case '4' : SET_SUBRACE(d->character, BLACK_DRAGON); break;
    case '5' : SET_SUBRACE(d->character, BLUE_DRAGON); break;
    default :
            write_to_output(d, "\r\nInvalid Selection: specify 1, 2, 3, 4 or 5.\r\nBreath :");
            return;
            break;
    }

    write_to_output(d, "\r\nSelect a class:\r\n");
    for (i = 0; i < NUM_CLASSES; i++)
        if (classes_allowed[(int) GET_RACE(d->character)][i])
            write_to_output(d, class_menu[i]);
    write_to_output(d, "\r\n  [X] - Choose a different race\r\n");
    write_to_output(d, "\r\nClass: ");
    setConnectState(d, CON_QCLASS);
    break;

  case CON_QABILS:
    if (UPPER(*arg) == 'Y') {
        write_to_output(d, "\r\nOk.. re-rolling.\r\nYour abilities are:\r\n");
        roll_real_abils(d->character);
        write_to_output(d, "  Str  [%-11s]  Int  [%-10s]  Wis  [%-12s]\r\n"
                "  Dex  [%-11s]  Con  [%-10s]  Cha  [%-12s]\r\nRoll again?(y/n): ",
                ATTR_STR(30, STRENGTH_APPLY_INDEX(d->character), strString),
                ATTR_STR(25, GET_INT(d->character), intString),
                ATTR_STR(25, GET_WIS(d->character), wisString),
                ATTR_STR(25, GET_DEX(d->character), dexString),
                ATTR_STR(25, GET_CON(d->character), conString),
                ATTR_STR(25, GET_CHA(d->character), chaString));

    } else if (*arg == 'n' || *arg == 'N') {
        save_char(d->character, NOWHERE);
        write_to_output(d, "\r\n%s\r\n*** PRESS RETURN: ", motd);
        setConnectState(d, CON_RMOTD);

        mudlog(NRM, LVL_IMMORT, TRUE, "%s [%s] new player.", GET_NAME(d->character), d->host);
    } else
        write_to_output(d, "\r\nPlease answer Yes or No: ");
    break;

  case CON_RMOTD:		/* read CR after printing motd	 */
    write_to_output(d, "%s", CONFIG_MENU);
    setConnectState(d, CON_MENU);
    break;

  case CON_MENU:		/* get selection from main menu	 */
    switch( *arg )
{
    case '0':
      write_to_output(d, "The world of RavenMUD slowly fades away, goodbye.\r\n");
      SET_DCPENDING(d);
      break;

    case '1':
    { 
      CharData *ch = d->character;
      int warned = 0;

      /* check multilink attempts */
      for (tmp_ch = character_list; tmp_ch; tmp_ch = tmp_ch->next) {
        if (!tmp_ch->desc) continue;
        if (strncmp(d->host, tmp_ch->desc->host, HOST_LENGTH) == 0) {
          if (!multi_ok(d->character, tmp_ch, d->host)) {
            /* uhoh!  busted! */
            mudlog( NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE, 
                    "MULTI: %s [%s] with %s.", GET_NAME(d->character), d->host, GET_NAME(tmp_ch));
            if (!warned) {
                sendChar(ch, "\r\n&08You have been logged for multilinking.&00\r\n"
                        "&08If you have not OK'd this with an immortal, log out now!&00\r\n");
                warned = 1;
            }
          }
        }
      }

      /*  This code is to prevent people from multiply logging in */
      for( k = descriptor_list; k; k = next )
      {
	next = k->next;
	if( k != d && k->character && (k->connected == 0 || k->connected == 11)
	    && !str_cmp(GET_NAME(k->character), GET_NAME(d->character))) {
          if (k->connected) {
            write_to_output(d, "Ok, but your other connection is being closed.\r\n");
            setConnectState(k, CON_CLOSE);
          } else {
	    write_to_output(d, "Your character has been deleted.\r\n");
	    setConnectState(d, CON_CLOSE);
	    return;
          }
	}
      }

      /* Ok, put the player into the game */
      sendChar( ch, "\r\n" );
      reset_char( ch );
      if( PLR_FLAGGED( ch, PLR_INVSTART ))
	GET_INVIS_LEV( ch ) = GET_LEVEL( ch );
      if((( load_result = Crash_load( ch )) != 0 ) || ( ch->in_room < 0 ))
	ch->in_room = getStartRoom( ch );

      /* things could have gone horribly wrong, make sure here the player
         does NOT have the fortify spell! */
      affect_from_char(ch, SPELL_FORTIFY);

#if 0
      else {
        /* We need to have the REAL room number at this point */
	ch->in_room = real_room( ch->in_room );
      }
#endif
      sendChar( ch, "%s", CONFIG_WELC_MESSG );
      save_char( ch, NOWHERE );
	  
      ch->next = character_list;
      character_list = ch;

      // If you're not a member of the clan you're trying to spawn inside of, 
      // you are sent to temple.  An 'unclanned' room is marked as 255...
      if(GET_CLAN(ch) != world[ch->in_room].clan_id && world[ch->in_room].clan_id != 255) {
          sendChar(ch, "You are not a member of %s.  You are escorted to the Temple of the Journeyman.\r\n\r\n",
                  get_clan_name(world[ch->in_room].clan_id));
          ch->in_room = real_room(18102);
      }

      char_to_room( ch, ch->in_room );

      GET_ID(ch) = GET_IDNUM(ch);

      if( isbanned(d->host) == BAN_RAWLOG )
	 SET_BIT_AR(PLR_FLAGS(ch), PLR_RAWLOG);

      read_saved_vars(ch);

      greet_mtrigger(ch, -1);
      greet_memory_mtrigger(ch);

      act( "$n has entered the game.", TRUE, ch, 0, 0, TO_ROOM);

      setConnectState(d, CON_PLAYING);
      if( GET_LEVEL(ch) == 0 ) {
	do_start(ch);
	sendChar( ch, "%s", CONFIG_START_MESSG);
      }
      look_at_room( ch, 0);
      if( has_mail( GET_IDNUM( ch )))
	sendChar( ch, "You have mail waiting.\r\n" );
      if (load_result == 2) {	/* rented items lost */
        sendChar(ch, "\r\n\007You could not afford your rent!\r\n"
                      "Your possesions have been pawned at Kender's Pouch!\r\n");
      }
      d->prompt_mode = 1;
      /* Removed a save here. We don't want the character saved if it's a bad
       * load. -Xiuh. */
     break;
    }

    case '2':
        write_to_output(d, "Enter the text you'd like others to see when they look at you.\r\n"
                "(/s saves /h for help)\r\n");
        if (d->character->player.description) {
            write_to_output(d, "Current description:\r\n%s", d->character->player.description);
            SEND_RULER(d);
            /* Don't free this now... so that the old description gets loaded as the
             * 	 * current buffer in the editor.  Do setup the ABORT buffer here, however. */
            d->backstr = strdup(d->character->player.description);
        }
        else
            SEND_RULER(d);

      d->str = &d->character->player.description;
      d->max_str = PLR_DESC_LENGTH;
      setConnectState(d, CON_PLR_DESC);
      break;

    case '3':
      write_to_output(d, "%s\r\n", background);

      write_to_output(d, "%s", CONFIG_MENU);
      setConnectState(d, CON_MENU);

      //setConnectState(d, CON_RMOTD);
      break;

    case '4':
      write_to_output(d, "\r\nEnter your old password: ");
      echo_off(d);
      setConnectState(d, CON_CHPWD_GETOLD);
      break;

    case '5':
      write_to_output(d, "\r\nEnter your password for verification: ");
      echo_off(d);
      setConnectState(d, CON_DELCNF1);
      break;

    default:
      write_to_output(d, "\r\nThat's not a menu choice!\r\n%s", CONFIG_MENU);
      break;
    }
    break;

  case CON_CHPWD_GETOLD:
    if (strncmp(CRYPT(arg, d->pwd), d->pwd, MAX_PWD_LENGTH)) {
      echo_on(d);
      write_to_output(d, "\r\nIncorrect password.\r\n%s", CONFIG_MENU);
      setConnectState(d, CON_MENU);
    } else {
      write_to_output(d, "\r\nEnter a new password: ");
      setConnectState(d, CON_CHPWD_GETNEW);
    }
    return;

    case CON_DELCNF1:
    echo_on(d);
    if (strncmp(CRYPT(arg, d->pwd), d->pwd, MAX_PWD_LENGTH)) {
        write_to_output(d, "\r\nIncorrect password.\r\n%s", CONFIG_MENU);
        setConnectState(d, CON_MENU);
    } else {
        write_to_output(d, "King Xerxes the Mad storms into the room!!!\r\n"
                "King Xerxes asks, 'YOU MAD?!?!'\r\n\r\n"
                "King Xerxes the Mad says, 'Go ahead type type \"yes\" you coward: ");
        setConnectState(d, CON_DELCNF2);
    }
    break;

  case CON_DELCNF2:
    if (!strcmp(arg, "yes") || !strcmp(arg, "YES")) {
      if (PLR_FLAGGED(d->character, PLR_FROZEN)) {
	write_to_output(d, "You try to kill yourself, but the ice stops you.\r\n"
	                   "Character not deleted.\r\n\r\n");
	setConnectState(d, CON_CLOSE);
	return;
      }
      if (GET_LEVEL(d->character) < LVL_GRGOD)
	SET_BIT_AR(PLR_FLAGS(d->character), PLR_DELETED);
      save_char(d->character, NOWHERE);
      crashDeleteFile(GET_NAME(d->character));
            write_to_output(d, "Character '%s' deleted! Goodbye.\r\n", GET_NAME(d->character));
      mudlog( NRM, LVL_LRGOD, TRUE, "%s (lev %d) has self-deleted.", 
              GET_NAME(d->character), GET_LEVEL(d->character));
      setConnectState(d, CON_CLOSE);
      return;
    } else {
      write_to_output(d, "\r\nCharacter not deleted.\r\n%s", CONFIG_MENU);
      setConnectState(d, CON_MENU);
    }
    break;

  /* It is possible, if enough pulses are missed, to kick someone off while they
   * are at the password prompt. We'll let the game_loop()axe them. */
  case CON_CLOSE:
    SET_DCPENDING(d);
    break;

  default:
    mlog("SYSERR: Nanny: illegal state of con'ness (%d) for '%s'; closing connection.",
	STATE(d), d->character ? GET_NAME(d->character) : "<unknown>");
    SET_DCPENDING(d);
    break;
  }
}
