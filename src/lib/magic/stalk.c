
/*
**++
**  RCSID:     $Id: stalk.c,v 1.3 2001/08/04 06:59:06 raven Exp $
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
**      Elendil from RavenMUD
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
**  $Log: stalk.c,v $
**  Revision 1.3  2001/08/04 06:59:06  raven
**  timed scrolls
**
**  Revision 1.2  2000/10/10 13:47:04  raven
**
**  Transitioned over to the new include structures.
**
**  Revision 1.1.1.1  2000/10/10 04:15:17  raven
**  RavenMUD 2.0
**
**  Revision 1.6  1997/09/26 05:32:30  vex
**  code clean up
**
**  Revision 1.5  1997/09/18 12:52:36  vex
**  world was declared above the inclusion of structs.h, which caused a compiler
**  error after I switched everything to typdefs.
**
**  Revision 1.4  1997/09/18 11:06:42  vex
**  Replaced all obj_data, room_data, mob_special_data, char_data,
**  descriptor_data structs with appropriate typedef.
**
**  Revision 1.3  1997/01/03 12:32:45  digger
**  Renamed several of the functions from skills.c and added skill
**  avoidance to fist and hamstring. Vex has put in MAJOR changes
**  to the summoning code and many checks for Book of Blood signatures
**  were added.
**
**  Revision 1.2  1996/02/21 12:33:15  digger
**  Added the IF_UNLEARNED_SKILL check.
**
 * Revision 1.1  1995/03/22  14:16:55  raven
 * Initial revision
 *
 * Revision 1.1  1995/02/14  04:40:27  raven
 * Initial revision
 *
 * Revision 1.1  1994/12/16  14:23:52  jbp
 * Initial revision
 *
**
*/


/*
** STANDARD U*IX INCLUDES
*/
#include "general/conf.h"
#include "general/sysdep.h"

#define THIS_SKILL                SKILL_STALK

#define SKILL_ADVANCES_WITH_USE   TRUE
#define SKILL_ADVANCE_STRING      "You've become notably more proficient."
#define SKILL_MAX_LEARN           90
#define DEX_AFFECTS               FALSE
#define INT_AFFECTS               FALSE
#define WIS_AFFECTS               FALSE

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
**      SKILL_NAME - stalk
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
**      this skill enables the user to follow a victim and begin sneaking.
**
**  NOTES:
**      The following standard macros can be used from skills.h:
**
**          IF_CH_CANT_SEE_VICTIM( string );             IF_VICTIM_NOT_WIELDING( string );
**          IF_CH_CANT_BE_VICTIM( string );              IF_VICTIM_CANT_BE_FIGHTING( string );
**          IF_CH_NOT_WIELDING( string );
**
**      The parameter `string' is the string that is sent to the user when the
**      error condition is met and the skill function exits. For example when an
**      attempt to disarm a weaponless victim is made:
**
**          IF_VICTIM_NOT_WIELDING( "That person isn't even wielding a weapon!\r\n" );
**
*/
ACMD(do_stalk)
{
    char   arg[100];
    CharData *victim;
	struct follow_type *k;

    IF_UNLEARNED_SKILL( "You start acting like a piece of celery.\r\n" );

    one_argument( argument, arg );

    IF_CH_CANT_SEE_VICTIM( "Stalk who?\r\n" );

    IF_CH_CANT_BE_VICTIM( "You chase your tail.\r\n" );

	if (ch->master == victim)
	{
		act("You are already following $M.", FALSE, ch, 0, victim, TO_CHAR);
		return;
	}
	if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master))
	{
		act("You only feel like following $N!", FALSE, ch, 0, victim, TO_CHAR);
		return;
	}
	if (circle_follow(ch, victim))
	{
		act("Sorry, following in loops is not allowed.", FALSE, ch, 0, victim, TO_CHAR);
		return;
	}

    if( skillSuccess( ch, THIS_SKILL )){
	struct affected_type af;
        advanceSkill( ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING, DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS );
	if (affected_by_spell(ch, THIS_SKILL))
		affect_from_char(ch, THIS_SKILL);
        af.type      = THIS_SKILL;
        af.duration  = GET_LEVEL(ch) TICKS;
        af.modifier  = 0;
        af.location  = APPLY_NONE;
        af.bitvector = AFF_SNEAK;
		affect_to_char(ch, &af);
	act("You begin stalking $N.", FALSE, ch, 0, victim, TO_CHAR);
		
    }/* if */

    else {
        act("$n starts following you.", FALSE, ch, 0, victim, TO_VICT);
        act("$n starts following $N.", FALSE, ch, 0, victim, TO_NOTVICT);
	act("$N notices you fall in behind them.", FALSE, ch, 0, victim, TO_CHAR);
    }/* else */

    if (ch->master)
      stop_follower(ch);
    REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_GROUP);
    ch->master = victim;

    CREATE(k, struct follow_type, 1);

    k->follower = ch;
    k->next = victim->followers;
    victim->followers = k;

    STUN_USER_MIN;
}/* do_SKILL */

