/*
 **++
 **  RCSID:     $Id: sing.c,v 1.2 2003/11/06 06:36:37 raven Exp $
 **  Fixed?
 **  FACILITY:  RavenMUD
 **
 **  LEGAL MUMBO JUMBO:
 **
 **      This is based on code developed for DIKU and Circle MUDs.
 **
 **  MODULE DESCRIPTION:
 **
 **  AUTHORS:
 **
 **      Digger from RavenMUD (Hastily thrown together template :)
 **      Fleee from RavenMUD
 **      Craklyn from RavenMUD (adapted it into a chanting system)
 **
 */


/*
 **
 **  MODIFICATION HISTORY:
 **
 **  Revision 1.1  2002/05/11 03:52:31  raven
 **  the song system, and a few bugfixes
 **
 */


/*
 ** STANDARD U*IX INCLUDES
 */

/*
 ** MUD SPECIFIC INCLUDES
 */
#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/class.h"
#include "general/comm.h"
#include "general/handler.h"
#include "actions/interpreter.h"
#include "magic/skills.h"
#include "magic/spells.h"
#include "util/utils.h"
#include "magic/missile.h"
#include "specials/special.h"
#include "magic/sing.h"
#include "specials/muckle.h"

#define SINFO spell_info[songnum]
/*
 ** MUD SPECIFIC GLOBAL VARS
 */

/*
 ** EXTERNAL DEFINITIONS OF SKILLS/SPELLS CALLED FROM THIS FILE
 */
int
ok_damage_victim (CharData *ch, CharData *victim);

struct singer
{
  CharData *ch;
  struct singer *next;
  int sing_state;
} *singers = NULL;

void
add_singer (CharData *ch)
{
  struct singer *singer;

  CREATE (singer, struct singer, 1);
  singer->ch = ch;
  singer->next = singers;
  singers = singer;
}

void
do_fireball (CharData *ch, CharData *victim)
{
  int songnum = SINGING (ch);
  int dam = 0;
  int stunDuration = ch->state - FIREBALL1;

  if (ok_damage_victim (ch, victim))
    {
      // make sure victim exists, and such :)
      if (IN_ROOM (ch) == IN_ROOM (victim))
        {
          sendChar (ch, "You release your fireball on %s.\r\n",
                    GET_NAME (victim));
          sendChar (victim, "%s causes the fireball to descend on you!\r\n",
                    GET_NAME (ch), GET_NAME (victim));
          sprintf (buf, "%s causes the fireball to descend on %s!\r\n",
                   GET_NAME (ch), GET_NAME (victim));
          act (buf, FALSE, ch, 0, victim, TO_NOTVICT);

          // 5, 20, 35, 50, 65% chance to stun for how many rounds it was charged minus 1.

          if (IS_NPC (victim) && IS_SET_AR (MOB_FLAGS (victim), MOB_NOBASH))
          {
              // If the mob is unstunnable, then don't do anything, otherwise stun...
          }
          else if (stunDuration > 0 && percentSuccess (5 + 15 * stunDuration))
          {
              act("As the fireball strikes you, it explodes with concussive force!",
                      FALSE, victim, 0, ch, TO_CHAR);
              act("The fireball explodes upon impact with $n.",
                      FALSE, victim, 0, ch, TO_NOTVICT);
              act("You fireball explodes concussively upon impact with $N.",
                      FALSE, ch, 0, victim, TO_CHAR);
              WAIT_STATE(victim, SET_STUN(stunDuration));
          }

          dam = 50 + number (80, 95) * (stunDuration) + number (1, 70);
          damage (ch, victim, dam, songnum);
          ch->state = NOT_SINGING;

          return;
        }
    }

  sendChar (ch, "You realize your target is no longer nearby and dispel your fireball.\n");
  sprintf (buf, "%s dispels $s fireball with a look of disgust on $s face.\r\n",
           GET_NAME (ch));
  act (buf, FALSE, ch, 0, ch, TO_ROOM);

  ch->state = NOT_SINGING;
}

