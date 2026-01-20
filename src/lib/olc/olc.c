/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*  _TwyliteMud_ by Rv.                          Based on CircleMud3.0bpl9 *
*    				                                          *
*  OasisOLC - olc.c 		                                          *
*    				                                          *
*  Copyright 1996 Harvey Gilpin.                                          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define _RV_OLC_

#define REDRAW 1

#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "actions/interpreter.h"
#include "general/comm.h"
#include "util/utils.h"
#include "olc/olc.h"
#include "general/color.h"
#include "general/class.h"
#include "magic/spells.h"
#include "olc/oedit.h"
#include "olc/redit.h"
#include "olc/zedit.h"
#include "olc/medit.h"
#include "olc/sedit.h"
#include "olc/qedit.h"
#include "actions/act.clan.h"
#include "scripts/dg_scripts.h"
#include "scripts/dg_olc.h"

/*. Internal function prototypes .*/
int  olc_real_zone(int number); /* defined this in db.c -Vex. */
void olc_saveinfo(CharData *ch);
int  can_edit_mob(int number);

/*. Internal data .*/

struct olc_scmd_data {
  char *text;
  int con_type;
};

struct olc_scmd_data olc_scmd_info[] =
{ {"room", 	CON_REDIT},
  {"object", 	CON_OEDIT},
  {"room",	CON_ZEDIT},
  {"mobile", 	CON_MEDIT},
  {"shop", 	CON_SEDIT},
  {"trigger", 	CON_TRIGEDIT},
  {"quest",     CON_QEDIT},
};

/* Internal Function prototypes  */
static void free_config(struct config_data *data);

/* Only player characters should be using OLC anyway. */
void clear_screen(struct descriptor_data *d)
{
  if (PRF_FLAGGED(d->character, PRF_CLS))
    write_to_output(d, "[H[J");
}

/* Exported utilities */
/* Set the color string pointers for that which this char will see at color
 * level NRM.  Changing the entries here will change the colour scheme
 * throughout the OLC. */
void get_char_colors(struct char_data *ch)
{
  nrm = CCNRM(ch, C_NRM);
  grn = CCGRN(ch, C_NRM);
  cyn = CCCYN(ch, C_NRM);
  yel = CCYEL(ch, C_NRM);
}

/*------------------------------------------------------------*\
 Eported ACMD do_olc function

 This function is the OLC interface.  It deals with all the 
 generic OLC stuff, then passes control to the sub-olc sections.
\*------------------------------------------------------------*/

