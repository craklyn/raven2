
/*
**++
**  RCSID:     $Id: berserk.c,v 1.3 2001/05/13 13:43:42 raven Exp $
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
**  $Log: berserk.c,v $
**  Revision 1.3  2001/05/13 13:43:42  raven
**  grammatical error
**
**  Revision 1.2  2000/10/10 13:47:04  raven
**
**  Transitioned over to the new include structures.
**
**  Revision 1.1.1.1  2000/10/10 04:15:17  raven
**  RavenMUD 2.0
**
**  Revision 1.1  2000/09/09 19:11:27  mortius
**  Initial revision
**
**  Revision 1.5  1997/09/26 05:32:30  vex
**  Code clean up.
**
**  Revision 1.4  1997/09/18 12:52:36  vex
**  world was declared above the inclusion of structs.h, which caused a compiler
**  error after I switched everything to typdefs.
**
**  Revision 1.3  1997/09/18 10:45:02  vex
**  Replaced all obj_data, room_data, mob_special_data, char_data,
**  descriptor_data structs with appropriate typedef.
**
**  Revision 1.2  1997/09/12 07:05:51  vex
**  *** empty log message ***
**
**  Revision 1.1  1997/07/01 08:50:49  Vex
**  Initial revision
**
**
**
*/


/*
** STANDARD U*IX INCLUDES
*/
#include "general/conf.h"
#include "general/sysdep.h"

#define THIS_SKILL                SKILL_BERSERK

#define SKILL_ADVANCES_WITH_USE   TRUE
#define SKILL_ADVANCE_STRING      "You feel at one with the &08blood rage&00!"
#define SKILL_MAX_LEARN           90
#define DEX_AFFECTS               FALSE
#define INT_AFFECTS               FALSE
#define WIS_AFFECTS               TRUE

#define STUN_MIN                  1
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
#include "magic/sing.h"

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
**      Sets Berserk bit, rest taken care of else where for the most part.
**
*/
ACMD(do_berserk)
{
    int berserk_cost = 50;

    IF_UNLEARNED_SKILL( "You're not violent enough to enter a &08blood rage&00.\r\n" );

    if( affected_by_spell(ch, SKILL_STEADFASTNESS) ) {
       send_to_char("You are too steadfast to enter a &08blood rage&00.\r\n", ch);
       return;
       }
    if( affected_by_spell(ch, SKILL_INVIGORATE) ) {
	send_to_char("You are too invigorated to enter a &08blood rage&00.\r\n", ch);
        return;
       }

    if( IS_AFFECTED(ch, AFF_BERSERK) )
    {
        if( !IS_NPC(ch) && GET_MOVE(ch) < berserk_cost * 2 / 5 ){
            send_to_char("You're too tired to enter the &08blood rage&00.\r\n", ch);
            return;
        }/* if */

        if (!FIGHTING(ch) )
        {
            send_to_char("You should be fighting someone if you want to get that angry.\r\n", ch);
            return;
        }
        
        if (!IS_NPC(ch))
            GET_MOVE(ch) -= berserk_cost * 2 / 5;
        
        if( skillSuccess( ch, THIS_SKILL ))
        {
            STUN_USER_MAX;
            begin_singing(ch, ch, SKILL_BERSERK);
            return;
        }
        else send_to_char("You try, but can't work yourself into a &08blood&00 rage.\r\n", ch);

        STUN_USER_MIN;
        return;
    }
    
    if( !IS_NPC(ch) && GET_MOVE(ch) < berserk_cost ){
        send_to_char("You're too tired to enter the &08blood rage&00.\r\n", ch);
        return;
    }/* if */

    if (!IS_NPC(ch))
        GET_MOVE(ch) -= berserk_cost;

    if( skillSuccess( ch, THIS_SKILL )){
        struct affected_type af;

        send_to_char("You feel &08enraged!&00\r\n", ch);
        act("$n has gone berserk!", TRUE, ch, NULL, NULL, TO_ROOM);
        advanceSkill( ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING, DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS );

        af.type      = THIS_SKILL;
        af.duration  = 2 TICKS;
        af.modifier  = GET_LEVEL(ch) * 2;
        af.location  = APPLY_HIT;
        af.bitvector = AFF_BERSERK;

        affect_to_char(ch, &af);
        
        if(IS_DRAGONSLAYER(ch) && GET_ADVANCE_LEVEL(ch) >= FIRST_ADVANCE_SKILL)
            add_affect(ch, ch, SKILL_ENHANCED_DAMAGE, GET_LEVEL(ch), APPLY_DAMROLL, number(2, 8), 2 TICKS, FALSE, FALSE, FALSE, FALSE, FALSE);

    }/* if */
    else {
        send_to_char("You try, but can't work yourself into a &08blood&00 rage.\r\n", ch);
    }

    STUN_USER_MIN;

}/* do_sneak */

