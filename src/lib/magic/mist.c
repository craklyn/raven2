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
**  $Log: mist.c,v $
**  Revision 1.3  2003/11/24 00:01:06  raven
**  local changes
**
**  Revision 1.2  2003/09/25 23:28:55  raven
**  lotsastuff
**
**  Revision 1.1  2002/07/12 05:22:31  raven
**  remort code; bugfixes to curse and movement guards
**
**
*/


/*
** STANDARD U*IX INCLUDES
*/
#include "general/conf.h"
#include "general/sysdep.h"

#define THIS_SKILL                SKILL_MIST

#define SKILL_ADVANCES_WITH_USE   TRUE
#define SKILL_ADVANCE_STRING      "You have a stronger connection to the demon plane."
#define SKILL_MAX_LEARN           90
#define DEX_AFFECTS               FALSE
#define INT_AFFECTS               TRUE
#define WIS_AFFECTS               FALSE

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
#include "specials/special.h"

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

ACMD(do_mist)
{
    char   arg[100];
    CharData *victim;

    one_argument( argument, arg );

    IF_UNLEARNED_SKILL( "You don't know how!\r\n" );

    if (IS_GOOD(ch)) {
        sendChar(ch, "You cannot enter the demon realm.\r\n");
        return;
    }

    if (!*arg) {
        sendChar(ch, "Mist to who?\r\n");
        return;
    }

    if (!(victim = get_char_vis(ch, arg, 0))) {
        sendChar(ch, "No-one around by that name.\r\n");
        return;
    }
    IF_CH_CANT_BE_VICTIM("You begin a soul searching journey to find yourself.\r\n");

    if (IN_ROOM(ch) == IN_ROOM(victim)) {
        sendChar(ch, "You are already in the same room.\r\n");
        return;
    }

    if (!IS_NPC(victim) &&  (!PRF_FLAGGED(victim, PRF_SUMMONABLE) || !victim->desc)) {
        sendChar(ch, "The demon mists refuse to part for you.\r\n");
        return;
    }

    if (IN_ROOM(victim) < 0
     || SEEKING(victim)
     || ZONE_FLAGGED(world[victim->in_room].zone, ZONE_NOPORTAL)
     || IS_SET_AR(world[victim->in_room].room_flags, ROOM_PRIVATE)
     || IS_SET_AR(world[victim->in_room].room_flags, ROOM_DEATH)
     || IS_SET_AR(world[victim->in_room].room_flags, ROOM_GODROOM)
     || IS_SET_AR(world[victim->in_room].room_flags, ROOM_CLAN )
     || (IS_NPC(victim) && IS_SET_AR(MOB_FLAGS(victim), MOB_NOSUMMON))
     || ((zone_table[world[victim->in_room].zone].reset_mode == 3) && (GET_LEVEL(ch) < LVL_IMMORT))) {
        sendChar(ch, "You cannot pinpoint your target's location.\r\n");
        return;
    }

    if (GET_MOVE(ch) < (25 - GET_LEVEL(ch) / 5)) {
        sendChar(ch, "You are too exhausted to step through the demon realm.\r\n");
        return;
    }

    GET_MOVE(ch) = GET_MOVE(ch) - (25 - GET_LEVEL(ch)/5);

    if( skillSuccess( ch, THIS_SKILL )){
        advanceSkill( ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING, DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS );
        act("$n vanishes in a swirling mist.", FALSE, ch, 0, 0, TO_ROOM);
        sendChar(ch, "You vanish in a swirling mist.\r\n\r\n");
        char_from_room(ch);
        char_to_room(ch, IN_ROOM(victim));
        look_at_room(ch, 0);
        act("$n arrives in a swirling mist.", FALSE, ch, 0, 0, TO_ROOM);

        if (ch->mount) {
            ch->mount->rider = NULL;
            ch->mount = NULL;
            sendChar(ch, "Uhoh... your mount is missing!\r\n");
            sendChar(ch, "You land on your butt with a THUMP!\r\n");
            GET_POS(ch) = POS_SITTING;
        }
    } else {
        sendChar(ch, "You fail to step into the demon realm.\r\n");
        STUN_USER_MAX;
        return;
    }

    STUN_USER_MIN;

}/* do_SKILL */

