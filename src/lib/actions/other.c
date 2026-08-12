
/* ************************************************************************
*   File: other.c                                       Part of CircleMUD *
*  Usage: Miscellaneous player-level commands                             *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

/* needed by sysdep.h to allow for definition of <sys/stat.h> */
#define __OTHER_C__

#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "util/utils.h"
#include "general/comm.h"
#include "actions/interpreter.h"
#include "general/handler.h"
#include "general/class.h"
#include "magic/spells.h"
#include "general/color.h"
#include "specials/house.h"
#include "general/objsave.h"
#include "actions/act.h"          /* ACMDs located within the act*.c files */
#include "scripts/dg_scripts.h"
#include "specials/flag_game.h"
#include "magic/skills.h"
#include "magic/sing.h"
#include "specials/mail.h"       /* for has_mail() */
#include "util/weather.h"


/* extern variables */
extern struct spell_info_type spell_info[];

/* extern procedures */
SPECIAL(shop_keeper);
ObjData *die( CharData *ch, CharData *killer, int pkill);

void water_death(CharData *ch);


// Returns true if ch is carrying unrentable objects.
int carrying_unrentables(CharData *ch, ObjData *obj) {
    int norent = 0;

    if (obj) {
        if(Crash_is_unrentable(obj)) {
            sendChar(ch, "Hasana tells you, 'You cannot store %s.'\r\n", OBJS(obj, ch));
            norent += 1;
        }
        norent += carrying_unrentables(ch, obj->contains);
        norent += carrying_unrentables(ch, obj->next_content);
    }

    return norent;
}


ACMD(do_quit)
{
  int i, norent = 0;

  if (IS_NPC(ch) || !ch->desc)
    return;

  if (subcmd != SCMD_QUIT && IS_MORTAL(ch))
    send_to_char("You have to type quit - no less, to quit!\r\n", ch);
  else if (GET_POS(ch) == POS_FIGHTING)
    send_to_char("No way!  You're fighting for your life!\r\n", ch);
  else if (IS_SET_AR(PLR_FLAGS(ch), PLR_HUNTED))
    send_to_char("Stay a while longer.. the fun is just beginning.\r\n", ch);
  else if (GET_POS(ch) < POS_STUNNED) {
    send_to_char("You die before your time...\r\n", ch);
    die(ch, NULL, 0);
  }
  else {
    // Don't let them quit with all their gear.
    if(IS_MORTAL(ch)) {
      if(ch->carrying && GET_LEVEL(ch) > 30) {
          sendChar(ch, "You are still carrying equipment! Drop it first.\r\n");
          return;
      }
      
      for(i = 0; i < NUM_WEARS; i++ ) {
        if(ch->equipment[i] && GET_LEVEL(ch) > 30) {
            sendChar(ch, "You are wearing equipment! Remove and drop it first.\r\n");
            return;
        }
        if(Crash_is_unrentable(ch->equipment[i])) {
            sendChar(ch, "Hasana tells you, 'You cannot store %s.'\r\n", OBJS(ch->equipment[i], ch));
            norent += 1;
        }
      }
    }

    norent += carrying_unrentables(ch, ch->carrying);
    
    if (norent) {
      sendChar(ch, "You cannot quit while carrying these items.\r\n");
      return;
    }

    if (!GET_INVIS_LEV(ch))
      act("$n has left the game.", TRUE, ch, 0, 0, TO_ROOM);
    mlog("%s has quit the game.", GET_NAME(ch));
    send_to_char("Goodbye, friend.. Come back soon!\r\n", ch);

    /*
     * kill off all sockets connected to the same player as the one who is
     * trying to quit.  Helps to maintain sanity as well as prevent duping.
     */
    {
      struct descriptor_data *d, *next_d;

      for (d = descriptor_list; d; d = next_d) {
	next_d = d->next;
	if (d == ch->desc)
	  continue;
	if (d->character && (GET_IDNUM(d->character) == GET_IDNUM(ch)))
	  close_socket(d);
      }
    }

    if(ch->desc)
        save_char(ch, ch->in_room);
    
    /* Used to verify the player wasn't duping - Vex. */
    if (ch->desc)
        ch->desc->connected = CON_MENU;
    
    extract_char(ch);       /* Char is saved in extract char */
  }
}



ACMD(do_save)
{
  if (IS_NPC(ch) || !ch->desc)
    return;

  if (cmd) {
    sprintf(buf, "Saving %s.\r\n", GET_NAME(ch));
    send_to_char(buf, ch);
  }
  save_char(ch, ch->in_room);
  Crash_crashsave(ch);
  if (ROOM_FLAGGED(ch->in_room, ROOM_HOUSE_CRASH))
    House_crashsave(world[ch->in_room].number);
}


