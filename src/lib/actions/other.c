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
    if(norent) {
        sendChar(ch, "You cannot exit the game with some of your items.  Please drop them first.\r\n");
        return;
    }

    if(!GET_INVIS_LEV(ch))
      act("$n has left the game.", TRUE, ch, 0, 0, TO_ROOM);
    mudlog( NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE, "%s has quit the game.", GET_NAME(ch));
    send_to_char("The spirit of Hasana escorts you to a safe place to camp.\r\n"
            "Please expect to rent in the inns once you've grown to a higher level adventurer.\r\n"
            "Goodbye, friend.. Come back soon!\r\n", ch);

    crashRentSave(ch, 0);
    extract_char(ch);		/* Char is saved in extract char */
  }
}

ACMD(do_recall) {
    if(GET_LEVEL(ch) > 25) {
        sendChar(ch, "Hasana refuses to heed your prayers.  Seek your own salvation.\r\n");
        return;
    }
    else {
        sendChar(ch, "In your meekness, the goddess Hasana aids you by whisking you away from harm.\r\n");
        cast_spell(ch, ch, 0, SPELL_WORD_OF_RECALL);
        WAIT_STATE(ch, 2*PULSE_VIOLENCE);
    }
}

ACMD(do_locate) {
    
}

ACMD(do_rub)
{

	if ( IS_NPC(ch) )
		return;

	if( !affected_by_spell(ch, SPELL_BLINDNESS) && !affected_by_spell(ch, SKILL_DUST) 
		&& !IS_AFFECTED(ch, AFF_BLIND) )
	{
			act("You rub the sun out of your eyes.", FALSE, ch, 0, 0, TO_CHAR);
			return;
	}

	if( (20 + 20 * !FIGHTING(ch)) > number(1, 100) ) 
	{
		if( affected_by_spell( ch, SPELL_BLINDNESS ))
		{
			affect_from_char( ch, SPELL_BLINDNESS );
			act("You rub away the magical effects of blindness.", FALSE, ch, 0, 0, TO_CHAR);
			act("$n rubs vigorously at his eyes.", FALSE, ch, 0, 0, TO_ROOM);
		}
		if( affected_by_spell( ch, SKILL_DUST ))
		{
			affect_from_char( ch, SKILL_DUST );
			act("You scrape the dust off your eyes.", FALSE, ch, 0, 0, TO_CHAR);
			act("$n scrapes at his eyes vigorously to remove the dust.", FALSE, ch, 0, 0, TO_ROOM);
		}
		if(IS_AFFECTED(ch, AFF_BLIND) && affected_by_spell( ch, SKILL_DIRTY_TACTICS) )
		{
			affect_from_char( ch, SKILL_DIRTY_TACTICS );
   			act("You wipe the blood from your eyes.", FALSE, ch, 0, 0, TO_CHAR);
			sprintf(buf, "$n wipes away the blood in %s eyes.",	GET_NAME(ch), HSHR(ch));
			act(buf, FALSE, ch, 0, 0, TO_ROOM);
		}
	}
	else act("You rub at your eyes, but to no avail.", FALSE, ch, 0, 0, TO_CHAR);
	
	WAIT_STATE(ch, SET_STUN(1));
		
}

ACMD(do_vote)
{
	int vote = 0;

	extern int poll_running;

	if( !poll_running ) 
	{
		send_to_char( "There is no poll running right now.\r\n", ch);
		return;
	}

	if( GET_VOTE(ch) > 0 ) {
		send_to_char( "Let's not be a flip-flop.\r\n", ch);
		return;
	}

	one_argument(argument, arg);
	vote = atoi(arg);

	if (vote > 0  &&  vote < 6)
	{
		GET_VOTE(ch) = vote;
		sprintf(buf, "You have voted for %d.\r\n", vote );
        send_to_char(buf, ch);
	}
}



ACMD(do_save)
{
    void save_aliases( CharData *ch );

    if( IS_NPC(ch) || !ch->desc ) return;

    if( cmd ){
        sprintf(buf, "Saving %s.\r\n", GET_NAME(ch));
        send_to_char(buf, ch);
    }
    
    affect_total(ch);
    save_char( ch, NOWHERE );
    Crash_crashsave( ch ); /* This saves aliases as well - Vex. */
    save_locker_of( ch );

    if( ROOM_FLAGGED( ch->in_room, ROOM_HOUSE_CRASH ))
        House_crashsave( world[ch->in_room].number );
}

/* generic function for commands which are normally overridden by
   special procedures - i.e., shop commands, mail commands, etc. */
ACMD(do_not_here)
{
  send_to_char("Sorry, but you cannot do that here!\r\n", ch);
}

ACMD(do_practice)
{
  char arg[MAX_INPUT_LENGTH];

  if (IS_NPC(ch))
    return;

  one_argument(argument, arg);

  if (*arg)
    sendChar(ch, "You can only practice skills in your guild.\r\n");
  else
    list_skills(ch);
}

ACMD(do_visible)
{
  void appear(CharData * ch);

  if IS_AFFECTED(ch, AFF_INVISIBLE)
  {
    appear(ch);
    send_to_char("You break the spell of invisibility.\r\n", ch);
  }
  else if (affected_by_spell(ch, SPELL_DANCE_SHADOWS))
  {
    send_to_char("You stop dancing with the shadows.\r\n", ch);
    affect_from_char(ch, SPELL_DANCE_SHADOWS);
  }
  else if (affected_by_spell(ch, SPELL_SHADOW_SPHERE))
  {
    send_to_char("You dissipate the shadow sphere.\r\n", ch);
    affect_from_char(ch, SPELL_SHADOW_SPHERE);
    act("The globe of darkness around $n vanishes.", FALSE, ch, 0, 0, TO_ROOM);
  }
  else
    send_to_char("You are already visible.\r\n", ch);
}

