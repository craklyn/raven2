/* ************************************************************************
*   File: act.offensive.c                               Part of CircleMUD *
*  Usage: player-level commands of an offensive nature                    *
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
#include "actions/act.h"          /* ACMDs located within the act*.c files */
#include "general/handler.h"
#include "general/class.h"
#include "magic/spells.h"
#include "util/weather.h"
#include "actions/offensive.h"
#include "magic/skills.h" 
#include "magic/sing.h"
#include "actions/fight.h"
#include "actions/outlaw.h"       /* For player_attack_victim */

/** Help buffer the global variable definitions */
#define __OFFENSIVE_C__

#define STUN_MIN                  1  //For Flee Stun
#define STUN_MAX                  1  //For Flee Stun


CharData guard_group;

ACMD( do_guard )
{
    char toGuard[40];
    CharData *myCharge;

    one_argument( argument, toGuard );

    if(!IS_NPC(ch) && !GET_SKILL( ch, SKILL_GUARD )){
        sendChar( ch, "You lack the required training to guard other people.\r\n" );
        return;
    }

    if( toGuard == NULL ){
        if( ch->guarding == NULL )
            sendChar( ch, "You are not guarding anyone.\r\n" );
        else
            sendChar( ch, "You stop guarding %s.\r\n",
                    ch->guarding == &guard_group ? "your group"
                    : GET_NAME(ch->guarding));
        ch->guarding = NULL;
        return;
    }
    else {
        if( ( myCharge = get_char_room_vis(ch, toGuard) ) != NULL ){
            ch->guarding = myCharge;
            sendChar(ch, "You start guarding %s.\r\n", GET_NAME(ch->guarding));
        }
        /*
         * Guarding group is disabled until the crash bug associated with it can be
         * better understood.  - Arbaces 12/19/2010
        else if (is_abbrev(toGuard, "group")) {
            ch->guarding = &guard_group;
            sendChar( ch, "You start guarding your group.\r\n" );
        }
         */
        else {
            sendChar( ch, "Who do you wish to guard?\r\n" );
        }
    }
}

ACMD( do_assist )
{
    CharData *helpee   = NULL;
    CharData *opponent = NULL;

    if( FIGHTING(ch) ){
        send_to_char("You're already fighting!  How can you assist someone else?\r\n", ch);
        return;
    }

    one_argument(argument, arg);

    if (!*arg) {
        /* No argument, help anyone we can! */
    } else if (!(helpee = get_char_room_vis(ch, arg))) {
        sendChar( ch, CONFIG_NOPERSON );
        return;
    } else if (helpee == ch) {
        sendChar(ch, "You can't help yourself any more than this!\r\n");
        return;
    } else if (!FIGHTING(helpee)) {
        act("But nobody is fighting $M!", FALSE, ch, 0, helpee, TO_CHAR);
        return;
    }

    /* Find the opponent */
    for (opponent = world[ch->in_room].people;
         opponent; opponent = opponent->next_in_room ) {
        CharData *vict = FIGHTING(opponent);

        /* Only opponents who are fighting */
        if (!vict) continue;

		/* You can't assist yourself */
		if (vict == ch) continue;

        /* If we don't have a helpee, see if we can assign one */
        if (!helpee && IS_AFFECTED(ch, AFF_CHARM) && vict == ch->master)
            helpee = vict;

        if (!helpee && in_same_group(vict, ch))
            helpee = vict;

        /* If we found someone fighting our helpee, exit */
        if (helpee && vict == helpee) break;
    }

    if (!helpee) {
        sendChar(ch, "Whom do you wish to assist?\r\n");
        return;
    }

    if (!opponent || FIGHTING(opponent) != helpee)
        act( "But nobody is fighting $M!", FALSE, ch, 0, helpee, TO_CHAR );
    else if (!CAN_SEE(ch, opponent))
        act("You can't see who is fighting $M!", FALSE, ch, 0, helpee, TO_CHAR);
    else {
        sendChar( ch, "You join the fight!\r\n" );
        act( "$N assists you!", 0, helpee, 0, ch, TO_CHAR);
        act( "$n assists $N.", FALSE, ch, 0, helpee, TO_NOTVICT);
        player_attack_victim(ch, opponent);
        hit( ch, opponent, TYPE_UNDEFINED);
    }
}