/* generic function for commands which are normally spammed by players */
ACMD(do_not_here)
{
  send_to_char("Sorry, but you cannot do that here!\r\n", ch);
}



ACMD(do_sneak)
{
  struct affected_type af;
  byte percent;

  if (IS_NPC(ch) || !GET_SKILL(ch, SKILL_SNEAK)) {
    send_to_char("You have no idea how to do that.\r\n", ch);
    return;
  }
  send_to_char("Okay, you'll try to move silently for a while.\r\n", ch);
  if (IS_AFFECTED(ch, AFF_SNEAK))
    affect_from_char(ch, SKILL_SNEAK);

  percent = number(1, 101);     /* 101% is a complete failure */

  if (percent > GET_SKILL(ch, SKILL_SNEAK) + dex_app_skill[GET_DEX(ch)].sneak)
    return;

  af.type = SKILL_SNEAK;
  af.duration = GET_LEVEL(ch);
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_SNEAK;
  affect_to_char(ch, &af);
  
  if(percentSuccess(5))
      advanceSkill( ch, SKILL_SNEAK, 60, "You feel lighter on your feet.", TRUE, TRUE, FALSE );
}



ACMD(do_hide)
{
  byte percent;

  if (IS_NPC(ch) || !GET_SKILL(ch, SKILL_HIDE)) {
    send_to_char("You have no idea how to do that.\r\n", ch);
    return;
  }
  send_to_char("You attempt to hide yourself.\r\n", ch);

  if (IS_AFFECTED(ch, AFF_HIDE))
    REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_HIDE);

  percent = number(1, 101);     /* 101% is a complete failure */

  if (percent > GET_SKILL(ch, SKILL_HIDE) + dex_app_skill[GET_DEX(ch)].hide)
    return;

  SET_BIT_AR(AFF_FLAGS(ch), AFF_HIDE);
  
  if(percentSuccess(5))
      advanceSkill( ch, SKILL_HIDE, 60, "You feel invisible.", TRUE, TRUE, FALSE );
}