/* Mortius : adding a land command so players can land when they want to */
ACMD(do_land)
{
  if( affected_by_spell(ch, SPELL_FLY) )
  {
    if( !ROOM_FLAGGED(IN_ROOM(ch), ROOM_FALL))
    {
      sendChar( ch, "You float slowly back to the ground.\r\n" );
      act("$n slowly floats to the ground.", FALSE, ch, 0, 0, TO_ROOM);
      affect_from_char(ch, SPELL_FLY);
    }

    affect_from_char(ch, SPELL_FLY);
    doFallRoom(ch);
  }
  else
  {
    sendChar( ch, "You are not even flying.\r\n" );
  }
}


ACMD(do_title)
{
  skip_spaces(&argument);
  delete_doubledollar(argument);

  if (!IS_NPC(ch) && IS_SET_AR(PLR_FLAGS(ch), PLR_NOCOMM)) {
      send_to_char("You seem unable to do that just now.\r\n", ch);
      return;
  }

  if (IS_NPC(ch))
    send_to_char("Your title is fine... go away.\r\n", ch);
  else if (PLR_FLAGGED(ch, PLR_NOTITLE))
    send_to_char("You can't title yourself -- you shouldn't have abused it!\r\n", ch);
  else if (strstr(argument, "(") || strstr(argument, ")"))
    send_to_char("Titles can't contain the ( or ) characters.\r\n", ch);
  else if (strlen(argument) > MAX_TITLE_LENGTH) {
    sprintf(buf, "Sorry, titles can't be longer than %d characters.\r\n",
	    MAX_TITLE_LENGTH);
    send_to_char(buf, ch);
  }
  else if (!mortal_color(argument,ch))
    return;
  else {
    set_title(ch, argument);
    sprintf(buf, "Okay, you're now %s %s.\r\n", GET_NAME(ch), GET_TITLE(ch));
    send_to_char(buf, ch);
  }
}

#define RATIO_HIT  0
#define RATIO_MANA 1
#define RATIO_MOVE 2

