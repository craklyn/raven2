
/*
 **++
 **  RCSID:     $Id: palm.c,v 1.8 2003/07/03 10:54:18 raven Exp $
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
 **      Fleee(Elendil)
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
 **  $Log: palm.c,v $
 **  Revision 1.8  2003/07/03 10:54:18  raven
 **  changes everywhere
 **
 **  Revision 1.7  2002/11/19 13:11:26  raven
 **  auto flag game scoring
 **
 **  Revision 1.6  2001/06/17 07:06:47  raven
 **  Rogue Team stuff
 **
 **  Revision 1.2  2000/10/10 13:47:04  raven
 **
 **  Transitioned over to the new include structures.
 **
 **  Revision 1.1.1.1  2000/10/10 04:15:17  raven
 **  RavenMUD 2.0
 **
 **  Revision 1.6  1997/09/26 05:32:30  vex
 **  Code clean up.
 **
 **  Revision 1.5  1997/09/18 12:52:36  vex
 **  world was declared above the inclusion of structs.h, which caused a compiler
 **  error after I switched everything to typdefs.
 **
 **  Revision 1.4  1997/09/18 11:01:05  vex
 **  Replaced all obj_data, room_data, mob_special_data, char_data,
 **  descriptor_data structs with appropriate typedef.
 **
 **  Revision 1.3  1997/01/03 12:32:45  digger
 **  Renamed several of the functions from skills.c and added skill
 **  avoidance to fist and hamstring. Vex has put in MAJOR changes
 **  to the summoning code and many checks for Book of Blood signatures
 **  were added.
 **
 **  Revision 1.2  1996/02/21 12:34:30  digger
 **  Added the IF_UNLEARNED_SKILL check.
 **
 ** Revision 1.1  1995/01/04  20:46:24  jbp
 ** Initial revision
 **
 * Revision 1.1  1994/12/16  14:23:52  jbp
 * Initial revision
 *
 **
 */


/*
 ** STANDARD U*IX INCLUDES
 */

#define THIS_SKILL                SKILL_PALM

#define SKILL_ADVANCES_WITH_USE   FALSE
#define SKILL_ADVANCE_STRING      "You've become notably more sneaky."
#define SKILL_MAX_LEARN           90
#define DEX_AFFECTS               TRUE
#define INT_AFFECTS               FALSE
#define WIS_AFFECTS               FALSE

#define STUN_MIN                  2
#define STUN_MAX                  4

/*
 ** MUD SPECIFIC INCLUDES
 */
#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/class.h"
#include "general/comm.h"
#include "general/handler.h"
#include "actions/interpreter.h"
#include "magic/skills.h"
#include "magic/spells.h"
#include "util/utils.h"
#include "specials/flag_game.h"

/*
 ** EXTERNAL DEFINITIONS OF SKILLS/SPELLS CALLED FROM THIS FILE
 **
 **
 */

int can_take_obj(CharData *ch, ObjData *obj);
void quest_echo(char *msg);
void get_check_money(CharData *ch, ObjData *obj);


