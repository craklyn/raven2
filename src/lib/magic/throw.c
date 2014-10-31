/*
**++
**  RCSID:     $Id: throw.c,v 1.6 2001/08/04 06:59:06 raven Exp $
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
**      Digger from RavenMUD (Hastily thrown together template :)
**        Fleee from RavenMUD
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
**  $Log: throw.c,v $
**  Revision 1.6  2001/08/04 06:59:06  raven
**  timed scrolls
**
**  Revision 1.5  2001/04/18 23:26:56  raven
**  immortals can throw anything
**
**  Revision 1.4  2001/02/12 13:14:23  raven
**  scripting
**
**  Revision 1.2  2000/10/10 13:47:04  raven
**
**  Transitioned over to the new include structures.
**
**  Revision 1.1.1.1  2000/10/10 04:15:17  raven
**  RavenMUD 2.0
**
**  Revision 1.12  2000/04/05 23:18:49  mortius
**  FIxed the problem in throw with the peacerooms, can't throw in or out
**  of them.  Can only throw missiles or weapons.
**
**  Revision 1.11  1997/12/07 15:54:27  vex
**  Cleanup crew came through.
**
**  Revision 1.10  1997/09/26 05:32:30  vex
**  I fixed a problem with a thrown object being given to a victim thats just
**  been killed.
**  /
**
**  Revision 1.9  1997/09/18 12:52:36  vex
**  world was declared above the inclusion of structs.h, which caused a compiler
**  error after I switched everything to typdefs.
**
**  Revision 1.8  1997/09/18 11:07:54  vex
**  Replaced all obj_data, room_data, mob_special_data, char_data,
**  descriptor_data structs with appropriate typedef.
**
**  Revision 1.7  1997/01/03 12:32:45  digger
**  Renamed several of the functions from skills.c and added skill
**  avoidance to fist and hamstring. Vex has put in MAJOR changes
**  to the summoning code and many checks for Book of Blood signatures
**  were added.
**
**  Revision 1.6  1996/02/21 12:33:15  digger
**  Added the IF_UNLEARNED_SKILL check.
**
** Revision 1.5  1996/01/03  23:07:19  digger
** Checkpointing.
**
** Revision 1.4  1995/10/20  11:52:54  digger
** Began mods to FIX this skill.
**
** Revision 1.3  1995/07/26  20:51:26  digger
** Removed the unused range variable.
**
**  Revision 1.2  1994/12/19 22:19:53  moon
**  Added check so you couldn't throw stuff at yourself.
**
**
*/


/*
** STANDARD U*IX INCLUDES
*/
#include "general/conf.h"
#include "general/sysdep.h"

#define THIS_SKILL                SKILL_THROW

#define SKILL_ADVANCES_WITH_USE   TRUE
#define SKILL_ADVANCE_STRING      "You've become notably more proficient."
#define SKILL_MAX_LEARN           90
#define DEX_AFFECTS               TRUE
#define INT_AFFECTS               FALSE
#define WIS_AFFECTS               FALSE

#define STUN_MIN                  1
#define STUN_MAX                  3
#define MOVE_PER_THROW            5

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
#include "magic/missile.h"
#include "specials/special.h"
#include "actions/fight.h"
#include "actions/act.h"          /* ACMDs located within the act*.c files */