ACMD(do_hit)
{
    CharData *vict;

    one_argument(argument, arg);

    if( !*arg ) sendChar( ch, "Hit who?\r\n" );

    else if( !( vict = get_char_room_vis( ch, arg )))
        sendChar( ch, "They don't seem to be here.\r\n" );

    else if (vict == ch) {
        sendChar( ch, "You hit yourself...OUCH!\r\n" );
        act("$n hits $mself, and says OUCH!", FALSE, ch, 0, vict, TO_ROOM);
    }

    else if (IS_SET_AR(ROOM_FLAGS(ch->in_room), ROOM_PEACEFUL)) {
        sendChar( ch, "You are overcome with a feeling of peace.\r\n" );
    }

    else if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == vict))
        act( "$N is just such a good friend, you simply can't hit $M.", FALSE, ch, 0, vict, TO_CHAR );
    else {
    if( !IS_NPC(vict) ){
        if( !IS_NPC(ch) && (subcmd != SCMD_MURDER) && !(IN_ARENA(ch))){
            sendChar( ch, "Use 'murder' to hit another player.\r\n" );
            return;
        }
        /*
        ** Let's really screw up shunned killers.
        */
        if( PLR_FLAGGED( ch, PLR_SHUNNED )){
            WAIT_STATE(ch, 50);
            GET_HIT(ch) = -3;
            SET_DAMROLL(ch, 0);
            hit(ch, vict, TYPE_UNDEFINED);
            return;
        }
    }

    if( (GET_POS(ch) == POS_STANDING) && (vict != FIGHTING(ch)) ){
        WAIT_STATE(ch, PULSE_VIOLENCE + 2);
        hit(ch, vict, TYPE_UNDEFINED);
    } else
        sendChar( ch, "You do the best you can!\r\n" );
    }
}/* do_hit */


ACMD(do_kill)
{
  CharData *vict;

  if ((GET_LEVEL(ch) < LVL_IMPL) || IS_NPC(ch)) {
    do_hit(ch, argument, cmd, subcmd);
    return;
  }
  one_argument(argument, arg);

  if (!*arg) {
    send_to_char("Kill who?\r\n", ch);
  } else {
    if (!(vict = get_char_room_vis(ch, arg)))
      send_to_char( "They aren't here.\r\n", ch);
    else if (ch == vict)
      send_to_char("Your mother would be so sad.. :(\r\n", ch);
    else {
      if (GET_INVIS_LEV(ch) > GET_LEVEL(vict)) {
        send_to_char("You are dead!  Sorry...\r\n", vict);
      } else {
        act("You chop $M to pieces!  Ah!  The blood!", FALSE, ch, 0, vict, TO_CHAR);
        act("$N chops you to pieces!", FALSE, vict, 0, ch, TO_CHAR);
        act("$n brutally slays $N!", FALSE, ch, 0, vict, TO_NOTVICT);
      }
      raw_kill(vict, ch);
    }
  }
}


ACMD(do_order)
{
  char name[100], message[MAX_STRING_LENGTH];
  char buf[256];
  bool found = FALSE;
  int org_room;
  CharData *vict;
  FollowType *k, *kn;

  half_chop(argument, name, message);

  if (!*name || !*message)
    send_to_char("Order who to do what?\r\n", ch);
  else if (!(vict = get_char_room_vis(ch, name)) && !is_abbrev(name, "followers"))
    send_to_char("That person isn't here.\r\n", ch);
  else if (ch == vict)
    send_to_char("You obviously suffer from skitzofrenia.\r\n", ch);

  else {
    if (IS_AFFECTED(ch, AFF_CHARM)) {
      send_to_char("Your superior would not aprove of you giving orders.\r\n", ch);
      return;
    }
    if (vict) {
      sprintf(buf, "$N orders you to '%s'", message);
      act(buf, FALSE, vict, 0, ch, TO_CHAR);

      if ((vict->master != ch) || !IS_AFFECTED(vict, AFF_CHARM))
        act("$n has an indifferent look.", FALSE, vict, 0, 0, TO_ROOM);
      else {
        sendChar( ch, CONFIG_OK );
        command_interpreter(vict, message);
      }
    } else {                /* This is order "followers" */
      org_room = ch->in_room;

      for (k = ch->followers; k; k = kn) {
        kn = k->next;
        if (org_room == k->follower->in_room)
          if (IS_AFFECTED(k->follower, AFF_CHARM)) {
            found = TRUE;
            command_interpreter(k->follower, message);
          }
      }
      if (found)
        sendChar( ch, CONFIG_OK );
      else
        send_to_char("Nobody here is a loyal subject of yours!\r\n", ch);
    }
  }
}

