/*
**++
**  RCSID:     $Id: cower.c,v 1.2 2000/10/10 13:47:04 raven Exp $
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
**  $Log: cower.c,v $
**  Revision 1.2  2000/10/10 13:47:04  raven
**
**  Transitioned over to the new include structures.
**
**  Revision 1.1.1.1  2000/10/10 04:15:17  raven
**  RavenMUD 2.0
**
**  Revision 1.5  1997/09/26 05:32:30  vex
**  Code clean up.
**
**  Revision 1.4  1997/09/18 12:52:36  vex
**  world was declared above the inclusion of structs.h, which caused a compiler
**  error after I switched everything to typdefs.
**
**  Revision 1.3  1997/09/18 10:48:30  vex
**  Replaced all obj_data, room_data, mob_special_data, char_data,
**  descriptor_data structs with appropriate typedef.
**
**  Revision 1.2  1997/09/12 07:06:28  vex
**  Prevented butchering of pc corpses.
**
**  Revision 1.1  1997/06/25 11:00:40  Vex
**  Initial revision
**
**
*/


/*
** STANDARD U*IX INCLUDES
*/
#include "general/conf.h"
#include "general/sysdep.h"

#define THIS_SKILL                SKILL_COWER

#define SKILL_ADVANCES_WITH_USE   TRUE
#define SKILL_ADVANCE_STRING      "You feel more cowardly then ever!"
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
#include "magic/backstab.h"

/*
** EXTERNAL DEFINITIONS OF SKILLS/SPELLS CALLED FROM THIS FILE
**
**
*/


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      do_cower -
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
ACMD(do_cower)
{
    char   arg[100];
    CharData *tmp_ch, *victim;

    one_argument( argument, arg );

    IF_UNLEARNED_SKILL( "You cringe in terror! It doesn't help though.\r\n" );
    IF_CH_CANT_SEE_VICTIM( "Who do you want to cower behind?\r\n" );
    IF_CH_CANT_BE_VICTIM( "What about fleeing instead?\r\n" );

    /*
    ** The following special tests were written because this section
    ** of the code doesn't really follow the standard ch/victim interface
    ** since a cowering ch indirectly places the targett in combat with
    ** a new victim.
    */

    /* You have to be fighting someone */
    for( tmp_ch = world[ch->in_room].people; tmp_ch && (FIGHTING(tmp_ch) != ch); tmp_ch = tmp_ch->next_in_room);

    if( !tmp_ch ){
        act("But nobody is fighting you!", FALSE, ch, 0, victim, TO_CHAR);
        return;
    }/* if */

    /* Victim must be in your group. */
    if (IS_AFFECTED(ch, AFF_GROUP) && IS_AFFECTED(victim, AFF_GROUP) &&
       ((ch->master && ch->master == victim) || /* victim is group leader */
        (victim->master && victim->master == ch) || /* ch is group leader */
	(victim->master && ch->master && /* they have same group leader */
	 IS_AFFECTED(ch->master, AFF_GROUP) && victim->master == ch->master)))
    {
        if( skillSuccess( ch, THIS_SKILL )){
            advanceSkill( ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING, DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS );

            if(FIGHTING(ch) == tmp_ch) stop_fighting(ch);

            if(FIGHTING(tmp_ch)) stop_fighting(tmp_ch);

            if(FIGHTING(victim)) stop_fighting(victim);

            set_fighting(victim, tmp_ch);
            set_fighting(tmp_ch, victim);

            act("You cower behind $n!", FALSE, victim, 0, ch, TO_VICT);
            act("$N cowers behind you!", FALSE, victim, 0, ch, TO_CHAR);
            act("$n cowers behind $N!", FALSE, ch, 0, victim, TO_NOTVICT);

            if(IS_TRICKSTER(ch) && GET_ADVANCE_LEVEL(ch) >= 1 && percentSuccess(40)) {
                tmp_ch->flee_timer = 1;
                act("You seen an opening.  It's worth a shot...", FALSE, victim, 0, ch, TO_VICT);
                act("$n takes one last shot at $N!", FALSE, ch, 0, tmp_ch, TO_NOTVICT);
                do_backstab(ch, tmp_ch->player.name, 0, 0);
            }

            STUN_VICTIM_MIN;
        }/* skill success */
        else
            send_to_char("Ack! You can't get away!\r\n", ch);

    STUN_USER_RANGE;
    } /* if grouped */
    else  /* victim is not a member of ch's group */
	send_to_char("No use hiding behind them, they are not a member of your group!\r\n", ch);

}/* do_cower */

