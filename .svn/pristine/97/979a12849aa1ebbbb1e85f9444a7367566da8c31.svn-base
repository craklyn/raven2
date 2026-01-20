/*
**++
**  RCSID:     $Id: blind_strike.c,v 1.3 2003/10/13 23:02:11 raven Exp $
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
**      Digger from RavenMUD (template)
**      Fleee (Elendil) from RavenMUD
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
**
*/


/*
** STANDARD U*IX INCLUDES
*/
#include "general/conf.h"
#include "general/sysdep.h"

#define THIS_SKILL                SKILL_BLINDING_STRIKE

#define SKILL_ADVANCES_WITH_USE   TRUE
#define SKILL_ADVANCE_STRING      "You've become notably more proficient."
#define SKILL_MAX_LEARN           90
#define DEX_AFFECTS               TRUE
#define INT_AFFECTS               FALSE
#define WIS_AFFECTS               FALSE

#define STUN_MIN                  2
#define STUN_MAX                  4

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
#include "actions/outlaw.h"
#include "actions/fight.h"

/*
** MUD SPECIFIC GLOBAL VARS
*/

/*
** EXTERNAL DEFINITIONS OF SKILLS/SPELLS CALLED FROM THIS FILE
**
**
*/

int mag_savingthrow( CharData *ch, int savetype );


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      SKILL_NAME - blinding strike
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
**    Skill used to attempt to blind an opponent.
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
ACMD(do_blind_strike)
{
    char   arg[100];
    CharData *victim;
    struct affected_type af;

    one_argument( argument, arg );

    IF_UNLEARNED_SKILL   ( "They say playing with yourself will cause blindness.\r\n" );
    IF_CH_CANT_SEE_VICTIM( "Blind who?\n\r" );
    IF_CH_CANT_BE_VICTIM ( "You poke yourself in the eye.\r\n" );
    IF_ROOM_IS_PEACEFUL  ( "You playfully poke them in the eyes.\r\n" );
    IF_CH_IS_WIELDING    ( "You cannot attempt this while wielding a weapon.\r\n" );
	IF_CANT_HURT_VICTIM;
    
    if( artMonkey(ch, victim, THIS_SKILL) ) return;

    if( skillSuccess( ch, THIS_SKILL ) && !mag_savingthrow(victim, SAVING_SPELL)) {
        advanceSkill( ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING, DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS );
        act("You strike with tremendous speed, blinding $N.", FALSE, ch, 0, victim, TO_CHAR);
        act("$n's strike hits you out of nowhere, blinding you.", FALSE, ch, 0, victim, TO_VICT);
        act("$n strikes with tremendous speed, blinding $N.", FALSE, ch, 0, victim, TO_NOTVICT);

        af.type = SPELL_BLINDNESS;
        af.duration = 2 TICKS;
        af.bitvector = AFF_BLIND;
        af.location = APPLY_NONE;
        af.modifier = 0;
        affect_join(victim, &af, FALSE, FALSE, FALSE, FALSE);

        STUN_USER_MIN;
    }/* if */

    else {
        act("You attempt to blind $N, but $E nimbly avoids your strike.", FALSE, ch, 0, victim, TO_CHAR);
        act("You nimbly avoid $n's attempt to blind you.", FALSE, ch, 0, victim, TO_VICT);
        act("$N nimbly avoids $n's blinding strike.", FALSE, ch, 0, victim, TO_NOTVICT);
        STUN_USER_MIN;
    }/* else */

    if (!FIGHTING(ch))
        set_fighting(ch, victim);
    if (!FIGHTING(victim))
        set_fighting(victim, ch);
    player_attack_victim(ch, victim);
}/* do_SKILL */