#define RATIO(chr, type) {\
    sprintf(buf, "%s%d%s", colorRatio(ch, COLOR_RAW, C_CMP, \
                GET_##type(chr), GET_MAX_##type(chr)), GET_##type(chr), \
                CCNRM(ch, C_NRM)); \
    sprintf(ratios[RATIO_##type], ""); \
    if (GET_##type(chr) < 1000) strcat(ratios[RATIO_##type], " "); \
    if (GET_##type(chr) <  100) strcat(ratios[RATIO_##type], " "); \
    if (GET_##type(chr) <   10) strcat(ratios[RATIO_##type], " "); \
    strcat(ratios[RATIO_##type], buf); \
}

ACMD(do_group)
{
  char ratios[3][MAX_INPUT_LENGTH];
  CharData *vict, *k;
  FollowType *f, *g;
  bool found;
  int playerCount = 0;

  one_argument(argument, buf);

  if (!*buf) {
      if (!IS_AFFECTED(ch, AFF_GROUP)) {
          send_to_char("But you are not the member of a group!\r\n", ch);
      } else {
          k = (ch->master ? ch->master : ch);

          if(IS_AFFECTED(k, AFF_GROUP))
              playerCount++;

          for(f = k->followers; f; f = f->next)
              if(IS_AFFECTED(f->follower, AFF_GROUP) && !IS_AFFECTED(f->follower, AFF_CHARM))
                  playerCount++;

          sendChar(ch, "Your group consists of %d player%s:\r\n",
                   playerCount, SINGPLUR(playerCount));

          if (IS_AFFECTED(k, AFF_GROUP)) {
              RATIO(k, HIT); RATIO(k, MANA); RATIO(k, MOVE);
              sprintf(buf, "     [%sH %sM %sV] [%2d %2s %3s] $N (Head of group)",
              ratios[0], ratios[1], ratios[2], GET_LEVEL(k),
              CLASS_ABBR(k), RACE_ABBR(k));
              act(buf, FALSE, ch, 0, k, TO_CHAR | TO_SLEEP);
          }

          // First, we follow the leader with any pets the leader controls.
          for (f = k->followers; f; f = f->next) {
              RATIO(f->follower, HIT);
              RATIO(f->follower, MANA);
              RATIO(f->follower, MOVE);
              sprintf(buf, "   + [%sH %sM %sV] [%2d %2s %3s] $N",
                            ratios[0], ratios[1], ratios[2], GET_LEVEL(f->follower),
                            CLASS_ABBR(f->follower), RACE_ABBR(f->follower));
              if(AFF_FLAGGED(f->follower, AFF_CHARM))
                  act(buf, FALSE, ch, 0, f->follower, TO_CHAR | TO_SLEEP);
          }

          // Now the rest of the group displays.
          for (f = k->followers; f; f = f->next)
              if(IS_AFFECTED(f->follower, AFF_GROUP)) {
                  RATIO(f->follower, HIT);
                  RATIO(f->follower, MANA);
                  RATIO(f->follower, MOVE);
                  sprintf(buf, "     [%sH %sM %sV] [%2d %2s %3s] $N",
                                ratios[0], ratios[1], ratios[2], GET_LEVEL(f->follower),
                                CLASS_ABBR(f->follower), RACE_ABBR(f->follower));
                  if(!AFF_FLAGGED(f->follower, AFF_CHARM))
                      act(buf, FALSE, ch, 0, f->follower, TO_CHAR | TO_SLEEP);
                  
                  // Does f->follower have any pets?  If so, let's mention them
                  for(g = f->follower->followers; g; g = g->next) {
                      if(g->follower->master && AFF_FLAGGED(g->follower, AFF_CHARM)) {
                          RATIO(g->follower, HIT);
                          RATIO(g->follower, MANA);
                          RATIO(g->follower, MOVE);
                          sprintf(buf, "   + [%sH %sM %sV] [%2d %2s %3s] $N",
                          ratios[0], ratios[1], ratios[2], GET_LEVEL(g->follower),
                          CLASS_ABBR(g->follower), RACE_ABBR(g->follower));
                          act(buf, FALSE, ch, 0, g->follower, TO_CHAR | TO_SLEEP);
                      }
                  }
              }
      }

      return;
  }
  if (ch->master) {
    act("You can not enroll group members without being head of a group.",
	FALSE, ch, 0, 0, TO_CHAR);
    return;
  }
  if (!str_cmp(buf, "all")) {
    found = FALSE;
    SET_BIT_AR(AFF_FLAGS(ch), AFF_GROUP);
    for (f = ch->followers; f; f = f->next) {
      vict = f->follower;
      if (!IS_AFFECTED(vict, AFF_GROUP)) {
	found = TRUE;
	if (ch != vict)
	  act("$N is now a member of your group.", FALSE, ch, 0, vict, TO_CHAR);
	act("You are now a member of $n's group.", FALSE, ch, 0, vict, TO_VICT);
	act("$N is now a member of $n's group.", FALSE, ch, 0, vict, TO_NOTVICT);
	SET_BIT_AR(AFF_FLAGS(vict), AFF_GROUP);
      }
    }

    if (!found)
      send_to_char("Everyone following you is already in your group.\r\n", ch);

    return;
  }
  if (!(vict = get_char_room_vis(ch, buf))) {
    sendChar( ch, CONFIG_NOPERSON );
  } else {
    found = FALSE;

    if (vict == ch)
      found = TRUE;
    else {
      for (f = ch->followers; f; f = f->next) {
	if (f->follower == vict) {
	  found = TRUE;
	  break;
	}
      }
    }

    if (found) {
      if (IS_AFFECTED(vict, AFF_GROUP)) {
	if (ch != vict)
	  act("$N is no longer a member of your group.", FALSE, ch, 0, vict, TO_CHAR);
	act("You have been kicked out of $n's group!", FALSE, ch, 0, vict, TO_VICT);
	act("$N has been kicked out of $n's group!", FALSE, ch, 0, vict, TO_NOTVICT);
	REMOVE_BIT_AR(AFF_FLAGS(vict), AFF_GROUP);
      } else {
	if (ch != vict)
	  act("$N is now a member of your group.", FALSE, ch, 0, vict, TO_CHAR);
	act("You are now a member of $n's group.", FALSE, ch, 0, vict, TO_VICT);
	act("$N is now a member of $n's group.", FALSE, ch, 0, vict, TO_NOTVICT);
	SET_BIT_AR(AFF_FLAGS(vict), AFF_GROUP);
      }
    } else
      act("$N must follow you to enter your group.", FALSE, ch, 0, vict, TO_CHAR);
  }
}

/* ============================================================================ 
This function returns 1 if ch and target are in the same group, 0 if they
are not.
============================================================================ */
int chGroupedWithTarget(CharData *ch, CharData *target)
{
    /* Are they both even in a group? */
//    if ( !IS_NPC(ch) && !(AFF_FLAGGED(ch, AFF_GROUP) && AFF_FLAGGED(target, AFF_GROUP)) )
//	return 0;
    /* They are grouped, but is it the same one? */

    /* ch is the target, and is grouped, so that counts. */
    if (ch == target)
	return 1;

    /* Is the target the group leader? */
    if (ch->master && ch->master == target)
	return 1;

    /* Is ch the group leader? */
    if (target->master && target->master == ch)
	return 1;

    /* Do they have the same group leader? */
    if (target->master && ch->master && target->master == ch->master)
	return 1;

    if(ch->master && chGroupedWithTarget(ch->master, target))
        return 1;
    if(target->master && chGroupedWithTarget(ch, target->master))
        return 1;

    /* They are in different groups. */
    return 0;
}

int has_owner(ObjData *blade) {
    return isname("ownedby", blade->name);
}

int owns_item(CharData *ch, ObjData *blade) {
    char ownerstring[50];
    sprintf(ownerstring, "ownedby%s ", GET_NAME(ch));
    return isname(ownerstring, blade->name);
}

int set_owner(CharData *ch, ObjData *blade) {
    char ownerstring[50];

    if(has_owner(blade)) {
        mudlog(CMP, LVL_IMMORT, TRUE, "Error in has_owner.  Player %s is trying to become owner of %s.\r\n",
                GET_NAME(ch), blade->name);
        return FALSE;
    }
    
    sprintf(ownerstring, "%s ownedby%s ", blade->name, GET_NAME(ch));
    blade->name = strdup(ownerstring);
    
    return TRUE;
}

/* banish_conjured:
** This function removes the NPC passed into it from the game.
** It's purpose is to prevent mages from ordering their followers to attack
** and then simply ungrouping them so they don't flee the fight. It will also
** help to get around the problem of them summoning and ungrouping mobs on
** a continual basis(there by violating the follower limit effectively).
** Vex.
*/

void banish_conjured(CharData *the_mob)
{
  int i;
  ObjData *temp_obj;

  if (!IS_NPC(the_mob))
	return; /* Just in case someone screws up some where... */

  send_to_char("Your master has abandoned you! Time to go home...\r\n", the_mob);
  act("$n fades from this reality.", TRUE, the_mob, 0, 0, TO_ROOM);
  for (i=0;i < NUM_WEARS;i++)
  {
	if(the_mob->equipment[NUM_WEARS])
		extract_obj(the_mob->equipment[NUM_WEARS]);
  }
  if (the_mob->carrying)
	while (the_mob->carrying)
	{
		temp_obj = the_mob->carrying;
		the_mob->carrying = the_mob->carrying->next_content;
		extract_obj(temp_obj);
	}
  extract_char(the_mob);
}

ACMD(do_ungroup)
{
	FollowType *f, *next_fol;
	CharData *tch;
	CharData *the_mob;

	one_argument(argument, buf);

	if (!*buf) {
            if (ch->master || !(IS_AFFECTED(ch, AFF_GROUP))) {
                send_to_char("But you lead no group!\r\n", ch);
                return;
            }
            sprintf(buf2, "%s has disbanded the group.\r\n", GET_NAME(ch));
            for (f = ch->followers; f; f = next_fol) {
                next_fol = f->next;
                if (IS_AFFECTED(f->follower, AFF_GROUP)) {
                    REMOVE_BIT_AR(AFF_FLAGS(f->follower), AFF_GROUP);
                    send_to_char(buf2, f->follower);
                    the_mob = f->follower;
                    if (IS_NPC(the_mob) && affected_by_spell(the_mob, SPELL_CHARM_CORPSE))
                    {
                        affect_from_char(the_mob, SPELL_CHARM_CORPSE);
                        continue;
                    }
                    stop_follower(f->follower);
                    if (IS_NPC(the_mob) && IS_SET_AR(MOB_FLAGS(the_mob), MOB_CONJURED))
                        banish_conjured(the_mob);
                }
            }

            send_to_char("You have disbanded the group.\r\n", ch);
            return;
        }
        if (!(tch = get_char_room_vis(ch, buf))) {
            send_to_char("There is no such person!\r\n", ch);
            return;
        }
        if (tch->master != ch) {
            send_to_char("That person is not following you!\r\n", ch);
            return;
        }
        if (IS_AFFECTED(tch, AFF_GROUP))
            REMOVE_BIT_AR(AFF_FLAGS(tch), AFF_GROUP);
        
        act("$N is no longer a member of your group.", FALSE, ch, 0, tch, TO_CHAR);
        act("You have been kicked out of $n's group!", FALSE, ch, 0, tch, TO_VICT);
        act("$N has been kicked out of $n's group!", FALSE, ch, 0, tch, TO_NOTVICT);
        if (IS_NPC(tch) && affected_by_spell(tch, SPELL_CHARM_CORPSE)) {
            affect_from_char(tch, SPELL_CHARM_CORPSE);
            return;
        }
	stop_follower(tch);
	if (IS_NPC(tch) && IS_SET_AR(MOB_FLAGS(tch), MOB_CONJURED))
		banish_conjured(tch);
}




ACMD(do_report)
{
  CharData *k;
  FollowType *f;

  if (!IS_AFFECTED(ch, AFF_GROUP)) {
    send_to_char("But you are not a member of any group!\r\n", ch);
    return;
  }
  sprintf(buf, "%s reports: %d/%dH, %d/%dM, %d/%dV\r\n",
	  GET_NAME(ch), GET_HIT(ch), GET_MAX_HIT(ch),
	  GET_MANA(ch), GET_MAX_MANA(ch),
	  GET_MOVE(ch), GET_MAX_MOVE(ch));

  CAP(buf);

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
  FollowType *f;

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
      send_to_char("You don't seem to have that much gold to split.\r\n", ch);
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
      sprintf(buf, "$n splits %d coins; you receive %d.\r\n", amount, share);
      act(buf, FALSE, ch, 0, k, TO_VICT);
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
  /* Berserk characters can't use magical items */
  if (IS_AFFECTED(ch, AFF_BERSERK)) {
	sendChar(ch, "You're too angry to worry about that! KILL!!\r\n");
	return;
  }

  mag_item = ch->equipment[WEAR_HOLD];

  if (!mag_item || !isname(arg, mag_item->name)) {
    switch (subcmd) {
    case SCMD_RECITE:
    case SCMD_QUAFF:
    case SCMD_TOSS:
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
      break;
    default:
      mlog("SYSERR: Unknown subcmd passed to do_use");
      return;
      break;
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
      send_to_char("You can only recite scrolls.\r\n", ch);
      return;
    }
    else if IS_AFFECTED(ch, AFF_SILENCE) {
      send_to_char("You look at it, but can't seem to get the words out.\r\n", ch);
      return;
    }
    break;
  case SCMD_TOSS:
    if (GET_OBJ_TYPE(mag_item) != ITEM_DUST) {
      send_to_char("You can only toss dust.\r\n", ch);
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

  /* if it's QUAFF'd or TOSS'd or RECITE'd, unequip it */
  if (equipped && subcmd != SCMD_USE) unequip_char(ch, WEAR_HOLD);

  mag_objectmagic(ch, mag_item, buf);

}

ACMD(do_display)
{
  size_t i;

  if (IS_NPC(ch)) {
    sendChar(ch, "Monsters don't need displays.  Go away.\r\n");
    return;
  }
  skip_spaces(&argument);

  if (!*argument) {
    sendChar(ch, "Usage: prompt { H | M | V | E | T | all | none }\r\n");
    return;
  }
  if ((!str_cmp(argument, "on")) || (!str_cmp(argument, "all"))) {
        SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPHP);
        SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPMANA);
        SET_BIT_AR(PRF_FLAGS(ch), PRF_SHOWTANK);
	SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPMOVE);
	SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPEXP);
        SET_BIT_AR(PRF_FLAGS(ch), PRF_SHOWTANK);
  } else if (!str_cmp(argument, "off") || !str_cmp(argument, "none")) {
        REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_DISPHP);
	REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_DISPMANA);
	REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_DISPMOVE);
	REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_DISPEXP);
        REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_SHOWTANK);
  } else {
        REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_DISPHP);
	REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_DISPMANA);
	REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_DISPMOVE);
	REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_DISPEXP);
        REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_SHOWTANK);

    for (i = 0; i < strlen(argument); i++) {
      switch (LOWER(argument[i])) {
      case 'h':
	SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPHP);
	break;
      case 'm':
	SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPMANA);
	break;
      case 'v':
	SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPMOVE);
	break;
      case 'e':
	SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPEXP);
	break;
      case 't':
	SET_BIT_AR(PRF_FLAGS(ch), PRF_SHOWTANK);
	break;
      default:
	sendChar(ch, "Usage: prompt { H | M | V | E | T | all | none }\r\n");
	return;
      }
    }
  }

  sendChar(ch, "%s", CONFIG_OK);
}


