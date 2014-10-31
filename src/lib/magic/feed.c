/*
**++
**  RCSID:     $Id: feed.c,v 1.2 2002/10/14 02:11:19 raven Exp $
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
**      Digger from RavenMUD
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
**  $Log: feed.c,v $
**  Revision 1.2  2002/10/14 02:11:19  raven
**  feed replenishes mana and moves also
**
**  Revision 1.1  2002/07/12 05:22:31  raven
**  remort code; bugfixes to curse and movement guards
**
**
**
*/


/*
** STANDARD U*IX INCLUDES
*/
#include "general/conf.h"
#include "general/sysdep.h"

#define THIS_SKILL                SKILL_FEED

#define SKILL_ADVANCES_WITH_USE   TRUE
#define SKILL_ADVANCE_STRING      "Your fangs have grown sharper."
#define SKILL_MAX_LEARN           90
#define DEX_AFFECTS               FALSE
#define INT_AFFECTS               TRUE
#define WIS_AFFECTS               FALSE

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
#include "actions/fight.h"
#include "magic/skills.h"
#include "magic/spells.h"
#include "util/utils.h"
#include "magic/feed.h"
#include "specials/special.h"

/*
** EXTERNAL DEFINITIONS OF SKILLS/SPELLS CALLED FROM THIS FILE
**
**
*/

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      do_feed -
**
**  FORMAL PARAMETERS:
**
**      ch:
**          A pointer to the character structure that is trying to use the skill/spell.
**		victim:
**			A pointer to the character structure that is being fed off of.
**
**
**  RETURN VALUE:
**
**      None
**
**  DESIGN:
**
**      A character (vampire) can drain life from an enemy and gain hit, mana
**		 and move for doing it.
**
*/
void feed_internal(CharData *ch, CharData *victim)
{
    int dam;
    dam = dice(4, GET_LEVEL(ch)/2 );
    damage(ch, victim, dice(1, dam), THIS_SKILL);
     
    if(IS_PVP(ch, victim))
        dam /= pvpFactor();
			
    GET_HIT(ch) += dam /2;
    GET_MANA(ch) += dam /4;
    GET_MOVE(ch) += dam /4;
  
    if (GET_HIT(ch) > GET_MAX_HIT(ch)) GET_HIT(ch) = GET_MAX_HIT(ch);
    if (GET_MANA(ch) > GET_MAX_MANA(ch)) GET_MANA(ch) = GET_MAX_MANA(ch);
    if (GET_MOVE(ch) > GET_MAX_MOVE(ch)) GET_MOVE(ch) = GET_MAX_MOVE(ch);

    mag_affects(GET_LEVEL(ch), ch, victim, SPELL_CURSE, SAVING_PARA);
    STUN_USER_MIN;

    if( (GET_COND(ch, THIRST) > 20))
    {
        sendChar( ch, "You are wholely satisfied by the &08blood&00 of your victims.\r\n" );
        return;
    }
    else
    {
        GET_COND(ch, THIRST) += 3;
        sendChar( ch, "The &08blood&00 of your victim satisfies your appetite.\r\n" );
    }

}

ACMD(do_feed)
{
    char   arg[100];
    CharData *victim;

    one_argument( argument, arg );

    IF_UNLEARNED_SKILL( "You wouldn't know where to begin.\r\n" );
    IF_CH_CANT_SEE_VICTIM( "Whose blood do you want to drink?\r\n" );
    IF_CH_CANT_BE_VICTIM( "You cannot feed off yourself.\r\n" );
    IF_ROOM_IS_PEACEFUL("You suck gently on their neck.\r\n");
    IF_CANT_HURT_VICTIM;

    if ( skillSuccess(ch, THIS_SKILL) || IS_RACIAL(ch)  ) {
        advanceSkill( ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING, DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS );
        feed_internal(ch, victim);
    }
    else {
          damage(ch, victim, 0, THIS_SKILL);
          STUN_USER_MAX;
    }/* else */

}/* do_feed */

