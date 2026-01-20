
/*
**++
**  RCSID:     $Id: expose.c,v 1.1 2002/04/13 08:12:04 raven Exp $
**
**  FACILITY:  RavenMUD
**
**  LEGAL MUMBO JUMBO:
**
**      This is based on code developed for DIKU and Circle MUDs.
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
**  $Log: expose.c,v $
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

#define THIS_SKILL                SKILL_EXPOSE

#define SKILL_ADVANCES_WITH_USE   FALSE
#define SKILL_ADVANCE_STRING      "You have become notably more proficient."
#define SKILL_MAX_LEARN           90
#define DEX_AFFECTS               FALSE
#define INT_AFFECTS               FALSE
#define WIS_AFFECTS               FALSE

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
#include "util/weather.h"

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
ACMD(do_expose)
{
    CharData *next_victim, *victim;
    int found = FALSE;

    IF_UNLEARNED_SKILL( "You wouldn't know where to begin.\r\n" );

    if(GET_MOVE(ch) < 15) {
        sendChar(ch, "You are too tired to search this room.\r\n");
        return;
    }

    GET_MOVE(ch) -= 15;

    if ((IS_DARK(ch->in_room)
            && !CAN_SEE_IN_DARK(ch))
            || SHADOW_NOT_OK(ch)
            || IS_AFFECTED(ch, AFF_BLIND))
    {
        sendChar(ch, "You cannot see anything at all - how can you find someone who is hiding?\r\n");
        return;
    }

    advanceSkill( ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING, DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS );

    for( victim = world[ch->in_room].people; victim; victim = next_victim ){
        next_victim = victim->next_in_room;

        if(!chGroupedWithTarget(ch, victim))
        {
            if(skillSuccess( ch, THIS_SKILL) &&
                    ((IS_AFFECTED(victim, AFF_HIDE) ||
                        affected_by_spell(victim, SPELL_DANCE_SHADOWS) ||
                        affected_by_spell(victim, SPELL_INVISIBLE) ||
                        affected_by_spell(victim, SPELL_SHADOW_SPHERE))))
            {
                found = TRUE;

                act("$n is dislodged from $s hiding place.", TRUE, victim, 0, ch, TO_ROOM);
                act("You are dislodged from hiding by $N", TRUE, victim, 0, ch, TO_CHAR);
                REMOVE_BIT_AR(AFF_FLAGS(victim), AFF_HIDE);
                affect_from_char(victim, SPELL_DANCE_SHADOWS);
                affect_from_char(victim, SPELL_INVISIBLE);
                affect_from_char(victim, SPELL_SHADOW_SPHERE);
                if(SINGING(victim) == SPELL_DANCE_SHADOWS)
                    stop_singing(victim);

                if((GET_INT(ch) + GET_DEX(ch)*number(7, 10) > (GET_INT(victim) + GET_DEX(ch))*number(7, 9))) {
                    victim = victim;
                    STUN_VICTIM_MAX;
                }

                // Now that it's exposed, we might want to attack it.
                aggroRoomCheck(victim, FALSE);
            }
        }
    }

    STUN_USER_MIN;
    if(!found)
        sendChar(ch, "You cannot find anything hidden here.\r\n");
}/* do_expose */