ACMD(do_gen_write)
{
  FILE *fl;
  char *tmp, *filename;
  struct stat fbuf;
  time_t ct;

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

  ct = time(0);
  tmp = asctime(localtime(&ct));

  if( IS_NPC(ch) )
  {
    sendChar( ch, "Monsters can't have ideas - Go away.\r\n" );
    return;
  }

  skip_spaces(&argument);
  delete_doubledollar(argument);

  if( !*argument )
  {
    sendChar( ch, "That must be a mistake...\r\n" );
    return;
  }

  if (strlen(argument) > 150) {
      sendChar(ch, "Please don't make such a long comment.  It can cause crashes, which is rather rude.\r\n");
      mudlog( CMP, LVL_IMMORT, TRUE, "[CRASH] %s used a very long %s", GET_NAME(ch), CMD_NAME);
      return;
  }

  mudlog( CMP, LVL_IMMORT, FALSE, "%s %s: %s", GET_NAME(ch), CMD_NAME, argument);

  if( stat(filename, &fbuf) < 0 )
  {
    perror("Error statting file");
    return;
  }

  if( fbuf.st_size >= CONFIG_MAX_FILESIZE )
  {
    sendChar( ch, "Sorry, the file is full right now.  Please try again later.\r\n" );
    return;
  }

  if( !(fl = fopen(filename, "a")) )
  {
    perror("do_gen_write");
    sendChar( ch, "Could not open the file.  Sorry.\r\n" );
    return;
  }

  if( strlen(argument) < 20 )
  {
    mudlog(NRM, LVL_IMMORT, TRUE, "Stupid idea ignored from %s [%s]", GET_NAME(ch), argument );
    sendChar(ch, "Your message has been deemed too short to be intelligent and was ignored.\r\n");

  }
  else
  {
    fprintf( fl, "%-8s (%6.6s) [%5d] %s\n", GET_NAME(ch), (tmp + 4),
	     world[ch->in_room].number, argument);
  }
  fclose(fl);
  sendChar( ch, "Okay.  Thanks!\r\n" );

}

