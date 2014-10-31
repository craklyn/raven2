/*
**++
**  RCSID:     $Id: shadowjump.c,v 1.2 2004/01/06 00:00:44 raven Exp $
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
**      Imhotep from RavenMUD
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
**  $Log: shadowjump.c,v $
**  Revision 1.2  2004/01/06 00:00:44  raven
**  local changes
**
**  Revision 1.1  2003/11/24 01:29:26  raven
**  shadow jump, Sd Legend
**
**  Revision 1.2  2000/10/10 13:47:04  raven
**
**  Transitioned over to the new include structures.
**
**  Revision 1.1.1.1  2000/10/10 04:15:17  raven
**  RavenMUD 2.0
**
**  Revision 1.6  1998/01/29 03:38:48  digger
**  Removed all references to BLOOD_ENABLED
**
**  Revision 1.5  1997/09/26 05:32:30  vex
**  Code clean up.
**
**  Revision 1.4  1997/09/18 11:00:44  vex
**  Replaced all obj_data, room_data, mob_special_data, char_data,
**  descriptor_data structs with appropriate typedef.
**
**  Revision 1.3  1997/01/03 12:32:45  digger
**  Renamed several of the functions from skills.c and added skill
**  avoidance to fist and hamstring. Vex has put in MAJOR changes
**  to the summoning code and many checks for Book of Blood signatures
**  were added.
**
**
*/


/*
** STANDARD U*IX INCLUDES
*/
#include "general/conf.h"
#include "general/sysdep.h"

#define THIS_SKILL                SKILL_SHADOW_JUMP

#define SKILL_ADVANCES_WITH_USE   TRUE
#define SKILL_ADVANCE_STRING      "Your shadow lengthens."

/*
** SKILL_MAX_LEARN 
**         Define as either a hard number or use the skill/class array
**         scheme to allow certain classes to learn particular skills
**         better than others.
**
** define SKILL_MAX_LEARN         90
**
*/

/*                                Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  XX  XX  XX  XX  XX  XX  XX  XX  XX  XX */
static int max_skill_lvls[] =   { 00, 00, 00, 00, 00, 00, 00, 00, 00, 90, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 };
#define SKILL_MAX_LEARN           max_skill_lvls[ (int)GET_CLASS(ch) ]

/*
** If dex, int, or wis affects the speed at which this skill is learned then
** set the following macros to TRUE. This means the user will learn the skill
** a little faster based on those attributes.
*/
#define DEX_AFFECTS               TRUE
#define INT_AFFECTS               FALSE
#define WIS_AFFECTS               TRUE

/*
** These are the two macros that define the range of the stun duration for
** an executed skill. These values are used for both the char and the victim.
*/
#define STUN_MIN                  1
#define STUN_MAX                  1

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
**      SKILL_NAME - [SHADOWJUMP]
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
**      Lets the player move into a room to look around without being seen.
**
**  NOTES:
**      The following standard macros can be used from skills.h:
**
**          IF_CH_CANT_SEE_VICTIM( string );    IF_VICTIM_NOT_WIELDING( string );
**          IF_CH_CANT_BE_VICTIM( string );     IF_VICTIM_CANT_BE_FIGHTING( string );
**          IF_CH_NOT_WIELDING( string );       IF_VICTIM_NOT_WIELDING( string );
**          IF_ROOM_IS_PEACEFUL( string );      IF_VICTIM_NOT_STANDING( string );
**          IF_UNLEARNED_SKILL( string );
**
**      The parameter `string' is the string that is sent to the user when the
**      error condition is met and the skill function exits. For example when an
**      attempt to disarm a weaponless victim is made:
**
**          IF_VICTIM_NOT_WIELDING( "That person isn't even wielding a weapon!\r\n" );
**
*/
ACMD(do_shadowjump)
{
    char arg[100];
    int direction, step, dist = 10, i, maxdist;

    one_argument(argument, arg);

    IF_UNLEARNED_SKILL   ("You wouldn't know where to begin.\r\n");

    if (ch->mount) {
        sendChar(ch, "You cannot do that while mounted.\r\n", ch);
        return;
    }

    if (GET_MOVE(ch) <= 11) {
        send_to_char("You feel to tired to leap around just now.\r\n", ch);
        return;
    }

    argument = one_argument(argument, arg);
    direction = search_block(arg, dirs, FALSE);

    if (direction < 0) {
        sendChar(ch, "Which direction is that?\r\n");
        return;
    }

    argument = one_argument(argument, arg);
    if (*arg >= '0' && *arg <= '9') {
        dist = *arg - '0';
    }

    /* Work out the real distance they're going to step */
    maxdist = (GET_SKILL(ch, THIS_SKILL) - 30) / 10;
    if (maxdist < 2) maxdist = 2;
    if (dist > maxdist) dist = maxdist;

    /* Take out 10MV */
    GET_MOVE(ch) -= 10;

    /* Attempt the skill */
    if (skillSuccess(ch, THIS_SKILL)) {
       advanceSkill(ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING,
               DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS);
       sendChar(ch, "You slide into the shadows.\r\n");
    } else {
        sendChar(ch, "You fail to enter the shadows.\r\n");
        dist = 0; /* don't step any rooms at all! */
    }

    act("$n melts into $s shadow.", TRUE, ch, 0, 0, TO_ROOM);

    /* Step as many rooms as they can */
    for (i = 0; i < dist; i++) {
        if (!EXIT(ch, direction) || EXIT(ch, direction)->to_room <= NOWHERE) {
            sendChar(ch, "Your shadow cannot pass through walls.\r\n");
            break;
        }

        /* closed, locked, !pick doors will block you too */
        if (IS_SET(EXIT(ch, direction)->exit_info, EX_CLOSED) &&
            IS_SET(EXIT(ch, direction)->exit_info, EX_LOCKED) &&
            IS_SET(EXIT(ch, direction)->exit_info, EX_PICKPROOF)) {
            sendChar(ch, "Your shadow cannot pass this door.\r\n");
            break;
        }

        /* GODROOM or CLAN rooms will block you. */
        if (ROOM_FLAGGED(EXIT(ch, direction)->to_room, ROOM_GODROOM) ||
            ROOM_FLAGGED(EXIT(ch, direction)->to_room, ROOM_CLAN)) {
            sendChar(ch, "Your shadow is mysteriously blocked.\r\n");
            break;
        }

        step = EXIT(ch, direction)->to_room;
        char_from_room(ch);
        char_to_room(ch, step);
    }

    act("$n slides out of $s shadow.", TRUE, ch, 0, 0, TO_ROOM);

    /* If they tried to move, show them their new location */
    if (dist > 0) look_at_room(ch, 0);

    STUN_USER_MIN;
}