ACMD(do_olc)
{
  int number = -1, save = 0, real_num;
  DescriptorData *d;

  if (IS_NPC(ch))
    /*. No screwing around .*/
    return;

  if (subcmd == SCMD_OLC_SAVEINFO) {
    olc_saveinfo(ch);
    return;
  }

  /*. Parse any arguments .*/
  two_arguments(argument, buf1, buf2);
  if (!*buf1) {
    /* No argument given .*/
    switch(subcmd) {
      case SCMD_OLC_ZEDIT:
      case SCMD_OLC_REDIT:
        number = world[IN_ROOM(ch)].number;
        break;
      case SCMD_OLC_OEDIT:
      case SCMD_OLC_MEDIT:
      case SCMD_OLC_SEDIT:
      case SCMD_OLC_QEDIT:
      case SCMD_OLC_TRIGEDIT:
        sprintf(buf, "Specify a %s VNUM to edit.\r\n", olc_scmd_info[subcmd].text);
        send_to_char (buf, ch);
        return;
    }
  }
  else if (!isdigit (*buf1)) {
    if (strncmp("save", buf1, 5) == 0) {
      if (!*buf2) {
        send_to_char("Save which zone?\r\n", ch);
        return;
      }
      else  {
        save = 1;
        number = atoi(buf2) * 100;
      }
    }
    else if (subcmd == SCMD_OLC_ZEDIT && GET_LEVEL(ch) >= LVL_GRGOD) {
      if ((strncmp("new", buf1, 4) == 0) && *buf2)
        zNewZone(ch, atoi(buf2));
      else
        send_to_char("Specify a new zone number.\r\n", ch);
      return;
    }
    else {
      send_to_char ("Yikes!  Stop that, someone will get hurt!\r\n", ch);
      return;
    }
  }

  /*. If a numeric argument was given, get it .*/
  if (number == -1)
    number = atoi(buf1);

  /*. Check whatever it is isn't already being edited .*/
  for (d = descriptor_list; d; d = d->next)
    if (d->connected == olc_scmd_info[subcmd].con_type)
      if (d->olc && OLC_NUM(d) == number) {
        sprintf(buf, "That %s is currently being edited by %s.\r\n",
                olc_scmd_info[subcmd].text, GET_NAME(d->character));
        send_to_char(buf, ch);
        return;
      }

  d = ch->desc; 

  /*. Give descriptor an OLC struct .*/
  CREATE(d->olc, struct olc_data, 1);

  /*. Find the zone .*/
  OLC_ZNUM(d) = olc_real_zone(number);
  if (OLC_ZNUM(d) == -1) {
    send_to_char ("Sorry, there is no zone for that number!\r\n", ch); 
    free(d->olc); d->olc = NULL;
    return;
  }

  /*. Everyone but LVL_DEITY+ can only edit zones they have been assigned .*/
  if ((GET_LEVEL(ch) < LVL_DEITY) &&
      (zone_table[OLC_ZNUM(d)].number != GET_OLC_ZONE(ch))) {
    send_to_char("You do not have permission to edit this zone.\r\n", ch); 
    free(d->olc); d->olc = NULL;
    return;
  }

  if (IS_SET_AR(PLR_FLAGS(ch), PLR_RETIRED) && 
      zone_table[OLC_ZNUM(d)].number != GET_OLC_ZONE(ch)) {
      send_to_char("Sit back and relax, after all your retired!\r\n", ch);
      free(d->olc); d->olc = NULL;
      return;
  }

 
  if(save) {
    switch(subcmd) {
      case SCMD_OLC_REDIT: 
        send_to_char("Saving all rooms in zone.\r\n", ch);
        mudlog( CMP, LVL_BUILDER, TRUE, "OLC: %s saves rooms for zone %d",
		 GET_NAME(ch), zone_table[OLC_ZNUM(d)].number);
        rSaveToDisk(d); 
        break;
      case SCMD_OLC_ZEDIT:
        send_to_char("Saving all zone information.\r\n", ch);
        mudlog( CMP, LVL_BUILDER, TRUE, "OLC: %s saves zone info for zone %d",
		 GET_NAME(ch), zone_table[OLC_ZNUM(d)].number);
        zSaveToDisk(d); 
        break;
      case SCMD_OLC_OEDIT:
        send_to_char("Saving all objects in zone.\r\n", ch);
        mudlog( CMP, LVL_BUILDER, TRUE, "OLC: %s saves objects for zone %d",
		 GET_NAME(ch), zone_table[OLC_ZNUM(d)].number);
        oSaveToDisk(d); 
        break;
      case SCMD_OLC_MEDIT:
        send_to_char("Saving all mobiles in zone.\r\n", ch);
        mudlog( CMP, LVL_BUILDER, TRUE, "OLC: %s saves mobs for zone %d",
		 GET_NAME(ch), zone_table[OLC_ZNUM(d)].number);
        mSaveToDisk(d); 
        break;
      case SCMD_OLC_SEDIT:
        send_to_char("Saving all shops in zone.\r\n", ch);
        mudlog( CMP, LVL_BUILDER, TRUE, "OLC: %s saves shops for zone %d",
		 GET_NAME(ch), zone_table[OLC_ZNUM(d)].number);
        sSaveToDisk(d); 
        break;
      case SCMD_OLC_QEDIT:
        send_to_char("Saving all quests in zone.\r\n", ch);
        mudlog( CMP, LVL_BUILDER, TRUE, "OLC: %s saves quests for zone %d",
		 GET_NAME(ch), zone_table[OLC_ZNUM(d)].number);
        qSaveToDisk(d); 
        break;
    }
    free(d->olc); d->olc = NULL;
    return;
  }
 
  OLC_NUM(d) = number;

  /*. Steal players descriptor start up subcommands .*/
  switch(subcmd) {
    case SCMD_OLC_TRIGEDIT:
      if ((real_num = real_trigger(number)) >= 0)
        trigedit_setup_existing(d, real_num);
      else
        trigedit_setup_new(d);
      STATE(d) = CON_TRIGEDIT;
      break;
    case SCMD_OLC_REDIT:
      real_num = real_room(number);
      if (real_num >= 0)
        rSetupExisting(d, real_num);
      else
        rSetupNew(d, number);
      setConnectState(d, CON_REDIT);
      break;
    case SCMD_OLC_ZEDIT:
      real_num = real_room(number);
      if (real_num < 0) {
         send_to_char("That room does not exist.\r\n", ch); 
         free(d->olc); d->olc = NULL;
         return;
      }
      zSetup(d, real_num);
      setConnectState(d, CON_ZEDIT);
      break;
    case SCMD_OLC_MEDIT:
      if(!can_edit_mob(number)) {
          //sendChar(ch, "That mob has been locked from editing.\r\n");
          //return;
      }
      real_num = real_mobile(number);
      if (real_num < 0)
        mSetupNew(d);
      else
        mSetupExisting(d, real_num);
      setConnectState(d, CON_MEDIT);
      break;
    case SCMD_OLC_OEDIT:
      //if(!can_edit_obj(number)) {
      //    sendChar(ch, "That object has been locked from editing.\r\n");
      //    return;
      //}
      real_num = real_object(number);
      if (real_num >= 0)
        oSetupExisting(d, real_num);
      else
        oSetupNew(d);
      setConnectState(d, CON_OEDIT);
      break;
    case SCMD_OLC_SEDIT:
      real_num = sRealShop(number);
      if (real_num >= 0)
        sSetupExisting(d, real_num);
      else
        sSetupNew(d);
      setConnectState(d, CON_SEDIT);
      break;
    case SCMD_OLC_QEDIT:
      real_num = real_quest(number);
      if (real_num >= 0)
        qSetupExisting(d, real_num);
      else
        qSetupNew(d);
      setConnectState(d, CON_QEDIT);
      break;
  }
  act("$n starts using OLC.", TRUE, d->character, 0, 0, TO_ROOM);
  SET_BIT_AR(PLR_FLAGS (ch), PLR_WRITING);
}
/*------------------------------------------------------------*\
 Internal utlities 
\*------------------------------------------------------------*/

