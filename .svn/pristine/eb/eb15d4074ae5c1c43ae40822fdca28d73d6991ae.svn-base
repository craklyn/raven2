/*
**++
**  RCSID:     $Id: instant poison.c,v 1.1 2002/01/14 12:56:42 raven Exp $
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
**
**  MODIFICATION HISTORY:
**
**  $Log: instant_poison.c,v $
**
** Creation 1.0  2005/11/20  20:40:22  Craklyn
** Initial skill creation
**
**
*/


/*
** STANDARD U*IX INCLUDES
*/
#include "general/conf.h"
#include "general/sysdep.h"

#define THIS_SKILL                SKILL_INSTANT_POISON

#define SKILL_ADVANCES_WITH_USE   TRUE
#define SKILL_ADVANCE_STRING      "You see more ways to debilitate your foes."
#define SKILL_MAX_LEARN           90
#define DEX_AFFECTS               TRUE
#define INT_AFFECTS               FALSE
#define WIS_AFFECTS               TRUE

#define STUN_MIN                  2
#define STUN_MAX                  3

#define NO_MOD   0

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
#include "magic/devour.h"
#include "specials/special.h"
#include "actions/fight.h"

/*
** EXTERNAL DEFINITIONS OF SKILLS/SPELLS CALLED FROM THIS FILE
**
**
*/


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      do_devour -
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
*/
ACMD(do_instant_poison)
{
    char   arg[100];
    CharData *victim;
    int dam = 0;

    one_argument( argument, arg );

    IF_UNLEARNED_SKILL( "You wouldn't know where to begin.\r\n" );
    IF_CH_CANT_SEE_VICTIM( "Who would you like to poison?\r\n" );
    IF_CH_CANT_BE_VICTIM( "You think better of it.\r\n" );
    IF_ROOM_IS_PEACEFUL("You shoot off some stinging words.\r\n");
    IF_CANT_HURT_VICTIM;

	if (skillSuccess(ch, THIS_SKILL)) {
        advanceSkill( ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING, DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS );
	
		if(OFF_BALANCE(victim) > 0)
		{ 
			act( "You take advantage of $M being vulnerable and inject a strong poison!", 
				FALSE, ch, NULL, victim, TO_CHAR );
			act( "$n inserts a narrow pin into $N's arm while he is vulnerable.", 
				TRUE, ch, NULL, victim, TO_NOTVICT );
			act( "$n inserts a narrow pin into your arm!", TRUE, ch, NULL, victim, TO_VICT );
			//Short paralysis for victims
			add_affect( ch, victim, SPELL_PARALYZE, 0, APPLY_AC, 20,
				number(1, 2), AFF_PARALYZE, FALSE, FALSE, FALSE, FALSE);
			
			// Slightly longer 'slow' as poison wears off
			if(!affected_by_spell(victim, SPELL_SLOW))
			add_affect( ch, victim, SPELL_SLOW, GET_LEVEL(ch), APPLY_NONE, NO_MOD, 
				4, NO_MOD, FALSE, FALSE, FALSE, FALSE);
		}
		else {
			act( "You inject $M with a weakly debilitating poison.", 
				FALSE, ch, NULL, victim, TO_CHAR );
			act( "$n pricks $N with a narrow pin.", 
				TRUE, ch, NULL, victim, TO_NOTVICT );
			act( "$n pricks you with a pin making you feel lethargic.", TRUE, ch, NULL, victim, TO_VICT );
		}

		add_affect( ch, victim, THIS_SKILL, GET_LEVEL(ch), APPLY_NONE,
			NO_MOD, 25, NO_MOD, FALSE, FALSE, FALSE, FALSE);
		STUN_USER_RANGE;
	}
	else {
		act( "You try to inject $M with poison, but drop the needle!", 
				FALSE, ch, NULL, victim, TO_CHAR );
		act( "$n approaches $N aggressively, but drops a small, thin impliment.", 
			TRUE, ch, NULL, victim, TO_NOTVICT );
		act( "$n tries to jab you with a pin, but it slips from his fingers!", TRUE, ch, NULL, victim, TO_VICT );
		STUN_USER_MAX;
	}

	/* regardless, we'll start fighting now */
	damage(ch, victim, dam, THIS_SKILL);

}/* do_instant_poison */
