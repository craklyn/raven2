
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

#define THIS_SKILL                SKILL_REDOUBT

#define SKILL_ADVANCES_WITH_USE   TRUE
#define SKILL_ADVANCE_STRING      "You learn to shake off the mightiest blows."
#define SKILL_MAX_LEARN           90
#define DEX_AFFECTS               FALSE
#define INT_AFFECTS               FALSE
#define WIS_AFFECTS               TRUE

#define STUN_MIN                  2
#define STUN_MAX                  3

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
**      SKILL_NAME - Redoubt
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
**      Sets the redoubt spell on.  The rest is taken care of
**      in other places
*/
ACMD(do_redoubt)
{
    IF_UNLEARNED_SKILL( "You should focus on doubt.\r\n" );

    if( COOLDOWN(ch, SLOT_REDOUBT) && !IS_IMMORTAL(ch) ) {
	sendChar(ch, "You are not ready to defend yourself so carefully again.\r\n");
	return;
    }

    if( !(ch->equipment[WEAR_SHIELD]) ) {
	sendChar(ch, "You must be wearing a shield to protect yourself properly.\r\n");
	return;
      }

    if( skillSuccess( ch, THIS_SKILL )) {
        send_to_char("You raise your shield and brace yourself.\r\n", ch);
        act("$n raises $s shield and braces for an onslaught.", TRUE, ch, NULL, NULL, TO_ROOM);
        advanceSkill( ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING, DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS );

	add_affect(ch, ch, THIS_SKILL, GET_LEVEL(ch), 0, 0, 
		4, 0, FALSE, FALSE, FALSE, FALSE);
	STUN_USER_MAX;
	COOLDOWN(ch, SLOT_REDOUBT) = (IS_DEFENDER(ch) && GET_ADVANCE_LEVEL(ch) >= THIRD_ADVANCE_SKILL)? 4:8;

    }/* if */
    else {
	send_to_char("You try, but fail, to prepare for the worst.\r\n", ch);
	STUN_USER_MIN;
    }

}/* do_redoubt */