ACMD(do_steal)
{
  CharData *vict;
  ObjData *obj;
  char vict_name[240];
  char obj_name[240];
  int percent, gold, eq_pos, pcsteal = 0, ohoh = 0;

  if (IS_NPC(ch) || !GET_SKILL(ch, SKILL_STEAL)) {
    send_to_char("You have no idea how to do that.\r\n", ch);
    return;
  }
  if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
    send_to_char("This room just has such a peaceful, easy feeling...\r\n", ch);
    return;
  }
  two_arguments(argument, obj_name, vict_name);

  if (!(vict = get_char_room_vis(ch, vict_name))) {
    send_to_char("Steal what from who?\r\n", ch);
    return;
  } else if (vict == ch) {
    send_to_char("Come on now, that's rather stupid!\r\n", ch);
    return;
  }
  /* 101% is a complete failure */
  percent = number(1, 101) - dex_app_skill[GET_DEX(ch)].p_pocket;

  if (GET_POS(vict) < POS_SLEEPING)
    percent = -1;       /* ALWAYS SUCCESS */

  /* NO NO With Imp's and Shopkeepers! */
  if ((GET_LEVEL(vict) >= LVL_IMMORT) || (IS_NPC(vict) && MOB_FLAGGED(vict, MOB_AWARE)))
    percent = 101;      /* Failure */

  if (str_cmp(obj_name, "coins") && str_cmp(obj_name, "gold")) {

    if (!(obj = get_obj_in_list_vis(vict, obj_name, vict->carrying))) {

      for (eq_pos = 0; eq_pos < NUM_WEARS; eq_pos++)
	if (vict->equipment[eq_pos] &&
	    (isname(obj_name, vict->equipment[eq_pos]->name)) &&
	    CAN_SEE_OBJ(ch, vict->equipment[eq_pos])) {
	  obj = vict->equipment[eq_pos];
	  break;
	}
      if (!obj) {
	act("$E hasn't got that item.", FALSE, ch, 0, vict, TO_CHAR);
	return;
      } else {         /* It is equipment */
	if ((GET_POS(vict) > POS_STUNNED)) {
	send_to_char("Steal the equipment?  Impossible!\r\n", ch);
	return;
	} else {
	act("You unequip $p and steal it.", FALSE, ch, obj, 0, TO_CHAR);
	act("$n steals $p from $N.", FALSE, ch, obj, vict, TO_NOTVICT);
	obj_to_char(unequip_char(vict, eq_pos), ch);
	}
      }
    } else {          /* obj found in inventory */

      percent += GET_OBJ_WEIGHT(obj);     /* Make heavy harder */

      if (AWAKE(vict) && (percent > GET_SKILL(ch, SKILL_STEAL))) {
	ohoh = TRUE;
	act("Oops..", FALSE, ch, 0, 0, TO_CHAR);
	act("$n tried to steal something from you!", FALSE, ch, 0, vict, TO_VICT);
	act("$n tries to steal something from $N.", TRUE, ch, 0, vict, TO_NOTVICT);
      } else {         /* Steal the item */
	if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch))) {
	if ((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) < CAN_CARRY_W(ch)) {
	obj_from_char(obj);
	obj_to_char(obj, ch);
	send_to_char("Got it!\r\n", ch);
        
        if(percentSuccess(5))
            advanceSkill( ch, SKILL_STEAL, 60, "You've really developed your touch.", TRUE, TRUE, FALSE );

	  } 
	} else
	send_to_char("You cannot carry that much.\r\n", ch);
      }
    }
  } else {          /* Steal some coins */
    if (AWAKE(vict) && (percent > GET_SKILL(ch, SKILL_STEAL))) {
      ohoh = TRUE;
      act("Oops..", FALSE, ch, 0, 0, TO_CHAR);
      act("$n tried to steal gold from you!", FALSE, ch, 0, vict, TO_VICT);
      act("$n tries to steal gold from $N.", TRUE, ch, 0, vict, TO_NOTVICT);
    } else {
      /* Steal some gold coins */
      gold = (int) ((GET_GOLD(vict) * number(1, 10)) / 100);
      gold = MIN(1782, gold);
      if (gold > 0) {
	GET_GOLD(ch) += gold;
	GET_GOLD(vict) -= gold;
	if (gold > 1) {
	sprintf(buf, "Bingo!  You got %d gold coins.\r\n", gold);
	send_to_char(buf, ch);
	} else {
	send_to_char("You manage to pry one gold coin from the purse.\r\n", ch);
	}
    
    if(percentSuccess(5))
        advanceSkill( ch, SKILL_STEAL, 60, "You've really developed your touch.", TRUE, TRUE, FALSE );

      } else {
	send_to_char("You couldn't get any gold...\r\n", ch);
      }
    }
  }

  if (ohoh && IS_NPC(vict) && AWAKE(vict))
    hit(vict, ch, TYPE_UNDEFINED);
}



ACMD(do_practice)
{
  int save_vis;
  int skill_num;

  if (IS_NPC(ch))
    return;

  /* Elemancer Self-Practice Logic */
  if (IS_ELEMANCER(ch) && ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
      skip_spaces(&argument);

      if (*argument) {
          skill_num = find_skill_num(argument);
          
          if (skill_num < 1 || 
              GET_LEVEL(ch) < spell_info[skill_num].min_level[(int)GET_CLASS(ch)]) {
              sprintf(buf, "You do not know of that %s.\r\n", SPLSKL(ch));
              send_to_char(buf, ch);
              return;
          }

          if (perform_practice(ch, skill_num)) {
              send_to_char("You center yourself, drawing inspiration from the universal ebb and flow...\r\n", ch);
              act("$n centers $mself, seemingly drawing power from the air around $m.", FALSE, ch, 0, 0, TO_ROOM);
          }
          return;
      }
  }

  save_vis = GET_INVIS_LEV(ch);
  GET_INVIS_LEV(ch) = LVL_IMPL + 1;

  list_skills(ch);

  GET_INVIS_LEV(ch) = save_vis;
}


ACMD(do_visible)
{
  if (GET_INVIS_LEV(ch) > 0) {
    perform_immort_vis(ch);
    return;
  }
  if IS_AFFECTED(ch, AFF_INVISIBLE) {
    appear(ch);
    send_to_char("You break the spell of invisibility.\r\n", ch);
  } else
    send_to_char("You are already visible.\r\n", ch);
}



