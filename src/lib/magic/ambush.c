
/*
**++
**  RCSID:     $Id: ambush.c,v 1.3 2001/08/19 15:54:23 raven Exp $
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
**      Imhotep from RavenMUD
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
**  $Log: ambush.c,v $
**  Revision 1.3  2001/08/19 15:54:23  raven
**  state of the world
**
**  Revision 1.2  2001/08/16 00:19:10  raven
**  assassin branch merged
**
**  Revision 1.1.2.1  2001/08/15 12:19:46  raven
**  assassin stuff
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
**  Revision 1.8  1997/09/26 05:32:30  vex
**  Code clean up.
**
**  Revision 1.7  1997/09/18 12:52:36  vex
**  world was declared above the inclusion of structs.h, which caused a compiler
**  error after I switched everything to typdefs.
**
**  Revision 1.6  1997/09/18 10:57:26  vex
**  Replaced all obj_data, room_data, mob_special_data, char_data,
**  descriptor_data structs with appropriate typedef.
**
**  Revision 1.5  1997/01/03 12:32:45  digger
**  Renamed several of the functions from skills.c and added skill
**  avoidance to fist and hamstring. Vex has put in MAJOR changes
**  to the summoning code and many checks for Book of Blood signatures
**  were added.
**
**  Revision 1.4  1996/02/21 12:32:03  digger
**  Added the IF_UNLEARNED_SKILL check.
**
** Revision 1.3  1995/10/20  11:52:54  digger
** Added a move cost to the attempt at hiding.
**
** Revision 1.2  1995/07/26  20:45:49  digger
** Removed the unused victim variable.
**
**  Revision 1.1  1994/12/07 21:02:02  digger
**  Initial revision
**
**
*/


/*
** STANDARD U*IX INCLUDES
*/
#include "general/conf.h"
#include "general/sysdep.h"

#define THIS_SKILL                SKILL_AMBUSH

#define SKILL_ADVANCE_STRING      "You become more skilled at lying in wait."
#define SKILL_MAX_LEARN           90
#define DEX_AFFECTS               TRUE
#define INT_AFFECTS               FALSE
#define WIS_AFFECTS               FALSE

#define STUN_MIN                  1
#define STUN_MAX                  1

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
ACMD(do_ambush)
{
    char   arg[100];

    int hide_cost = 25;

    IF_UNLEARNED_SKILL( "You wouldn't know where to begin.\r\n" );

    if (ZONE_FLAGGED(world[ch->in_room].zone, ZONE_SLEEPTAG)) {
        send_to_char("You seem unable to wait in ambush here.\r\n", ch);
        return;
    }

    if( GET_MOVE(ch) < hide_cost ){
        send_to_char("You're too exhausted to find a good spot.\r\n", ch);
        return;
    }/* if */

    one_argument( argument, arg );

    if (arg[0] == '\0') {
      send_to_char("Who do you wish to wait for?\r\n", ch);
      return;
    }

    GET_MOVE(ch) -= hide_cost;

    send_to_char("You find a spot to wait for your victim.\r\n", ch);

    if( skillSuccess( ch, THIS_SKILL )){
        advanceSkill( ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING, DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS );
        ch->ambushing = strdup(arg);
    }/* if */
    STUN_USER_RANGE;

}/* do_hide */

int ambushVictim(CharData *ch, CharData *victim)
{
	if( STUNNED(ch) )
		return 0;


  if (ch->ambushing && isname(ch->ambushing, victim->player.name)) {
    // peace rooms are treated all special
    if( IS_SET_AR( ROOM_FLAGS( ch->in_room ), ROOM_PEACEFUL )) {
      act("You leap out at $N and say, 'Boo!'", TRUE, ch, 0, victim, TO_CHAR);
      act("$n leaps out at $N and says, 'Boo!'", TRUE, ch, 0, victim, TO_ROOM);
      act("$n leaps out at you and says, 'Boo!'", TRUE, ch, 0, victim,
          TO_NOTVICT);
      free(ch->ambushing);
      ch->ambushing = NULL;
      return 0;
    }

    // successful ambush, try the stab
    return 1;
  }
  return 0;
}