ACMD(do_flee)
{
  int i, oldroom;

  if( STUNNED(ch) && !(IS_AFFECTED(ch, AFF_CHARM) && FIGHTING(ch))){ return; }

  if( IS_AFFECTED( ch, AFF_PARALYZE )){ return; }

  if (GET_POS(ch) < POS_FIGHTING)
  {
    static char *positions[] = {
      "dead!\r\n", "mortally wounded!\r\n", "incapacitated.\r\n",
      "stunned.\r\n", "sleeping.\r\n", "resting.\r\n", "sitting.\r\n",
    };
    sendChar(ch, "You can't run away while ");
    sendChar(ch, positions[GET_POS(ch)]);
    return;
  }

  if (ch->mount) {
      sendChar(ch, "You wildly knee your mount!\r\n");
      do_flee(ch->mount, "", 0, 0);
      return;
  }

  if( IS_AFFECTED(ch, AFF_HAMSTRUNG))
  {
    sendChar( ch, "Your legs hurt far too much for you to run away!\r\n");
    return;
  }

  if( IS_AFFECTED(ch, AFF_BERSERK))
  {
    sendChar( ch, "You are far too angry to flee! KILLL!!!!\r\n" );
    return;
  }

  

  if (IS_PVP(ch, FIGHTING(ch)) && !FLEEING(FIGHTING(ch)) && (number(1, 100) > fleeFactor(ch)))
  {
	  ch->flee_timer = 12;
	  act("$n tries to flee, but can't!", TRUE, ch, 0, 0, TO_ROOM);
	  sendChar( ch, "PANIC!  You couldn't escape!\r\n" );
	  return;
  }

  oldroom = IN_ROOM(ch);

  for( i = 0; i < 6; i++ )
  {
    int attempt = number(0, NUM_OF_DIRS - 1);

    if( CAN_GO(ch, attempt) && 
       !(IS_SET_AR(ROOM_FLAGS(EXIT(ch, attempt)->to_room), ROOM_DEATH) &&
           IS_NPC(ch)) &&
       (!IS_SET_AR(ROOM_FLAGS(EXIT(ch, attempt)->to_room), ROOM_TUNNEL) ||
        world[EXIT(ch, attempt)->to_room].people == 0))
    {
      act("$n panics, and attempts to flee!", TRUE, ch, 0, 0, TO_ROOM);

      if( exit_guarded(ch, attempt)) {
          /* do nothing, the guard has already booted you back */
      } else if( do_simple_move(ch, attempt, TRUE ))
      {
        sendChar( ch, "You flee head over heels.\r\n" );
        if( FIGHTING(ch))
        {
          if (!IS_NPC(ch)) {
            int loss = 0;
            loss  = GET_MAX_HIT(FIGHTING(ch)) - GET_HIT(FIGHTING(ch));
            loss *= GET_LEVEL(FIGHTING(ch)) / 10;
            loss = MIN(loss, GET_LEVEL(ch) * 25000);
            if (loss >= GET_EXP(ch)) loss = GET_EXP(ch) - 1;
            gain_exp(ch, -loss);
          }
          end_fight(ch);
        }
      }
      else
      {
        act("$n tries to flee, but can't!", TRUE, ch, 0, 0, TO_ROOM);
        ch->flee_timer = 12;
      }
      return;
    }
  }
/*
  if (!IS_NPC(ch))
  STUN_USER_MIN;
*/  
  sendChar( ch, "PANIC!  You couldn't escape!\r\n" );
}