// can_edit_mob is a hardcoded means of blocking mobs from being edited.
// Used primarily for important mobs that need to be finely calibrated (players'
// pets, for example)
int can_edit_mob(int number) {
    if (number >= 100 && number <= 300)
        return FALSE;

    switch(number) {
        case 43912: // Dwerhi
        case 43901: // Chishan guard
            return FALSE;
        default:
            return TRUE;
    }

}

// can_edit_obj is a hardcoded means of blocking objects from being edited.
// Used primarily for important objects that need to be finely calibrated
// (quest items, for example)
int can_edit_obj(int number) {

    switch(number) {
        case 38850: // Quietus
        case 38625: // Lochaber (aka darksword)
        case 13230: // Avernus
        case 27199: // Sequioa Staff
        case 38210: // gloves of harmony
        case 43919: // bonestaff named 'soul reaver'
        case 18196: // Evil sanc robes
        case 18195: // Neutral sanc robes
        case 14784: // Good sanc robes
        case 21280: // Holy Avenger
        case 13201: // Wand of death
        case  9015: // the Touch of Chaos
        case 38312: // Armor of the mind
        case 40498: // Shield of the Dragonslayer
        case 38113: // razor edged boots
            return FALSE;
        default:
            return TRUE;
    }
}


void olc_saveinfo(CharData *ch)
{ struct olc_save_info *entry;
  static char *save_info_msg[6] = { "Rooms", "Objects", "Zone info", "Mobiles", "Shops", "Quests" };

  if (olc_save_list)
    send_to_char("The following OLC components need saving:-\r\n", ch);
  else
    send_to_char("The database is up to date.\r\n", ch);

  for (entry = olc_save_list; entry; entry = entry->next)
  { sprintf(buf, " - %s for zone %d.\r\n", 
	save_info_msg[(int)entry->type],
	entry->zone 
    );
    send_to_char(buf, ch);
  }
}