ACMD(do_title)
{
  skip_spaces(&argument);
  delete_doubledollar(argument);

  if (IS_NPC(ch))
    return;
    
  if (strlen(argument) > MAX_TITLE_LENGTH) {
    sendChar(ch, "Sorry, titles can't be longer than %d characters.\r\n", MAX_TITLE_LENGTH);
    return;
  }
  if (!*argument) {
      sendChar(ch, "Your title is: %s\r\n", GET_TITLE(ch));
      return;
  }
  
  if( !PLR_FLAGGED(ch, PLR_SHUNNED)){
      set_title(ch, argument);
      sendChar(ch, "Okay, you're now %s %s.\r\n", GET_NAME(ch), GET_TITLE(ch));
  }
  else {
      sendChar(ch, "You are unable to change your title.\r\n");
  }
}


int perform_group(CharData *ch, CharData *vict)
{
  if (IS_AFFECTED(vict, AFF_GROUP) || !CAN_SEE(ch, vict))
    return 0;

  SET_BIT_AR(AFF_FLAGS(vict), AFF_GROUP);
  if (ch != vict)
    act("$N is now a member of your group.", FALSE, ch, 0, vict, TO_CHAR);
  act("You are now a member of $n's group.", FALSE, ch, 0, vict, TO_VICT);
  act("$N is now a member of $n's group.", FALSE, ch, 0, vict, TO_NOTVICT);
  return 1;
}


void print_group(CharData *ch)
{
  CharData *k;
  struct follow_type *f;

  if (!IS_AFFECTED(ch, AFF_GROUP))
    send_to_char("But you are not the member of a group!\r\n", ch);
  else {
    send_to_char("Your group consists of:\r\n", ch);

    k = (ch->master ? ch->master : ch);

    if (IS_AFFECTED(k, AFF_GROUP)) {
      sprintf(buf, "     [%3d %s] %-25s\r\n", GET_LEVEL(k),
	      CLASS_ABBR(k), GET_NAME(k));
      send_to_char(buf, ch);
    }
    for (f = k->followers; f; f = f->next) {
      if (!IS_AFFECTED(f->follower, AFF_GROUP))
	continue;

      sprintf(buf, "     [%3d %s] %-25s\r\n", GET_LEVEL(f->follower),
	      CLASS_ABBR(f->follower), GET_NAME(f->follower));
      send_to_char(buf, ch);
    }
  }
}



ACMD(do_group)
{
  CharData *vict;
  struct follow_type *f;
  int found;

  one_argument(argument, buf);

  if (!*buf) {
    print_group(ch);
    return;
  }
  if (ch->master) {
    act("You can not enroll group members without being head of a group.",
	FALSE, ch, 0, 0, TO_CHAR);
    return;
  }
  if (!str_cmp(buf, "all")) {
    perform_group(ch, ch);
    for (found = 0, f = ch->followers; f; f = f->next)
      found += perform_group(ch, f->follower);
    if (!found)
      send_to_char("Everyone following you is already in your group.\r\n", ch);
    return;
  }
  if (!(vict = get_char_room_vis(ch, buf)))
    send_to_char(CONFIG_NOPERSON, ch);
  else if ((vict->master != ch) && (vict != ch))
    act("$N must follow you to enter your group.", FALSE, ch, 0, vict, TO_CHAR);
  else {
    if (!IS_AFFECTED(vict, AFF_GROUP))
      perform_group(ch, vict);
    else {
      if (ch != vict)
	act("$N is no longer a member of your group.", FALSE, ch, 0, vict, TO_CHAR);
      act("You have been kicked out of $n's group!", FALSE, ch, 0, vict, TO_VICT);
      act("$N has been kicked out of $n's group!", FALSE, ch, 0, vict, TO_NOTVICT);
      REMOVE_BIT_AR(AFF_FLAGS(vict), AFF_GROUP);
    }
  }
}



