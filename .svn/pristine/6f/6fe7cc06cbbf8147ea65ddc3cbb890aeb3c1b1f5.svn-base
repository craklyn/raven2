
/*
**++
**  RCSID:     $Id: camp.c,v 1.4 2003/11/14 04:04:40 raven Exp $
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
**  $Log: camp.c,v $
**  Revision 1.4  2003/11/14 04:04:40  raven
**  false trails
**
**  Revision 1.3  2003/09/29 02:11:18  raven
**  nifty new object type, fewer ways to Cheat the PK System
**
**  Revision 1.2  2003/09/02 01:37:55  raven
**  start of necromancing
**
**  Revision 1.1  2002/04/13 08:12:04  raven
**  demote command, wa/ra revisions
**
**
*/


/*
** STANDARD U*IX INCLUDES
*/
#include "general/conf.h"
#include "general/sysdep.h"

#define THIS_SKILL                SKILL_CAMP

#define SKILL_ADVANCE_STRING      "Lord Baden-Powell would be proud of you."
#define SKILL_MAX_LEARN           90
#define DEX_AFFECTS               FALSE
#define INT_AFFECTS               TRUE
#define WIS_AFFECTS               TRUE

#define STUN_MIN                  3
#define STUN_MAX                  3

#define CAMP_OBJECT               897

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
ACMD(do_camp)
{
    char   arg[100];

    int camp_mv_cost = 50;
    int camp_mp_cost = 30;

    IF_UNLEARNED_SKILL( "You fumble about with some sticks.\r\n" );

    if(IS_NPC(ch) && ch->master)
        return;

    if (ROOM_FLAGGED(ch->in_room, ROOM_MANA_REGEN) ||
            ROOM_FLAGGED(ch->in_room, ROOM_HEALTH_REGEN)) {
        send_to_char("You decide you already feel quite cosy here.\r\n", ch);
        return;
    }
    
    int sect = world[ch->in_room].sector_type;
    if(sect == SECT_WATER_SWIM || sect == SECT_WATER_NOSWIM ||
            sect == SECT_UNDERWATER || sect == SECT_FLYING) {
        act("There's no place to light a campfire.", FALSE, ch, 0, 0, TO_CHAR);
        return;
    }
    
    if( GET_MOVE(ch) < camp_mv_cost ){
        send_to_char("You're too exhausted to prepare a campsite.\r\n", ch);
        return;
    }/* if */

    if (GET_MANA(ch) < camp_mp_cost) {
        send_to_char("You don't have enough energy to prepare a campsite.\r\n",
                ch);
        return;
    }

    GET_MOVE(ch) -= camp_mv_cost;
    GET_MANA(ch) -= camp_mp_cost;

    one_argument( argument, arg );

    if( skillSuccess( ch, THIS_SKILL )){
        ObjData *campsite;

        advanceSkill( ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING, DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS );

        act("You prepare a campsite.", FALSE, ch, 0, 0, TO_CHAR);
        act("$n prepares a campsite.", FALSE, ch, 0, 0, TO_ROOM);

        campsite = read_perfect_object(CAMP_OBJECT, VIRTUAL);
        if (!campsite) {
            send_to_char("Oh no!  Oh no!  No object!  Oh no!\r\n", ch);
            mlog("Can't create campsite, object doesn't exist");
            return;
        }
        SET_BIT_AR(campsite->obj_flags.extra_flags, ITEM_TIMED);
        GET_OBJ_TIMER(campsite) = 6;
        obj_to_room(campsite, ch->in_room);
    } else {
        char *msg;

        switch (number(1,4)) {
            case 1:
                msg = "You try to make camp, but you can't start the fire!";
                break;
            case 2:
                msg = "You try to set up your tent, but it falls over!";
                break;
            case 3:
                msg = "You try to make camp, but run out of tea leaves!";
                break;
            default:
                msg = "You prepare a camp, but the results are unimpressive.";
                break;
        }
        send_to_char(msg, ch);
    }

    STUN_USER_RANGE;

}/* do_camp */

