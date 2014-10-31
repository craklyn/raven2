/*
**++
**  RCSID:     $Id: poultice.c,v 1.1 2002/04/13 08:12:04 raven Exp $
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
**  $Log: poultice.c,v $
**  Revision 1.1  2002/04/13 08:12:04  raven
**  demote command, wa/ra revisions
**
**
*/


/*
** STANDARD U*IX INCLUDES
*/
#include "general/conf.h"
#include "general/sysdep.h"

#define THIS_SKILL                SKILL_POULTICE

#define SKILL_ADVANCE_WITH_USE    TRUE
#define SKILL_ADVANCE_STRING      "You understand the tending of wounds much better now."
#define SKILL_MAX_LEARN           90
#define DEX_AFFECTS               FALSE
#define INT_AFFECTS               FALSE
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
ACMD(do_poultice)
{
#   define MOVE_COST    50

    char   arg[100];
    CharData *victim;

    IF_UNLEARNED_SKILL( "You wouldn't know where to begin.\r\n" );

    one_argument( argument, arg );

    if(!HAS_HERBS(ch)) {
        send_to_char("You must search a forest for herbs first.\r\n", ch);
        return;
    }

    if (!(victim = get_char_room_vis(ch, arg))) {
        sendChar(ch, "You have %d poultice%s prepared. Whose wounds would you like to mend?\r\n", HAS_HERBS(ch), SINGPLUR(HAS_HERBS(ch)));
        return;
    }

    if (( GET_MOVE( ch ) < MOVE_COST ) && ( GET_LEVEL( ch ) < LVL_IMMORT )) {
        send_to_char("You are too weary to prepare a poultice.\r\n", ch);
        return;
    }

    else HAS_HERBS(ch) -= 1;

    GET_MOVE(ch) -= MOVE_COST;

    if( skillSuccess(ch, THIS_SKILL )){
        int heal = (GET_MAX_HIT(victim) - GET_HIT(victim)) * GET_LEVEL(ch) / 200;
        GET_HIT(victim) += heal;
        affect_from_char(victim, SKILL_HAMSTRING);
        affect_from_char(victim, SKILL_DIRTY_TACTICS);
        affect_from_char(victim, SKILL_TRIP);
        if(affected_by_spell(victim, SKILL_DUST)) {
          affect_from_char( victim, SKILL_DUST );
          sendChar(victim, "The dust is flushed out of your eyes.");
        }
        
        send_to_char( "You prepare a poultice of herbs on a gauze bandage.\r\n", ch );
        act( "$n applies a poultice to $N's wounds.", TRUE, ch, NULL,
                victim, TO_NOTVICT );

        if(ch == victim)
            act("Your poultice eases your wounds.\r\n", TRUE, ch, NULL, victim, TO_VICT );
        else 
            act("$n's poultice eases your wounds.\r\n", TRUE, ch, NULL, victim, TO_VICT );

        
        advanceSkill( ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING,
                DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS );

        STUN_USER_RANGE;
    } else {
        send_to_char("You try to prepare a poultice, but discover you collected the wrong herbs!\r\n", ch );
        act( "$n attempts to prepare a poultice, but gives up in confusion.",
                TRUE, ch, NULL, victim, TO_ROOM );
        STUN_USER_RANGE;
    }
}/* do_poultice */


