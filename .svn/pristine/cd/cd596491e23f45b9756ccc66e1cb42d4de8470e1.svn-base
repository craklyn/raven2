/*
**++
**  RCSID:     $Id:
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

#define THIS_SKILL                SKILL_SHADOW_DANCE

#define SKILL_ADVANCES_WITH_USE   TRUE
#define SKILL_ADVANCE_STRING      "You feel at home in the darkness."
#define SKILL_MAX_LEARN           90
#define DEX_AFFECTS               TRUE
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
#include "magic/skills.h"
#include "magic/spells.h"
#include "util/utils.h"
#include "util/weather.h"
#include "actions/fight.h"


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
**  shadowdance allows the shadow dancer to manipulate the surrounding
**  darkness so as to temporaraily fade from view... this allows the shadow
**  dancer to backstab an opponent they are fighting.
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
ACMD(do_shadowdance)
{
    char   arg[100];
    CharData *victim;
    CharData *tmp_ch = NULL;

    one_argument( argument, arg );
    
    IF_UNLEARNED_SKILL   ( "You have no idea how!\r\n" );
    IF_CH_CANT_SEE_VICTIM( "Backstab who?\r\n" );
    IF_CH_CANT_BE_VICTIM ( "You can't get a good reach in at your back.\r\n" );
    IF_ROOM_IS_PEACEFUL  ( "You do the hot shoe shuffle.\r\n" );
    IF_CH_NOT_WIELDING   ( "You may want to wield something first." );
    
    if( !FIGHTING(victim) ){
        send_to_char("Perhaps you'd better just backstab.\n\r", ch);
        return;
    }

    if( IS_NPC( victim ) && IS_SET_AR( MOB_FLAGS( victim ), MOB_AWARE )) {
        act("$N watches you attempt to merge with the darkness.\r\n", FALSE, ch, 0, victim, TO_CHAR );
        STUN_USER_MIN;
        return;
    }

    if(!( IS_DARK(ch->in_room) || IS_AFFECTED(ch, AFF_SHADOW_SPHERE))){
        act("You don't have any shadows to work with.", FALSE, ch, 0, tmp_ch, TO_CHAR);
        return;
    }

    if( GET_OBJ_VAL(ch->equipment[WEAR_WIELD], 3) != TYPE_PIERCE - TYPE_HIT ){
        send_to_char("Only piercing weapons can be used for backstabbing.\r\n", ch);
        return;
    }

    if( skillSuccess( ch, THIS_SKILL )){
        advanceSkill( ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING, DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS );

        if( victimAvoidsSkill( ch, victim, THIS_SKILL )){
            if( victimIsAngry( 30 ) ) set_fighting(victim, ch);
        }
        else {
            STUN_VICTIM_RANGE;
	    act("Shadows play around $n... $n vanishes!", FALSE, ch, 0, victim, TO_ROOM );
	    act("$n appears from the darkness.", FALSE, ch, 0, victim, TO_ROOM );
            hit(ch, victim, SKILL_SHADOW_DANCE);
        }
    }/* if */

    else {
	act("Shadows play around $n... $n vanishes!", FALSE, ch, 0, victim, TO_ROOM );
	act("$n appears from the darkness.", FALSE, ch, 0, victim, TO_ROOM );
        damage(ch, victim, 0, SKILL_SHADOW_DANCE);
    }/* else */

    STUN_USER_RANGE;

}/* do_SKILL */

