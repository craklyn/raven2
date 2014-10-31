/* ************************************************************************
 *   File: objact.c                                    Part of CircleMUD *
 *  Usage: object handling routines -- get/drop and container handling     *
 *                                                                         *
 *  All rights reserved.  See license.doc for complete information.        *
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
#include "magic/skills.h"
#include "magic/fishing.h"
#include "util/weather.h"
#include "actions/act.h"          /* ACMDs located within the act*.c files */
#include "actions/quest.h"
#include "actions/outlaw.h"
#include "scripts/dg_scripts.h"
#include "specials/flag_game.h"
#include "specials/scatter.h"
#include "specials/bloodbowl.h"
#include "specials/scatter.h"  /* for check_scatter_give */

static int
count_items (ObjData * obj)
{
  int count = 1;
  ObjData *i;

  if (IS_CONTAINER (obj))
    {
      i = obj->contains;
      while (i)
        {
          count += count_items (i);
          i = i->next_content;
        }
    }
  return count;
}

static int
locker_check (CharData *ch, ObjData * obj)
{
  int count = 1;
  ObjData *i;

  if (Crash_is_unrentable (obj))
    {
      act ("You cannot put $p in there!", FALSE, ch, obj, NULL, TO_CHAR);
      return 0;
    }
  if (IS_CONTAINER (obj))
    {
      i = obj->contains;
      while (i)
        {
          int sub = locker_check (ch, i);

          if (!sub) return 0;
          count += sub;
          i = i->next_content;
        }
    }
  return count;
}

void
perform_put (CharData * ch, ObjData * obj, ObjData * cont)
{
    ObjData *tempObj;
    int holdingContainer = FALSE;

    for( tempObj = ch->carrying; tempObj != NULL; tempObj = tempObj->next_content )
    {
        if(tempObj == cont  )
            holdingContainer = TRUE;
    }

    if(!holdingContainer && (IS_SET_AR(GET_OBJ_EXTRA(obj), ITEM_SOULBOUND) || contains_soulbound(obj))) {
        sendChar(ch, "You cannot transfer a bound item to a container you aren't holding.\r\n");
        return;
    }

    if(IS_LOCKER (cont))
    { // special handling for lockers
        int inobj = locker_check(ch, obj);
        int incont = count_items(cont) - 1;

        if (inobj < 1) return;
        if (inobj + incont <= GET_OBJ_VAL (cont, 0))
        {
            obj_from_char (obj);
            obj_to_obj (obj, cont);
            act ("You put $p in $P.", FALSE, ch, obj, cont, TO_CHAR);
        }
        else
        {
            act ("There are too many items in $P!", FALSE, ch, obj, cont, TO_CHAR);
        }
        return;
    }
    if (GET_OBJ_WEIGHT (cont) + GET_OBJ_WEIGHT (obj) > GET_OBJ_VAL (cont, 0))
        act ("$p won't fit in $P.", FALSE, ch, obj, cont, TO_CHAR);
    else if (IS_SET_AR (GET_OBJ_EXTRA (obj), ITEM_ARENA))
        act ("You can't hide $p in $P!", FALSE, ch, obj, cont, TO_CHAR);
    else
    {
        obj_from_char (obj);
        obj_to_obj (obj, cont);
        act ("You put $p in $P.", FALSE, ch, obj, cont, TO_CHAR);
#ifdef _SHOW_EVERYTHING
        act ("$n puts $p in $P.", TRUE, ch, obj, cont, TO_ROOM);
#endif
    }
}

int contains_soulbound(ObjData *obj) {
    ObjData *tempObj;

    if(!obj) {
        mlog("SYSERR: Null pointer sent to contains_soulbound!");
        return FALSE;
    }


    // if it contains nothing, it cannot contain a soulbound object.
    if(! obj->contains)
        return FALSE;

    // Look through everything inside the object, and everything inside
    // those objects, etc
    for(tempObj = obj->contains; tempObj; tempObj = tempObj->next_content) {
        if(IS_SET_AR(GET_OBJ_EXTRA(tempObj), ITEM_SOULBOUND))
            return TRUE;
        if (tempObj->contains)
            if(contains_soulbound(tempObj))
                return TRUE;
    }

    // Nothing found, so report it contains no soulbound object.
    return FALSE;
}


/* The following put modes are supported by the code below:

        1) put <object> <container>
        2) put all.<object> <container>
        3) put all <container>

        <container> must be in inventory or on ground.
        all objects to be put into container must be in inventory.
 */
ACMD (do_put)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  ObjData *obj, *next_obj, *cont;
  CharData *tmp_char;
  int obj_dotmode, cont_dotmode, found = 0;

  if (!IS_NPC (ch) && IS_SET_AR (PLR_FLAGS (ch), PLR_HUNTED))
  {
      send_to_char ("Hunted players can't transfer or destroy equipment.\r\n", ch);
      return;
  }

  two_arguments (argument, arg1, arg2);
  obj_dotmode = find_all_dots (arg1);
  cont_dotmode = find_all_dots (arg2);

  if (!*arg1)
      send_to_char ("Put what in what?\r\n", ch);
  else if (cont_dotmode != FIND_INDIV)
      send_to_char ("You can only put things into one container at a time.\r\n", ch);
  else if (!*arg2)
  {
      sprintf (buf, "What do you want to put %s in?\r\n",
              ((obj_dotmode == FIND_INDIV) ? "it" : "them"));
      send_to_char (buf, ch);
  }
  else
  {
      generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &tmp_char, &cont);
      if (!cont)
      {
          sprintf (buf, "You don't see %s %s here.\r\n", AN (arg2), arg2);
          send_to_char (buf, ch);
      }
      else if (GET_OBJ_TYPE(cont) != ITEM_CONTAINER)
          act ("$p is not a container.", FALSE, ch, cont, 0, TO_CHAR);
      else if (IS_SET (GET_OBJ_VAL (cont, 1), CONT_CLOSED))
          send_to_char ("You'd better open it first!\r\n", ch);
      else
      {
          if (obj_dotmode == FIND_INDIV)
          { /* put <obj> <container> */
              if (!(obj = get_obj_in_list_vis (ch, arg1, ch->carrying)))
              {
                  sprintf (buf, "You aren't carrying %s %s.\r\n", AN (arg1), arg1);
                  send_to_char (buf, ch);
              }
              else if (obj == cont)
                  send_to_char ("You attempt to fold it into itself, but fail.\r\n", ch);
              else
                  perform_put(ch, obj, cont);
          }
          else
          {
              for (obj = ch->carrying; obj; obj = next_obj)
              {
                  next_obj = obj->next_content;
                  if (obj != cont && CAN_SEE_OBJ (ch, obj) &&
                          (obj_dotmode == FIND_ALL || isname (arg1, obj->name)))
                  {
                      found = 1;
                      perform_put(ch, obj, cont);
                  }
              }
              if (!found)
              {
                  if (obj_dotmode == FIND_ALL)
                      send_to_char ("You don't seem to have anything to put in it.\r\n", ch);
                  else
                  {
                      sprintf (buf, "You don't seem to have any %ss.\r\n", arg1);
                      send_to_char (buf, ch);
                  }
              }
          }
      }
  }
}

int
can_take_obj (CharData * ch, ObjData * obj)
{
    if (IS_CARRYING_N (ch) >= CAN_CARRY_N (ch))
    {
        act ("$p: you can't carry that many items.", FALSE, ch, obj, 0, TO_CHAR);
        return 0;
    }
    // Immortals not subject to weight restrictions.
    else if (((IS_CARRYING_W (ch) + GET_OBJ_WEIGHT (obj)) > CAN_CARRY_W (ch)) &&
            (GET_LEVEL (ch) < LVL_IMMORT)
            )
    {
        act ("$p: you can't carry that much weight.", FALSE, ch, obj, 0, TO_CHAR);
        return 0;
    }
    else if (!(CAN_WEAR (obj, ITEM_WEAR_TAKE)))
    {
        if ((GET_LEVEL (ch) >= LVL_CREATOR)
                && (GET_OBJ_TYPE (obj) == ITEM_CONTAINER)
                && (GET_OBJ_VAL (obj, 3) == 1))
            return 1; /* CREATOR+ can get player corpses - Vex. */
        else
        {
            act ("$p: you can't take that!", FALSE, ch, obj, 0, TO_CHAR);
            return 0;
        }
    }
    return 1;
}

void
get_check_money (CharData * ch, ObjData * obj)
{
  if ((GET_OBJ_TYPE (obj) == ITEM_MONEY) && (GET_OBJ_VAL (obj, 0) > 0))
    {
      obj_from_char (obj);
      if (GET_OBJ_VAL (obj, 0) > 1)
        {
          sprintf (buf, "There were %d coins.\r\n", GET_OBJ_VAL (obj, 0));
          send_to_char (buf, ch);
        }
      GET_GOLD (ch) += GET_OBJ_VAL (obj, 0);
      extract_obj (obj);
    }
}