int olc_real_zone(int number)
{ int counter;
  for (counter = 0; counter <= top_of_zone_table; counter++)
    if ((number >= (zone_table[counter].number * 100)) &&
        (number <= (zone_table[counter].top)))
      return counter;

  return -1;
}

/*------------------------------------------------------------*\
 Exported utlities 
\*------------------------------------------------------------*/

/*. Add an entry to the 'to be saved' list .*/

void olc_add_to_save_list(int zone, byte type)
{ struct olc_save_info *new;

  /*. Return if it's already in the list .*/
  for(new = olc_save_list; new; new = new->next)
    if ((new->zone == zone) && (new->type == type))
      return;

  CREATE(new, struct olc_save_info, 1);
  new->zone = zone;
  new->type = type;
  new->next = olc_save_list;
  olc_save_list = new;
}

/*. Remove an entry from the 'to be saved' list .*/

void olc_remove_from_save_list(int zone, byte type)
{ struct olc_save_info **entry;
  struct olc_save_info *temp;

  for(entry = &olc_save_list; *entry; entry = &(*entry)->next)
    if (((*entry)->zone == zone) && ((*entry)->type == type))
    { temp = *entry;
      *entry = temp->next;
      free(temp);
      return;
    }
}


/*. This procedure removes the '\r\n' from a string so that it may be
    saved to a file.  Use it only on buffers, not on the oringinal
    strings.*/

void strip_string(char *buffer)
{ register char *ptr, *str;

  ptr = buffer;
  str = ptr;

  while((*str = *ptr))
  { str++;
    ptr++;
    if (*ptr == '\r')
      ptr++;
  }
}


/*. This procdure frees up the strings and/or the structures
    attatched to a descriptor, sets all flags back to how they
    should be .*/

void cleanup_olc(DescriptorData *d, byte cleanup_type)
{ 
  if (d->olc)
  {
    /*. Check for room .*/
    if(OLC_ROOM(d))
    { /*. rFreeRoom performs no sanity checks, must be carefull here .*/
      switch(cleanup_type)
      { case CLEANUP_ALL:
          rFreeRoom(OLC_ROOM(d));
          break;
        case CLEANUP_STRUCTS:
          free(OLC_ROOM(d));
          break;
    case CLEANUP_CONFIG:
      free_config(OLC_CONFIG(d));
      break;
        default:
          /*. Caller has screwed up .*/
          break;
      }
    }
  
    /*. Check for object .*/
    if(OLC_OBJ(d))
    { /*. free_obj checks strings arn't part of proto .*/
      free_obj(OLC_OBJ(d));
    }

    /*. Check for mob .*/
    if(OLC_MOB(d))
    { /*. free_char checks strings arn't part of proto .*/
/*      free_char(OLC_MOB(d));*/
      mFreeMob(OLC_MOB(d));
    }
  
    /*. Check for zone .*/
    if(OLC_ZONE(d))
    { /*. cleanup_type is irrelivent here, free everything .*/
      free(OLC_ZONE(d)->name);
      free(OLC_ZONE(d)->cmd);
      free(OLC_ZONE(d));
    }

    /*. Check for shop .*/
    if(OLC_SHOP(d))
    { /*. sFreeShop performs no sanity checks, must be carefull here .*/
      switch(cleanup_type)
      { case CLEANUP_ALL:
          sFreeShop(OLC_SHOP(d));
          break;
        case CLEANUP_STRUCTS:
          free(OLC_SHOP(d));
          break;
        default:
          /*. Caller has screwed up .*/
          break;
      }
    }

    /* check for quest */
    if (OLC_QUEST(d)) {
      switch (cleanup_type) {
        case CLEANUP_ALL:
          qFreeQuest(OLC_QUEST(d));
          break;
        case CLEANUP_STRUCTS:
          free(OLC_QUEST(d));
          break;
      }
    }

   /* Used for cleanup of Hedit */
  if (OLC_HELP(d))  {
    switch(cleanup_type)  {
      case CLEANUP_ALL:
 	free_help(OLC_HELP(d));
 	break;
      case CLEANUP_STRUCTS:
        free(OLC_HELP(d));
        break;
      default:
 	break;
    }
  }

  /* Free storage if allocated (tedit, aedit, and trigedit). This is the command
   * list - it's been copied to disk already, so just free it -Welcor. */
   if (OLC_STORAGE(d)) {
     free(OLC_STORAGE(d));
     OLC_STORAGE(d) = NULL;
   }

    /*. Restore desciptor playing status .*/
    if (d->character) {
      REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_WRITING);
      act("$n stops using OLC.", TRUE, d->character, 0, 0, TO_ROOM);

    if (cleanup_type == CLEANUP_CONFIG)
      mudlog(BRF, LVL_IMMORT, TRUE, "OLC: %s stops editing the game configuration", GET_NAME(d->character));
    else if (STATE(d) == CON_TEDIT)
      mudlog(BRF, LVL_IMMORT, TRUE, "OLC: %s stops editing text files.", GET_NAME(d->character));
    else if (STATE(d) == CON_HEDIT)
      mudlog(CMP, LVL_IMMORT, TRUE, "OLC: %s stops editing help files.", GET_NAME(d->character));
    else
      mudlog(CMP, LVL_IMMORT, TRUE, "OLC: %s stops editing zone %d allowed zone %d", GET_NAME(d->character), zone_table[OLC_ZNUM(d)].number, GET_OLC_ZONE(d->character));
      setConnectState( d, CON_PLAYING );
    }

    free(d->olc);
    d->olc = NULL;
  }
}

