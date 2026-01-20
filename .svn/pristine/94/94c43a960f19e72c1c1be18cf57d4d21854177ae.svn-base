/*
**++
**  RCSID:     $Id: shadowstep.c,v 1.6 2005/08/02 02:36:34 raven Exp $
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
**      Mortius from RavenMUD
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
**  $Log: shadowstep.c,v $
**  Revision 1.6  2005/08/02 02:36:34  raven
**  Stuff didn't work, reverted code
**
**  Revision 1.4  2004/07/21 23:15:38  raven
**  current code state
**
**  Revision 1.3  2003/11/24 01:29:26  raven
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

#define THIS_SKILL                SKILL_SHADOW_STEP

#define SKILL_ADVANCES_WITH_USE   TRUE
#define SKILL_ADVANCE_STRING      "You become one with the shadows."

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
#define STUN_MIN                  2
#define STUN_MAX                  6

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
**      SKILL_NAME - [SHADOWSTEP]
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
ACMD(do_shadowstep)
{
    char arg[100];
    int direction, was_in_room;

    one_argument(argument, arg);
    skip_spaces(&argument);

    IF_UNLEARNED_SKILL   ("You wouldn't know where to begin.\r\n");

    if (GET_MOVE(ch) <= 11) {
        send_to_char("You feel too tired to sneak around just now.\r\n", ch);
        return;
    }

    direction = search_block(argument, dirs, FALSE);

    if (direction < 0 || !EXIT(ch, direction) ||
        EXIT(ch, direction)->to_room <= NOWHERE) {
        send_to_char("You are unable to move in that direction.\r\n", ch);
        return;
    }

    if (IS_SET(EXIT(ch, direction)->exit_info, EX_CLOSED)) {
        send_to_char("Your unable to step through solid objects.\r\n", ch);
        return;
    }

    if (ROOM_FLAGGED(EXIT(ch, direction)->to_room, ROOM_GODROOM)) {
        send_to_char("Something seems to block you from moving that way.\r\n", ch);
        return;
    }

    if (ROOM_FLAGGED(EXIT(ch, direction)->to_room, ROOM_CLAN)) {
	send_to_char("Something seems to block you from moving that way.\r\n", ch);
	return;
    }

    if (ch->mount) {
        sendChar(ch, "You cannot do that while mounted.\r\n", ch);
        return;
    }

    if (skillSuccess(ch, SKILL_SHADOW_STEP)) {          /* Skill Success */
        if (!IS_SET(EXIT(ch, direction)->exit_info, EX_CLOSED) &&
                EXIT(ch, direction)->keyword)
        {
            sprintf(buf, "The %s is closed.\r\n",
                    fname(EXIT(ch, direction)->keyword));
            GET_MOVE(ch) = MAX(0, MIN(GET_MAX_MOVE(ch), GET_MOVE(ch) - 2));
            STUN_USER_MIN;
        }
        else
        {
            GET_MOVE(ch) = MAX(0, MIN(GET_MAX_MOVE(ch), GET_MOVE(ch) - 5));
            was_in_room = ch->in_room;
            
            affect_from_char(ch, THIS_SKILL);
            add_affect(ch, ch, THIS_SKILL, GET_LEVEL(ch), 0, 0, 4, 0, FALSE, FALSE, FALSE, FALSE);

            act("$n steps into the shadows.", FALSE, ch, 0, 0, TO_ROOM);
            send_to_char("You step into the shadows and see,\r\n\r\n", ch);

            char_from_room(ch);
            char_to_room(ch, world[was_in_room].dir_option[direction]->to_room);
            look_at_room(ch, 1);
            advanceSkill(ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING,
                    DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS);
        }
    } 
    else {                                            /* Skill Fail */
        GET_MOVE(ch) = MAX(0, MIN(GET_MAX_MOVE(ch), GET_MOVE(ch) - 7));
        was_in_room = ch->in_room;
        char_from_room(ch);
        char_to_room(ch, world[was_in_room].dir_option[direction]->to_room);
        send_to_char("You step into the shadows but knock something over as you enter.\r\n", ch);
        look_at_room(ch, 1);
        act("You hear a noise as $n steps into the shadows and vanishes.\r\n",
                FALSE, ch, 0, 0, TO_ROOM);
        STUN_USER_MAX;
    }                                                    /* End of If */

}                                                       /* do_shadowstep */