void general_log(char *argument, char *filename) {
    FILE *fl;
    char *tmp;
    struct stat fbuf;
    time_t ct;
    
    ct = time(0);
    tmp = asctime(localtime(&ct));
    
    skip_spaces(&argument);
    delete_doubledollar(argument);

    if(!*argument) {
        return;
    }
    
    if( stat(filename, &fbuf) < 0 ) {
        //Error statting file
        return;
    }

    if(fbuf.st_size >= CONFIG_MAX_FILESIZE) {
        //"The file is full right now."
        return;
    }
    
    if(!(fl = fopen(filename, "a"))) {
        //"do_gen_write"
        //"Could not open the file.  Sorry.\r\n"
        return;
    }
    
    fprintf(fl, "%s\n", argument);
    fclose(fl);
}

ACMD(do_noteam)
{
    /* Remove them from the gold team */
    if (PRF_FLAGGED(ch, PRF_GOLD_TEAM)) {
        send_to_char("You are no longer a member of the gold team.\r\n", ch);
        REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_GOLD_TEAM);
    }
    /* Remove them from the black team */
    else if (PRF_FLAGGED(ch, PRF_BLACK_TEAM)) {
        send_to_char("You are no longer a member of the black team.\r\n", ch);
        REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_BLACK_TEAM);
    }
    else if (PRF_FLAGGED(ch, PRF_ROGUE_TEAM)) {
        send_to_char("You are no longer a member of the rogue team.\r\n", ch);
        REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_ROGUE_TEAM);
    } else {
        send_to_char("You're not even IN a team.\r\n", ch);
        return;
    }
    /* Set start room back to default */
    if (PLR_FLAGGED(ch, PLR_LOADROOM))
        REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_LOADROOM);
    GET_LOADROOM(ch) = getStartRoom(ch);

    /* tell flag code the player has been removed from a team */
    flag_player_from_game(ch);
}

