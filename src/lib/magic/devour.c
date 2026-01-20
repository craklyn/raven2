/*
**++
**  RCSID:     $Id: devour.c,v 1.1 2002/01/14 12:56:42 raven Exp $
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
**  $Log: devour.c,v $
**  Revision 1.1  2002/01/14 12:56:42  raven
**  knight/deathknight changes complete
**
**  Revision 1.2  2000/10/10 13:47:04  raven
**
**  Transitioned over to the new include structures.
**
**  Revision 1.1.1.1  2000/10/10 04:15:17  raven
**  RavenMUD 2.0
**
**  Revision 1.9  1998/01/29 03:38:48  digger
**  Removed all references to BLOOD_ENABLED
**
**  Revision 1.8  1997/12/07 15:54:27  vex
**  Cleanup crew came through.
**
**  Revision 1.7  1997/09/26 05:32:30  vex
**  Code clean up.
**
**  Revision 1.6  1997/09/18 12:52:36  vex
**  world was declared above the inclusion of structs.h, which caused a compiler
**  error after I switched everything to typdefs.
**
**  Revision 1.5  1997/09/18 11:02:02  vex
**  Replaced all obj_data, room_data, mob_special_data, char_data,
**  descriptor_data structs with appropriate typedef.
**
**  Revision 1.4  1997/05/08 02:10:50  liam
**  Removed BLOODCHK.
**  Vex.
**
**  Revision 1.3  1997/01/03 12:32:45  digger
**  Renamed several of the functions from skills.c and added skill
**  avoidance to fist and hamstring. Vex has put in MAJOR changes
**  to the summoning code and many checks for Book of Blood signatures
**  were added.
**
**  Revision 1.2  1996/02/21 12:33:15  digger
**  Added the IF_UNLEARNED_SKILL check.
**
** Revision 1.1  1994/12/07  20:40:22  digger
** Initial revision
**
**
*/


/*
** STANDARD U*IX INCLUDES
*/
#include "general/conf.h"
#include "general/sysdep.h"

#define THIS_SKILL                SKILL_DEVOUR

#define SKILL_ADVANCES_WITH_USE   TRUE
#define SKILL_ADVANCE_STRING      "Your thirst for violence has grown."
#define SKILL_MAX_LEARN           90
#define DEX_AFFECTS               TRUE
#define INT_AFFECTS               TRUE
#define WIS_AFFECTS               FALSE

#define STUN_MIN                  0
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
#include "magic/devour.h"

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
ACMD(do_devour)
{
  char   arg[100];
  CharData *victim;
  int dam;

  int PERCENT_CHECK;  // Figures how hurt a target is.

  one_argument( argument, arg );

  IF_UNLEARNED_SKILL( "You wouldn't know where to begin.\r\n" );
  IF_CH_CANT_SEE_VICTIM( "Whose life do you want to devour?\r\n" );
  IF_CH_CANT_BE_VICTIM( "You cannot devour yourself.\r\n" );
  IF_ROOM_IS_PEACEFUL("You nibble delicately on their ear.\r\n");
  IF_CANT_HURT_VICTIM;
  IF_CANT_HIT_FLYING;

  PERCENT_CHECK = 100 * GET_HIT(victim)/( (GET_MAX_HIT(victim)>0)?GET_MAX_HIT(victim):1);

  if (skillSuccess(ch, THIS_SKILL)) {
    advanceSkill( ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING, DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS );

    //dam = calculateSimpleDamage(ch);
    if (IS_PVP(ch, victim) && !IS_NPC(victim)) dam = 3 * GET_LEVEL(ch); // Changing things up - Craklyn
    else dam =  3 * calculateSimpleDamage(ch);

    if (IS_NPC(ch))
      dam = MIN( 3 * calculateSimpleDamage(ch), 3 * GET_LEVEL(ch) );

    /* This is where things get ugly.  Because the damage done
     * depends on whether the victim WOULD die or not, we can't
     * simply do the damage and see if the victim is then dead. */

    /* Addendum:  I added percent check, so this skill doesn't decimate
     * casting classes in PvP.  It shouldn't make a big difference for
     * MOST mobs, anyway.  Craklyn */
		 
    if ((GET_HIT(victim) > dam - 10)  ||  (PERCENT_CHECK > 10)) {         
      dam /= 5;
      GET_HIT(ch) += number(4, 12);
      GET_MANA(ch) = MIN(GET_MANA(ch) + number(5, 11), GET_MAX_MANA(ch));
      STUN_USER_MAX;
    } 
    else {
      /* ... but, we also can't calculate if that damage really
       * is enough to kill the victim - they might have
       * sanctuary or a lot of AC or something else unexpected,
       * so this code here ensures they really, REALLY >REALLY<
       * do die. */
      GET_HIT(victim) = -1000;
      STUN_USER_MIN;
                        
      if(IS_NPC(victim)) GET_MANA(ch) += GET_MAX_MANA(victim) / 15;
      else GET_MANA(ch) += GET_MAX_MANA(victim) / 4;
      if(IS_NPC(victim)) GET_HIT(ch) += GET_MAX_HIT(victim) / 20;
      else GET_HIT(ch) += GET_MAX_HIT(victim) / 4;
    }
  } 
  else {
    WAIT_STATE(ch, SET_STUN(3));
    dam = 0;
  }

  /* regardless, we'll start fighting now */
  damage(ch, victim, dam, THIS_SKILL);

}/* do_devour */

