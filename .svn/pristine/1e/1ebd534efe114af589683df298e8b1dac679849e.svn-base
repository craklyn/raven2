/*
**++
**  RCSID:     $Id: knock.c,v 1.3 2001/03/13 02:16:57 raven Exp $
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
**  $Log: knock.c,v $
**  Revision 1.3  2001/03/13 02:16:57  raven
**  end_fight() uses
**
**  Revision 1.2  2000/10/10 13:47:04  raven
**
**  Transitioned over to the new include structures.
**
**  Revision 1.1.1.1  2000/10/10 04:15:17  raven
**  RavenMUD 2.0
**
**  Revision 1.20  2000/04/02 22:06:06  mortius
**  Added a check so that if a player has been killed in the arena with
**  fists that they don't get KO'd in either.
**
**  Revision 1.19  1998/10/05 12:23:16  vex
**  Added checks for feign.
**
**  Revision 1.18  1998/08/13 12:22:09  digger
**  Fixed the lack of a delay on missed knocks.
**
**  Revision 1.17  1998/01/29 03:38:48  digger
**  Removed all references to BLOOD_ENABLED
**
**  Revision 1.16  1997/12/07 15:54:27  vex
**  Cleanup crew came through.
**
**  Revision 1.15  1997/10/19 09:21:07  vex
**  Added art of the monkey/wind/flower.
**
**  Revision 1.14  1997/09/26 05:32:30  vex
**  Code clean up.
**
**  Revision 1.13  1997/09/18 12:52:36  vex
**  world was declared above the inclusion of structs.h, which caused a compiler
**  error after I switched everything to typdefs.
**
**  Revision 1.12  1997/09/18 10:58:21  vex
**  Replaced all obj_data, room_data, mob_special_data, char_data,
**  descriptor_data structs with appropriate typedef.
**
**  Revision 1.11  1997/05/08 02:13:16  liam
**  Prevented knock working on no_bash mobs, removed BLOODCHK.
**  Vex.
**
**  Revision 1.10  1997/01/28 15:53:34  liam
**  Vex - if a mob is !bash it can't be ko'd
**
**  Revision 1.9  1997/01/09 20:41:40  digger
**  Changed the call from dam() to set_fighting().
**
**  Revision 1.8  1997/01/03 12:32:45  digger
**  Renamed several of the functions from skills.c and added skill
**  avoidance to fist and hamstring. Vex has put in MAJOR changes
**  to the summoning code and many checks for Book of Blood signatures
**  were added.
**
**  Revision 1.7  1996/09/17 17:59:21  digger
**  *** empty log message ***
**
**  Revision 1.6  1996/02/21 12:34:30  digger
**  Added the IF_UNLEARNED_SKILL check.
**
** Revision 1.5  1996/01/03  23:07:19  digger
** Checkpointing.
**
** Revision 1.4  1995/10/20  11:52:54  digger
** Added a check for the PEACEFUL room flag.
**
** Revision 1.3  1995/08/21  13:40:16  digger
** Reduced the STUN_MAX from 8 to 6 rounds.
**
** Revision 1.2  1995/07/05  23:19:48  digger
** Modified the STUN durations.
**
** Revision 1.1  94/12/16  14:23:52  jbp
** Initial revision
** 
**
*/


/*
** STANDARD U*IX INCLUDES
*/
#include "general/conf.h"
#include "general/sysdep.h"

#define THIS_SKILL                SKILL_KNOCK

#define SKILL_ADVANCES_WITH_USE   TRUE
#define SKILL_ADVANCE_STRING      "Your knuckles momentarily glow with raw Shou-Lin power."
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
#include "magic/knock.h"
#include "specials/special.h"
#include "actions/fight.h"

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      do_knock -
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
ACMD(do_knock)
{
  char   arg[100];
  CharData *victim;

  one_argument( argument, arg );

  IF_UNLEARNED_SKILL( "You wouldn't know where to begin.\r\n" );
  IF_CH_CANT_SEE_VICTIM( "Knockout who?\r\n" );
  IF_CH_CANT_BE_VICTIM( "You punch yourself in the eye. OUCH!" );
  IF_ROOM_IS_PEACEFUL( "A sense of calm overwhelms you.\r\n" );
  IF_CH_IS_WIELDING( "Impossible! You are ARMED!\r\n" );
  IF_CANT_HURT_VICTIM;

  advanceSkill(ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING, DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS );
  knock_internal(ch, victim);

}/* do_knock */

void knock_internal(CharData *ch, CharData *victim) {

    //Mortius : Check to make sure the player is still in the room before you KO them
    if(ch->in_room != victim->in_room)  return;
    
    
    // Vex January 29 If you can't bash a mob, you can't knock it out either.
    if(IS_NPC(victim) && IS_SET_AR(MOB_FLAGS(victim), MOB_NOBASH)) {
        act("$N ignores your pitiful attempt to knock $M out.\r\n", FALSE, ch, 0, victim, TO_CHAR);
        STUN_USER_RANGE;
        return;
    }

    if(victimAvoidsSkill(ch, victim, THIS_SKILL) || raceSkillAvoid(ch, victim, THIS_SKILL)) {
        if(victimIsAngry(30))
            set_fighting(victim, ch);
        STUN_USER_RANGE;
        return;
    }

    if(skillSuccess(ch, THIS_SKILL ))
    {
        int knock_damage = (number( 1, (GET_LEVEL(ch)>>1)) * 4);
        
        // If hitpoints of ch is zero for some odd reason, abort.
        if(GET_HIT(ch) == 0)
            return;

        int knocked_out  = (number( 1, ((GET_HIT(victim)/GET_HIT(ch))+6)) == 1 ? 1 : 0);
        int stunned      = (number( 1, ((GET_HIT(victim)/GET_HIT(ch))+2)) == 1 ? 1 : 0);

        damage(ch, victim, knock_damage, THIS_SKILL );
        
        if(knocked_out) {
            struct affected_type af;
            
            act("You knocked $M out!", FALSE, ch, NULL, victim, TO_CHAR);
            act("$N goes down like a sack of potatoes from $n's knockout punch.", TRUE, ch, NULL, victim, TO_NOTVICT);
            act("$n knocks you out!", TRUE, ch, NULL, victim, TO_VICT);
            end_fight(victim);
            
            af.type      = SPELL_SLEEP;
            af.modifier  = 0;
            af.duration  = number(1, 2) TICKS;
            af.location  = APPLY_NONE;
            af.bitvector = AFF_SLEEP;
            affect_join(victim, &af, FALSE, FALSE, FALSE, FALSE);
            
            GET_POS(victim) = POS_SLEEPING;
            STUN_VICTIM_RANGE;
        }/* if KO */
        else if(stunned)
        {
            act( "You knock $M on $S butt!", FALSE, ch, NULL, victim, TO_CHAR );
            act( "$N lands on $S butt from $n's knockout punch.", TRUE, ch, NULL, victim, TO_NOTVICT );
            act( "$n sends you to the ground with a knockout punch!", TRUE, ch, NULL, victim, TO_VICT );
            GET_POS(victim) = POS_SITTING;
            STUN_VICTIM_MAX;
        }

        STUN_USER_MIN;
    } /* skill success */
    else
    {
        STUN_USER_RANGE;
        damage( ch, victim, 0, THIS_SKILL );
    }

    if(HAS_FEIGNED(ch)) {
        HAS_FEIGNED(ch) = 0;
        STUN_USER_MAX;
    }

}
