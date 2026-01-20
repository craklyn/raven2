/*
**++
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
*/


/*
** STANDARD U*IX INCLUDES
*/
#include "general/conf.h"
#include "general/sysdep.h"

#define THIS_SKILL                SKILL_BREATHE

#define SKILL_ADVANCES_WITH_USE   TRUE
#define SKILL_ADVANCE_STRING      "You feel more in touch with your Dragon side."
#define SKILL_MAX_LEARN           90
#define DEX_AFFECTS               TRUE
#define INT_AFFECTS               FALSE
#define WIS_AFFECTS               FALSE

#define STUN_MIN                  2
#define STUN_MAX                  2

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
#include "actions/fight.h"

/*
** MUD SPECIFIC GLOBAL VARS
*/

/*
** EXTERNAL DEFINITIONS OF SKILLS/SPELLS CALLED FROM THIS FILE
**
**
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
ACMD(do_breathe)
{
    char   arg[100];
    CharData *victim;

    one_argument( argument, arg );

    IF_UNLEARNED_SKILL( "Only Dragons and Draconians have breath weapons.\r\n" );
    IF_CH_CANT_SEE_VICTIM( "Breathe on who?\r\n" );
    IF_CH_CANT_BE_VICTIM( "You breathe on yourself, perhaps those scales need polishing?\r\n" );
    IF_ROOM_IS_PEACEFUL( "A sense of calm overwhelms you.\r\n" );
    IF_CANT_HURT_VICTIM;
    
    if( skillSuccess( ch, THIS_SKILL) || IS_RACIAL(ch) ) {
        int amount_of_damage = (15 * GET_HIT(ch)/GET_MAX_HIT(ch)) * number(1, (int)(GET_LEVEL(ch)/2));
        advanceSkill( ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING, DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS );

	switch (GET_SUBRACE(ch)) {
	case RED_DRAGON:
		damage(ch, victim, amount_of_damage, SPELL_FIRE_BREATH);
		break;
	case GREEN_DRAGON:
		damage(ch, victim, amount_of_damage, SPELL_GAS_BREATH);
		break;
	case BLACK_DRAGON:
		damage(ch, victim, amount_of_damage, SPELL_ACID_BREATH);
		break;
	case WHITE_DRAGON:
		damage(ch, victim, amount_of_damage, SPELL_FROST_BREATH);
		break;
	case BLUE_DRAGON:
		damage(ch, victim, amount_of_damage, SPELL_LIGHTNING_BREATH);
		break;
	default:
		mudlog(NRM, LVL_IMMORT, TRUE, "breathe.c invoked for unknown breath type by %s (vnum %d) of race %d.", 
                        GET_NAME(ch), IS_NPC(ch)? GET_MOB_VNUM(ch) : -1, GET_RACE(ch));
		send_to_char("Your breath weapon type is unknown!\r\n", ch);
		return;
	}

    }/* if */

    else {
	switch (GET_SUBRACE(ch)) {
	case RED_DRAGON:
		damage(ch, victim, 0, SPELL_FIRE_BREATH);
		break;
	case GREEN_DRAGON:
		damage(ch, victim, 0, SPELL_GAS_BREATH);
		break;
	case WHITE_DRAGON:
		damage(ch, victim, 0, SPELL_FROST_BREATH);
		break;
	case BLACK_DRAGON:
		damage(ch, victim, 0, SPELL_ACID_BREATH);
		break;
	case BLUE_DRAGON:
		damage(ch, victim, 0, SPELL_LIGHTNING_BREATH);
		break;
	default:
                mudlog(NRM, LVL_IMMORT, TRUE, "breathe.c invoked for unknown breath type by %s (vnum %d) of race %d.",
                        GET_NAME(ch), IS_NPC(ch)? GET_MOB_VNUM(ch) : -1, GET_RACE(ch));
		send_to_char("Your breath weapon type is unknown!\r\n", ch);
		return;
	}
    }/* else */

    STUN_USER_RANGE;

}/* do_SKILL */