void split_argument(char *argument, char *tag)
{
  char *tmp = argument, *ttag = tag, *wrt = argument;
  int i;

  for (i = 0; *tmp; tmp++, i++) {
    if (*tmp != ' ' && *tmp != '=')
      *(ttag++) = *tmp;
    else if (*tmp == '=')
      break;
  }

  *ttag = '\0';

  while (*tmp == '=' || *tmp == ' ')
    tmp++;

  while (*tmp)
    *(wrt++) = *(tmp++);

  *wrt = '\0';
}

static void free_config(struct config_data *data)
{
  /* Free strings. */
  free_strings(data, OASIS_CFG);

  /* Free the data structure. */
  free(data);
}

/* Vex utils */

/* Number of letters being used for bitvector resolution(a..z, A..Z) */
#define NUM_LETTERS 52

/* -------------------------------------------------------------------------- */
/* These functions are just used internally. Vex.                             */
/* -------------------------------------------------------------------------- */

/* Returns a letter equivalent to the integer passed in, 0 = a, 1 = b, etc. */
/* Just used to help print help pages. */
char integer_to_letter(int i)
{
	char alphabet[NUM_LETTERS] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	if ((i < 0) || (i >= NUM_LETTERS)) { /* caller is a twit. */
		mudlog(NRM, LVL_IMMORT, TRUE, "(OLC): Integer[%d] is out of bounds for integer_to_letter", i);
		return '0';
	}
	return alphabet[i];
}

/* Assumes a alpha character is passed in, returns the "number". */
/* Basically equivalent to counting upwards with letters i.e. */
/* 1 = a, 2 = b, 3 = c, ..., 26 = z, 27 = A, ..., 52 = Z */
int char_to_number(char c)
{
	if (islower(c))
		return ((c - 'a'));
	else if (isupper(c))
		return (26 + (c - 'A'));
	else /* caller is a twit */
		mudlog(NRM, LVL_IMMORT, TRUE, "(OLC) Bad argument %c passed to char_to_number.", c);
	return 0;
}

/* -------------------------------------------------------------------------- */
/* These functions are to manipulate flags and integer values consistently.   */
/* Vex.									      */
/* -------------------------------------------------------------------------- */

/* Simply returns 2 raised to the power i */
u_int find_bitvector(int i)
{
	if (i < 0) {
		mudlog(NRM, LVL_IMMORT, TRUE, "(OLC): Negative integer %d passed to find_bitvector.", i);
		return 0;
	}
	else
		return 1 << i;
}


/* Assumes a alpha character is passed in, returns the "bit". */
/* a = 1, b = 2, c = 4, d = 8, etc. */
u_int char_to_bit(char c)
{
	if (isalpha(c)) /* no problemo */
		return (1 << char_to_number(c));
	else /* caller is a twit */
		mudlog(NRM, LVL_IMMORT, TRUE, "(OLC): Bad argument: %c passed to char_to_bit", c);
	return 0;
}

