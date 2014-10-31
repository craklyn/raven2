/**************************************************************************
*   File: copy.c                                    Part of RavenMUD      *
*  Usage: Source file for class-specific code                             *
*  15-Apr-2000                                                            *
*  All rights reserved.  Based on CircleMUD : For RavenMud only : Mortius *
**************************************************************************/

#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/comm.h"
#include "general/class.h"
#include "magic/spells.h"
#include "util/utils.h"
#include "specials/boards.h"
#include "specials/shop.h"
#include "olc/olc.h"
#include "olc/oedit.h"
#include "olc/medit.h"
#include "magic/missile.h"
#include "magic/trap.h"
#include "actions/interpreter.h"
#include "olc/copy.h"


/* Local function Declarations */
int copy_real_zone(int number);

/*
 * Usage message
*/

#define USAGE_MSG "Usage: copy { room | obj} <source> <target>\r\n"

/*
 * Lets do all the checks here.  Make sure source exists and target
 * isn't already used, etc.  Most of this will be based on the Circle3.0
 * version but the crashing bugs will be removed and the code changed.
*/

ACMD(do_copy)
{
  int vnum_targ = 0, vnum_src = 0, save_zone = 0, room_or_obj = -1;
  int rnum_targ = 0, rnum_src = 0;
  char src_num[20], targ_num[20], type[20];

  argument = two_arguments(argument, type, src_num);
  one_argument(argument, targ_num);

  /* Check all arguments were supplied */
  if (!*type || !*src_num) {
      send_to_char(USAGE_MSG, ch);
      return;
  } else if (!*targ_num && (room_or_obj == OBJECT)) {
       send_to_char("You must specify a target when copying objects.\r\n", ch);
       return;
  }

  /* Room or Object? */
  if (is_abbrev(type, "room") && is_number(src_num)) {
      room_or_obj = ROOM;
      vnum_src = atoi(src_num);
      rnum_src = real_room(vnum_src);
      if (!*targ_num) {
          vnum_targ = world[IN_ROOM(ch)].number;
          rnum_targ = IN_ROOM(ch);
      } else {
           if (!is_number(targ_num)) {
	       send_to_char(USAGE_MSG, ch);
	       return;
	   }
	   vnum_targ = atoi(targ_num);
	   rnum_targ = real_room(vnum_targ);
      }
  } else if (is_abbrev(type, "obj") && *targ_num && is_number(src_num) &&
             is_number(targ_num)) {
    room_or_obj = OBJECT;
        vnum_src = atoi(src_num);
    rnum_src = real_object(vnum_src);
    vnum_targ = atoi(targ_num);
    rnum_targ = real_object(vnum_targ);
  } else {
    send_to_char(USAGE_MSG, ch);
    return;
  }

  save_zone = copy_real_zone(vnum_targ);
/*
  if ((rnum_src < 0) || (rnum_targ < 0)) {
    sprintf(buf, "The source and target %ss must both currently exist.\r\n", (room_or_obj == OBJECT ? "object" : "room"));
    send_to_char(buf, ch);
    return;
  } else if (!can_edit_zone(ch, save_zone)) {
    send_to_char("You cannot edit that zone.\r\n", ch);
    return;
  }
*/


  /* We should now be ready to go.  All errors have been trapped (?) *
   * and we know what to do.                      TR 5-21-98         */
  switch (room_or_obj) {
  case ROOM:
    copy_room(rnum_src, rnum_targ);
    break;
  case OBJECT:
    copy_object(rnum_src, rnum_targ);
    break;
  default:
    mudlog(NRM, MAX(LVL_BUILDER, GET_INVIS_LEV(ch)), TRUE, "SYSERR: OLC: Reached default case in do_copy!");
    send_to_char("There was an error in your copy.  Please report to an "
                "administrator.\r\n", ch);
    return;
  }

  /* I cheated right here a little bit.  By coincidence, *
   * ROOM == OLC_SAVE_ROOM, and OBJECT = OLC_SAVE_OBJ.   *
   * I think this is a Good Thing(tm), although if you   *
   * change the OLC_SAVE_x defines, it will blow up.  :P *
   *                                    TR 5-21-98       */
  sprintf(buf, "You copy %s %d to %d.\r\n", (room_or_obj == ROOM ? "room" : "object"), vnum_src, vnum_targ);
  send_to_char(buf, ch);
  olc_add_to_save_list(save_zone, room_or_obj);
  mudlog( CMP, GET_INVIS_LEV(ch), TRUE, "OLC: %s copies %s %d to %d.",
	  GET_NAME(ch), (room_or_obj == ROOM ? "room" : "object"),
	  vnum_src, vnum_targ);
}

/* At some point when I have more time fix this so all builders can
 * copy into there own zones but for now this will get Hasana going.
*/

int can_edit_zone(CharData *ch, int zone_num)
{
  if (GET_LEVEL(ch) < LVL_DEITY) {
      send_to_char("Your are to low of lvl to use copy.\r\n", ch);
      return FALSE;
  }

  /* Ok we got this far so lets give them the ok to edit */ 
  return TRUE;
}

void copy_room(int rnum_src, int rnum_targ)
{
  if (world[rnum_src].name)
    world[rnum_targ].name = str_dup(world[rnum_src].name);
  if (world[rnum_src].description)
    world[rnum_targ].description = str_dup(world[rnum_src].description);

  world[rnum_targ].sector_type = world[rnum_src].sector_type;
  world[rnum_targ].room_flags[0] = world[rnum_src].room_flags[0];
  world[rnum_targ].room_flags[1] = world[rnum_src].room_flags[1];
  world[rnum_targ].room_flags[2] = world[rnum_src].room_flags[2];
  world[rnum_targ].room_flags[3] = world[rnum_src].room_flags[3];

  /* Note:  ex_descriptions are not being      *
   * copied.  I think it will stay that way.   *
   *                       TR 5-20-98          */
  return;
}


void copy_object(int rnum_src, int rnum_targ)
{
  if( obj_proto[rnum_src].name )
    obj_proto[rnum_targ].name = str_dup( obj_proto[rnum_src].name );
  else
    obj_proto[rnum_targ].name = str_dup( "NONAME" );

  if( obj_proto[rnum_src].description )
    obj_proto[rnum_targ].description = str_dup(obj_proto[rnum_src].description);
  else
    obj_proto[rnum_targ].description = str_dup( "NODESCRIPT" );

  if( obj_proto[rnum_src].short_description )
    obj_proto[rnum_targ].short_description =
                               str_dup(obj_proto[rnum_src].short_description);
  else
    obj_proto[rnum_targ].short_description = str_dup( "NOSHORT" );

  if( obj_proto[rnum_src].action_description )
    obj_proto[rnum_targ].action_description =
                         str_dup(obj_proto[rnum_src].action_description);
  else
    obj_proto[rnum_targ].action_description = str_dup( "NOACTDESC" );

/*
  if( obj_proto[rnum_src].ex_description )
    obj_proto[rnum_targ].ex_description = obj_proto[rnum_src].ex_description;
  else
    obj_proto[rnum_targ].ex_description = NULL;
*/

  obj_proto[rnum_targ].obj_flags = obj_proto[rnum_src].obj_flags;
  obj_proto[rnum_targ].worn_at   = obj_proto[rnum_src].worn_at;

  /* add more if you want... */

  return;
}

int copy_real_zone(int number)
{
  int counter;

  for (counter = 0; counter <= top_of_zone_table; counter++)
       if ((number >= (zone_table[counter].number * 100)) &&
          (number <= (zone_table[counter].top))) 
       return zone_table[counter].number;

  return -1;
}
