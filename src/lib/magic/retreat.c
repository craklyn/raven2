
/*
**++
**  RCSID:     $Id: retreat.c,v 1.6 2005/07/28 20:24:56 raven Exp $
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
**  $Log: retreat.c,v $
**  Revision 1.6  2005/07/28 20:24:56  raven
**  I changed retreat to be less punishing to players
**
**  Revision 1.5  2003/11/24 00:01:06  raven
**  local changes
**
**  Revision 1.4  2001/08/20 00:18:49  raven
**  bugfixes to envenom, escape and retreat
**
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
**  Code clean up.
**
**  Revision 1.5  1997/09/18 12:52:36  vex
**  world was declared above the inclusion of structs.h, which caused a compiler
**  error after I switched everything to typdefs.
**
**  Revision 1.4  1997/09/18 11:02:32  vex
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
** Revision 1.1  1995/01/04  20:46:24  jbp
** Initial revision
**
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

#define THIS_SKILL                SKILL_RETREAT

#define SKILL_ADVANCES_WITH_USE   FALSE
#define SKILL_ADVANCE_STRING      "You've developed a deeper understanding of ways out of combat."
#define SKILL_MAX_LEARN           90
#define DEX_AFFECTS               TRUE
#define INT_AFFECTS               TRUE
#define WIS_AFFECTS               TRUE

#define STUN_MIN                  1
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
#include "actions/fight.h"
#include "actions/act.h"          /* ACMDs located within the act*.c files */

/*
** EXTERNAL DEFINITIONS OF SKILLS/SPELLS CALLED FROM THIS FILE
**
**
*/


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      SKILL_NAME - retreat
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
**      Allows you to get out of combat in a particular direction without losing xps
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

#define PERCENT_CHECK 	80 /* max chance of retreating even if successful */
 
ACMD(do_retreat)
{
    char   arg[100];
	char   buf[240];
	int dir;

    IF_UNLEARNED_SKILL( "You wouldn't know where to begin.\r\n" );

    one_argument( argument, arg );

	if (!FIGHTING(ch))
	{
		send_to_char("You aren't even in combat...why retreat?\r\n", ch);
		return;
	}

	if (IS_PVP(ch, FIGHTING(ch)) && !FLEEING(FIGHTING(ch)) && 
		(number(1, 100) > fleeFactor(ch) || (STUNNED(FIGHTING(ch)) && percentSuccess(PERCENT_CHECK)) ))
	{
		STUN_USER_MIN;
		act("$n looks around for a way out of the fight.",
                FALSE, ch, 0, 0, TO_ROOM);
		act("You don't see the opportunity to make your escape.",
			FALSE, ch, 0, 0, TO_CHAR);
		return;
	}

	if ((dir = search_block( arg, dirs, FALSE )) < 0)
	{
		send_to_char("What direction is that?\r\n", ch);
		return;
	}

	if(!CAN_GO(ch, dir))
	{
		send_to_char("You cannot retreat in that direction!\r\n", ch);
		return;
	}

    if (ch->mount) {
        if (IS_AFFECTED(ch->mount, AFF_HAMSTRUNG)) {
            sendChar(ch, "Your mount is unable to walk right now!\r\n");
            return;
        }
    } else if (IS_AFFECTED(ch, AFF_HAMSTRUNG)) {
      send_to_char("Your legs hurt too much for you to try running now!\r\n",
          ch);
      return;
    }

    if( skillSuccess( ch, THIS_SKILL )){
        advanceSkill( ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING, DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS );
        if (number(1,100) < PERCENT_CHECK) {
            sprintf(buf,"You retreat %s.", dirs[dir]);
            act(buf, FALSE, ch, 0, 0, TO_CHAR);
            sprintf(buf, "$n retreats %s.", dirs[dir]);
            act(buf, FALSE, ch, 0, 0, TO_ROOM);
            if (!exit_guarded(ch, dir)) {
                if (ch->mount)
                    do_simple_move(ch->mount, dir, 1);
                else
                    do_simple_move(ch, dir, 1);
            }
            STUN_USER_MIN;
        } else {
            act("You don't see the opportunity to make your escape.",
                    FALSE, ch, 0, 0, TO_CHAR);
            STUN_USER_RANGE;
        }
    }/* if */
    else {
        act("You can't see any way out of this right now!",
                FALSE, ch, 0, 0, TO_CHAR);
        act("$n looks around frantically for a way out!",
                FALSE, ch, 0, 0, TO_ROOM);
        STUN_USER_MAX;
    }/* else */
}/* do_SKILL */

