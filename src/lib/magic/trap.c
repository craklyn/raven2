/*
**++
**  RCSID:     $Id: trap.c,v 1.4 2001/11/05 06:52:24 raven Exp $
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
**      Digger  from RavenMUD
**      Vex     from RavenMUD
**      Mortius from RavenMUD
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
**  $Log: trap.c,v $
**  Revision 1.4  2001/11/05 06:52:24  raven
**  fix disable/search
**
**  Revision 1.3  2001/08/16 00:19:10  raven
**  assassin branch merged
**
**  Revision 1.2.2.1  2001/08/15 12:19:46  raven
**  assassin stuff
**
**  Revision 1.2  2000/10/10 13:47:04  raven
**
**  Transitioned over to the new include structures.
**
**  Revision 1.1.1.1  2000/10/10 04:15:17  raven
**  RavenMUD 2.0
**
**  Revision 1.7  1997/09/26 05:32:30  vex
**  Code clean up.
**
**  Revision 1.6  1997/09/18 12:52:36  vex
**  world was declared above the inclusion of structs.h, which caused a compiler
**  error after I switched everything to typdefs.
**
**  Revision 1.5  1997/09/18 11:08:16  vex
**  Replaced all obj_data, room_data, mob_special_data, char_data,
**  descriptor_data structs with appropriate typedef.
**
**  Revision 1.4  1997/01/03 12:32:45  digger
**  Renamed several of the functions from skills.c and added skill
**  avoidance to fist and hamstring. Vex has put in MAJOR changes
**  to the summoning code and many checks for Book of Blood signatures
**  were added.
**
**  Revision 1.3  1996/02/21 12:33:15  digger
**  Added the IF_UNLEARNED_SKILL check.
**
** Revision 1.2  1996/01/03  23:07:19  digger
** Checkpointing.
**
** Revision 1.1  1995/01/04  20:43:13  Elendil
** Initial revision
**
**
*/


/*
** STANDARD U*IX INCLUDES
*/
#include "general/conf.h"
#include "general/sysdep.h"


#define SKILL_ADVANCES_WITH_USE   TRUE
#define SKILL_ADVANCE_STRING      "You've become notably more proficient."


/*                                Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  XX  XX  XX  XX  XX  XX  XX  XX  XX  XX */
static int max_skill_lvls[] =   { 00, 00, 90, 00, 90, 90, 00, 00, 00, 90, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 };
#define SKILL_MAX_LEARN           max_skill_lvls[ (int)GET_CLASS(ch) ]

#define DEX_AFFECTS               TRUE
#define INT_AFFECTS               TRUE
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
#include "magic/trap.h"
#include "magic/magic.h"
#include "actions/fight.h"


int check_room(CharData *ch, int to_room, ObjData *obj);



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      SKILL_TRAP -
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

#define THIS_SKILL SKILL_TRAP
ACMD(do_trap)
{
    char   arg[100];
    RoomDirectionData *wld = NULL;
    ObjData *obj;
    int dirn;

    IF_UNLEARNED_SKILL( "You wouldn't know where to begin.\r\n" );

    one_argument( argument, arg );

    if (arg[0] == '\0') {
      send_to_char("Where do you want to place the trap?\r\n", ch);
      return;
    }

    // costs 15 movement to set a trap
    if (GET_MOVE(ch) < 15) {
      send_to_char("You are too exhausted to place traps.\r\n", ch);
      return;
    }

    // if the argument is an object in inventory
    obj = get_obj_in_list_vis( ch, arg, ch->carrying);
    // or an object in the room
    if (!obj) obj = get_obj_in_list_vis( ch, arg, world[ch->in_room].contents);

    if (obj) {
      // can only trap containers
      if (!IS_CONTAINER(obj)) {
        send_to_char("You can't set a trap on that object!\r\n", ch);
        return;
      }

      // can't trap it unless it is closed
      if (!IS_SET(GET_OBJ_VAL(obj, 1), CONT_CLOSED)) {
        send_to_char("You can't trap open containers.\r\n", ch);
        return;
      }

      // if there is already a trap on it, ka-BOOOM!
      if (GET_TRAP(obj) > 0) {
        trigger_obj_trap(ch, obj);
        return;
      }

      // go ahead and set an object trap
    } else {
      // is this a direction?
      for (dirn = 0; dirn < NUM_OF_DIRS; dirn++)
          if (is_abbrev(arg, dirs[dirn])) break;
      if (dirn == NUM_OF_DIRS) {
          send_to_char("What do you want to set a trap on?\r\n", ch);
          return;
      }

      // is there an exit there?
      wld = world[ch->in_room].dir_option[dirn];
      if (!wld) {
        send_to_char("There is no exit in that direction.\r\n", ch);
        return;
      }

      // if it's trapped, ka-BOOOOM
      if (GET_TRAP(wld)) {
        trigger_wld_trap(ch, dirn);
        return;
      }

      // go ahead and set a world trap
    }

    GET_MOVE(ch) -= 15;

    // if you fail, it hurts
    if( !skillSuccess( ch, THIS_SKILL )) {
        STUN_USER_MIN;
        damage(ch, ch,
                GET_LEVEL(ch) <= 40 ? dice(1, GET_LEVEL(ch)) : dice(2, GET_LEVEL(ch)),
                SKILL_TRAP);
        // if the trap is level 50+, attempt a poison
        if (GET_LEVEL(ch) >= 50 && !mag_savingthrow(ch, SAVING_SPELL))
            add_affect(ch, ch, SKILL_TRAP, GET_LEVEL(ch), APPLY_STR, -2,
                    1 TICKS, AFF_POISON, FALSE, FALSE, FALSE, FALSE);
        return;
    }

    advanceSkill( ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING, DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS );

    STUN_USER_MIN;

    if (obj) {
      GET_TRAP(obj) = GET_LEVEL(ch);
      if(IS_BUTCHER(ch) && GET_ADVANCE_LEVEL(ch) >= THIRD_ADVANCE_SKILL)
          GET_TRAP(obj) += number(0, 25);
      act("You cunningly place a trap on $p.", FALSE, ch, obj, 0, TO_CHAR);
    } else {
      GET_TRAP(wld) = GET_LEVEL(ch);
      if(IS_BUTCHER(ch) && GET_ADVANCE_LEVEL(ch) >= THIRD_ADVANCE_SKILL)
          GET_TRAP(wld) += number(0, 25);
      send_to_char("You cunningly place a trap.\r\n", ch);
    }

}/* do_trap */