/*
** Takes one argument from theArgument, determines if this argument is a
** valid name. Returns this name(as an integer) if it is, other wise
** returns it will return -1 if the name is'nt valid, or -2 if theArgument
** is null. 
** Note that one argument will be pulled from theArgument.
*/
int
getType( DescriptorData *d,
         char *theArgument,
         char *names[], 
         const int num_names,
         const char *this_type )
{
  int i;
  char ourArg[MAX_INPUT_LENGTH];

  if( !*theArgument )
  {
#if 0
    mudlog(NRM, LVL_IMMORT, TRUE, "(OLC): ERROR! Null value argument passed to getType!!" );
#endif
    return -2;
  }

  case_chop(theArgument, ourArg, theArgument);
  if( PRF_FLAGGED(d->character, PRF_OLCV) )
  {
    /* They should enter its actual name */
    for( i = 0; i < num_names; i++ )
    {
      if( !strncasecmp(ourArg, names[i], strlen(ourArg)) )
        break; /* Valid string found, so exit. */
    }
  }
  else
  { /* They should be entering digits. */
    if( is_integer(ourArg) )
      i = atoi(ourArg);
    else
    {
      return -1;
    }
  }

  if( (i < 0) || (i >= num_names) )
  {
    return -1;
  }
  return i;
}

/*
** Takes one argument from theArgument, determines if this argument is a
** valid attack type. Returns this attack(as an integer) if it is, other wise
** returns it will return -1 if the attack is'nt valid, or -2 if theArgument
** is null. Basically very similiar to getType above.
** Note that one argument will be pulled from theArgument.
 *
 * Note2: Appears this is no longer being used in the code. Commented out -Xiuh
int getAttackType(DescriptorData *d, char *theArgument)
{
    int i;
    char ourArg[MAX_INPUT_LENGTH];

    if (!*theArgument) { // Problem!
	mudlog(NRM, LVL_IMMORT, TRUE, "(OLC): ERROR! Null value argument passed to getAttackType!!");
	return -2;
    }
    case_chop(theArgument, ourArg, theArgument);
    if (PRF_FLAGGED(d->character, PRF_OLCV)) // They should enter its actual name
	for(i = 0; i < NUM_WEAPON_TYPES; i++) {
	    if ( !strncasecmp(ourArg, attack_hit_text[i].singular, strlen(ourArg)) )
		break; // Valid string found, so exit.
	}
     else { // They should be entering digits.
        if (is_integer(ourArg))
            i = atoi(ourArg);
        else {
            sendChar(d->character, "%s is not a valid attack type.\r\n", ourArg);
            return -1;
    	}
    }
    if ((i < 0) || (i >= NUM_WEAPON_TYPES)) {
	sendChar(d->character, "%s is not a known attack type.\r\n", ourArg);
	return -1;
    }
    return i;
}
*/

/*
** This function will toggle the bitvector passed in if its a valid member
** of the_bits. Note that if ch is is simple mode, one space delimited
** argument will be taken from theArgument, otherwise, they will ALL be
** taken.
*/
int toggleBit(DescriptorData *d,
              char *theArgument,
              char *the_bits[],
              const int num_bits,
              const char *this_flag,
              u_int *bitvector)
{
    char let_array[MAX_INPUT_LENGTH], ourArg[MAX_INPUT_LENGTH];
    int i;

    if( !*theArgument ) return 1; /* Digger: Used to return -2 internal problem */

    case_chop(theArgument, ourArg, theArgument);
    if (PRF_FLAGGED(d->character, PRF_OLCV)) { /* They should enter their actual names */
	while (*ourArg) {
	    for(i = 0; i < num_bits; i++) {
	        if ( !strncasecmp(ourArg, the_bits[i], strlen(ourArg)) )
		    break; /* Valid string found, so exit loop. */
	    }
	    if (i >= num_bits) /* Then we did'nt find anything. */
	        sendChar(d->character, "%s is'nt a valid %s flag.\r\n", ourArg, this_flag);
	    else
		*bitvector = *bitvector ^ find_bitvector(i);
	    case_chop(theArgument, ourArg, theArgument);
	}
    }
    else { /* They should be entering letters. */
	strxfrm(let_array, ourArg, strlen(ourArg));
	for (i = 0; i < strlen(ourArg); i++) {
	    if (let_array[i] == '0') { /* Clear all flags and exit */
		*bitvector = 0;
		return -1; /* Digger: Used to return 1 Success */
	    }
	    else if (let_array[i] == '+') { /* Set the following bits */
		*bitvector = 0;
	    }
	    else if (isalpha(let_array[i])) { /* Ignore non-letters. */
		if (char_to_number(let_array[i]) >= num_bits)
		    sendChar(d->character, "%c is undefined.\r\n", let_array[i]);
		else
		    *bitvector = *bitvector ^ char_to_bit(let_array[i]);
	    }
	    else { /* Complain and exit */
		sendChar(d->character, "Unknown bit letter, must be a..z, A..Z, or 0.\r\n");
		return -1; /* They need typing lessons */
	    }
	}
    }
    return -1; /* Digger: Used to return 1 No problemo */
}