void
perform_get_from_container (CharData *ch,
                            ObjData *obj,
                            ObjData * cont,
                            int mode)
{
  DescriptorData *i, *next_desc;
  CharData *corpsee; /* Person whose corpse this is.. */
  int loot_ok;

  if (mode == FIND_OBJ_INV || can_take_obj(ch, obj))
  {
      /* Controls on PC corpse looting - Vex. */
      if ((GET_OBJ_VAL (cont, 3) == 1) /* cont is a corpse */
              && !CAN_WEAR (cont, ITEM_WEAR_TAKE) /* cont is a pc corpse */
              && !(strcmp (cont->ex_description->keyword, GET_NAME (ch)) == 0) /* Names are not an exact match. */
              && !IN_QUEST_FIELD (ch))
          /* && !IS_SET_AR(PLR_FLAGS(ch), PLR_HUNTED)) /* ch is not hunted */
      {
          /* Ok, lets search the character list and see if the person whose
           *            ** corpse it is has given consent for it to be looted.
           *            */
          loot_ok = 0;
          for (i = descriptor_list; i; i = next_desc)
          {
              next_desc = i->next;
              if (i->connected || !(corpsee = i->character) || !(strcmp (cont->ex_description->keyword, GET_NAME (corpsee)) == 0))
                  continue;
              /* If we got this far, we found the person whose corpse this is. */
              if (IS_SET_AR (PRF_FLAGS (corpsee), PRF_CONSENT))
                  loot_ok = 1; /* They have given consent... */
              break; /* No need to continue with for loop. */
          }

          /* Mortius, lets make it so switched mobs can loot */
          if (IS_NPC (ch) && ch->desc != NULL)
              loot_ok = TRUE;
          
          if (!loot_ok)
          {
              if (!CONFIG_PK_ALLOWED) return;

              /*
              // A given corpse can only be looted once
              if( IS_OBJ_STAT(cont, ITEM_LOOTED) )
              {
                  send_to_char("This corpse has already been robbed once.  Isn't that enough?\r\n", ch);
                  return;
              }
              
              // A given player can only loot one item at a time
              if( IS_HUNTED(ch) )
              {
                  send_to_char("You can't loot another corpse when you're already hunted.\r\n", ch);
                  return;
              }
              */
              
              if (!IS_SET_AR (PRF_FLAGS (ch), PRF_CONSENT) &&
                      !IS_SET_AR (PLR_FLAGS (ch), PLR_HUNTED))
              {
                  send_to_char ("If you want to loot this corpse and be hunted, type consent first.\r\n", ch);
                  return; /* bail out. */
              }
              else if(IS_OBJ_STAT(obj, ITEM_SOULBOUND)) {
                  send_to_char("You cannot steal a bound item from another player.\r\n", ch);
                  return;
              }
              else
              {
                  // Call outlaw code.
                  player_loot_corpse (ch, cont);
              } /* else */
          } /* if !loot_ok */
          else if(IS_OBJ_STAT(obj, ITEM_SOULBOUND)) {
              send_to_char("You cannot steal a bound item from another player.\r\n", ch);
              return;
          } /* Even with permission, you're not allowed to loot another player's bound item */

      } /* End of PC specefic corpse checks */

      if (IS_CARRYING_N (ch) >= CAN_CARRY_N (ch))
          act ("$p: you can't hold any more items.", FALSE, ch, obj, 0, TO_CHAR);
      else if (get_otrigger (obj, ch))
      {
          obj_from_obj (obj);
          obj_to_char (obj, ch);
          act ("You get $p from $P.", FALSE, ch, obj, cont, TO_CHAR);
          if (GET_OBJ_VAL(cont, 3) == 1) /* Show objects from corpses. */
              act ("$n gets $p from $P.", FALSE, ch, obj, cont, TO_ROOM);
          get_check_money (ch, obj);

          if (!IS_NPC(ch))
              Crash_crashsave(ch);
      }
  }
}

void
get_from_container (CharData *ch, ObjData *cont, char *arg, int mode)
{
  ObjData *obj, *next_obj;
  int obj_dotmode, found = 0;

  obj_dotmode = find_all_dots(arg);

  if (IS_SET (GET_OBJ_VAL (cont, 1), CONT_CLOSED))
      act ("$p is closed.", FALSE, ch, cont, 0, TO_CHAR);
  else if (obj_dotmode == FIND_INDIV)
  {
      if (!(obj = get_obj_in_list_vis(ch, arg, cont->contains)))
      {
          sprintf (buf, "There doesn't seem to be %s %s in $p.", AN (arg), arg);
          act (buf, FALSE, ch, cont, 0, TO_CHAR);
      }
      else
          perform_get_from_container(ch, obj, cont, mode);
  }
  else
  {
      if (obj_dotmode == FIND_ALLDOT && !*arg)
      {
          send_to_char ("Get all of what?\r\n", ch);
          return;
      }
      for (obj = cont->contains; obj; obj = next_obj)
      {
          next_obj = obj->next_content;
          if (CAN_SEE_OBJ (ch, obj) &&
                  (obj_dotmode == FIND_ALL || isname (arg, obj->name)))
          {
              found = 1;
              perform_get_from_container(ch, obj, cont, mode);
          }
      }
      if (!found)
      {
          if (obj_dotmode == FIND_ALL)
              act ("$p seems to be empty.", FALSE, ch, cont, 0, TO_CHAR);
          else
          {
              sprintf (buf, "You can't seem to find any %ss in $p.", arg);
              act (buf, FALSE, ch, cont, 0, TO_CHAR);
          }
      }
  }
}


#define STUN_MIN 1

int
perform_get_from_room (CharData * ch, ObjData * obj)
{
  static char *black = "&07(black)&00";
  static char *gold = "&10(gold)&00";
  static char *none = "&03(no team)&00";
  static char *rogue = "&09(rogue)&00";
  
  char *lColor = (PRF_FLAGGED (ch, PRF_GOLD_TEAM) ? gold : black);
  if (!PRF_FLAGGED (ch, PRF_GOLD_TEAM) && !PRF_FLAGGED (ch, PRF_BLACK_TEAM)) lColor = none;
  if (PRF_FLAGGED (ch, PRF_ROGUE_TEAM)) lColor = rogue;
  if (can_take_obj (ch, obj) && get_otrigger (obj, ch))
  {
      obj_from_room (obj);
      obj_to_char (obj, ch);
      if (!IS_NPC(ch))
          Crash_crashsave(ch);
      
      act ("You get $p.", FALSE, ch, obj, 0, TO_CHAR);
      act ("$n gets $p.", TRUE, ch, obj, 0, TO_ROOM);
      if ((GET_OBJ_VNUM (obj) == GOLD_TEAM_STANDARD) && (GET_LEVEL (ch) <= 51))
      {
          if (PRF_FLAGGED (ch, PRF_GOLD_TEAM) || PRF_FLAGGED (ch, PRF_ROGUE_TEAM))
          {
              sprintf (buf, "&08FLAG GAME:&00&11 %s&00 %s &11has returned the&00 &10GOLD&00&11 Flag!&00\r\n", GET_NAME (ch), lColor);
              obj_from_char (obj);
              obj_to_room (obj, real_room (20095));
              quest_echo (buf);
              flag_player_return (ch);
          }
          else
          {
              sprintf (buf, "&08FLAG GAME:&00 &08%s&00 %s &08has taken the&00 &10GOLD&00 &08Flag!&00\r\n", GET_NAME (ch), lColor);
              quest_echo (buf);
              STUN_USER_MIN;
          }
      }
      if ((GET_OBJ_VNUM (obj) == BLACK_TEAM_STANDARD) && (GET_LEVEL (ch) <= 51))
      {
          if (PRF_FLAGGED (ch, PRF_BLACK_TEAM) || PRF_FLAGGED (ch, PRF_ROGUE_TEAM))
          {
              sprintf (buf, "&08FLAG GAME:&00 &11%s&00 %s &11has returned the&00 &07BLACK&00 &11Flag!&00\r\n", GET_NAME (ch), lColor);
              obj_from_char (obj);
              obj_to_room (obj, real_room (20195));
              quest_echo (buf);
              flag_player_return (ch);
          }
          else
          {
              sprintf (buf, "&08FLAG GAME:&00 &08%s&00 %s &08has taken the&00 &07BLACK&00 &08Flag!&00\r\n", GET_NAME (ch), lColor);
              quest_echo (buf);
              STUN_USER_MIN;
          }
      }
      if ((GET_OBJ_VNUM (obj) == BLOODBALL) && (GET_LEVEL (ch) <= 51))
      {
          set_title (ch, "&08(BallCarrier)&00");
          sprintf (buf, "&08BLOODBOWL:&00 %s %s has recovered the ball.\r\n", GET_NAME (ch), lColor);
          quest_echo (buf);
      }
      get_check_money (ch, obj);
      return 1;
  }
  return 0;
}

void
get_from_room (CharData * ch, char *arg)
{
  ObjData *obj, *next_obj;
  int dotmode, found = 0, immortal = FALSE;

  if (GET_LEVEL (ch) >= LVL_IMMORT)
    immortal = TRUE;

  dotmode = find_all_dots (arg);

  if (dotmode == FIND_INDIV)
    {
      if (!(obj = get_obj_in_list_vis (ch, arg, world[ch->in_room].contents)))
        {
          sprintf (buf, "You don't see %s %s here.\r\n", AN (arg), arg);
          send_to_char (buf, ch);
        }
      else
        {
          perform_get_from_room (ch, obj);
        }
    }
  else
    {
      if (dotmode == FIND_ALLDOT && !*arg)
        {
          send_to_char ("Get all of what?\r\n", ch);
          return;
        }
      for (obj = world[ch->in_room].contents; obj; obj = next_obj)
        {
          next_obj = obj->next_content;

          if (CAN_SEE_OBJ (ch, obj) &&
              (dotmode == FIND_ALL || isname (arg, obj->name)))
            {
              found = TRUE;
              perform_get_from_room (ch, obj);
            }
        }
      if (!found)
        {
          if (dotmode == FIND_ALL)
            send_to_char ("There doesn't seem to be anything here.\r\n", ch);
          else
            {
              sprintf (buf, "You don't see any %ss here.\r\n", arg);
              send_to_char (buf, ch);
            }
        }
    }
}