#undef THIS_SKILL
#define THIS_SKILL SKILL_SEARCH_TRAP

ACMD(do_detect_trap)
{
  ObjData *obj;
  int i;

  IF_UNLEARNED_SKILL( "You wouldn't know where to begin.\r\n" );

  send_to_char("You carefully examine the room for traps.\r\n", ch);

  // scan exits
  for (i = 0; i < NUM_OF_DIRS; i++) {
    RoomDirectionData *dir = world[ch->in_room].dir_option[i];
    if (dir && GET_TRAP(dir) > 0 && skillSuccess(ch, SKILL_SEARCH_TRAP)) {
      send_to_char("You notice something rather suspicious ", ch);
      switch (i) {
        case NORTH: send_to_char("to the north.\r\n", ch); break;
        case EAST: send_to_char("to the east.\r\n", ch); break;
        case WEST: send_to_char("to the west.\r\n", ch); break;
        case SOUTH: send_to_char("to the south.\r\n", ch); break;
        case DOWN: send_to_char("below you.\r\n", ch); break;
        case UP: send_to_char("above you.\r\n", ch); break;
      }
    }
  }

  // scan objects
  for (obj = world[ch->in_room].contents; obj; obj = obj->next_content) {
    if (GET_TRAP(obj) > 0 && skillSuccess(ch, SKILL_SEARCH_TRAP)) {
      act("You notice something rather suspicious about $p.", FALSE, ch, obj,
          0, TO_CHAR);
    }
  }
}

// bonuses for disabling traps
static const int dex_bonus[] = {
  -100, -60, -40, -30, -30, -20, -20, -15, -15, -10, -10, -5, 0, 0, 0, 0, 0,
  0, 5, 5, 10, 15, 15, 15, 15, 15,
};

/*
 * We're going to combine disable and search for traps into one skill.
   #undef THIS_SKILL
   #define THIS_SKILL SKILL_DISABLE_TRAP
 */

