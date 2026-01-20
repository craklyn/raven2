/*
 **++
 **  RCSID:     $Id: shadowmist.c,v 1.3 2003/11/24 01:29:26 raven Exp $
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
 **  $Log: shadowmist.c,v $
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

#define THIS_SKILL                SKILL_SHADOW_MIST

#define SKILL_ADVANCES_WITH_USE   TRUE
#define SKILL_ADVANCE_STRING      "You become one with the mists around you."

/*
 ** SKILL_MAX_LEARN
 **         Define as either a hard number or use the skill/class array
 **         scheme to allow certain classes to learn particular skills
 **         better than others.
 **
 ** define SKILL_MAX_LEARN         90
 **
 */


static int max_skill_lvls[] = {

/* Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  XX  XX  XX  XX  XX  XX  XX  XX  XX  XX */
   00, 00, 00, 00, 00, 00, 00, 00, 00, 90, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00
};
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
#define STUN_MAX                  4

/*
 ** MUD SPECIFIC INCLUDES
 */
#include "general/conf.h"
#include "general/sysdep.h"

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
 ** EXTERNAL DEFINITIONS OF SKILLS/SPELLS CALLED FROM THIS FILE
 **
 **
 */
extern int find_door(CharData * ch, char *type, char *dir);

/*
 **++
 **  FUNCTIONAL DESCRIPTION:
 **
 **      SKILL_NAME - [ShadowMist]
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
 **      This is like doorbash apart from it puts the player on the other side
 **      of the door without opening the door.
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
ACMD(do_shadowmist)
{
    char dir[100];
    int door, direction, was_in_room;

    one_argument(argument, dir);

    IF_UNLEARNED_SKILL("You wouldn't know where to begin.\r\n");

    if (GET_MOVE(ch) <= 30)
    {
        send_to_char("Your body is too weary to form a fine mist.\r\n", ch);
        return;
    }

    if (!*dir)
    {
        send_to_char("In which direction do you wish to move?\r\n", ch);
        return;
    }

    if (ch->mount)
    {
        send_to_char("Your mount is too large to fit in the shadows.\r\n", ch);
        return;
    }

    if ((door = find_door(ch, "", dir)) < 0)
        return;

    if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR))
    {
        send_to_char("As the mist begins to form around your body you notice that there is no way to pass\r\nthrough this door.\r\n", ch);
        act("A mist begins to form around $n's body then quickly vanishes.",
            FALSE, ch, 0, 0, TO_ROOM);
        STUN_USER_MAX;
        return;
    }
    else if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
    {
        send_to_char("It's already open!\r\n", ch);
        return;
    }
    else if (IS_SET(EXIT(ch, door)->exit_info, EX_PICKPROOF))
    {
        send_to_char("There are no gaps to pass through in this door.\r\n", ch);
        return;
    }

    GET_MOVE(ch) -= number(22, 30);

    direction = search_block(argument + 1, dirs, FALSE);

    if (direction < 0 || !EXIT(ch, direction) ||
            EXIT(ch, direction)->to_room <= NOWHERE)
    {
        send_to_char("Shadow mist in which direction?\r\n", ch);
        return;
    }

    was_in_room = ch->in_room;

    if (skillSuccess(ch, SKILL_SHADOW_MIST))
    { /* Skill Success */
        act("A strange mist surrounds $n.  Once the mist has left $n is gone.",
            FALSE, ch, 0, 0, TO_ROOM);
        char_from_room(ch);
        char_to_room(ch, world[was_in_room].dir_option[direction]->to_room);
        send_to_char("You change form into a mist and flow through the cracks.\r\n", ch);
        look_at_room(ch, 1);
        advanceSkill(ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING,
                     DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS);
    }
    else
    { /* Skill Fail */
        send_to_char("A huge gust of wind blows the mist from your body.\r\n", ch);
        act("A strange mist forms around $n then leaves as quickly as it was formed.",
            FALSE, ch, 0, 0, TO_ROOM);
        STUN_USER_MAX;
    } /* End of If */
}/* do_shadowmist */