ACMD (do_get)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];

  int cont_dotmode, found = 0, mode;
  ObjData *cont;
  CharData *tmp_char;

  two_arguments (argument, arg1, arg2);

  if (IS_CARRYING_N (ch) >= CAN_CARRY_N (ch))
    send_to_char ("Your arms are already full!\r\n", ch);
  else if (!*arg1)
    send_to_char ("Get what?\r\n", ch);
  else if (!*arg2)
    get_from_room (ch, arg1);
  else
    {
      cont_dotmode = find_all_dots (arg2);
      if (cont_dotmode == FIND_INDIV)
        {
          mode = generic_find (arg2, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &tmp_char, &cont);
          if (!cont)
            {
              sprintf (buf, "You don't have %s %s.\r\n", AN (arg2), arg2);
              send_to_char (buf, ch);
            }
          else if (!IS_CONTAINER (cont))
            act ("$p is not a container.", FALSE, ch, cont, 0, TO_CHAR);
          else
            get_from_container (ch, cont, arg1, mode);
        }
      else
        {
          if (cont_dotmode == FIND_ALLDOT && !*arg2)
            {
              send_to_char ("Get from all of what?\r\n", ch);
              return;
            }
          for (cont = ch->carrying; cont; cont = cont->next_content)
            if (CAN_SEE_OBJ (ch, cont) &&
                (cont_dotmode == FIND_ALL || isname (arg2, cont->name)))
              {
                if (GET_OBJ_TYPE (cont) == ITEM_CONTAINER)
                  {
                    found = 1;
                    get_from_container (ch, cont, arg1, FIND_OBJ_INV);
                  }
                else if (cont_dotmode == FIND_ALLDOT)
                  {
                    found = 1;
                    act ("$p is not a container.", FALSE, ch, cont, 0, TO_CHAR);
                  }
              }
          for (cont = world[ch->in_room].contents; cont; cont = cont->next_content)
            if (CAN_SEE_OBJ (ch, cont) &&
                (cont_dotmode == FIND_ALL || isname (arg2, cont->name)))
              {
                if (GET_OBJ_TYPE (cont) == ITEM_CONTAINER)
                  {
                    get_from_container (ch, cont, arg1, FIND_OBJ_ROOM);
                    found = 1;
                  }
                else if (cont_dotmode == FIND_ALLDOT)
                  {
                    act ("$p is not a container.", FALSE, ch, cont, 0, TO_CHAR);
                    found = 1;
                  }
              }
          if (!found)
            {
              if (cont_dotmode == FIND_ALL)
                send_to_char ("You can't seem to find any containers.\r\n", ch);
              else
                {
                  sprintf (buf, "You can't seem to find any %ss here.\r\n", arg2);
                  send_to_char (buf, ch);
                }
            }
        }
    }
}

void
perform_drop_gold (CharData * ch, int amount, byte mode, sh_int RDR)
{
  ObjData *obj;

  if (amount <= 0)
    send_to_char ("Heh heh heh.. we are jolly funny today, eh?\r\n", ch);
  else if (GET_GOLD (ch) < amount)
    send_to_char ("You don't have that many coins!\r\n", ch);
  else
  {
      if (mode != SCMD_JUNK)
      {
          WAIT_STATE (ch, PULSE_VIOLENCE); /* to prevent coin-bombing */
          obj = create_money (amount);
          if (mode == SCMD_DONATE)
          {
              send_to_char ("You throw some gold into the air where it disappears in a puff of smoke!\r\n", ch);
              act ("$n throws some gold into the air where it disappears in a puff of smoke!",
                      FALSE, ch, 0, 0, TO_ROOM);
              obj_to_room (obj, RDR);
              act ("$p suddenly appears in a puff of orange smoke!", 0, 0, obj, 0, TO_ROOM);
          }
          else
          {
              if (!drop_wtrigger (obj, ch))
              {
                  extract_obj (obj);
                  return;
              }
              send_to_char ("You drop some gold.\r\n", ch);
              sprintf (buf, "$n drops %s.", money_desc (amount));
              act (buf, FALSE, ch, 0, 0, TO_ROOM);
              obj_to_room (obj, ch->in_room);
          }
      }
      else
      {
          sprintf (buf, "$n drops %s which disappears in a puff of smoke!", money_desc (amount));
          act (buf, FALSE, ch, 0, 0, TO_ROOM);
          send_to_char ("You drop some gold which disappears in a puff of smoke!\r\n", ch);
      }
      GET_GOLD (ch) -= amount;
  }
}


#define VANISH(mode) ((mode == SCMD_DONATE || mode == SCMD_JUNK) ? \
		      "  It vanishes in a puff of smoke!" : "")

int
perform_drop (CharData * ch, ObjData * obj,
              byte mode, char *sname, sh_int RDR)
{
  int value;
  static char *black = "&07(black)&00";
  static char *gold = "&10(gold)&00";
  static char *none = "&03(no team)&00";
  static char *rogue = "&09(rogue)&00";
  char *lColor = (PRF_FLAGGED (ch, PRF_GOLD_TEAM) ? gold : black);
  if (!PRF_FLAGGED (ch, PRF_GOLD_TEAM) &&
      !PRF_FLAGGED (ch, PRF_BLACK_TEAM)) lColor = none;
  if (PRF_FLAGGED (ch, PRF_ROGUE_TEAM)) lColor = rogue;

  // this was in perform_put for some bizarre reason
  if (!drop_otrigger (obj, ch))
    return;

  if (IS_OBJ_STAT (obj, ITEM_CURSED))
  {
      sprintf (buf, "You can't %s $p, it must be CURSED!", sname);
      act (buf, FALSE, ch, obj, 0, TO_CHAR);
      return 0;
  }

  if ((mode != SCMD_JUNK) && (IS_OBJ_STAT(obj, ITEM_SOULBOUND) || contains_soulbound(obj)) && !PRF_FLAGGED(ch, PRF_NOHASSLE )) {
      act("You cannot drop bound items.", FALSE, ch, obj, 0, TO_CHAR);
      return 0;
  }

  if ((mode == SCMD_DONATE) && IS_OBJ_STAT (obj, ITEM_NODONATE)) {
      sendChar(ch, "This cannot be donated.\r\n");
      return 0;
  }
  
  /* Digger added exploding EQ here. */
  if (IS_OBJ_STAT (obj, ITEM_EXPLODES))
  {
      act ("$p explodes with a blinding flash as it touches the floor!", FALSE, ch, obj, 0, TO_CHAR);
      act ("$p explodes with a blinding flash as it touches the floor!", FALSE, ch, obj, 0, TO_ROOM);
      extract_obj (obj);
      return 0;
  }

  sprintf (buf, "You %s $p.%s", sname, VANISH (mode));
  act (buf, FALSE, ch, obj, 0, TO_CHAR);
  sprintf (buf, "$n %ss $p.%s", sname, VANISH (mode));
  act (buf, TRUE, ch, obj, 0, TO_ROOM);
  obj_from_char (obj);

  switch (mode)
  {
      case SCMD_CAPT:
          if ((GET_OBJ_VNUM(obj) != BLACK_TEAM_STANDARD) && GET_OBJ_VNUM(obj) != GOLD_TEAM_STANDARD)
          {
              obj_to_char(obj, ch);
          }
          if (GET_OBJ_VNUM(obj) == BLACK_TEAM_STANDARD)
          {
              if ((IN_ROOM(ch) == real_room(20095)) && get_obj_in_list_vis(ch, "gold-flag-item", world[ch->in_room].contents))
              {
                  sprintf(buf, "&08FLAG GAME:&00 &14%s&00 %s &14has captured the&00 &07Black Flag&00!\r\n", GET_NAME (ch), lColor);
                  obj_to_room(obj, real_room (20195));
                  quest_echo(buf);
                  flag_player_capture(ch);
                  return 0;
              }
              //      if ((IN_ROOM(ch) == real_room(20095)) && !(get_obj_in_list_vis( ch, "gold-flag-item", world[ch->in_room].contents)))
              //        {
              //        send_to_char("Your flag isn't here, how can you capture their flag?\r\n", ch);
              //        return 0;
              //        }
              //      if (IN_ROOM(ch) != real_room(20095))
              else
              {
                  send_to_char("You cannot capture the flag here!\r\n", ch);
                  obj_to_char(obj, ch);
                  return 0;
              }
          }
          if (GET_OBJ_VNUM(obj) == GOLD_TEAM_STANDARD)
          {
              if ((IN_ROOM(ch) == real_room (20195)) && get_obj_in_list_vis(ch, "black-flag-item", world[ch->in_room].contents))
              {
                  sprintf(buf, "&08FLAG GAME:&00 &14%s&00 %s &14has captured the&00 &10Gold Flag&00!\r\n", GET_NAME (ch), lColor);
                  obj_to_room(obj, real_room (20095));
                  quest_echo(buf);
                  flag_player_capture(ch);
                  return 0;
              }
              //      if ((IN_ROOM(ch) == real_room(20095)) && (!get_obj_in_list_vis( ch, "gold-flag-item", world[ch->in_room].contents)))
              //        {
              //        send_to_char("Your flag isn't here, how can you capture their flag?\r\n", ch);
              //        return 0;
              //        }
              //      if (IN_ROOM(ch) != real_room(20195))
              else
              {
                  send_to_char("You cannot capture the flag here!\r\n", ch);
                  obj_to_char(obj, ch);
                  return 0;
              }
          }
          break;
      case SCMD_DROP:
          if (GET_OBJ_VNUM(obj) == BLACK_TEAM_STANDARD && GET_LEVEL (ch) <= 51)
          {
              sprintf(buf, "&08FLAG GAME:&00 %s %s has dropped the &07Black Flag&00 at &12%s&00!\r\n", GET_NAME (ch), lColor,
                      world[ch->in_room].name);
              quest_echo(buf);
          }
          if (GET_OBJ_VNUM(obj) == GOLD_TEAM_STANDARD && GET_LEVEL (ch) <= 51)
          {
              sprintf(buf, "&08FLAG GAME:&00 %s %s has dropped the &10Gold Flag&00 at &12%s&00!!\r\n", GET_NAME (ch), lColor,
                      world[ch->in_room].name);
              quest_echo(buf);
          }
          if (GET_OBJ_VNUM(obj) == BLOODBALL && GET_LEVEL(ch) <= 51)
          {
              set_title(ch, "");
              sprintf(buf, "&08BLOODBOWL:&00 %s %s has dropped the ball\r\n", GET_NAME (ch), lColor);
              quest_echo (buf);
          }
          obj_to_room(obj, ch->in_room);
          if (!IS_NPC(ch))
              Crash_crashsave(ch); /* close dupe method - Vex */
          return 0;
      case SCMD_DONATE:
          SET_BIT_AR(GET_OBJ_EXTRA(obj), ITEM_NOSELL);
          obj_to_room(obj, RDR);
          act("$p suddenly appears in a puff a smoke!", FALSE, 0, obj, 0, TO_ROOM);
          if (!IS_NPC(ch))
              Crash_crashsave(ch);
          return 0;
      case SCMD_DEST:
          extract_obj(obj);
          if (!IS_NPC(ch))
              Crash_crashsave(ch);
          return 0;
      case SCMD_JUNK:
          value = MAX(1, MIN (200, GET_OBJ_COST (obj) >> 4));
          extract_obj(obj);
          if (!IS_NPC(ch))
              Crash_crashsave(ch);
          return value;
      default:
          mlog("SYSERR: Incorrect argument passed to perform_drop");
          break;
  }

  return 0;
}