void
stop_singing (CharData *ch)
{
  struct singer *singer = singers, *prev = NULL;
  int songnum = SINGING (ch);

  while (singer)
    {
      if (singer->ch == ch)
        {
          if (prev) prev->next = singer->next;
          else singers = singer->next;
          free (singer);
          singer = NULL;
        }
      else singer = singer->next;
    }

  switch (songnum)
    {
    case SPELL_HEAL:
    case SPELL_REVIVE:
      send_to_char ("You end your prayer prematurely.\r\n", ch);
      act ("$n ends his prayer prematurely.\r\n", FALSE, ch, 0, 0, TO_ROOM);
      break;
    case SONG_MINOR_REFRESHMENT:
      send_to_char ("You stop singing.\r\n\r", ch);
      act ("$n ends $s song.\r\n", FALSE, ch, 0, 0, TO_ROOM);
      break;
    case SPELL_FIREBALL:
      do_fireball (ch, TARGET(ch));
      break;
    case SPELL_DANCE_SHADOWS:
      if (ch->state <= DANCE_SHADOWS6)
        {
          ch->state = NOT_SINGING;
          send_to_char ("You emerge from the shadows.\r\n", ch);
          act ("$n emerges from the shadows.\r\n", FALSE, ch, 0, 0, TO_ROOM);
        }
      else
        {
          send_to_char ("You realize you were exposed all along!\r\n", ch);
          ch->state = NOT_SINGING;
        }

      break;
    case SKILL_BERSERK:
      send_to_char ("You end your &08blood rage&00 prematurely.\r\n", ch);
      act ("$n ends his &08blood rage&00 prematurely.\r\n", FALSE, ch, 0, 0, TO_ROOM);
      break;
    case SPELL_SHIELD:
      send_to_char ("The reflective film around around you dissipates.\r\n", ch);
      act ("The shimmering film around $n dissipates rapidly.\r\n", FALSE, ch, 0, 0, TO_ROOM);
      break;
    default:
      send_to_char ("Please report that you saw this message - from sing.c\r\n", ch);
      break;
    }

  SINGING (ch) = 0;
  ch->state = NOT_SINGING;
  TARGET(ch) = NULL;
}

/* called for everyone in the room as a singer */
void
enact_song (CharData *ch, CharData *tch, int songnum)
{
  int grouped = in_same_group (ch, tch);

  /* don't be violent to group members */
  if (grouped && SINFO.violent) return;

  switch(songnum)
  {
    case SONG_MINOR_REFRESHMENT:
      GET_HIT (tch) += 5;
      if (GET_HIT(tch) > GET_MAX_HIT(tch))
        GET_HIT(tch) = GET_MAX_HIT(tch);
      break;
    }

  /* violent songs will have already done a message via damage() */
  if (!SINFO.violent) skill_message (0, ch, tch, songnum);
}

