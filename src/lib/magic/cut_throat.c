/*
 **++
 **  RCSID:     $Id: cut_throat.c,v 1.3 2004/07/21 23:15:38 raven Exp $
 **
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
 **      Digger from RavenMUD
 **
 **  NOTES:
 **
 **      Use 132 column editing in here.
 **
 **--
 */


/*
 **
 **  MODIFICATION HISTORY:
 **
 **  $Log: cut_throat.c,v $
 **  Revision 1.3  2004/07/21 23:15:38  raven
 **  current code state
 **
 **  Revision 1.2  2000/10/10 13:47:04  raven
 **
 **  Transitioned over to the new include structures.
 **
 **  Revision 1.1.1.1  2000/10/10 04:15:17  raven
 **  RavenMUD 2.0
 **
 **  Revision 1.6  1998/01/29 03:38:48  digger
 **  Removed all references to BLOOD_ENABLED
 **
 **  Revision 1.5  1997/09/26 05:32:30  vex
 **  Code clean up.
 **
 **  Revision 1.4  1997/09/18 11:00:44  vex
 **  Replaced all obj_data, room_data, mob_special_data, char_data,
 **  descriptor_data structs with appropriate typedef.
 **
 **  Revision 1.3  1997/01/03 12:32:45  digger
 **  Renamed several of the functions from skills.c and added skill
 **  avoidance to fist and hamstring. Vex has put in MAJOR changes
 **  to the summoning code and many checks for Book of Blood signatures
 **  were added.
 **
 **
 */


/*
 ** STANDARD U*IX INCLUDES
 */

#define THIS_SKILL                SKILL_CUT_THROAT

#define SKILL_ADVANCES_WITH_USE   TRUE
#define SKILL_ADVANCE_STRING      "You feel more ruthless."

/*
 ** SKILL_MAX_LEARN
 **         Define as either a hard number or use the skill/class array
 **         scheme to allow certain classes to learn particular skills
 **         better than others.
 **
 ** define SKILL_MAX_LEARN         90
 **
 */
#define SKILL_MAX_LEARN         90

/*
 ** If dex, int, or wis affects the speed at which this skill is learned then
 ** set the following macros to TRUE. This means the user will learn the skill
 ** a little faster based on those attributes.
 */
#define DEX_AFFECTS               TRUE
#define INT_AFFECTS               FALSE
#define WIS_AFFECTS               FALSE

/*
 ** These are the two macros that define the range of the stun duration for
 ** an executed skill. These values are used for both the char and the victim.
 */
#define STUN_MIN                  2
#define STUN_MAX                  4

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

/*
 **++
 **  FUNCTIONAL DESCRIPTION:
 **
 **      SKILL_NAME - [Cut Throat]
 **
 **  FORMAL PARAMETERS:
 **
 **      ch:
 **          A pointer to the character structure that is trying to use the skill/spell.
 **
 **  RETURN VALUE:
 **
 **      None
 **
 **  DESIGN:
 **
 **      Moderate Damage and silence
 **
 **  NOTES:
 **      The following standard macros can be used from skills.h:
 **
 **          IF_CH_CANT_SEE_VICTIM( string );    IF_VICTIM_NOT_WIELDING( string );
 **          IF_CH_CANT_BE_VICTIM( string );     IF_VICTIM_CANT_BE_FIGHTING( string );
 **          IF_CH_NOT_WIELDING( string );       IF_VICTIM_NOT_WIELDING( string );
 **          IF_ROOM_IS_PEACEFUL( string );      IF_VICTIM_NOT_STANDING( string );
 **          IF_UNLEARNED_SKILL( string );
 **
 **      The parameter `string' is the string that is sent to the user when the
 **      error condition is met and the skill function exits. For example when an
 **      attempt to disarm a weaponless victim is made:
 **
 **          IF_VICTIM_NOT_WIELDING( "That person isn't even wielding a weapon!\r\n" );
 **
 */
ACMD(do_cut_throat)
{
  struct affected_type af;
  char   arg[100];
  CharData *victim;
  int chance;
  bool success = 0;

    one_argument(argument, arg);

  IF_UNLEARNED_SKILL   ( "You wouldn't know where to begin.\r\n" );
  IF_CH_CANT_SEE_VICTIM( "Cut whose throat?\r\n" );
  IF_CH_CANT_BE_VICTIM ( "You think about the benefits of bettering the gene pool.\r\n" );
  IF_CH_NOT_WIELDING   ( "You need to wield a weapon to make it a success.\r\n" );
  IF_ROOM_IS_PEACEFUL  ( "Your violence has been suppressed.\r\n" );

    if (skillSuccess(ch, THIS_SKILL))
    {
        int cutthroat_damage = (number(1, (GET_LEVEL(ch) >> 1)) * 4);
        int cutthroat_victim = (number(1, 50) + ((GET_LEVEL(victim) - GET_LEVEL(ch)) * 5) + GET_DEX(victim));

        int cutthroat_user = (number(1, 50) + ((GET_LEVEL(ch) - GET_LEVEL(victim)) * 5) + GET_DEX(ch));

        advanceSkill(ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING,
                     DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS);
        /* Do damage first */
        damage(ch, victim, cutthroat_damage, THIS_SKILL);

      // Against mortals, Good chance of silence.  Against imms, 20%.
      if(GET_LEVEL(victim) < LVL_IMMORT) chance = 85;
      else chance = 25;

      if(IS_BRIGAND(ch) && affected_by_spell(victim, SKILL_FLASHBANG))
          chance += 20;

      success = number(chance/10, chance) > MIN(GET_LEVEL(victim), 25);

      if ( cutthroat_victim < cutthroat_user && success ) {
          af.type = SKILL_CUT_THROAT;
          af.duration = 1 TICKS;
          af.bitvector = AFF_SILENCE;
          af.location = APPLY_NONE;
          af.modifier = 0;
          affect_join(victim, &af, FALSE, FALSE, FALSE, FALSE);

            act("You start to choke on your own blood!",
                FALSE, ch, 0, victim, TO_VICT);
            act("$N starts to choke on their own blood!",
                FALSE, ch, 0, victim, TO_CHAR);
            act("$N starts to choke on their own blood!",
                FALSE, ch, 0, victim, TO_NOTVICT);

        }
    }
    else
    { /* if */
        /* make character pay the price for being clumsy */
        act("$n makes a wild slash at your throat, but only succeeds in cutting $mself!",
            FALSE, ch, 0, victim, TO_VICT);
        act("You loose balance and miss $N's throat. You've sliced open your own hand!",
            FALSE, ch, 0, victim, TO_CHAR);
        act("$n slashes at $N's jugular, but finishes with a weak suicide attempt.",
            FALSE, ch, 0, victim, TO_NOTVICT);

        damage(victim, ch, GET_LEVEL(victim), TYPE_UNDEFINED);
    }
    STUN_USER_MIN;
}