ACMD (do_drop)
{
  ObjData *obj, *next_obj;
  sh_int RDR = 0;
  byte mode = SCMD_DROP;
  int dotmode, amount = 0;
  char *sname;

  if (!IS_NPC (ch) && IS_SET_AR (PLR_FLAGS (ch), PLR_HUNTED))
    {
      send_to_char ("Hunted players can't transfer or destroy equipment.\r\n", ch);
      return;
    }

  switch (subcmd)
    {
    case SCMD_JUNK:
      sname = "junk";
      mode = SCMD_JUNK;
      break;
    case SCMD_CAPT:
      sname = "attempt to capture";
      mode = SCMD_CAPT;
      break;
    case SCMD_DONATE:
      sname = "donate";
      mode = SCMD_DONATE;
      switch (number (0, 2))
      {
          case 0:
              //mode = SCMD_JUNK;
              //break;
          case 1:
          case 2:
              RDR = real_room(CONFIG_DON_ROOM_1);
              break;
      }
      if (RDR == NOWHERE)
      {
          send_to_char ("Sorry, you can't donate anything right now.\r\n", ch);
          return;
      }
      break;
    case SCMD_DEST:
      sname = "destroy";
      mode = SCMD_DEST;
      break;
    default:
      sname = "drop";
      break;
    }

  argument = one_argument (argument, arg);

  if (!*arg)
    {
      sprintf (buf, "What do you want to %s?\r\n", sname);
      send_to_char (buf, ch);
      return;
    }
  else if (is_number (arg))
    {
      amount = atoi (arg);
      argument = one_argument (argument, arg);
      if (!str_cmp ("coins", arg) || !str_cmp ("coin", arg))
        perform_drop_gold (ch, amount, mode, RDR);
      else
        {
          /* code to drop multiple items.  anyone want to write it? -je */
          send_to_char ("Sorry, you can't do that to more than one item at a time.\r\n", ch);
        }
      return;
    }
  else
    {
      if (subcmd == SCMD_DEST)
      {
          /* JBP - Junk something in the room -  FIND_OBJ_ROOM */
          CharData *null_ch = NULL;
          ObjData *obj_to_dest = NULL;

          (void) generic_find (arg, FIND_OBJ_ROOM, ch, &null_ch, &obj_to_dest);
          if (obj_to_dest == NULL)
          {
              send_to_char ("You can't find anything to destroy.\r\n", ch);
          }
          // One may destroy her own corpse if it's empty.
          else if (CAN_WEAR(obj_to_dest, ITEM_WEAR_TAKE) ||
                  (!(obj_to_dest->contains) &&
                  (GET_OBJ_VAL(obj_to_dest, 3) == 1) &&
                  (obj_to_dest->ex_description && obj_to_dest->ex_description->keyword &&
                   strcmp(obj_to_dest->ex_description->keyword, GET_NAME(ch)) == 0)))
          {              
              sprintf (buf, "$n calls upon the gods to destroy $p.\r\nA divine flash incinerates $p.");
              act (buf, FALSE, ch, obj_to_dest, 0, TO_CHAR);
              act (buf, TRUE, ch, obj_to_dest, 0, TO_ROOM);
              extract_obj (obj_to_dest);
          }
          else
          {
              send_to_char ("You can't destroy that!\r\n", ch);
          }
          return;
      }

      dotmode = find_all_dots (arg);

      /* Can't junk or donate all */
      if ((dotmode == FIND_ALL) && (subcmd == SCMD_JUNK || subcmd == SCMD_DONATE))
      {
          if (subcmd == SCMD_JUNK)
              send_to_char ("Go to the dump if you want to junk EVERYTHING!\r\n", ch);
          else
              send_to_char ("Go to the donation room if you want to donate EVERYTHING!\r\n", ch);
          return;
      }
      if (dotmode == FIND_ALL)
      {
          if (!ch->carrying)
              send_to_char ("You don't seem to be carrying anything.\r\n", ch);
          else
              for (obj = ch->carrying; obj; obj = next_obj)
              {
                  next_obj = obj->next_content;
                  amount += perform_drop (ch, obj, mode, sname, RDR);
              }
      }
      else if (dotmode == FIND_ALLDOT)
        {
          if (!*arg)
            {
              sprintf (buf, "What do you want to %s all of?\r\n", sname);
              send_to_char (buf, ch);
              return;
            }
          if (!(obj = get_obj_in_list_vis (ch, arg, ch->carrying)))
            {
              sprintf (buf, "You don't seem to have any %ss.\r\n", arg);
              send_to_char (buf, ch);
            }
          while (obj)
            {
              next_obj = get_obj_in_list_vis (ch, arg, obj->next_content);
              amount += perform_drop (ch, obj, mode, sname, RDR);
              obj = next_obj;
            }
        }
      else
        {
          if (!(obj = get_obj_in_list_vis (ch, arg, ch->carrying)))
            {
              sprintf (buf, "You don't seem to have %s %s.\r\n", AN (arg), arg);
              send_to_char (buf, ch);
            }
          else
            amount += perform_drop (ch, obj, mode, sname, RDR);
        }
    }

  if (amount && (subcmd == SCMD_JUNK))
    {
      send_to_char ("You have been rewarded by the gods!\r\n", ch);
      act ("$n has been rewarded by the gods!", TRUE, ch, 0, 0, TO_ROOM);
      GET_GOLD (ch) += amount;
    }
}

void
perform_give (CharData * ch, CharData * vict,
              ObjData * obj)
{
  static char *black = "&07(black)&00";
  static char *gold = "&10(gold)&00";
  static char *rogue = "&09(rogue)&00";
  static char *none = "&03(no team)&00";

  char *gColor = (PRF_FLAGGED (ch, PRF_GOLD_TEAM) ? gold : black);
  char *rColor = (PRF_FLAGGED (vict, PRF_GOLD_TEAM) ? gold : black);
  if (!PRF_FLAGGED (ch, PRF_GOLD_TEAM) &&
      !PRF_FLAGGED (ch, PRF_BLACK_TEAM)) gColor = none;
  if (!PRF_FLAGGED (vict, PRF_GOLD_TEAM) &&
      !PRF_FLAGGED (vict, PRF_BLACK_TEAM)) rColor = none;
  if (PRF_FLAGGED (ch, PRF_ROGUE_TEAM)) gColor = rogue;
  if (PRF_FLAGGED (vict, PRF_ROGUE_TEAM)) rColor = rogue;

  if (IS_OBJ_STAT (obj, ITEM_CURSED))
  {
      act ("You can't let go of $p!!  Yeech!", FALSE, ch, obj, 0, TO_CHAR);
      return;
  }

  if ( (IS_OBJ_STAT(obj, ITEM_SOULBOUND) || contains_soulbound(obj)) && !PRF_FLAGGED(ch, PRF_NOHASSLE )) {
      act("You cannot transfer bound items.", FALSE, ch, obj, 0, TO_CHAR);
      return;
  }

  if (IS_CARRYING_N (vict) >= CAN_CARRY_N (vict))
  {
      act ("$N seems to have $S hands full.", FALSE, ch, 0, vict, TO_CHAR);
      return;
  }

  if (GET_OBJ_WEIGHT (obj) + IS_CARRYING_W (vict) > CAN_CARRY_W (vict))
  {
      act ("$E can't carry that much weight.", FALSE, ch, 0, vict, TO_CHAR);
      return;
  }

  if (!give_otrigger (obj, ch, vict) || !receive_mtrigger (vict, ch, obj))
      return;

  if ((GET_OBJ_VNUM (obj) == GOLD_TEAM_STANDARD) && (GET_LEVEL (ch) <= 51))
  {
      sprintf (buf, "&08FLAG GAME:&00 %s %s has passed the &10Gold Flag&00 to %s %s\r\n", GET_NAME (ch), gColor, GET_NAME (vict), rColor);
      quest_echo (buf);
  }
  if ((GET_OBJ_VNUM (obj) == BLACK_TEAM_STANDARD) && (GET_LEVEL (ch) <= 51))
  {
      sprintf (buf, "&08FLAG GAME:&00 %s %s has passed the &07Black Flag&00 to %s %s\r\n", GET_NAME (ch), gColor, GET_NAME (vict), rColor);
      quest_echo (buf);
  }

  obj_from_char(obj);
  if (!IS_NPC (ch)) Crash_crashsave(ch); /* Close dupe hole - Vex */
  obj_to_char (obj, vict);
  if(!IS_NPC(vict)) Crash_crashsave(vict);

  // Try to fix crash bug...
  char buf1[MAX_OBJ_DESC], buf2[MAX_OBJ_DESC], buf3[MAX_OBJ_DESC];
  sprintf(buf1, "You give %s to $N",  obj->short_description);
  sprintf(buf2, "$n gives you %s.",   obj->short_description);
  sprintf(buf3, "$n gives %s to $N.", obj->short_description);
  
  /* only perform one of these */
  if(!(check_quest_give(ch, vict, obj) ||
          check_fishing_give(ch, vict, obj) ||
          check_scatter_give(ch, vict, obj)) && IS_NPC(vict)
          && !(IS_SET_AR(AFF_FLAGS(vict), AFF_CHARM) && vict->master && vict->master == ch))
  {
      // The player might have given this item on accident.  Maybe it's a valuable item
      // that the player didn't mean to hand over to a nigh-unkillable mob.  We'll
      // give it back if he's a quest master
      act ("$N refuses to accept your $p.", FALSE, ch, obj, vict, TO_CHAR);
      act ("$n tries to give you $p, but you refuse.", FALSE, ch, obj, vict, TO_VICT);

        obj_from_char(obj);
        if (!IS_NPC (vict)) Crash_crashsave(vict); /* Close dupe hole - Vex */
        obj_to_char (obj, ch);
        if(!IS_NPC(ch)) Crash_crashsave(ch);
  }
  else {
        act (buf1, FALSE, ch, NULL, vict, TO_CHAR);
        act (buf2, FALSE, ch, NULL, vict, TO_VICT);
        act (buf3,  TRUE, ch, NULL, vict, TO_NOTVICT);
  }

}

