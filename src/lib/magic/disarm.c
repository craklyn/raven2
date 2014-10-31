/*
**++
**  RCSID:     $Id: disarm.c,v 1.5 2003/05/21 03:24:10 raven Exp $
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
**      Liam from RavenMUD
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
**  $Log: disarm.c,v $
**  Revision 1.5  2003/05/21 03:24:10  raven
**  lucky little halflings
**
**  Revision 1.4  2002/04/13 08:12:04  raven
**  demote command, wa/ra revisions
**
**  Revision 1.3  2001/03/20 19:37:57  raven
**
**  disarm/steal disabled in arena
**
**  added mana recharge object spell
**
**  Revision 1.2  2000/10/10 13:47:04  raven
**
**  Transitioned over to the new include structures.
**
**  Revision 1.1.1.1  2000/10/10 04:15:17  raven
**  RavenMUD 2.0
**
**  Revision 1.17  1998/02/18 10:23:34  vex
**  Prevented immortal level creatures being disarmed.
**
**  Revision 1.16  1998/01/29 03:38:48  digger
**  Removed all references to BLOOD_ENABLED
**
**  Revision 1.15  1997/12/07 15:54:27  vex
**  Cleanup crew came through.
**
**  Revision 1.14  1997/09/26 05:32:30  vex
**  Code clean up.
**
**  Revision 1.13  1997/09/18 12:52:36  vex
**  world was declared above the inclusion of structs.h, which caused a compiler
**  error after I switched everything to typdefs.
**
**  Revision 1.12  1997/09/18 10:49:05  vex
**  Replaced all obj_data, room_data, mob_special_data, char_data,
**  descriptor_data structs with appropriate typedef.
**
**  Revision 1.11  1997/05/08 02:14:51  liam
**  Removed bloodchk
**  Vex.
**
**  Revision 1.10  1997/03/04 19:28:04  digger
**  Fixed the double success/fail problem.
**
**  Revision 1.9  1997/02/27 19:23:13  digger
**  General cleanup.
**
**  Revision 1.8  1997/01/03 12:32:45  digger
**  Renamed several of the functions from skills.c and added skill
**  avoidance to fist and hamstring. Vex has put in MAJOR changes
**  to the summoning code and many checks for Book of Blood signatures
**  were added.
**
**  Revision 1.7  1996/06/17 11:50:53  digger
**  Modified disarm in the arena to keep players from dropping their weapons.
**
** Revision 1.1  1994/12/07  19:36:01  liam
** Initial revision
**
*/


/*
** STANDARD U*IX INCLUDES
*/
#include "general/conf.h"
#include "general/sysdep.h"

#define THIS_SKILL                SKILL_DISARM

#define SKILL_ADVANCE_WITH_USE    TRUE
#define SKILL_ADVANCE_STRING      "You have become notably more proficient."
#define SKILL_MAX_LEARN           (max_skill_learn[(int)GET_CLASS(ch)])
#define DEX_AFFECTS               TRUE
#define INT_AFFECTS               TRUE
#define WIS_AFFECTS               FALSE

#define STUN_MIN                  2
#define STUN_MAX                  3

/*                                Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  XX  XX  XX  XX  XX  XX  XX  XX  XX  XX */
static int max_skill_learn[] =  { 00, 00, 75, 90, 75, 00, 75, 75, 75, 75, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 };


/*
** MUD SPECIFIC GLOBAL VARS
*/

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

/*
** EXTERNAL DEFINITIONS OF SKILLS/SPELLS CALLED FROM THIS FILE
**
**
*/


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      do_disarm -
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
**      This allows one character to disarm another.
**
*/
ACMD(do_disarm)
{
    char arg[100];
    CharData *victim;

    one_argument(argument, arg);

    IF_UNLEARNED_SKILL    ( "You wouldn't know where to begin.\r\n" );
    IF_CH_CANT_SEE_VICTIM ( "Disarm who?\r\n" );
    IF_CH_CANT_BE_VICTIM  ( "Just unwield your weapon, dolt.\r\n" );
    IF_VICTIM_NOT_WIELDING( "They aren't even using a weapon!\r\n" );
    IF_ROOM_IS_PEACEFUL   ( "You playfully tug on their arm.\r\n" );
    IF_CANT_HURT_VICTIM;

    if(GET_CLASS(ch) == CLASS_SHOU_LIN) {
        IF_CH_IS_WIELDING("You can't do that while wielding a weapon.\r\n");
    }
    else {
        IF_CH_NOT_WIELDING("You can't do that with your bare hand!\r\n");
    }

    set_fighting(ch, victim);

    // ch must use the skill successfully and overcome an extra change hurdle
    int disarmPercent = 90 + (GET_LEVEL(ch) - GET_LEVEL(victim)) + (IS_IMMORTAL(victim) ? -40 : 0);
    disarmPercent = clamp( disarmPercent, 10, 90 );
    
    if (skillSuccess(ch, THIS_SKILL) && percentSuccess(disarmPercent)) {
        advanceSkill( ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING, DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS );

        skill_message(1, ch, victim, THIS_SKILL);

        // SHalflings get nearly double the chance of not being disarmed
        if ((IS_HALFLING(ch) && number(1,100) < 20) ||
                (GET_RACE(ch) == RACE_SHALFLING && number(1, 100) < 20)) {
            send_to_char("You snatch at your weapon as it falls, and somehow manage to recover it!\r\n", victim);
            act("$N snatches at $S weapon, and somehow manages to recovery it!", FALSE, ch, 0, victim, TO_CHAR);
            act("$N snatches at $S weapon, and somehow manages to recover it!", FALSE, ch, 0, victim, TO_NOTVICT);
            return;
        } else if(IS_DEFENDER(ch) && GET_ADVANCE_LEVEL(ch) >= 7 && percentSuccess(80)) {
            send_to_char("Your fists turn white gripping your weapon.  You manage not to drop it!\r\n", victim);
            act("$N's fists turn white gripping $S weapon, somehow managing not to drop it!", FALSE, ch, 0, victim, TO_CHAR);
            act("$N's fists turn white gripping $S weapon, somehow managing not to drop it!", FALSE, ch, 0, victim, TO_NOTVICT);
            return;
        }

        send_to_char ("You fumble your weapon and nearly drop it!\n\r", victim );
        affect_from_char(victim, THIS_SKILL);
        add_affect(ch, victim, THIS_SKILL, 0, APPLY_NONE, 0, 5, 0, FALSE, FALSE, FALSE, FALSE);
        STUN_USER_MIN;
    }/* if */
    else {
        skill_message(0, ch, victim, THIS_SKILL);
        STUN_USER_MAX;
    }

}/* do_disarm */