ACMD(do_ungroup)
{
  struct follow_type *f, *next_fol;
  CharData *vict;

  one_argument(argument, buf);

  if (!*buf) {
    if (ch->master || !(IS_AFFECTED(ch, AFF_GROUP))) {
      send_to_char("But you lead no group!\r\n", ch);
      return;
    }
    sprintf(buf, "Are you sure you want to disband your group?  ");
    send_to_char(buf, ch);
    /*
     * We don't want to define a new procedure in act.other.c
     * or a new macro in utils.h so...
     */
/*
    if (!GetYesNo(ch))
      return;
*/

    for (f = ch->followers; f; f = next_fol) {
      next_fol = f->next;
      if (IS_AFFECTED(f->follower, AFF_GROUP)) {
	REMOVE_BIT_AR(AFF_FLAGS(f->follower), AFF_GROUP);
	act("$N has disbanded the group.", TRUE, f->follower, 0, ch, TO_CHAR);
	if (!IS_AFFECTED(f->follower, AFF_CHARM))
	  stop_follower(f->follower);
      }
    }

    REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_GROUP);
    send_to_char("You disband the group.\r\n", ch);
    return;
  }
  if (!(vict = get_char_room_vis(ch, buf))) {
    send_to_char(CONFIG_NOPERSON, ch);
    return;
  }
  if ((vict->master != ch) && (vict != ch)) {
    send_to_char("That person is not in your group.\r\n", ch);
    return;
  }
  if (!IS_AFFECTED(vict, AFF_GROUP)) {
    send_to_char("That person is not in your group.\r\n", ch);
    return;
  }
  if (ch != vict)
    act("$N is no longer a member of your group.", FALSE, ch, 0, vict, TO_CHAR);
  act("You have been kicked out of $n's group!", FALSE, ch, 0, vict, TO_VICT);
  act("$N has been kicked out of $n's group!", FALSE, ch, 0, vict, TO_NOTVICT);
  REMOVE_BIT_AR(AFF_FLAGS(vict), AFF_GROUP);
  if (!IS_AFFECTED(vict, AFF_CHARM))
    stop_follower(vict);
}



ACMD(do_report)
{
  CharData *k;
  struct follow_type *f;
  char buf[255];

  if (!IS_AFFECTED(ch, AFF_GROUP)) {
    send_to_char("But you are not a member of any group!\r\n", ch);
    return;
  }
  sprintf(buf, "%s reports: %d/%dH, %d/%dM, %d/%dV\r\n",
	  GET_NAME(ch), GET_HIT(ch), GET_MAX_HIT(ch),
	  GET_MANA(ch), GET_MAX_MANA(ch),
	  GET_MOVE(ch), GET_MAX_MOVE(ch));

  k = (ch->master ? ch->master : ch);

  for (f = k->followers; f; f = f->next)
    if (IS_AFFECTED(f->follower, AFF_GROUP) && f->follower != ch)
      send_to_char(buf, f->follower);

  if (k != ch)
    send_to_char(buf, k);

  send_to_char("You report to the group.\r\n", ch);
}



ACMD(do_split)
{
  int amount, num, share;
  CharData *k;
  struct follow_type *f;

  if (IS_NPC(ch))
    return;

  one_argument(argument, buf);

  if (is_number(buf)) {
    amount = atoi(buf);
    if (amount <= 0) {
      send_to_char("Sorry, you can't do that.\r\n", ch);
      return;
    }
    if (amount > GET_GOLD(ch)) {
      send_to_char("You haven't got that much gold.\r\n", ch);
      return;
    }
    k = (ch->master ? ch->master : ch);

    if (IS_AFFECTED(k, AFF_GROUP) && (k->in_room == ch->in_room))
      num = 1;
    else
      num = 0;

    for (f = k->followers; f; f = f->next)
      if (IS_AFFECTED(f->follower, AFF_GROUP) &&
	  (!IS_NPC(f->follower)) &&
	  (f->follower->in_room == ch->in_room))
	num++;

    if (num && IS_AFFECTED(ch, AFF_GROUP))
      share = amount / num;
    else {
      send_to_char("With whom do you wish to share your gold?\r\n", ch);
      return;
    }

    GET_GOLD(ch) -= share * (num - 1);

    if (IS_AFFECTED(k, AFF_GROUP) && (k->in_room == ch->in_room)
	&& !(IS_NPC(k)) && k != ch) {
      GET_GOLD(k) += share;
      sprintf(buf, "%s splits %d coins; you receive %d.\r\n", GET_NAME(ch),
	      amount, share);
      send_to_char(buf, k);
    }
    for (f = k->followers; f; f = f->next) {
      if (IS_AFFECTED(f->follower, AFF_GROUP) &&
	  (!IS_NPC(f->follower)) &&
	  (f->follower->in_room == ch->in_room) &&
	  f->follower != ch) {
	GET_GOLD(f->follower) += share;
	sprintf(buf, "%s splits %d coins; you receive %d.\r\n", GET_NAME(ch),
		amount, share);
	send_to_char(buf, f->follower);
      }
    }
    sprintf(buf, "You split %d coins among %d members -- %d coins each.\r\n",
	    amount, num, share);
    send_to_char(buf, ch);
  } else {
    send_to_char("How many coins do you wish to split with your group?\r\n", ch);
    return;
  }
}