/* utility function for give */
CharData *
give_find_vict (CharData * ch, char *arg)
{
  CharData *vict;

  if (!*arg)
    {
      send_to_char ("To who?\r\n", ch);
      return NULL;
    }
  else if (!(vict = get_char_room_vis (ch, arg)))
    {
      sendChar (ch, CONFIG_NOPERSON);
      return NULL;
    }
  else if (vict == ch)
    {
      send_to_char ("What's the point of that?\r\n", ch);
      return NULL;
    }
  else
    return vict;
}

void
perform_give_gold (CharData * ch, CharData * vict,
                   int amount)
{
  if (amount <= 0)
    {
      send_to_char ("Heh heh heh ... we are jolly funny today, eh?\r\n", ch);
      return;
    }
  if ((GET_GOLD (ch) < amount) && (IS_NPC (ch) || (GET_LEVEL (ch) < LVL_GOD)))
    {
      send_to_char ("You don't have that many coins!\r\n", ch);
      return;
    }
  sendChar (ch, CONFIG_OK);
  sprintf (buf, "$n gives you %d gold coins.", amount);
  act (buf, FALSE, ch, 0, vict, TO_VICT);
  sprintf (buf, "$n gives %s to $N.", money_desc (amount));
  act (buf, TRUE, ch, 0, vict, TO_NOTVICT);
  if (IS_NPC (ch) || (GET_LEVEL (ch) < LVL_GOD))
    GET_GOLD (ch) -= amount;
  GET_GOLD (vict) += amount;

  bribe_mtrigger (vict, ch, amount);
}

ACMD (do_give)
{
  int amount, dotmode;
  CharData *vict;
  ObjData *obj, *next_obj;

  argument = one_argument (argument, arg);

  if (!*arg)
      send_to_char ("Give what to who?\r\n", ch);
  else if (!IS_NPC (ch) && IS_SET_AR (PLR_FLAGS (ch), PLR_HUNTED))
      send_to_char ("Hunted players can't transfer equipment.\r\n", ch);
  else if (is_number (arg))
  {
      amount = atoi (arg);
      argument = one_argument (argument, arg);
      if (!str_cmp ("coins", arg) || !str_cmp ("coin", arg))
      {
          argument = one_argument (argument, arg);
          if ((vict = give_find_vict (ch, arg)))
              perform_give_gold (ch, vict, amount);
          return;
      }
      else
      {
          /* code to give multiple items.  anyone want to write it? -je */
          send_to_char ("You can't give more than one item at a time.\r\n", ch);
          return;
      }
  }
  else
  {
      one_argument (argument, buf1);
      if (!(vict = give_find_vict (ch, buf1)))
          return;
      dotmode = find_all_dots (arg);
      if (dotmode == FIND_INDIV)
      {
          if (!(obj = get_obj_in_list_vis (ch, arg, ch->carrying)))
          {
              sprintf (buf, "You don't seem to have %s %s.\r\n", AN (arg), arg);
              send_to_char (buf, ch);
          }
          else
              perform_give (ch, vict, obj);
      }
      else
      {
          if (dotmode == FIND_ALLDOT && !*arg)
          {
              send_to_char ("All of what?\r\n", ch);
              return;
          }
          if (!ch->carrying)
              send_to_char ("You don't seem to be holding anything.\r\n", ch);
          else
              for (obj = ch->carrying; obj; obj = next_obj)
              {
                  next_obj = obj->next_content;
                  if (CAN_SEE_OBJ (ch, obj) &&
                          ((dotmode == FIND_ALL || isname (arg, obj->name))))
                      perform_give (ch, vict, obj);
              }
      }
  }
}

/* Everything from here down is what was formerly act.obj2.c */


void
weight_change_object (ObjData * obj, int weight)
{
  ObjData *tmp_obj;
  CharData *tmp_ch;

  if (obj->in_room != NOWHERE)
    {
      GET_OBJ_WEIGHT (obj) += weight;
    }
  else if ((tmp_ch = obj->carried_by))
    {
      obj_from_char (obj);
      GET_OBJ_WEIGHT (obj) += weight;
      obj_to_char (obj, tmp_ch);
    }
  else if ((tmp_obj = obj->in_obj))
    {
      obj_from_obj (obj);
      GET_OBJ_WEIGHT (obj) += weight;
      obj_to_obj (obj, tmp_obj);
    }
  else
    {
      mlog ("SYSERR: Unknown attempt to subtract weight from an object.");
      return;
    }
}

void
name_from_drinkcon (ObjData * obj)
{
  int i;
  char *new_name;

  for (i = 0; (*((obj->name) + i) != ' ') && (*((obj->name) + i) != '\0'); i++);

  if (*((obj->name) + i) == ' ')
    {
      new_name = str_dup ((obj->name) + i + 1);
      if (GET_OBJ_RNUM (obj) < 0 || obj->name != obj_proto[GET_OBJ_RNUM (obj)].name)
        free (obj->name);
      obj->name = new_name;
    }
}

void
name_to_drinkcon (ObjData *obj, int type)
{
  char *drinknames[NUM_LIQ_TYPES + 1] = {
    "water", "beer", "wine", "ale", "ale",
    "whisky", "lemonade", "firebreather", "local", "juice",
    "milk", "tea", "coffee", "blood", "salt",
    "water",
    "\n"
  };

  char *new_name;

  CREATE (new_name, char, strlen (obj->name) + strlen (drinknames[type]) + 2);
  sprintf (new_name, "%s %s", drinknames[type], obj->name);
  if (GET_OBJ_RNUM (obj) < 0 || obj->name != obj_proto[GET_OBJ_RNUM (obj)].name)
    free (obj->name);
  obj->name = new_name;
}

