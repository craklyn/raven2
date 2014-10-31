/*
**++
**  RCSID:     $Id: dust.c,v 1.5 2004/07/21 23:15:38 raven Exp $
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
**  $Log: dust.c,v $
**  Revision 1.5  2004/07/21 23:15:38  raven
**  current code state
**
**  Revision 1.4  2001/08/04 06:59:06  raven
**  timed scrolls
**
**  Revision 1.3  2001/06/17 16:25:06  raven
**  detection levels
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
**  Revision 1.11  1997/12/07 15:54:27  vex
**  Cleanup crew came through.
**
**  Revision 1.10  1997/10/19 09:21:07  vex
**  Added art of the monkey/wind/flower.
**
**  Revision 1.9  1997/09/26 05:32:30  vex
**  Code clean up.
**
**  Revision 1.8  1997/09/18 12:52:36  vex
**  world was declared above the inclusion of structs.h, which caused a compiler
**  error after I switched everything to typdefs.
**
**  Revision 1.7  1997/05/08 02:13:52  liam
**  Removed BLOODCHK
**  Vex.
**
**  Revision 1.6  1997/01/03 12:32:45  digger
**  Renamed several of the functions from skills.c and added skill
**  avoidance to fist and hamstring. Vex has put in MAJOR changes
**  to the summoning code and many checks for Book of Blood signatures
**  were added.
**
**  Revision 1.5  1996/09/17 17:59:21  digger
**  *** empty log message ***
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

#define THIS_SKILL                SKILL_DUST

#define SKILL_ADVANCES_WITH_USE   TRUE
#define SKILL_ADVANCE_STRING      "You've become notably more proficient."
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

int mag_savingthrow( CharData *ch, int type);

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      SKILL_NAME - dust
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
**      The skill enables the user to 'throw', as it were, dust/sand/dirt or
**      whatever material is available towards the opponent, in an attempt to
**      temporarily blind the victim.
**
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
ACMD(do_dust)
{
    char arg[100];
    CharData *victim;
    int sect;
    int chance;
    bool success;

    one_argument( argument, arg );

    IF_UNLEARNED_SKILL( "You start to clean the woodwork.\r\n" );
    IF_CH_CANT_SEE_VICTIM( "Blind who?\r\n" );
    IF_CH_CANT_BE_VICTIM( "If you want to blind yourself, try a sharp weapon.\r\n" );
    IF_ROOM_IS_PEACEFUL( "You brush lint from their lapels.\r\n" );
    IF_CANT_HURT_VICTIM;

    if(!AWAKE(victim)){
        act("$N's eyes are shut right now.", FALSE, ch, 0, victim, TO_CHAR);
        return;
    }

    sect = world[ch->in_room].sector_type;
    if ( sect == SECT_WATER_SWIM || sect == SECT_WATER_NOSWIM ||
            sect == SECT_UNDERWATER || sect == SECT_FLYING ) {
        act("You can't find anything worth throwing at $N.", FALSE, ch, 0, victim, TO_CHAR);
        return;
    }

    if( artMonkey(ch, victim, THIS_SKILL ))
    {
        STUN_USER_MIN;
        return;
    }
    
    chance = 15;
    if(GET_LEVEL(victim) < LVL_IMMORT) chance  += 15;
    if(IS_BRIGAND(ch) && affected_by_spell(ch, SKILL_FLASHBANG))
        chance += 15;
    
    success = percentSuccess(chance);

    if( skillSuccess( ch, THIS_SKILL ) ){
        advanceSkill( ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING, DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS );
        
        // This is a mess.  It says that if it's a mortal enemy it will land *at least*
        // 30% of the time, if it's immortal it lands 15% of the time.
        if(!mag_savingthrow(victim, SAVING_PARA) || success)
        {
            act("You throw dust in $N's eyes.", FALSE, ch, 0, victim, TO_CHAR);
            act("$n throws dust in your eyes.", FALSE, ch, 0, victim, TO_VICT);
            act("$n throws dust in $N's eyes.", FALSE, ch, 0, victim, TO_NOTVICT);

            affect_from_char(victim, THIS_SKILL);
            affect_from_char(victim, THIS_SKILL);
            add_affect(ch, victim, THIS_SKILL, 0, APPLY_NONE, 0, 2 TICKS, AFF_BLIND, FALSE, FALSE, FALSE, FALSE);
            if(IS_BRIGAND(ch) && GET_ADVANCE_LEVEL(ch) >= 5)
                add_affect(ch, victim, THIS_SKILL, 0, APPLY_HITROLL, -number(1, 3), 3 TICKS, AFF_BLIND, FALSE, FALSE, FALSE, FALSE);
        }
        else {
            act("You narrowly miss throwing dust into $N's eyes.", FALSE, ch, 0, victim, TO_CHAR);
            act("You barely avoid dust thrown at you by $n.", FALSE, ch, 0, victim, TO_VICT);
            act("$N barely sidesteps dust thrown by $n.", FALSE, ch, 0, victim, TO_NOTVICT);
            damage(ch, victim, 0, THIS_SKILL);
        }
    }/* if */
    else {
        act("You throw dust at $N, who easily sidesteps it.", FALSE, ch, 0, victim, TO_CHAR);
        act("You easily avoid dust thrown at you by $n.", FALSE, ch, 0, victim, TO_VICT);
        act("$N easily sidesteps dust thrown by $n.", FALSE, ch, 0, victim, TO_NOTVICT);
        STUN_USER_MIN;
        damage(ch, victim, 0, THIS_SKILL);
    }/* else */

    STUN_USER_MIN;

}/* do_SKILL */