/*
 **++
 **  FUNCTIONAL DESCRIPTION:
 **
 **      SKILL_NAME - palm
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
 **      use to pick up an item without everyone else seeing
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

#define MOVE_PER_PALM 15

ACMD(do_palm)
{
    char arg[100];
    ObjData *obj;
    CharData *temp_ch;
    static char *black = "&05(black)&00";
    static char *gold = "&03(gold)&00";
    static char *rogue = "&08(rogue team)&00";
    static char *none = "&10(no team)&00";
    char *lColor = (PRF_FLAGGED(ch, PRF_GOLD_TEAM) ? gold : black);
    if (!PRF_FLAGGED(ch, PRF_GOLD_TEAM) &&
            !PRF_FLAGGED(ch, PRF_BLACK_TEAM)) lColor = none;
    if (PRF_FLAGGED(ch, PRF_ROGUE_TEAM)) lColor = rogue;
    IF_UNLEARNED_SKILL("You wouldn't know where to begin.\r\n");


    one_argument(argument, arg);

    if (!*arg)
    {
        send_to_char("Pick up what?\r\n", ch);
        return;
    }
    if (!(obj = get_obj_in_list_vis(ch, arg, world[ch->in_room].contents)))
    {
        send_to_char("Pick up what?\r\n", ch);
        return;
    }
    if (GET_MOVE(ch) < MOVE_PER_PALM)
    {
        send_to_char("You are too tired to concentrate properly on this task.\r\n", ch);
        return;
    }
    if (!can_take_obj(ch, obj))
        return;

    if ((GET_OBJ_VNUM(obj) == GOLD_TEAM_STANDARD) && (GET_LEVEL(ch) <= 51))
    {
        if (PRF_FLAGGED(ch, PRF_GOLD_TEAM))
        {
            sprintf(buf, "&08FLAG GAME:&00&11 %s&00 %s &11has returned the&00 &10GOLD&00&11 Flag!&00\r\n", GET_NAME(ch), lColor);
            obj_from_room(obj);
            obj_to_room(obj, real_room(20095));
            quest_echo(buf);
            flag_player_return(ch);
        }
        else
        {
            sprintf(buf, "&08FLAG GAME:&00 &08%s&00 %s &08has taken the&00 &10GOLD&00 &08Flag!&00\r\n", GET_NAME(ch), lColor);
            quest_echo(buf);
        }
    }
    if ((GET_OBJ_VNUM(obj) == BLACK_TEAM_STANDARD) && (GET_LEVEL(ch) <= 51))
    {
        if (PRF_FLAGGED(ch, PRF_BLACK_TEAM))
        {
            sprintf(buf, "&08FLAG GAME:&00 &11%s&00 %s &11has returned the&00 &07BLACK&00 &11Flag!&00\r\n", GET_NAME(ch), lColor);
            obj_from_room(obj);
            obj_to_room(obj, real_room(20195));
            quest_echo(buf);
            flag_player_return(ch);
        }
        else
        {
            sprintf(buf, "&08FLAG GAME:&00 &08%s&00 %s &08has taken the&00 &07BLACK&00 &08Flag!&00\r\n", GET_NAME(ch), lColor);
            quest_echo(buf);
        }
    }

    if( skillSuccess( ch, THIS_SKILL )){
        advanceSkill( ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING, DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS );

		obj_from_room(obj);
		obj_to_char(obj, ch);
		
		act("You sneakily grab $p while no-one is looking.", FALSE, ch, obj, 0, TO_CHAR);
		for (temp_ch = world[ch->in_room].people; temp_ch; temp_ch = temp_ch->next_in_room)
		{
			if (!IS_NPC(temp_ch) && GET_LEVEL(temp_ch) > MAX_MORTAL && GET_LEVEL(temp_ch) > GET_LEVEL(ch))
				act("$n sneakily grabs $p while no-one ($e thinks) is looking.", FALSE, ch, obj, temp_ch, TO_VICT);
			else if(IS_TRICKSTER(ch) && percentSuccess(40))
			{
				act("$N notices something is not right.  $e is growing very paranoid!", FALSE, ch, obj, temp_ch, TO_CHAR);
				add_affect(ch, temp_ch, SPELL_PARANOIA, GET_LEVEL(ch), 0, 0, 1 TICKS, FALSE, FALSE, FALSE, FALSE, FALSE);
			}
		}
		get_check_money(ch, obj);
		GET_MOVE(ch) -= MOVE_PER_PALM;
		GET_MOVE(ch) = MAX(0, GET_MOVE(ch));
    }/* if */
    else
    {
        obj_from_room(obj);
        obj_to_char(obj, ch);
        act("You sneakily grab $p while no-one is looking.", FALSE, ch, obj, 0, TO_CHAR);
        act("$n gets $p.", FALSE, ch, obj, 0, TO_ROOM);
        get_check_money(ch, obj);
    }/* else */
}/* do_palm */