ACMD (do_drink)
{
  /*
   ** The drink_aff array was moved into this function because this is the
   ** ONLY place it was ever used. It controls the effect of drinks on hunger,
   ** thirst, and drunkenness -- see values.doc
   */
  static const int drink_aff[NUM_LIQ_TYPES][3] = {
    {0, 1, 10},
    {3, 2, 5},
    {5, 2, 5},
    {2, 2, 5},
    {1, 2, 5},
    {6, 1, 4},
    {0, 1, 8},
    {10, 0, 0},
    {3, 3, 3},
    {0, 4, -8},
    {0, 3, 6},
    {0, 1, 6},
    {0, 1, 6},
    {0, 2, -1},
    {0, 1, -2},
    {0, 0, 13}
  };

  ObjData *temp;
  struct affected_type af;
  int amount, weight;
  int on_ground = 0;

  one_argument (argument, arg);

  if (IS_NPC (ch)) /* Cannot use GET_COND() on mobs. */
    return;

  if (!*arg) {
      send_to_char ("Drink from what?\r\n", ch);
      return;
  }

  if (!(temp = get_obj_in_list_vis (ch, arg, ch->carrying))) {
      if (!(temp = get_obj_in_list_vis (ch, arg, world[ch->in_room].contents))) {
          sendChar (ch, "You can't find it!\r\n");
          return;
      }
      else
          on_ground = 1;
  }
  if ((GET_OBJ_TYPE(temp) != ITEM_DRINKCON) &&
      (GET_OBJ_TYPE(temp) != ITEM_FOUNTAIN)) {
      send_to_char ("You can't drink from that!\r\n", ch);
      return;
  }
  if (on_ground && (GET_OBJ_TYPE (temp) == ITEM_DRINKCON)) {
      send_to_char ("You have to be holding that to drink from it.\r\n", ch);
      return;
  }
  if ((GET_COND(ch, DRUNK) > 20) && (GET_COND(ch, THIRST) > 0)) {
      /* The pig is drunk */
      send_to_char ("You can't seem to get close enough to your mouth.\r\n", ch);
      act ("$n tries to drink but misses $s mouth!", TRUE, ch, 0, 0, TO_ROOM);
      return;
  }
  if ((GET_COND(ch, HUNGER) > 20) && (GET_COND(ch, THIRST) > 20)) {
      sendChar (ch, "Your stomach cannot contain any more!\r\n");
      return;
  }
  if ((GET_OBJ_VAL(temp, 1) == 0) || (!GET_OBJ_VAL(temp, 0) == 1)) {
      sendChar (ch, "It's empty.\r\n");
      return;
  }
  if (IS_VAMPIRE(ch) && (GET_OBJ_VAL (temp, 2) != LIQ_BLOOD)) {
      sendChar (ch, "That would never quench your thirst.\r\n");
      return;
  }

  if (subcmd == SCMD_DRINK) {
      if (GET_OBJ_TYPE(temp) != ITEM_FOUNTAIN) {
          sprintf(buf, "$n drinks %s from $p.", drinks[GET_OBJ_VAL (temp, 2)]);
          act(buf, TRUE, ch, temp, 0, TO_ACTSPAM);
      }
      sendChar(ch, "The %s tastes cool and refreshing.\r\n", drinks[GET_OBJ_VAL (temp, 2)]);

      if (drink_aff[GET_OBJ_VAL(temp, 2)][DRUNK] > 0)
        amount = (25 - GET_COND (ch, THIRST)) / drink_aff[GET_OBJ_VAL (temp, 2)][DRUNK];
      else
        amount = number (3, 10);
  }
  else
  {
      act ("$n sips from $p.", TRUE, ch, temp, 0, TO_ACTSPAM);
      sendChar (ch, "It tastes like %s.\r\n", drinks[GET_OBJ_VAL (temp, 2)]);
      amount = 1;
  }

  amount = MIN(amount, GET_OBJ_VAL (temp, 1));

  /* You can't subtract more than the object weighs, unless its unlimited. */
  if (GET_OBJ_VAL (temp, 0) > 0)
  {
      weight = MIN(amount, GET_OBJ_WEIGHT (temp));
      weight_change_object(temp, -weight); /* Subtract amount */
  }

  gain_condition(ch, DRUNK, drink_aff[GET_OBJ_VAL (temp, 2)][DRUNK]);

  if (IS_VAMPIRE(ch))
    gain_condition(ch, THIRST, drink_aff[GET_OBJ_VAL (temp, 2)][THIRST] * -1 * amount / 4);
  else
    gain_condition(ch, THIRST, drink_aff[GET_OBJ_VAL (temp, 2)][THIRST] * amount / 4);

  if (GET_COND (ch, DRUNK) > 10)
    send_to_char ("You feel drunk.\r\n", ch);

  if (GET_COND (ch, THIRST) > 20) {
      if (IS_VAMPIRE(ch))
          sendChar (ch, "You feel like a fat tick.\r\n");
      else
          sendChar (ch, "You don't feel parched any more.\r\n");
  }
  if (GET_COND (ch, HUNGER) > 20)
      send_to_char ("You are full.\r\n", ch);

  if (GET_OBJ_VAL (temp, 3)) { /* The shit was poisoned ! */
      send_to_char ("Oops, it tasted rather strange!\r\n", ch);
      act ("$n chokes and sputters out some strange sounds.", TRUE, ch, 0, 0, TO_ROOM);

      af.type = SPELL_POISON;
      af.duration = amount * 3 TICKS;
      af.modifier = 0;
      af.location = APPLY_NONE;
      af.bitvector = AFF_POISON;
      affect_join (ch, &af, FALSE, FALSE, FALSE, FALSE);
  }
  /* Empty the container (unless unlimited), and no longer poison. */
  if (GET_OBJ_VAL(temp, 0) > 0) {
      GET_OBJ_VAL(temp, 1) -= amount;
      if (!GET_OBJ_VAL(temp, 1)) { /* The last bit */
          name_from_drinkcon (temp);
          GET_OBJ_VAL (temp, 2) = 0;
          GET_OBJ_VAL (temp, 3) = 0;
      }
  }
  return;
}

/* Trolls can eat almost anything :p */
#define TROLL_CANT_EAT(obj) ( IS_OBJ_STAT(obj, ITEM_MAGIC) || \
		              IS_OBJ_STAT(obj, ITEM_CURSED) || \
			      IS_OBJ_STAT(obj, ITEM_ARTIFACT) )

ACMD (do_eat)
{
  ObjData *food;
  struct affected_type af;
  int amount;

  one_argument (argument, arg);

  if (!*arg)
    {
      sendChar (ch, "Eat what?\r\n");
      return;
    }

  if (!(food = get_obj_in_list_vis (ch, arg, ch->carrying)))
    {
      sendChar (ch, "You don't seem to have %s %s.\r\n", AN (arg), arg);
      return;
    }

  if (subcmd == SCMD_TASTE && ((GET_OBJ_TYPE (food) == ITEM_DRINKCON) ||
                               (GET_OBJ_TYPE (food) == ITEM_FOUNTAIN)))
    {
      do_drink (ch, argument, 0, SCMD_SIP);
      return;
    }

  if ((GET_OBJ_TYPE (food) != ITEM_FOOD) && (GET_LEVEL (ch) < LVL_LRGOD))
  {
      if (!IS_TROLL(ch) || TROLL_CANT_EAT(food))
      {
          sendChar (ch, "You can't eat THAT!\r\n");
          return;
      }

      if(IS_TROLL(ch) && food->contains) {
          sendChar(ch, "You must empty the object of its contents before consuming it.\r\n");
          return;
      }
  }
  
  /* Stomach full */
  if (GET_COND (ch, HUNGER) > 20)
  {
      act ("You are too full to eat more!", FALSE, ch, 0, 0, TO_CHAR);
      return;
  }

  if (subcmd == SCMD_EAT)
  {
      act ("The $o fills the hollow spot in your stomach.", FALSE, ch, food, 0, TO_CHAR);
      /* The TO_ used to be TO_ROOM */
      act ("$n eats $p.", TRUE, ch, food, 0, TO_ACTSPAM);
  }
  else
  {
      act ("You nibble a little bit of the $o.", FALSE, ch, food, 0, TO_CHAR);
      act ("$n tastes a little bit of $p.", TRUE, ch, food, 0, TO_ROOM);
  }

  if (GET_OBJ_TYPE (food) == ITEM_FOOD)
      amount = (subcmd == SCMD_EAT ? GET_OBJ_VAL (food, 0) : 1);
  else
      amount = (subcmd == SCMD_EAT ? GET_OBJ_WEIGHT (food) : 1);

  amount = (amount > 20 ? 20 : amount); /* Clamp the affect */
  // Don't allow players to eat infinite amounts of food for infinite health gain
  WAIT_STATE (ch, PULSE_VIOLENCE);

  gain_condition(ch, HUNGER, amount);

  if (IS_VAMPIRE(ch))
  {
      sendChar (ch, "The food of mortals could never satisfy your desires.\r\n");
  }
  if (GET_COND (ch, HUNGER) > 20)
      act ("Your mighty hunger has been fulfilled.", FALSE, ch, 0, 0, TO_CHAR);
  
  if (GET_OBJ_VAL (food, 3) && (GET_LEVEL (ch) < LVL_IMMORT) && !IS_TROLL(ch))
  {
      /* It was poisoned ! */
      send_to_char ("Oops, that tasted rather strange!\r\n", ch);
      act ("$n coughs and sputters out some strange sounds.", FALSE, ch, 0, 0, TO_ROOM);

      af.type = SPELL_POISON;
      af.duration = amount * 2 TICKS;
      af.modifier = 0;
      af.location = APPLY_NONE;
      af.bitvector = AFF_POISON;
      affect_join (ch, &af, FALSE, FALSE, FALSE, FALSE);
  }
  if (subcmd == SCMD_EAT)
  {
      if (GET_OBJ_TYPE (food) == ITEM_FOOD)
          mag_objectmagic (ch, food, buf); /* Vex -food can have magic stuff on it now. */
      extract_obj (food);
  }
  else
  {
      if (!(--GET_OBJ_VAL (food, 0)))
      {
          send_to_char ("There's nothing left now.\r\n", ch);
          extract_obj(food);
      }
  }
}

