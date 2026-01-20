
/*
**++
**  RCSID:     $Id: berserk.c,v 1.3 2001/05/13 13:43:42 raven Exp $
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
**      Arbaces from RavenMUD
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

#define THIS_SKILL                SKILL_STEADFASTNESS

#define SKILL_ADVANCES_WITH_USE   TRUE
#define SKILL_ADVANCE_STRING      "You learn to better stand firm."
#define SKILL_MAX_LEARN           90
#define DEX_AFFECTS               FALSE
#define INT_AFFECTS               FALSE
#define WIS_AFFECTS               TRUE

#define STUN_MIN                  1
#define STUN_MAX                  4

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
**++
**  FUNCTIONAL DESCRIPTION:
**
**      SKILL_NAME - Steadfastness
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
**      Sets the steadfastness spell on.  The rest is taken care of
**      in other parts of the code
*/
ACMD(do_steadfastness)
{
    int steadfast_cost = 30;

    IF_UNLEARNED_SKILL( "You will have to stand on your own two feet.\r\n" );

    if(affected_by_spell(ch, SKILL_BERSERK)) {
       send_to_char("You are too enraged to hold your ground.\r\n", ch);
       return;
    }

    if(affected_by_spell(ch, SKILL_INVIGORATE)) {
	send_to_char("You are too invigorated to hold your ground.\r\n", ch);
	return;
    }

    if(!IS_NPC(ch) && GET_MOVE(ch) < steadfast_cost){
        send_to_char("You're too tired to stand fast.\r\n", ch);
        return;
    }/* if */

    if (!IS_NPC(ch))
	GET_MOVE(ch) -= steadfast_cost;

    if( skillSuccess( ch, THIS_SKILL )) {
        send_to_char("You plant your feet and stand fast.\r\n", ch);
        act("$n plants $s feet and stands fast.", TRUE, ch, NULL, NULL, TO_ROOM);
        advanceSkill( ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING, DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS );

        affect_from_char(ch, THIS_SKILL);
	add_affect(ch, ch, THIS_SKILL, GET_LEVEL(ch), 0, 0, 
		4 TICKS, 0, FALSE, FALSE, FALSE, FALSE);
    }/* if */
    else {
	send_to_char("You try, but fail to plant your feet firmly.\r\n", ch);
    }

    STUN_USER_MIN;

}/* do_sneak */

