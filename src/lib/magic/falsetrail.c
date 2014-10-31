/*
**++
**  RCSID:     $Id: falsetrail.c,v 1.1 2003/11/14 04:04:40 raven Exp $
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
**  $Log: falsetrail.c,v $
**  Revision 1.1  2003/11/14 04:04:40  raven
**  false trails
**
**
*/


/*
** STANDARD U*IX INCLUDES
*/
#include "general/conf.h"
#include "general/sysdep.h"

#define THIS_SKILL                SKILL_FALSE_TRAIL

#define SKILL_ADVANCE_STRING      "You learn more about your own tracks."
#define SKILL_MAX_LEARN           90
#define DEX_AFFECTS               FALSE
#define INT_AFFECTS               TRUE
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
ACMD(do_false_trail)
{
    char   arg[100];

    int falsify_mv_cost = 50;

    IF_UNLEARNED_SKILL("You scuff the dirt with your foot hopefully.\r\n" );

    if( GET_MOVE(ch) < falsify_mv_cost ){
        send_to_char("You're too exhausted to lay a false trail.\r\n", ch);
        return;
    }/* if */

    GET_MOVE(ch) -= falsify_mv_cost;

    one_argument( argument, arg );

    if( skillSuccess( ch, THIS_SKILL )){
        advanceSkill( ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING, DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS );

        act("You carefully lay a false trail.", FALSE, ch, 0, 0, TO_CHAR);
        act("$n lays a false trail.", FALSE, ch, 0, 0, TO_ROOM);

        ch->false_trail = ch->in_room;
    } else {
        send_to_char("You lay a false trail, but walk through it.\r\n", ch);
    }

    STUN_USER_RANGE;

}/* do_false_trail */