#define TOG_OFF 0
#define TOG_ON  1
ACMD(do_gen_tog)
{
    long result;
    /*
    ** Some commands accept a 2nd level parameter for selectively disabling
    ** certain features. For example, nogoss 10 would be that the character
    ** wouldn't see anything on the goss channel from anyone under 10th level.
    */
    char levelStr[30] = "";
    int  levelVal     = 0;

  const char *tog_messages[][2] = {
        {"You are now safe from summoning by other players.\r\n",
         "You may now be summoned by other players.\r\n"},
        {"Nohassle disabled.\r\n",
         "Nohassle enabled.\r\n"},
        {"Brief mode off.\r\n",
         "Brief mode on.\r\n"},
        {"Compact mode off.\r\n",
         "Compact mode on.\r\n"},
        {"You can now hear tells.\r\n",
         "You are now deaf to tells.\r\n"},
        {"You can now hear auctions.\r\n",
         "You are now deaf to auctions.\r\n"},
        {"You can now hear shouts.\r\n",
         "You are now deaf to shouts.\r\n"},
        {"You can now hear roleplays.\r\n",
         "You are now deaf to roleplays.\r\n"},
        {"You can now hear the congratulation messages.\r\n",
         "You are now deaf to the congratulation messages.\r\n"},
        {"You will now see the arena results.\r\n",
         "You will not receive arena results.\r\n"},
        {"You are no longer part of the Quest.\r\n",
         "Okay, you are part of the Quest!\r\n"},
        {"You will no longer see the room flags.\r\n",
         "You will now see the room flags.\r\n"},
        {"You will now have your communication repeated.\r\n",
         "You will no longer have your communication repeated.\r\n"},
        {"HolyLight mode off.\r\n",
         "HolyLight mode on.\r\n"},
        {"Nameserver_is_slow changed to NO; IP addresses will now be resolved.\r\n",
         "Nameserver_is_slow changed to YES; sitenames will no longer be resolved.\r\n"},
        {"Autoexits disabled.\r\n",
         "Autoexits enabled.\r\n"},
        {"Your character information is now hidden.\r\n",
         "Your character information is now visible.\r\n"},
        {"You have rescinded your consent.\r\n",
         "You have given consent.\r\n"},
        {"Plagues are not contagious.\r\n",
         "Plagues will spread.\r\n" },
        {"Raw I/O logging disabled.\r\n",
         "Raw I/O logging enabled.\r\n" },
        {"You will receive all character actions (SPAM).\r\n",
         "You will only receive pertinant character action (SPAM LITE).\r\n"},
        {"You will now use simple OLC entries.\r\n", 
         "You will now use verbose OLC entries.\r\n"},
        {"You can now hear OOCs.\r\n",
         "You are now deaf to OOCs.\r\n"},
        {"You can now hear queries.\r\n",
         "You are now deaf to queries.\r\n"},
        {"You can now hear guild conversations.\r\n",
         "You are now deaf to guild conversations.\r\n"},
        {"You are now safe from recall by other players.\r\n",
         "You may now be recalled by other players.\r\n"},
	{"Auto splitting of gold disabled.\r\n",
	 "Auto splitting of gold enabled.\r\n"},
	{"Auto looting of corpses disabled.\r\n",
	 "Auto looting of corpses enabled.\r\n"},
	{"You can now hear the clan channel.\r\n",
	 "You can no longer hear the clan channel.\r\n"},
        {"Auto looting of gold disabled.\r\n",
	 "Auto looting of gold enabled.\r\n"},
        {"Player killing disabled.\r\n",
         "Player killing enabled.\r\n"},
        {"Player stealing disabled.\r\n",
         "Player stealing enabled.\r\n"},
        {"You will now appear on the WHO list.\r\n",
         "You are now hidden from the WHO list.\r\n"},
         {"You will no longer see numeric damage.\r\n",
         "You will now see numeric damage.\r\n"},
         {"Will no longer clear screen in OLC.\r\n",
         "Will now clear screen in OLC.\r\n"},
         {"You can now hear the Wiz-channel.\r\n",
         "You are now deaf to the Wiz-channel.\r\n"},
         {"AFK flag is now off.\r\n",
         "AFK flag is now on.\r\n"}
    };


    if (IS_NPC(ch))
        return;

    one_argument( argument, levelStr );
    if( levelStr != NULL ){
        if( is_number( levelStr ))
            levelVal = atoi( levelStr );
    }

    switch (subcmd) {
        case SCMD_NOSUMMON:  result = PRF_TOG_CHK(ch, PRF_SUMMONABLE); break;
        case SCMD_NOHASSLE:  result = PRF_TOG_CHK(ch, PRF_NOHASSLE);   break;
        case SCMD_BRIEF:     result = PRF_TOG_CHK(ch, PRF_BRIEF); break;
        case SCMD_COMPACT:   result = PRF_TOG_CHK(ch, PRF_COMPACT); break;
        case SCMD_NOAUCTION: result = PRF_TOG_CHK(ch, PRF_NOAUCT); break;
        case SCMD_DEAF:      result = PRF_TOG_CHK(ch, PRF_DEAF); break;
        case SCMD_NOGRATZ:   result = PRF_TOG_CHK(ch, PRF_NOGRATZ); break;
        case SCMD_NOARENA:   result = PRF_TOG_CHK(ch, PRF_NOARENA); break;
        case SCMD_QUEST:     result = PRF_TOG_CHK(ch, PRF_QUEST); break;
        case SCMD_ROOMFLAGS: result = PRF_TOG_CHK(ch, PRF_SHOWVNUMS); break;
        case SCMD_NOREPEAT:  result = PRF_TOG_CHK(ch, PRF_NOREPEAT); break;
        case SCMD_HOLYLIGHT: result = PRF_TOG_CHK(ch, PRF_HOLYLIGHT); break;
        case SCMD_AUTOEXIT:  result = PRF_TOG_CHK(ch, PRF_AUTOEXIT); break;
        case SCMD_ANON:      result = PRF_TOG_CHK(ch, PRF_NOTANON); break;
	/* Anyone can use showdam - that way imms can always turn on showdam
	   but only explorers can see the message. */
        case SCMD_SHOWDAM:
            result = PRF_TOG_CHK(ch, PRF_SHOWDAM);
            if(!PLR_IS_EXPLORER(ch)) return;
            break;
        case SCMD_CONSENT:   
          if (!IS_SET_AR(PLR_FLAGS(ch), PLR_HUNTED)) 
                             result = PRF_TOG_CHK(ch, PRF_CONSENT); break;
        case SCMD_PLAGUE:    result = (CONFIG_PLAGUE_IS_CONTAGIOUS = !CONFIG_PLAGUE_IS_CONTAGIOUS); break;
	case SCMD_PSTEAL:    result = (CONFIG_PT_ALLOWED = !CONFIG_PT_ALLOWED); break;
	case SCMD_PKILL:     result = (CONFIG_PK_ALLOWED = !CONFIG_PK_ALLOWED); break;
        case SCMD_RAWLOG:    result = (raw_input_logging = !raw_input_logging); break;
        case SCMD_SPAM:      result = PRF_TOG_CHK(ch, PRF_NOSPAM); break;
        case SCMD_OLCVERBOSE: result = PRF_TOG_CHK(ch, PRF_OLCV); break;
        case SCMD_NORPLAY:   result = PRF_TOG_CHK(ch, PRF_NORPLAY); break;
        case SCMD_NOTELL:    result = PRF_TOG_CHK(ch, PRF_NOTELL); break;
        case SCMD_NOOOC:     result = PRF_TOG_CHK(ch, PRF_NOOOC); break;
        case SCMD_NOQUERY:   result = PRF_TOG_CHK(ch, PRF_NOQUERY); break;
        case SCMD_NOGUILD:   result = PRF_TOG_CHK(ch, PRF_NOGUILD); break;
        case SCMD_NORECALL:  result = PRF_TOG_CHK(ch, PRF_NORECALL); break;
	case SCMD_AUTOSPLIT: result = PRF_TOG_CHK(ch, PRF_AUTOSPLIT); break;
	case SCMD_AUTOLOOT:  result = PRF_TOG_CHK(ch, PRF_AUTOLOOT); break;
	case SCMD_NOCLAN:    result = PRF_TOG_CHK(ch, PRF_NOCLAN); break;
	case SCMD_AUTOGOLD:  result = PRF_TOG_CHK(ch, PRF_AUTOGOLD); break;
	case SCMD_NOWHO:     result = PRF_TOG_CHK(ch, PRF_NOWHO); break;
        case SCMD_CLS:       result = PRF_TOG_CHK(ch, PRF_CLS); break;
        case SCMD_NOWIZ:     result = PRF_TOG_CHK(ch, PRF_NOWIZ); break;
        case SCMD_AFK:       result = PRF_TOG_CHK(ch, PRF_AFK);
            if (PRF_FLAGGED(ch, PRF_AFK))
                act("$n has gone AFK.", TRUE, ch, 0, 0, TO_ROOM);
            else {
                act("$n has come back from AFK.", TRUE, ch, 0, 0, TO_ROOM);
                if (has_mail(GET_IDNUM(ch)))
                    sendChar(ch, "You have mail waiting.\r\n");
            }
            break;
        default:
          mlog("SYSERR: Unknown subcmd %d in do_gen_toggle.", subcmd);
            return;
    }

  if (result)
    sendChar(ch, "%s", tog_messages[subcmd][TOG_ON]);
  else
    sendChar(ch, "%s", tog_messages[subcmd][TOG_OFF]);

    return;
}