/* Converts a bitvector to the "letter" format, which is easier to read. 
 * Note that the bit vector 0, is '0' in this system.
 * Note2 Doesn't appear to be used anywhere in the code, have commented it out
 * for now.
void bitString(u_int theBits, char *workStr)
{
    char *theChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    u_int bitMask = 1;
    int i, j = 0;

    if (theBits == 0) { // we represent 0, as well, 0
	workStr[0] = '0';
	j++;
    }
    else {
	i = 0;
	// concatenate the letters to the string until we get the whole bit string
	while (i < strlen(theChars)) {
	    if (bitMask & theBits) {
		workStr[j] = theChars[i];
		j++;
	    }
	    i++;
	    bitMask <<= 1;
        }
    }
    workStr[j] = '\0'; // terminate string
}
*/

/* -------------------------------------------------------------------------- */
/* OLC table display functions. Vex.                                          */
/* -------------------------------------------------------------------------- */

/* Similiar to olcTable, cept only for weapons. */
void attackTable(DescriptorData *d)
{
	int i, cnt;

        sprintf(buf, "[H[J");
	for (i = 0, cnt = 0; i < NUM_WEAPON_TYPES; i++) {
	    cnt++;
            sprintf(((buf) + strlen(buf)), "&06[%3d]%20s&00%s", i, 
	            attack_hit_text[i].singular, (cnt % 2 ? "  &08|&00  " : "\r\n"));
	}
	sprintf(((buf) + strlen(buf)), "\r\n");
	page_string(d, buf, 1);
	return;
}

/* Similiar to olcTable, specifcally to show how strength and weight relate. */
void weightTable(DescriptorData *d)
{
#define NUM_STRENGTH_STRINGS 30
	int i, cnt;

        sprintf(buf, "[H[J");
	sprintf(((buf) + strlen(buf)), "&02Wield  Carry              Strength  &08|&00&02  Wield  Carry              Strength\r\n");

	for (i = 1, cnt = 0; i <= NUM_STRENGTH_STRINGS; i++) {
	    cnt++;
            sprintf(((buf) + strlen(buf)), "&06[%4d] [%5d]%20s&00%s", str_app[i].wield_w, str_app[i].carry_w,
	            strString[i], (cnt % 2 ? "  &08|&00  " : "\r\n"));
	}
	sprintf(((buf) + strlen(buf)), "\r\n");
	page_string(d, buf, 1);
	return;
}


void
intTable( DescriptorData *d,
          char *names[],
          int num_names,
          const char *intName,
          const int current )
{
  int i;

  sprintf(buf, "\r\n"); /* Required to ensure buf in right state. */

  if( d->olc->tableDisp ) /* Table is needed should be displayed only once. */
    sprintf(buf, "[H[J"); /* Send the cursor home and clear screen */
  else
    sprintf(buf, "[0;0H");  /* Send the cursor HOME */
    
  for( i = 0; i < num_names; i++ )
  {
    sprintf(((buf) + strlen(buf)),
           "[&06%3d&00] %3s%20s&00%s", i,
           (i==current ? "&10" : "&06"),
            names[i], (i % 2 == 0 ? "  &08|&00  " : "\r\n"));
  }
  sprintf(((buf) + strlen(buf)), "\r\n[0J"); /* Clear to end of screen */

  /* Have to make prompt here or it will be screwed once we go over one page. */
  sprintf(((buf) + strlen(buf)), "Enter new %s : ", intName);
  page_string(d, buf, 1);
  return;
}


