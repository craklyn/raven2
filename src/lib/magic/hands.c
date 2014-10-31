
/*
**++
**  RCSID:     $Id: hands.c,v 1.2 2000/10/10 13:47:04 raven Exp $
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
**  $Log: hands.c,v $
**  Revision 1.2  2000/10/10 13:47:04  raven
**
**  Transitioned over to the new include structures.
**
**  Revision 1.1.1.1  2000/10/10 04:15:17  raven
**  RavenMUD 2.0
**
**  Revision 1.9  2000/04/06 14:57:24  mortius
**  If hands is now failed you get a 2 round stun.
**
**  Revision 1.8  1997/09/26 05:32:30  vex
**  Code clean up.
**
**  Revision 1.7  1997/09/18 12:52:36  vex
**  world was declared above the inclusion of structs.h, which caused a compiler
**  error after I switched everything to typdefs.
**
**  Revision 1.6  1997/09/18 10:57:01  vex
**  Replaced all obj_data, room_data, mob_special_data, char_data,
**  descriptor_data structs with appropriate typedef.
**
**  Revision 1.5  1997/01/03 12:32:45  digger
**  Renamed several of the functions from skills.c and added skill
**  avoidance to fist and hamstring. Vex has put in MAJOR changes
**  to the summoning code and many checks for Book of Blood signatures
**  were added.
**
**  Revision 1.4  1996/06/17 11:51:27  digger
**  Boosted MAX_MANA_HANDS from 60 to 70.
**
** Revision 1.3  1996/02/21  12:32:33  digger
** Added the IF_UNLEARNED_SKILL check.
**
** Revision 1.2  1995/03/22  14:09:42  raven
** Updates too numerous to enumerate.
**
** Revision 1.1  1994/12/07  19:37:31  digger
** Initial revision
**
**
*/


/*
** STANDARD U*IX INCLUDES
*/
#include "general/conf.h"
#include "general/sysdep.h"

#define THIS_SKILL                SKILL_LAY_HANDS

#define SKILL_ADVANCE_WITH_USE    TRUE
#define SKILL_ADVANCE_STRING      "Your bond with the Shou-Lin spirits seems stronger."
#define SKILL_MAX_LEARN           90
#define DEX_AFFECTS               FALSE
#define INT_AFFECTS               TRUE
#define WIS_AFFECTS               TRUE

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
*/
ACMD(do_hands)
{
#   define MAX_MANA_HANDS  70

    char   arg[100];
    CharData *victim;
    int    max_mana = (MAX_MANA_HANDS - GET_LEVEL(ch));

    IF_UNLEARNED_SKILL( "You wouldn't know where to begin.\r\n" );

    one_argument( argument, arg );

    if (( GET_MANA( ch ) < max_mana ) && ( GET_LEVEL( ch ) < LVL_IMMORT )) {
        send_to_char("You don't have enough energy to call upon the Shou-Lin spirits.\r\n", ch);
        return;
    }

    if ( !(victim = get_char_room_vis(ch, arg)) &&
            !(victim = get_char_room(arg, ch->in_room)))
        send_to_char("Lay hands on who?\r\n", ch);
    else {

        // Advanced ancient dragons may lay on hands while blind.  Others may not.
        if(!(IS_ANCIENT_DRAGON(ch) && GET_ADVANCE_LEVEL(ch) >= 3))
            IF_CH_CANT_SEE_VICTIM("Lay hands on who?\r\n");

        if( skillSuccess(ch, SKILL_LAY_HANDS )){
            
            int  hit  = ( ch == victim ? number(16,32) : 8 ) + GET_LEVEL(ch) + (IS_ANCIENT_DRAGON(ch)? GET_ADVANCE_LEVEL(ch)*3/2 : 0);
            int  move = number( 1, GET_LEVEL(ch)>>2 );

            /* The AFFECTS on the victim */
            GET_HIT ( victim ) = MIN( GET_MAX_HIT ( victim ), GET_HIT ( victim ) + hit  );
            GET_MOVE( victim ) = MIN( GET_MAX_MOVE( victim ), GET_MOVE( victim ) + move );

            send_to_char( "Your hands pour out Shou-Lin spirits.\r\n", ch);
            if(ch == victim) {
                send_to_char( "Your wounds glow softly as the Shou-Lin spirits heal you.\r\n", ch);
                act( "$n lays hands on $Mself, healing $S wounds.", TRUE, ch, NULL, victim, TO_NOTVICT );
            }
            else {
                act( "$N's wounds glow softly as the Shou-Lin spirits heal them.", TRUE, ch, NULL, victim, TO_CHAR );
                act( "$n lays hands on $N, healing $S wounds.", TRUE, ch, NULL, victim, TO_NOTVICT );
                act( "Your wounds glow softly as $n calls on the Shou-Lin spirits to heal you.",
                        TRUE, ch, NULL, victim, TO_VICT );
            }

            advanceSkill( ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING, DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS );

            GET_MANA(ch) -= max_mana;
            
			// If ancient dragon, stun for 1 + (fraction of target's health) rounds.  Otherwise, standard 2 round stun
			if(IS_ANCIENT_DRAGON(ch) && GET_MAX_HIT(victim) != 0)
				WAIT_STATE(ch, SET_STUN(1) + PULSE_VIOLENCE * GET_HIT(victim)/GET_MAX_HIT(victim));
			else
				STUN_USER_RANGE;

        } else {                                /* Skill SUCCEEDED */
            GET_MANA(ch) -= (max_mana >> 1);
            send_to_char( "You botch your attempt to lay hands.\r\n", ch );
            act( "$n attempts to lay hands on $N but fails.", TRUE, ch, NULL, victim, TO_NOTVICT);
            act( "$n attempts to lay hands on you but fails.", TRUE, ch, NULL, victim, TO_VICT);
            STUN_USER_RANGE;
        }/* else */

    }/* else */

}/* do_hands */