void
water_activity(int pulse)
{
  register CharData *ch, *next_ch;

/*
 * Mortis : This will deal with players/mobiles that are on/in/under water in
 *          any form.  For now NPC's will not be moving on water.
 */

  /* Lets handle all characters in water/rivers */

  for( ch = character_list; ch; ch = next_ch )
  {
    next_ch = ch->next;

    if( IS_IMMORTAL(ch) || IS_NPC(ch) || !UNDERWATER(ch) ) continue;

    if( !IS_AFFECTED(ch, AFF_AIRSPHERE ))
    {
      if( WATER_COUNTER(ch) == 0 )
        WATER_COUNTER(ch) = 10;
      else if (WATER_COUNTER(ch) <= 1)
      {
        water_death(ch);
        continue;
      }
      else
      {
        if( WATER_COUNTER(ch) == 9 )
        {
          sendChar( ch, "Tiny bubbles start to escape from your mouth.  You may need air soon!\r\n" );
          GET_HIT(ch) = GET_HIT(ch) / 1.3;
          act( "$n has tiny little bubbles coming from $s mouth.",
                FALSE, ch, 0, 0, TO_ROOM);
        }
        else if( WATER_COUNTER(ch) == 7 )
        {
          sendChar( ch, "Large air bubbles are escaping from your mouth.  You feel light headed!\r\n" );
          GET_HIT(ch) = GET_HIT(ch) / 2;
          act( "$n has large bubbles escaping from $s mouth.",
                FALSE, ch, 0, 0, TO_ROOM);
        }
        else if( WATER_COUNTER(ch) == 4)
        {
          sendChar( ch,  "You start to panic as your lungs begin to fill with water!\r\n" );
          GET_HIT(ch) = GET_HIT(ch) / 4;
          act( "$n starts to panic as $s lungs fill up with &11water&00.\r\n",
                FALSE, ch, 0, 0, TO_ROOM);
        }
        else if (WATER_COUNTER(ch) == 2)
        {
          send_to_char("You black out.\r\n", ch);
          GET_HIT(ch) = number(-5, -1);
          update_pos(ch);
          act( "$n has blacked out due to the lack of air.",
                FALSE, ch, 0, 0, TO_ROOM);
        }
        WATER_COUNTER(ch) -= 1;
      }
    }
  }     /* End of Player (FOR) loop */
}


