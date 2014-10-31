/*
**++
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
**      Vex from RavenMUD
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
*/


/*
** STANDARD U*IX INCLUDES
*/
#include "general/conf.h"
#include "general/sysdep.h"

#define THIS_SKILL                SKILL_TURN

#define SKILL_ADVANCES_WITH_USE   TRUE
#define SKILL_ADVANCE_STRING      "Your faith in your god grows stronger."
#define SKILL_MAX_LEARN           90
#define DEX_AFFECTS               FALSE
#define INT_AFFECTS               FALSE
#define WIS_AFFECTS               TRUE

#define STUN_MIN                  1
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
#include "specials/special.h"
#include "actions/fight.h"
#include "actions/act.h"          /* ACMDs located within the act*.c files */

/*
** MUD SPECIFIC GLOBAL VARS
*/

/*
** EXTERNAL DEFINITIONS OF SKILLS/SPELLS CALLED FROM THIS FILE
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
ACMD(do_flee);

ACMD(do_turn)
{
    char   arg[100];
    CharData *victim;

    one_argument( argument, arg );

    IF_UNLEARNED_SKILL( "You turn around, nifty.\r\n" );
    IF_CH_CANT_SEE_VICTIM( "Turn who?\r\n" );
    IF_CH_CANT_BE_VICTIM( "Turning yourself, interesting idea..\r\n");
    IF_ROOM_IS_PEACEFUL( "A sense of calm overwhelms you.\r\n" );
    IF_CANT_HURT_VICTIM;
    
    if ( !( IS_UNDEAD(victim) || IS_VAMPIRE(victim)) ){
	sendChar(ch, "You can only turn an undead creature!\r\n");
	return;
    }
	
    if( skillSuccess( ch, THIS_SKILL )){
        act( "You call upon your god to blast $n!", FALSE, victim, 0, ch, TO_VICT);
        act( "$n calls upon $s god to blast $N!", FALSE, ch, 0, victim, TO_ROOM);
        act( "$n calls upon $s god to harm you!", FALSE, ch, 0, victim, TO_VICT);
	if ( GET_LEVEL(ch) > GET_LEVEL(victim)) {
	    do_flee(victim, "", 0, 0);
            advanceSkill( ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING, DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS );
	}
	else {
	    sendChar(ch, "They are too powerful for you to affect!\r\n");
	    sendChar(victim, "They are too weak to affect you.\r\n");
	    hit(victim, ch, TYPE_UNDEFINED);
	}
    }/* if */

    else {
	sendChar(ch, "You failed!");
        act( "$n calls upon $s god to blast $N, but nothing seems to happen...", FALSE, ch, 0, victim, TO_NOTVICT);
	hit(victim, ch, TYPE_UNDEFINED);
    }/* else */

    STUN_USER_RANGE;

}/* do_SKILL */

