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
*/


/*
** STANDARD U*IX INCLUDES
*/
#include "general/conf.h"
#include "general/sysdep.h"

#define THIS_SKILL                SKILL_GORE

#define SKILL_ADVANCES_WITH_USE   TRUE
#define SKILL_ADVANCE_STRING      "The matadors will tremble in your prescence."
#define SKILL_MAX_LEARN           90
#define DEX_AFFECTS               TRUE
#define INT_AFFECTS               FALSE
#define WIS_AFFECTS               FALSE

#define STUN_MIN                  1
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
#include "specials/special.h"

/*
** MUD SPECIFIC GLOBAL VARS
*/

/*
** EXTERNAL DEFINITIONS OF SKILLS/SPELLS CALLED FROM THIS FILE
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
void gore_internal(CharData *ch, CharData *victim)
{
    if( skillSuccess( ch, THIS_SKILL) || IS_RACIAL(ch) ){
        int amount_of_damage = 3 * (str_app[STRENGTH_APPLY_INDEX(ch)].todam + number(1, GET_LEVEL(ch))) + GET_DAMROLL(ch);
        advanceSkill( ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING, DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS );

        if( victimAvoidsSkill( ch, victim, THIS_SKILL )){
            if( victimIsAngry( 30 ) ) set_fighting(victim, ch);
        }

        else {
            damage( ch, victim, amount_of_damage, THIS_SKILL );
        }

    }/* if */

    else {
        damage(ch, victim, 0, THIS_SKILL);
    }/* else */

    STUN_USER_RANGE;
}

ACMD(do_gore)
{
    char   arg[100];
    CharData *victim;

    one_argument( argument, arg );

    IF_UNLEARNED_SKILL( "You don't even have horns!\r\n" );
    IF_CH_CANT_SEE_VICTIM( "Gore who?\r\n" );
    IF_CH_CANT_BE_VICTIM( "Goring yourself, interesting idea..\r\n");
    IF_ROOM_IS_PEACEFUL( "A sense of calm overwhelms you.\r\n" );
    IF_CANT_HURT_VICTIM;

    gore_internal(ch, victim);

}/* do_SKILL */