void
bitTable(DescriptorData *d,
         char *names[], 
         const int num_names,
	 const char *bitName,
	 const u_int current)
{
    int i, cnt;

    sprintf(buf, "\r\n"); /* required to ensure buf in right state. */

    if ( d->olc->tableDisp) /* Table is needed should be displayed only once. */
        sprintf(buf, "[H[J"); /* Send the cursor home and clear screen */
    else
        sprintf(buf, "[0;0H");  /* Send the cursor HOME */
    
    for (i = 0, cnt = 0; i < num_names; i++) {
        cnt++;
        if (i >= NUM_LETTERS) { /* We'll need to figure what else to use for chars. */
            mudlog(NRM, LVL_IMMORT, TRUE, "HOUSTON WE HAVE A PROBLEM! Too many bit types[%d] passed to bitTable -> Need bigger alphabet?", num_names);
            sendChar(d->character, "&22Internal bitTable error!&00\r\n");
            return;
        }
        sprintf(((buf) + strlen(buf)), "[%3s%3c&00] %3s%20s&00%s", 
                (BIT_IS_SET(i,current) ? "&10":"&06"), integer_to_letter(i),
                (BIT_IS_SET(i,current) ? "&10":"&06"), names[i], (cnt % 2 ? "  &08|&00  " : "\r\n"));
    }
    sprintf(((buf) + strlen(buf)), "\r\n[0J"); /* Clear to end of screen */

    /* Have to make prompt here or it will be screwed once we go over one page. */
    sprintbit(current, names, buf2);
    sprintf(((buf) + strlen(buf)),
             "Current %s flags: &03%s&00\r\n", bitName, buf2);
    sprintf(((buf) + strlen(buf)),
             "Enter %s flags to toggle(0 clears all): ", bitName);
    page_string(d, buf, 1);
    return;
}

/* The function that prints the tables for most of the olc menu functions.
** Arguments are: d - descriptor table is being shown too.
**		  names - the constant describing the field, e.g. room_bits
**		  num_names - the number of names e.g. NUM_ROOM_FLAGS
**		  pbit - if this is true, the function will print letters,
**                       otherwise it will print numbers when labelling the
**                       *names[].
*/
void
olcTable(DescriptorData *d, char *names[], const int num_names, bool pbit)
{
    int i, cnt;
    char buf[MAX_STRING_LENGTH];

    sprintf(buf, "\r\n");
    if (d->olc->tableDisp) {
        sprintf(buf, "[H[J");
	for (i = 0, cnt = 0; i < num_names; i++) {
	    cnt++;
	    if (pbit && (i >= NUM_LETTERS)) { /* We'll need to figure what else to use for chars. */
		mudlog(NRM, LVL_IMMORT, TRUE, "HOUSTON WE HAVE A PROBLEM! Too many bit types[%d] passed to olcTable -> Need bigger alphabet?", num_names);
		sendChar(d->character, "&22Internal olcTable error!&00\r\n");
		return;
	    }
	    if (pbit) {
	        sprintf(((buf) + strlen(buf)), "[&06%3c&00] &06%20s&00%s", 
	                integer_to_letter(i), names[i], (cnt % 2 ? "  &08|&00  " : "\r\n"));
	    }
	    else
	        sprintf(((buf) + strlen(buf)), "[&06%3d&00] &06%20s&00%s", 
	                i, names[i], (cnt % 2 ? "  &08|&00  " : "\r\n"));
	}
	sprintf(((buf) + strlen(buf)), "\r\n");
    }
    /* Have to make prompt here or it will be screwed once we go over one page. */
    sprintf(((buf) + strlen(buf)), "Enter whateva: ");
    page_string(d, buf, 1);
    return;
}

/* End Vex utils. */
