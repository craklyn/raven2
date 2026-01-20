
/*
**++
**  RCSID:     $Id: convert.c,v 1.4 2001/05/23 03:23:00 raven Exp $
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
**      Imhotep from RavenMUD
**
**  NOTES:
**
**      Use 132 column editing in here.
**
**--
*/


/*
** STANDARD U*IX INCLUDES
*/
#include "general/conf.h"
#include "general/sysdep.h"

#define THIS_SKILL                SKILL_CONVERT

#define SKILL_ADVANCE_WITH_USE    TRUE
#define SKILL_ADVANCE_STRING      "You feel more effective at ministering!"
#define SKILL_MAX_LEARN           90
#define DEX_AFFECTS               FALSE
#define INT_AFFECTS               TRUE
#define WIS_AFFECTS               TRUE

#define STUN_MIN                  2
#define STUN_MAX                  2

/*
** MUD SPECIFIC INCLUDES
*/
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
** MUD SPECIFIC GLOBAL VARS
*/

/*
** EXTERNAL DEFINITIONS OF SKILLS/SPELLS CALLED FROM THIS FILE
**
**
*/


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      SKILL_NAME -
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
**      What's it do ?
**
*/
ACMD(do_convert)
{
# define MAX_MANA_CONVERT  100
# define MIN_MANA_CONVERT  26

  char   arg[100];
  CharData *victim;
  int max_mana = MAX(MIN_MANA_CONVERT, (MAX_MANA_CONVERT - GET_LEVEL(ch)*2));


  IF_UNLEARNED_SKILL( "You wouldn't know where to begin.\r\n" );

  one_argument( argument, arg );

  if (( GET_MANA( ch ) < max_mana ) && ( GET_LEVEL( ch ) < LVL_IMMORT )) {
    send_to_char("You don't have the energy to preach right now.\r\n", ch);
    return;
  }

  IF_ROOM_IS_PEACEFUL( "You are too calm to discuss religion right now.\r\n");


  if (!(victim = get_char_room_vis(ch, arg)))
    send_to_char("Preach to who?\r\n", ch);
  else if(GET_POS(victim) <= POS_SLEEPING)
    send_to_char("You can only preach to someone who is awake.\r\n", ch);
  else if (victim == ch) {
    send_to_char("You convince yourself you have chosen the right god.\r\n",
        ch);
  } else {
    if( skillSuccess(ch, SKILL_CONVERT )){

      // minister to them
      if (IS_EVIL(ch)) {
        GET_ALIGNMENT(victim) = MAX(-1000, GET_ALIGNMENT(victim) - 100);
      } else if (IS_GOOD(ch)) {
        GET_ALIGNMENT(victim) = MIN(1000, GET_ALIGNMENT(victim) + 100);
      } else {
        if (GET_ALIGNMENT(victim) > 0) {
          GET_ALIGNMENT(victim) = MAX(0, GET_ALIGNMENT(victim) - 100);
        } else {
          GET_ALIGNMENT(victim) = MIN(0, GET_ALIGNMENT(victim) + 100);
        }
      }

      // tell them what happened
      act("You instruct $N in the ways of your god.",
          FALSE, ch, NULL, victim, TO_CHAR);
      act("$n instructs you in the ways of $s god.",
           FALSE, ch, NULL, victim, TO_VICT);
      act("$n instructs $N in the ways of $s god.",
          FALSE, ch, NULL, victim, TO_NOTVICT);

      advanceSkill( ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING, DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS );

      GET_MANA(ch) -= max_mana;
      STUN_USER_RANGE;
    } else {                                /* Skill SUCCEEDED */
      GET_MANA(ch) -= (max_mana >> 1);
      send_to_char("You babble meaninglessly about your god.\r\n", ch );
      act( "$n babbles to $N about $s god..", TRUE, ch, NULL, victim, TO_ROOM );
      STUN_USER_RANGE;
    }
  }
}