void
water_death(CharData *ch)
{
  // If not underwater we shouldn't be here
  //
  if( !SECT(ch->in_room) == SECT_UNDERWATER )
  {
      mudlog(BRF, LVL_IMMORT, TRUE, "ERROR: Problem in water_death!");
     return;
  }

  if( IS_IMMORTAL(ch) ) return;

  if( FIGHTING(ch) )                     /* Stop the fighting b/c ch has died */
    stop_fighting(ch);

  while( ch->affected )
    affect_remove(ch, ch->affected, FALSE); /* FALSE added for fall room */

  sendChar( ch,
      "Your lungs are full of water which you can no longer breathe!\r\n" );
  sendChar( ch, "You are dead!  R.I.P!\r\n" );
  act( "You see the look of sheer panic on $n's face as $e drowns to death!",
        FALSE, ch, 0, 0, TO_ROOM);

  GET_HIT(ch) = GET_MAX_HIT(ch)/4;
  GET_MANA(ch) = GET_MAX_MANA(ch)/4;
  GET_MOVE(ch) = GET_MAX_MOVE(ch)/2;

  // Time to make them lose xp, this is the same amount as a dt death
  //
  if (IN_QUEST_FIELD(ch))
  {
    jail_char(ch, FALSE);
  }
  else
  {
  gain_exp(ch,  -MIN(1000000, (GET_LEVEL(ch) * GET_LEVEL(ch) * 500)));
  make_corpse(ch);
  extract_char(ch);
  WATER_COUNTER(ch) = 0;        /* Reset counter */
  }
}

void logItem(int vnum, char *fileName) {
    bool valid_value = TRUE;
    int i, found;
    ObjData *j2;
    struct extra_descr_data *desc;

    int r_num = real_object(vnum);
    ObjData *obj = obj_proto + r_num;

    sprintf(buf, "Name: '%s', Aliases: %s",
    ((obj->short_description) ? obj->short_description : "<None>"), (obj->name) ? obj->name : "<None>");
    general_log(buf, fileName);
    sprinttype(GET_OBJ_TYPE(obj), item_types, buf1, sizeof(buf1));
    sprintf(buf, "VNum: [%5d], RNum: [%5d], Type: %s",
    vnum, GET_OBJ_RNUM(obj), buf1);

    general_log(buf, fileName);

    /* Special procedures. */
    if (GET_OBJ_RNUM(obj) >= 0) {
        combSpecName(obj_index[obj->item_number].combatSpec, buf1);
        specialName(obj_index[obj->item_number].func, buf2);
    }
    else {
        strcpy(buf1, "None");
        strcpy(buf2, "None");
    }
    sprintf(buf, "CombSpecProc: %s SpecProc: %s", buf1, buf2);
    general_log(buf, fileName);

    sprintf(buf, "L-Des: %s", ((obj->description) ? obj->description : "None"));
    general_log(buf, fileName);

    if (obj->ex_description) {
        sprintf(buf, "Extra descs:");
        for (desc = obj->ex_description; desc; desc = desc->next) {
            strcat(buf, " ");
            strcat(buf, desc->keyword);
        }
        general_log(buf, fileName);
    }

    sprintf(buf, "Can be worn on: ");
    sprintbitarray(obj->obj_flags.wear_flags, wear_bits, TW_ARRAY_MAX, buf1);
    strcat(buf, buf1);
    general_log(buf, fileName);

    sprintf(buf, "Set char bits : ");
    sprintbitarray(obj->obj_flags.bitvector, affected_bits, AF_ARRAY_MAX, buf1);
    strcat(buf, buf1);
    general_log(buf, fileName);

    sprintf(buf, "Extra flags    : ");
    sprintbitarray(GET_OBJ_EXTRA(obj), extra_bits, EF_ARRAY_MAX, buf1);
    strcat(buf, buf1);
    general_log(buf, fileName);

    sprintf(buf, "Weight: %d, Value: %d, Cost/day: %d, Timer: %d",
    GET_OBJ_WEIGHT(obj), GET_OBJ_COST(obj), GET_OBJ_RENT(obj), GET_OBJ_TIMER(obj));
    general_log(buf, fileName);

    /* Check to make sure the values are valid. */
    for (i = 0; i < 4; i++)
        if (obj->obj_flags.value[i] != objValueLimit(obj->obj_flags.type_flag, i, obj->obj_flags.value[i]))
            valid_value = FALSE;

    if (valid_value)
        oPrintValues(obj, buf);
    else /* Could be risk of causing crash trying to show them, print integers. */
        sprintf(buf, "&08Values invalid!&00 True values: %d %d %d %d",
                obj->obj_flags.value[0], obj->obj_flags.value[1], obj->obj_flags.value[2], obj->obj_flags.value[3]);

    general_log(buf, fileName); /* show em the values. */

    sprintf(buf, "Affections:");

    found = 0;
    for (i = 0; i < MAX_OBJ_AFFECT; i++) {
        if (obj->affected[i].modifier) {
            sprinttype(obj->affected[i].location, apply_types, buf2, sizeof(buf2));
            sprintf(buf1, "%s %+d to %s", found++ ? "," : "",
            obj->affected[i].modifier, buf2);
            strcat(buf, buf1);
        }
    }
    if (!found)
        strcat(buf, " None");

    general_log(buf, fileName);

    if (GET_TRAP(obj)) {
        sprintf(buf, "Object is trapped at level %d.", GET_TRAP(obj));
        general_log(buf, fileName);
    }

}


