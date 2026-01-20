/**************************************************************************
*   File: insert_items.c                                Part of RavenMUD  *
*  Usage: Inserting items into weapons/etc                                *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*									  *
* Base On CircleMUD By Mortius of RavenMUD : 05-Sep-00			  *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "util/utils.h"
#include "general/comm.h"
#include "actions/interpreter.h"
#include "general/handler.h"
#include "general/class.h"
#include "magic/spells.h"
#include "general/color.h"
#include "specials/house.h"
#include "general/objsave.h"
#include "actions/act.h"          /* ACMDs located within the act*.c files */

ACMD(do_meld)
{

  ObjData *main_obj = NULL, *insert_obj = NULL, *new_obj, *tmp_obj;
  char *desc;
  char arg1[30], arg2[30];
  char new_item[10] = "0";
  char insert_item[10] = "NULL";
  char insert_to_char[120]   = "INSERT ERROR: REPORT THIS TO AN IMM\r\n";
  char insert_to_room[120]   = "INSERT ERROR: REPORT THIS TO AN IMM\r\n";
  char insert_fail_char[120] = "INSERT ERROR: REPORT THIS TO AN IMM\r\n";
  char insert_fail_room[120] = "INSERT ERROR: REPORT THIS TO AN IMM\r\n";

  two_arguments(argument, arg1, arg2);

    if (!*arg2)  {
      send_to_char("You need something to meld it into!\r\n", ch);
      return;
  }

  if (!*arg1) {
      send_to_char("You must want to meld something?\r\n", ch);
      return;
  }

  if (!(main_obj = get_obj_in_list_vis(ch, arg2, ch->carrying))) {
        send_to_char("You don't have one of those!\r\n", ch);
        return;
  } 

  if (!(insert_obj = get_obj_in_list_vis(ch, arg1, ch->carrying))) {
        send_to_char("You don't have one of those!\r\n", ch);
        return;
  }

  if (!IS_OBJ_STAT(main_obj, ITEM_MAIN_PART)) {
      send_to_char("You can't meld anything into that!\r\n", ch);
      return;
  }

  if (!IS_OBJ_STAT(insert_obj, ITEM_INSERT)) {
      send_to_char("That item doesn't seem to meld into that!\r\n", ch);
      return;
  }

  /* We are going to pull all of the info for which items to use/load, messages
     to send to the players/room all from the main objects extra_desc */

  if ((desc = find_exdesc("insert_to_char", main_obj->ex_description)) != NULL)
       strcpy(insert_to_char, desc);

  if ((desc = find_exdesc("insert_to_room", main_obj->ex_description)) != NULL)
       strcpy(insert_to_room, desc);

  if ((desc = find_exdesc("insert_fail_char", main_obj->ex_description)) != NULL)
       strcpy(insert_fail_char, desc);

  if ((desc = find_exdesc("insert_fail_room", main_obj->ex_description)) != NULL)
       strcpy(insert_fail_room, desc);

  if ((desc = find_exdesc("insert_item", main_obj->ex_description)) != NULL)
       strcpy(insert_item, desc);

  if ((desc = find_exdesc("new_item", main_obj->ex_description)) != NULL)
       strcpy(new_item, desc);

  /* Mortius: If we don't add this here it crashes when we try and remove the
             item later */

  tmp_obj = insert_obj;

  if (insert_obj->item_number == real_object(atoi(insert_item))) {
      act(insert_to_char, FALSE, ch, main_obj, 0, TO_CHAR);
      act(insert_to_room, FALSE, ch, main_obj, 0, TO_ROOM);
      new_obj = read_object(atoi(new_item), VIRTUAL);
      obj_from_char(tmp_obj);
      obj_from_char(main_obj);
      obj_to_char(new_obj, ch);
  } else {
        act(insert_fail_char, FALSE, ch, main_obj, 0, TO_CHAR);
	act(insert_fail_room, FALSE, ch, main_obj, 0, TO_ROOM);
        return;
  }
}
