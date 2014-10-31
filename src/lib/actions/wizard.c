
/* ************************************************************************
*   File: wizard.c                                      Part of CircleMUD *
*  Usage: Player-level god commands and other goodies                     *
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
#include "actions/interpreter.h"
#include "actions/act.clan.h"
#include "specials/seek.h"
#include "general/handler.h"
#include "general/class.h"
#include "magic/spells.h"
#include "specials/house.h"
#include "general/color.h"
#include "olc/olc.h"
#include "actions/fight.h"
#include "util/weather.h"
#include "olc/oedit.h"
#include "olc/medit.h"
#include "olc/qedit.h"
#include "actions/act.h"          /* ACMDs located within the act*.c files */
#include "specials/combspec.h"
#include "specials/special.h"
#include "actions/outlaw.h"
#include "actions/quest.h"
#include "scripts/dg_scripts.h"
#include "specials/flag_game.h"
#include "actions/ban.h"
#include "magic/sing.h"
#include "general/objsave.h"
#include "specials/mobact.h"
#include "magic/magic.h"
#include "specials/muckle.h"
#include "magic/fishing.h"
#include "actions/auction.h"
#include "general/structs.h"

ACMD( do_dump )
{

    int    i;
    int    bytesOnLine = 1;
    FILE  *fl;
    char   dump[132];
    char   dumpStr[132];
    char   filename[32];
    struct zone_data *zptr;
    unsigned char  *ptr;

    if( !*argument ){
        sendChar( ch, "Usage: dump &08<filename>&00\r\n" );
        return;
    }

    one_argument(argument, filename);

    if( !(fl = fopen(filename, "w")) ){
        sendChar( ch, "Could not open the file.  Sorry.\r\n" );
        return;
    }
/*
** Digger - dump info here
*/
    fprintf( fl, "------Zone info --------\r\n" );
    for( i = 0, zptr = zone_table; i <= top_of_zone_table; i++, zptr++ )
        fprintf( fl, "%3d %-30.30s Kills: %3d; Age: %3d; Reset: %3d (%1d); Top: %5d\r\n",
                     zptr->number,     zptr->name,
                     zptr->kill_count, zptr->age,
                     zptr->lifespan,   zptr->reset_mode,
                     zptr->top);

    fprintf( fl, "----- Raw Msg Dump -----\r\n" );
    for( i = 0, ptr = (char *)&fight_messages[0].msg->die_msg; i < 1024; i++, ptr++ ){

        if( bytesOnLine == 1 ){
            strcpy( dump,    "[ "     );
            strcpy( dumpStr, "] =>  " );
        }

        sprintf( dump, "%s%02X ", dump, *ptr );
        if( bytesOnLine == 8 ) sprintf( dump, "%s: ", dump );

        sprintf( dumpStr, "%s%c", dumpStr, ((*ptr < ' ' || *ptr > '~') ? '.': *ptr ));

        if( bytesOnLine == 16 ){
            bytesOnLine = 0;
            fprintf( fl, "%s%s\r\n", dump, dumpStr );
        }
        ptr++;
        bytesOnLine++;
    }
/*
** Dump is done - close the file.
*/
    fclose(fl);
}


ACMD(do_echo)
{
    if( IS_NPC(ch) ) return;

    if (IS_SET_AR(PLR_FLAGS(ch), PLR_NOCOMM)) {
        send_to_char("You are unable to do that.\r\n", ch);
        return;
    }

    skip_spaces( &argument );

    if (!*argument)
        sendChar( ch, "Yes.. but what?\r\n" );

    else {
        if( subcmd == SCMD_EMOTE )
            sprintf( buf, "$n %s&00", argument );
        else
            strcpy( buf, argument );

        if( !PLR_FLAGGED( ch, PLR_SHUNNED ))
            act( buf, FALSE, ch, 0, 0, TO_ROOM );

        if( PRF_FLAGGED(ch, PRF_NOREPEAT) )
            sendChar( ch, CONFIG_OK );
        else
            act( buf, FALSE, ch, 0, 0, TO_CHAR );
    }
}


ACMD(do_send)
{
  CharData *vict;

  half_chop(argument, arg, buf);

  if (!*arg) {
    send_to_char("Send what to who?\r\n", ch);
    return;
  }
  if (!(vict = get_char_vis(ch, arg, 1))) {
    sendChar( ch, CONFIG_NOPERSON );
    return;
  }
  send_to_char(buf, vict);
  send_to_char("\r\n", vict);
  if (PRF_FLAGGED(ch, PRF_NOREPEAT))
    send_to_char("Sent.\r\n", ch);
  else {
    sprintf(buf2, "You send '%s' to %s.\r\n", buf, GET_NAME(vict));
    send_to_char(buf2, ch);
  }
}



/*
** take a string, and return an rnum.. used for goto, at, etc.  -je 4/6/93
*/
sh_int
find_target_room( CharData * ch, char *rawroomstr )
{
  int tmp;
  sh_int location;
  CharData *target_mob;
  ObjData *target_obj;
  char roomstr[MAX_INPUT_LENGTH];

  one_argument(rawroomstr, roomstr);

  if( !*roomstr )
  {
    sendChar( ch, "You must supply a room number or name.\r\n" );
    return NOWHERE;
  }

  if( isdigit(*roomstr) && !strchr(roomstr, '.') )
  {
    tmp = atoi(roomstr);
    if( (location = real_room(tmp)) < 0 )
    {
      sendChar( ch, "No room exists with that number.\r\n" );
      return NOWHERE;
    }
  }

  else if( (target_mob = get_char_vis(ch, roomstr, 0)) )
  {
    location = target_mob->in_room;
  }

  else if( (target_obj = get_obj_vis(ch, roomstr)) )
  {
    if( target_obj->in_room != NOWHERE )
      location = target_obj->in_room;
    else
    {
      sendChar( ch, "That object is not available.\r\n" );
      return NOWHERE;
    }
  }

  else
  {
    sendChar( ch, "No such creature or object around.\r\n" );
    return NOWHERE;
  }

  /* a location has been found -- if you're < GOD, check restrictions. */
  if( GET_LEVEL(ch) < LVL_HERO )
  {
    if( ROOM_FLAGGED(location, ROOM_GODROOM) )
    {
      sendChar( ch, "You are not godly enough to use that room!\r\n" );
      return NOWHERE;
    }

/*
** Removed 12/11/97 - Feel free to add it back - Kaidon
**
*/
# ifdef PRIVATE_ROOMS
    if( ROOM_FLAGGED(location, ROOM_PRIVATE) &&
	world[location].people && world[location].people->next_in_room )
    {
      sendChar( "There's a private conversation going on in that room.\r\n", ch );
      return NOWHERE;
    }

    if( ROOM_FLAGGED(location, ROOM_HOUSE) &&
	!House_can_enter(ch, world[location].number))
    {
      send_to_char("That's private property -- no trespassing!\r\n", ch);
      return NOWHERE;
    }
# endif
  }
  return location;
}


ACMD(do_affected)
{
    CharData *theAffected = character_list;
    int               infectedPC  = 0;
    int               infectedMOB = 0;

  if (IS_SET_AR(PLR_FLAGS(ch), PLR_RETIRED)) {
      send_to_char("Relax and enjoy being retired.\r\n", ch);
      return;
  }

    while( theAffected != NULL ){

        if( IS_AFFECTED( theAffected, AFF_PLAGUE )){
            sendChar( ch, " Plagued: %-30s   Room: %5d\r\n",
                          GET_NAME(theAffected),
                       world[theAffected->in_room].number );
            if( IS_MOB( theAffected ) )
                infectedMOB += 1;
            else
                infectedPC  += 1;
        }
        theAffected = theAffected->next;
    }
    sendChar( ch, "--- Totals (PC/MOB) = [%d/%d] --- Plagues are currently %s.\r\n",
                  infectedPC, infectedMOB, 
                  ( CONFIG_PLAGUE_IS_CONTAGIOUS ? "CONTAGIOUS" : "noncontagious" ));
}

void do_at_part(CharData *ch, char *command, int location)
{
  int original_loc;

  /* a location has been found. */
  original_loc = ch->in_room;
  char_from_room(ch);
  char_to_room(ch, location);
  command_interpreter(ch, command);

  /* check if the char is still there */
  if (ch->in_room == location) {
    char_from_room(ch);
    char_to_room(ch, original_loc);
  }
}

ACMD(do_at)
{
  char command[MAX_INPUT_LENGTH];
  int location;

  if (IS_NPC(ch)) return;

  half_chop(argument, buf, command);
  if (!*buf) {
    sendChar( ch, "You must supply a room number, a name, or '*'.\r\n" );
    return;
  }

  if (!*command) {
    sendChar( ch, "What do you want to do there?\r\n" );
    return;
  }

  /* '*' means, for every mortal, replacing '$p' with mortal name */
  if (!strcmp(buf, "*") || !strcmp(buf, "all")) {
    CharData *vict;
    char newcmd[MAX_INPUT_LENGTH];
    char *warned = newcmd;
    for (vict = character_list; vict; vict = vict->next) {
        if (CAN_SEE(ch, vict) && GET_LEVEL(vict) < LVL_IMMORT
                && !IS_NPC(vict) && vict->in_room >= 0) {
            strcpy(buf, command);
            replace_str(&warned, "$p", GET_NAME(vict), 1, MAX_INPUT_LENGTH);
            do_at_part(ch, newcmd, vict->in_room);
        }
    }
    return;
  } else if ((location = find_target_room(ch, buf)) >= 0) {
    do_at_part(ch, command, location);
  } else {
    sendChar(ch, "Where?  What?  Who?\r\n");
  }
}


ACMD(do_goto)
{
  sh_int location;

  if (IS_NPC(ch))
    return;

  if ((location = find_target_room(ch, argument)) < 0)
    return;

  if (POOFOUT(ch))
    sprintf(buf, "%s", POOFOUT(ch));
  else
    strcpy(buf, "$n disappears in a puff of smoke.");

  act(buf, TRUE, ch, 0, 0, TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, location);

  if (POOFIN(ch))
    sprintf(buf, "%s", POOFIN(ch));
  else
    strcpy(buf, "$n appears with an ear-splitting bang.");

  act(buf, TRUE, ch, 0, 0, TO_ROOM);
  look_at_room(ch, 0);
}



ACMD(do_trans)
{
  DescriptorData *i;
  CharData *victim;

  if (IS_NPC(ch))
    return;

  one_argument(argument, buf);
  if (!*buf)
    send_to_char("Whom do you wish to transfer?\r\n", ch);
  else if (str_cmp("all", buf)) {
    if (!(victim = get_char_vis(ch, buf, 0)))
      sendChar( ch, CONFIG_NOPERSON );
    else if (victim == ch)
      send_to_char("That doesn't make much sense, does it?\r\n", ch);
    else {
      if ((GET_LEVEL(ch) < GET_LEVEL(victim)) && !IS_NPC(victim)) {
	send_to_char("Go transfer someone your own size.\r\n", ch);
	return;
      }
      act("$n disappears in a mushroom cloud.", FALSE, victim, 0, 0, TO_ROOM);
      char_from_room(victim);
      char_to_room(victim, ch->in_room);
      act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
      act("$n has transferred you!", FALSE, ch, 0, victim, TO_VICT);
      look_at_room(victim, 0);
      
      if(victim->mount) {
          act("$n disappears in a mushroom cloud.", FALSE, victim->mount, 0, 0, TO_ROOM);
          char_from_room(victim->mount);
          char_to_room(victim->mount, victim->in_room);
          act("$n arrives from a puff of smoke.", FALSE, victim->mount, 0, 0, TO_ROOM);
          act("$n has transferred you!", FALSE, ch, 0, victim->mount, TO_VICT);
      }
      else if(victim->rider) {
          act("$n disappears in a mushroom cloud.", FALSE, victim->rider, 0, 0, TO_ROOM);
          char_from_room(victim->rider);
          char_to_room(victim->rider, victim->in_room);
          act("$n arrives from a puff of smoke.", FALSE, victim->rider, 0, 0, TO_ROOM);
          act("$n has transferred you!", FALSE, ch, 0, victim->rider, TO_VICT);
      }
    }
  }          /* Trans All */
  else {
      if (GET_LEVEL(ch) < LVL_GRGOD) {
          send_to_char("I think not.\r\n", ch);
          return;
      }

      for (i = descriptor_list; i; i = i->next)
          if (!i->connected && i->character && i->character != ch) {
              victim = i->character;
              if (GET_LEVEL(victim) >= GET_LEVEL(ch))
                  continue;
              act("$n disappears in a mushroom cloud.", FALSE, victim, 0, 0, TO_ROOM);
              char_from_room(victim);
              char_to_room(victim, ch->in_room);
              act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
              act("$n has transferred you!", FALSE, ch, 0, victim, TO_VICT);
              look_at_room(victim, 0);

              if(victim->mount) {
                  act("$n disappears in a mushroom cloud.", FALSE, victim->mount, 0, 0, TO_ROOM);
                  char_from_room(victim->mount);
                  char_to_room(victim->mount, victim->in_room);
                  act("$n arrives from a puff of smoke.", FALSE, victim->mount, 0, 0, TO_ROOM);
                  act("$n has transferred you!", FALSE, ch, 0, victim->mount, TO_VICT);
              }  
              else if(victim->rider) {
                  act("$n disappears in a mushroom cloud.", FALSE, victim->rider, 0, 0, TO_ROOM);
                  char_from_room(victim->rider);
                  char_to_room(victim->rider, victim->in_room);
                  act("$n arrives from a puff of smoke.", FALSE, victim->rider, 0, 0, TO_ROOM);
                  act("$n has transferred you!", FALSE, ch, 0, victim->rider, TO_VICT);
              }
	  }
	  sendChar( ch, CONFIG_OK );
  }
}

