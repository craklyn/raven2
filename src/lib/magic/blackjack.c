/*
**++
**  RCSID:     $Id: brain.c,v 1.3 2001/03/13 02:16:57 raven Exp $
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
**      Craklyn from RavenMUD
**
**  NOTES:
**
**      Use 132 column editing in here.
**
**--
*/

/*
** STANDARD U*IX INCLUDES
*/
#include "general/conf.h"
#include "general/sysdep.h"

#define THIS_SKILL                SKILL_BLACKJACK

#define SKILL_ADVANCES_WITH_USE   TRUE
#define SKILL_ADVANCE_STRING      "You've become more proficient at dealing spades."


#define SKILL_MAX_LEARN         90
        
/*
** If dex, int, or wis affects the speed at which this skill is learned then
** set the following macros to TRUE. This means the user will learn the skill
** a little faster based on those attributes.
*/
#define DEX_AFFECTS               TRUE
#define INT_AFFECTS               FALSE
#define WIS_AFFECTS               FALSE

/*
** These are the two macros that define the range of the stun duration for
** an executed skill. These values are used for both the char and the victim.
*/
#define STUN_MIN                  2
#define STUN_MAX                  3

/*
** MUD SPECIFIC INCLUDES
*/
#include <stdlib.h>

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
**      SKILL_NAME - [ Brain ]
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
**      With this skill the player will take his weapon and smash it off the victim's
**	head.  It does no damage, but disables the target making them behave as though
**      paralyzed.  Any action taken against the target removes this effect.
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
ACMD(do_blackjack)
{
    char   arg[100];
    CharData *victim;

    one_argument( argument, arg );

    IF_UNLEARNED_SKILL   ( "You wouldn't know where to begin.\r\n" );
    IF_CH_CANT_SEE_VICTIM( "Blackjack who?\r\n" );
    IF_CH_CANT_BE_VICTIM ( "Aren't we funny today...\r\n" );
    IF_CH_NOT_WIELDING   ( "You need to wield a weapon to make it a success.\r\n" );
    IF_ROOM_IS_PEACEFUL  ( "Your violence has been suppressed.\r\n" );

    if(COOLDOWN(ch, SLOT_BLACKJACK))
    {
        act("You are not ready to blackjack another enemy.",
                FALSE, ch, 0, victim, TO_CHAR);
        return;
    }
    if(FIGHTING(victim) == ch) {
        act("You can't blackjack someone who is focusing his attention on you.",
                FALSE, ch, 0, victim, TO_CHAR);
        return;
    }

    // The skill becomes more and more difficult against increasingly large opponents.
    // Cannot blackjack an opponent with more than 16000 hitpoints (about half the max)
    if (skillSuccess(ch, THIS_SKILL) && GET_MAX_HIT(victim) < number(1, 16000)) {
        advanceSkill(ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING,
                DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS);
        
        end_fight(victim);
        COOLDOWN(ch, SLOT_BLACKJACK) = 3;

        act("You are stunned senseless as someone smacks you on the backside of your head.",
                FALSE, ch, 0, victim, TO_VICT);
        act("You smack $N from behind; $E sees stars and slumps over, knocked out.",
                FALSE, ch, 0, victim, TO_CHAR);
        act("$N sees stars and slumps over, knocked out, after $n blackjacks $M.",
                FALSE, ch, 0, victim, TO_NOTVICT);
        add_affect(ch, victim, SKILL_BLACKJACK, GET_LEVEL(ch),
                0, 0, 20, AFF_PARALYZE, FALSE, FALSE, FALSE, FALSE);

        STUN_USER_MIN;
        STUN_VICTIM_MAX;
    } else {
        act("Your blackjack is easily sidestepped by $N.", FALSE, ch, 0, victim, TO_CHAR);
        act("You easily sidestep $n's attempt to blackjack you.", FALSE, ch, 0, victim, TO_VICT);
        act("$N easily sidesteps $n's attempted blackjack.", FALSE, ch, 0, victim, TO_NOTVICT);
        STUN_USER_MAX;
        damage(ch, victim, 0, THIS_SKILL);
    }
}
