/*
**++
**  RCSID:     $Id: charge.c,v 1.4 2004/02/25 04:36:53 raven Exp $
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
**  $Log: charge.c,v $
**  Revision 1.4  2004/02/25 04:36:53  raven
**  bug fixes
**
**  Revision 1.3  2004/02/17 04:05:36  raven
**  a few mount fixes
**
**  Revision 1.2  2004/02/11 00:19:57  raven
**  local changes
**
**  Revision 1.1  2004/01/28 02:21:54  raven
**  lotsastuff
**
**
*/


/*
** STANDARD U*IX INCLUDES
*/
#include "general/conf.h"
#include "general/sysdep.h"

#define THIS_SKILL                SKILL_CHARGE

#define SKILL_ADVANCES_WITH_USE   TRUE
#define SKILL_ADVANCE_STRING      "Target dummies everywhere quail in fear."

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
static int max_skill_lvls[] =   { 00, 00, 00, 00, 00, 00, 00, 90, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 };
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
#include "actions/act.h"          /* ACMDs located within the act*.c files */
#include "actions/fight.h"

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
ACMD(do_charge)
{
    char arg[100], target[100];
    int direction, i, maxdist;
    CharData *victim = NULL;
    int old_room, dam;

    one_argument(argument, arg);

    IF_UNLEARNED_SKILL   ("You wouldn't know where to begin.\r\n");
    IF_CH_NOT_WIELDING("You cannot do this barehanded!\r\n");

    if (!ch->mount) {
        sendChar(ch, "You must be mounted to do this.\r\n", ch);
        return;
    }

    if (GET_OBJ_VAL(ch->equipment[WEAR_WIELD], 3) != TYPE_IMPALE - TYPE_HIT) {
        sendChar(ch, "You cannot charge with that weapon!\r\n");
        return;
    }

    if (GET_MOVE(ch) <= 15) {
        send_to_char("You feel too tired to charge around.\r\n", ch);
        return;
    }

    argument = one_argument(argument, target);

    /* Work out the max distance they're able to charge */
    maxdist = MAX(1, (GET_SKILL(ch, THIS_SKILL) - 60) / 10);

    argument = one_argument(argument, arg);
    direction = search_block(arg, dirs, FALSE);

    if (direction < 0) {
        sendChar(ch, "Which direction is that?\r\n");
        return;
    }

    /* Hunt a suitable target in that direction */
    old_room = ch->in_room;
    while (!victim && maxdist--) {
        if (!world[ch->in_room].dir_option[direction]) break;
        if (world[ch->in_room].dir_option[direction]->to_room < 0) break;
        if (world[ch->in_room].dir_option[direction]->exit_info & EX_CLOSED)
            break;
        ch->in_room = world[ch->in_room].dir_option[direction]->to_room;
        if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) break;
        victim = get_char_room_vis(ch, target);
    }
    ch->in_room = old_room;

    if (!victim) {
        sendChar(ch, "There's no suitable target by that name.\r\n");
        return;
    }

    if (victim == ch) {
        sendChar(ch, "It would be silly to charge at yourself.\r\n");
        return;
    }

    /* Take out 15MV */
    GET_MOVE(ch) -= 15;

    /* Gadump.  Gadump.  Gadump. */
    sendChar(ch, "You heel your mount and charge off!\r\n");

    i = 0;
    sprintf(arg, "$n charges off %swards!", dirs[direction]);
    while (ch->in_room != victim->in_room) {
        old_room = world[ch->in_room].dir_option[direction]->to_room;
        act(arg, TRUE, ch, 0, 0, TO_ROOM);
        char_from_room(ch);
        char_to_room(ch, old_room);
        act("$n charges into the room!", TRUE, ch, 0, 0, TO_ROOM);
        look_at_room(ch, 0);
        i++;
    }

    /* Bring their mount along too */
    char_from_room(ch->mount);
    char_to_room(ch->mount, ch->in_room);

    /* Attempt the skill */
    if (skillSuccess(ch, THIS_SKILL)) {
        advanceSkill(ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING,
                DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS);
        if (victim->rider) awkward_dismount(victim->rider);
        if (victim->mount) awkward_dismount(victim);
        dam = calculateDamage(ch, victim, THIS_SKILL, 0) * i;
        GET_POS(victim) = POS_SITTING;
        STUN_VICTIM_MAX;
        damage(ch, victim, dam, THIS_SKILL);
    } else {
        damage(ch, victim, 0, THIS_SKILL);
    }

    STUN_USER_MIN;
}

