/*
**++
**  RCSID:     $Id: circle.c,v 1.7 2005/02/07 21:55:01 raven Exp $
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
**      Elendil from RavenMUD
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
**  $Log: circle.c,v $
**  Revision 1.7  2005/02/07 21:55:01  raven
**  Spec procs for clan 13(arc)
**
**  Revision 1.5  2003/11/14 01:42:42  raven
**  mount up
**
**  Revision 1.4  2001/08/23 01:06:37  raven
**  fixes to envenom, added 'pulse heal' and 'pulse gain' spells and affects, minlevel of 5 to post on board, a few typo fixes
**
**  Revision 1.3  2001/08/16 00:19:10  raven
**  assassin branch merged
**
**  Revision 1.2.2.1  2001/08/15 12:19:46  raven
**  assassin stuff
**
**  Revision 1.2  2000/10/10 13:47:04  raven
**
**  Transitioned over to the new include structures.
**
**  Revision 1.1.1.1  2000/10/10 04:15:17  raven
**  RavenMUD 2.0
**
**  Revision 1.12  1998/01/29 03:38:48  digger
**  Removed all references to BLOOD_ENABLED
**
**  Revision 1.11  1997/10/19 09:21:07  vex
**  Added art of the monkey/wind/flower.
**
**  Revision 1.10  1997/09/26 05:32:30  vex
**  Code clean up.
**
**  Revision 1.9  1997/09/18 10:47:59  vex
**  Replaced all obj_data, room_data, mob_special_data, char_data,
**  descriptor_data structs with appropriate typedef.
**
**  Revision 1.8  1997/05/18 04:27:36  liam
**  Changed Aware message cause it was annoyingly vague :p
**
**  Revision 1.7  1997/05/08 02:17:52  liam
**  removed blood_chk
**  Vex.
**
**  Revision 1.6  1997/01/09 20:41:40  digger
**  Changed the call from dam() to set_fighting().
**
**  Revision 1.5  1997/01/03 12:32:45  digger
**  Renamed several of the functions from skills.c and added skill
**  avoidance to fist and hamstring. Vex has put in MAJOR changes
**  to the summoning code and many checks for Book of Blood signatures
**  were added.
**
**  Revision 1.4  1996/02/21 12:33:57  digger
**  Added the IF_UNLEARNED_SKILL check.
**
** Revision 1.1  1994/12/16  14:23:52  jbp
** Initial revision
**
*/


/*
** STANDARD U*IX INCLUDES
*/
#include "general/conf.h"
#include "general/sysdep.h"

#define THIS_SKILL                SKILL_CIRCLE

#define SKILL_ADVANCES_WITH_USE   TRUE
#define SKILL_ADVANCE_STRING      "You've become notably more proficient."
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
**      Circle enables the skill user to sneak around behind the victim and
**      backstab that person while in combat, assuming that no-one is fighting
**      the user.
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
ACMD(do_circle)
{
    char   arg[100];
    CharData *victim;
    CharData *tmp_ch;

    one_argument( argument, arg );

    IF_UNLEARNED_SKILL   ( "You start playing ring-around-the-rosey.\r\n" );
    IF_CH_CANT_SEE_VICTIM( "Backstab who?\r\n" );
    IF_CH_CANT_BE_VICTIM ( "You can't get a good reach in at your back.\r\n" );
    IF_ROOM_IS_PEACEFUL  ( "You start playing ring around the rosey.\r\n" );
    IF_CH_NOT_WIELDING   ( "You may want to wield something first." );
    
    if( !FIGHTING(victim) ){
        send_to_char("Perhaps you'd better just backstab.\n\r", ch);
        return;
    }

    if (!IS_NPC(victim) && victim->mount) {
        sendChar(ch, "You cannot backstab a mounted person.\r\n");
        return;
    }

    if( IS_NPC( victim ) && IS_SET_AR( MOB_FLAGS( victim ), MOB_AWARE )) {
        act("$N is too alert for you to sneak behind.\r\n", FALSE, ch, 0, victim, TO_CHAR );
        STUN_USER_MIN;
        return;
    }

    for (tmp_ch = world[ch->in_room].people; tmp_ch; tmp_ch = tmp_ch->next_in_room) {
        if( FIGHTING(tmp_ch) == ch ){
            act("You are concentrating too much on avoiding $N.", FALSE, ch, 0, tmp_ch, TO_CHAR);
            return;
        }
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
            STUN_VICTIM_MIN;
            hit(ch, victim, THIS_SKILL);
        }
    }/* if */

    else {
        damage(ch, victim, 0, THIS_SKILL);
    }/* else */

    STUN_USER_RANGE;

}/* do_SKILL */