ACMD(do_disable_trap)
{
    char   arg[100];
    RoomDirectionData *wld = NULL;
    ObjData *obj;
    int dirn = 0;
    int success;
    int trap;

    IF_UNLEARNED_SKILL( "You wouldn't know where to begin.\r\n" );

    one_argument( argument, arg );

    if (arg[0] == '\0') {
      send_to_char("Which trap do you want to disable?\r\n", ch);
      return;
    }

    // if the argument is an object in inventory
    obj = get_obj_in_list_vis( ch, arg, ch->carrying);
    // or an object in the room
    if (!obj) obj = get_obj_in_list_vis( ch, arg, world[ch->in_room].contents);


    if (obj) {
      // if there isn't a trap on it, we don't care
      if (GET_TRAP(obj) == 0) {
        send_to_char("You relax as you notice it is not trapped.\r\n", ch);
        return;
      }

      // go ahead and make an attempt at disarming this object
      trap = GET_TRAP(obj);
    } else {
      // is this a direction?
      for (dirn = 0; dirn < NUM_OF_DIRS; dirn++)
        if (is_abbrev(arg, dirs[dirn])) break;
      if (dirn == NUM_OF_DIRS) {
        send_to_char("Which trap do you want to disable?\r\n", ch);
        return;
      }

      // is there an exit there?
      wld = world[ch->in_room].dir_option[dirn];
      if (!wld) {
        send_to_char("There is no exit in that direction.\r\n", ch);
        return;
      }

      // if it's not trapped, what do we care?
      if (GET_TRAP(wld) == 0) {
        send_to_char("You relax as you notice it is not trapped.\r\n", ch);
        return;
      }

      // go ahead and disable a world trap
      trap = GET_TRAP(wld);
    }

    // dex bonus, 90% base chance, diff in levels
    success = dex_bonus[GET_DEX(ch)] + 90 + GET_LEVEL(ch) - trap;

    // ka-BOOOM
    if( number(1,100) > success || !skillSuccess( ch, THIS_SKILL )) {
      if (obj) trigger_obj_trap(ch, obj);
      else trigger_wld_trap(ch, dirn);
      STUN_USER_MIN;
      return;
    }

    advanceSkill( ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING, DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS );

    STUN_USER_MIN;

    // remove the trap
    if (obj) GET_TRAP(obj) = 0;
    else GET_TRAP(wld) = 0;

    act("You carefully disable the trap.", FALSE, ch, 0, 0, TO_CHAR);
}

/*******************************************************************************
 *			       	    Traps				       *
 *			           1-10-99				       *
 ******************************************************************************/


// make cause pain
static void cause_trap_go_boom_now(CharData *ch, int trap)
{
  CharData *vict, *nextvict;
  int dam;

  // oh no!
  act("$n sets off a trap, which explodes!", FALSE, ch, 0, 0, TO_ROOM);
  act("You set off a trap, which explodes!", FALSE, ch, 0, 0, TO_CHAR);

  project_sound(ch, "You hear a distant explosion from %s.", "You hear a distant explosion");

  // no damage, phew
  if( IS_SET_AR( ROOM_FLAGS( ch->in_room ), ROOM_PEACEFUL )) return;

  // ouch!
  for (vict = world[ch->in_room].people; vict; vict = nextvict) {
    nextvict = vict->next_in_room;

    // work out the damage done
    if (trap <= 40) dam = dice(1, trap);
    else dam = dice(2, trap);

    if(percentSuccess(trap)) {
        sendChar(vict, "The concussive blow from the trap leaves you stunned.\r\n");
        STUN_SET(vict, 2);
    }

    // do it
    damage(vict, vict, dam, SKILL_TRAP);

    // if the trap is level 50+, attempt a poison
    if (trap >= 50 && !mag_savingthrow(vict, SAVING_SPELL))
      add_affect(ch, vict, SKILL_TRAP, trap, APPLY_STR, -2, 2 TICKS, AFF_POISON,
          FALSE, FALSE, FALSE, FALSE);
    if (trap >= 60 && !mag_savingthrow(vict, SAVING_SPELL))
        add_affect(ch, vict, SKILL_TRAP, GET_LEVEL(ch), 0, 0,
                1 TICKS, AFF_DISEASE, FALSE, FALSE, FALSE, FALSE);
    if (percentSuccess((trap-50)*2))
        add_affect(ch, vict, SKILL_TRAP, GET_LEVEL(ch), 0, 0,
                5, AFF_PARALYZE, FALSE, FALSE, FALSE, FALSE);
    // shake their nerves and rattle their brains
  }
}

static int trap_explodes(CharData *ch, int trap)
{
  int chance = (trap - GET_LEVEL(ch)) / 2 + 75;

  if (percentSuccess(chance)) {
    if ((IS_HALFLING(ch) && percentSuccess(15)) ||
            (GET_RACE(ch) == RACE_SHALFLING && percentSuccess(15))) {
      send_to_char("You trigger a trap, but it doesn't go off!\r\n", ch);
      return 0;
    }
  } else {
    act("A trap makes a loud bang but fails to cause any damage.", FALSE, ch,
        0, 0, TO_ROOM);
    act("A trap makes a loud bang but fails to cause any damage.", FALSE, ch,
        0, 0, TO_CHAR);
    return 0;
  }
  return 1;
}

void trigger_obj_trap(CharData *ch, ObjData *obj)
{
    if (trap_explodes(ch, GET_TRAP(obj)))
        cause_trap_go_boom_now(ch, GET_TRAP(obj));
    GET_TRAP(obj) = 0;
}

void trigger_wld_trap(CharData *ch, int dir)
{
    RoomDirectionData *exit = world[ch->in_room].dir_option[dir];
    if (trap_explodes(ch, GET_TRAP(exit)))
        cause_trap_go_boom_now(ch, GET_TRAP(exit));
    GET_TRAP(exit) = 0;
}