/*
** EXTERNAL DEFINITIONS OF SKILLS/SPELLS CALLED FROM THIS FILE
**
**
*/


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      SKILL_NAME - SKILL_THROW
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
**      Skill for throwing all sorts of different stuff. 
**
*/
ACMD(do_throw)
{
  static const char *throwDirs[] = { "north", "east",
                               "south", "west",
                               "up", "down", "\n"};
 
  char dir[255], target[255], weapon[255], temp[255];
  int dir_num = 0;
  char buf[MAX_INPUT_LENGTH];
  CHAR_DATA *victim = 0;
  OBJ_DATA  *ammo   = 0;
  bool fatality = FALSE;
  int target_room = 0, can_throw = FALSE;

  IF_UNLEARNED_SKILL( "You wouldn't know where to begin.\r\n" );

  half_chop (argument, weapon, temp);

  two_arguments(temp, target, dir);

  if( !*weapon || !*target )
  {
    sendChar( ch, "Usage : throw <item> <target> [direction] \n\r" );
    return;
  }

  if( !(ammo = get_obj_in_list_vis(ch, weapon, ch->carrying)))
  {
    sendChar( ch, "What did you want to throw?\n\r" );
    return;
  }

  if( GET_OBJ_WEIGHT(ammo) > str_app[STRENGTH_APPLY_INDEX(ch)].wield_w &&
      GET_LEVEL(ch) <= LVL_IMMORT)
  {
    sendChar(ch, "You're not strong enough to throw that.\r\n");
    return;
  }

  if( !*dir && !(victim = get_char_room_vis( ch, target )))
  {
    sendChar( ch, "Throw something at what?\n\r" );
    return;
  }

  if(*dir && ( dir_num = search_block( dir, throwDirs, FALSE )) < 0)
  {
    sendChar( ch, "What direction???\n\r" );
    return;
  }

  if( !victim && !(victim = find_vict_dir( ch, target, DEFAULT_LONG_RANGE, dir_num )))
  {
    sendChar( ch, "You cannot see that in range.\n\r" );
    return;
  }

  if (GET_OBJ_TYPE(ammo)  == ITEM_MISSILE)      can_throw = TRUE;
  else
  if (GET_OBJ_TYPE(ammo)  == ITEM_WEAPON)       can_throw = TRUE;

  // gods can throw anything
  if (GET_LEVEL(ch) >= LVL_IMMORT) can_throw = TRUE;

  if (can_throw == FALSE) {
      send_to_char("You can't throw that.\r\n", ch);
      return;
  }

  if (ROOM_FLAGGED(IN_ROOM(victim), ROOM_PEACEFUL) ||
      ROOM_FLAGGED(IN_ROOM(ch), ROOM_PEACEFUL)) {
      send_to_char("A flash of white light fills the room, dispelling your violent deed!\r\n", ch);
      return;
  }




  IF_CH_CANT_BE_VICTIM("Why would you want to throw things at yourself?\n\r");
  IF_CANT_HURT_VICTIM;
  
    if (GET_MOVE(ch) < MOVE_PER_THROW )
	{
		send_to_char("You are too tired to throw straight!\r\n", ch);
		return;
    }

  if( skillSuccess( ch, THIS_SKILL ))
  {
    if( ch->in_room == victim->in_room )
    {
      act("You throw $p towards $N.",  FALSE, ch, ammo, victim, TO_CHAR);
      act("$n throws $p towards you.", FALSE, ch, ammo, victim, TO_VICT);
      act("$n throws $p towards $N.",  FALSE, ch, ammo, victim, TO_NOTVICT);
      set_fighting(ch, victim);
    }

    else
    {
      sprintf(buf, "You throw $p %swards towards $N.", throwDirs[dir_num]);
      act(buf, FALSE, ch, ammo, victim, TO_CHAR);
      sprintf(buf, "$n throws $p %swards.", throwDirs[dir_num]);
      act(buf, FALSE, ch, ammo, victim, TO_ROOM);
    }

    target_room = victim->in_room;
    advanceSkill( ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING, DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS );
    fatality = gen_missile(ch, victim, ammo, 0, dir_num, MISSILE_TYPE_THROW);

    obj_from_char( ammo );
    if(!fatality && number(1,100) > 50 && CAN_CARRY_OBJ( ch, ammo ))
      obj_to_char(ammo, victim);
    else
      obj_to_room(ammo, target_room);

  }/* if */

  else
    sendChar( ch, "You fumble with %s, trying to figure out which way to throw it.\n\r", ammo->short_description );

  if (GET_LEVEL(ch) <= LVL_IMMORT) STUN_USER_MAX;
GET_MOVE(ch) -= MOVE_PER_THROW;
GET_MOVE(ch) = MAX(0, GET_MOVE(ch));

}/* do_SKILL */