ACMD(do_use)
{
  ObjData *mag_item;
  int equipped = 1;

  half_chop(argument, arg, buf);
  if (!*arg) {
    sprintf(buf2, "What do you want to %s?\r\n", CMD_NAME);
    send_to_char(buf2, ch);
    return;
  }
  mag_item = GET_EQ(ch, WEAR_HOLD);

  if (!mag_item || !isname(arg, mag_item->name)) {
    switch (subcmd) {
    case SCMD_RECITE:
    case SCMD_QUAFF:
      equipped = 0;
      if (!(mag_item = get_obj_in_list_vis(ch, arg, ch->carrying))) {
	sprintf(buf2, "You don't seem to have %s %s.\r\n", AN(arg), arg);
	send_to_char(buf2, ch);
	return;
      }
      break;
    case SCMD_USE:
      sprintf(buf2, "You don't seem to be holding %s %s.\r\n", AN(arg), arg);
      send_to_char(buf2, ch);
      return;
    default:
      mlog("SYSERR: Unknown subcmd %d passed to do_use.", subcmd);
      return;
    }
  }
  switch (subcmd) {
  case SCMD_QUAFF:
    if (GET_OBJ_TYPE(mag_item) != ITEM_POTION) {
      send_to_char("You can only quaff potions.", ch);
      return;
    }
    break;
  case SCMD_RECITE:
    if (GET_OBJ_TYPE(mag_item) != ITEM_SCROLL) {
      send_to_char("You can only recite scrolls.", ch);
      return;
    }
    break;
  case SCMD_USE:
    if ((GET_OBJ_TYPE(mag_item) != ITEM_WAND) &&
	(GET_OBJ_TYPE(mag_item) != ITEM_STAFF)) {
      send_to_char("You can't seem to figure out how to use it.\r\n", ch);
      return;
    }
    break;
  }

  mag_objectmagic(ch, mag_item, buf);
}



ACMD(do_wimpy)
{
  int wimp_lev;

  one_argument(argument, arg);

  if (!*arg) {
    if (GET_WIMP_LEV(ch) == 0)
      send_to_char("You are not currently wimpy.\r\n", ch);
    else {
      sprintf(buf, "You are currently wimpy to %d hit points.\r\n",
	      GET_WIMP_LEV(ch));
      send_to_char(buf, ch);
    }
    return;
  }
  if (isdigit(*arg)) {
    if ((wimp_lev = atoi(arg))) {
      if (wimp_lev < 0)
	send_to_char("Heh, heh, heh.. we are jokers, aren't we?\r\n", ch);
      else if (wimp_lev > GET_MAX_HIT(ch))
	send_to_char("That doesn't make much sense, now does it?\r\n", ch);
      else if (wimp_lev > (GET_MAX_HIT(ch) >> 1))
	send_to_char("You can't set your wimpy level above 50% of your hit points.\r\n", ch);
      else {
	sprintf(buf, "Okay, you'll wimp out if you drop below %d hit points.\r\n",
		wimp_lev);
	send_to_char(buf, ch);
	GET_WIMP_LEV(ch) = wimp_lev;
      }
    } else {
      send_to_char("Okay, you'll now tough it out to the bitter end.\r\n", ch);
      GET_WIMP_LEV(ch) = 0;
    }
  } else
    send_to_char("Specify a number of hit points.\r\n", ch);

}


ACMD(do_display)
{
  int i;

  if (IS_NPC(ch)) {
    send_to_char("Mosters don't need displays.  Go away.\r\n", ch);
    return;
  }
  skip_spaces(&argument);

  if (!*argument) {
    send_to_char("The following options are available:\r\n", ch);
    for (i = 0; i < NUM_PRF_FLAGS; i++) {
      if (!preference_bits[i][0])
	continue;
      sprintf(buf, "  %-14s%s\r\n", preference_bits[i],
	      PRF_FLAGGED(ch, i) ? "ON" : "OFF");
      send_to_char(buf, ch);
    }
    send_to_char("Usage: display <option>\r\n", ch);
    return;
  }
  if ((i = search_block(argument, preference_bits, FALSE)) == -1) {
    send_to_char("That is not a valid display option.\r\n", ch);
    return;
  }
  if (PRF_FLAGGED(ch, i)) {
    REMOVE_BIT_AR(PRF_FLAGS(ch), i);
    send_to_char(CONFIG_OK, ch);
  } else {
    SET_BIT_AR(PRF_FLAGS(ch), i);
    send_to_char(CONFIG_OK, ch);
  }
}



