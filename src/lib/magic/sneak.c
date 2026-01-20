
/*
**++
**  RCSID:     $Id: sneak.c,v 1.3 2002/04/13 08:12:04 raven Exp $
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
**  $Log: sneak.c,v $
**  Revision 1.3  2002/04/13 08:12:04  raven
**  demote command, wa/ra revisions
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
**  Revision 1.4  1997/09/18 11:05:03  vex
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
** Revision 1.1  1994/12/16  14:23:52  jbp
** Initial revision
**
**
*/


/*
** STANDARD U*IX INCLUDES
*/
#include "general/conf.h"
#include "general/sysdep.h"

#define THIS_SKILL                SKILL_SNEAK

#define SKILL_ADVANCES_WITH_USE   TRUE
#define SKILL_ADVANCE_STRING      "You can hardly believe your ears - you're making LESS noise!"
#define SKILL_MAX_LEARN           90
#define DEX_AFFECTS               TRUE
#define INT_AFFECTS               TRUE
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
ACMD(do_sneak)
{
#   define MAX_MOVE_COST     25
#   define MIN_MOVE_COST     25
    int sneak_cost = 25;

    IF_UNLEARNED_SKILL( "You wouldn't know where to begin.\r\n" );

    if (IS_AFFECTED(ch, AFF_SNEAK)) {
        send_to_char("You'll now stomp around like a warrior.\r\n", ch);
        affect_from_char(ch, THIS_SKILL);
        return;
    }

    if( GET_MOVE(ch) < sneak_cost ){
        send_to_char("You're too exhausted to go sneaking around.\r\n", ch);
        return;
    }/* if */

    GET_MOVE(ch) -= sneak_cost;

    send_to_char("Okay, you'll try to move silently for a while.\r\n", ch);

    if( IS_AFFECTED(ch, AFF_SNEAK)) return;

    if( skillSuccess( ch, THIS_SKILL )){
        struct affected_type af;

        advanceSkill( ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING, DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS );

        af.type      = THIS_SKILL;
        af.duration  = -1;
        af.modifier  = 0;
        af.location  = APPLY_NONE;
        af.bitvector = AFF_SNEAK;

        affect_to_char(ch, &af);
    }/* if */

    STUN_USER_RANGE;

}/* do_sneak */

