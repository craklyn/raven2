/*
**++
**  RCSID:     $Id: rescue.c,v 1.4 2002/01/18 04:36:51 raven Exp $
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
**  $Log: rescue.c,v $
**  Revision 1.4  2002/01/18 04:36:51  raven
**  bugtown
**
**  Revision 1.3  2002/01/14 12:56:42  raven
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

#define THIS_SKILL                SKILL_RESCUE

#define SKILL_ADVANCES_WITH_USE   TRUE
#define SKILL_ADVANCE_STRING      "You've become notably more proficient."
#define SKILL_MAX_LEARN           85
#define DEX_AFFECTS               TRUE
#define INT_AFFECTS               TRUE
#define WIS_AFFECTS               FALSE

#define STUN_MIN                  2
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
#include "magic/rescue.h"
#include "actions/fight.h"

/*
** EXTERNAL DEFINITIONS OF SKILLS/SPELLS CALLED FROM THIS FILE
**
**
*/

void char_rescue_char(CharData *ch, CharData *victim)
{
    CharData *tmp_ch;

    for( tmp_ch = world[ch->in_room].people; tmp_ch && (FIGHTING(tmp_ch) != victim); tmp_ch = tmp_ch->next_in_room);

    if( !tmp_ch ){
        act("But nobody is fighting $M!", FALSE, ch, 0, victim, TO_CHAR);
        return;
    }/* if */
    if( skillSuccess( ch, THIS_SKILL )){
        advanceSkill( ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING, DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS );

        if( FIGHTING(victim) == tmp_ch)
            stop_fighting(victim);

        if( FIGHTING(tmp_ch))
            stop_fighting(tmp_ch);

        if( FIGHTING(ch) )
            stop_fighting(ch);

        set_fighting(ch, tmp_ch);
        set_fighting(tmp_ch, ch);

        send_to_char("Banzai!  To the rescue...\r\n", ch);
        act("You are rescued by $N, you are confused!", FALSE, victim, 0, ch, TO_CHAR);
        act("$n heroically rescues $N!", FALSE, ch, 0, victim, TO_NOTVICT);

        STUN_VICTIM_MAX;

    }/* if */

    else
        send_to_char("You fail the rescue!\r\n", ch);

    STUN_USER_MIN;
}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      do_rescue -
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
ACMD(do_rescue)
{
    char   arg[100];
    CharData *victim;

    one_argument( argument, arg );

    IF_UNLEARNED_SKILL( "You wouldn't know where to begin.\r\n" );
    IF_CH_CANT_SEE_VICTIM( "Who do you want to rescue?\r\n" );
    IF_CH_CANT_BE_VICTIM( "What about fleeing instead?\r\n" );

    /*
    ** The following special tests were written because this section
    ** of the code doesn't really follow the standard ch/victim interface
    ** since a rescued victim indirectly places the ch in combat with
    ** a new victim.
    */
    if( FIGHTING(ch) == victim ){
        send_to_char("How can you rescue someone you are trying to kill?\r\n", ch);
        return;
    }/* if */

    char_rescue_char(ch, victim);
}/* do_rescue */