ACMD (do_pour)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  ObjData *from_obj;
  ObjData *to_obj;
  int amount;

  two_arguments (argument, arg1, arg2);

  if (subcmd == SCMD_POUR)
    {
      if (!*arg1)
        { /* No arguments */
          act ("What do you want to pour from?", FALSE, ch, 0, 0, TO_CHAR);
          return;
        }
      if (!(from_obj = get_obj_in_list_vis (ch, arg1, ch->carrying)))
        {
          act ("You can't find it!", FALSE, ch, 0, 0, TO_CHAR);
          return;
        }
      if (GET_OBJ_TYPE (from_obj) != ITEM_DRINKCON)
        {
          act ("You can't pour from that!", FALSE, ch, 0, 0, TO_CHAR);
          return;
        }
    }
  if (subcmd == SCMD_FILL)
    {
      if (!*arg1)
        { /* no arguments */
          send_to_char ("What do you want to fill?  And what are you filling it from?\r\n", ch);
          return;
        }
      if (!(to_obj = get_obj_in_list_vis (ch, arg1, ch->carrying)))
        {
          send_to_char ("You can't find it!", ch);
          return;
        }
      if (GET_OBJ_TYPE (to_obj) != ITEM_DRINKCON)
        {
          act ("You can't fill $p!", FALSE, ch, to_obj, 0, TO_CHAR);
          return;
        }
      if (!*arg2)
        { /* no 2nd argument */
          act ("What do you want to fill $p from?", FALSE, ch, to_obj, 0, TO_CHAR);
          return;
        }
      if (!(from_obj = get_obj_in_list_vis (ch, arg2, world[ch->in_room].contents)))
        {
          sprintf (buf, "There doesn't seem to be %s %s here.\r\n", AN (arg2), arg2);
          send_to_char (buf, ch);
          return;
        }
      if (GET_OBJ_TYPE (from_obj) != ITEM_FOUNTAIN)
        {
          act ("You can't fill something from $p.", FALSE, ch, from_obj, 0, TO_CHAR);
          return;
        }
    }
  if (GET_OBJ_VAL (from_obj, 1) == 0)
    {
      act ("The $p is empty.", FALSE, ch, from_obj, 0, TO_CHAR);
      return;
    }
  if (subcmd == SCMD_POUR)
    { /* pour */
      if (!*arg2)
        {
          act ("Where do you want it?  Out or in what?", FALSE, ch, 0, 0, TO_CHAR);
          return;
        }
      if (!str_cmp (arg2, "out"))
        {
          act ("$n empties $p.", TRUE, ch, from_obj, 0, TO_ROOM);
          act ("You empty $p.", FALSE, ch, from_obj, 0, TO_CHAR);

          weight_change_object (from_obj, -GET_OBJ_VAL (from_obj, 1)); /* Empty */

          GET_OBJ_VAL (from_obj, 1) = 0;
          GET_OBJ_VAL (from_obj, 2) = 0;
          GET_OBJ_VAL (from_obj, 3) = 0;
          if (GET_OBJ_WEIGHT (from_obj) < 0) GET_OBJ_WEIGHT (from_obj) = 1;

          name_from_drinkcon (from_obj);

          return;
        }
      if (!(to_obj = get_obj_in_list_vis (ch, arg2, ch->carrying)))
        {
          act ("You can't find it!", FALSE, ch, 0, 0, TO_CHAR);
          return;
        }
      if ((GET_OBJ_TYPE (to_obj) != ITEM_DRINKCON) &&
          (GET_OBJ_TYPE (to_obj) != ITEM_FOUNTAIN))
        {
          act ("You can't pour anything into that.", FALSE, ch, 0, 0, TO_CHAR);
          return;
        }
    }
  if (to_obj == from_obj)
    {
      act ("A most unproductive effort.", FALSE, ch, 0, 0, TO_CHAR);
      return;
    }
  if ((GET_OBJ_VAL (to_obj, 1) != 0) &&
      (GET_OBJ_VAL (to_obj, 2) != GET_OBJ_VAL (from_obj, 2)))
    {
      act ("There is already another liquid in it!", FALSE, ch, 0, 0, TO_CHAR);
      return;
    }
  if (!(GET_OBJ_VAL (to_obj, 1) < GET_OBJ_VAL (to_obj, 0)))
    {
      act ("There is no room for more.", FALSE, ch, 0, 0, TO_CHAR);
      return;
    }
  if (subcmd == SCMD_POUR)
    {
      sprintf (buf, "You pour the %s into the %s.",
               drinks[GET_OBJ_VAL (from_obj, 2)], arg2);
      send_to_char (buf, ch);
    }
  if (subcmd == SCMD_FILL)
    {
      act ("You gently fill $p from $P.", FALSE, ch, to_obj, from_obj, TO_CHAR);
      act ("$n gently fills $p from $P.", TRUE, ch, to_obj, from_obj, TO_ROOM);
    }
  /* New alias */
  if (GET_OBJ_VAL (to_obj, 1) == 0)
    name_to_drinkcon (to_obj, GET_OBJ_VAL (from_obj, 2));

  /* First same type liq. */
  GET_OBJ_VAL (to_obj, 2) = GET_OBJ_VAL (from_obj, 2);

  /* Then how much to pour */
  GET_OBJ_VAL (from_obj, 1) -= (amount =
                                (GET_OBJ_VAL (to_obj, 0) - GET_OBJ_VAL (to_obj, 1)));

  GET_OBJ_VAL (to_obj, 1) = GET_OBJ_VAL (to_obj, 0);

  if (GET_OBJ_VAL (from_obj, 1) < 0)
    { /* There was too little */
      GET_OBJ_VAL (to_obj, 1) += GET_OBJ_VAL (from_obj, 1);
      amount += GET_OBJ_VAL (from_obj, 1);
      GET_OBJ_VAL (from_obj, 1) = 0;
      GET_OBJ_VAL (from_obj, 2) = 0;
      GET_OBJ_VAL (from_obj, 3) = 0;
      name_from_drinkcon (from_obj);
    }
  /* Then the poison boogie */
  GET_OBJ_VAL (to_obj, 3) =
          (GET_OBJ_VAL (to_obj, 3) || GET_OBJ_VAL (from_obj, 3));

  /* And the weight boogie */
  weight_change_object (from_obj, -amount);
  weight_change_object (to_obj, amount); /* Add weight */

  return;
}

void
wear_message (CharData * ch, ObjData * obj, int where)
{
  char *wear_messages[][2] = {
    {"$n lights $p and holds it.",
      "You light $p and hold it."},

    {"$n slides $p on to $s right ring finger.",
      "You slide $p on to your right ring finger."},

    {"$n slides $p on to $s left ring finger.",
      "You slide $p on to your left ring finger."},

    {"$n wears $p as $s cloak.",
      "You start to use $p as your cloak."},

    {"$n wears $p around $s neck.",
      "You wear $p around your neck."},

    {"$n wears $p on $s body.",
      "You wear $p on your body.",},

    {"$n wears $p on $s head.",
      "You wear $p on your head."},

    {"$n wears $p on $s ears.",
      "You wear $p on your ears."},

    {"$n wears $p on $s face.",
      "You wear $p on your face."},

    {"$n puts $p on $s legs.",
      "You put $p on your legs."},

    {"$n puts $p on $s ankles.",
      "You put $p on your ankles."},

    {"$n wears $p on $s feet.",
      "You wear $p on your feet."},

    {"$n puts $p on $s hands.",
      "You put $p on your hands."},

    {"$n wears $p on $s arms.",
      "You wear $p on your arms."},

    {"$n straps $p around $s arm as a shield.",
      "You start to use $p as a shield."},

    {"$n wears $p about $s body.",
      "You wear $p around your body."},

    {"$n wears $p around $s waist.",
      "You wear $p around your waist."},

    {"$n puts $p on around $s right wrist.",
      "You put $p on around your right wrist."},

    {"$n puts $p on around $s left wrist.",
      "You put $p on around your left wrist."},

    {"$n wields $p.",
      "You wield $p."},

    {"$n grabs $p.",
      "You grab $p."},

    {"$n wears $p.",
      "You wear $p."},
  };

  act(wear_messages[where][1], FALSE, ch, obj, 0, TO_CHAR);
}

void
perform_wear (CharData * ch, ObjData * obj, int where)
{
  /* Need to add the extras in here - Vex */
  int wear_bitvectors[] = {
    ITEM_WEAR_TAKE, ITEM_WEAR_FINGER, ITEM_WEAR_FINGER, ITEM_WEAR_CLOAK,
    ITEM_WEAR_NECK, ITEM_WEAR_BODY, ITEM_WEAR_HEAD, ITEM_WEAR_EARS,
    ITEM_WEAR_FACE, ITEM_WEAR_LEGS, ITEM_WEAR_ANKLES, ITEM_WEAR_FEET,
    ITEM_WEAR_HANDS, ITEM_WEAR_ARMS, ITEM_WEAR_SHIELD, ITEM_WEAR_ABOUT,
    ITEM_WEAR_WAIST, ITEM_WEAR_WRIST, ITEM_WEAR_WRIST, ITEM_WEAR_WIELD,
    ITEM_WEAR_HOLD, ITEM_WEAR_ORBIT
  };

  char *already_wearing[] = {
    "You're already using a light.\r\n",
    "YOU SHOULD NEVER SEE THIS MESSAGE.  PLEASE REPORT.\r\n",
    "You're already wearing something on both of your ring fingers.\r\n",
    "You are already wearing something as a cloak.\r\n",
    "You can't wear anything else around your neck.\r\n",
    "You're already wearing something on your body.\r\n",
    "You're already wearing something on your head.\r\n",
    "You're already wearing something on your ears.\r\n",
    "You're already wearing something on your face.\r\n",
    "You're already wearing something on your legs.\r\n",
    "You're already wearing something on your ankles.\r\n",
    "You're already wearing something on your feet.\r\n",
    "You're already wearing something on your hands.\r\n",
    "You're already wearing something on your arms.\r\n",
    "You're already using a shield.\r\n",
    "You're already wearing something about your body.\r\n",
    "You already have something around your waist.\r\n",
    "YOU SHOULD NEVER SEE THIS MESSAGE.  PLEASE REPORT.\r\n",
    "You're already wearing something around both of your wrists.\r\n",
    "You're already wielding a weapon.\r\n",
    "You're already holding something.\r\n",
    "Nothing else can orbit your head.\r\n",
  };

  /* If its a mob and its charmed don't let it wear eq */
  /* Imhotep: fixed this so the code agrees with the comment */
  if ((IS_NPC (ch) && ch->master != NULL))
    return;

  /* first, make sure that the wear position is valid. */
  if (!CAN_WEAR (obj, wear_bitvectors[where]))
  {
      act ("You can't wear $p there.", FALSE, ch, obj, 0, TO_CHAR);
      return;
  }
  /* for finger, and wrist, try pos 2 if pos 1 is already full */
  if ((where == WEAR_FINGER_R) || (where == WEAR_WRIST_R))
    if (ch->equipment[where])
      where++;

  if (ch->equipment[where])
  {
      send_to_char(already_wearing[where], ch);
      return;
  }

  if (!wear_otrigger(obj, ch, where))
      return;
  wear_message(ch, obj, where);
  obj_from_char(obj);
  equip_char(ch, obj, where);
}

