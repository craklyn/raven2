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
 **  $Log: befriend.c,v $
 **  Revision 1.1  2004/01/20 01:06:51  raven
 **  ranger veteran and legend, necromancer veteran
 **
 **
 */


/*
 ** STANDARD U*IX INCLUDES
 */

#define THIS_SKILL                SKILL_BEFRIEND

#define SKILL_ADVANCES_WITH_USE   TRUE
#define SKILL_ADVANCE_STRING      "You are Dr Doolittle himself!"
#define SKILL_MAX_LEARN           90
#define DEX_AFFECTS               FALSE
#define INT_AFFECTS               FALSE
#define WIS_AFFECTS               TRUE

#define STUN_MIN                  1
#define STUN_MAX                  1

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
#include "magic/magic.h"
#include "actions/fight.h"

/*
 ** MUD SPECIFIC GLOBAL VARS
 */

/*
 ** EXTERNAL DEFINITIONS OF SKILLS/SPELLS CALLED FROM THIS FILE
 */
int magic_savingthrow( CharData *caster, CharData *victim, int type );

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

ACMD(do_befriend)
{
    char arg[100];
    CharData *victim;
    struct follow_type *f;

    one_argument(argument, arg);

    IF_UNLEARNED_SKILL("You don't know how!\r\n");

    if (!*arg)
    {
        sendChar(ch, "What do you wish to befriend?\r\n");
        return;
    }

    if (!(victim = get_char_room_vis(ch, arg)))
    {
        sendChar(ch, "Nothing around by that name.\r\n");
        return;
    }
    IF_CH_CANT_BE_VICTIM("You hug yourself gleefully.\r\n");

    if (!IS_NPC(victim))
    {
        sendChar(ch, "You could try being nice to them instead?\r\n");
        return;
    }

    if (GET_RACE(victim) != RACE_ANIMAL)
    {
        sendChar(ch, "You don't feel you could form a bond with them.\r\n");
        return;
    }

    if (GET_MAX_HIT(victim) > GET_MAX_HIT(ch) * 5)
    {
        sendChar(ch, "You are too puny to befriend THAT!\r\n");
        return;
    }

    /* You cannot befriend a charmed creature */
    if (IS_AFFECTED(victim, AFF_CHARM))
    {
        sendChar(ch, "They're not willing to listen right now.\r\n");
        return;
    }

    /* Make sure there are no preexisting befriended critters */
    for (f = ch->followers; f; f = f->next)
    {
        if (affected_by_spell(f->follower, SKILL_BEFRIEND))
        {
            sendChar(ch, "You already have a friend!\r\n");
            return;
        }
    }

    act("$n whistles at $N beckoningly.", FALSE, ch, 0, victim, TO_ROOM);
    act("You whistle at $N beckoningly.", FALSE, ch, 0, victim, TO_CHAR);

    if (skillSuccess(ch, THIS_SKILL) &&
            !magic_savingthrow(ch, victim, SAVING_ROD))
    {
        advanceSkill(ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING,
                     DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS);

        if (victim->master)
            stop_follower(victim);
        add_follower(victim, ch);

        act("$N gazes at $n with adoration.", FALSE, ch, 0, victim, TO_ROOM);
        act("$N gazes at you with adoration.", FALSE, ch, 0, victim, TO_CHAR);
        add_affect(ch, victim, SKILL_BEFRIEND, GET_LEVEL(ch), APPLY_NONE, 0,
                (GET_LEVEL(ch)/2) TICKS, AFF_CHARM, FALSE, FALSE, FALSE, FALSE);

        REMOVE_BIT_AR(MOB_FLAGS(victim), MOB_SPEC);
    }
    else
    {
        act("$N looks at $n hungrily.", FALSE, ch, 0, victim, TO_ROOM);
        act("$N looks at you hungrily.", FALSE, ch, 0, victim, TO_ROOM);
        set_fighting(victim, ch);
    }
    STUN_USER_MIN;
}/* do_SKILL */