ACMD(do_gen_write)
{
  FILE *fl;
  char *tmp, buf[MAX_STRING_LENGTH];
  const char *filename;
  struct stat fbuf;

  switch (subcmd) {
  case SCMD_BUG:
    filename = BUG_FILE;
    break;
  case SCMD_TYPO:
    filename = TYPO_FILE;
    break;
  case SCMD_IDEA:
    filename = IDEA_FILE;
    break;
  default:
    return;
  }

  if (IS_NPC(ch)) {
    send_to_char("Monsters can't have ideas - Go away.\r\n", ch);
    return;
  }

  skip_spaces(&argument);
  delete_doubledollar(argument);

  if (!*argument) {
    send_to_char("That must be a mistake...\r\n", ch);
    return;
  }
  sprintf(buf, "%s %s: %s", GET_NAME(ch), CMD_NAME, argument);
  mudlog(CMP, LVL_IMMORT, FALSE, buf);

  if (stat(filename, &fbuf) < 0) {
    perror("Error statting file");
    return;
  }
  if (fbuf.st_size >= CONFIG_MAX_FILESIZE) {
    send_to_char("Sorry, the file is full right now.. try again later.\r\n", ch);
    return;
  }
  if (!(fl = fopen(filename, "a"))) {
    perror("do_gen_write");
    send_to_char("Could not open the file.  Sorry.\r\n", ch);
    return;
  }
  gecho("%s\n", argument);
  
  time_t t = time(0);
  tmp = asctime(localtime(&t));
  *(tmp + strlen(tmp) - 1) = '\0';

  fprintf(fl, "%s (%s) [%5d] %s\n", GET_NAME(ch), tmp,
	  world[ch->in_room].number, argument);
  fclose(fl);
  send_to_char("Okay.  Thanks!\r\n", ch);
}

#define TOG_OFF 0
#define TOG_ON  1

ACMD(do_gen_tog)
{
  long result;
  const char *onset, *offset;

  if (IS_NPC(ch))
    return;

  switch (subcmd) {
  case SCMD_NOSUMMON:
    result = PRF_TOG_CHK(ch, PRF_SUMMONABLE);
    onset = "You are now summonable.";
    offset = "You are no longer summonable.";
    break;
  case SCMD_NOHASSLE:
    result = PRF_TOG_CHK(ch, PRF_NOHASSLE);
    onset = "Nohassle disabled.";
    offset = "Nohassle enabled.";
    break;
  case SCMD_BRIEF:
    result = PRF_TOG_CHK(ch, PRF_BRIEF);
    onset = "Brief mode off.";
    offset = "Brief mode on.";
    break;
  case SCMD_COMPACT:
    result = PRF_TOG_CHK(ch, PRF_COMPACT);
    onset = "Compact mode off.";
    offset = "Compact mode on.";
    break;
  case SCMD_NOTELL:
    result = PRF_TOG_CHK(ch, PRF_NOTELL);
    onset = "You can now hear tells.";
    offset = "You are now deaf to tells.";
    break;
  case SCMD_NOAUCTION:
    result = PRF_TOG_CHK(ch, PRF_NOAUCT);
    onset = "You can now hear auctions.";
    offset = "You are now deaf to auctions.";
    break;
  case SCMD_DEAF:
    result = PRF_TOG_CHK(ch, PRF_DEAF);
    onset = "You can now hear shouts.";
    offset = "You are now deaf to shouts.";
    break;
  case SCMD_NORPLAY:
    result = PRF_TOG_CHK(ch, PRF_NORPLAY);
    onset = "You can now hear roleplay.";
    offset = "You are now deaf to roleplay.";
    break;
  case SCMD_NOGRATZ:
    result = PRF_TOG_CHK(ch, PRF_NOGRATZ);
    onset = "You can now hear gratz.";
    offset = "You are now deaf to gratz.";
    break;
  case SCMD_NOWIZ:
    result = PRF_TOG_CHK(ch, PRF_NOWIZ);
    onset = "You can now hear the God Channel.";
    offset = "You are now deaf to the God Channel.";
    break;
  case SCMD_QUEST:
    result = PRF_TOG_CHK(ch, PRF_QUEST);
    onset = "You are now part of the Quest.";
    offset = "You are no longer part of the Quest.";
    break;
  case SCMD_ROOMFLAGS:
    result = PRF_TOG_CHK(ch, PRF_SHOWVNUMS);
    onset = "You will now see room flags.";
    offset = "You will no longer see room flags.";
    break;
  case SCMD_NOREPEAT:
    result = PRF_TOG_CHK(ch, PRF_NOREPEAT);
    onset = "You will now have your communication repeated.";
    offset = "You will no longer have your communication repeated.";
    break;
  case SCMD_HOLYLIGHT:
    result = PRF_TOG_CHK(ch, PRF_HOLYLIGHT);
    onset = "HolyLight mode off.";
    offset = "HolyLight mode on.";
    break;
  case SCMD_SLOWNS:
    result = (CONFIG_NS_IS_SLOW = !CONFIG_NS_IS_SLOW);
    onset = "Nameserver is fast.";
    offset = "Nameserver is slow.";
    break;
  case SCMD_AUTOEXIT:
    result = PRF_TOG_CHK(ch, PRF_AUTOEXIT);
    onset = "Autoexits disabled.";
    offset = "Autoexits enabled.";
    break;
  case SCMD_AFK:
      result = PRF_TOG_CHK(ch, PRF_AFK);
      onset = "AFK disabled.";
      offset = "AFK enabled.";
      if (IS_AFK(ch))
          act("$n is now AFK.", TRUE, ch, 0, 0, TO_ROOM);
      else {
          act("$n has returned to the keyboard.", TRUE, ch, 0, 0, TO_ROOM);
          ch->char_specials.timer = 0;
      }
      break;


  default:
    mlog("SYSERR: Unknown subcmd %d in do_gen_tog.", subcmd);
    return;
  }

  if (result)
    send_to_char(offset, ch);
  else
    send_to_char(onset, ch);
    
  send_to_char("\r\n", ch);
}