ACMD(do_teleport)
{
  CharData *victim;
  sh_int target;

  if (IS_NPC(ch))
    return;

  two_arguments(argument, buf, buf2);

  if (!*buf)
    send_to_char("Whom do you wish to teleport?\r\n", ch);
  else if (!(victim = get_char_vis(ch, buf, 0)))
    sendChar( ch, CONFIG_NOPERSON );
  else if (victim == ch)
    send_to_char("Use 'goto' to teleport yourself.\r\n", ch);
  else if (GET_LEVEL(victim) >= GET_LEVEL(ch))
    send_to_char("Maybe you shouldn't do that.\r\n", ch);
  else if (!*buf2)
    send_to_char("Where do you wish to send this person?\r\n", ch);
  else if ((target = find_target_room(ch, buf2)) >= 0) {
    sendChar( ch, CONFIG_OK );
    act("$n disappears in a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
    char_from_room(victim);
    char_to_room(victim, target);
    act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
    act("$n has teleported you!", FALSE, ch, 0, (char *) victim, TO_VICT);
    look_at_room(victim, 0);
  }
}



ACMD(do_vnum)
{
  if (IS_NPC(ch)) {
    send_to_char("What would a monster do with a vnum?\r\n", ch);
    return;
  }

  two_arguments(argument, buf, buf2);

  if (!*buf || !*buf2 || (!is_abbrev(buf, "mob") && !is_abbrev(buf, "obj"))) {
    send_to_char("Usage: vnum { obj | mob } <name>\r\n", ch);
    return;
  }
  if (is_abbrev(buf, "mob"))
    if (!vnum_mobile(buf2, ch))
      send_to_char("No mobiles by that name.\r\n", ch);

  if (is_abbrev(buf, "obj"))
    if (!vnum_object(buf2, ch))
      send_to_char("No objects by that name.\r\n", ch);
}


  char *exit_bits[NUM_EXIT_BITS + 1] = {
    "DOOR", "CLOSED", "LOCKED", "PICKPROOF", "TOCLAN", "DOORBASHED",
    "\n"
  };


void do_stat_room(CharData * ch)
{
  struct extra_descr_data *desc;
  RoomData *rm = &world[ch->in_room];
  int i, found = 0;
  ObjData *j = 0;
  CharData *k = 0;

  sprintf(buf, "Room name: %s%s%s\r\n", CCCYN(ch, C_NRM), rm->name,
	  CCNRM(ch, C_NRM));
  send_to_char(buf, ch);

  sprinttype(rm->sector_type, sector_types, buf2, sizeof(buf2));
  sprintf(buf, "Zone: [%3d], VNum: [%s%5d%s], RNum: [%5d], Type: %s\r\n",
	  rm->zone, CCGRN(ch, C_NRM), rm->number, CCNRM(ch, C_NRM), ch->in_room, buf2);
  send_to_char(buf, ch);

//128  sprintbit((long) rm->room_flags, room_bits, buf2);
  sprintbitarray(rm->room_flags, room_bits, RF_ARRAY_MAX, buf2);

  specialName(rm->func, buf1);
  sprintf(buf, "SpecProc: %s, Flags: %s\r\n", buf1, buf2);
  send_to_char(buf, ch);

  send_to_char("Description:\r\n", ch);
  if (rm->description)
    send_to_char(rm->description, ch);
  else
    send_to_char("  None.\r\n", ch);

  if (rm->ex_description) {
    sprintf(buf, "Extra descs:%s", CCCYN(ch, C_NRM));
    for (desc = rm->ex_description; desc; desc = desc->next) {
      strcat(buf, " ");
      strcat(buf, desc->keyword);
    }
    strcat(buf, CCNRM(ch, C_NRM));
    send_to_char(strcat(buf, "\r\n"), ch);
  }
  sprintf(buf, "Chars present:%s", CCYEL(ch, C_NRM));
  for (found = 0, k = rm->people; k; k = k->next_in_room) {
    if (!CAN_SEE(ch, k))
      continue;
    sprintf(buf2, "%s %s(%s)", found++ ? "," : "", GET_NAME(k),
	    (!IS_NPC(k) ? "PC" : (!IS_MOB(k) ? "NPC" : "MOB")));
    strcat(buf, buf2);
    if (strlen(buf) >= 62) {
      if (k->next_in_room)
	send_to_char(strcat(buf, ",\r\n"), ch);
      else
	send_to_char(strcat(buf, "\r\n"), ch);
      *buf = found = 0;
    }
  }

  if (*buf)
    send_to_char(strcat(buf, "\r\n"), ch);
  send_to_char(CCNRM(ch, C_NRM), ch);

  if (rm->contents) {
    sprintf(buf, "Contents:%s", CCGRN(ch, C_NRM));
    for (found = 0, j = rm->contents; j; j = j->next_content) {
      if (!CAN_SEE_OBJ(ch, j))
	continue;
      sprintf(buf2, "%s %s", found++ ? "," : "", j->short_description);
      strcat(buf, buf2);
      if (strlen(buf) >= 62) {
	if (j->next_content)
	  send_to_char(strcat(buf, ",\r\n"), ch);
	else
	  send_to_char(strcat(buf, "\r\n"), ch);
	*buf = found = 0;
      }
    }

    if (*buf)
      send_to_char(strcat(buf, "\r\n"), ch);
    send_to_char(CCNRM(ch, C_NRM), ch);
  }
  for (i = 0; i < NUM_OF_DIRS; i++) {
    if (rm->dir_option[i]) {
      if (rm->dir_option[i]->to_room == NOWHERE)
	sprintf(buf1, " %sNONE%s", CCCYN(ch, C_NRM), CCNRM(ch, C_NRM));
      else
	sprintf(buf1, "%s%5d%s", CCCYN(ch, C_NRM),
		world[rm->dir_option[i]->to_room].number, CCNRM(ch, C_NRM));
      sprintbit(rm->dir_option[i]->exit_info, exit_bits, buf2);
      sprintf(buf, "Exit %s%-5s%s:  To: [%s], Key: [%5d], Keywrd: %s, Type: %s\r\n ",
	      CCCYN(ch, C_NRM), dirs[i], CCNRM(ch, C_NRM), buf1, rm->dir_option[i]->key,
	   rm->dir_option[i]->keyword ? rm->dir_option[i]->keyword : "None",
	      buf2);
      send_to_char(buf, ch);
      if (rm->dir_option[i]->general_description)
	strcpy(buf, rm->dir_option[i]->general_description);
      else
	strcpy(buf, "  No exit description.\r\n");
      send_to_char(buf, ch);
      if (GET_TRAP(rm->dir_option[i]) > 0) {
        sprintf(buf, "  Exit trapped at level %d.\r\n",
            GET_TRAP(rm->dir_option[i]));
        send_to_char(buf, ch);
      }
    }
  }

  /* check the room for a script */
  do_sstat_room(ch);
}

void do_stat_object(CharData * ch, ObjData * j) {
  bool valid_value = TRUE;
  int i, virtual, found;
  ObjData *j2;
  struct extra_descr_data *desc;

  virtual = GET_OBJ_VNUM(j);
  sprintf(buf, "Name: '%s%s%s', Aliases: %s\r\n", CCYEL(ch, C_NRM),
	  ((j->short_description) ? j->short_description : "<None>"),
	  CCNRM(ch, C_NRM), j->name);
  send_to_char(buf, ch);
  sprinttype(GET_OBJ_TYPE(j), item_types, buf1, sizeof(buf1));
  sprintf(buf, "VNum: [%s%5d%s], RNum: [%5d], Type: %s\r\n",
   CCGRN(ch, C_NRM), virtual, CCNRM(ch, C_NRM), GET_OBJ_RNUM(j), buf1);
  send_to_char(buf, ch);

  /* Special procedures. */
  if (GET_OBJ_RNUM(j) >= 0) {
    combSpecName(obj_index[j->item_number].combatSpec, buf1);
    specialName(obj_index[j->item_number].func, buf2);
  }
  else {
    strcpy(buf1, "None");
    strcpy(buf2, "None");
  }
  sprintf(buf, "CombSpecProc: %s SpecProc: %s\r\n", buf1, buf2);
  send_to_char(buf, ch);

  sprintf(buf, "L-Des: %s\r\n", ((j->description) ? j->description : "None"));
  send_to_char(buf, ch);

  if (j->ex_description) {
    sprintf(buf, "Extra descs:%s", CCCYN(ch, C_NRM));
    for (desc = j->ex_description; desc; desc = desc->next) {
      strcat(buf, " ");
      strcat(buf, desc->keyword);
    }
    strcat(buf, CCNRM(ch, C_NRM));
    send_to_char(strcat(buf, "\r\n"), ch);
  }
  send_to_char("Can be worn on: ", ch);
//128  sprintbit(j->obj_flags.wear_flags, wear_bits, buf);
  sprintbitarray(j->obj_flags.wear_flags, wear_bits, TW_ARRAY_MAX, buf);
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  send_to_char("Set char bits : ", ch);
//128  sprintbit(j->obj_flags.bitvector, affected_bits, buf);
  sprintbitarray(j->obj_flags.bitvector, affected_bits, AF_ARRAY_MAX, buf);
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  send_to_char("Extra flags   : ", ch);
//128  sprintbit(GET_OBJ_EXTRA(j), extra_bits, buf);
  sprintbitarray(GET_OBJ_EXTRA(j), extra_bits, EF_ARRAY_MAX, buf);
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  sprintf(buf, "Weight: %d, Value: %d, Cost/day: %d, Timer: %d\r\n",
     GET_OBJ_WEIGHT(j), GET_OBJ_COST(j), GET_OBJ_RENT(j), GET_OBJ_TIMER(j));
  send_to_char(buf, ch);

  strcpy(buf, "In room: ");
  if (j->in_room == NOWHERE)
    strcat(buf, "Nowhere");
  else {
    sprintf(buf2, "%d", world[j->in_room].number);
    strcat(buf, buf2);
  }
  strcat(buf, ", In object: ");
  strcat(buf, j->in_obj ? j->in_obj->short_description : "None");
  strcat(buf, ", Carried by: ");
  strcat(buf, j->carried_by ? GET_NAME(j->carried_by) : "Nobody");
  strcat(buf, ", Worn by: ");
  strcat(buf, j->worn_by ? GET_NAME(j->worn_by) : "Nobody");
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  /* Check to make sure the values are valid. */
  for (i = 0; i < 4; i++)
    if (j->obj_flags.value[i] != objValueLimit(j->obj_flags.type_flag, i, j->obj_flags.value[i]))
	valid_value = FALSE;

  if (valid_value)
    oPrintValues(j, buf);
  else /* Could be risk of causing crash trying to show them, print integers. */
    sprintf(buf, "&08Values invalid!&00 True values: %d %d %d %d",
    j->obj_flags.value[0], j->obj_flags.value[1], j->obj_flags.value[2], j->obj_flags.value[3]);

  send_to_char(buf, ch); /* show em the values. */

  strcpy(buf, "\r\nEquipment Status: ");
  if (!j->carried_by)
    strcat(buf, "None");
  else {
    found = FALSE;
    for (i = 0; i < NUM_WEARS; i++) {
      if (j->carried_by->equipment[i] == j) {
	sprinttype(i, equipment_types, buf2, sizeof(buf2));
	strcat(buf, buf2);
	found = TRUE;
      }
    }
    if (!found)
      strcat(buf, "Inventory");
  }
  send_to_char(strcat(buf, "\r\n"), ch);

  if (j->contains) {
    sprintf(buf, "Contents:%s", CCGRN(ch, C_NRM));
    for (found = 0, j2 = j->contains; j2; j2 = j2->next_content) {
      sprintf(buf2, "%s %s", found++ ? "," : "", j2->short_description);
      strcat(buf, buf2);
      if (strlen(buf) >= 62) {
	if (j2->next_content)
	  send_to_char(strcat(buf, ",\r\n"), ch);
	else
	  send_to_char(strcat(buf, "\r\n"), ch);
	*buf = found = 0;
      }
    }

    if (*buf)
      send_to_char(strcat(buf, "\r\n"), ch);
    send_to_char(CCNRM(ch, C_NRM), ch);
  }
  found = 0;
  send_to_char("Affections:", ch);
  for (i = 0; i < MAX_OBJ_AFFECT; i++)
    if (j->affected[i].modifier) {
      sprinttype(j->affected[i].location, apply_types, buf2, sizeof(buf2));
      sprintf(buf, "%s %+d to %s", found++ ? "," : "",
	      j->affected[i].modifier, buf2);
      send_to_char(buf, ch);
    }
  if (!found)
    send_to_char(" None", ch);

  send_to_char("\r\n", ch);

  if (GET_TRAP(j)) {
    sprintf(buf, "Object is trapped at level %d.\r\n", GET_TRAP(j));
    send_to_char(buf, ch);
  }
  
  if(IS_OBJ_STAT(j, ITEM_TROPHY)) {
      sendChar(ch, "Expires on: %d%s day of the %d%s month, %d.\r\n",
              (GET_OBJ_TIMER(j)%TICKS_PER_MUD_MONTH)/TICKS_PER_MUD_DAY, 
              ndth((GET_OBJ_TIMER(j)%TICKS_PER_MUD_MONTH)/TICKS_PER_MUD_DAY ),
              (GET_OBJ_TIMER(j)%TICKS_PER_MUD_YEAR)/TICKS_PER_MUD_MONTH , 
              ndth((GET_OBJ_TIMER(j)%TICKS_PER_MUD_YEAR)/TICKS_PER_MUD_MONTH),
              GET_OBJ_TIMER(j)/TICKS_PER_MUD_YEAR);
  }

  /* check the object for a script */
  do_sstat_object(ch, j);
}

void do_stat_character(CharData * ch, CharData * k)
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char aspect[MAX_STRING_LENGTH];
  int i, i2, found = 0;
  ObjData *j;
  FollowType *fol;
  struct affected_type *aff;
  int real_ac;

  CharData *find_char(int n);

  real_ac = realAC(k);
    
  sprinttype(GET_SEX(k), genders, buf, sizeof(buf));
  sendChar(ch, "%s %s '%s'  IDNum: [%5ld], In room [%5d], Loadroom : [%5d]\r\n",
	  buf, (!IS_NPC(k) ? "PC" : (!IS_MOB(k) ? "NPC" : "MOB")),
	  GET_NAME(k), IS_NPC(k) ? GET_ID(k) : GET_IDNUM(k), GET_ROOM_VNUM(IN_ROOM(k)),
          IS_NPC(k) ? NOWHERE : GET_LOADROOM(k));

    if( IS_MOB(k) )
        sendChar( ch, "Keyword: %s, VNum: [&12%5d&00], RNum: [&12%5d&00]\r\n",
                       k->player.name, GET_MOB_VNUM(k), GET_MOB_RNUM(k));

    sendChar(ch, "Title: %s\r\n", k->player.title ? k->player.title : "<None>");

    sendChar(ch, "L-Des: %s", k->player.long_descr ? k->player.long_descr : "<None>\r\n");
    sendChar(ch, "D-Des: %s", k->player.description ? k->player.description : "<None>\r\n");

    if( IS_NPC(k) ){
        sendChar( ch, "Monster Class: ");
        /* Liam */
        sprinttype( k->player.class, pc_class_types, buf, sizeof(buf) );
    }
    else {
        sendChar( ch, "Class: " );
        sprinttype( k->player.class, pc_class_types, buf, sizeof(buf) );
    }
    
    sendChar( ch, buf );
    sendChar( ch, ", Lev: [&10%2d&00], XP: [&08%7d&00], Align: [&10%4d&00], QP: [&08%2d&00]\r\n",
                     GET_LEVEL(k), GET_EXP(k), GET_ALIGNMENT(k), GET_QP(k) );

    sendChar( ch, "Race: " );
    sprinttype(GET_RACE(k), race_types, buf, sizeof(buf));
    sendChar( ch, buf );
    sendChar( ch, " Subrace: %d", GET_SUBRACE(k));
    
    if(!IS_NPC(k)) {        
        const char *specs[12][3] = {{"none", "Arcanist", "Enchanter"}, //Mu
                        {"none", "Dark Priest", "Holy Priest"}, //Cl
                        {"none", "Trickster", "Brigand"}, //Th
                        {"none", "Defender", "Dragon Slayer"}, //Wa
                        {"none", "Naturalist", "Hunter"}, //Ra
                        {"none", "Bounty Hunter", "Butcher"}, //As
                        {"none", "Chi Warrior", "Ancient Dragon"}, //Sl
                        {"none", "Dragoon", "Knight Templar"}, //Kn
                        {"none", "Knight Errant", "Defiler"}, //Dk
                        {"none", "Prestidigitator", "Shade"}, //Sd
                        {"none", "Witch Doctor", "Revenant"}, //Nm
                        {"none", "none", "none"}};

        sendChar(ch, " Adv  [%2d] Spec [%12s]",
                GET_ADVANCE_LEVEL(k), specs[GET_CLASS(k)][GET_SPEC(k)]);    
    }
    
    sendChar(ch, "\r\n");

  if (!IS_NPC(k)) {
    strcpy(buf1, (char *) asctime(localtime(&(k->player.time.birth))));
    strcpy(buf2, (char *) asctime(localtime(&(k->player.time.logon))));
    buf1[10] = buf2[10] = '\0';

    sprintf(buf, "Created: [%d/%d/%d], Last Logon: [%d/%d/%d], Played [%dh %dm], Age [%d]\r\n",
	    localtime(&(k->player.time.birth))->tm_mon + 1, localtime(&(k->player.time.birth))->tm_mday,
            localtime(&(k->player.time.birth))->tm_year + 1900, localtime(&(k->player.time.logon))->tm_mon + 1,
            localtime(&(k->player.time.logon))->tm_mday, localtime(&(k->player.time.logon))->tm_year + 1900,
            k->player.time.played / 3600,
	    ((k->player.time.played / 3600) % 60), age(k).year);
    send_to_char(buf, ch);

    sprintf(buf, "Levels lost: [%d], Practices: [%d], Locker: [%d], Friend: [%d]",
            k->player.lostlevels, GET_PRACTICES(k), GET_LOCKER(k), GET_CLAN(k));
    /* Display OLC zone for immorts */
    if (GET_LEVEL(k) >= LVL_IMMORT)
        sprintf(buf, "%s, OLC[%d]", buf, GET_OLC_ZONE(k));
    send_to_char(buf, ch);
  }
  sprintf(buf, "\r\nStr: [%s%d/%d%s]  Int: [%s%d%s]  Wis: [%s%d%s]  "
	  "Dex: [%s%d%s]  Con: [%s%d%s]  Cha: [%s%d%s]\r\n",
	  CCCYN(ch, C_NRM), GET_STR(k), GET_ADD(k), CCNRM(ch, C_NRM),
	  CCCYN(ch, C_NRM), GET_INT(k), CCNRM(ch, C_NRM),
	  CCCYN(ch, C_NRM), GET_WIS(k), CCNRM(ch, C_NRM),
	  CCCYN(ch, C_NRM), GET_DEX(k), CCNRM(ch, C_NRM),
	  CCCYN(ch, C_NRM), GET_CON(k), CCNRM(ch, C_NRM),
	  CCCYN(ch, C_NRM), GET_CHA(k), CCNRM(ch, C_NRM));
  send_to_char(buf, ch);

  sprintf(buf, "Hit p.:[%s%d/%d+%d%s]  Mana p.:[%s%d/%d+%d%s]  Move p.:[%s%d/%d+%d%s]\r\n",
	  CCGRN(ch, C_NRM), GET_HIT(k), GET_MAX_HIT(k), hit_gain(k), CCNRM(ch, C_NRM),
	  CCGRN(ch, C_NRM), GET_MANA(k), GET_MAX_MANA(k), mana_gain(k), CCNRM(ch, C_NRM),
	  CCGRN(ch, C_NRM), GET_MOVE(k), GET_MAX_MOVE(k), move_gain(k), CCNRM(ch, C_NRM));
  send_to_char(buf, ch);

  sprintf(buf, "Coins: [%9d], Bank: [%9d] (Total: %d), Cooldown: [%2d, %2d, %2d, %2d]\r\n",
	  GET_GOLD(k), GET_BANK_GOLD(k), GET_GOLD(k) + GET_BANK_GOLD(k),
          COOLDOWN(k, 1), COOLDOWN(k, 2), COOLDOWN(k, 3), COOLDOWN(k, 4));
  send_to_char(buf, ch);

  sprintf(buf, "AC: [%d/10], Hitroll: [%2d], Damroll: [%2d], SpellDam: [%3d%%] Saving throws: [%d/%d/%d/%d/%d]\r\n",
	  real_ac, k->points.hitroll, GET_DAMROLL(k), 100 + spell_damage_gear(ch)/3, GET_SAVE(k, 0),
	  GET_SAVE(k, 1), GET_SAVE(k, 2), GET_SAVE(k, 3), GET_SAVE(k, 4));
  send_to_char(buf, ch);

  if(IS_NPC(k)) {
    sprintf(buf, "Skill Success: [%2d], Spell damage multiplier: [%3d]\r\n",
	  GET_MOB_SKILL_SUC(k), GET_MOB_SPELL_DAM(k));
    send_to_char(buf, ch);
  }

  sprinttype(GET_POS(k), position_types, buf2, sizeof(buf2));
  sprintf(buf, "Pos: %s, Fighting: %s", buf2,
	  (FIGHTING(k) ? GET_NAME(FIGHTING(k)) : "Nobody"));

  if (IS_NPC(k)) {
    strcat(buf, "\r\nAttack types: Primary: ");
    strcat(buf, attack_hit_text[k->mob_specials.attack_type].singular);
    if (k->mob_specials.attack_type2 >= 0)
    {
    	strcat(buf, " Secondary: ");
    	strcat(buf, attack_hit_text[k->mob_specials.attack_type2].singular);
    }
    if (k->mob_specials.attack_type3 >= 0)
    {
   	 strcat(buf, " Tertiary: ");
   	 strcat(buf, attack_hit_text[k->mob_specials.attack_type3].singular);
    }
  }
  if (k->desc) {
    sprinttype(k->desc->connected, connected_types, buf2, sizeof(buf2));
    strcat(buf, ", Connected: ");
    strcat(buf, buf2);
  }
  send_to_char(strcat(buf, "\r\n"), ch);

  /* Positions for NPC, idle for PCs */
  if (IS_NPC(k)) {
	strcpy(buf, "Default position: ");
	sprinttype((k->mob_specials.default_pos), position_types, buf2, sizeof(buf2));
	strcat(buf, buf2);
	strcat(buf, " Load position: ");
	sprinttype((k->mob_specials.load_pos), position_types, buf2, sizeof(buf2));
	strcat(buf, buf2);
	strcat(buf, "\r\n");
	send_to_char(buf, ch);
  }
  else {
	sprintf(buf2, "Idle Timer (in tics) [%d]\r\n", k->char_specials.timer);
	strcpy(buf, buf2);
	send_to_char(buf, ch);
        sprintf(buf2, "On quest: %s  Quest wait time: %ds\r\n",
            GET_QUEST(k) ? GET_QUEST(k)->name : "NONE",
            GET_QUEST_WAIT(k) ? GET_QUEST_WAIT(k) - time(NULL) : 0);
	strcpy(buf, buf2);
	send_to_char(buf, ch);
  }

  if (IS_NPC(k)) {
//128    sprintbit(MOB_FLAGS(k), action_bits, buf2);
    sprintbitarray(MOB_FLAGS(k), action_bits, PM_ARRAY_MAX, buf2);
    sprintf(buf, "NPC flags: %s%s%s\r\n", CCCYN(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
    send_to_char(buf, ch);
  } else {
//128    sprintbit(PLR_FLAGS(k), player_bits, buf2);
    sprintbitarray(PLR_FLAGS(k), player_bits, PM_ARRAY_MAX, buf2);
    sprintf(buf, "PLR: %s%s%s\r\n", CCCYN(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
    send_to_char(buf, ch);
//128    sprintbit(PRF_FLAGS(k), preference_bits, buf2);
    sprintbitarray(PRF_FLAGS(k), preference_bits, PR_ARRAY_MAX, buf2);
    sprintf(buf, "PRF: %s%s%s\r\n", CCGRN(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
    send_to_char(buf, ch);
  }

  if (IS_MOB(k)) {
    specialName(mob_index[GET_MOB_RNUM(k)].func, buf1);
    combSpecName(mob_index[GET_MOB_RNUM(k)].combatSpec, buf2);
    sprintf(buf, "Spec-Proc: %s, Combat-Spec: %s NPC Bare Hand Dam: %dd%d\r\n",
	    buf1, buf2,
	    k->mob_specials.damnodice, k->mob_specials.damsizedice);
    send_to_char(buf, ch);
  }
  sprintf(buf, "Carried: weight: %d, items: %d; ",
	  IS_CARRYING_W(k), IS_CARRYING_N(k));

  for (i = 0, j = k->carrying; j; j = j->next_content, i++);
  sprintf(buf, "%sItems in: inventory: %d, ", buf, i);

  for (i = 0, i2 = 0; i < NUM_WEARS; i++)
    if (k->equipment[i])
      i2++;
  sprintf(buf2, "eq: %d\r\n", i2);
  strcat(buf, buf2);
  send_to_char(buf, ch);

  sprintf(buf, "Hunger: %d, Thirst: %d, Drunk: %d\r\n",
	  GET_COND(k, HUNGER), GET_COND(k, THIRST), GET_COND(k, DRUNK));
  send_to_char(buf, ch);

  sprintf(buf, "Master is: %s, Followers are:",
	  ((k->master) ? GET_NAME(k->master) : "<none>"));

  for (fol = k->followers; fol; fol = fol->next) {
    sprintf(buf2, "%s %s", found++ ? "," : "", PERS(fol->follower, ch));
    strcat(buf, buf2);
    if (strlen(buf) >= 62) {
      if (fol->next)
	send_to_char(strcat(buf, ",\r\n"), ch);
      else
	send_to_char(strcat(buf, "\r\n"), ch);
      *buf = found = 0;
    }
  }

  if (*buf)
    send_to_char(strcat(buf, "\r\n"), ch);

  /* Showing the bitvector */
//128  sprintbit(AFF_FLAGS(k), affected_bits, buf2);
  sprintbitarray(AFF_FLAGS(k), affected_bits, AF_ARRAY_MAX, buf2);
  sprintf(buf, "AFF: %s%s%s\r\n", CCYEL(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
  send_to_char(buf, ch);

  if(GET_ASPECT(k)) {
      sprintf(aspect, aspects[GET_ASPECT(k)]);
      sendChar(ch, "Aspect [%10s]\r\n", CAP(aspect));
  }

  /* Routine to show what spells a char is affected by */
  if (k->affected) {
    for (aff = k->affected; aff; aff = aff->next) {
        *buf2 = '\0';
        
        // Changes for spell duration into seconds - Craklyn
        if (aff->duration == -1) {
            sprintf(buf, "SPL: (  On ) lvl%02d %s%-21s%s ",
                    aff->level, CCCYN(ch, C_NRM), spells[aff->type], CCNRM(ch, C_NRM));
        }
        else if (aff->duration / 72) {
            sprintf(buf, "SPL: (%3dhr) lvl%02d %s%-21s%s ", (aff->duration)/72 + 1,
                    aff->level, CCCYN(ch, C_NRM), spells[aff->type], CCNRM(ch, C_NRM));
        }
        else {
            sprintf(buf, "SPL: (%3dmin) lvl%02d %s%-21s%s ", (aff->duration)*60/72 + 1,
                    aff->level, CCCYN(ch, C_NRM), spells[aff->type], CCNRM(ch, C_NRM));
        }
        
        if (aff->modifier) {
            sprintf(buf2, "%+d to %s", aff->modifier, apply_types[(int) aff->location]);
            strcat(buf, buf2);
        }
        if (aff->bitvector) {
            if (*buf2)
                strcat(buf, ", sets ");
            else
                strcat(buf, "sets ");
//128	sprintbit(aff->bitvector, affected_bits, buf2);
            strcpy(buf2, affected_bits[aff->bitvector]);
            strcat(buf, buf2);
        }
        send_to_char(strcat(buf, "\r\n"), ch);
    }
  }
  /* check mobiles for a script */
  if (IS_NPC(k)) {
    do_sstat_character(ch, k);
    if (SCRIPT_MEM(k)) {
      struct script_memory *mem = SCRIPT_MEM(k);
      send_to_char("Script memory:\r\n  Remember             Command\r\n", ch);
      while (mem) {
        struct char_data *mc = find_char(mem->id);
        if (!mc) send_to_char("  ** Corrupted!\r\n", ch);
        else {
          if (mem->cmd) sprintf(buf,"  %-20.20s%s\r\n",GET_NAME(mc),mem->cmd);
          else sprintf(buf,"  %-20.20s <default>\r\n",GET_NAME(mc));
          send_to_char(buf, ch);
        }
        mem = mem->next;
      }
    }
  } else {
    /* this is a PC, display their global variables */
    if (k->script && k->script->global_vars) {
      struct trig_var_data *tv;
      char name[MAX_INPUT_LENGTH];
      void find_uid_name(char *uid, char *name);

      send_to_char("Global Variables:\r\n", ch);

      /* currently, variable context for players is always 0, so it is */
      /* not displayed here. in the future, this might change */
      for (tv = k->script->global_vars; tv; tv = tv->next) {
        if (*(tv->value) == UID_CHAR) {
          find_uid_name(tv->value, name);
          sprintf(buf, "    %10s:  [UID]: %s\r\n", tv->name, name);
        } else
          sprintf(buf, "    %10s:  %s\r\n", tv->name, tv->value);
        send_to_char(buf, ch);
      }
    }
  }
}

void do_stat_quest(CharData *ch, QuestData *q)
{
  int i, j;
  char *task_types[] = {
    "  None", "  Fetch object #%d", "  Kill mob #%d", "  Enter room #%d",
  };
  char *reward_types[] = {
    "None", "Give %d QP", "Give %d experience", "Give object #%d",
    "Give %d coins", "Give %d practice sessions", "Cast spell %s",
    "Remort to race %s",
  };

  sprintf(buf, "Quest #%d: %s\r\n", q->virtual, q->name);
  send_to_char(buf, ch);
  sprintf(buf, "Questmaster's speech:\r\n%s%s%sQuest info:\r\n%s%s%s",
      CCBLU(ch, C_NRM), q->speech, CCNRM(ch, C_NRM),
      CCBLU(ch, C_NRM), q->description, CCNRM(ch, C_NRM));
  send_to_char(buf, ch);
  sprintf(buf, "Levels %d to %d, limit of %d ticks\r\n",
      q->minlvl, q->maxlvl, q->timelimit);
  send_to_char(buf, ch);
  sprintf(buf, "Wait %d minutes on completion, and %d minutes on refusal\r\n",
      q->waitcomplete, q->waitrefuse);
  send_to_char(buf, ch);
  sprintbit(q->flags, questflag_bits, buf2);
  sprintf(buf, "Flags: %s%s%s\r\n", CCCYN(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
  send_to_char(buf, ch);
  j = sprintf(buf, "Prerequisites: %s", CCYEL(ch, C_NRM));
  for (i = 0; i < MAX_QST_PREREQ; i++)
    if (q->prereqs[i]) j += sprintf(buf+j, "#%d ", q->prereqs[i]);
  sprintf(buf + j, "%s\r\n", CCNRM(ch, C_NRM));
  send_to_char(buf, ch);
  j = sprintf(buf, "Givers: %s", CCYEL(ch, C_NRM));
  for (i = 0; i < MAX_QST_GIVERS; i++)
    if (q->givers[i]) j += sprintf(buf+j, "#%d ", q->givers[i]);
  sprintf(buf + j, "%s\r\nTasks:\r\n%s", CCNRM(ch, C_NRM), CCGRN(ch, C_NRM));
  send_to_char(buf, ch);
  for (i = 0; i < MAX_QST_TASKS; i++)
    if (q->tasks[i].type != QST_TASK_NONE) {
      sprintf(buf, task_types[q->tasks[i].type], q->tasks[i].identifier);
      send_to_char(buf, ch); send_to_char("\r\n", ch);
    }
  sprintf(buf, "%sRewards:\r\n", CCNRM(ch, C_NRM));
  send_to_char(buf, ch);
  for (i = 0; i < MAX_QST_REWARDS; i++)
    if (q->rewards[i].type != QST_REWARD_NONE) {
      if (q->rewards[i].type == QST_REWARD_SPELL)
        sprintf(buf2, reward_types[q->rewards[i].type],
            spells[q->rewards[i].amount]);
      else if (q->rewards[i].type == QST_REWARD_REMORT)
          sprintf(buf2, reward_types[q->rewards[i].type],
                  qst_remort_races[q->rewards[i].amount]);
      else
        sprintf(buf2, reward_types[q->rewards[i].type], q->rewards[i].amount);
      sprintf(buf, "  %s%s and act:\r\n%s%s\r\n",
          CCGRN(ch, C_NRM), buf2, CCNRM(ch, C_NRM), q->rewards[i].speech);
      send_to_char(buf, ch);
    }
}

ACMD(do_reboot)
{
  extern char reboot_reason[80];
  char *the_reason;
  int ticks_to_reboot = -1;

  if (IS_SET_AR(PLR_FLAGS(ch), PLR_RETIRED)) {
      send_to_char("Relax and enjoy being retired.\r\n", ch);
      return;
  }

  if(auction.ticks != -1)  // -1 is AUC_NONE, i.e. no auction in place
  {
      send_to_char("Please wait for the auction to finish before rebooting.  (Or kill the auction with auct -kill.)\r\n", ch);
      return;
  }

  if (IS_NPC(ch)) return;

  if( subcmd != SCMD_REBOOT ){
    send_to_char( "Usage: reboot <ticks> <message>\r\n", ch );
    send_to_char( "   Ex: reboot 15 Quick reboot to reset the mud.\r\n", ch );
    return;
  }
  the_reason = one_argument(argument, arg);

  ticks_to_reboot = atoi( arg );

  if( ticks_to_reboot > 0 ){
      strcpy( reboot_reason, the_reason );
      oracle_counter = ticks_to_reboot + 1;
      circle_reboot  = 1;
      oracle_reboot  = 1;
      mudlog( BRF, LVL_IMMORT, TRUE, "(GC) Reboot issued by %s in %d ticks.", GET_NAME(ch), ticks_to_reboot );
      mudlog( BRF, LVL_IMMORT, TRUE, "(GC) Reason: %s", the_reason );
  }

  else if( ticks_to_reboot == 0 ){
      oracle_counter = 0;
      sendChar( ch, "System reboot has been &08cancelled.&00" );
      mudlog( BRF, LVL_IMMORT, TRUE, "(GC) Reboot cancelled by %s.", GET_NAME(ch) );
  }

  if( oracle_counter > 1 ){
      sprintf( buf, "Reboot in %d ticks - [%s]\r\n", oracle_counter, reboot_reason );
      send_to_char( buf, ch );
  }
}

/* If the function is not taking any parameters than use void */
void poll_clear(void)
{
	DescriptorData *dsc;
	extern char poll_question[255], poll_answer1[255], poll_answer2[255],
	  poll_answer3[255], poll_answer4[255], poll_answer5[255];

	for (dsc = descriptor_list; dsc; dsc = dsc->next) {
		if (dsc->character && dsc->character && !IS_NPC(dsc->character)) 
			GET_VOTE(dsc->character) = 0;
	}

	poll_question[0] = '\0';
	poll_answer1[0] = '\0';
	poll_answer2[0] = '\0';
	poll_answer3[0] = '\0';
	poll_answer4[0] = '\0';
	poll_answer5[0] = '\0';
}

int poll_ready() {
    extern char poll_question[255], poll_answer1[255];


    if (poll_question[0] != '\0' && poll_answer1[0] != '\0')
        return TRUE;
    else
        return FALSE;
}


ACMD(do_poll)
{
  extern int poll_running;
  extern char poll_question[255], poll_answer1[255], poll_answer2[255],
	  poll_answer3[255], poll_answer4[255], poll_answer5[255];
  char scmd[100];

  if (IS_NPC(ch)) return;
  if (GET_LEVEL(ch) < LVL_IMMORT) return;

  one_argument(argument, scmd);

  if (is_abbrev(scmd, "clear")) {
	  poll_clear();
	  sendChar(ch, "The poll has been cleared.\r\n");
	  return;
  }
  
  if(scmd[0] == '\0') {
	  if( poll_ready() )
	  {
		  mudlog( BRF, LVL_IMMORT, TRUE, "(POLL) Poll began by %s.", GET_NAME(ch));
		  mudlog( BRF, LVL_IMMORT, TRUE, "(POLL) Question: %s", poll_question);
		  poll_running = 4;
	  }
	  else
		  sendChar(ch, "The poll doesn't have enough info to go.\r\n");
	  return;
  }
  
  
  if( strlen(argument) > 250 )
  {
    sendChar(ch, "Your poll was too long.  It has been ignored.\r\n");
  }


  if (  poll_question[0] == '\0') {
	  send_to_char("Poll question set.\r\n", ch );
	  strcpy( poll_question, argument); 
  }
  else if ( poll_answer1[0] == '\0') {
	  send_to_char( "Poll answer #1 set.\r\n", ch );
	  strcpy( poll_answer1, argument); 
  }
  else if ( poll_answer2[0] == '\0') {
	  send_to_char("Poll answer #2 set.\r\n", ch );
	  strcpy( poll_answer2, argument);
  } 
  else if ( poll_answer3[0] == '\0') {
	  send_to_char("Poll answer #3 set.\r\n", ch );
	  strcpy( poll_answer3, argument); 
  }
  else if ( poll_answer4[0] == '\0') {	
	  strcpy( poll_answer4, argument);
      send_to_char("Poll answer #4 set.\r\n", ch ); 
  }
  else if ( poll_answer5[0] == '\0') {
	  send_to_char("Poll answer #5 set.\r\n", ch );
	  strcpy( poll_answer5, argument); 
  }
  else sendChar(ch, "The poll is full.  You must either run it or clear it.\r\n");

}


ACMD(do_shutdown)
{
    
  if (IS_SET_AR(PLR_FLAGS(ch), PLR_RETIRED)) {
      send_to_char("Relax and enjoy being retired.\r\n", ch);
      return;
  }

  if (IS_NPC(ch)) return;

  if (subcmd != SCMD_SHUTDOWN) {
    send_to_char("If you want to shut something down, say so!\r\n", ch);
    return;
  }
  one_argument(argument, arg);

  if( GET_LEVEL(ch) < LVL_IMPL && !mini_mud){
    sendChar( ch, "You may only shutdown in mini-mode.\n" );
    return;
  }

  if (!*arg) {
    mlog("(GC) Shutdown by %s.", GET_NAME(ch));
    send_to_all("Shutting down.\r\n");
    circle_shutdown = 1;
  } else if (!str_cmp(arg, "reboot")) {
    mlog("(GC) Reboot by %s.", GET_NAME(ch));
    send_to_all("Rebooting.. come back in a minute or two.\r\n");
    touch("../.fastboot");
    circle_shutdown = circle_reboot = 1;
  } else if (!str_cmp(arg, "halt")) {
    mlog("(GC) Shutdown by %s.", GET_NAME(ch));
    send_to_all("Shutting down for maintenance.\r\n");
    touch("../.killscript");
    circle_shutdown = 1;
  } else if (!str_cmp(arg, "pause")) {
    mlog("(GC) Shutdown by %s.", GET_NAME(ch));
    send_to_all("Shutting down for maintenance.\r\n");
    touch("../pause");
    circle_shutdown = 1;
  } else
    send_to_char("Unknown shutdown option.\r\n", ch);
}


void stop_snooping(CharData * ch)
{
  if (!ch->desc->snooping)
    send_to_char("You aren't snooping anyone.\r\n", ch);
  else {
    send_to_char("You stop snooping.\r\n", ch);
    ch->desc->snooping->snoop_by = NULL;
    ch->desc->snooping = NULL;
  }
}


ACMD(do_snoop)
{
  CharData *victim, *tch;

  if (IS_SET_AR(PLR_FLAGS(ch), PLR_RETIRED)) {
      send_to_char("Relax and enjoy being retired.\r\n", ch);
      return;
  }
  if (!ch->desc)
    return;

  one_argument(argument, arg);

  if (!*arg)
    stop_snooping(ch);
  else if (!(victim = get_char_vis(ch, arg, 1)))
    send_to_char("No such person around.\r\n", ch);
  else if (!victim->desc)
    send_to_char("There's no link.. nothing to snoop.\r\n", ch);
  else if (victim == ch)
    stop_snooping(ch);
  else if (victim->desc->snoop_by)
    send_to_char("Busy already. \r\n", ch);
  else {
    if (victim->desc->original)
      tch = victim->desc->original;
    else
      tch = victim;

    if (GET_LEVEL(tch) >= GET_LEVEL(ch)) {
      send_to_char("You can't.\r\n", ch);
      return;
    }

    if (GET_LEVEL(tch) >= LVL_IMMORT) {
      send_to_char("You sense someone peeking over your shoulder.\r\n", tch);
    }

    sendChar( ch, CONFIG_OK );

    mudlog( BRF, GET_LEVEL(ch), FALSE, "%s has started to snoop %s", GET_NAME(ch), GET_NAME(victim));

    if (ch->desc->snooping)
      ch->desc->snooping->snoop_by = NULL;

    ch->desc->snooping = victim->desc;
    victim->desc->snoop_by = ch->desc;
  }
}



ACMD(do_switch)
{
  CharData *victim;

  if (IS_NPC(ch))
    return;

  one_argument(argument, arg);

  if( ch->desc->original )
    send_to_char("You're already switched.\r\n", ch);

  else if( !*arg )
    send_to_char("Switch with who?\r\n", ch);

  else if( !(victim = get_char_vis(ch, arg, 0)) )
    send_to_char("No such character.\r\n", ch);

  else if( ch == victim )
    send_to_char("Hee hee... we are jolly funny today, eh?\r\n", ch);

  else if( victim->desc )
    send_to_char("You can't do that, the body is already in use!\r\n", ch);

  else if( (GET_LEVEL(ch) < LVL_IMPL) && !IS_NPC(victim) )
    send_to_char("You aren't holy enough to use a mortal's body.\r\n", ch);

  else if( PLR_FLAGGED(victim, PLR_FROZEN ))
    sendChar( ch, "That player is currently forzen." );

  else
  {
    sendChar( ch, CONFIG_OK );

    ch->desc->character = victim;
    ch->desc->original = ch;

    victim->desc = ch->desc;
    ch->desc = NULL;
  }
}


ACMD(do_return)
{
  if (ch->desc && ch->desc->original) {
    send_to_char("You return to your original body.\r\n", ch);

    ch->desc->character = ch->desc->original;
    ch->desc->original = NULL;

    ch->desc->character->desc = ch->desc;
    ch->desc = NULL;
  }
}


void
cloneEquipment( CharData *ch )
{
    FILE *fl;
    char fname[MAX_STRING_LENGTH];
    struct obj_file_elem object;
    struct rent_info rent;

    if( !get_filename(GET_NAME(ch), fname, CRASH_FILE) ) return;

    if( !(fl = fopen(fname, "r+b")) ){
        mudlog(NRM, LVL_IMMORT, TRUE, "CLONE: Cannot open object file." );
        return;
    }

    if( !feof(fl) )
        fread( &rent, sizeof(rent), 1, fl );

    while( !feof(fl) ){
        fread(&object, sizeof(struct obj_file_elem), 1, fl);
        if (ferror(fl)) {
            mudlog(NRM, LVL_IMMORT, TRUE, "ERROR loading cloned equipment for %s", GET_NAME(ch));
            fclose(fl);
            return;
        }

        if( !feof(fl) ){
            ObjData *clonedObj = Obj_from_store(object);

            if( clonedObj != NULL ){
                if( clonedObj->worn_at >= 0 ){
                    equip_char(ch, clonedObj, clonedObj->worn_at );
		    SET_BIT_AR(clonedObj->obj_flags.extra_flags, ITEM_TIMED);
                    SET_BIT_AR(clonedObj->obj_flags.extra_flags, ITEM_NORENT);
                    GET_OBJ_TIMER(clonedObj)= 96;
                    GET_OBJ_COST(clonedObj) = 0;
                }
                else {
                    obj_to_char( clonedObj, ch);
                    extract_obj( clonedObj );
                }
            }
        }
    }
}


ACMD(do_clone)
{
    CharData *clone;
    CharFileU template;
    char cloneDescr[80] = "";
    char cloneName[80]  = "";

  if (IS_SET_AR(PLR_FLAGS(ch), PLR_RETIRED)) {
      send_to_char("Relax and enjoy being retired.\r\n", ch);
      return;
  }

    one_argument(argument, buf);

    if( !*buf ){
        sendChar( ch, "Usage: clone <player>\r\n" );
        return;
    }

    CREATE( clone, CharData, 1 );
    clear_char( clone );
    if( load_char( buf, &template ) < 0 ){
        sendChar( ch, "No such player - clone aborted.\r\n" );
        free(clone);
        return;
    }
    /*
    ** The saved character has been loaded into the template.
    */
    store_to_char( &template, clone );
    /*
    ** Load up the equipment.
    */
    cloneEquipment(clone);

    /*
    ** Start setting things for the clone.
    */
    GET_EXP(clone)  = GET_LEVEL(clone)*500;
    GET_GOLD(clone) = GET_LEVEL(clone)*100;
    GET_HIT(clone)  = GET_MAX_HIT(clone);
    GET_MANA(clone) = GET_MAX_MANA(clone);
    GET_MOVE(clone) = GET_MAX_MOVE(clone);

    /*
    ** Finally, turn it into a mob, add 'clone' to its name
    ** and place it in the mud.
    */
    strcpy( cloneDescr, clone->player.name );
    strcat( cloneDescr, "'s clone" );
    if( clone->player.short_descr != NULL )
        free( clone->player.short_descr );
    clone->player.short_descr = strdup( cloneDescr );

    strcpy( cloneName, clone->player.name );
    strcat( cloneName, " clone" );
    free( clone->player.name );
    clone->player.name = strdup( cloneName );

    SET_BIT_AR(MOB_FLAGS(clone), MOB_ISNPC);
    SET_BIT_AR(MOB_FLAGS(clone), MOB_CLONE);

    char_to_room(clone, ch->in_room);

    clone->next = character_list;
    character_list = clone;

    act("You have cloned $N!", FALSE, clone, 0, ch, TO_CHAR);
    sendChar( ch, "The clone has been created.\r\n" );
    mudlog(NRM, LVL_IMMORT, TRUE, "(CLONE) %s cloned %s", GET_NAME(ch), buf );
}

ACMD(do_load)
{
  CharData *mob;
  ObjData *obj;
  int number, r_num;
  int i=0, n=1;
  char buf[132], *buf3;

  if (IS_SET_AR(PLR_FLAGS(ch), PLR_RETIRED)) {
      send_to_char("Relax and enjoy being retired.\r\n", ch);
      return;
  }

  if (IS_NPC(ch))
    return;

 buf3=two_arguments(argument, buf, buf2);

  if (!*buf || !*buf2 || !isdigit(*buf2)) {
    send_to_char("Usage: load { obj | mob } <number>\r\n", ch);
    return;
  }
  if ((number = atoi(buf2)) < 0) {
    send_to_char("A NEGATIVE number??\r\n", ch);
    return;
  }
  if (is_abbrev(buf, "mob")) {
    if ((r_num = real_mobile(number)) < 0) {
      send_to_char("There is no monster with that number.\r\n", ch);
      return;
    }
    mob = read_mobile(r_num, REAL);
    if (!mob) {
      send_to_char("Failed to load object!\r\n", ch);
      return;
    }
    char_to_room(mob, ch->in_room);

    act("$n makes a quaint, magical gesture with one hand.", TRUE, ch,
	0, 0, TO_ROOM);
    act("$n has created $N!", FALSE, ch, 0, mob, TO_ROOM);
    act("You create $N.", FALSE, ch, 0, mob, TO_CHAR);
    load_mtrigger(mob);
    mlog("(LOAD) %s loaded mob %d", GET_NAME(ch), number );
  } else if (is_abbrev(buf, "obj") && GET_LEVEL(ch) >= LVL_ANGEL) {
    if ((r_num = real_object(number)) < 0) {
      send_to_char("There is no object with that number.\r\n", ch);
      return;
    }
    obj = read_perfect_object(r_num, REAL);
    if (!obj) {
      send_to_char("Failed to load object!\r\n", ch);
      return;
    }
    obj_to_room(obj, ch->in_room);
    act("$n makes a strange magical gesture.", TRUE, ch, 0, 0, TO_ROOM);
    act("$n has created $p!", FALSE, ch, obj, 0, TO_ROOM);
    act("You create $p.", FALSE, ch, obj, 0, TO_CHAR);
    load_otrigger(obj);
    mlog("(LOAD) %s loaded object %d", GET_NAME(ch), number );
  } else if( is_abbrev(buf,"tobj")) {
    skip_spaces(&buf3);
    if (!*buf2 || !*buf3  || !isdigit(*buf3)) {
    send_to_char("Usage: load tobj <number> <duration>\r\n", ch);
    return;
    }
    if ((r_num = real_object(number)) < 0) {
      send_to_char("There is no object with that number.\r\n", ch);
      return;
    }
    obj = read_perfect_object(r_num, REAL);
    SET_BIT_AR(obj->obj_flags.extra_flags, ITEM_TIMED);
    GET_OBJ_TIMER(obj) = atoi(buf3);

        for (i = 0; i < n; i++)
        {
            obj = read_object(r_num, REAL);
            if (CONFIG_LOAD_INVENTORY)
                obj_to_char(obj, ch);
            else
                obj_to_room(obj, ch->in_room);
            act("$n makes a strange magical gesture.", TRUE, ch, 0, 0, TO_ROOM);
            act("$n has created $p!", FALSE, ch, obj, 0, TO_ROOM);
            act("You create $p.", FALSE, ch, obj, 0, TO_CHAR);
            mlog("(LOAD) %s loaded object %d", GET_NAME(ch), number);
        }
  } else
    send_to_char("That'll have to be either 'obj', 'tobj', or 'mob'.\r\n", ch);
}


char *qpHelp = "&09  Usage: &10qpshow <player>\r\n"
               "&09Show player's QPs          \r\n"
			   "&10qpclr  <player>            \r\n"
			   "&09Clear player's QPs         \r\n"
			   "&10qpadd  <player> <modifier> \r\n"
			   "&09Increment QP by modifier   \r\n"
			   "&10qpdec  <player> <modifier> \r\n"
			   "&09Decrement QP by modifier   \r\n"
			   "&10qpset  <player> <modifier> \r\n"
			   "&09Set QP to modifier          &00";

ACMD(do_questPnts)
{
  char buf[132];
  int number;
  CharData *god = ch;
  CharData *mortal;

  if( IS_NPC( ch ) ) return;

  if( argument == NULL || strlen(argument) == 0 )
  {
    sendChar( god, qpHelp );
    return;
  }

  two_arguments( argument, buf, buf2 );

  if( subcmd != SCMD_QPCLR && subcmd != SCMD_QPSHOW )
  {
    if( !*buf || !*buf2 || !isdigit(*buf2) )
    {
      sendChar( god, qpHelp );
      return;
    }

    if(( number = atoi( buf2 )) < 0 ){
      sendChar( god, "The modifier must be unsigned.\r\n" );
      sendChar( god, qpHelp );
      return;
    }
  }

  if( !(mortal = get_char_vis( ch, buf, 0 )) ){
    sendChar( god, "Can't find player %s\r\n", buf );
    return;
  }

  switch( subcmd )
  {
    case SCMD_QPCLR: GET_QP(mortal)  = 0;      break;
    case SCMD_QPADD: GET_QP(mortal) += number; break;
    case SCMD_QPSET: GET_QP(mortal)  = number; break;
    case SCMD_QPDEC: GET_QP(mortal) -= number; break;

    case SCMD_QPSHOW: break;

    default: return;
  }

  sendChar( god, "\r\n&10%s has %d quest points.\r\n\n&00",
                 GET_NAME(mortal), GET_QP(mortal) );
    switch (subcmd) {
        case SCMD_QPADD:
            sendChar(mortal, "&10You have been given %d quest points.&00\r\n",
                    number);
            break;
        case SCMD_QPDEC:
            sendChar(mortal, "&10You have lost %d quest points!&00\r\n",
                    number);
            break;
    }
}


ACMD(do_reimb)
{
    char buf[132];
    ObjData *obj;
    int number, r_num;
    CharData *god = ch;
    CharData *mortal;

    if( IS_NPC( ch ) ) return;

    two_arguments( argument, buf, buf2 );

    if( !*buf || !*buf2 || !isdigit(*buf2) ){
        send_to_char("Usage: reimb <PC> <obj number>\r\n", god );
        return;
    }

    if( !(mortal = get_char_vis( ch, buf, 0 )) ){
        send_to_char( "Can't find the player.\r\n", god );
        return;
    }

    if(( number = atoi( buf2 )) < 0 ){
        send_to_char("A NEGATIVE number??\r\n", ch);
        return;
    }

    if(( r_num = real_object( number )) < 0 ){
        send_to_char("There is no object with that number.\r\n", ch);
        return;
    }
    // Reimbursment of an artifact, loads max - Bean
//    if ( IS_OBJ_STAT ( obj, ITEM_ARTIFACT ) ){
//    obj = read_perfect_object( r_num, REAL );
//    obj_to_char(obj, mortal); }
//    else
    obj = read_perfect_object( r_num, REAL );
    obj_to_char(obj, mortal );
    act( "$p appears in your hands with a flash.", TRUE, mortal, obj, 0, TO_CHAR );
    act( "You create $p.", FALSE, god, obj, 0, TO_CHAR );
    mlog("(REIMB) %s reimbursed %s with object %d", GET_NAME( god ), GET_NAME( mortal ), number );

}



ACMD(do_vstat)
{
  CharData *mob;
  int number, r_num;

  if (IS_NPC(ch))
    return;

  two_arguments(argument, buf, buf2);

  if (!*buf || !*buf2 || !isdigit(*buf2)) {
    send_to_char("Usage: vstat { obj | mob } <number>\r\n", ch);
    return;
  }
  if ((number = atoi(buf2)) < 0) {
    send_to_char("A NEGATIVE number??\r\n", ch);
    return;
  }
  if (is_abbrev(buf, "mob")) {
    if ((r_num = real_mobile(number)) < 0) {
      send_to_char("There is no monster with that number.\r\n", ch);
      return;
    }
    mob = read_mobile(r_num, REAL);
    char_to_room(mob, 0);
    do_stat_character(ch, mob);
    extract_char(mob);
  } else if (is_abbrev(buf, "obj")) {
    if ((r_num = real_object(number)) < 0) {
      send_to_char("There is no object with that number.\r\n", ch);
      return;
    }
    do_stat_object(ch, obj_proto + r_num);
  } else
    send_to_char("That'll have to be either 'obj' or 'mob'.\r\n", ch);
}

ACMD(do_qstat)
{
  int number, r_num;

  if (IS_NPC(ch))
    return;

  one_argument(argument, buf);

  if (!*buf || !isdigit(*buf)) {
    send_to_char("Usage: qstat <number>\r\n", ch);
    return;
  }
  if ((number = atoi(buf)) < 0) {
    send_to_char("A NEGATIVE number??\r\n", ch);
    return;
  }
  if ((r_num = real_quest(number)) < 0) {
    send_to_char("There is no quest with that number.\r\n", ch);
    return;
  }
  do_stat_quest(ch, qst_list + r_num);
}

/* list the quests for a zone */
ACMD(do_qlist)
{
  int number, r_num, i;

  if (IS_NPC(ch))
    return;

  one_argument(argument, buf);

  if (!*buf || !isdigit(*buf)) {
    send_to_char("Usage: qlist <zone>\r\n", ch);
    return;
  }
  if ((number = atoi(buf)) < 0) {
    send_to_char("A NEGATIVE number??\r\n", ch);
    return;
  }
  if ((r_num = real_zone(number)) < 0) {
    send_to_char("Which zone is that?\r\n", ch);
    return;
  }
  sprintf(buf, "Quests for zone #%d, %s\r\n", number, zone_table[r_num].name);
  send_to_char(buf, ch);
  for (i = 0; i <= top_of_qstt; i++) {
    if (qst_list[i].virtual < number * 100) continue;
    if (qst_list[i].virtual > zone_table[r_num].top) break;
    sprintf(buf, "#%-5d   %s\r\n", qst_list[i].virtual, qst_list[i].name);
    send_to_char(buf, ch);
  }
}


/* clean a room of all mobiles and objects */
ACMD(do_purge)
{
  char buf[MAX_INPUT_LENGTH];
  CharData *vict, *next_v;
  ObjData *obj, *next_o;



  if (IS_SET_AR(PLR_FLAGS(ch), PLR_RETIRED)) {
      send_to_char("Relax and enjoy being retired.\r\n", ch);
      return;
  }

  if (IS_NPC(ch))
    return;

  one_argument(argument, buf);

  if (*buf) {			/* argument supplied. destroy single object
				 * or char */
    if ((vict = get_char_room_vis(ch, buf))) {
      if (!IS_NPC(vict) && (GET_LEVEL(ch) <= GET_LEVEL(vict))) {
	send_to_char("Fuuuuuuuuu!\r\n", ch);
	return;
      }
      act("$n disintegrates $N.", FALSE, ch, 0, vict, TO_NOTVICT);

      if (!IS_NPC(vict)) {
	mudlog( BRF, LVL_LRGOD, TRUE, "(GC) %s has purged %s.", GET_NAME(ch), GET_NAME(vict));
	if (vict->desc) {
	  SET_DCPENDING(vict->desc);
	  vict->desc = NULL;
	}
      }
      extract_char(vict);
    } else if ((obj = get_obj_in_list_vis(ch, buf, world[ch->in_room].contents))) {
      act("$n destroys $p.", FALSE, ch, obj, 0, TO_ROOM);
      extract_obj(obj);
    } else {
      send_to_char("Nothing here by that name.\r\n", ch);
      return;
    }

    sendChar( ch, CONFIG_OK );
  } else {			/* no argument. clean out the room */
    if (IS_NPC(ch)) {
      send_to_char("Don't... You would only kill yourself..\r\n", ch);
      return;
    }
    act("$n gestures... You are surrounded by scorching flames!",
	FALSE, ch, 0, 0, TO_ROOM);
    send_to_room("The world seems a little cleaner.\r\n", ch->in_room);

    for (vict = world[ch->in_room].people; vict; vict = next_v) {
      next_v = vict->next_in_room;
      if (IS_NPC(vict))
	extract_char(vict);
    }

    for (obj = world[ch->in_room].contents; obj; obj = next_o) {
      next_o = obj->next_content;
      extract_obj(obj);
    }
  }
}



ACMD(do_demote)
{
  CharData *victim;
  char name[100], level[100];
  int newlevel, difference;
  int max_con, max_int, max_wis;
  int hpmax, mpmax, mvmax, ppmax;
  int i, newpracs;

  if (IS_SET_AR(PLR_FLAGS(ch), PLR_RETIRED)) {
      send_to_char("Relax and enjoy being retired.\r\n", ch);
      return;
  }

  if (IS_NPC(ch))
    return;

  two_arguments(argument, name, level);

  if (*name) {
    if (!(victim = get_char_vis(ch, name, 1))) {
      send_to_char("That player is not here.\r\n", ch);
      return;
    }
  } else {
    send_to_char("Demote who?\r\n", ch);
    return;
  }

  if (GET_LEVEL(ch) <= GET_LEVEL(victim)) {
    send_to_char("Maybe that's not such a great idea.\r\n", ch);
    return;
  }
  if (IS_NPC(victim)) {
    send_to_char("NO!  Not on NPC's.\r\n", ch);
    return;
  }
  if (!*level || (newlevel = atoi(level)) <= 0) {
    send_to_char("That's not a level!\r\n", ch);
    return;
  }
  if (newlevel < 1) {
    send_to_char("You can't demote below level 1.\r\n", ch);
    return;
  }
  if (newlevel > GET_LEVEL(victim)) {
      send_to_char("You'd probably better use advance.\r\n", ch);
      return;
  }

  sendChar( ch, CONFIG_OK );

  mlog("(GC) %s has demoted %s to level %d (from %d)",
	  GET_NAME(ch), GET_NAME(victim), newlevel, GET_LEVEL(victim));
  GET_EXP(victim) = titles[(int) GET_CLASS(victim)][newlevel].exp;
  difference = GET_LEVEL(victim) - newlevel;
  GET_LEVEL(victim) = newlevel;

  max_con = race_stat_limits[(int)GET_RACE(victim)][CONSTITUTION_INDEX];
  max_int = race_stat_limits[(int)GET_RACE(victim)][INTELLIGENCE_INDEX];
  max_wis = race_stat_limits[(int)GET_RACE(victim)][WISDOM_INDEX];

  hpmax = con_app[max_con].hitp + con_app[max_con].extra ? 1 : 0;
  mpmax = int_app[max_con].manap + con_app[max_con].extra ? 1 : 0;
  mvmax = 0;
  ppmax = 0;

  switch (GET_CLASS(victim)) {
      case CLASS_MAGIC_USER:
          hpmax += 8; mpmax += 9; mvmax += 3; break;
      case CLASS_CLERIC:
          hpmax += 10; mpmax += 7; mvmax += 3; break;
      case CLASS_THIEF:
          hpmax += 13; mpmax = 0; mvmax += 3; break;
      case CLASS_WARRIOR:
          hpmax += 16; mpmax = 0; mvmax += 3; break;
      case CLASS_RANGER:
          hpmax += 15; mpmax = (mpmax/2) + 3; mvmax += 4; break;
      case CLASS_ASSASSIN:
          hpmax += 13; mpmax = (mpmax/2) + 3; mvmax += 3; break;
      case CLASS_SHOU_LIN:
          hpmax += 13; mpmax = (mpmax/2) + 3; mvmax += 3; break;
      case CLASS_SOLAMNIC_KNIGHT:
          hpmax += 15; mpmax = (mpmax/2) + 4; mvmax += 3; break;
      case CLASS_DEATH_KNIGHT:
          hpmax += 15; mpmax = (mpmax/2) + 4; mvmax += 3; break;
      case CLASS_SHADOW_DANCER:
          hpmax += 13; mpmax = (mpmax/2) + 5; mvmax += 3; break;
  }

  victim->points.max_hit -= difference * hpmax;
  victim->points.max_mana -= difference * mpmax;
  victim->points.max_move -= difference * mvmax;

  if (victim->points.max_hit < 1) victim->points.max_hit = 1;
  if (victim->points.max_mana < 1) victim->points.max_mana = 1;
  if (victim->points.max_move < 1) victim->points.max_move = 1;

  newpracs = GET_PRACTICES(victim);
  newpracs -= difference * wis_app[max_wis].bonus;
  if (wis_app[max_wis].extra) newpracs -= difference;
  if (newpracs < 0) newpracs = 0;
  GET_PRACTICES(victim) = newpracs;

  for (i = 0; i < TOP_SPELL_DEFINE; i++) {
      int minlev = spell_info[i].min_level[GET_CLASS(victim)];
      if (minlev > GET_LEVEL(victim) && minlev < LVL_IMMORT) {
          SET_SKILL(victim, i, 0);
      } else if (minlev > 0 && GET_SKILL(victim, i) > 60) {
          SET_SKILL(victim, i, GET_SKILL(victim, i) - difference * 2);
          if (GET_SKILL(victim, i) < 60) SET_SKILL(victim, i, 60);
      }
  }

  save_char(victim, NOWHERE);
}

ACMD(do_advance)
{
  CharData *victim;
  char name[100], level[100];
  int newlevel;

  void gain_exp(CharData * ch, int gain);

  if (IS_SET_AR(PLR_FLAGS(ch), PLR_RETIRED)) {
      send_to_char("Relax and enjoy being retired.\r\n", ch);
      return;
  }

  if (IS_NPC(ch))
    return;

  two_arguments(argument, name, level);

  if (*name) {
    if (!(victim = get_char_vis(ch, name, 1))) {
      send_to_char("That player is not here.\r\n", ch);
      return;
    }
  } else {
    send_to_char("Advance who?\r\n", ch);
    return;
  }

  if (GET_LEVEL(ch) <= GET_LEVEL(victim)) {
    send_to_char("Maybe that's not such a great idea.\r\n", ch);
    return;
  }
  if (IS_NPC(victim)) {
    send_to_char("NO!  Not on NPC's.\r\n", ch);
    return;
  }
  if (!*level || (newlevel = atoi(level)) <= 0) {
    send_to_char("That's not a level!\r\n", ch);
    return;
  }
  if (newlevel > LVL_IMPL) {
    sprintf(buf, "%d is the highest possible level.\r\n", LVL_IMPL);
    send_to_char(buf, ch);
    return;
  }
  if (newlevel > GET_LEVEL(ch)) {
    send_to_char("Yeah, right.\r\n", ch);
    return;
  }
  if (newlevel < GET_LEVEL(victim)) {
    roll_real_abils(victim);	/* used to be in do_start */
    do_start(victim);
    GET_LEVEL(victim) = newlevel;
  } else {
    act("$n makes some strange gestures.\r\n"
	"A strange feeling comes upon you,\r\n"
	"Like a giant hand, light comes down\r\n"
	"from above, grabbing your body, that\r\n"
	"begins to pulse with colored lights\r\n"
	"from inside.\r\n\r\n"
	"Your head seems to be filled with demons\r\n"
	"from another plane as your body dissolves\r\n"
	"to the elements of time and space itself.\r\n"
	"Suddenly a silent explosion of light\r\n"
	"snaps you back to reality.\r\n\r\n"
	"You feel slightly different.", FALSE, ch, 0, victim, TO_VICT);
  }

  sendChar( ch, CONFIG_OK );

  mlog("(GC) %s has advanced %s to level %d (from %d)",
	  GET_NAME(ch), GET_NAME(victim), newlevel, GET_LEVEL(victim));
  gain_exp_regardless(victim,
	 (titles[(int) GET_CLASS(victim)][newlevel].exp) - GET_EXP(victim));
  save_char(victim, NOWHERE);
}

void restore( CharData *ch, CharData *vict )
{
  int i;

  if(GET_HIT(vict) < GET_MAX_HIT(vict))
      GET_HIT(vict) = GET_MAX_HIT(vict);
  GET_MANA(vict) = GET_MAX_MANA(vict);
  GET_MOVE(vict) = GET_MAX_MOVE(vict);

  if(GET_COND(ch, THIRST) != -1)
      GET_COND(ch, THIRST) = 24;
  if(GET_COND(ch, HUNGER) != -1)
      GET_COND(ch, HUNGER)   = 24;
  GET_COND(ch, DRUNK)  = 0;

  if ((GET_LEVEL(ch) >= LVL_GRGOD) && (GET_LEVEL(vict) >= LVL_IMMORT)) {
    for (i = 1; i <= MAX_SKILLS; i++)
      SET_SKILL(vict, i, 100);

    if (GET_LEVEL(vict) >= LVL_HERO) {
      vict->real_abils.str_add = 100;
      vict->real_abils.intel = 25;
      vict->real_abils.wis = 25;
      vict->real_abils.dex = 25;
      vict->real_abils.str = 25;
      vict->real_abils.con = 25;
    }
    vict->aff_abils = vict->real_abils;
  }

  update_pos(vict);
  
  if(ch != vict)
    act("You have been fully healed by $N!", FALSE, vict, 0, ch, TO_CHAR | TO_SLEEP);

}

ACMD(do_restore)
{
  CharData *vict;

  if (IS_NPC(ch))
    return;

  if (IS_SET_AR(PLR_FLAGS(ch), PLR_RETIRED)) {
      send_to_char("Sit back and relax.  Your retired.\r\n", ch);
      return;
  }

  one_argument(argument, buf);
  if (!*buf)
    send_to_char("Whom do you wish to restore?\r\n", ch);
  else if ( !strcmp (buf, "all") )
    {
      int count =0;
      
      send_to_char("Restoring all...\r\n",ch);
            
      for (vict=character_list;vict;vict=vict->next) 
	{
	  if ((CAN_SEE(ch,vict)) && (GET_LEVEL(vict)< LVL_IMMORT) && (!IS_NPC(vict)))
	    {
	      sprintf(buf,"Restoring %s.\r\n",vict->player.name);
	      send_to_char(buf,ch);
	      restore(ch,vict);
	      count++;
	    }
	}
      sprintf(buf,"%d character(s) restored.\r\n",count);
      send_to_char(buf,ch);
    }
  else if (!(vict = get_char_vis(ch, buf, 0)))
    sendChar( ch, CONFIG_NOPERSON );
  else {
    restore(ch, vict);
    sendChar( ch, CONFIG_OK );
  }
}


ACMD(do_fishoff) {
    one_argument(argument, arg);
    
    int duration = atoi(arg);
    
    if(!*arg) {
      if(fishoff) {
          sendChar(ch, "To end a fishoff prematurely, you must set the fishoff duration of -1!\r\n");
          return;
      }
      else
          duration = 24;
    }
    

    if(duration < -1 || duration == 0 || duration > 72) {
        sendChar(ch, "Usage: fishoff <duration> - Set duration to -1 to end the fishoff.\r\n");
    }
    if(duration == -1) {
        if(fishoff) {
            fishoff = 0;
            gecho("&10The Fishmaster announces: &13An Immortal has terminated The Great Fish-Off early!&00\r\n");
            final_fishoff_score();
            return;
        }
        else {
            sendChar(ch, "There is no fishoff in progress!");
        }
    }
        
    if(!fishoff) {
        gecho("&10The Fishmaster announces: &13The Great Fish-Off has BEGUN!&00\r\n");
        fishoff = duration;
    }
    else {   
        sendChar(ch, "A fishoff is already in session.  To end it, set the duration to -1.\r\n");
    }
    
}

ACMD(do_muckle) {
    DescriptorData *d;
    CharData *tch = NULL;
    char duration[250], pvpFactor[250];
   
    two_arguments(argument, duration, pvpFactor);

    if(muckle_active) {
        sendChar(ch, muckle_scoreboard());
        return;
    }

    // Immortal can specify game duration, else it assumes 3 minutes
    if(!(muckle_duration = atoi(duration))) {
        muckle_duration = 3;
    }
    muckle_duration = clamp(muckle_duration, 1, 10);

    if(!(muckle_pvpFactor = atoi(pvpFactor))) {
        muckle_pvpFactor = 1;
    }
    muckle_pvpFactor = clamp(muckle_pvpFactor, 1, 10);

    if(GET_LEVEL(ch) < LVL_IMMORT)
    {
        sendChar(ch, "There is no muckling going on right now.\r\n");
        return;
    }

    if(CONFIG_QUEST_ACTIVE) {
        sendChar(ch, "Quest channel must be off to begin a game of muckle.\r\n");
        return;
    }

    muckle_active = 1;
    the_muckle = NULL;

    do_restore(ch,  "all", 0, 0);
    do_quest(ch, "active", 0, 0);
    send_to_all("&12A game of muckle has begun.&00\r\n");
    send_to_all("&12Your quest flag has been enabled.  Type 'quest' to leave the game's chat.&00\r\n");

    for( d = descriptor_list; d; d = d->next )
    {
        if( !d->connected && d->character )
            tch = d->character;
        else continue;

        do_quest(d->character, "", 0, 0);

        sendChar(d->character, "You have been granted spells to aid you.\r\n");
        // Level the playing field by giving all players vision and sanctuary.
        affect_from_char(d->character, SPELL_SANCTUARY);
        add_affect(ch, d->character, SPELL_SANCTUARY, 50, 0, 0, 15 TICKS, AFF_SANCTUARY, TRUE, FALSE, FALSE, FALSE);
        affect_from_char(d->character, SPELL_SENSE_LIFE);
        add_affect(ch, d->character, SPELL_SENSE_LIFE, 50, 0, 0, 15 TICKS, AFF_SENSE_LIFE, TRUE, FALSE, FALSE, FALSE);
        affect_from_char(d->character, SPELL_DETECT_INVIS);
        add_affect(ch, d->character, SPELL_DETECT_INVIS, 50, 0, 0, 15 TICKS, AFF_DETECT_INVIS, TRUE, FALSE, FALSE, FALSE);
        affect_from_char(d->character, SPELL_INFRAVISION);
        add_affect(ch, d->character, SPELL_INFRAVISION, 50, 0, 0, 15 TICKS, AFF_INFRAVISION, TRUE, FALSE, FALSE, FALSE);

        if(IN_ARENA(tch)) {
            char_from_room(tch);
            stop_fighting(tch);
            if (tch->mount) {
                char_from_room(tch->mount);
                stop_fighting(tch->mount);
                char_to_room(tch->mount, real_room(18001));
                sendChar(tch->mount, "You have been magically returned to Samsera.\r\n\r\n");
                do_look(tch->mount, "", 0, 0);
            }
            char_to_room(tch, real_room(18001));
            sendChar(tch, "You have been magically returned to Samsera.\r\n");
            do_look(tch, "", 0, 0);
        }
    }

}

ACMD(do_invis)
{
  int level;

  if (IS_NPC(ch)) {
    send_to_char("Cha.. like a mob knows how to bend light.\r\n", ch);
    return;
  }
  one_argument(argument, arg);
  if (!*arg) {
    if (GET_INVIS_LEV(ch) > 0) {
      GET_INVIS_LEV(ch) = 0;
      sprintf(buf, "You are now fully visible.\r\n");
    } else {
      GET_INVIS_LEV(ch) = GET_LEVEL(ch);
      sprintf(buf, "Your invisibility level is %d.\r\n", GET_LEVEL(ch));
    }
  } else {
    level = atoi(arg);
    if (level > GET_LEVEL(ch)) {
      send_to_char("You can't go invisible above your own level.\r\n", ch);
      return;
    } else if (level < 1) {
      GET_INVIS_LEV(ch) = 0;
      sprintf(buf, "You are now fully visible.\r\n");
    } else {
      GET_INVIS_LEV(ch) = level;
      sprintf(buf, "Your invisibility level is now %d.\r\n", level);
    }
  }
  send_to_char(buf, ch);
}


ACMD(do_gecho)
{
	DescriptorData *pt;

	if (IS_NPC(ch))
		return;

	skip_spaces(&argument);

	if (!*argument)
	{
		send_to_char("That must be a mistake...\r\n", ch);
		return;
	}

	// Imms were just using inverted color gecho's to be obnoxious, 
	// so going to disable it for the time being.  Craklyn
	if ( !immortal_color(argument, ch) )
		return;

	sprintf(buf, "%s\r\n", argument);
	for (pt = descriptor_list; pt; pt = pt->next)
		if (!pt->connected && pt->character && pt->character != ch)
			send_to_char(buf, pt->character);
	if (PRF_FLAGGED(ch, PRF_NOREPEAT))
		sendChar( ch, CONFIG_OK );
	else
		send_to_char(buf, ch);
}

ACMD(do_qecho)
{
  if(IS_NPC(ch)) return;

  skip_spaces(&argument);

  if (!*argument)
  {
    send_to_char("That must be a mistake...\r\n", ch);
  }

  else
  {
    DescriptorData *vict;
    char buf[256] = "";

    sprintf( buf, "%s\r\n", argument );

    for( vict = descriptor_list; vict; vict = vict->next )
    {
      if( !vict->connected &&
           vict->character &&
           vict->character != ch &&
           PRF_FLAGGED( vict->character, PRF_QUEST ))
	send_to_char(buf, vict->character);
    }
    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
      sendChar( ch, CONFIG_OK );
    else
      send_to_char(buf, ch);
  }
}

#ifdef OLDWAY
ACMD(do_poofset)
{
  char **msg;

  switch (subcmd) {
  case SCMD_POOFIN:
    msg = &(POOFIN(ch));
    break;
  case SCMD_POOFOUT:
    msg = &(POOFOUT(ch));
    break;
  default:
    return;
    break;
  }

  skip_spaces(&argument);

  if (*msg)
    free(*msg);

  if (!*argument)
    *msg = NULL;
  else
    *msg = str_dup(argument);

  sendChar( ch, CONFIG_OK );
}
#else
ACMD(do_poofset)
{
  int show_poof = 0, set_poofin = 0, set_poofout = 0;

  switch (subcmd) {
     case SCMD_POOF:
       show_poof = 1;
       break;
     case SCMD_POOFIN:
       set_poofin = 1;
       break;
     case SCMD_POOFOUT:
       set_poofout = 1;
       break;
     default:
       break;
  }

  skip_spaces(&argument);

  if (show_poof == 1) {
      send_to_char("Your current poofin string is:\r\n", ch);
      sprintf(buf, "%s", POOFIN(ch));
      send_to_char(buf, ch);
      send_to_char("\r\n\r\n", ch);
      send_to_char("Your current poofout string is:\r\n", ch);
      sprintf(buf, "%s", POOFOUT(ch));
      send_to_char(buf, ch);
      return;
  }

  if (set_poofin == 1) {
      if (GET_LEVEL(ch) < 59)
          if (strstr(argument, "%s") == NULL) {
              send_to_char("You must place '%s' for where you want your name to be.\r\n", ch);
              return;
          }
      sprintf(buf, argument, GET_NAME(ch), "", "", "", "", "", "");
      if (strlen(buf) > 79) {   /* chars + NULL */
          send_to_char("Your entrance string is too long!\r\n", ch);
          return;
      }
      POOFIN(ch) = str_dup(buf);
      send_to_char("Your entrance string is\r\n", ch);
      send_to_char(POOFIN(ch), ch);
      send_to_char("\r\n", ch);
  }

  if (set_poofout == 1) {
      if (GET_LEVEL(ch) < 59)
          if (strstr(argument, "%s") == NULL) {
              send_to_char("You must place '%s' for where you want your name to be.\r\n", ch);
              return;
          }
      sprintf(buf, argument, GET_NAME(ch), "", "", "", "", "", "");
      if (strlen(buf) > 79) {   /* chars + NULL: */
          send_to_char("Your exit string is too long!\r\n", ch);
          return;
      }
      POOFOUT(ch) = str_dup(buf);
      send_to_char("Your exit string is\r\n", ch);
      send_to_char(POOFOUT(ch), ch);
      send_to_char("\r\n", ch);
  }
}
#endif



ACMD(do_dc)
{
  char arg[MAX_INPUT_LENGTH];
  DescriptorData *d;
  int num_to_dc;

  if( IS_NPC(ch) )
  {
    sendChar( ch, "Monsters can't cut connections... leave me alone.\r\n" );
    return;
  }

  one_argument(argument, arg);
  if (!(num_to_dc = atoi(arg))) {
    sendChar(ch, "Usage: DC <user number> (type USERS for a list)\r\n");
    return;
  }
  for (d = descriptor_list; d && d->desc_num != num_to_dc; d = d->next);

  if (!d) {
    sendChar(ch, "No such connection.\r\n");
    return;
  }
  if (d->character && GET_LEVEL(d->character) >= GET_LEVEL(ch)) {
    if (!CAN_SEE(ch, d->character))
      sendChar(ch, "No such connection.\r\n");
    else
      sendChar(ch, "Umm.. maybe that's not such a good idea...\r\n");
    return;
  }

  SET_DCPENDING(d);

    sendChar(ch, "Connection #%d closed.\r\n", num_to_dc);
    mlog("(GC) Connection closed by %s.", GET_NAME(ch));
}



ACMD(do_wizlock)
{
  int value;
  char *when;

  if (IS_SET_AR(PLR_FLAGS(ch), PLR_RETIRED)) {
      send_to_char("Relax and enjoy being retired.\r\n", ch);
      return;
  }

  one_argument(argument, arg);
  if (*arg) {
    value = atoi(arg);
    if (value < 0 || value > GET_LEVEL(ch)) {
      send_to_char("Invalid wizlock value.\r\n", ch);
      return;
    }
    circle_restrict = value;
    when = "now";
  } else
    when = "currently";

  switch (circle_restrict) {
  case 0:
    sprintf(buf, "The game is %s completely open.\r\n", when);
    break;
  case 1:
    sprintf(buf, "The game is %s closed to new players.\r\n", when);
    break;
  default:
    sprintf(buf, "Only level %d and above may enter the game %s.\r\n",
	    circle_restrict, when);
    break;
  }
  send_to_char(buf, ch);
}

ACMD(do_date)
{
  char *tmstr;
  time_t mytime;
  int d, h, m;

  if (IS_NPC(ch))
    return;

  if (subcmd == SCMD_DATE)
    mytime = time(0);
  else
    mytime = boot_time;

  tmstr = (char *) asctime(localtime(&mytime));
  *(tmstr + strlen(tmstr) - 1) = '\0';

  if (subcmd == SCMD_DATE)
    sendChar(ch, "Current machine time: %s\r\n", tmstr);
  else {
    mytime = time(0) - boot_time;
    d = mytime / 86400;
    h = (mytime / 3600) % 24;
    m = (mytime / 60) % 60;

    sendChar(ch, "Up since %s: %d day%s, %d:%02d\r\n", tmstr, d, d == 1 ? "" : "s", h, m);
  }
}

ACMD(do_last)
{
  struct char_file_u chdata;

  if( IS_NPC(ch) ) return;

  one_argument(argument, arg);

  if( !*arg )
  {
    sendChar( ch, "For whom do you wish to search?\r\n" );
    return;
  }

  if( load_char( arg, &chdata ) < 0 )
  {
    sendChar( ch, "There is no such player.\r\n" );
    return;
  }

  if((chdata.level > GET_LEVEL(ch)) && (GET_LEVEL(ch) < LVL_DEITY))
  {
    sendChar( ch, "You are not sufficiently godly for that!\r\n" );
    return;
  }

  sendChar( ch, "[%5ld] [%2d %s %s] %-12s : %-18s : %-20s\r\n",
	    chdata.char_specials_saved.idnum,
            (int) chdata.level,
	    class_abbrevs[(int) chdata.class],
            race_abbrevs[(int) chdata.race],
	    chdata.name, chdata.host, ctime(&chdata.last_logon));
}


ACMD(do_force)
{
  DescriptorData *i, *next_desc;
  CharData *vict, *next_force;
  char to_force[MAX_STRING_LENGTH + 2];

  if (IS_SET_AR(PLR_FLAGS(ch), PLR_RETIRED)) {
      send_to_char("Relax and enjoy being retired.\r\n", ch);
      return;
  }

  if (IS_NPC(ch)) {
    send_to_char("Umm.... no.\r\n", ch);
    return;
  }
  half_chop(argument, arg, to_force);

  sprintf(buf1, "$n has forced you to '%s'.", to_force);

  if (!*arg || !*to_force)
    send_to_char("Whom do you wish to force do what?\r\n", ch);
  else if ((GET_LEVEL(ch) < LVL_GOD) || (str_cmp("all", arg) && str_cmp("room", arg))) {
    if (!(vict = get_char_vis(ch, arg, 0)))
      sendChar( ch, CONFIG_NOPERSON );
    else if (GET_LEVEL(ch) <= GET_LEVEL(vict))
      send_to_char("No, no, no!\r\n", ch);
    else {
      sendChar( ch, CONFIG_OK );
      act(buf1, TRUE, ch, NULL, vict, TO_VICT);
      mudlog( NRM, MAX(LVL_LRGOD, GET_INVIS_LEV(ch)), TRUE, "(GC) %s forced %s to %s", GET_NAME(ch), GET_NAME(vict), to_force);
      command_interpreter(vict, to_force);
    }
  } else if (!str_cmp("room", arg)) {
    sendChar( ch, CONFIG_OK );
    mudlog( NRM, MAX(LVL_LRGOD, GET_INVIS_LEV(ch)), TRUE,
           "(GC) %s forced room %d to %s", GET_NAME(ch), world[ch->in_room].number, to_force);

    for (vict = world[ch->in_room].people; vict; vict = next_force) {
      next_force = vict->next_in_room;
      if (GET_LEVEL(vict) >= GET_LEVEL(ch))
	continue;
      act(buf1, TRUE, ch, NULL, vict, TO_VICT);
      command_interpreter(vict, to_force);
    }
  } else { /* force all */
    sendChar( ch, CONFIG_OK );
    mudlog( NRM, MAX(LVL_LRGOD, GET_INVIS_LEV(ch)), TRUE, "(GC) %s forced all to %s", GET_NAME(ch), to_force);

    for (i = descriptor_list; i; i = next_desc) {
      next_desc = i->next;

      if (i->connected || !(vict = i->character) || GET_LEVEL(vict) >= GET_LEVEL(ch))
	continue;
      act(buf1, TRUE, ch, NULL, vict, TO_VICT);
      command_interpreter(vict, to_force);
    }
  }
}



ACMD(do_wiznet)
{
  char buf1[MAX_INPUT_LENGTH + MAX_NAME_LENGTH + 32],
       buf2[MAX_INPUT_LENGTH + MAX_NAME_LENGTH + 32], *msg;
  DescriptorData *d;
  char emote = FALSE;
  char any = FALSE;
  int level = LVL_IMMORT;

  if (IS_NPC(ch)) {
    sendChar(ch, "Yeah - like the Gods are interested in listening to mobs.\r\n");
    return;
  }
  skip_spaces(&argument);
  delete_doubledollar(argument);

  if (!*argument) {
    sendChar(ch, "Usage: wiznet [ #<level> ] [<text> | *<emotetext> | @ ]\r\n");
    return;
  }
  switch (*argument) {
  case '*':
    emote = TRUE;
  case '#':
    one_argument(argument + 1, buf1);
    if (is_number(buf1)) {
      half_chop(argument+1, buf1, argument);
      level = MAX(atoi(buf1), LVL_IMMORT);
      if (level > GET_LEVEL(ch)) {
	sendChar(ch, "You can't wizline above your own level.\r\n");
	return;
      }
    } else if (emote)
      argument++;
    break;

  case '@':
    sendChar(ch, "God channel status:\r\n");
    for (any = 0, d = descriptor_list; d; d = d->next) {
      if (STATE(d) != CON_PLAYING || GET_LEVEL(d->character) < LVL_IMMORT)
        continue;
      if (!CAN_SEE(ch, d->character))
        continue;

      sendChar(ch, "  %-*s%s%s%s\r\n", MAX_NAME_LENGTH, GET_NAME(d->character),
		PLR_FLAGGED(d->character, PLR_WRITING) ? " (Writing)" : "",
		PLR_FLAGGED(d->character, PLR_MAILING) ? " (Writing mail)" : "",
		PRF_FLAGGED(d->character, PRF_NOWIZ) ? " (Offline)" : "");
      }
    return;

  case '\\':
    ++argument;
    break;
  default:
    break;
  }
  if (PRF_FLAGGED(ch, PRF_NOWIZ)) {
    sendChar(ch, "You are offline!\r\n");
    return;
  }
  skip_spaces(&argument);

  if (!*argument) {
    sendChar(ch, "Don't bother the gods like that!\r\n");
    return;
  }
  if (level > LVL_IMMORT) {
    snprintf(buf1, sizeof(buf1), "&06%s: <%d> %s%s&00\r\n", GET_NAME(ch), level, emote ? "<--- " : "", argument);
    snprintf(buf2, sizeof(buf1), "&06Someone: <%d> %s%s&00\r\n", level, emote ? "<--- " : "", argument);
  } else {
    snprintf(buf1, sizeof(buf1), "&06%s: %s%s&00\r\n", GET_NAME(ch), emote ? "<--- " : "", argument);
    snprintf(buf2, sizeof(buf1), "&06Someone: %s%s&00\r\n", emote ? "<--- " : "", argument);
  }

  for (d = descriptor_list; d; d = d->next) {
    if (IS_PLAYING(d) && (GET_LEVEL(d->character) >= level) &&
	(!PRF_FLAGGED(d->character, PRF_NOWIZ))
	&& (d != ch->desc || !(PRF_FLAGGED(d->character, PRF_NOREPEAT)))) {
      if (CAN_SEE(d->character, ch)) {
        msg = strdup(buf1);
      sendChar(d->character, "%s", buf1);
      } else {
        msg = strdup(buf2);
        sendChar(d->character, "%s", buf2);
     }
   }
 }

  if (PRF_FLAGGED(ch, PRF_NOREPEAT))
    sendChar(ch, "%s", CONFIG_OK);
}



ACMD(do_zreset)
{
  int i, j;

    if( IS_NPC(ch) ) return;

  one_argument(argument, arg);
  if (!*arg) {
    send_to_char("You must specify a zone.\r\n", ch);
    return;
  }
  if (*arg == '*') {
    for (i = 0; i <= top_of_zone_table; i++)
      reset_zone(i);
    send_to_char("Reset world.\r\n", ch);
    return;
  } else if (*arg == '.')
    i = world[ch->in_room].zone;
  else {
    j = atoi(arg);
    for (i = 0; i <= top_of_zone_table; i++)
      if (zone_table[i].number == j)
	break;
  }
  if (i >= 0 && i <= top_of_zone_table) {
    reset_zone(i);
    sprintf(buf, "Reset zone %d (#%d): %s.\r\n", i, zone_table[i].number,
	    zone_table[i].name);
    send_to_char(buf, ch);
    mudlog( NRM, MAX(LVL_LRGOD, GET_INVIS_LEV(ch)), TRUE,
           "(GC) %s reset zone %d (%s)", GET_NAME(ch), i, zone_table[i].name);
  } else
    send_to_char("Invalid zone number.\r\n", ch);
}


/*
 *  General fn for wizcommands of the sort: cmd <player>
 */

ACMD(do_wizutil)
{
  CharData *vict;
  int taeller;
  long result;

  one_argument(argument, arg);

  if (IS_NPC(ch))
    sendChar(ch, "You're just an unfrozen caveman NPC.\r\n");
  else if (!*arg)
    sendChar(ch, "Yes, but for whom?!?\r\n");
  else if (!(vict = get_char_vis(ch, arg, 0)))
    sendChar(ch, "There is no such player.\r\n");
  else if (IS_SET_AR(PLR_FLAGS(ch), PLR_RETIRED))
    sendChar(ch, "Relax and enjoy being retired.\r\n");
  else if (IS_NPC(vict))
    sendChar(ch, "You can't do that to a mob!\r\n");
  else if (GET_LEVEL(vict) >= GET_LEVEL(ch) && vict != ch)
    sendChar(ch, "Hmmm...you'd better not.\r\n");
  else {
    switch (subcmd) {
    case SCMD_REROLL:
      sendChar(ch, "Rerolled...\r\n");
      roll_real_abils(vict);
      mlog("(GC) %s has rerolled %s.", GET_NAME(ch), GET_NAME(vict));
      sendChar(ch, "New stats: Str %d/%d, Int %d, Wis %d, Dex %d, Con %d, Cha %d\r\n",
	      GET_STR(vict), GET_ADD(vict), GET_INT(vict), GET_WIS(vict),
	      GET_DEX(vict), GET_CON(vict), GET_CHA(vict));
      break;
    case SCMD_PARDON:
      if (!PLR_FLAGGED(vict, PLR_THIEF) && !PLR_FLAGGED(vict, PLR_KILLER)) {
	sendChar(ch, "They need to commit a crime before you can pardon them deputy Do-Good!\r\n");
	return;
      }
      	REMOVE_BIT_AR(PLR_FLAGS(vict), PLR_THIEF);
	REMOVE_BIT_AR(PLR_FLAGS(vict), PLR_KILLER);
	REMOVE_BIT_AR(PLR_FLAGS(vict), PLR_JAILED);
	REMOVE_BIT_AR(PLR_FLAGS(vict), PLR_CONJ_TMR);
      (vict)->player_specials->saved.pkill_countdown = 0;
      (vict)->player_specials->saved.pthief_countdown = 0;
      (vict)->player_specials->saved.jail_timer = 0;
      (vict)->player_specials->saved.conj_countdown[0] = 0;
      (vict)->player_specials->saved.conj_countdown[1] = 0;
      (vict)->player_specials->saved.conj_countdown[2] = 0;
      (vict)->player_specials->saved.conj_countdown[3] = 0;
      GET_QUEST_WAIT(vict) = 0;
      unset_hunted_player(vict);
      sendChar(ch, "Pardoned.\r\n");
      sendChar(vict, "You have been pardoned by the Gods!\r\n");
      mudlog( BRF, MAX(LVL_LRGOD, GET_INVIS_LEV(ch)), TRUE, "(GC) %s pardoned by %s", GET_NAME(vict), GET_NAME(ch));
      break;
    case SCMD_NOTITLE:
      result = PLR_TOG_CHK(vict, PLR_NOTITLE);
      mudlog( NRM, MAX(LVL_LRGOD, GET_INVIS_LEV(ch)), TRUE, "(GC) Notitle %s for %s by %s.", ONOFF(result),
	      GET_NAME(vict), GET_NAME(ch));
      sendChar(vict, "Your NOTITLE has been turned %s.\r\n", ONOFF(result));
      break;
    case SCMD_SQUELCH:
      result = PLR_TOG_CHK(vict, PLR_NOSHOUT);
      sendChar(vict, "A bitter cold gust of wind blasts into your nostrils and down your throat!\r\n");
      sendChar(ch, "You've turned %s's mute %s.\r\n", GET_NAME(vict), ONOFF(result));
      mudlog( BRF, MAX(LVL_LRGOD, GET_INVIS_LEV(ch)), TRUE, "(GC) Mute %s for %s by %s.", ONOFF(result),
	      GET_NAME(vict), GET_NAME(ch));
      break;
    case SCMD_FREEZE:
      if (ch == vict) {
	sendChar(ch, "Oh, yeah, THAT'S real smart...\r\n");
	return;
      }
      if (PLR_FLAGGED(vict, PLR_FROZEN)) {
	sendChar(ch, "Your victim is already pretty cold.\r\n");
	return;
      }
      SET_BIT_AR(PLR_FLAGS(vict), PLR_FROZEN);
      GET_FREEZE_LEV(vict) = GET_LEVEL(ch);
      sendChar(vict, "A bitter wind suddenly rises and drains every erg of heat from your body!\r\nYou feel frozen!\r\n");
      sendChar(ch, "Frozen.\r\n");
      act("A sudden cold wind conjured from nowhere freezes $n!", FALSE, vict, 0, 0, TO_ROOM);
      mudlog( BRF, MAX(LVL_LRGOD, GET_INVIS_LEV(ch)), TRUE, "(GC) %s frozen by %s.", GET_NAME(vict), GET_NAME(ch));
      break;
    case SCMD_THAW:
      if (!PLR_FLAGGED(vict, PLR_FROZEN)) {
	sendChar(ch, "Sorry, your victim is not morbidly encased in ice at the moment.\r\n");
	return;
      }
      if (GET_FREEZE_LEV(vict) > GET_LEVEL(ch)) {
	sendChar(ch, "Sorry, a level %d God froze %s... you can't unfreeze %s.\r\n",
	   GET_FREEZE_LEV(vict), GET_NAME(vict), HMHR(vict));
	return;
      }
      mudlog( BRF, MAX(LVL_LRGOD, GET_INVIS_LEV(ch)), TRUE, "(GC) %s un-frozen by %s.", GET_NAME(vict), GET_NAME(ch));
      REMOVE_BIT_AR(PLR_FLAGS(vict), PLR_FROZEN);
      sendChar(vict, "A fireball suddenly explodes in front of you, melting the ice!\r\nYou feel thawed.\r\n");
      sendChar(ch, "Thawed.\r\n");
      act("A sudden fireball conjured from nowhere thaws $n!", FALSE, vict, 0, 0, TO_ROOM);
      break;
    case SCMD_UNAFFECT:
      if (vict->affected || AFF_FLAGS(vict)) {
	while (vict->affected) /* affect_remove FALSE is for fall rooms */
	  affect_remove(vict, vict->affected, FALSE);
            for(taeller=0; taeller < AF_ARRAY_MAX; taeller++)
      AFF_FLAGS(ch)[taeller] = 0;
	sendChar(vict, "There is a brief flash of light!\r\n"
		     "You feel slightly different.\r\n");
	sendChar(ch, "All spells removed.\r\n");
      } else {
	sendChar(ch, "Your victim does not have any affections!\r\n");
	return;
      }
      break;
    default:
      mlog("SYSERR: Unknown subcmd %d passed to do_wizutil (%s)", subcmd, __FILE__);
      /*  SYSERR_DESC: This is the same as the unhandled case in do_gen_ps(),
       *  but this function handles 'reroll', 'pardon', 'freeze', etc. */
      break;
    }
    save_char(vict, NOWHERE);
  }
}

void show_mobload(CharData *ch, char *arg) 
{
  int arg1,arg2,arg3,zone,subcmd,found,mob,room;
  char temp[8192];
 // sendChar(ch, "mobload: under implementation %s\r\n",arg);

  if (strcmp(arg,"")==0) {
    sendChar(ch, "Usage: show mobload <vnum>\r\n");
    return;
  }
  arg1=arg2=arg3=zone=subcmd=mob=room=0;
  found=0;
  mob=atoi(arg);
  sprintf(temp,"Mob %d is loaded:\r\nMax    Room\r\n", mob);
  sendChar(ch, temp);

  for (zone=0; zone <= top_of_zone_table; zone++) {
   if (zone_table[zone].cmd) 
   for (subcmd=0; zone_table[zone].cmd[subcmd].command!='S'; subcmd++) {
    if (zone_table[zone].cmd[subcmd].command=='M') {
      arg1=mob_index[zone_table[zone].cmd[subcmd].arg1].virtual;
      if (arg1==mob) {
        arg2=zone_table[zone].cmd[subcmd].arg2;
        room=zone_table[zone].cmd[subcmd].arg3;
        arg3=world[room].number;
        sprintf(temp, "%-3d in [%5d] %s\r\n", arg2, arg3, world[room].name); 
        sendChar(ch, temp);
        found=1;
      }
    }
   }
  }
  if (found==0)
    sendChar(ch, "Nowhere!\r\n"); 
}

void show_spelldamgear(CharData *ch, char *value) {
    ObjData *obj;
    int nr, i, j, count = 0;
    int found;
    
    static const char *keywords[] = {
    "N/A",
    "finger",
    "cloak",
    "body",
    "head",
    "legs",
    "feet",
    "hands",
    "arms",
    "shield",
    "about",
    "waist",
    "wrist",
    "wield",
    "hold",
    "neck",
    "orbit",
    "ankles",
    "ears",
    "face"
    "\n"
  };
    
    
    for (i = 1; i < NUM_WEARS; i++) {
        count = 0;
        
        sprintf(buf, "Worn on %s:\r\n", keywords[i]);
        for (nr = 0; nr <= top_of_objt; nr++) {
            found = 0;
            obj = (obj_proto + nr);
            if(!CAN_WEAR(obj, i))
                continue;
            
            for (j = 0; j < MAX_OBJ_AFFECT; j++) {
                if (obj->affected[j].location == APPLY_SPELL_DAMAGE)
                    found += obj->affected[j].modifier;
            }
            if (found) {
                sprintf(buf, "%s%3d. [%5d] {%3d} %s\r\n", buf,                      
                        ++count, GET_OBJ_VNUM(obj), found,
                        obj->short_description);           
            }
        } 
       
        if(count > 0) {
            send_to_char(buf, ch); 
            sendChar(ch, "\r\n");
        }
    }

}

void show_graf(CharData *ch, char *value) {
    int age;
    
    sendChar(ch, "Hit: \r\n");
    for(age = 12; age < 100; age++) {
        sendChar(ch, "%d\r\n", graf(age, 18, 19, 22, 24, 20, 16, 15));
    }
    
    sendChar(ch, "\r\nMana: \r\n");
    for(age = 12; age < 100; age++) {
        sendChar(ch, "%d\r\n", graf(age, 12, 11, 7, 8, 10, 15, 16));
    }
    
    sendChar(ch, "\r\nMove: \r\n");
    for(age = 12; age < 100; age++) {
        sendChar(ch, "%d\r\n", graf(age, 24, 23, 19, 16, 13, 11, 10));
    }   
    
}

void show_trophies(CharData *ch, char *value) {
    ObjData *obj;
    int nr, i, count = 0;
    int found;

    for (nr = 0; nr <= top_of_objt; nr++) {
        obj = (obj_proto + nr);

        if (IS_OBJ_STAT(obj, ITEM_TROPHY)) {
            sprintf(buf, "%3d. [%5d] {%3d} %s\r\n", ++count,
            GET_OBJ_VNUM(obj), GET_OBJ_TIMER(obj),
            obj->short_description);
            send_to_char(buf, ch);
        }
    }

}

void show_weapons(CharData *ch, char *value) {
    ObjData *obj;
    bool found, forgable;
    int nr, i, count = 0, strapp, wpn_avg;

    int oldMaxWeight = 0;
    int newMaxWeight = 0;   
    
    for(strapp = 13; strapp < 30; strapp++) {
        oldMaxWeight = newMaxWeight;
        newMaxWeight = str_app[strapp].wield_w;
        
        sendChar(ch, "\r\nNow searching StrApp: %d (%d - %d)\r\n", strapp, oldMaxWeight, newMaxWeight);
                
        for (nr = 0; nr <= top_of_objt; nr++) {
            obj = (obj_proto + nr);
            forgable = 0;
            
            if (GET_OBJ_TYPE(obj) == ITEM_WEAPON) {
                if(GET_OBJ_WEIGHT(obj) > oldMaxWeight && 
                   GET_OBJ_WEIGHT(obj) <= newMaxWeight) {
                    if(!IS_SET_AR(GET_OBJ_EXTRA(obj), ITEM_MAGIC))
                        forgable = 1;
  
                    if(forgable)
                        wpn_avg = (obj->obj_flags.value[1] + 2)* (((obj->obj_flags.value[2]+1) + 1)/2);
                    else
                        wpn_avg = obj->obj_flags.value[1] * ((obj->obj_flags.value[2] + 1)/2);
                    
                    if(wpn_avg < 25)
                        continue;
                    
                    sprintf(buf, "%3d. [%5d] %s{%3d}&00 {%3d} %s\r\n", ++count, GET_OBJ_VNUM(obj), 
                            forgable ? "&17":"&00", wpn_avg, wpn_avg + str_app[strapp].todam,obj->short_description);
                    send_to_char(buf, ch);
                }
            }
        }
    }
}


void high_restrict(CharData *ch, char *arg) {
    ObjData *obj;
    int nr, i, count = 0;

    for (nr = 0; nr <= top_of_objt; nr++) {
        obj = (obj_proto + nr);

        int found = FALSE;
        int remort = 0, premort = 0;

        if(IS_OBJ_STAT(obj, ITEM_ANTI_MINOTAUR))
            premort += 1;
        if(IS_OBJ_STAT(obj, ITEM_ANTI_GNOME))
            premort += 1;
        if(IS_OBJ_STAT(obj, ITEM_ANTI_ORC))
            premort += 1;
        if(IS_OBJ_STAT(obj, ITEM_ANTI_ELF))
            premort += 1;
        if(IS_OBJ_STAT(obj, ITEM_ANTI_DROW))
            premort += 1;
        if(IS_OBJ_STAT(obj, ITEM_ANTI_DRACONIAN))
            premort += 1;
        if(IS_OBJ_STAT(obj, ITEM_ANTI_HALFLING))
            premort += 1;
        if(IS_OBJ_STAT(obj, ITEM_ANTI_OGRE))
            premort += 1;
        if(IS_OBJ_STAT(obj, ITEM_ANTI_TROLL))
            premort += 1;
        if(IS_OBJ_STAT(obj, ITEM_ANTI_DWARF))
            premort += 1;
        if(IS_OBJ_STAT(obj, ITEM_ANTI_DEMON))
            remort += 1;
        if(IS_OBJ_STAT(obj, ITEM_ANTI_IZARTI))
            remort += 1;
        if(IS_OBJ_STAT(obj, ITEM_ANTI_VAMPIRE))
            remort += 1;
        if(IS_OBJ_STAT(obj, ITEM_ANTI_WEREWOLF))
            remort += 1;
        if(IS_OBJ_STAT(obj, ITEM_ANTI_ELEMENTAL))
            remort += 1;
        if(IS_OBJ_STAT(obj, ITEM_ANTI_GIANT))
            remort += 1;
        if(IS_OBJ_STAT(obj, ITEM_ANTI_FAERIE))
            remort += 1;
        if(IS_OBJ_STAT(obj, ITEM_ANTI_AMARA))
            remort += 1;
        if(IS_OBJ_STAT(obj, ITEM_ANTI_UNDEAD))
            remort += 1;
        if(IS_OBJ_STAT(obj, ITEM_ANTI_HUMAN))
            premort += 1;

        if((remort >= 7 || premort >= 8) && !IS_OBJ_STAT(obj, ITEM_SOULBOUND))
            found = TRUE;
        
        if (found) {
            sprintf(buf, "%3d. [%5d] %s\r\n", ++count,
                    GET_OBJ_VNUM(obj),
                    obj->short_description);
            send_to_char(buf, ch);
        }
    }
}

void show_magicobj(CharData *ch, char *arg) {
    char help[MAX_STRING_LENGTH];
    int i;
    int qend;
    int nr, count = 0;
    int found;
    ObjData *obj;
    int skill;

    if (strcmp(arg,"")==0) {	/* no arguments. print an informative text */
        send_to_char("Syntax: show magicobj '<skill>'\r\n\r\n", ch);
        strcpy(help, "Skill being one of the following:\n\r");
        for (i = 0; *spells[i] != '\n'; i++) {
            if (*spells[i] == '!')
                continue;
            sprintf(help + strlen(help), "%18s", spells[i]);
            if (i % 4 == 3) {
                strcat(help, "\r\n");
                send_to_char(help, ch);
                *help = '\0';
            }
        }
        if (*help)
            send_to_char(help, ch);
        send_to_char("\n\r", ch);
        return;
    }

    /* If there is no chars in argument */
    if (!*arg) {
        send_to_char("Spell name expected.\n\r", ch);
        return;
    }
    if (*arg != '\'') {
        send_to_char("Skill must be enclosed in: ''\n\r", ch);
        return;
    }

    for (qend = 1; *(arg + qend) && (*(arg + qend) != '\''); qend++)
        *(arg + qend) = LOWER(*(arg + qend));
   
    if (*(arg + qend) != '\'') {
        send_to_char("Skill must be enclosed in: ''\n\r", ch);
        return;
    }
    strcpy(help, (arg + 1));
    help[qend - 1] = '\0';
    if ((skill = find_skill_num(help)) <= 0) {
        send_to_char("Unrecognized skill.\n\r", ch);
        return;
    }
    
    for (nr = 0; nr <= top_of_objt; nr++) {
        obj = (obj_proto + nr);
        found = FALSE;

        if((GET_OBJ_TYPE(obj) == ITEM_STAFF ||
               GET_OBJ_TYPE(obj) == ITEM_WAND) &&
                GET_OBJ_VAL(obj, 3) == skill)
            found = TRUE;

        if((GET_OBJ_TYPE(obj) == ITEM_SCROLL ||
                GET_OBJ_TYPE(obj) == ITEM_DUST ||
                GET_OBJ_TYPE(obj) == ITEM_POTION) &&
                (GET_OBJ_VAL(obj, 1) == skill ||
                 GET_OBJ_VAL(obj, 2) == skill ||
                 GET_OBJ_VAL(obj, 3) == skill ))
            found = TRUE;

        if(GET_OBJ_TYPE(obj) == ITEM_FOOD &&
                GET_OBJ_VAL(obj, 2) == skill)
            found = TRUE;

        if (found) {
            sprintf(buf, "%3d. [%5d] %s\r\n", ++count,
                    GET_OBJ_VNUM(obj),
                    obj->short_description);
            send_to_char(buf, ch);
        }
    }
    return;

}

void show_objload(CharData *ch, char *arg)
{
  int arg1,arg2,arg3,zone,subcmd,found,obj,room,mob,mobw;
  char temp[8192],cmd;
  if (strcmp(arg,"")==0) { sendChar(ch, "Usage: show objload <vnum>\r\n");
    return;
  }
  arg1=arg2=arg3=zone=subcmd=obj=room=0;
  found=0;
  obj=atoi(arg);
  sprintf(temp,"Object %d is loaded:\r\n", obj);
  sendChar(ch, temp);
   for (zone=0; zone <= top_of_zone_table; zone++) {
   if (zone_table[zone].cmd)
   for (subcmd=0; zone_table[zone].cmd[subcmd].command!='S'; subcmd++) {
    cmd=zone_table[zone].cmd[subcmd].command;
    arg1=zone_table[zone].cmd[subcmd].arg1;
    arg2=zone_table[zone].cmd[subcmd].arg2;
    arg3=zone_table[zone].cmd[subcmd].arg3;
    switch (cmd) {
      case 'M':
        mob=mob_index[arg1].virtual;
        mobw=world[arg3].number;
        break;
      case 'O':
        if (obj_index[arg1].virtual==obj) {
          sprintf(temp,"in [%-5d] load max %d\r\n", world[arg3].number, arg2);
          sendChar(ch, temp);
          found=1;
        }
        break;
      case 'G':
        if (obj_index[arg1].virtual==obj) {
          sprintf(temp,  "in [%-5d] give mob [%-5d] max %d\r\n", mobw, mob, arg2);
          sendChar(ch,temp);
          found=1;
        }
        break;
      case 'E':
        if (obj_index[arg1].virtual==obj) {
          sprintf(temp, "in [%-5d] equip mob [%-5d] max %d\r\n", mobw, mob, arg2);
          sendChar(ch,temp);
          found=1;
        }
        break;
      case 'P':
        if (obj_index[arg1].virtual==obj) {
          sprintf(temp, "in [-----] put in [%-5d] max %d\r\n", obj_index[arg3].virtual, arg2);
          sendChar(ch,temp);
          found=1;
        }
        break;
      default:
        break;   

    } // end switch
   } // end for subcmd
  }  // end for zone

  if (found==0)
    sendChar(ch, "Nowhere!\r\n");
}

void show_levels(CharData *ch) 
{
   char temp[512];
   int x, count[61], level;
   CharFileU vict;
   for (x=0; x<61; x++)
     count[x]=0;
   fseek(player_fl, 0L, SEEK_SET);

   sendChar(ch, "Level List:\r\n");

   for (x=0; x<= top_of_p_file; x++) {
     fread(&vict, sizeof(vict), 1, player_fl);
     level=vict.level;
     if (level>0 && level<61)
       count[level]++;
    }   
   for (x=1; x<61; x++) {
     strcpy(temp,"#########################################################################################################");
     if (count[x]%2) {
      temp[count[x]/2]='+';
      temp[count[x]/2+1]='\0';
     }
       else temp[count[x]/2]='\0';
     sendChar(ch, "[%2d] %03d %s\r\n", x, count[x], temp);
   }
}

/*
strlocaltime(&(k->player.time.birth))->tm_mon + 1, localtime(&(k->player.time.birth))->tm_mday,
            localtime(&(k->player.time.birth))->tm_year + 1900, buf2, k->player.time.played / 3600,cpy(buf2, (char *) asctime(localtime(&(k->player.time.logon))));
    buf1[10] = buf2[10] = '\0';

    sprintf(buf, "Created: [%d/%d/%d], Last Logon: [%s], Played [%dh %dm], Age [%d]\r\n",
	    localtime(&(k->player.time.birth))->tm_mon + 1, localtime(&(k->player.time.birth))->tm_mday,
            localtime(&(k->player.time.birth))->tm_year + 1900,
 *
 * */

void show_races(CharData *ch, char *arg)
{
    time_t timetemp = time(NULL);
    struct tm *now = localtime(&timetemp);

    //#define AGE(x) ((now->tm_year - x.birth->tm_year)*12 + now->tm_mon - x.birth->tm_mon)
    #define AGE(x) ((now->tm_year - localtime(&vict.last_logon)->tm_year)*12 + now->tm_mon - localtime(&vict.last_logon)->tm_mon)

    char temp[512];
    int x, i, count[NUM_RACES], elementalSub[5], race, srace, class;
    CharFileU vict;
    int low = 0, high = LVL_IMPL, maxAge = 0;
    int showclass = 0;

    // Cut off the word "races"
    half_chop(arg, buf, arg);

    sscanf(arg, "%d-%d", &low, &high);
    if(low != 0 || high != 60)
        half_chop(arg, buf, arg);

    sscanf(arg, "-A%d", &maxAge);
    if(low != 0 || high != 60)
        half_chop(arg, buf, arg);

    showclass = find_class_bitvector(arg);

    sendChar(ch, "Race List:  Usage: 'show races min-max [class abbreviations]'\r\n"
                 "            Characters level %d-%d", low, high, arg);

    if(maxAge)
        sendChar(ch, ", aged less than %d months", maxAge);

    if(showclass)
        sendChar(ch, ", races %s.\r\n\r\n", arg);
    else
        sendChar(ch, ".\r\n");


    for(x=0; x < NUM_RACES; x++)
        count[x]=0;
    for(i=0; i < 5; i++)
        elementalSub[i] = 0;
    
    fseek(player_fl, 0L, SEEK_SET);
    for (x=0; x<= top_of_p_file; x++) {
        fread(&vict, sizeof(vict), 1, player_fl);
        race=vict.race;
        srace=vict.player_specials_saved.sub_race;
        if (race>=0 && race < NUM_RACES && vict.level >= low && vict.level <= high 
                && !(showclass && !(showclass & (1 << vict.class)))
                && !(maxAge && AGE(vict) > maxAge)) {
            count[race]++;
            if(race == RACE_ELEMENTAL)
                elementalSub[srace]++;
        }
    }
    for (x=0; x < NUM_RACES; x++) {

        strcpy(temp,"#########################################################################################################");
        if (count[x]%5) {
            temp[count[x]/5]='+';
            temp[count[x]/5+1]='\0';
        }
        else temp[count[x]/2]='\0';
        sendChar(ch, "[%3s] %03d %s\r\n", race_abbrevs[x], count[x], temp);

        if(x == RACE_ELEMENTAL) {
            for(i = 1; i <= 4; i++){
                strcpy(temp,"****************************************************************");
                temp[elementalSub[i]]='\0';
                sendChar(ch, " - [%3s] %02d %s\r\n", ele_subrace_abbrevs[i], elementalSub[i], temp);
            }
        }
    }

}

void show_classes(CharData *ch, char *arg)
{
    const char *specs[12][3] = {
    {"none", "Arcanist", "Enchanter"}, //Mu
    {"none", "Dark Priest", "Holy Priest"}, //Cl
    {"none", "Trickster", "Brigand"}, //Th
    {"none", "Defender", "Dragon Slayer"}, //Wa
    {"none", "Naturalist", "Hunter"}, //Ra
    {"none", "Bounty Hunter", "Butcher"}, //As
    {"none", "Chi Warrior", "Ancient Dragon"}, //Sl
    {"none", "Dragoon", "Knight Templar"}, //Kn
    {"none", "Knight Errant", "Defiler"}, //Dk
    {"none", "Prestidigitator", "Shade"}, //Sd
    {"none", "Witch Doctor", "Revenant"}, //Nm
    {"none", "none", "none"}};

    char temp[512];
    int x, y, count[NUM_CLASSES][3], class, spec;
    CharFileU vict;
    int low = 0, high = LVL_IMPL;
    sscanf(arg, "%d-%d", &low, &high);

    sendChar(ch, "Class List:  Characters level %d-%d.\r\n", low, high);

    for(x=0; x < NUM_CLASSES; x++)
        for(y = 0; y < 3; y++)
            count[x][y]=0;

    fseek(player_fl, 0L, SEEK_SET);
    for (x=0; x<= top_of_p_file; x++) {
        fread(&vict, sizeof(vict), 1, player_fl);
        class = vict.class;
        spec = vict.player_specials_saved.specialization;
        
        if (class >=0 && class < NUM_RACES && vict.level >= low && vict.level <= high) {
            count[class][0]++;
            if(spec)
                count[class][spec]++;
        }
    }

    for (x=0; x < NUM_CLASSES; x++) {

        strcpy(temp,"#########################################################################################################");
        if(count[x][0]%5) {
            temp[count[x][0]/5]='+';
            temp[count[x][0]/5+1]='\0';
        }
        else temp[count[x][0]/5]='\0';

        sendChar(ch, "\r\n[%3s] %03d %s\r\n", class_abbrevs[x], count[x][0], temp);

        for(y = 1; y <= 2; y++){
            strcpy(temp,"****************************************************************************");
            temp[count[x][y]]='\0';
            sendChar(ch, " - [%14s] %03d %s\r\n", specs[x][y], count[x][y], temp);
        }
    }
}

void show_lockers(CharData *ch, int locker_num)
{
    int x;
    CharFileU vict;
    int found = 0;

    if(locker_num == 0) {
        sendChar(ch, "Please supply a locker number.\r\n");
        return;
    }

    sendChar(ch, "The following people have access to locker #%d:\r\n", locker_num);
    
    fseek(player_fl, 0L, SEEK_SET);
    for (x=0; x<= top_of_p_file; x++) {
        fread(&vict, sizeof(vict), 1, player_fl);

        if (locker_num == vict.player_specials_saved.locker_num) {
            sendChar(ch, "%s\r\n", vict.name);
            found = TRUE;
        }

    }

    if(!found)
        sendChar(ch, "Nobody found!\r\n");
}

/* single zone printing fn used by "show zone" so it's not repeated in the
   code 3 times ... -je, 4/6/93 */
void print_zone_to_buf(CharData *ch, int zone)
{
    sendChar( ch, "%3d %-30.30s Kills: %3d; Age: %3d; Reset: %3d (%1d); Top: %5d\r\n",
                   zone_table[zone].number, zone_table[zone].name, zone_table[zone].kill_count,
                   zone_table[zone].age, zone_table[zone].lifespan,
                   zone_table[zone].reset_mode, zone_table[zone].top);
}

void printVector(CharData *ch, int array[100])
{
  int x,count;

  count=0;

  for (x=0; x<100; x++) {
    if (array[x]==0)
      sendChar(ch,"%03d ", x);
    else
      sendChar(ch, "### ");
    count++;
    if ((count%10)==0)
      sendChar(ch,"\r\n");
  }
}

void show_FreeRoom(CharData *ch, int qzone)
{
  int i,room, zone;
  int data[100];
  
  for (i=0; i<100; i++)
    data[i]=0;

  for (i=0; i<=top_of_world; i++) {
    room=world[i].number % 100;
    zone=world[i].number/100;
    if (qzone==zone)
      data[room]=1;
  }

  sendChar(ch, "Free rooms in zone %d\r\n\r\n", qzone);
  printVector(ch, data);
}

void show_FreeObj(CharData *ch, int qzone)
{
  sendChar(ch, "Under development\r\n");
/*  int i,obj, zone;
  int data[100];  

  for (i=0; i<100; i++)
    data[i]=0;

  for (i=0; i<=top_of_objt; i++) {
    obj=object_list[i].number % 100;
    zone=object_list[i].number/100;
    if (qzone==zone)
      data[room]=1;   
  }

  sendChar(ch, "Free objects in zone %d\r\n\r\n", qzone);
  printVector(ch, data); */
}

void leak_room(CharData *ch,int vnum)
{
  int door, room;
  *buf='\0';
  *buf2='\0';

  for (room=0; room<=top_of_world; room++)
    for (door=0; door<NUM_OF_DIRS; door++)
      if (world[room].dir_option[door])
        if (world[world[room].dir_option[door]->to_room].number == vnum &&
          ((vnum /100 )!=(world[room].number /100))) {
          sprintf(buf2, "%5d ", world[room].number);
          strcat(buf,buf2);
        }
  if ((*buf)) {
    sprintf(buf2, "\r\n%5d: ", vnum);
    sendChar(ch, buf2);
    sendChar(ch, buf);
  }
}



void show_leaks(CharData *ch, int qzone) {
  int i,room, zone;

  sendChar(ch, "Leaks in zone %d\r\n", qzone);
  for (i=0; i<=top_of_world; i++) {
    room=world[i].number % 100;
    zone=world[i].number/100;
    if (qzone==zone)
      leak_room(ch,world[i].number);
  }

}

void show_alts(CharData *ch, char *name)
{
    CharFileU victim, index;
    CharData *temp;
    extern FILE *player_fl;

    if (load_char(name, &victim) == -1) {
        send_to_char("No such player.\r\n", ch);
        return;
    } else {
        sprintf(buf, "Showing alts for [%s]\n", victim.host);
        send_to_char(buf, ch);
        fseek(player_fl, 0L, SEEK_SET);
        while (!feof(player_fl)) {
            fread(&index, sizeof(CharFileU), 1, player_fl);
            if (!feof(player_fl)) {
                if (!strcmp(index.host, victim.host)) {
                    sprintf(buf, "%s[ %2d %s %s ] %s %s\n",
                            (temp = get_char_vis(ch, index.name, 1)) && GET_IDNUM(temp) == index.char_specials_saved.idnum ? "&09*&00" : " ",
                            index.level, class_abbrevs[index.class],
                            race_abbrevs[index.race], index.name, index.title);
                    send_to_char(buf, ch);
                }
            }
        }
    }
}

void show_quests(CharData *ch, char *name)
{
  FILE *f;
  char fname[100];
  time_t wait;
  int count, i;
  int quest, real;

  if (!get_filename(name, fname, QUESTS_FILE)) {
    send_to_char("Can't get quest filename.\r\n", ch);
    return;
  }
  if ((f = fopen(fname, "rb")) == NULL) {
    send_to_char("No quest file.\r\n", ch);
    return;
  }
  fread(&wait, sizeof(time_t), 1, f);
  fread(&count, sizeof(int), 1, f);
  sprintf(buf, "Quest file: %s\n%d quests completed:\r\n", fname, count);
  send_to_char(buf, ch);
  for (i = 1; i <= count; i++) {
    fread(&quest, sizeof(int), 1, f);
    real = real_quest(quest);
    if (real == -1) {
      sprintf(buf, "%3d: Unknown quest #%d\r\n", i, quest);
    } else {
      sprintf(buf, "%3d: #%d, %s\r\n", i, quest, qst_list[real].name);
    }
    send_to_char(buf, ch);
  }
}



ACMD(do_show)
{
  struct char_file_u vbuf;
  int i, j, k, l, con;
  char self = 0;
  CharData *vict;
  ObjData *obj;
  char field[40], value[40], birth[80];
  extern int buf_switches, buf_largecount, buf_overflows;
  void show_shops(CharData * ch, char *value);
  char big_buf[32*1024];

    struct show_struct {
        char *cmd;
        char level;
    } fields[] = { { "nothing", 0  },
        { "zones",    LVL_IMMORT    }, { "player",   LVL_LRGOD     },
        { "rent",     LVL_LRGOD     }, { "stats",    LVL_IMPL      },
        { "errors",   LVL_IMPL      }, { "death",    LVL_LRGOD     },
        { "godrooms", LVL_LRGOD     }, { "shops",    LVL_IMMORT    },
        { "kills",    LVL_IMMORT    }, { "mobload",  LVL_IMMORT    },
        { "objload",  LVL_IMMORT    }, { "levels",   LVL_IMMORT    },
        { "freeroom", LVL_IMMORT    }, { "freeobj",  LVL_IMMORT    },
        { "leaks",    LVL_IMMORT    }, { "underwater", LVL_IMMORT  },
        { "portals",  LVL_IMMORT    }, { "traps",    LVL_IMMORT    },
	{ "hot",      LVL_IMMORT    }, { "cold",     LVL_IMMORT    },
        { "dry",      LVL_IMMORT    }, { "quests",   LVL_IMMORT    },
        { "alts",     LVL_IMMORT    }, { "returns",  LVL_IMMORT    },
        { "races",    LVL_LRGOD     }, { "classes",  LVL_LRGOD     },
        { "lockers",  LVL_CREATOR   }, { "magicobj", LVL_IMMORT    },
        { "spelldam", LVL_IMMORT    }, { "highrestrict", LVL_IMMORT },
        { "trophy",   LVL_IMMORT    }, { "weapon",   LVL_IMMORT    },
        { "graf",     LVL_IMMORT    },
	{ "\n", 0 }
    };

    if( IS_NPC(ch) ) return;

    skip_spaces(&argument);

    if( !*argument ){
        sendChar( ch, "Show options:\r\n");
        for( j = 0, i = 1; fields[i].level; i++ )
            if( fields[i].level <= GET_LEVEL(ch) )
                sendChar( ch, "%-14s%s", fields[i].cmd, (!(++j % 4) ? "\r\n" : ""));
        sendChar( ch, "\r\n" );
        return;
    }

    strcpy( arg, two_arguments( argument, field, value ));

    for( l = 0; *(fields[l].cmd) != '\n'; l++ )
        if( !strncmp(field, fields[l].cmd, strlen(field) )) break;

    if (GET_LEVEL(ch) < fields[l].level) {
        sendChar( ch, "You are not godly enough for that!\r\n" );
        return;
    }

    if( !strcmp(value, ".") ) self = 1;

    buf[0] = '\0';

    switch (l) {
        case 1:    /* zone */
            if( self ){
                print_zone_to_buf(ch, world[ch->in_room].zone);
            }
            else if( *value && is_number(value) ){
                for (j = atoi(value), i = 0; zone_table[i].number != j && i <= top_of_zone_table; i++);
                if (i <= top_of_zone_table){
	            print_zone_to_buf(ch, i);
                }
                else {
	            sendChar( ch, "That is not a valid zone.\r\n" );
	            return;
                }
            } else
#ifdef SCROLL_UNTIL_WE_PUKE
                for (i = 0; i <= top_of_zone_table; i++){
  	          print_zone_to_buf(ch, i);
                }
#else
		strcpy(big_buf,"\r\nRaven Zones:\r\n\r\n");
		for (i = 0; i <= top_of_zone_table; i++){
		   sprintf(((big_buf) + strlen(big_buf)), "%3d %-30.30s KC: %3d; Age: %3d; Reset: %3d (%1d); Top: %5d\r\n",
                   zone_table[i].number, zone_table[i].name, zone_table[i].kill_count,
                   zone_table[i].age, zone_table[i].lifespan,
                   zone_table[i].reset_mode, zone_table[i].top);
		}
		page_string(ch->desc, big_buf, 1);
#endif
            break;

  case 2:			/* player */
    if (load_char(value, &vbuf) < 0) {
      send_to_char("There is no such player.\r\n", ch);
      return;
    }
    sprintf(buf, "Player: %-12s (%s) [%2d %s]\r\n", vbuf.name,
      genders[(int) vbuf.sex], vbuf.level, class_abbrevs[(int) vbuf.class]);
    sprintf(buf,
	 "%sAu: %-8d  Bal: %-8d  Exp: %-8d  Align: %-5d  Lessons: %-3d\r\n",
	    buf, vbuf.points.gold, vbuf.points.bank_gold, vbuf.points.exp,
	    vbuf.char_specials_saved.alignment,
	    vbuf.player_specials_saved.spells_to_learn);
    strcpy(birth, ctime(&vbuf.birth));
    sprintf(buf,
	    "%sStarted: %-20.16s  Last: %-20.16s  Played: %3dh %2dm\r\n",
	    buf, birth, ctime(&vbuf.last_logon), (int) (vbuf.played / 3600),
	    (int) (vbuf.played / 60 % 60));
    send_to_char(buf, ch);
    break;
  case 3:
    Crash_listrent(ch, value);
    break;
  case 4:
    i = 0;
    j = 0;
    k = 0;
    con = 0;
    for (vict = character_list; vict; vict = vict->next) {
      if (IS_NPC(vict))
	j++;
      else if (CAN_SEE(ch, vict)) {
	i++;
	if (vict->desc)
	  con++;
      }
    }
    for (obj = object_list; obj; obj = obj->next)
      k++;
    sprintf(buf, "Current stats:\r\n");
    sprintf(buf, "%s  %5d players in game  %5d connected\r\n", buf, i, con);
    sprintf(buf, "%s  %5d registered\r\n", buf, top_of_p_table + 1);
    sprintf(buf, "%s  %5d mobiles          %5d prototypes\r\n",
	    buf, j, top_of_mobt + 1);
    sprintf(buf, "%s  %5d objects          %5d prototypes\r\n",
	    buf, k, top_of_objt + 1);
    sprintf(buf, "%s  %5d rooms            %5d zones\r\n",
	    buf, top_of_world + 1, top_of_zone_table + 1);
    sprintf(buf, "%s  %5d large bufs\r\n", buf, buf_largecount);
    sprintf(buf, "%s  %5d buf switches     %5d overflows\r\n", buf,
	    buf_switches, buf_overflows);
    send_to_char(buf, ch);
    break;
  case 5:
    strcpy(buf, "Errant Rooms\r\n------------\r\n");
    for (i = 0, k = 0; i <= top_of_world; i++)
      for (j = 0; j < NUM_OF_DIRS; j++)
	if (world[i].dir_option[j] && world[i].dir_option[j]->to_room == 0)
	  sprintf(buf, "%s%2d: [%5d] %s\r\n", buf, ++k, world[i].number,
		  world[i].name);
    send_to_char(buf, ch);

  case 6:
    strcpy(buf, "Death Traps\r\n-----------\r\n");
    for (i = 0, j = 0; i <= top_of_world; i++)
      if (IS_SET_AR(ROOM_FLAGS(i), ROOM_DEATH))
	sprintf(buf, "%s%2d: [%5d] %s\r\n", buf, ++j,
		world[i].number, world[i].name);
    send_to_char(buf, ch);
    break;
  case 7:
#define GOD_ROOMS_ZONE 2
    strcpy(buf, "Godrooms\r\n--------------------------\r\n");
    for (i = 0, j = 0; i < top_of_world; i++)
      if (world[i].zone == GOD_ROOMS_ZONE)
	sprintf(buf, "%s%2d: [%5d] %s\r\n", buf, j++, world[i].number,
		world[i].name);
    send_to_char(buf, ch);
    break;
  case 8:
    show_shops(ch, value);
    break;
  case 9:
    for( i = 0; i <= top_of_zone_table; i++ )
        if( zone_table[i].kill_count != 0 )
            print_zone_to_buf(ch, i);
    send_to_char(buf, ch);
    break;
  case 10:
    show_mobload(ch, value);
    break;
  case 11:
    show_objload(ch, value);
    break;
  case 12:
    show_levels(ch);
    break;
  case 13:
    show_FreeRoom(ch, atoi(value));
    break;
  case 14:
    show_FreeObj(ch, atoi(value)); 
    break;
  case 15:
    show_leaks(ch, atoi(value));
    break;
  case 16:
    /*strcpy(buf, "Underwater Rooms\r\n-------------------------\r\n");
    for (i = 0, j =0; i < top_of_world; i++)
         if (SECT(i) == SECT_UNDERWATER)
             sprintf(buf + strlen(buf), "%2d: [%5d] %s\r\n",
                     ++j, world[i].number, world[i].name);
    page_string(ch->desc, buf, TRUE);*/
    sendChar(ch, "This crashes us, so don't use it, mmk?\r\n");
    break;
  case 17:
    strcpy(buf, "Portal Objects\r\n-------------------------\r\n");
    for (i = 0, j = 0; i < top_of_objt; i++)
         if (obj_proto[i].obj_flags.type_flag == ITEM_PORTAL)
             sprintf(buf + strlen(buf), "%2d: [%5d] %s\r\n",
	       ++j, obj_index[i].virtual, obj_proto[i].short_description);
    page_string(ch->desc, buf, TRUE);
    break;
  case 18:
    strcpy(buf, "Trap Objects\r\n-------------------------\r\n");
    //for (i = 0, j = 0; i < top_of_objt; i++)
         //if (obj_proto[i].obj_flags.type_flag == ITEM_TRAP)
             //sprintf(buf + strlen(buf), "%2d: [%5d] %s\r\n",
               //++j, obj_index[i].virtual, obj_proto[i].short_description);
    page_string(ch->desc, buf, TRUE);
    break;                                                      
  case 19:
    strcpy(buf, "Suffer Rooms (HOT)\r\n-------------------------\r\n");
    for (i = 0, j = 0; i <= top_of_world; i++)
      if (IS_SET_AR(ROOM_FLAGS(i), ROOM_HOT))
        sprintf(buf, "%s%2d: [%5d] %s\r\n", buf, ++j,
                world[i].number, world[i].name);
    send_to_char(buf, ch);
    break;
  case 20:
    strcpy(buf, "Suffer Rooms (COLD)\r\n-------------------------\r\n");
    for (i = 0, j = 0; i <= top_of_world; i++)
      if (IS_SET_AR(ROOM_FLAGS(i), ROOM_COLD))
        sprintf(buf, "%s%2d: [%5d] %s\r\n", buf, ++j,
                world[i].number, world[i].name);
    send_to_char(buf, ch);
    break;
  case 21:
    strcpy(buf, "Suffer Rooms (DRY)\r\n-------------------------\r\n");
    for (i = 0, j = 0; i <= top_of_world; i++)
      if (IS_SET_AR(ROOM_FLAGS(i), ROOM_DRY))
        sprintf(buf, "%s%2d: [%5d] %s\r\n", buf, ++j,
                world[i].number, world[i].name);
    send_to_char(buf, ch);
    break;
  case 22:
    show_quests(ch, value);
    break;
  case 23:
    show_alts(ch, value);
    break;
  case 24:
    show_kills(ch, value);
    break;
  case 25:
    show_races(ch, argument);
    break;
  case 26:
    show_classes(ch, value);
    break;
  case 27:
    show_lockers(ch, atoi(value));
    break;
  case 28: /* "magicobj" */
    show_magicobj(ch, value);
    break;
  case 29:
    show_spelldamgear(ch, value);
    break;
  case 30:
    high_restrict(ch, value);
    break;
  case 31:
    show_trophies(ch, value);
    break;
  case 32:
    show_weapons(ch, value);
    break;  
  case 33:
    show_graf(ch, value);
    break;
  default:
    send_to_char("Sorry, I don't understand that.\r\n", ch);
    break;
  }
}


#define PC   1
#define NPC  2
#define BOTH 3

#define MISC	0
#define BINARY	1
#define NUMBER	2

/* 128 Bit
#define SET_OR_REMOVE(flagset, flags) { \
	if (on) SET_BIT(flagset, flags); \
	else if (off) REMOVE_BIT(flagset, flags); }
*/
#define SET_OR_REMOVE(flagset, flags) { \
	if (on) SET_BIT_AR(flagset, flags); \
        else if (off) REMOVE_BIT_AR(flagset, flags); }

#define RANGE(low, high) (value = MAX((low), MIN((high), (value))))

ACMD(do_set)
{
    int i, l;
    CharData *vict;
    CharData *cbuf;
    struct char_file_u tmp_store;
    char field[MAX_INPUT_LENGTH], name[MAX_INPUT_LENGTH], val_arg[MAX_INPUT_LENGTH];
    int on = 0, off = 0, value = 0;
    char is_file = 0, is_mob = 0, is_player = 0;
    int player_i;
    CharData * find_char(int n);

    struct set_struct {
        char *cmd;
        char level;
        char pcnpc;
        char type;
    } fields[] = {
/*00*/ { "brief",    LVL_GOD,         PC,   BINARY },
       { "invstart", LVL_SAINT,         PC,   BINARY },
       { "title",    LVL_SAINT,     PC,   MISC },
       { "nosummon", LVL_GOD,         PC,   BINARY },
       { "maxhit",   LVL_GRGOD , BOTH, NUMBER },
/*05*/ { "maxmana",  LVL_GRGOD , BOTH, NUMBER },
       { "maxmove",  LVL_GRGOD , BOTH, NUMBER },
       { "hit",      LVL_GRGOD,  BOTH, NUMBER },
       { "mana",     LVL_GRGOD,  BOTH, NUMBER },
       { "move",     LVL_GRGOD,  BOTH, NUMBER },
/*10*/ { "align",    LVL_GRGOD,  BOTH, NUMBER },
       { "str",      LVL_GRGOD , BOTH, NUMBER },
       { "stradd",   LVL_GRGOD , BOTH, NUMBER },
       { "int",      LVL_GRGOD , BOTH, NUMBER },
       { "wis",      LVL_GRGOD , BOTH, NUMBER },
/*15*/ { "dex",      LVL_GRGOD , BOTH, NUMBER },
       { "con",      LVL_GRGOD , BOTH, NUMBER },
       { "sex",      LVL_GRGOD , BOTH, MISC },
       { "ac",       LVL_GRGOD,  BOTH, NUMBER },
       { "gold",     LVL_GOD,  BOTH, NUMBER },
/*20*/ { "bank",     LVL_GRGOD,  PC,   NUMBER },
       { "exp",      LVL_LRGOD , BOTH, NUMBER },
       { "hitroll",  LVL_LRGOD,   BOTH, NUMBER },
       { "damroll",  LVL_LRGOD,   BOTH, NUMBER },
       { "invis",    LVL_IMPL, PC,   NUMBER },
/*25*/ { "nohassle", LVL_GRGOD,  PC,   BINARY },
       { "frozen",   LVL_FREEZE,  PC,   BINARY },
       { "practices",LVL_GRGOD,  PC,   NUMBER },
       { "lessons",  LVL_GRGOD,  PC,   NUMBER },
       { "drunk",    LVL_GRGOD,  BOTH, MISC },
/*30*/ { "hunger",   LVL_GRGOD,  BOTH, MISC },
       { "thirst",   LVL_GRGOD,  BOTH, MISC },
       { "killer",   LVL_LRGOD,   PC,   BINARY },
       { "thief",    LVL_LRGOD,   PC,   BINARY },
       { "level",    LVL_GRGOD , BOTH, NUMBER },
/*35*/ { "room",     LVL_GRGOD , BOTH, NUMBER },
       { "roomflag", LVL_GRGOD,  PC,   BINARY },
       { "siteok",   LVL_GRGOD,  PC,   BINARY },
       { "deleted",  LVL_IMPL, PC,   BINARY },
       { "class",    LVL_GRGOD,  BOTH, MISC },
/*40*/ { "nowizlist",LVL_GOD,         PC,   BINARY },
       { "quest",    LVL_LRGOD,   PC,   BINARY },
       { "loadroom", LVL_LRGOD,   PC,   MISC },
       { "color",    LVL_GOD,         PC,   BINARY },
       { "idnum",    LVL_IMPL, PC,   NUMBER },
/*45*/ { "passwd",   LVL_IMPL, PC,   MISC },
       { "nodelete", LVL_GOD,         PC,   BINARY },
       { "cha",      LVL_GRGOD,  BOTH, NUMBER },
       { "shun",     LVL_DEITY,   	  PC,   BINARY },
       { "clan",     LVL_LRGOD,   PC,   NUMBER },
/*50*/ { "clanlvl",  LVL_LRGOD,   PC,   NUMBER },
       { "plague",   LVL_IMPL, BOTH, NUMBER },
       { "rmc",      LVL_LRGOD,   BOTH, BINARY },
       { "olc",      LVL_DGOD,     PC,   NUMBER },
       { "team",     LVL_SAINT,       PC,   MISC   },
/*55*/ { "hunted",   LVL_LRGOD,   PC,   BINARY },
       { "race",     LVL_GRGOD,  BOTH, MISC },
       { "subrace",  LVL_GRGOD,  BOTH, NUMBER },
       { "qp",       LVL_CREATOR,     PC,   NUMBER },
       { "reward",   LVL_GOD,         PC,   NUMBER },
/*60*/ { "rawlog",   LVL_LRGOD,   PC,   BINARY },
       { "reimbursed", LVL_IMPL, PC, BINARY },
       { "ageadd",   LVL_CREATOR,     PC, NUMBER},
       { "retired",  LVL_IMPL, PC,   MISC },
       { "name",     LVL_IMPL, PC,   MISC },
/*65*/ { "nocomm",   LVL_DGOD,     PC,   MISC },
       { "locker",   LVL_CREATOR,     PC,   NUMBER },
       { "grief",    LVL_DGOD,     PC,   BINARY },
       { "orcs",     LVL_GRGOD,  PC,   NUMBER },
       { "afk",      LVL_BUILDER, PC,	BINARY },
/*70*/ { "advlevel", LVL_IMPL,    PC,   NUMBER },
       { "spec",     LVL_IMPL,    PC,   NUMBER },
       { "\n",       0,           BOTH, MISC }
    };
#define NUM_SET_FIELDS 72 /* 0 is 1, 1 is 2, etc. keep this up to date. */

  half_chop(argument, name, buf);
  if (!strcmp(name, "file")) {
    is_file = 1;
    half_chop(buf, name, buf);
  } else if (!str_cmp(name, "player")) {
    is_player = 1;
    half_chop(buf, name, buf);
  } else if (!str_cmp(name, "mob")) {
    is_mob = 1;
    half_chop(buf, name, buf);
  }
  half_chop(buf, field, buf);
  strcpy(val_arg, buf);

  if (!*name || !*field) { /* show all the fields. */
    int i, cnt;

    sprintf(buf1, "\r\n"); /* Required to ensure buf in right state. */
    sprintf(buf1, "[H[J");
    sprintf(((buf1) + strlen(buf1)), "You can set the following:\r\n");
    for (i = 0, cnt = 0; i < NUM_SET_FIELDS; i++) {
	if (GET_LEVEL(ch) >= fields[i].level) {
	    /* Prepare type buffer */
	    switch(fields[i].pcnpc) {
	    case PC: sprintf(buf2, "PC  "); break;
	    case NPC: sprintf(buf2, "NPC "); break;
	    case BOTH: sprintf(buf2, "BOTH"); break;
	    default: sprintf(buf2, "UNKN");
	    }
	    switch(fields[i].type) {
	    case MISC: sprintf(((buf2) + strlen(buf2)), " MISC"); break;
	    case BINARY: sprintf(((buf2) + strlen(buf2)), " BIN "); break;
	    case NUMBER: sprintf(((buf2) + strlen(buf2)), " NUM "); break;
	    default: sprintf(((buf2) + strlen(buf2)), " UNKN");
	    }
	    cnt++;
            sprintf(((buf1) + strlen(buf1)), "[&06%3d&00] &06%15s %8s&00%s", 
	            fields[i].level, fields[i].cmd, buf2,
		    (cnt % 2 ? "  &08|&00  " : "\r\n"));
	}
    } /* for loop */
    sprintf(((buf1) + strlen(buf1)), "\r\n");
    /* Have to make prompt here or it will be screwed once we go over one page. */
    sprintf(((buf1) + strlen(buf1)), "Usage: set <victim> <field> <value>\r\n");
    page_string(ch->desc, buf1, 1);
    return;
  }
  if (IS_NPC(ch)) {
    send_to_char("None of that!\r\n", ch);
    return;
  }
  if (!is_file) {
    if (is_player) {
      if (!(vict = get_player_vis(ch, name))) {
	send_to_char("There is no such player.\r\n", ch);
	return;
      }
    } else {
      if (!(vict = get_char_vis(ch, name, 0))) {
	send_to_char("There is no such creature.\r\n", ch);
	return;
      }
    }
  } else if (is_file) {
    CREATE(cbuf, CharData, 1);
    clear_char(cbuf);
    if ((player_i = load_char(name, &tmp_store)) > -1) {
      store_to_char(&tmp_store, cbuf);
      if (GET_LEVEL(cbuf) >= GET_LEVEL(ch) && GET_LEVEL(ch) != LVL_IMPL) {
	free_char(cbuf);
	send_to_char("Sorry, you can't do that.\r\n", ch);
	return;
      }
      vict = cbuf;
    } else {
      free(cbuf);
      send_to_char("There is no such player.\r\n", ch);
      return;
    }
  }
  if (GET_LEVEL(ch) != LVL_IMPL) {
    if (!IS_NPC(vict) && GET_LEVEL(ch) <= GET_LEVEL(vict) && vict != ch) {
      send_to_char("Maybe that's not such a great idea...\r\n", ch);
      return;
    }
  }
  for (l = 0; *(fields[l].cmd) != '\n'; l++)
    if (!strncmp(field, fields[l].cmd, strlen(field)))
      break;

  if (GET_LEVEL(ch) < fields[l].level)
  {
    send_to_char("You are not godly enough for that!\r\n", ch);
    return;
  }
  if (IS_NPC(vict) && (!fields[l].pcnpc && NPC)) {
    send_to_char("You can't do that to a beast!\r\n", ch);
    return;
  } else if (!IS_NPC(vict) && (!fields[l].pcnpc && PC)) {
    send_to_char("That can only be done to a beast!\r\n", ch);
    return;
  }
  if (fields[l].type == BINARY) {
    if (!strcmp(val_arg, "on") || !strcmp(val_arg, "yes"))
      on = 1;
    else if (!strcmp(val_arg, "off") || !strcmp(val_arg, "no"))
      off = 1;
    if (!(on || off)) {
      send_to_char("Value must be on or off.\r\n", ch);
      return;
    }
  } else if (fields[l].type == NUMBER) {
    value = atoi(val_arg);
  }
  strcpy(buf, "Okay.");
  switch (l) {
  case 0: SET_OR_REMOVE(PRF_FLAGS(vict), PRF_BRIEF); break;
  case 1: SET_OR_REMOVE(PLR_FLAGS(vict), PLR_INVSTART); break;
  case 2:
    if (strlen(val_arg) > MAX_TITLE_LENGTH) { 
           send_to_char("Sorry, titles cannot be that long, or it corrupts the players file!\r\n", ch);
           return;
	}
       else 
	  set_title(vict, val_arg);
          sprintf(buf, "%s's title is now: %s", GET_NAME(vict), GET_TITLE(vict));
          break;
  case 3: SET_OR_REMOVE(PRF_FLAGS(vict), PRF_SUMMONABLE);
          on = !on;			/* so output will be correct */
          break;
  case 4: vict->points.max_hit = RANGE(1, 30000);
          affect_total(vict);
          break;
  case 5: vict->points.max_mana = RANGE(1, 30000);
          affect_total(vict);
          break;
  case 6: vict->points.max_move = RANGE(1, 30000);
          affect_total(vict);
          break;
  case 7: vict->points.hit = RANGE(-9, vict->points.max_hit);
          affect_total(vict);
          break;
  case 8: vict->points.mana = RANGE(0, vict->points.max_mana);
          affect_total(vict);
          break;
  case 9: vict->points.move = RANGE(0, vict->points.max_move);
          affect_total(vict);
          break;
  case 10: GET_ALIGNMENT(vict) = RANGE(-1000, 1000);
           affect_total(vict);
           break;
  case 11:
    if (GET_LEVEL(vict) >= LVL_GRGOD)
      RANGE(1, 25);
    else
      RANGE(1, race_stat_limits[(int)GET_RACE(vict)][STRENGTH_INDEX]);
    vict->real_abils.str = value;
    if (value > 18)
         vict->real_abils.str_add = 100;
    else
	vict->real_abils.str_add = 0;
    affect_total(vict);
    break;
  case 12:
    vict->real_abils.str_add = RANGE(0, race_stat_limits[(int)GET_RACE(vict)][STRENGTH_ADD_INDEX]);
    if ((value > 0) && (vict->real_abils.str < 18))
      vict->real_abils.str = 18;
    affect_total(vict);
    break;
  case 13:
    if (GET_LEVEL(vict) >= LVL_GRGOD)
      RANGE(1, 25);
    else
      RANGE(1, race_stat_limits[(int)GET_RACE(vict)][INTELLIGENCE_INDEX]);
    vict->real_abils.intel = value;
    affect_total(vict);
    break;
  case 14:
    if (GET_LEVEL(vict) >= LVL_GRGOD)
      RANGE(1, 25);
    else
      RANGE(1, race_stat_limits[(int)GET_RACE(vict)][WISDOM_INDEX]);
    vict->real_abils.wis = value;
    affect_total(vict);
    break;
  case 15:
    if (GET_LEVEL(vict) >= LVL_GRGOD)
      RANGE(1, 25);
    else
      RANGE(1, race_stat_limits[(int)GET_RACE(vict)][DEXTERITY_INDEX]);
    vict->real_abils.dex = value;
    affect_total(vict);
    break;
  case 16:
    if (GET_LEVEL(vict) >= LVL_GRGOD)
      RANGE(1, 25);
    else
      RANGE(1, race_stat_limits[(int)GET_RACE(vict)][CONSTITUTION_INDEX]);
    vict->real_abils.con = value;
    affect_total(vict);
    break;
  case 17:
    if (!str_cmp(val_arg, "male"))
      vict->player.sex = SEX_MALE;
    else if (!str_cmp(val_arg, "female"))
      vict->player.sex = SEX_FEMALE;
    else if (!str_cmp(val_arg, "neutral"))
      vict->player.sex = SEX_NEUTRAL;
    else {
      send_to_char("Must be 'male', 'female', or 'neutral'.\r\n", ch);
      return;
    }
    break;
  case 18:
    vict->points.armor = RANGE(-100, 100);
    affect_total(vict);
    break;
  case 19:
    GET_GOLD(vict) = RANGE(0, 100000000);
    break;
  case 20:
    GET_BANK_GOLD(vict) = RANGE(0, 100000000);
    break;
  case 21:
    vict->points.exp = RANGE(0, 1500000000);
    break;
  case 22:
    vict->points.hitroll = RANGE(-20, 20);
    affect_total(vict);
    break;
  case 23:
    SET_DAMROLL(vict, RANGE(-20, 100));
    affect_total(vict);
    break;
  case 24:
    if (GET_LEVEL(ch) < LVL_IMPL && ch != vict) {
      send_to_char("You aren't godly enough for that!\r\n", ch);
      return;
    }
    GET_INVIS_LEV(vict) = RANGE(0, GET_LEVEL(vict));
    break;
  case 25:
    if (GET_LEVEL(ch) < LVL_IMPL && ch != vict) {
      send_to_char("You aren't godly enough for that!\r\n", ch);
      return;
    }
    SET_OR_REMOVE(PRF_FLAGS(vict), PRF_NOHASSLE);
    break;
  case 26:
    if (ch == vict) {
      send_to_char("Better not -- could be a long winter!\r\n", ch);
      return;
    }
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_FROZEN);
    break;
  case 27:
  case 28: GET_PRACTICES(vict) = RANGE(0, 100); break;
  case 29:
  case 30:
  case 31:
    if (!str_cmp(val_arg, "off")) {
      GET_COND(vict, (l - 29)) = (char) -1;
      sprintf(buf, "%s's %s now off.", GET_NAME(vict), fields[l].cmd);
    } else if (is_number(val_arg)) {
      value = atoi(val_arg);
      RANGE(0, 24);
      GET_COND(vict, (l - 29)) = (char) value;
      sprintf(buf, "%s's %s set to %d.", GET_NAME(vict), fields[l].cmd,
	      value);
    } else {
      send_to_char("Must be 'off' or a value from 0 to 24.\r\n", ch);
      return;
    }
    break;
  case 32: 
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_KILLER); 
    if (IS_SET_AR(PLR_FLAGS(vict), PLR_KILLER))
	set_killer_player(vict, KILLER_TIME);
    else
	unset_killer_player(vict);
    break;
  case 33:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_THIEF);
    if (IS_SET_AR(PLR_FLAGS(vict), PLR_THIEF))
	set_thief_player(vict, THIEF_TIME);
    else
	unset_thief_player(vict);
    break;
  case 34:
    if (value > GET_LEVEL(ch) || value > LVL_IMPL) {
      send_to_char("You can't do that.\r\n", ch);
      return;
    }
    RANGE(0, LVL_IMPL);
    vict->player.level = (byte) value;
    break;
  case 35:
    if ((i = real_room(value)) < 0) {
      send_to_char("No room exists with that number.\r\n", ch);
      return;
    }
    char_from_room(vict);
    char_to_room(vict, i);
    break;
  case 36: SET_OR_REMOVE(PRF_FLAGS(vict), PRF_SHOWVNUMS); break;
  case 37: SET_OR_REMOVE(PLR_FLAGS(vict), PLR_SITEOK); break;
  case 38: SET_OR_REMOVE(PLR_FLAGS(vict), PLR_DELETED); break;
  case 39:
      if ((i = parse_class(*val_arg)) == CLASS_UNDEFINED) {
        sendChar(ch, "That is not a class.\r\n");
        return;
      }
      GET_CLASS(vict) = i;
    break;
  case 40: SET_OR_REMOVE(PLR_FLAGS(vict), PLR_NOWIZLIST); break;
  case 41: SET_OR_REMOVE(PRF_FLAGS(vict), PRF_QUEST); break;
  case 42:
    if (!str_cmp(val_arg, "on"))
        SET_BIT_AR(PLR_FLAGS(vict), PLR_LOADROOM);
    else if (!str_cmp(val_arg, "off"))
	REMOVE_BIT_AR(PLR_FLAGS(vict), PLR_LOADROOM);
    else {
      if (real_room(i = atoi(val_arg)) > -1) {
	GET_LOADROOM(vict) = i;
	sprintf(buf, "%s will enter at %d.", GET_NAME(vict),
		GET_LOADROOM(vict));
      } else
	sprintf(buf, "That room does not exist!");
    }
    break;
  case 43:
    SET_OR_REMOVE(PRF_FLAGS(vict), PRF_COLOR_1);
    SET_OR_REMOVE(PRF_FLAGS(vict), PRF_COLOR_2);
  case 44:
    if (GET_IDNUM(ch) != 1 || !IS_NPC(vict))
      return;
    GET_IDNUM(vict) = value;
    break;
  case 45:
    if (!is_file)
      return;
/*
    if (GET_IDNUM(ch) > 1) {
     
      send_to_char("Please don't use this command, yet.\r\n", ch);
      return;
    }
*/
    if (GET_LEVEL(vict) >= LVL_GRGOD) {
      send_to_char("You cannot change that.\r\n", ch);
      return;
    }
    strncpy(tmp_store.pwd, CRYPT(val_arg, tmp_store.name), MAX_PWD_LENGTH);
    tmp_store.pwd[MAX_PWD_LENGTH] = '\0';
    sprintf(buf, "Password changed to '%s'.", val_arg);
    break;
  case 46: SET_OR_REMOVE(PLR_FLAGS(vict), PLR_NODELETE); break;
  case 47:
    if (GET_LEVEL(vict) >= LVL_GRGOD)
      RANGE(1, 25);
  else
      RANGE(1, race_stat_limits[(int)GET_RACE(vict)][CHARISMA_INDEX]);
    vict->real_abils.cha = value;
    affect_total(vict);
    break;
  case 48: if( GET_LEVEL(vict) < LVL_IMMORT ) SET_OR_REMOVE(PLR_FLAGS(vict), PLR_SHUNNED); break;
  case 49: GET_CLAN(vict) = RANGE(0, 100); break;
  case 50: GET_CLAN_RANK(vict) = RANGE(0, 8); break;
  case 51: /* plague */ break;
  case 52: SET_OR_REMOVE( PLR_FLAGS(vict), PLR_RMC ); break;
  case 53: GET_OLC_ZONE(vict) = value; break;
  case 54:
	if(IS_NPC(vict)) {
	  send_to_char("You may not assign a mob to a team.\r\n", ch);
	  break;
	}
	if (!str_cmp(val_arg, "gold"))
	{
          /* Make them a member of the gold team */
          if (PRF_FLAGGED(vict, PRF_GOLD_TEAM))
	    send_to_char("They are already a member of the gold team.\r\n.", ch);
	  else
	  {
	    SET_BIT_AR(PRF_FLAGS(vict), PRF_GOLD_TEAM);
	    send_to_char("Done. They are now a member of the gold team.\r\n", ch);
	    send_to_char("You are now a member of the gold team!\r\n", vict);
	    if (PRF_FLAGGED(vict, PRF_BLACK_TEAM))
	        REMOVE_BIT_AR(PRF_FLAGS(vict), PRF_BLACK_TEAM);
	    if (PRF_FLAGGED(vict, PRF_ROGUE_TEAM))
	        REMOVE_BIT_AR(PRF_FLAGS(vict), PRF_ROGUE_TEAM);
	    if (real_room(GOLD_TEAM_JAIL) > -1)
	    {
	      GET_LOADROOM(vict) = GOLD_TEAM_JAIL;
	      if (!PLR_FLAGGED(vict, PLR_LOADROOM))
	          SET_BIT_AR(PLR_FLAGS(vict), PLR_LOADROOM);
	    }
	    else /* Ack! */
	      send_to_char("Error: Gold Team Jail does not exist!\r\n", ch);
            flag_player_into_team(vict, FLAG_TEAM_GOLD);
	  }
	}
	else if (!str_cmp(val_arg, "black"))
	{
          /* Make them a member of the black team */
	  if (PRF_FLAGGED(vict, PRF_BLACK_TEAM))
	    send_to_char("They are already a member of the black team.\r\n", ch);
	  else
	  {
	    SET_BIT_AR(PRF_FLAGS(vict), PRF_BLACK_TEAM);
	    send_to_char("Done. They are now a member of the black team.\r\n", ch);
	    send_to_char("You are now a member of the black team!\r\n", vict);
	    if (PRF_FLAGGED(vict, PRF_GOLD_TEAM))
		REMOVE_BIT_AR(PRF_FLAGS(vict), PRF_GOLD_TEAM);
	    if (PRF_FLAGGED(vict, PRF_ROGUE_TEAM))
	        REMOVE_BIT_AR(PRF_FLAGS(vict), PRF_ROGUE_TEAM);
	    if (real_room(BLACK_TEAM_JAIL) > -1)
	    {
	      GET_LOADROOM(vict) = BLACK_TEAM_JAIL;
	      if (!PLR_FLAGGED(vict, PLR_LOADROOM))
                  SET_BIT_AR(PLR_FLAGS(vict), PLR_LOADROOM);
	    }
	    else /* Ack! */
	      send_to_char("Error: Black Team Jail does not exist!\r\n", ch);
            flag_player_into_team(vict, FLAG_TEAM_BLACK);
	  }
	}
    else if (!str_cmp(val_arg, "rogue"))
	{
          /* Make them a member of the Rogue team */
          if (PRF_FLAGGED(vict, PRF_ROGUE_TEAM))
	    send_to_char("They are already a member of the rogue team.\r\n.", ch);
	  else
	  {
	    SET_BIT_AR(PRF_FLAGS(vict), PRF_ROGUE_TEAM);
	    send_to_char("Done. They are now a member of the rogue team.\r\n", ch);
	    send_to_char("You are now a member of the rogue team!\r\n", vict);
	    if (PRF_FLAGGED(vict, PRF_BLACK_TEAM))
	        REMOVE_BIT_AR(PRF_FLAGS(vict), PRF_BLACK_TEAM);
	    if (PRF_FLAGGED(vict, PRF_GOLD_TEAM))
		REMOVE_BIT_AR(PRF_FLAGS(vict), PRF_GOLD_TEAM);
		if (real_room(ROGUE_TEAM_JAIL) > -1)
	    {
	      GET_LOADROOM(vict) = ROGUE_TEAM_JAIL;
	      if (!PLR_FLAGGED(vict, PLR_LOADROOM))
	          SET_BIT_AR(PLR_FLAGS(vict), PLR_LOADROOM);
	    }
	    else /* Ack! */
	      send_to_char("Error: Rogue Team Jail does not exist!\r\n", ch);
            flag_player_into_team(vict, FLAG_TEAM_ROGUE);
	  }
	}
	else if (!str_cmp(val_arg, "none"))
	{
	  /* Remove them from the gold team */
	  if (PRF_FLAGGED(vict, PRF_GOLD_TEAM))
	  {
	    send_to_char("Done. They are no longer a member of the gold team.\r\n", ch);
	    send_to_char("You have been removed from the gold team!\r\n", vict);
	    REMOVE_BIT_AR(PRF_FLAGS(vict), PRF_GOLD_TEAM);
	  }
	  /* Remove them from the black team */
	  else if (PRF_FLAGGED(vict, PRF_BLACK_TEAM))
	  {
	    send_to_char("Done. They are no longer a member of the black team.\r\n", ch);
	    send_to_char("You have been removed from the black team!\r\n", vict);
	    REMOVE_BIT_AR(PRF_FLAGS(vict), PRF_BLACK_TEAM);
	  }
	  /* Remove them from the rogue team */
	  else if (PRF_FLAGGED(vict, PRF_ROGUE_TEAM))
	  {
	    send_to_char("Done. They are no longer a member of the rogue team.\r\n", ch);
	    send_to_char("You have been removed from the rogue team!\r\n", vict);
	    REMOVE_BIT_AR(PRF_FLAGS(vict), PRF_ROGUE_TEAM);
	  }
	  /* Set start room back to default */
	  if (PLR_FLAGGED(vict, PLR_LOADROOM))
	      REMOVE_BIT_AR(PLR_FLAGS(vict), PLR_LOADROOM);
	    GET_LOADROOM(vict) = getStartRoom(vict);;
          flag_player_from_game(vict);
	}
	else
	{
	  send_to_char("Must be 'gold', 'black', or 'none'.\r\n", ch);
	  return;
	}
	break;

  case 55:
	SET_OR_REMOVE(PLR_FLAGS(vict), PLR_HUNTED);
	if (IS_SET_AR(PLR_FLAGS(vict), PLR_HUNTED))
	    set_hunted_player(vict, HUNTED_TIME);
	else
	    unset_hunted_player(vict);
	break;
    case 56:
        // Accept numbers or letters for race setting, except for human which
        // can't accept the number 0.
        value = atoi(val_arg);
        if ((i = all_parse_race(*val_arg)) == RACE_UNDEFINED && value == 0)
        {
            sendChar(ch, "That is not a valid race.\r\n");
            return;
        }
        RANGE(-1, NUM_RACES - 1);
        if(value != 0)
            SET_RACE(vict, value);
        else
            SET_RACE(vict, i);
        break;
  case 57:
        SET_SUBRACE(vict, RANGE(SUBRACE_UNDEFINED, NUM_SUBRACES));
	break;
  case 58:
        GET_QP(vict) = RANGE(0, 9999);
        affect_total(vict);
        break;
  case 59:
        //GET_REWARD(vict) = RANGE(0, 100000000);
	sendChar(ch, "Reward is disabled.\r\n");
        affect_total(vict);
        break;
  case 60: 
        SET_OR_REMOVE(PLR_FLAGS(vict), PLR_RAWLOG);
        break;
  case 61: 
        SET_OR_REMOVE(PLR_FLAGS(vict), PLR_REIMBURSED);
        break;
  case 62: 
	vict->player.time.birth -= (SECS_PER_MUD_YEAR) * value;
        affect_total(vict);
	break;
  case 63:
        if (vict == ch) {
            send_to_char("Erm, no.  Its not a very good idea.\r\n", ch);
	    return;
	}
	if (IS_SET_AR(PLR_FLAGS(vict), PLR_RETIRED))
            REMOVE_BIT_AR(PLR_FLAGS(vict), PLR_RETIRED);
        else
	    SET_BIT_AR(PLR_FLAGS(vict), PLR_RETIRED);
        break;
  case 64:
        if (IS_NPC(vict)) {
            send_to_char("If you want to change a mob's name use OLC.\r\n", ch);
            return;
	}
        if (GET_LEVEL(vict) >= GET_LEVEL(ch) && ch != vict) {
	    send_to_char("I don't think so.\r\n", ch);
	    return;
	}

	mudlog( BRF, LVL_IMMORT, TRUE, "(GC) %s changed %s's name to %s.",
		GET_NAME(ch), GET_NAME(vict), val_arg);
        strcpy(name, vict->player.name);
	strcpy(vict->player.name, val_arg);
        // save the freshly renamed character
        save_char(ch, NOWHERE);
        Crash_crashsave(ch);
        save_locker_of(ch);
        // remove the old rentfile
        crashDeleteFile(name);
    	break;
  case 65:
        if (vict == ch && !IS_SET_AR(PLR_FLAGS(vict), PLR_NOCOMM)) {
            send_to_char("Erm, no.  Its not a very good idea.\r\n", ch);
            return;
        }
        if (IS_SET_AR(PLR_FLAGS(vict), PLR_NOCOMM)) {
            send_to_char("The player can now use comms again.\r\n", ch);
            REMOVE_BIT_AR(PLR_FLAGS(vict), PLR_NOCOMM);
        } else {
            SET_BIT_AR(PLR_FLAGS(vict), PLR_NOCOMM);
	    send_to_char("The player can no longer use comm channels.\r\n", ch);
        }
        break;
  case 66:
        {
          char abuf[40];
          if (GET_LOCKER(vict) != 0) {
            sprintf(abuf, "That player HAD locker #%d\r\n", GET_LOCKER(vict));
            send_to_char(abuf, ch);
          }
          GET_LOCKER(vict) = value;
          sprintf(abuf, "Player's locker set to #%d\r\n", GET_LOCKER(vict));
        } break;
  case 67:
        SET_OR_REMOVE(PLR_FLAGS(vict), PLR_GRIEF);
        break;
  case 68:
        GET_ORCS(vict) = RANGE(0, 12);
        break;
  case 69: /* afk */
        SET_OR_REMOVE(PRF_FLAGS(vict), PRF_AFK);
        break;
  case 70:
        GET_ADVANCE_LEVEL(vict) = RANGE(0, 10);
        break;
  case 71: 
        GET_SPEC(vict) = RANGE(0, 2);
        break;
  default:
    sprintf(buf, "Can't set that!");
    break;
  }

  if (fields[l].type == BINARY) {
    sprintf(buf, "%s %s for %s.\r\n", fields[l].cmd, ONOFF(on),
	    GET_NAME(vict));
    CAP(buf);
  } else if (fields[l].type == NUMBER) {
    sprintf(buf, "%s's %s set to %d.\r\n", GET_NAME(vict),
	    fields[l].cmd, value);
  } else
    strcat(buf, "\r\n");
  send_to_char(CAP(buf), ch);

  if (!is_file && !IS_NPC(vict))
    save_char(vict, NOWHERE);

  if (is_file) {
    char_to_store(vict, &tmp_store);
    fseek(player_fl, (player_i) * sizeof(struct char_file_u), SEEK_SET);
    fwrite(&tmp_store, sizeof(struct char_file_u), 1, player_fl);
    free_char(cbuf);
    send_to_char("Saved in file.\r\n", ch);
  }
}

ACMD(do_entrance)
{
  int door, room, rnum;
  *buf='\0';
  *buf2='\0';
  rnum=ch->in_room; 
  sendChar(ch, "Entrances to this room:\r\n");

  for (room=0; room<=top_of_world; room++)
    for (door=0; door<NUM_OF_DIRS; door++)
      if (world[room].dir_option[door])
        if (world[room].dir_option[door]->to_room == rnum) {
          sprintf(buf2, "%-5s from [%5d] %s \r\n", 
             dirs[door], world[room].number, world[room].name); 
	  strcat(buf,buf2);
        } 
  if (!(*buf))
    sendChar(ch," None!\r\n");
  else
    sendChar(ch, buf);
}

/* Mortius : 30-03-2000 : Take an item from a lower level without asking */

ACMD(do_chown)
{
  CharData *victim;
  ObjData  *obj = NULL;
  char buf2[80];
  char buf3[80];
  int i, k = 0;

  if (IS_SET_AR(PLR_FLAGS(ch), PLR_RETIRED)) {
      send_to_char("Relax and enjoy being retired.\r\n", ch);
      return;
  }

  two_arguments(argument, buf2, buf3);

  if (!*buf2)
    send_to_char("Syntax: chown <object> <character>.\r\n", ch);
  else if (!(victim = get_char_vis(ch, buf3, 0)))
    send_to_char("Nobody by that name is here.\r\n", ch);
  else if (victim == ch)
    send_to_char("Are you sure you're feeling ok?\r\n", ch);
  else if (GET_LEVEL(victim) >= GET_LEVEL(ch))
    send_to_char("That's really not such a good idea.\r\n", ch);
  else if (!*buf3)
    send_to_char("Syntax: chown <object> <character>.\r\n", ch);
  else {
    for (i = 0; i < NUM_WEARS; i++) {
      if (GET_EQ(victim, i) && CAN_SEE_OBJ(ch, GET_EQ(victim, i)) &&
         isname(buf2, GET_EQ(victim, i)->name)) {
        obj_to_char(unequip_char(victim, i), victim);
        k = 1;
      }
    }

  if (!(obj = get_obj_in_list_vis(victim, buf2, victim->carrying))) {
    if (!k && !(obj = get_obj_in_list_vis(victim, buf2, victim->carrying))) {
      sprintf(buf, "%s does not appear to have the %s.\r\n", GET_NAME(victim), buf2);
      send_to_char(buf, ch);
      return;
    }
  }

  /* Mort : When I get more time I will fix this but for now this will stop us
            crashing */

  if (!obj || obj == NULL) {
      send_to_char("Something has gone wrong with chown.  You broke it!\r\n",ch);
      return;
  }

  if (GET_INVIS_LEV(ch) < GET_LEVEL(victim)) {
      act("$n makes a magical gesture and $p flies from $N to $m.",
           FALSE, ch, obj, victim, TO_NOTVICT);
      act("$n makes a magical gesture and $p flies away from you to $m.",
           FALSE, ch, obj, victim, TO_VICT);
      act("You make a magical gesture and $p flies away from $N to you.",
           FALSE, ch, obj, victim, TO_CHAR);
  } else 
        act("You remove $p from $N without $M knowing it happened.",
   	     FALSE, ch, obj, victim, TO_CHAR);

  obj_from_char(obj);
  obj_to_char(obj, ch);
  save_char(ch, NOWHERE);
  save_char(victim, NOWHERE);
  }

}

/* Motius : This was writen by Forge of NCMUD.  It works really well and saves
	    the hassle of messing with files or loading items for players */

ACMD(do_corpse)
{
  #define DO_CORPSE_MAX_STRLEN 20000

  FILE *fl;
  ObjData *corpse, *obj;
  ExtraDescrData *new_descr;
  char buf[DO_CORPSE_MAX_STRLEN], arg[DO_CORPSE_MAX_STRLEN],
       name[DO_CORPSE_MAX_STRLEN], tmp[DO_CORPSE_MAX_STRLEN];
  char list[DO_CORPSE_MAX_STRLEN];
  int obj_no, counter = 0;
  long int money;

  if (IS_SET_AR(PLR_FLAGS(ch), PLR_RETIRED)) {
      send_to_char("Relax and enjoy being retired.\r\n", ch);
      return;
  }

  if (IS_NPC(ch))		return;
  if (ch->in_room < 0)		return;

  argument = one_argument(argument, arg);
  argument = one_argument(argument, name);

  if(!(*name) || !(*arg)) {
    if(GET_LEVEL(ch) >= 55)
      send_to_char("USAGE: corpse <look | create> <name>\n\r", ch);
    else
      send_to_char("USAGE: corpse look <name>\n\r", ch);
    return;
  }

  strlower(name);

  sprintf(buf, "corpse/%s", name);
  if(!(fl = fopen(buf, "r"))) {
    sprintf(buf, "No corpse found in database for %s.\n\r", name);
    send_to_char(buf, ch);
    return;
  }

  /* create the corpse will all the stuff in it */
  if(GET_LEVEL(ch) >= 55 && *arg == 'c') {
    fgets(buf, 100, fl);
    sprintf(tmp, "%s died on %s", name, buf);
    send_to_char(tmp, ch);
    act("$n reaches deep into the ground and pulls out a corpse.",
        1,ch,0,0,TO_ROOM);

    CREATE(corpse, struct obj_data, 1);
    clear_object(corpse);

    sprintf(buf, "corpse %s", name);
    corpse->name = strdup(buf);

    sprintf(buf, "Corpse of %s", name);
    corpse->short_description = strdup(buf);
    sprintf(buf, "Corpse of %s is lying here.", name);

/* Mortius : take all from a corpse seems to depend on this */
    sprintf(buf2, "%s", CAP(name));
    CREATE(new_descr, struct extra_descr_data, 1);
    new_descr->keyword = str_dup(buf2); /* Checked later on.. */
    sprintf(buf2, "Ewwww! What ever killed %s sure made it messy!",name);
    new_descr->description = str_dup(buf2); /* Just decoration. */
    new_descr->next = NULL;
    corpse->ex_description = new_descr;


    corpse->description = strdup(buf);
    corpse->obj_flags.type_flag = ITEM_CONTAINER;
    corpse->obj_flags.value[0] = 0;
    corpse->obj_flags.value[3] = 1;
    corpse->obj_flags.weight = 10;
    corpse->obj_flags.cost_per_day = 100000;
    corpse->obj_flags.timer = 10;

    corpse->next = object_list;
    object_list = corpse;

    obj_to_room(corpse, ch->in_room);

    for(;;)
    {
      if(fgets(buf, 100, fl) == NULL)
        break;
      if(*buf == 'E')
        break;
      else if(*buf == 'M') {
        counter++;
        sscanf(buf, "M %ld \n", &money);
        if((obj = create_money(money)))
          obj_to_obj(obj, corpse);
      }
      else if(*buf == 'O') {
        int timer;
        counter++;
        sscanf(buf, "O %d %d \n", &obj_no, &timer);
        // Right now, a new set of equipment which might not
		// Be as good as the old set loads here.  Craklyn
		if((obj = read_object(obj_no, VIRTUAL)))
          obj_to_obj(obj, corpse);
        obj->obj_flags.timer = timer;
      }
      else if(*buf == 'W') {
        char *roomname;
        int room;
        char bufw[MAX_STRING_LENGTH];
        sscanf(buf, "W %d ", &room);
        /* coded so that won't crash if format of buf is screwed up */
        for(roomname=buf; *roomname; roomname++) {
            if(roomname>buf+2 && *roomname==' ') {
                roomname++;
                break;
            }
        }
        if(*roomname)
            sprintf(bufw, "Died in room [%d] %s\n\r", room, roomname);
        else
            sprintf(bufw, "Died in room [%d].\n\r", room);
        send_to_char(bufw, ch);
      }
    }
    sprintf(buf, "%d items loaded to corpse.\n\r", counter);
    send_to_char(buf, ch);
    mudlog( BRF, GET_LEVEL(ch), TRUE, "(GC) [CORPSE] >: %s created the corpse of %s (%d items loaded)",
            GET_NAME(ch), name, counter);
  }
  else {
    fgets(buf, 100, fl);
    sprintf(list, "Contents of %s's corpse: %s", name, buf);
    for(;;)
    {
      if(fgets(buf, 100, fl) == NULL)
        break;
      if(*buf == 'E')
        break;
      else if(*buf == 'M') {
        counter++;
        sscanf(buf, "M %ld \n", &money);
        sprintf(tmp, "[%3d] %ld gold coins\n\r", counter, money);
        /* Forge (11/10/94)
          checking */
        if(strlen(list) + strlen(tmp) +40 < DO_CORPSE_MAX_STRLEN)
            strcat(list, tmp);
        else {
            strcat(list, "More not listed");
            break;
        }
      }
      else if(*buf == 'O') {
          int obj_no, timer;
          char *name;
          counter++;
          /* Forge (11/12/94)
            fixed this to support timers
            sprintf(tmp, "[%3d] %s", counter, buf+1);
          */
          sscanf(buf, "O %d %d ", &obj_no, &timer);
          /* new format uses ';', old format uses ':' */
          if(! (name = strchr(buf,';')) ) name = strchr(buf,':');
          if(*name) {
            if(timer) sprintf(tmp, "[%3d] %d (timer: %d) %s",
              counter, obj_no, timer, name+1);
            else sprintf(tmp, "[%3d] %d (untimered) %s",
              counter, obj_no, name+1);
          } else {
            if(timer) sprintf(tmp, "[%3d] (timer: %d) %d\n\r",
              counter, timer, obj_no);
            else sprintf(tmp, "[%3d] %d (untimered)\n\r",
              counter, obj_no);
          }
          /* Forge (11/10/94)
            checking */
          if(strlen(list) + strlen(tmp) +40 < DO_CORPSE_MAX_STRLEN)
              strcat(list, tmp);
          else {
              strcat(list, "More not listed");
              break;
          }
      }
      /* Forge (11/10/94)
        where the dude died */
      else if(*buf == 'W') {
        char *roomname;
        int room;
        char bufw[MAX_STRING_LENGTH];
        sscanf(buf, "W %d ", &room);
        /* coded so that won't crash if format of buf is screwed up */
        for(roomname=buf; *roomname; roomname++) {
            if(roomname>buf+2 && *roomname==' ') {
                roomname++;
                break;
            }
        }
        if(*roomname)
            sprintf(bufw, "Died in room [%d] %s\n\r", room, roomname);
        else
            sprintf(bufw, "Died in room [%d].\n\r", room);
        if(strlen(list) + strlen(tmp) +40 < DO_CORPSE_MAX_STRLEN)
            strcat(list, bufw);
          else {
              strcat(list, "More not listed");
              break;
          }
      }
    } /* end for */

    page_string(ch->desc, list, 1);
  }

  fclose(fl);
}

/* Mortius : Adding in liblist for listing rooms/objs/mobs/zones.  This was
	     from a snippet I found.  I have limited the list to 200 otherwise
	     it can crash the mud. */

ACMD(do_liblist)
{
  extern struct room_data *world;
  extern struct index_data *mob_index;
  extern struct char_data *mob_proto;
  extern struct index_data *obj_index;
  extern struct obj_data *obj_proto;
  extern struct zone_data *zone_table;
  extern int top_of_objt;
  extern int top_of_mobt;
  extern int top_of_world;

  int first, last, nr, found = 0, total = 0;

  two_arguments(argument, buf, buf2);

  if (!*buf || !*buf2) {
    switch (subcmd) {
      case SCMD_RLIST:
        send_to_char("Usage: rlist <begining number> <ending number>\r\n", ch);
        break;
      case SCMD_OLIST:
        send_to_char("Usage: olist <begining number> <ending number>\r\n", ch);
        break;
      case SCMD_MLIST:
        send_to_char("Usage: mlist <begining number> <ending number>\r\n", ch);
        break;
      case SCMD_ZLIST:
        send_to_char("Usage: zlist <begining number> <ending number>\r\n", ch);
        break;
      default:
        mudlog( BRF, LVL_IMMORT, TRUE, "SYSERR:: invalid SCMD passed to ACMDdo_build_list!");
        break;
    }
    return;
  }

  first = atoi(buf);
  last  = atoi(buf2);
  total = last - first;

  if ((first < 0) || (first > 99999) || (last < 0) || (last > 99999)) {
    send_to_char("Values must be between 0 and 99999.\n\r", ch);
    return;
  }

  if (first >= last) {
    send_to_char("Second value must be greater than first.\n\r", ch);
    return;
  }

  if (total > 100) {
      send_to_char("Max number 100 any more and you cause serious problems.\r\n", ch);
      return;
  }

  switch (subcmd) {
    case SCMD_RLIST:
      sprintf(buf, "Room List From Vnum %d to %d\r\n", first, last);
      for (nr = 0; nr <= top_of_world && (world[nr].number <= last); nr++) {
        if (world[nr].number >= first) {
          sprintf(buf, "%s%5d. [%5d] (%3d) %s\r\n", buf, ++found,
                  world[nr].number, world[nr].zone,
                  world[nr].name);
        }
      }
      break;
    case SCMD_OLIST:
      sprintf(buf, "Object List From Vnum %d to %d\r\n", first, last);
      for (nr = 0; nr <= top_of_objt && (obj_index[nr].virtual <= last); nr++) {
        if (obj_index[nr].virtual >= first) {
          sprintf(buf, "%s%5d. [%5d] %s\r\n", buf, ++found,
                  obj_index[nr].virtual,
                  obj_proto[nr].short_description);
        }
      }
      break;
    case SCMD_MLIST:
      sprintf(buf, "Mob List From Vnum %d to %d\r\n", first, last);
      for (nr = 0; nr <= top_of_mobt && (mob_index[nr].virtual <= last); nr++) {
        if (mob_index[nr].virtual >= first) {
          sprintf(buf, "%s%5d. [%5d] %s\r\n", buf, ++found,
                  mob_index[nr].virtual,
                  mob_proto[nr].player.short_descr);
        }
      }
      break;
    case SCMD_ZLIST:
      sprintf(buf, "Zone List From Vnum %d to %d\r\n", first, last);
      for (nr = 0; nr <= top_of_zone_table && (zone_table[nr].number <= last); nr++) {
        if (zone_table[nr].number >= first) {
          sprintf(buf, "%s%5d. [%5d] (%3d) %s\r\n", buf, ++found,
                  zone_table[nr].number, zone_table[nr].lifespan,
                  zone_table[nr].name);
        }
      }
      break;
    default:
      mudlog( BRF, LVL_IMMORT, TRUE, "SYSERR:: invalid SCMD passed to ACMD do_build_list!");
      return;
  }

  if (!found) {
    switch (subcmd) {
      case SCMD_RLIST:
        send_to_char("No rooms found within those parameters.\r\n", ch);
        break;
      case SCMD_OLIST:
        send_to_char("No objects found within those parameters.\r\n", ch);
        break;
      case SCMD_MLIST:
        send_to_char("No mobiles found within those parameters.\r\n", ch);
        break;
      case SCMD_ZLIST:
        send_to_char("No zones found within those parameters.\r\n", ch);
        break;
      default:
        mudlog( BRF, LVL_IMMORT, TRUE, "SYSERR:: invalid SCMD passed to do_build_list!");
        break;
    }
    return;
  }

  page_string(ch->desc, buf, 1);
}

ACMD(do_tick)
{
	int seconds = 0;

  if(IS_NPC(ch)) {
     send_to_char("Monsters can't tell time, let alone advance it!\r\n", ch);
     return;
  }

  send_to_all("You feel as if you have grown an hour older to soon!\r\n");

  // Affects now have to be removed 72 times to keep them on track. 
  for(; seconds < 72; seconds++) {
	  affect_update();
          regen_update();
  }
  point_update();
  zone_update();
  mobile_activity();
  water_activity();
  weather_and_time(1);
  fishoff_ticker();
  

  mudlog( CMP, GET_LEVEL(ch), TRUE, "(GC) [TICK]  %s has typed tick",
          GET_NAME(ch));
}

ACMD(do_peace)
{
        struct char_data *vict, *next_v;
        act ("\007\007$n shouts &15PEACE&00.....and &15PEACE&00 fills the room.",
                FALSE,ch,0,0,TO_ROOM);
        send_to_room("\007\007Everything is quite peaceful now.\r\n",ch->in_room);
        for (vict = world[ch->in_room].people; vict; vict = next_v) {
             next_v = vict->next_in_room;
             if (FIGHTING(vict)) {
                 if (FIGHTING(FIGHTING(vict))==vict)
                     stop_fighting(FIGHTING(vict));
                 stop_fighting(vict);
                 stop_fighting(ch);
                 GET_POS(vict) = POS_RESTING;
                 update_pos(vict);
             }
       }
}

/** Used to read and gather a bit of information about external log files while
 * in game.
 * Makes use of the '@' color codes in the file status information.
 * Some of the methods used are a bit wasteful (reading through the file
 * multiple times to gather diagnostic information), but it is
 * assumed that the files read with this function will never be very large.
 * Files to be read are assumed to exist and be readable and if they aren't,
 * log the name of the missing file.
 */
ACMD(do_file)
{
  /* Local variables */
  int def_lines_to_read = 15;  /* Set the default num lines to be read. */
  int max_lines_to_read = 300; /* Maximum number of lines to read. */
  FILE *req_file;              /* Pointer to file to be read. */
  size_t req_file_size = 0;    /* Size of file to be read. */
  int req_file_lines = 0;      /* Number of total lines in file to be read. */
  int lines_read = 0; /* Counts total number of lines read from the file. */
  int req_lines = 0;  /* Number of lines requested to be displayed. */
  int i, j;           /* Generic loop counters. */
  int l;              /* Marks choice of file in fields array. */
  char field[MAX_INPUT_LENGTH];  /* Holds users choice of file to be read. */
  char value[MAX_INPUT_LENGTH];  /* Holds # lines to be read, if requested. */
  char buf[MAX_STRING_LENGTH];   /* Display buffer for req_file. */

  /* Defines which files are available to read. */
  struct file_struct {
    char *cmd;          /* The 'name' of the file to view */
    char level;         /* Minimum level needed to view. */
    char *file;         /* The file location, relative to the working dir. */
    int read_backwards; /* Should the file be read backwards by default? */
  } fields[] = {
    { "bug",            LVL_DEITY,	BUG_FILE,                 TRUE},
    { "typo",           LVL_DEITY, 	TYPO_FILE,                 TRUE},
    { "ideas",          LVL_DEITY,     	IDEA_FILE,                 TRUE},
    { "xnames",         LVL_GOD,     	XNAME_FILE,                TRUE},
    { "badpws",         LVL_CREATOR,    BADPWS_LOGFILE,            TRUE},
    { "crash",          LVL_GRGOD,      CRASH_LOGFILE,             TRUE},
    { "deaths",         LVL_SAINT,      DEATHS_LOGFILE ,           TRUE},
    { "delete",         LVL_CREATOR,    DELETE_LOGFILE,            TRUE},
    { "dtraps",         LVL_SAINT,      DTRAPS_LOGFILE,            TRUE},
    { "errors",         LVL_LRGOD,      ERRORS_LOGFILE,            TRUE},
    { "godcmds",        LVL_GRGOD,      GODCMDS_LOGFILE,           TRUE},
    { "levels",         LVL_ANGEL,      LEVELS_LOGFILE,            TRUE},
    { "help",           LVL_DEITY,      HELP_LOGFILE,              TRUE},
    { "metaphys",       LVL_CREATOR,    METAPHYS_LOGFILE,          TRUE},
    { "newplayers",     LVL_ANGEL,     	NEWPLAYERS_LOGFILE,        TRUE},
    { "objecterrors",   LVL_LRGOD,     	OBJECTERRORS_LOGFILE,      TRUE},
    { "olc",            LVL_DGOD,       OLC_LOGFILE,               TRUE},
    { "prayers",        LVL_HERO,     	PRAYERS_LOGFILE,           TRUE},
    { "reboots",        LVL_GOD,     	REBOOTS_LOGFILE,           TRUE},
    { "rentgone",       LVL_CREATOR,    RENTGONE_LOGFILE,          TRUE},
    { "transcend",      LVL_SAINT,      TRANSCEND_LOGFILE,         TRUE},
    { "trigger",        LVL_DGOD,     	TRIGGER_LOGFILE,           TRUE},
    { "usage",          LVL_GOD,     	USAGE_LOGFILE,             TRUE},
    { "syslog",         LVL_GRGOD,      SYSLOG_LOGFILE,            TRUE},
    { "\n", 0, "\n", FALSE } /* This must be the last entry */
  };

   /* Initialize buffer */
   buf[0] = '\0';

   /**/
   /* End function variable set-up and initialization. */

  skip_spaces(&argument);

   /* Display usage if no argument. */
   if (!*argument) {
     sendChar(ch, "USAGE: file <filename> <num lines>\r\n\r\nFile options:\r\n");
     for (j = 0, i = 0; fields[i].level; i++)
      if (fields[i].level <= GET_LEVEL(ch))
         sendChar(ch, "%-15s%s\r\n", fields[i].cmd, fields[i].file);
    return;
  }

   /* Begin validity checks. Is the file choice valid and accessible? */
   /**/
   /* There are some arguments, deal with them. */
   two_arguments(argument, field, value);

  for (l = 0; *(fields[l].cmd) != '\n'; l++)
   {
    if (!strncmp(field, fields[l].cmd, strlen(field)))
      break;
   }

   if(*(fields[l].cmd) == '\n') {
     sendChar(ch, "'%s' is not a valid file.\r\n", field);
    return;
  }

   if (GET_LEVEL(ch) < fields[l].level) {
     sendChar(ch, "You have not achieved a high enough level to view '%s'.\r\n",
         fields[l].cmd);
    return;
  }

   /* Number of lines to view. Default is 15. */
  if(!*value)
     req_lines = def_lines_to_read;
   else if (!isdigit(*value))
   {
     /* This check forces the requisite positive digit and prevents negative
      * numbers of lines from being read. */
     sendChar(ch, "'%s' is not a valid number of lines to view.\r\n", value);
     return;
   }
  else
   {
     req_lines = atoi(value);
     /* Limit the maximum number of lines */
     req_lines = MIN( req_lines, max_lines_to_read );
   }

   /* Must be able to access the file on disk. */
   if (!(req_file=fopen(fields[l].file,"r"))) {
     sendChar(ch, "The file %s can not be opened.\r\n", fields[l].file);
     mudlog(BRF, LVL_IMMORT, TRUE,
            "SYSERR: Error opening file %s using 'file' command.",
             fields[l].file);
     return;
  }
   /**/
   /* End validity checks. From here on, the file should be viewable. */

   /* Diagnostic information about the file */
   req_file_size = file_sizeof(req_file);
   req_file_lines = file_numlines(req_file);

   snprintf( buf, sizeof(buf),
       "&07File:&00 %s&07; Min. Level to read:&00 %d&07; File Location:&00 %s&07\r\n"
       "File size (bytes):&00 %ld&07; Total num lines:&00 %d\r\n",
       fields[l].cmd, fields[l].level, fields[l].file, (long) req_file_size,
       req_file_lines);

   /* Should the file be 'headed' or 'tailed'? */
   if ( (fields[l].read_backwards == TRUE) && (req_lines < req_file_lines) )
  {
     snprintf( buf + strlen(buf), sizeof(buf) - strlen(buf),
               "&07Reading from the tail of the file.&00\r\n\r\n" );
     lines_read = file_tail( req_file, buf, sizeof(buf), req_lines );
  }
   else
   {
     snprintf( buf + strlen(buf), sizeof(buf) - strlen(buf),
              "&07Reading from the head of the file.&00\r\n\r\n" );
     lines_read = file_head( req_file, buf, sizeof(buf), req_lines );
   }

   /** Since file_head and file_tail will add the overflow message, we
    * don't check for status here. */
   if ( lines_read == req_file_lines )
  {
     /* We're reading the entire file */
     snprintf( buf + strlen(buf), sizeof(buf) - strlen(buf),
         "\r\n&07Entire file returned (&00%d &07lines).&00\r\n",
         lines_read );
   }
   else if ( lines_read == max_lines_to_read )
     {
     snprintf( buf + strlen(buf), sizeof(buf) - strlen(buf),
         "\r\n&07Maximum number of &00%d &07lines returned.&00\r\n",
         lines_read );
     }
   else
   {
     snprintf( buf + strlen(buf), sizeof(buf) - strlen(buf),
         "\r\n%d &07lines returned.&00\r\n",
         lines_read );
   }

   /* Clean up before return */
   fclose(req_file);

   page_string(ch->desc, buf, 1);
}


ACMD(do_wizslap)
{
  CharData* victim;
  int to_room = 0;
  int msg_num = number(0, 4);

  extern int top_of_world;
  extern struct room_data *world;

  // Lets set up some msg's to display
  //
  char* wizslapMsg[][3] = {
  { "You stare at $N and utter the words, 'azje al tan za'\n"
    "$N fades from view.",
    "$n stares at $N and utters the words, 'azje al tan za'\n"
    "$N fades from view.",
    "$n stares at you and utters the words, 'azje al tan za'\n"
    "You are no longer where you once were."
  },

  {"You summon forth demons to remove $N from your sight.",
   "A hord of demons are summoned to remove $N from $n's sight.",
   "$n summons demons to remove you from $s sight.  You are taken to, "},

  {"You laugh as the earth swallows $N taking $M to only god knows where!",
   "$n throws $s head back and laughs as the earth swallows $N.",
   "$n throws $s head back and laughs as the earth splits and swallows you."},

  {"You reach down and grab $N, tossing $M from this plane.",
   "$n's hand reaches down, grabs $N and tosses $M from this plane.",
   "$n's hand reaches down grabbing you by the head then tosses you from this plane."},

  {"You swing your boot down, kicking $N's scrawny butt clear across the mud.",
   "$n's boot swings down, kicking $N's scrawny butt clear across the mud.",
   "$n's boot swings down, kicking your scrawny butt clear across the mud."},
  };

  one_argument(argument, arg);

  /* Find a victim */
  if (!(victim = get_char_vis(ch, arg, 0))) {
      send_to_char("Pick another victim, that one isn't around!\r\n", ch);
      return;
  }

  /* Can only be done to players lower then yourself */
  if (GET_LEVEL(victim) >= GET_LEVEL(ch)) {
      send_to_char("Errm I don't think so.\r\n", ch);
      return;
  }

  // Lets hurt the victim a little
  //
  GET_HIT(victim)  = MIN(GET_HIT(victim), GET_MAX_HIT(victim)  / 2);
  GET_MANA(victim) = MIN(GET_MANA(victim), GET_MAX_MANA(victim) / 2);
  GET_MOVE(victim) = MIN(GET_MOVE(victim), GET_MAX_MOVE(victim) / 2);

  // Lets work out where to send the victim
  //
  do
  {
    to_room = number( 0, top_of_world );
  } while (IS_SET_AR(world[to_room].room_flags, ROOM_PRIVATE) || 
	   IS_SET_AR(world[to_room].room_flags, ROOM_DEATH));


  // Lets send out a message, more can be added later.
  //
  act( wizslapMsg[msg_num][0], FALSE, ch, 0, victim, TO_CHAR);
  act( wizslapMsg[msg_num][1], FALSE, ch, 0, victim, TO_NOTVICT);
  act( wizslapMsg[msg_num][2], FALSE, ch, 0, victim, TO_VICT);

  // Time to move the victim
  //
  char_from_room(victim);
  char_to_room(victim, to_room);
  look_at_room(victim, 0);

} /* End of wizslap */


// Mortius, command to let imms with set player level put there own
// lvl back to what it was.  Mainly for testing things.
//
/*
 * Mortius, small function created to help with the running of bloodbowl,
 * flag games or anything else thats using teams.
*/

#define NO_TEAM		0
#define GOLD_TEAM	1
#define BLACK_TEAM	2
#define ROGUE_TEAM  3
#define ALL_TEAM	4

ACMD(do_transquest)
{
  DescriptorData *i;
  CharData *victim;
  int team = NO_TEAM, restore = FALSE;
  char team_color[20], team_restore[20];

  if (IS_NPC(ch))
    return;

  if (IS_SET_AR(PLR_FLAGS(ch), PLR_RETIRED)) {
      send_to_char("Chill and relax for a while.\r\n", ch);
      return;
  }

  two_arguments(argument, team_color, team_restore);

  if (!*team_color) {
    send_to_char("Usage: transquest <all | gold | black> | <rogue> <norestore | restore>.\r\n", ch);
    return;
  }
  else if (strcmp("gold", team_color) == 0)
    team = GOLD_TEAM; 
  else if (strcmp("black", team_color) == 0)
    team = BLACK_TEAM;
  else if (strcmp("rogue", team_color) == 0)
    team = ROGUE_TEAM;
  else if (strcmp("all", team_color) == 0)
    team = ALL_TEAM;
  else
    return;

  /* Are we going to restore the team? */
  if (strcmp("restore", team_restore) == 0)
      restore = TRUE;

  for(i = descriptor_list; i; i = i->next)
      if (!i->connected && i->character && i->character != ch) {
          victim = i->character;

       if (team != ALL_TEAM) {
           if (team == GOLD_TEAM) 
               if (!PRF_FLAGGED(victim, PRF_GOLD_TEAM))
                   continue;
           if (team == BLACK_TEAM)
               if (!PRF_FLAGGED(victim, PRF_BLACK_TEAM))
                   continue;
           if (team == ROGUE_TEAM)
               if (!PRF_FLAGGED(victim, PRF_ROGUE_TEAM))
                   continue;
       } else {
           if (!PRF_FLAGGED(victim, PRF_BLACK_TEAM) &&
               !PRF_FLAGGED(victim, PRF_GOLD_TEAM))
               continue;
       }

       if (GET_LEVEL(victim) >= GET_LEVEL(ch))
           continue;

        act("$n disappears in a mushroom cloud.", FALSE, victim, 0, 0, TO_ROOM);
        char_from_room(victim);
        char_to_room(victim, ch->in_room);
        act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
        act("$n has transferred you!", FALSE, ch, 0, victim, TO_VICT);

        if( victim->mount != NULL ) {
            act("$n disappears in a mushroom cloud.", FALSE, victim->mount, 0, 0, TO_ROOM);
            char_from_room(victim->mount);
            char_to_room(victim->mount, victim->in_room);
            act("$n arrives from a puff of smoke.", FALSE, victim->mount, 0, 0, TO_ROOM);
            act("$n has transferred you!", FALSE, ch, 0, victim->mount, TO_VICT);
        }  // Mounteds shouldn't be on a team, but doing this anyway.
        else if( victim->rider != NULL ) {
            act("$n disappears in a mushroom cloud.", FALSE, victim->rider, 0, 0, TO_ROOM);
            char_from_room(victim->rider);
            char_to_room(victim->rider, victim->in_room);
            act("$n arrives from a puff of smoke.", FALSE, victim->rider, 0, 0, TO_ROOM);
            act("$n has transferred you!", FALSE, ch, 0, victim->rider, TO_VICT);
        }
       
	/* Lets restore the players */ 
	if (restore == TRUE) {
            while (victim->affected)
                affect_remove(victim, victim->affected, FALSE);
            GET_HIT(victim)  = GET_MAX_HIT(victim);
            GET_MANA(victim) = GET_MAX_MANA(victim);
            GET_MOVE(victim) = GET_MAX_MOVE(victim);
            send_to_char("The Game Master has restored you.\r\n", victim);
        }

        look_at_room(victim, 0);
       } /* End of for */
}

/* Xname: This will put a name into the xname list. */

ACMD(do_xname)
{
  char tempname[MAX_INPUT_LENGTH];
  int i = 0;
  FILE *fp;
  *buf = '\0';

  one_argument(argument, buf);

  if (!*buf) {
      send_to_char("Supply a name to xlist.\n\r", ch);
      return;
  }

  if (!(fp = fopen(XNAME_FILE, "a"))) {
      mudlog(BRF, GET_LEVEL(ch), TRUE, "Problem opening xname file.");
      return;
  }

  strcpy(tempname, buf);

  for (i = 0; tempname[i]; i++)
       tempname[i] = LOWER(tempname[i]);

  fprintf(fp, "%s\n", tempname);
  fclose(fp);

  sprintf(buf1, "%s has been xnamed.\r\n", tempname);
  send_to_char(buf1, ch);

  Read_Invalid_List();
}

/* Mortius: Lets search the files without having to go onto the site. */

ACMD(do_logfile)
{
  FILE *f;
  char filename[30], buf3[20], msg[MAX_STRING_LENGTH] = "\0";
  char usage[70]    = "Format:  logfile <filename> <search>\r\n";
  char options[256] = "Options: <syslog | deleted | level | password | death | god | new | rentgone>\r\n";

  two_arguments(argument, buf2, buf3);

  if (!*buf2) {
      send_to_char(usage, ch);
      send_to_char(options, ch);
      return;
  }

  if (!*buf3) {
      send_to_char(usage, ch);
      send_to_char(options, ch);
      return;
  }

  if (!str_cmp(buf2, "syslog")) {
      strcpy(filename, "../syslog");
  } else if (!strcmp(buf2, "deleted")) {
      strcpy(filename, "../log/DELETED");
  } else if (!strcmp(buf2, "level")) {
      strcpy(filename, "../log/advances");
  } else if (!strcmp(buf2, "password")) {
      strcpy(filename, "../log/badpws");
  } else if (!strcmp(buf2, "death")) {
      strcpy(filename, "../log/deaths");
  } else if (!strcmp(buf2, "god")) {
      strcpy(filename, "../log/godcmds");
  } else if (!strcmp(buf2, "new")) {
      strcpy(filename, "../log/newplayers");
  } else if (!strcmp(buf2, "rentgone")) {
      strcpy(filename, "../log/rentgone");
  } else {
      send_to_char("just spit it out will you, I don't have all day!\r\n", ch);
      return;
  }

  sprintf(buf, "egrep -i '%s' %s > ../log/log-file.grep", buf3, filename);

  system(buf);
      
  f = fopen("../log/log-file.grep","r");
  fread(msg, MAX_STRING_LENGTH, 1, f);
  page_string(ch->desc, msg, 1);
  fclose(f);
}

ACMD(do_ipcheck)
{
  FILE *f;
  char msg[MAX_STRING_LENGTH] = "\0";

  skip_spaces(&argument);
  
  if (!*argument) {
      send_to_char("Usage: ipcheck <ip-address>\r\n", ch);
      return;
  }

  sprintf(buf, "etc/listpfile a | grep '%s' > ../log/ip-check.grep", 
          argument);

  system(buf);

  f = fopen("../log/ip-check.grep", "r");
  fread(msg, MAX_STRING_LENGTH, 1, f);
  page_string(ch->desc, msg, FALSE);
  fclose(f);
}

ACMD(do_update)
{
  FILE *f;
  char *eol;
  long vnum;

  skip_spaces(&argument);

  if (!*argument) {
    send_to_char("Usage: update <vnum>\r\n", ch);
    return;
  }

  vnum = strtol(argument, &eol, 10);
  if (*eol != '\0') {
    send_to_char("Usage: update <vnum>\r\n", ch);
    return;
  }

  if (real_object(vnum) == -1) {
    send_to_char("No object by that number found.\r\n", ch);
    return;
  }

  f = fopen("etc/updates", "a");
  if (!f) {
    send_to_char("Cannot open etc/updates for writing!\r\n", ch);
    return;
  }
  fprintf(f, "%d\n", vnum);
  fclose(f);

  mudlog(NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE, "Object #%d flagged for updated by %s", vnum, GET_NAME(ch));
}

ACMD(do_clans)
{
    char cmnd[30], *args;

    cmnd[0] = 0;
    args = one_argument(argument, cmnd);

    if (!strncmp(cmnd, "list", strlen(cmnd))) {
        int i;

        send_to_char(" ID  NAME\r\n", ch);
        for (i = 0; i < clan_count; i++) {
            if (*clan_list[i].name) {
                sprintf(buf, " %-2d  %s\r\n", i, clan_list[i].name);
            } else {
                sprintf(buf, " %-2d  <disabled>\r\n", i);
            }
            send_to_char(buf, ch);
        }
    } else if (!strncmp(cmnd, "add", strlen(cmnd))) {
        time_t now = time(NULL);

        skip_spaces(&args);
        if (!*args) {
            send_to_char("What name should the new clan have?\r\n", ch);
            return;
        }
        clan_list = realloc(clan_list, ++clan_count * sizeof(ClanFileEntry));
        memset(&clan_list[clan_count - 1], 0, sizeof(ClanFileEntry));
        strncpy(clan_list[clan_count - 1].name, args, 22);
        clan_list[clan_count - 1].home = -1;
        strftime(clan_list[clan_count - 1].formed, 12,
                "%m/%d/%Y", localtime(&now));
        sprintf(buf, "Clan '%s' formed with id %d\r\n",
                clan_list[clan_count - 1].name, clan_count - 1);
        send_to_char(buf, ch);
        save_clans();
    } else if (!strncmp(cmnd, "del", strlen(cmnd))) {
        char id[30], *end;
        long clan_id;

        args = one_argument(args, id);
        skip_spaces(&args);
        clan_id = strtol(id, &end, 10);
        if (!*id || *end || clan_id < 0 || clan_id >= clan_count) {
            send_to_char("Invalid clan id!\r\n", ch);
            return;
        }
        clan_list[clan_id].name[0] = '\0';
        save_clans();
        sendChar(ch, "Clan %d disabled!\r\n", clan_id);
    } else if ((!strncmp(cmnd, "set", strlen(cmnd)) ||
               !strncmp(cmnd, "get", strlen(cmnd)))) {
        char id[30], opt[30], *end;
        long clan_id;

        *opt = 0;
        args = two_arguments(args, id, opt);
        skip_spaces(&args);
        clan_id = strtol(id, &end, 10);
        if (!*id || *end || clan_id < 0 || clan_id >= clan_count) {
            send_to_char("Invalid clan id!\r\n", ch);
            return;
        }

        if (cmnd[0] == 's' && !*args) {
            send_to_char("What do you want to set it to?\r\n", ch);
            return;
        }

        if (!strncmp(opt, "name", strlen(opt))) {
            if (cmnd[0] == 's')
                strncpy(clan_list[clan_id].name, args, 22);
            sprintf(buf, "Clan name: '%s'\r\n", clan_list[clan_id].name);
            send_to_char(buf, ch);
        } else if (!strncmp(opt, "home", strlen(opt))) {
            if (cmnd[0] == 's') {
                long room_id = strtol(args, &end, 10);
                if (*end) {
                    send_to_char("Invalid home room number!\r\n", ch);
                    return;
                }
                clan_list[clan_id].home = room_id;
            }
            sprintf(buf, "Clan home room: %d\r\n", clan_list[clan_id].home);
            send_to_char(buf, ch);
        } else if (!strncmp(opt, "formed", strlen(opt))) {
            if (cmnd[0] == 's')
                strncpy(clan_list[clan_id].formed, args, 11);
            sprintf(buf, "Clan formed: '%s'\r\n", clan_list[clan_id].formed);
            send_to_char(buf, ch);
        } else if (!strncmp(opt, "leaders", strlen(opt))) {
            char name[30];
            int i = 0;

            if (cmnd[0] == 's') for (i = 0; i < 5; i++) {
                args = one_argument(args, name);
                clan_list[clan_id].leaders[i] = 0;
                if (!*name) continue;
                clan_list[clan_id].leaders[i] = get_id_by_name(name);
                if (clan_list[clan_id].leaders[i] == -1) {
                    sprintf(buf, "Warning: Leader '%s' is unknown\r\n", name);
                    send_to_char(buf, ch);
                }
            }
            for (i = 0; i < 5; i++) {
                char *name = get_name_by_id(clan_list[clan_id].leaders[i]);
                sprintf(buf, "Leader #%d: '%s'\r\n", i,
                        clan_list[clan_id].leaders[i] ?
                            (name ? name : "<INVALID>") : "<UNSET>");
                send_to_char(buf, ch);
            }
        } else if (!strncmp(opt, "guard", strlen(opt))) {
            char guard_room[30],
                 guard_dir[30],
                 which_guard[30];
            long tmp1, tmp2, guardnum;
            args = one_argument(args, which_guard);
            guardnum = strtol(which_guard, &end, 10) - 1;
            if (!*which_guard || *end || guardnum < 0 || guardnum > 2) {
                send_to_char("Which clan guard do you mean?\r\n",
                        ch);
                return;
            }
            args = two_arguments(args, guard_room, guard_dir);
            skip_spaces(&args);
            if (cmnd[0] == 's') {
                if (!*guard_room || !*guard_dir || !*args) {
                    send_to_char("set guard expects three arguments: "
                                 "room, direction and guard's name\r\n", ch);
                    return;
                }
                tmp1 = strtol(guard_room, &end, 10);
                if (*end) {
                    send_to_char("Bad room number for guard\r\n", ch);
                    return;
                }
                if ((tmp2 = search_block(guard_dir, dirs, FALSE)) == -1) {
                    send_to_char("Which direction is that?\r\n", ch);
                    return;
                }
                clan_list[clan_id].guards[guardnum].room = tmp1;
                clan_list[clan_id].guards[guardnum].dir = tmp2 + 1;
                strncpy(clan_list[clan_id].guards[guardnum].name, args,
                        MAX_NAME_LENGTH);
            }
            sprintf(buf, "Clan guard #%d: room #%d, exit %s, name '%s'\r\n",
                    guardnum + 1,
                    clan_list[clan_id].guards[guardnum].room,
                    dirs[clan_list[clan_id].guards[guardnum].dir - 1],
                    clan_list[clan_id].guards[guardnum].name);
            send_to_char(buf, ch);
        } else if (!strncmp(opt, "color", strlen(opt))) {
            if (cmnd[0] == 's') {
                long tmp = strtol(args, &end, 10);
                if (*end || tmp < 0 || tmp > 25) {
                    send_to_char("Invalid colour!\r\n", ch);
                    return;
                }
                clan_list[clan_id].color = tmp;
            }
            sprintf(buf, "Clan color: &%02d&&%02d%02d&00\r\n",
                    clan_list[clan_id].color,
                    clan_list[clan_id].color,
                    clan_list[clan_id].color);
            send_to_char(buf, ch);
        } else {
            send_to_char("Bad option for set and get commands.\r\n", ch);
            return;
        }
        if (cmnd[0] == 's') save_clans();
    } else {
        send_to_char("Usage: clans <cmnd> <args>\r\n\r\n", ch);
        send_to_char("Where <cmnd> is one of:\r\n", ch);
        send_to_char("  list\r\n    List clan names and ids\r\n", ch);
        send_to_char("  add <name>\r\n    Add a new clan\r\n", ch);
        send_to_char("  del <id>\r\n    Disable a clan\r\n", ch);
        send_to_char("  set <id> <opt> <value>\r\n", ch);
        send_to_char("  get <id> <opt> <value>\r\n", ch);
        send_to_char("    With <opt> being one of:\r\n", ch);
        send_to_char("      name     Set the clan's name\r\n", ch);
        send_to_char("      home     Set the clan's home room\r\n", ch);
        send_to_char("      formed   Set the clan's formation date\r\n", ch);
        send_to_char("      leaders  Set the clan's leader(s)\r\n", ch);
        send_to_char("      guard    Set guard #, room, dir, name\r\n", ch);
        send_to_char("      color    Set the clan's color\r\n", ch);
    }
}

/* Player Search command shamelessly twisted from do_who, idea
   inspired by Brian Helms <bhelms1@gl.umbc.edu>'s show immortals
   snippet, and by Primacy <jmrobins@wired.uvm.edu>'s do_players.
   meer@meersan.net, 18 Jan 99
*/
#define PLAYER_FORMAT \
"format: players [minlev[-maxlev]] [-s] [-c classlist] [-r racelist] [-l lockernum]\r\n"

ACMD(do_players)
{
  FILE *fl;
  struct char_file_u player;
  char mode;
  int low = 0, high = LVL_IMPL;
  int showclass = 0, short_list = 0, showrace = 0, num_found = 0, done = 0;
  int lockerNum = 0;
  char tmp[64];

  skip_spaces(&argument);
  strcpy(buf, argument);

    if (!(fl = fopen(SYS_PLRFILES, "r+"))) {
      send_to_char("Can't open player file.", ch);
      return;
    }

  while (*buf) {
    half_chop(buf, arg, buf1);
    if (isdigit(*arg)) {
      sscanf(arg, "%d-%d", &low, &high);
      strcpy(buf, buf1);
    } else if (*arg == '-') {
      mode = *(arg + 1);       /* just in case; we destroy arg in the switch */
     switch (mode) {
      case 's':
        short_list = 1;
        strcpy(buf, buf1);
        break;
/*      case 'l':
        half_chop(buf1, arg, buf);
        sscanf(arg, "%d-%d", &low, &high);
        break;
*/
      case 'r':
        half_chop(buf1, arg, buf);
	showrace = find_race_bitvector(arg);
        break;
      case 'c':
        half_chop(buf1, arg, buf);
        showclass = find_class_bitvector(arg);
        break;
      case 'l':
	half_chop(buf1, arg, buf);
	lockerNum = atoi(arg);
	break;
      default:
        send_to_char(PLAYER_FORMAT, ch);
        return;
      }                         /* end of switch */

    } else {                    /* endif */
      send_to_char(PLAYER_FORMAT, ch);
      return;
    }
  }                             /* end while (parser) */

  send_to_char("\r\nPlayer Search:  ", ch);

  while (!done) {             /* begin while statement */
    fread(&player, sizeof(struct char_file_u), 1, fl);
    if (feof(fl)) {
      fclose(fl);
      done=TRUE;
    }
/*      if (PLR_FLAGGED(?player, PLR_DELETED))   <-----  Arrggh
        continue;
 */
      if (player.level < low || player.level > high)
        continue;
      if (showclass && !(showclass & (1 << player.class)))
        continue;
      if (showrace && !(showrace & (1 << player.race)))
        continue;
//      if (lockerNum && !(player->character != lockerNum))
//	continue;
      if (short_list) {
        sprintf(tmp, "%-12.12s%s",
               player.name,
               ((!(++num_found % 4)) ? "\r\n" : ""));
        strcat(buf, tmp);

      } else {
        num_found++;
        sprintf(tmp, "%s", player.name);
        strcat(buf, tmp);
        strcat(buf, "\r\n");
      }                           /* endif shortlist */

    if(num_found >= 300) {
	sprintf(tmp, "MAXIMUM NUMBER OF CHARACTERS REACHED");
	break;
    }

  }                             /* end of while */

  strcat(buf, "\r\n");

  if (num_found == 0)
    sprintf(tmp, "No players found.\r\n");
  else if (num_found == 1)
    sprintf(tmp, "One player found.\r\n");
  else
    sprintf(tmp, "%d players found.\r\n", num_found);
  send_to_char(tmp, ch);

  page_string(ch->desc, buf, 1);

}

ACMD(do_rawDamage) {
   CharData *vict;

  half_chop(argument, arg, buf);

  if (!*arg) {
    send_to_char("Send what to who?\r\n", ch);
    return;
  }
  if (!(vict = get_char_vis(ch, arg, 1))) {
    send_to_char(CONFIG_NOPERSON, ch);
    return;
  }

  if(!IS_NPC(vict)) {
      sendChar(ch, "This really only makes sense for NPCs.\r\n");
      return;
  }

  #define NOMINAL_AC -7
  int num_attacks, avg_dam, raw_damage;
  raw_damage = avgDamPerRound(vict, NOMINAL_AC);
  num_attacks = calculate_attack_count(vict);
  avg_dam = vict->mob_specials.damnodice * (vict->mob_specials.damsizedice + 1)/2 + GET_DAMROLL(vict);
  raw_damage = num_attacks*avg_dam + spellAttackStrength(vict)/100;

  sendChar(ch, "The raw damage of %s is %d.\r\n", GET_NAME(vict), raw_damage);
}
