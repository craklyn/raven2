/*
**++
**  RCSID:     $Id: calm.c,v 1.2 2003/09/25 03:13:00 raven Exp $
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
**      Memnoch from RavenMUD
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
**  $Log: calm.c,v $
**  Revision 1.2  2003/09/25 03:13:00  raven
**  lots of things all at once
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

#define THIS_SKILL                SKILL_CALM

#define SKILL_ADVANCES_WITH_USE   TRUE
#define SKILL_ADVANCE_STRING      "You have become more peaceful."

/*
** SKILL_MAX_LEARN 
**         Define as either a hard number or use the skill/class array
**         scheme to allow certain classes to learn particular skills
**         better than others.
**
** define SKILL_MAX_LEARN         90
**
*/

#define SKILL_MAX_LEARN           90

/*
** If dex, int, or wis affects the speed at which this skill is learned then
** set the following macros to TRUE. This means the user will learn the skill
** a little faster based on those attributes.
*/
#define DEX_AFFECTS               FALSE
#define INT_AFFECTS               TRUE
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
#include "actions/fight.h"

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      SKILL_NAME - [calm]
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
ACMD(do_calm)
{
    IF_UNLEARNED_SKILL("You wouldn't know where to begin.\r\n");

    if (IS_EVIL(ch)) {
        sendChar(ch, "Your soul is too corrupt to use this skill.\r\n");
        return;
    }

    if (GET_MANA(ch) < 10) {
        sendChar(ch, "You do not have enough mana to try.\r\n");
        return;
    }

    GET_MANA(ch) = GET_MANA(ch) - 10;

    if( skillSuccess(ch, THIS_SKILL) && number(1,100) < GET_LEVEL(ch) + 27) {
        CharData *victim;

        advanceSkill( ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING, DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS );      
        act("$n extends a calming influence.", TRUE, ch, NULL, NULL, TO_ROOM);
        sendChar(ch, "You extend a calming influence.\r\n");

        victim = world[ch->in_room].people;
        while (victim) {
            if (FIGHTING(victim) ) {
                stop_fighting(victim);
            }
            victim = victim->next_in_room;
        }
    } else {
        sendChar(ch, "You fail to radiate peacefulness.\r\n");
    }

    STUN_USER_MIN;

}/* do_SKILL */