int
perform_song (CharData *ch)
{
    int songnum = SINGING (ch);
    CharData *tch, *nextch;

    // The below switch is for any chants etc that are just keeping
    // track of time really.
    switch (songnum)
      {
      case SPELL_HEAL:
        if (ch->state == HEAL1)
          {
            ch->state = HEAL2;
            send_to_char ("You chant the sacred words, 'poir'.\n\r", ch);
            act ("$n solemnly chants the words, 'poir'.\r\n", FALSE, ch, 0, 0, TO_ROOM);
            return 1;
          }
        else if (ch->state == HEAL2)
          {
            GET_HIT (ch) = MIN (GET_MAX_HIT (ch), GET_HIT (ch) + 100 + number (3, 8));
            send_to_char ("You complete your prayer for life.\n\r", ch);
            act ("$n completes his prayer for life.\r\n", FALSE, ch, 0, 0, TO_ROOM);
            ch->state = NOT_SINGING;
            return 0;
          }
        break;

      case SPELL_REVIVE:
        if (ch->state == REVIVE1)
          {
            if (number (0, 1)) ch->state = REVIVE2;
            else ch->state = REVIVE3;
            send_to_char ("You cry out for healing.\n\r", ch);
            act ("$n fervently cries the words, 'canduszuzo'.\r\n", FALSE, ch, 0, 0, TO_ROOM);
            return 1;
          }
        else if (ch->state == REVIVE2)
          {
            ch->state = REVIVE3;
            send_to_char ("You continue your pleas for health.\n\r", ch);
            act ("$n continues his pleas for health.\r\n", FALSE, ch, 0, 0, TO_ROOM);
            return 1;
          }
        else if (ch->state == REVIVE3)
          {
            GET_HIT (ch) = MIN (GET_MAX_HIT (ch), GET_HIT (ch) + 200 + number (4, 6));
            send_to_char ("You complete your regenerative prayer.\n\r", ch);
            act ("$n is revived by unseen forces.\r\n", FALSE, ch, 0, 0, TO_ROOM);
            ch->state = NOT_SINGING;
            return 0;
          }
        break;

      case SPELL_DANCE_SHADOWS:
        if (ch->state == DANCE_SHADOWS7)
          {
            if (!number (0, 3))
              {
                send_to_char ("You notice that you're standing in broad daylight!\n\r", ch);
                return 0;
              }
            else return 1;
          }

        if (ch->state == DANCE_SHADOWS6)
          ch->state = DANCE_SHADOWS1;
        else if (++(ch->state) == DANCE_SHADOWS6)
          {
            if(IS_PRESTIDIGITATOR (ch) && GET_ADVANCE_LEVEL (ch) >= 5)
              if (GET_MANA (ch) < GET_MAX_MANA (ch)) GET_MANA (ch) += number (5, 15);
            send_to_char ("You bide your time in the darkness.\n\r", ch);
          }

        if (!number (0, (IS_PRESTIDIGITATOR (ch) && GET_ADVANCE_LEVEL (ch) >= 5) ? 20 : 12))
          {
            ch->state = DANCE_SHADOWS7;
            act ("$n slowly emerges from darkness, but seems unaware of it.\r\n", FALSE, ch, 0, 0, TO_ROOM);
            WAIT_STATE (ch, SET_STUN(3));
            if(the_muckle == ch)
                WAIT_STATE (ch, SET_STUN(5));
          }
        return 1;

      case SPELL_FIREBALL:
          if ((ch->state) >= FIREBALL1)
          {
              if (IS_NPC(TARGET(ch)) && victimIsAngry (10 + 15 * (ch->state - FIREBALL1))
                      && IN_ROOM (ch) == IN_ROOM (TARGET (ch)) && ((ch->state) <= FIREBALL5))
              {
                  if(!FIGHTING(TARGET(ch))) {
                      act ("$N grows weary of the hovering fireball and attacks!",
                              FALSE, TARGET(ch), 0, TARGET(ch), TO_NOTVICT);
                  }
                  set_fighting(TARGET(ch), ch);
              }

              if ((ch->state) < FIREBALL5)
              {
                  if (++(ch->state) % 2)
                  {
                      act ("$n focuses $s energy into an increasingly large fireball swirling in the air.\r\n",
                              FALSE, ch, 0, 0, TO_ROOM);
                      send_to_char ("You focus your energy into the increasingly large fireball "
                              "swirling in the air.\r\n", ch);
                  }
              }
              else
              {
                  send_to_char ("Your fireball burns at its maximum ferocity, ready for you to release it.\r\n", ch);
              }

              return 1;
          }
          else return 0;
      case SKILL_BERSERK:
        if (ch->state == BERSERK1)
          {
            send_to_char ("You feel &08enraged&00!\r\n", ch);
            sprintf (buf, "$n works %sself into a &08blood rage&00!", HMHR (ch));
            act (buf, FALSE, ch, 0, 0, TO_ROOM);
            ch->state = BERSERK2;
            return 1;
          }
        if (ch->state == BERSERK2)
          {
            ch->state = BERSERK3;
            return 1;
          }
        else if (ch->state == BERSERK3)
          {
            send_to_char ("You calm down a bit.\n\r", ch);
            act ("$n calms down a bit.\r\n", FALSE, ch, 0, 0, TO_ROOM);
            ch->state = NOT_SINGING;
            return 0;
          }
        break;

      case SPELL_SHIELD:
        if (ch->state != SHIELD1)
          return 0;
        if (GET_MANA (ch) > 15) GET_MANA (ch) -= number (1, 15);
        else ch->state = NOT_SINGING;
        return 1;

      default:
        return 0;
        break;
      }


    for (tch = world[ch->in_room].people; tch; tch = nextch)
      {
        nextch = tch->next_in_room;

        enact_song (ch, tch, songnum);
      }

    return 1;
  }

  /* Called once every PULSE_VIOLENCE to perform song affects */
  void perform_songs (void)
  {
    struct singer *singer, *next, *prev = NULL;

    for (singer = singers; singer; singer = next)
      {
        next = singer->next;
        if (!perform_song (singer->ch))
          {
            if (prev) prev->next = next;
            else singers = next;
            SINGING (singer->ch) = 0;
            free (singer);
          }
        else prev = singer;
      }
  }

  /* entry point for mobs and PCs to start singing a song */
  int begin_singing (CharData *ch, CharData *victim, int songnum)
  {
    /* You can't sing while sleeping, resting or sitting */
    switch (GET_POS (ch))
      {
      case POS_SLEEPING:
        send_to_char ("You dream about your singing career.\r\n", ch);
        return 0;
      case POS_MEDITATING:
        send_to_char ("You cannot sing while in a trance.\r\n", ch);
        return 0;
      case POS_RESTING:
        send_to_char ("You cannot sing while resting.\r\n", ch);
        return 0;
      case POS_SITTING:
        send_to_char ("You can't sing while sitting down!\r\n", ch);
        return 0;
      }

    if (SINGING (ch)) stop_singing (ch);
    SINGING (ch) = songnum;
    add_singer (ch);

    switch (songnum)
      {
      case SPELL_HEAL:
        send_to_char ("You begin to chant a prayer of healing.\n\r", ch);
        act ("$n closes his eyes and chants the word, 'poir'.\r\n", FALSE, ch, 0, 0, TO_ROOM);
        ch->state = HEAL1;
        break;
      case SPELL_REVIVE:
        send_to_char ("You beg your gods for renewed lifeforce.\n\r", ch);
        act ("$n closes his eyes and chants, 'canduszuzo'.\r\n", FALSE, ch, 0, 0, TO_ROOM);
        ch->state = REVIVE1;
        break;
      case SPELL_FIREBALL:
        send_to_char ("You summon a fireball which hangs suspended in the air.\n\r", ch);
        act ("A fireball hangs suspended in the air.\r\n", FALSE, ch, 0, 0, TO_ROOM);
        ch->state = FIREBALL1;
        break;
      case SKILL_BERSERK:
        send_to_char ("You start working yourself into a ferocious rage!\r\n", ch);
        act ("$n screams like a feral beast!", FALSE, ch, 0, 0, TO_ROOM);
        ch->state = BERSERK1;
        break;
      case SPELL_DANCE_SHADOWS:
        send_to_char ("You blanket yourself in a cool layer of dark shadows.\n\r", ch);
        act ("$n disappears in a heavy blanket of shadows.\r\n", FALSE, ch, 0, 0, TO_ROOM);
        ch->state = DANCE_SHADOWS1;
        break;
      case SPELL_SHIELD:
        send_to_char ("You create a thick white film to protect you from attacks.\n\r", ch);
        //sprintf (buf, "$n creates a sphere of loosely bound particles around $sself.", GET_NAME (ch), HMHR (ch));
        act("$n creates a sphere of loosely bound particles around $mself.", FALSE, ch, 0, 0, TO_ROOM);
        ch->state = SHIELD1;
        break;
      case SONG_MINOR_REFRESHMENT:
      default:
        send_to_char ("You clear your throat and begin singing.\r\n", ch);
        act ("$n clears $s throat and begins to sing.\r\n", FALSE, ch, 0, 0, TO_ROOM);
        break;
      }

    return 1;
  }

  ACMD (do_sing)
  {
    CharData *victim = NULL;
    int songnum, moves;
    char *s;

    if (IS_NPC (ch))
      if (ch->desc == NULL)
        return;

    s = argument;
    skip_spaces (&s);

    if (s == NULL || *s == '\0')
      {
        send_to_char ("You raise your clear (?) voice towards the sky.\r\n", ch);
        act ("SEEK SHELTER AT ONCE!  $n has begun to sing.", FALSE,
             ch, 0, 0, TO_ROOM);
        return;
      }

    songnum = find_skill_num (s);

    if ((songnum < SONG_FIRST) || (songnum > MAX_SONGS))
      {
        send_to_char ("Sing what!?!\r\n", ch);
        return;
      }

    if (GET_LEVEL (ch) < SINFO.min_level[(int) GET_CLASS (ch)])
      {
        send_to_char ("You do not know that song!\r\n", ch);
        return;
      }

    if (!IS_NPC (ch))
      if (GET_SKILL (ch, songnum) == 0)
        {
          send_to_char ("You have not learned that song.\r\n", ch);
          return;
        }

    moves = SINFO.mana_max;

    if (moves > GET_MOVE (ch) && GET_LEVEL (ch) < LVL_IMMORT)
      {
        send_to_char ("You are too exhausted to start singing.\r\n", ch);
        return;
      }

    begin_singing (ch, victim, songnum);
  }

  // This command is used for what "stop" used to do - stop singing

  ACMD (do_release)
  {
    if (SINGING (ch)) stop_singing (ch);
  }

  ACMD (do_stop)
  {
      CharData *tch, *next_tch;

      // If you're singing, you can release your song without stun.
      if(SINGING(ch))
      {
          send_to_char ("You stop chanting.\r\n", ch);
          act ("$n stops chanting.", FALSE, ch, 0, 0, TO_ROOM);
          stop_singing (ch);
          return;
      }

      // This command is no longer strictly used just for singing.  It also
      // can bring a player out of combat.     
      if(FIGHTING(ch))
      {
        if(IS_AFFECTED(ch, AFF_BERSERK))  {
          sendChar(ch, "You are far too angry to stop fighting! KILLL!!!!\r\n");
          return;
        }
        
        send_to_char ("You stop attacking your opponent.\r\n", ch);
        act("$n stands down.", FALSE, ch, 0, 0, TO_ROOM);
        affect_from_char(ch, SPELL_MALEDICT2);
        WAIT_STATE(ch, PULSE_VIOLENCE + 1);
        stop_fighting(ch);
        return;
      }

      for (tch = world[ch->in_room].people; tch; tch = next_tch)
      {
          next_tch = tch->next_in_room;

          if (IS_AFFECTED (tch, AFF_CHARM) && tch->master && tch->master == ch)
          {
              do_stop (tch, 0, 0, 0);
              affect_from_char(tch, SPELL_MALEDICT2);
              WAIT_STATE (tch, PULSE_VIOLENCE + 1);
          }
      }

  }