#define CAN_GO2(room, door) (world[room].dir_option[door] && \
			 (world[room].dir_option[door]->to_room != NOWHERE) && \
			 !IS_SET(world[room].dir_option[door]->exit_info, EX_CLOSED))

ACMD(do_ventriloquate)
{
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    int room_dir, sound_dir, target_room_num;
    struct char_data *vict, *next_vict;
    bool found = FALSE;

    /* Check for Elemancer class or skill */
    if (!IS_ELEMANCER(ch) && GET_LEVEL(ch) < LVL_IMMORT) {
        send_to_char("You are not musically inclined enough to do that.\r\n", ch);
        return;
    }

    if (GET_SKILL(ch, SPELL_VENTRILOQUATE) == 0) {
        send_to_char("You don't know how to throw your voice.\r\n", ch);
        return;
    }

    two_arguments(argument, arg1, arg2);

    if (!*arg1 || !*arg2) {
        send_to_char("Usage: ventriloquate <room direction> <sound direction>\r\n", ch);
        return;
    }

    /* Parse Room Direction */
    if ((room_dir = search_block(arg1, dirs, FALSE)) < 0) {
        send_to_char("That is not a valid direction.\r\n", ch);
        return;
    }

    /* Check if exit exists */
    if (!EXIT(ch, room_dir) || (target_room_num = EXIT(ch, room_dir)->to_room) == NOWHERE) {
        send_to_char("There is no room in that direction.\r\n", ch);
        return;
    }
    
    if (IS_SET(EXIT(ch, room_dir)->exit_info, EX_CLOSED)) {
        send_to_char("You can't throw your voice through a closed door.\r\n", ch);
        return;
    }

    /* Parse Sound Direction */
    if ((sound_dir = search_block(arg2, dirs, FALSE)) < 0) {
        send_to_char("That is not a valid sound direction.\r\n", ch);
        return;
    }

    /* Cost */
    if (GET_MANA(ch) < 20) {
        send_to_char("You don't have enough mana.\r\n", ch);
        return;
    }
    GET_MANA(ch) -= 20;

    send_to_char("You throw your voice...\r\n", ch);

    /* Loop through mobs */
    for (vict = world[target_room_num].people; vict; vict = next_vict) {
        next_vict = vict->next_in_room;

        if (IS_NPC(vict) && IS_SET_AR(MOB_FLAGS(vict), MOB_AGGRESSIVE) && !IS_SET_AR(MOB_FLAGS(vict), MOB_SENTINEL)) {
             /* Check Saving Throw. */
             if (!mag_savingthrow(vict, SAVING_SPELL)) {
                 if (CAN_GO2(target_room_num, sound_dir)) {
                    perform_move(vict, sound_dir, 0);
                    found = TRUE;
                 }
             }
        }
    }
    
    if (found) {
        advanceSkill(ch, SPELL_VENTRILOQUATE, 20, "You feel better at throwing your voice.", TRUE, TRUE, TRUE);
    }
    
    WAIT_STATE(ch, PULSE_VIOLENCE);
}
