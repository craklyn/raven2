
/*
**++
**  RCSID:     $Id: feign_death.c,v 1.8 2005/07/29 07:09:52 raven Exp $
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
**		Elendil from RavenMUD
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
**  $Log: feign_death.c,v $
**  Revision 1.8  2005/07/29 07:09:52  raven
**  Feign death made partially dependent on a player's health, nicer to its users
**
**  Revision 1.7  2005/07/28 22:46:24  raven
**  Reduced chance of feign death success to be in line with other changes
**
**  
**  Revision 1.7 2005/07/28  Craklyn
**  Reduced chance of success by 1/5
**  
**  Revision 1.6  2004/01/28 02:21:54  raven
**  lotsastuff
**
**  Revision 1.5  2002/01/30 00:15:44  raven
**  fix noise crash bug
**
**  Revision 1.4  2001/08/04 06:59:06  raven
**  timed scrolls
**
**  Revision 1.3  2001/03/13 02:16:57  raven
**  end_fight() uses
**
**  Revision 1.2  2000/10/10 13:47:04  raven
**
**  Transitioned over to the new include structures.
**
**  Revision 1.1.1.1  2000/10/10 04:15:17  raven
**  RavenMUD 2.0
**
**  Revision 1.10  2000/04/06 14:56:33  mortius
**  Changed feign so whens its failed you have to stand back up.  Msg is
**  You lay down and pretend to be dead, so your on the ground.
**
**  Revision 1.9  1998/09/05 00:57:45  vex
**  Setup a flag to remember if user has feigned death for use in
**  backstab.c and offensive.c(do_hit).
**
**  Revision 1.8  1998/09/04 23:52:05  vex
**  The user of this skill is now only stunned if they FAIL.
**
**  Revision 1.7  1997/09/26 05:32:30  vex
**  Code clean up.
**
**  Revision 1.6  1997/09/18 12:52:36  vex
**  world was declared above the inclusion of structs.h, which caused a compiler
**  error after I switched everything to typdefs.
**
**  Revision 1.5  1997/09/18 10:51:47  vex
**  Replaced all obj_data, room_data, mob_special_data, char_data,
**  descriptor_data structs with appropriate typedef.
**
**  Revision 1.4  1997/01/03 12:32:45  digger
**  Renamed several of the functions from skills.c and added skill
**  avoidance to fist and hamstring. Vex has put in MAJOR changes
**  to the summoning code and many checks for Book of Blood signatures
**  were added.
**
**  Revision 1.3  1996/02/21 12:33:57  digger
**  Added the IF_UNLEARNED_SKILL check.
**
 * Revision 1.2  1995/07/26  21:09:21  digger
 * Removed the arg array.
 *
**  Revision 1.1  1995/03/22 14:16:24  raven
**  Initial revision
**
 * Revision 1.1  1995/02/14  04:39:13  raven
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

#define THIS_SKILL                SKILL_FEIGN_DEATH

#define SKILL_ADVANCES_WITH_USE   TRUE
#define SKILL_ADVANCE_STRING      "You've become notably more proficient."
#define SKILL_MAX_LEARN           90
#define DEX_AFFECTS               FALSE
#define INT_AFFECTS               FALSE
#define WIS_AFFECTS               FALSE

#define STUN_MIN                  0
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

/*
** EXTERNAL DEFINITIONS OF SKILLS/SPELLS CALLED FROM THIS FILE
**
**
*/


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      SKILL_NAME - feign death
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
**      feign death.  a skill which allows the user to fake death, making his/
**      her attackers stop in the belief of the death of the character.
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
ACMD(do_feign_death)
{
    int PERCENT_CHECK;  // Causes hurt players to be more likely to feign death.

    PERCENT_CHECK = 100 - 25 * GET_HIT(ch)/GET_MAX_HIT(ch);
    PERCENT_CHECK = MAX(PERCENT_CHECK, 75); //Never goes below 75
    
    IF_UNLEARNED_SKILL( "You couldn't fake an orgasm, much less your own death!\r\n" );

    if( !FIGHTING(ch) ){
        send_to_char("Sorry, you've got no-one to fool.\n\r", ch);
        return;
    }

    // Much harder to feign PvP
    // Impossible above 40% hp, 70% chance at 0 HP
    if (IS_PVP(ch, FIGHTING(ch)) && !FLEEING(FIGHTING(ch)) )
        PERCENT_CHECK = 100 - 250 * GET_HIT(ch) / GET_MAX_HIT(ch);
    
    if (ch->mount) {
        sendChar(ch, "You cannot feign death while mounted.\r\n");
        return;
    }

    if (IS_AFFECTED(ch, AFF_HAMSTRUNG)) {
      send_to_char("Your legs hurt too much for you to fake your own death!\r\n", ch);
      return;
    }

    // removing Craklyn BS - Bean
    /*  && (number(1,100) < PERCENT_CHECK) */
	if( skillSuccess( ch, THIS_SKILL )){
        advanceSkill( ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING, DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS );

        act("You feign death.", FALSE, ch, 0, 0, TO_CHAR);
        act("$n is dead!  R.I.P.", FALSE, ch, 0, 0, TO_ROOM);
        end_fight(ch);
        GET_POS(ch) = POS_SITTING;
	/* Allow them to take an action immediately, but if this action */
	/* is another attack, THEN stun them. */
	HAS_FEIGNED(ch) = 1;
    }/* if */

    else {
        act("You feign death.", FALSE, ch, 0, 0, TO_CHAR);
        act("$n lies down and pretends to be dead.", FALSE, ch, 0, 0, TO_ROOM);
		act("$n is motionless on the ground and extremely vulnerable!", FALSE, ch, 0, 0, TO_ROOM);
		OFF_BALANCE(ch) = 1;
	STUN_USER_MAX;
        GET_POS(ch) = POS_SITTING;
        update_pos(ch);
    }/* else */
}/* do_feign_death */