int
find_eq_pos (CharData * ch, ObjData * obj, char *arg)
{
  int where = -1;

  static const char *keywords[] = {
    "!RESERVED!",
    "finger",
    "!RESERVED!",
    "cloak",
    "neck",
    "body",
    "head",
    "ears",
    "face",
    "legs",
    "ankles",
    "feet",
    "hands",
    "arms",
    "shield",
    "about",
    "waist",
    "wrist",
    "!RESERVED!",
    "!RESERVED!",
    "!RESERVED!",
    "orbit",
    "\n"
  };

  if (!arg || !*arg)
    {
      if (CAN_WEAR (obj, ITEM_WEAR_FINGER))
        where = WEAR_FINGER_R;
      if (CAN_WEAR (obj, ITEM_WEAR_CLOAK))
        where = WEAR_CLOAK;
      if (CAN_WEAR (obj, ITEM_WEAR_NECK))
        where = WEAR_NECK;
      if (CAN_WEAR (obj, ITEM_WEAR_BODY))
        where = WEAR_BODY;
      if (CAN_WEAR (obj, ITEM_WEAR_HEAD))
        where = WEAR_HEAD;
      if (CAN_WEAR (obj, ITEM_WEAR_LEGS))
        where = WEAR_LEGS;
      if (CAN_WEAR (obj, ITEM_WEAR_FEET))
        where = WEAR_FEET;
      if (CAN_WEAR (obj, ITEM_WEAR_HANDS))
        where = WEAR_HANDS;
      if (CAN_WEAR (obj, ITEM_WEAR_ARMS))
        where = WEAR_ARMS;
      if (CAN_WEAR (obj, ITEM_WEAR_SHIELD))
        where = WEAR_SHIELD;
      if (CAN_WEAR (obj, ITEM_WEAR_ABOUT))
        where = WEAR_ABOUT;
      if (CAN_WEAR (obj, ITEM_WEAR_WAIST))
        where = WEAR_WAIST;
      if (CAN_WEAR (obj, ITEM_WEAR_WRIST))
        where = WEAR_WRIST_R;
      if (CAN_WEAR (obj, ITEM_WEAR_ORBIT))
        where = WEAR_ORBIT;
      if (CAN_WEAR (obj, ITEM_WEAR_EARS))
        where = WEAR_EARS;
      if (CAN_WEAR (obj, ITEM_WEAR_FACE))
        where = WEAR_FACE;
      if (CAN_WEAR (obj, ITEM_WEAR_ANKLES))
        where = WEAR_ANKLES;
    }
  else
    {
      if (!isalpha (arg[0]))
        {
          sprintf (buf, "'%s'?  What part of your body is THAT?\r\n", arg);
          send_to_char (buf, ch);
        }
      else if ((where = search_block (arg, keywords, FALSE)) < 0)
        {
          sprintf (buf, "'%s'?  What part of your body is THAT?\r\n", arg);
          send_to_char (buf, ch);
        }
    }

  return where;
}

ACMD (do_wear)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  ObjData *obj, *next_obj;
  int where, dotmode, items_worn = 0;

  two_arguments (argument, arg1, arg2);

  if (!*arg1)
    {
      send_to_char ("Wear what?\r\n", ch);
      return;
    }
  dotmode = find_all_dots (arg1);

  if (IS_NPC (ch) && ch->master)
    return;


  if (*arg2 && (dotmode != FIND_INDIV))
    {
      send_to_char ("You can't specify the same body location for more than one item!\r\n", ch);
      return;
    }
  if (dotmode == FIND_ALL)
    {
      for (obj = ch->carrying; obj; obj = next_obj)
        {
          next_obj = obj->next_content;
          if (CAN_SEE_OBJ (ch, obj) && (where = find_eq_pos (ch, obj, 0)) >= 0)
            {
              items_worn++;
              perform_wear (ch, obj, where);
            }
        }
      if (!items_worn)
        send_to_char ("You don't seem to have anything wearable.\r\n", ch);
    }
  else if (dotmode == FIND_ALLDOT)
    {
      if (!*arg1)
        {
          send_to_char ("Wear all of what?\r\n", ch);
          return;
        }
      if (!(obj = get_obj_in_list_vis (ch, arg1, ch->carrying)))
        {
          sprintf (buf, "You don't seem to have any %ss.\r\n", arg1);
          send_to_char (buf, ch);
        }
      else
        while (obj)
          {
            next_obj = get_obj_in_list_vis (ch, arg1, obj->next_content);
            if ((where = find_eq_pos (ch, obj, 0)) >= 0)
              perform_wear (ch, obj, where);
            else
              act ("You can't wear $p.", FALSE, ch, obj, 0, TO_CHAR);
            obj = next_obj;
          }
    }
  else
    {
      if (!(obj = get_obj_in_list_vis (ch, arg1, ch->carrying)))
        {
          sprintf (buf, "You don't seem to have %s %s.\r\n", AN (arg1), arg1);
          send_to_char (buf, ch);
        }
      else
        {
          if ((where = find_eq_pos (ch, obj, arg2)) >= 0)
            perform_wear (ch, obj, where);
          else if (!*arg2)
            act ("You can't wear $p.", FALSE, ch, obj, 0, TO_CHAR);
        }
    }
}

ACMD (do_wield)
{
  ObjData *obj;

  one_argument (argument, arg);

  if (!*arg)
    send_to_char ("Wield what?\r\n", ch);
  else if (!(obj = get_obj_in_list_vis (ch, arg, ch->carrying)))
    {
      sprintf (buf, "You don't seem to have %s %s.\r\n", AN (arg), arg);
      send_to_char (buf, ch);
    }
  else
    {
      if (!CAN_WEAR (obj, ITEM_WEAR_WIELD))
        send_to_char ("You can't wield that.\r\n", ch);
      else if (GET_OBJ_WEIGHT (obj) > str_app[STRENGTH_APPLY_INDEX (ch)].wield_w)
        send_to_char ("It's too heavy for you to use.\r\n", ch);
      else
        perform_wear (ch, obj, WEAR_WIELD);
    }
}

ACMD (do_grab)
{
  ObjData *obj;

  one_argument (argument, arg);

  if (!*arg)
    send_to_char ("Hold what?\r\n", ch);
  else if (!(obj = get_obj_in_list_vis (ch, arg, ch->carrying)))
    {
      sprintf (buf, "You don't seem to have %s %s.\r\n", AN (arg), arg);
      send_to_char (buf, ch);
    }
  else
    {
      if (GET_OBJ_TYPE (obj) == ITEM_LIGHT)
        {
          if (ch->equipment[WEAR_HOLD])
            send_to_char ("You are already holding something in your hand.\r\n", ch);
          else
            perform_wear (ch, obj, WEAR_LIGHT);
        }
      else
        {
          if (!CAN_WEAR (obj, ITEM_WEAR_HOLD) && GET_OBJ_TYPE (obj) != ITEM_WAND &&
              GET_OBJ_TYPE (obj) != ITEM_STAFF && GET_OBJ_TYPE (obj) != ITEM_SCROLL &&
              GET_OBJ_TYPE (obj) != ITEM_POTION)
            send_to_char ("You can't hold that.\r\n", ch);
          else
            {
              if (ch->equipment[WEAR_LIGHT])
                send_to_char ("You are already holding something in your hand.\r\n", ch);
              else
                perform_wear (ch, obj, WEAR_HOLD);
            }
        }
    }
}

void
perform_remove (CharData * ch, int pos)
{
  ObjData *obj;

  if (!(obj = ch->equipment[pos]))
    {
      mlog ("Error in perform_remove: bad pos passed.");
      return;
    }

  if (IS_OBJ_STAT (obj, ITEM_CURSED))
  {
      act ("You can't it must be CURSED!", FALSE, ch, obj, 0, TO_CHAR);
      return;
  }

  if (IS_CARRYING_N (ch) >= CAN_CARRY_N (ch))
    act ("$p: you can't carry that many items!", FALSE, ch, obj, 0, TO_CHAR);
  else
    {
      if (!remove_otrigger (obj, ch))
        return;
      ch->equipment[pos]->worn_at = -1;
      obj_to_char (unequip_char (ch, pos), ch);
      act ("You stop using $p.", FALSE, ch, obj, 0, TO_CHAR);

      /* Mortius : If the player removes a perm affect item and its fly and he's
                   in a fall room, lets make em fall */

      if (IS_SET_AR (GET_OBJ_AFFECT (obj), AFF_FLY) &&
          IS_SET_AR (ROOM_FLAGS (IN_ROOM (ch)), ROOM_FALL))
        doFallRoom (ch);
    }
}

ACMD (do_remove)
{
  ObjData *obj;
  int dotmode;

  one_argument (argument, arg);

  if (!*arg)
    {
      sendChar (ch, "Remove what?\r\n");
      return;
    }

  dotmode = find_all_dots (arg);

  if (dotmode == FIND_ALL)
    {
      int i, found = 0;
      for (i = 0; i < NUM_WEARS; i++)
        if (ch->equipment[i])
          {
            perform_remove (ch, i);
            found = 1;
          }
      if (!found)
        send_to_char ("You're not using anything.\r\n", ch);
    }

  else if (dotmode == FIND_ALLDOT)
    {

      if (!*arg) sendChar (ch, "Remove all of what?\r\n");

      else
        {
          int i, found = 0;
          for (i = 0; i < NUM_WEARS; i++)
            if (ch->equipment[i] && CAN_SEE_OBJ (ch, ch->equipment[i]) &&
                isname (arg, ch->equipment[i]->name))
              {
                perform_remove (ch, i);
                found = 1;
              }
          if (!found) sendChar (ch, "You don't seem to be using any %ss.\r\n", arg);
        }
    }

  else
    {
      int i;
      if (!(obj = get_object_in_equip_vis (ch, arg, ch->equipment, &i)))
        sendChar (ch, "You don't seem to be using %s %s.\r\n", AN (arg), arg);

      else perform_remove (ch, i);
    }
}
