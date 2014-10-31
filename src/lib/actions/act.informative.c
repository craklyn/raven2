/**************************************************************************
*  File: act.informative.c                             	Part of CircleMUD *
*  Usage: Player-level commands of an informative nature                  *
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
#include "general/class.h"
#include "actions/act.clan.h"
#include "general/color.h"
#include "general/comm.h"
#include "general/handler.h"
#include "actions/interpreter.h"
#include "magic/spells.h"
#include "util/utils.h"
#include "actions/fight.h"
#include "actions/act.h"          /* ACMDs located within the act*.c files */
#include "util/weather.h"
#include "scripts/dg_scripts.h"
#include "specials/mail.h"        /**< For the has_mail function */
#include "magic/sing.h"
#include "olc/oedit.h"
#include "magic/skills.h"
#include "specials/combspec.h"
#include "specials/portal.h"      /* For look_in_portal function */
#include "actions/quest.h"

/* prototypes of local functions */
/* do_look and do_examine utility functions */
static void look_in_direction(CharData * ch, int dir);
static void look_in_obj(CharData * ch, char *arg);
/* do_where utility functions */
static void perform_immort_where(CharData *ch, char *arg);
static void perform_mortal_where(CharData *ch, char *arg);
static void print_object_location(int num, ObjData * obj, CharData * ch, int recur);
int hasQuest(CharData *i, CharData *ch);

/* Subcommands */
/* For show_obj_to_char 'mode'.	/-- arbitrary */
#define SHOW_OBJ_LONG     0
#define SHOW_OBJ_SHORT    1
#define SHOW_OBJ_ACTION   2

#define SHOWCONN(d) (( !d->connected && d->original ) ? \
                       "Switched" : connected_types[d->connected] )

void
show_obj_to_char( ObjData * object,
                  CharData* ch,
                  int mode )
{
  bool found;
  int i, done = 1, wanted = 0, prize = -1;

  *buf = '\0';
  if ((mode == 0) && object->description)
    strcpy(buf, object->description);

  else if (object->short_description && ((mode >= 1) && (mode <= 4)))
    strcpy(buf, object->short_description);

  else if (mode == 5)
  {
    if (GET_OBJ_TYPE(object) == ITEM_NOTE)
    {
      if (object->action_description)
      {
	strcpy(buf, "There is something written upon it:\r\n\r\n");
	strcat(buf, object->action_description);
	page_string(ch->desc, buf, 1);
      }
      else
	act("It's blank.", FALSE, ch, 0, 0, TO_CHAR);
      return;
    }
    else if (GET_OBJ_TYPE(object) != ITEM_DRINKCON)
    {
      strcpy(buf, "You see nothing special..");
    }
    else    /* ITEM_TYPE == ITEM_DRINKCON||FOUNTAIN */
      strcpy(buf, "It looks like a drink container.");
  }

  if (mode != 3)
  {
    found = FALSE;


    // This ridiculous peice of code is used to check if an item is wanted by a questmaster.  If so,
    // the object is marked with a (*) bullseye.
    if (!IS_NPC(ch) && GET_QUEST(ch) != NULL && GET_QUEST_TIME(ch) >= 0)
        for (i = 0; i < MAX_QST_TASKS; i++) {
            if ((GET_QUEST_TASKS(ch) & (1 << i)) == 0) { // find incomplete tasks
                if (wanted && GET_QUEST(ch)->tasks[i].type != QST_TASK_NONE) {
                    break;
                }
                switch (GET_QUEST(ch)->tasks[i].type) {
                    case QST_TASK_OBJ:
                        if (GET_OBJ_VNUM(object) == GET_QUEST(ch)->tasks[i].identifier) {
                            wanted = 1;
                        }
                        break;
                    case QST_TASK_MOB:              // unimped!
                        if (GET_OBJ_VNUM(object) == QUEST_KILL_TOKEN) {
                            // make sure this is the right token
                            if (GET_OBJ_RENT(object) == GET_QUEST(ch)->tasks[i].identifier) {
                                wanted = 1;
                            }
                        }
                    default:
                        break;
                }
            }
        }
    if(wanted) {
        strcat(buf, " (&08*&00)");
        found = TRUE;
    }

    if (IS_OBJ_STAT(object, ITEM_INVISIBLE))
    {
      strcat(buf, " (invisible)");
      found = TRUE;
    }
    if (IS_OBJ_STAT(object, ITEM_BLESS) && IS_AFFECTED(ch, AFF_DETECT_ALIGN))
    {
      strcat(buf, "(blue aura)");
      found = TRUE;
    }
    if (IS_OBJ_STAT(object, ITEM_IDENTIFIED) && IS_AFFECTED(ch, AFF_DETECT_MAGIC))
    {
      strcat(buf, " (divining aura)");
      found = TRUE;
    }
    if (IS_OBJ_STAT(object, ITEM_MAGIC) && IS_AFFECTED(ch, AFF_DETECT_MAGIC))
    {
#ifdef SHOW_MAGIC
		strcat(buf, "(yellow aura)");
#endif
      found = TRUE;
    }

    if (IS_OBJ_STAT(object, ITEM_GLOW))
    {
#ifdef SHOW_GLOW
      strcat(buf, "(glowing)");
#endif
      found = TRUE;
    }
    if (IS_OBJ_STAT(object, ITEM_HUM)) {
#ifdef SHOW_HUM
      strcat(buf, "(humming)");
#endif
      found = TRUE;
    }
  }
  strcat(buf, "\r\n");
  page_string(ch->desc, buf, 1);
}

/* Mortius : This function has been changed so players won't see traps that
	     are assigned to exits unless they have been detected.  Immortals
	     of course will see them. */
/* Imhotep : This function has been changed to remove Mortius' trap
             stuff in favour of a trap flag on containers. */
/* Sanji : This function has been changed to deal with
             drow sunblind affects. */

void
list_obj_to_char( ObjData * list,
                  CharData* ch,
                  int mode,
		  bool show)
{
  bool found = FALSE;
  ObjData *i;

  for (i = list; i; i = i->next_content)
  {
    if (CAN_SEE_OBJ(ch, i)) {
      if (IS_SUNBLIND(ch)) {
        /* chance to miss the object */
        if(number(1, 100) > GET_LEVEL(ch) + 50) continue;
      }
      show_obj_to_char(i, ch, mode);
      found = TRUE;
    }
  }

  if (!found && show)
    send_to_char(" Nothing.\r\n", ch);
}


void
diag_char_to_char( CharData * i,
                   CharData * ch )
{
  int percent;

  if (GET_MAX_HIT(i) > 0)
    percent = (100 * GET_HIT(i)) / GET_MAX_HIT(i);
  else
    percent = -1; /* How could MAX_HIT be < 1?? */

  strcpy(buf, PERS(i, ch));
  CAP(buf);

  if (percent >= 100)
    strcat(buf, " is in &06excellent condition&00.\r\n");
  else if (percent >= 90)
    strcat(buf, " has a &06few scratches&00.\r\n");
  else if (percent >= 75)
    strcat(buf, " has some &06small wounds and bruises&00.\r\n");
  else if (percent >= 50)
    strcat(buf, " has &06quite a few wounds&00.\r\n");
  else if (percent >= 30)
    strcat(buf, " has some &06big nasty wounds and scratches&00.\r\n");
  else if (percent >= 15)
    strcat(buf, " looks &06pretty hurt&00.\r\n");
  else if (percent >= 0)
    strcat(buf, " is in &06awful condition&00.\r\n");
  else
    strcat(buf, " is &06bleeding awfully from big wounds&00.\r\n");

  send_to_char(buf, ch);
}


void look_at_char(CharData * i, CharData * ch)
{
  int j     = 0;
  int found = FALSE;
  ObjData *tmp_obj;

  if (i->player.description)
    send_to_char(i->player.description, ch);
  else
    act("You see nothing special about $m.", FALSE, i, 0, ch, TO_VICT);

  diag_char_to_char(i, ch);

  for (j = 0; !found && j < NUM_WEARS; j++)
  {
    if (i->equipment[j] && CAN_SEE_OBJ(ch, i->equipment[j]))
      found = TRUE;
  }

  if (found)
  {
    act("\r\n$n is using:", FALSE, i, 0, ch, TO_VICT);
    for (j = 0; j < NUM_WEARS; j++) {
        if (i->equipment[j] && CAN_SEE_OBJ(ch, i->equipment[j]))
        {
            /* Differentiate between some races just for fun. */
            if (IS_MINOTAUR(i) && (j == WEAR_FEET))
                send_to_char("<worn on hooves>     ", ch);
            else if ((IS_DRACONIAN(i) || IS_DRAGON(i))
                    && ((j == WEAR_FINGER_R) || (j == WEAR_FINGER_L)))
                send_to_char("<worn on claw>       ", ch);
            else if ((j == WEAR_WIELD) && affected_by_spell(i, SKILL_DISARM))
                send_to_char("<fumbling>           ", ch);
            else
                sendChar(ch, "%s", wear_where[j]);

            show_obj_to_char(i->equipment[j], ch, 1);
        }
    }
  }

  if (ch != i && (GET_CLASS(ch) == CLASS_THIEF || GET_LEVEL(ch) >= LVL_IMMORT))
  {
    found = FALSE;
    act("\r\nYou attempt to discern $s possessions:", FALSE, i, 0, ch, TO_VICT);
    for (tmp_obj = i->carrying; tmp_obj; tmp_obj = tmp_obj->next_content)
    {
      if (CAN_SEE_OBJ(ch, tmp_obj) && (number(0, 20) < GET_LEVEL(ch)))
      {
	show_obj_to_char(tmp_obj, ch, 1);
	found = TRUE;
      }
    }

    if (!found)
      act("Your victim conceals $s possessions too well.\r\n", FALSE, i, 0, ch, TO_VICT);
  }

  /* christmas mistletoe */
#define ORBITAL (i->equipment[WEAR_ORBIT])
  if (ORBITAL && i != ch && (GET_OBJ_VNUM(ORBITAL) == 33197 ||
              GET_OBJ_VNUM(ORBITAL) == 33198 ||
              GET_OBJ_VNUM(ORBITAL) == 33199)) {
      act("\r\nYou smooch $m.  Hope $e liked it!", FALSE, i, 0, ch, TO_VICT);
      act("\r\n$N smooches $n on the cheek.", FALSE, i, 0, ch, TO_NOTVICT);
      act("\r\n$N smooches you.  *SMOOOCH*", FALSE, i, 0, ch, TO_CHAR);
  }

#undef ORBITAL
}


static void
loadCharPosition(CharData *ch, CharData *vict,
                  char* posStr )
{
  static const char *standard[] = {
    " is lying here, dead.",          " is lying here, mortally wounded.",
    " is lying here, incapacitated.", " is lying here, stunned.",
    " is sleeping here.",             " is meditating here.",
    " is resting here.",              " is sitting here.",
    "!FIGHTING!",                     " is standing here."
  };

  static const char *flying[] = {
    " is floating here, dead.",          " is floating here, mortally wounded.",
    " is floating here, incapacitated.", " is floating here, stunned.",
    " is sleeping here, in mid-air.",    " is meditating here, inches above the ground.",
    " is resting on a cushion of air.",  " is sitting on a cushion of air.",
    "!FIGHTING!",                        " is floating here."
  };

  static const char *underwater[] = {
   " is floating here underwater, dead.",
   " is floating here underwater, mortally wounded.",
   " is floating here underwater, incapacitated.",
   " is floating here underwater, stunned.",
   " is sleeping here while floating underwater.",
   " is meditating here on a coral rock.",
   " is resting here on a coral rock.",
   " is sitting here on a coral rock.",
   "!FIGHTING!",
   " is swimming here, underwater."
  };


#if 0
  static const char *mounted[] = {
    " is lying here, dead.",          " is lying here, mortally wounded.",
    " is lying here, incapacitated.", " is lying here, stunned.",
    " is sleeping here.",             "is meditating here.",
    " is resting here.",              " is sitting here.",
    "!FIGHTING!",                     " is standing here."
  };
#endif

  if( GET_POS(vict) != POS_FIGHTING )
  {
    if(UNDERWATER(vict))
       strcat(posStr, underwater[(int) GET_POS(vict)]);
    else if(vict->mount != NULL)
      sprintf( posStr, "%s is here riding %s.", posStr, GET_NAME(vict->mount));
    else if( IS_AFFECTED(vict, AFF_FLY))
      strcat(posStr, flying[(int) GET_POS(vict)]);
    else
      strcat(posStr, standard[(int) GET_POS(vict)]);
  }
  else
  {
    if(FIGHTING(vict))
    {
      strcat(posStr, " is here, fighting ");
      if (FIGHTING(vict) == ch)
	strcat(posStr, "YOU!");
      else
      {
	if (vict->in_room == FIGHTING(vict)->in_room)
	  strcat(posStr, PERS(FIGHTING(vict), ch));
	else
	  strcat(posStr, "someone who has already left");
	strcat(posStr, "!");
      }
    }
    else /* NULL pointer */
      strcat(posStr, " is here struggling with thin air.");
  }
}

int hasQuest(CharData *mob, CharData *ch) {
    if(!IS_NPC(mob))
        return FALSE;

    int i, q = -1, r = 0;

    // The player must wait for his or her current quest to end
    if(GET_QUEST_WAIT(ch) - time(NULL) > 0)
        return FALSE;

    // pick a random quest offerable by this mob
    for (i = 0; i <= top_of_qstt; i++) {
        if (can_take_quest(ch, qst_list + i, mob) && number(1, ++r) == 1) q = i;
    }
        
    // q is -1 if there is no available quest
    return (q != -1);
}


void
list_one_char( CharData * i,
               CharData * ch)
{
  int j;

  /*
  ** If the mob has a rider then simply return and show the mount
  ** and its rider when displaying the rider unless it's your mount.
  */
  if((i->rider) && (i->rider != ch)) return;

  if (IS_NPC(i) && i->player.long_descr && GET_POS(i) == GET_DEFAULT_POS(i))
  {
    if (IS_AFFECTED(i, AFF_INVISIBLE))
      strcpy(buf, "*");
    else
      *buf = '\0';

    if (IS_AFFECTED(ch, AFF_DETECT_ALIGN))
    {
      if (IS_EVIL(i))
	strcat(buf, "(Red Aura) ");
      else if (IS_GOOD(i))
	strcat(buf, "(Blue Aura) ");
    }

    if (affected_by_spell(i, SPELL_NOXIOUS_SKIN) &&
        affected_by_spell(i, SPELL_CHARM_CORPSE) &&
            i->master == ch)
        strcat(buf, "(Rotting) ");

    if (IS_AFFECTED(i, AFF_SANCTUARY))
      strcat(buf, "(Glowing) ");

    if (IS_AFFECTED(i, AFF_SHADOW_SPHERE) && IS_AFFECTED(ch, AFF_INFRAVISION))
      strcat(buf, "(Dark Aura) ");

    if( i->rider == ch )
    {
      strcat( buf, "Your mount, " );
    }

    // Alert the player that the mob has a quest to offer
    if(hasQuest(i, ch))
        strcat(buf, "&00&03(&10!&00&03) ");

    // If the player is tasked to kill the mob, mark the mob.
    if(!IS_NPC(ch) && IS_NPC(i) && GET_QUEST(ch) != NULL && GET_QUEST_TIME(ch) >= 0)
        for (j = 0; j < MAX_QST_TASKS; j++) {
            if (GET_QUEST(ch)->tasks[j].type == QST_TASK_MOB &&
                    GET_QUEST(ch)->tasks[j].identifier == GET_MOB_VNUM(i)) {
                strcat(buf, "&00&03(&08*&00&03) ");
                break;
            }
        }

    strcat(buf, i->player.long_descr);
    send_to_char(buf, ch);

    if (IS_AFFECTED(i, AFF_BLIND))
      act("$e is groping around blindly!", FALSE, i, 0, ch, TO_VICT);

    return;
  }

  if (IS_NPC(i))
  {
    strcpy(buf, i->player.short_descr);
    CAP(buf);
  }
  else
    sprintf(buf, "%s %s&00", i->player.name, GET_TITLE(i));

  if (IS_AFFECTED(i, AFF_INVISIBLE))
    strcat(buf, " (invisible)");
  if (IS_AFFECTED(i, AFF_HIDE))
    strcat(buf, " (hidden)");
  if (PRF_FLAGGED(i, PRF_GOLD_TEAM))
    strcat(buf, " &03(Gold Team)&00");
  if (PRF_FLAGGED(i, PRF_BLACK_TEAM))
    strcat(buf, " &05(Black Team)&00");
  if (PRF_FLAGGED(i, PRF_ROGUE_TEAM))
    strcat(buf, "&08(Rogue Team)&00");
  if (!IS_NPC(i) && IS_AFK(i))
    strcat(buf, " (AFK)");
   if (!IS_NPC(i) && !i->desc)
    strcat(buf, " (linkless)");
  if (PLR_FLAGGED(i, PLR_WRITING))
    strcat(buf, " (writing)");

/*
** Load the current position string.
*/
  loadCharPosition(ch, i, buf);

  if (IS_AFFECTED(ch, AFF_DETECT_ALIGN)) {
    if (IS_EVIL(i))
      strcat(buf, " (Red Aura)");
    else if (IS_GOOD(i))
      strcat(buf, " (Blue Aura)");
  }
  if (affected_by_spell(i, SPELL_NOXIOUS_SKIN) &&
          affected_by_spell(i, SPELL_CHARM_CORPSE) &&
          i->master == ch)
      strcat(buf, "(Rotting) ");

  if (IS_AFFECTED(i, AFF_SANCTUARY))
    strcat(buf, "(Glowing) ");

  if (IS_AFFECTED(i, AFF_SHADOW_SPHERE) && IS_AFFECTED(ch, AFF_INFRAVISION))
    strcat(buf, "(Dark Aura) ");

  if( hasQuest(i, ch))
      strcat(buf, "(&10!&00&03) ");

  strcat(buf, "&00\r\n");
  send_to_char(buf, ch);

}

void
list_char_to_char( CharData * list,
                   CharData * ch )
{
  CharData *i, *tmp_ch;
  int hiddenCount = 0;
  int shadowCount = 0;

  if( ROOM_FLAGGED(ch->in_room, ROOM_FOG ))
  {
    sendChar( ch, "The dense fog here makes it impossible to see very far.\r\n" );
    return;
  }

//  for( i = list; i; i = i->next_in_room )
  for(tmp_ch = list; tmp_ch; tmp_ch = tmp_ch->next_in_room)
  {
   i = tmp_ch;
    if( ch != i )
    {
      /*
      ** Detection of hiddens here.
      */
      if( CAN_SEE(ch, i) )
      {
        if( IS_AFFECTED( ch, AFF_SENSE_LIFE ) && IS_AFFECTED( i, AFF_HIDE ))
           hiddenCount++;
        else {
            if (IS_SUNBLIND(ch)) {
                /* chance to miss the object */
                if(number(1, 100) > GET_LEVEL(ch) + 50) continue;
            }
	   list_one_char(i, ch);
        }
      }
      else if (!SHADOW_OK(ch, i))
	shadowCount++;
      else if (IS_DARK(ch->in_room) && !CAN_SEE_IN_DARK(ch) &&
	       IS_AFFECTED(i, AFF_INFRAVISION))
	sendChar( ch, "Two fiery orbs flicker in and out of view.\r\n" );
    } /* if */
  }  /* for */

  if( hiddenCount > 0 )
    sendChar( ch, "You sense %s.\r\n",
            (hiddenCount == 1 ? "a hidden lifeform" : "hidden lifeforms"));

  if ( shadowCount > 0)
    sendChar( ch, "You see %s.\r\n",
            (shadowCount == 1 ? "a globe of darkness" : "a mass of darkness"));
}


void do_auto_exits(CharData * ch)
{
  static char *exits[] = { "North",
                           "East",
                           "South",
                           "West",
                           "Up",
                           "Down" };
  int door;
  char *minoExitColors[] = { "&03", "&04", "&05", "&07", "&02", "&00" };

  *buf = '\0';

  if( ROOM_FLAGGED( ch->in_room, ROOM_FOG ))
  {
    /*
    ** Special stuff for fog handling needs to go in - this
    ** is simply a start.      3/31/98 - Digger
    */
    if(IS_DWARF(ch) || GET_CLASS(ch) == CLASS_RANGER)
    {
    }
    else
      return;
  }

  for( door = 0; door < NUM_OF_DIRS; door++ )
  {

    if( EXIT(ch, door) &&                              /* If it's a door             */
        EXIT(ch, door)->to_room != NOWHERE &&          /* ... and it leads somewhere */
        !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED)) /* ... and it's open          */
    {
        
      // earlierExits is any exit earlier in the list which takes the player to
      // the same room.  Otherwise, it is -1
      int earlierExits = -1, laterExits = -1;
      int i = 0;
      for(i = 0; i < door; i++) {
          if(EXIT(ch, door) && EXIT(ch, i) && EXIT(ch, door)->to_room == EXIT(ch, i)->to_room) {
              earlierExits = i;
              break;
          }
      }
      
      for(i = door + 1; i < NUM_OF_DIRS; i++) {
          if(EXIT(ch, door) && EXIT(ch, i) && EXIT(ch, door)->to_room == EXIT(ch, i)->to_room)
          {
              laterExits = door;
              break;
          }
      }
      
      /* Lets show underwater rooms as blue */
      if (SECT(EXIT(ch, door)->to_room) == SECT_UNDERWATER)
          sprintf(buf, "%s &11-%s-&00", buf, exits[door]);
      else if(GET_RACE(ch) == RACE_SMINOTAUR && EXIT(ch, door)->to_room == ch->in_room)
          sprintf(buf, "%s &05|%s|&00", buf, exits[door]);
      else if(GET_RACE(ch) == RACE_SMINOTAUR && earlierExits >= 0)
          sprintf(buf, "%s %s%s&00", buf, minoExitColors[earlierExits], exits[door]);
      else if(GET_RACE(ch) == RACE_SMINOTAUR && laterExits >= 0)
          sprintf(buf, "%s %s%s&00", buf, minoExitColors[laterExits], exits[door]);
      else
          sprintf(buf, "%s &06%s&00", buf, exits[door]);

    }
    /* Let the dwarves see the doors, and also with the Drow.  -Kaidon :: -Sanji */
    else if( EXIT(ch, door) &&
             EXIT(ch, door)->to_room != NOWHERE &&
             IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) &&
            (IS_DWARF(ch) ||
            ((IS_DROW(ch)) && !IS_SUNBLIND(ch)) ||
            (GET_LEVEL(ch) >= LVL_IMMORT))) {
       if (SECT(EXIT(ch, door)->to_room) == SECT_UNDERWATER)
           sprintf(buf, "%s &11(%s)&00", buf, exits[door]);
       else
           sprintf(buf, "%s &06(%s)&00", buf, exits[door]);
    }
  }

  sprintf(buf2, "%sObvious Exits : %s%s\r\n\n", CCCYN(ch, C_NRM), *buf ? buf : "None! ", CCNRM(ch, C_NRM));
  send_to_char(buf2, ch);
}


ACMD(do_exits)
{
  int door;

  *buf  = '\0';
  *buf2 = '\0';

  if( IS_AFFECTED(ch, AFF_BLIND) )
  {
    sendChar(ch,"You can't see a damned thing, you're blind!\r\n");
    return;
  }

  for (door = 0; door < NUM_OF_DIRS; door++)
  {
    if( EXIT(ch, door) && EXIT(ch, door)->to_room != NOWHERE )
    {
      if IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED)
      {
        if (IS_DWARF(ch) || (IS_DROW(ch) && !IS_SUNBLIND(ch)) || GET_RACE(ch) == RACE_SDROW)
          sprintf(buf2, "%-5s - (closed)\r\n", dirs[door]);
      }
      else if (GET_LEVEL(ch) >= LVL_IMMORT) {
        if (SECT(EXIT(ch, door)->to_room) == SECT_UNDERWATER)
            sprintf(buf2, "%-5s - [%5d] &11(UW)&00 %s\r\n", dirs[door],
                    world[EXIT(ch, door)->to_room].number,
                    world[EXIT(ch, door)->to_room].name);
        else
            sprintf(buf2, "%-5s - [%5d] %s\r\n", dirs[door],
                    world[EXIT(ch, door)->to_room].number,
                    world[EXIT(ch, door)->to_room].name);
      } else {
          if (SECT(EXIT(ch, door)->to_room) == SECT_UNDERWATER)
              sprintf(buf2, "%-5s - &11(UW)&00 ", dirs[door]);
          else
              sprintf(buf2, "%-5s - ", dirs[door]);
          if (IS_DARK(EXIT(ch, door)->to_room) && !CAN_SEE_IN_DARK(ch))
              strcat(buf2, "Too dark to tell\r\n");
          else {
            strcat(buf2, world[EXIT(ch, door)->to_room].name);
            strcat(buf2, "\r\n");
          }
      }
    }
    strcat(buf, CAP(buf2));
    *buf2 = '\0';
}
  sendChar( ch, "Obvious exits:\r\n" );
  if (*buf)
      send_to_char(buf, ch);
  else
      send_to_char(" None.\r\n", ch);

}



void look_at_room(CharData * ch, int ignore_brief)
{
  if ((IS_DARK(ch->in_room) && !CAN_SEE_IN_DARK(ch)) || SHADOW_NOT_OK(ch))
  {
    send_to_char("Darkness engulfs you, pressing closer on all sides.\r\n", ch);
    return;
  }

  else if (IS_AFFECTED(ch, AFF_BLIND))
  {
    send_to_char("You see nothing but infinite darkness...\r\n", ch);
    return;
  }

  /* if char is sunblind, let him/her know it */
  if( IS_SUNBLIND(ch)) {
    send_to_char("&01The &03sunlight&01 is making your eyes water and run, but you think you see...\r\n", ch);
  }

  send_to_char(CCCYN(ch, C_NRM), ch);

  if( PRF_FLAGGED(ch, PRF_SHOWVNUMS) )
  {
    sprintbitarray(ROOM_FLAGS(ch->in_room), room_bits, RF_ARRAY_MAX, buf);
    sprintf(buf2, "[%5d] %s \r\n[ Room] %s\r\n",
            world[ch->in_room].number, world[ch->in_room].name, buf);
    send_to_char(buf2, ch);
    sprintbit(ZONE_FLAGS(world[ch->in_room].zone), zone_bits, buf);
    sprintf(buf2, "[ Zone] %s", buf);
    send_to_char(buf2, ch);
  }
  else
    send_to_char(world[ch->in_room].name, ch);

  send_to_char(CCNRM(ch, C_NRM), ch);
  send_to_char("\r\n", ch);

  if (!PRF_FLAGGED(ch, PRF_BRIEF) || ignore_brief ||
      ROOM_FLAGGED(ch->in_room, ROOM_DEATH))
    send_to_char(world[ch->in_room].description, ch);

  /* autoexits */
  if (PRF_FLAGGED(ch, PRF_AUTOEXIT))
    do_auto_exits(ch);

  /* now list characters & objects */
  send_to_char(CCGRN(ch, C_NRM), ch);
  list_obj_to_char(world[ch->in_room].contents, ch, 0, FALSE);
  send_to_char(CCYEL(ch, C_NRM), ch);
  list_char_to_char(world[ch->in_room].people, ch);
  send_to_char(CCNRM(ch, C_NRM), ch);
}

static void look_in_direction(CharData *ch, int dir)
{
  if (EXIT(ch, dir)) {
    if (EXIT(ch, dir)->general_description)
      sendChar(ch, "%s", EXIT(ch, dir)->general_description);
    else
      sendChar(ch, "You see nothing special.\r\n");

    if (EXIT_FLAGGED(EXIT(ch, dir), EX_CLOSED) && EXIT(ch, dir)->keyword)
      sendChar(ch, "The %s is closed.\r\n", fname(EXIT(ch, dir)->keyword));
    else if (EXIT_FLAGGED(EXIT(ch, dir), EX_ISDOOR) && EXIT(ch, dir)->keyword)
      sendChar(ch, "The %s is open.\r\n", fname(EXIT(ch, dir)->keyword));
  } else
    sendChar(ch, "Nothing special there...\r\n");
}

static void look_in_obj(CharData * ch, char *arg)
{
  ObjData *obj = NULL;
  CharData *dummy = NULL;
  int amt, bits;

  if (!*arg)
    sendChar(ch, "Look in what?\r\n");
  else if (!(bits = generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM |
				 FIND_OBJ_EQUIP, ch, &dummy, &obj))) {
    sendChar(ch, "There doesn't seem to be %s %s here.\r\n", AN(arg), arg);
  } else if ((GET_OBJ_TYPE(obj) != ITEM_DRINKCON) &&
	     (GET_OBJ_TYPE(obj) != ITEM_FOUNTAIN) &&
             (GET_OBJ_TYPE(obj) != ITEM_PORTAL)   &&
	     (GET_OBJ_TYPE(obj) != ITEM_CONTAINER))
    sendChar(ch, "There's nothing inside that!\r\n");
  else {
    /* Mortius : Changed look in portal to here */
    if (GET_OBJ_TYPE(obj) == ITEM_PORTAL) {
        look_in_portal(ch, obj);
    } else
    if (IS_CONTAINER(obj)) {
       if (OBJVAL_FLAGGED(obj, CONT_CLOSED) && (GET_LEVEL(ch) < LVL_IMMORT || !PRF_FLAGGED(ch, PRF_NOHASSLE)))
	sendChar(ch, "It is closed.\r\n");
      else {
	sendChar(ch, "%s", fname(obj->name));
	switch (bits) {
	case FIND_OBJ_INV:
	  sendChar(ch, " (carried): \r\n");
	  break;
	case FIND_OBJ_ROOM:
	  sendChar(ch, " (here): \r\n");
	  break;
	case FIND_OBJ_EQUIP:
	  sendChar(ch, " (used): \r\n");
	  break;
	}

	list_obj_to_char(obj->contains, ch, SHOW_OBJ_SHORT, TRUE);
      }
    } else {		/* item must be a fountain or drink container */
      if ((GET_OBJ_VAL(obj, 1) == 0) && (!GET_OBJ_VAL(obj, 0) == -1))
	sendChar(ch, "It is empty.\r\n");
      else {
        if (GET_OBJ_VAL(obj, 0) < 0)
        {
          char buf2[MAX_STRING_LENGTH];
          sprinttype(GET_OBJ_VAL(obj, 2), color_liquid, buf2, sizeof(buf2));
          sendChar(ch, "It's full of a %s liquid.\r\n", buf2);
      }
	else if (GET_OBJ_VAL(obj,1)>GET_OBJ_VAL(obj,0))
          sendChar(ch, "Its contents seem somewhat murky.\r\n"); /* BUG */
        else {
          char buf2[MAX_STRING_LENGTH];
          
          // This prevents crashes due to fountains with max-contents of 0.
          if(GET_OBJ_VAL(obj, 0) != 0)
              amt = (GET_OBJ_VAL(obj, 1) * 3) / GET_OBJ_VAL(obj, 0);
          else {
              mudlog(NRM, LVL_GOD, TRUE, "ERR: Fountain in room %d has zero max units.  FIX THIS!", world[(ch)->in_room].number);
              amt = 1;
          }
          
          sprinttype(GET_OBJ_VAL(obj, 2), color_liquid, buf2, sizeof(buf2));
	  sendChar(ch, "It's %sfull of a %s liquid.\r\n", fullness[amt], buf2);
	}
      }
    }
  }
}

char *find_exdesc(char *word, struct extra_descr_data * list)
{
  struct extra_descr_data *i;

  for (i = list; i; i = i->next)
    if (isname(word, i->keyword))
      return (i->description);

  return NULL;
}


/*
 * Given the argument "look at <target>", figure out what object or char
 * matches the target.  First, see if there is another char in the room
 * with the name.  Then check local objs for exdescs.
 */
void look_at_target(CharData * ch, char *arg)
{
  int bits, found = 0, j;
  CharData *found_char = NULL;
  ObjData *obj = NULL, *found_obj = NULL;
  char *desc;

  if (!*arg) {
    send_to_char("Look at what?\r\n", ch);
    return;
  }
  bits = generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP |
		      FIND_CHAR_ROOM, ch, &found_char, &found_obj);

  /* Is the target a character? */
  if (found_char != NULL) {
    if (ch != found_char) {
      if (CAN_SEE(found_char, ch))
	act("$n looks at you.", TRUE, ch, 0, found_char, TO_VICT);
      act("$n looks at $N.", TRUE, ch, 0, found_char, TO_NOTVICT);
    }
    look_at_char(found_char, ch);
    return;
  }
  /* Does the argument match an extra desc in the room? */
  if ((desc = find_exdesc(arg, world[ch->in_room].ex_description)) != NULL) {
    page_string(ch->desc, desc, 0);
    return;
  }
  /* Does the argument match an extra desc in the char's equipment? */
  for (j = 0; j < NUM_WEARS && !found; j++)
    if (ch->equipment[j] && CAN_SEE_OBJ(ch, ch->equipment[j]))
      if ((desc = find_exdesc(arg, ch->equipment[j]->ex_description)) != NULL) {
	send_to_char(desc, ch);
	found = 1;
      }
  /* Does the argument match an extra desc in the char's inventory? */
  for (obj = ch->carrying; obj && !found; obj = obj->next_content) {
    if (CAN_SEE_OBJ(ch, obj))
	if ((desc = find_exdesc(arg, obj->ex_description)) != NULL) {
	send_to_char(desc, ch);
	found = 1;
      }
  }

  /* Does the argument match an extra desc of an object in the room? */
  for (obj = world[ch->in_room].contents; obj && !found; obj = obj->next_content)
    if (CAN_SEE_OBJ(ch, obj))
	if ((desc = find_exdesc(arg, obj->ex_description)) != NULL) {
	send_to_char(desc, ch);
	found = 1;
      }
  if (bits) {			/* If an object was found back in
				 * generic_find */
    if (!found)
      show_obj_to_char(found_obj, ch, 5);	/* Show no-description */
    else
      show_obj_to_char(found_obj, ch, 6);	/* Find hum, glow etc */
  } else if (!found)
    send_to_char("You do not see that here.\r\n", ch);
}


ACMD(do_area)
{
  sendChar( ch, "\r\nYou are in &09%s&00.\r\n", ZONE_NAME(ch) );
}


ACMD(do_look)
{
  static char arg2[MAX_INPUT_LENGTH];
  int look_type;

  if (!ch->desc)
    return;

  if (GET_POS(ch) < POS_SLEEPING)
    send_to_char("You can't see anything but stars!\r\n", ch);
  else if (IS_AFFECTED(ch, AFF_BLIND))
    send_to_char("You can't see a damned thing, you're blind!\r\n", ch);

  else if( (IS_DARK(ch->in_room)  &&
          !CAN_SEE_IN_DARK(ch)) ||
           SHADOW_NOT_OK(ch) )
  {
    sendChar( ch, "Darkness engulfs you, pressing closer on all sides.\r\n" );
    list_char_to_char( world[ch->in_room].people, ch );
  }

  else
  {
    half_chop(argument, arg, arg2);

    if (subcmd == SCMD_READ) {
      if (!*arg)
	send_to_char("Read what?\r\n", ch);
      else
	look_at_target(ch, arg);
      return;
    }
    if (!*arg)			/* "look" alone, without an argument at all */
      look_at_room(ch, 1);
    else if (is_abbrev(arg, "in"))
      look_in_obj(ch, arg2);
    /* did the char type 'look <direction>?' */
    else if ((look_type = search_block(arg, dirs, FALSE)) >= 0)
      look_in_direction(ch, look_type);
    else if (is_abbrev(arg, "at"))
      look_at_target(ch, arg2);
    else
      look_at_target(ch, arg);
  }
}


ACMD(do_glance)
{
    CharData *target_ch  = NULL;
    ObjData  *target_obj = NULL;

    one_argument( argument, arg );
    if( *arg ){
        (void)generic_find(arg, FIND_CHAR_ROOM, ch, &target_ch, &target_obj);

        if(( target_ch != NULL ) && CAN_SEE( ch, target_ch )){
            diag_char_to_char( target_ch, ch );
            act("$n glances at you.", TRUE, ch, 0, target_ch, TO_VICT);
            act("$n glances at $N.",  TRUE, ch, 0, target_ch, TO_NOTVICT);
            return;
        }
    }
    send_to_char( "Glance at who?\n\r", ch );
}

ACMD(do_examine)
{
  int bits;
  CharData *tmp_char;
  ObjData  *tmp_object;

  one_argument(argument, arg);

  if (!*arg) {
    send_to_char("Examine what?\r\n", ch);
    return;
  }
  look_at_target(ch, arg);

  bits = generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_CHAR_ROOM |
		      FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);

  if( tmp_object == NULL ) return;

  if( (GET_OBJ_TYPE(tmp_object) == ITEM_DRINKCON) ||
      (GET_OBJ_TYPE(tmp_object) == ITEM_FOUNTAIN) ||
      (GET_OBJ_TYPE(tmp_object) == ITEM_CONTAINER))
  {
    sendChar( ch, "When you look inside, you see:\r\n" );
    look_in_obj(ch, arg);
  }
}



ACMD(do_gold)
{
  sendChar( ch, "---------------------------------------\r\n" );
  sendChar( ch, " Gold on Hand: %d\r\n", GET_GOLD( ch ));
  sendChar( ch, " Gold in Bank: %d\r\n", GET_BANK_GOLD(ch));
  sendChar( ch, "   Gold Total: %d\r\n", GET_GOLD(ch)+GET_BANK_GOLD(ch));
  sendChar( ch, "---------------------------------------\r\n" );
}

ACMD(do_arenarank)
{
  sendChar( ch, "Arena rank is no longer tracked.\r\n");
  return;
  sendChar( ch, "---------------------------------------\r\n" );
  sendChar( ch, " Your arena rank is: %d\r\n", GET_ARENA_RANK(ch));
  sendChar( ch, "---------------------------------------\r\n" );
}


#define SEPARATOR \
"-------------------------------------------------------------------------------\r\n"

int
getRangedIdx( int lowerVal, int upperVal, int idxRange, int theValue) {
    int maxRange  = upperVal - lowerVal;
    int baseValue = theValue - lowerVal;
    int idxThresh = maxRange / idxRange;
    int idxMax    = idxRange - 1;
    int theIdx    = baseValue / idxThresh;

    if( theIdx == upperVal ) return( idxMax );
    if( theIdx < 0 )         return( 0 );
    if( theIdx > idxMax )    return( idxMax );

    return( theIdx );
}

ACMD(do_score)
{
    char tmp[MAX_INPUT_LENGTH];

    static char *positAry[] = { "A Corpse", "Critical", "Moribund", "Stunned", "Sleeping","Meditating",
                                "Resting",  "Sitting",  "Fighting", "Standing" };
    static int positMax = sizeof(positAry)/sizeof(char *);
    static char *alignAry[] = { "Satanic",   "Demonic",   "Wicked",
                                "Evil",      "Sinful",    "Spiteful",
                                "Shifty",    "Neutral",   "Fair",
                                "Good",      "Moral",     "Pure",
                                "Virtuous",  "Righteous", "Holy" };
    static int   alignMax   = sizeof(alignAry)/sizeof(char *);
    static char *armorAry[] = { "Godlike",  "Impervious", "Shrouded",
                                "Shielded", "Armored",    "Protected",
                                "Clothed",  "Covered",    "Naked" };
    static int   armorMax   = sizeof(armorAry)/sizeof(char *);
    char *positStr = positAry[ getRangedIdx(     0,   10, positMax, GET_POS(ch)) ];
    char *alignStr = alignAry[ getRangedIdx( -1000, 1000, alignMax, GET_ALIGNMENT(ch)) ];

    int neededXp = (titles[(int) GET_CLASS(ch)][GET_LEVEL(ch) + 1].exp) - GET_EXP(ch);
    char *thirst = ((GET_COND(ch,THIRST) < 4 && GET_COND(ch,THIRST) >= 0) ? "&01Thirsty&00" : " ");
    char *hunger = ((GET_COND(ch,HUNGER  ) < 4 && GET_COND(ch, HUNGER  ) >= 0 && !IS_VAMPIRE(ch)) ? "&01Hungry&00" : " ");
    char *stupor = ((GET_COND(ch,DRUNK ) > 8 && GET_COND(ch,DRUNK ) >= 0) ? "&01Drunk&00" : " ");
    char *status = "&00None&00";

    char *armorStr;
    int real_ac;

    if (PLR_IS_LEGEND(ch)) status = "&14Legend&00";
    else if (PLR_IS_VETERAN(ch)) status = "&00Veteran&00";
    else if (PLR_IS_EXPLORER(ch)) status = "&00Explorer&00";

    if (ch->mount) positStr = "Mounted";

    real_ac = realAC(ch);

    armorStr = armorAry[ getRangedIdx( -100, 100, armorMax, real_ac)];


    if( GET_LEVEL(ch) > LVL_IMMORT ) neededXp = 999999999;

    sendChar( ch, "\r\n" );
    sendChar( ch, SEPARATOR );

    sendChar( ch, "Race   [%10s]  Cls  [&13%2s&00]          Lvl  [&13%2d&00]  Status [%16s]\r\n",
                   race_types[(int)GET_RACE(ch)], CLASS_ABBR(ch), GET_LEVEL(ch),
                   status);

    sendChar( ch, "Health [%s%4d/%4d%s ]  Exp  [&13%10d&00]  QP   [&13%2d&00]  %s  %s  %s\r\n",
                   colorRatio( ch, COLOR_COOKED, C_CMP, GET_HIT(ch), GET_MAX_HIT(ch) ),
                   GET_HIT(ch), GET_MAX_HIT(ch), CCNRM(ch,C_NRM), GET_EXP(ch),
                   ch->player_specials->saved.quest_pts, hunger, thirst, stupor);

    sendChar( ch, "Power  [%s%4d/%4d%s ]  Need [&13%10d&00]  Clan [&13%20s&00] %s\r\n",
                   colorRatio( ch, COLOR_COOKED, C_CMP, GET_MANA(ch), GET_MAX_MANA(ch) ),
                   GET_MANA(ch), GET_MAX_MANA(ch), CCNRM(ch,C_NRM),
                   neededXp, get_clan_name(GET_CLAN(ch)),
                   (IS_AFFECTED(ch, AFF_BLIND) ? "&08Blind&00":"" ));

    sendChar( ch, "Vigor  [%s%4d/%4d%s ]  Gold [&13%10d&00]  Rank [%20s]\r\n",
                   colorRatio( ch, COLOR_COOKED, C_CMP, GET_MOVE(ch), GET_MAX_MOVE(ch) ),
                   GET_MOVE(ch), GET_MAX_MOVE(ch), CCNRM(ch,C_NRM), GET_GOLD(ch),
                   get_clan_rank(GET_CLAN_RANK(ch)) );

    sendChar( ch, "Align  [&13%10s&00]   AC  [&13%10s&00]  Pos  [&13%12s&00]  Items [&13%2d&00]\r\n",
                   alignStr, armorStr, positStr, IS_CARRYING_N(ch));

#ifdef RESTRICT_ABIL_VIEW
    if( GET_LEVEL(ch) > 5 ){
#endif
        sendChar( ch, "Str  [&09%12s&00]  Int  [&09%10s&00]  Wis  [&09%12s&00]  Load  [&09%2d&00]\r\n",
                       ATTR_STR( 30, STRENGTH_APPLY_INDEX(ch), strString ),
                       ATTR_STR( 25, GET_INT(ch), intString ),
                       ATTR_STR( 25, GET_WIS(ch), wisString ), IS_CARRYING_W(ch));

        sendChar( ch, "Dex  [&09%12s&00]  Con  [&09%10s&00]  Cha  [&09%12s&00]  Age   [&09%2d&00]\r\n",
                       ATTR_STR( 25, GET_DEX(ch), dexString ),
                       ATTR_STR( 25, GET_CON(ch), conString ),
                       ATTR_STR( 25, GET_CHA(ch), chaString ),
                       GET_AGE(ch));
#ifdef RESTRICT_ABIL_VIEW
    }
#endif

    if(GET_ADVANCE_LEVEL(ch) > 0) {

        const char *specs[12][3] = {{"none", "Arcanist", "Enchanter"}, //Mu
                        {"none", "Dark Priest", "Holy Priest"}, //Cl
                        {"none", "Trickster", "Brigand"}, //Th
                        {"none", "Defender", "Dragon Slayer"}, //Wa
                        {"none", "Naturalist", "Hunter"}, //Ra
                        {"none", "Bounty Hunter", "Butcher"}, //As
                        {"none", "Chi Warrior", "Ancient Dragon"}, //Sl
                        {"none", "Dragoon", "Knight Templar"}, //Kn
                        {"none", "Knight Errant", "Defiler"}, //Dk
                        {"none", "Prestidigitator", "Shade"}, //Sd
                        {"none", "Witch Doctor", "Revenant"}, //Nm
                        {"none", "none", "none"}};

        sendChar(ch, "Spec [&09%12s&00]  Adv  [&13%2d&00]\r\n",
                specs[GET_CLASS(ch)][GET_SPEC(ch)], GET_ADVANCE_LEVEL(ch));
    }

    if(GET_CLASS(ch) == CLASS_SHOU_LIN && GET_LEVEL(ch) >= 20) {
        sprintf(tmp, stances[ch->stance]);
        sprintf(buf, "  Stance [%8s]", CAP(tmp));
        sprintf(tmp, aspects[GET_ASPECT(ch)]);
        sendChar(ch, "Aspect [%10s]%s\r\n", CAP(tmp), PLR_IS_VETERAN(ch) ? buf : "");
    }

    if( ch->affected && GET_LEVEL(ch) > 1 ){
        struct affected_type *aff;
        int affCnt = 0;

        for( aff = ch->affected; aff; aff = aff->next ){

            if( aff->type != SPELL_PLAGUE      &&
                aff->type != SPELL_CRUSADE2    &&
                aff->type != SPELL_SAGACITY2   &&
                aff->type != SPELL_MALEDICT2   &&
                aff->type != SKILL_EMACIATED_MANA &&
                aff->type != SKILL_EMACIATED_HIT  &&
                aff->type != SPELL_HEAL        &&
                aff->type != SPELL_CURE_CRITIC &&
                aff->type != SKILL_POULTICE    &&
                aff->type != SKILL_UNIMPEDED_ASSAULT &&
                aff->type != SKILL_DEFENDER_HEALTH &&
                aff->type != SKILL_RAGE        &&
                aff->type != SPELL_ZOMBIE_REWARD)
            {
                if( affCnt == 0 ){
                    sendChar( ch, SEPARATOR );
                    sendChar( ch, "You are affected by the following:\r\n" );
                }
# if 1

				if( (aff->duration)/72 ) {
					sendChar( ch, " %s%-21s%s (%2d hr)", CCCYN(ch, C_NRM),
						spells[aff->type], CCNRM(ch, C_NRM),
						((aff->duration )/72 + 1) );
				}
				else if(aff->duration >=0) {
					sendChar( ch, " %s%-21s%s (%2dmin)", CCCYN(ch, C_NRM),
						spells[aff->type], CCNRM(ch, C_NRM),
						(aff->duration + 1)*60/72 + 1 );
				}
                                else
                                    sendChar( ch, " %s%-21s%s ( On  )", CCCYN(ch, C_NRM),
						spells[aff->type], CCNRM(ch, C_NRM));

  # else
                sendChar( ch, " %s%-21s%s ", CCCYN(ch, C_NRM),
                               spells[aff->type], CCNRM(ch, C_NRM));
# endif
                affCnt += 1;

                if( affCnt % 2 ) sendChar( ch, "     " );
                else             sendChar( ch, "\r\n" );
            }
        }
        if( affCnt % 2 ) sendChar( ch, "\r\n" );
    }
    sendChar( ch, SEPARATOR );
}


ACMD(do_inventory)
{
  sendChar(ch, "You are carrying:\r\n");
  list_obj_to_char(ch->carrying, ch, SHOW_OBJ_SHORT, TRUE);
}

ACMD(do_equipment)
{
  int i, found = 0;

  sendChar(ch, "You are using:\r\n");
  for (i = 0; i < NUM_WEARS; i++) {
    if (GET_EQ(ch, i)) {
      if (CAN_SEE_OBJ(ch, GET_EQ(ch, i))) {

        /* Differentiate between some races just for fun. */
	if (IS_MINOTAUR(ch) && (i == WEAR_FEET))
		send_to_char("<worn on hooves>     ", ch);
	else if ((IS_DRACONIAN(ch) || IS_DRAGON(ch))
		&& ((i == WEAR_FINGER_R) || (i == WEAR_FINGER_L)))
		send_to_char("<worn on claw>       ", ch);
        else if ((i == WEAR_WIELD) && affected_by_spell(ch, SKILL_DISARM))
            send_to_char("<fumbling>           ", ch);
	else
            sendChar(ch, "%s", wear_where[i]);

	show_obj_to_char(GET_EQ(ch, i), ch, SHOW_OBJ_SHORT);
	found = TRUE;
      } else {

          /* Differentiate between some races just for fun. */
	if (IS_MINOTAUR(ch) && (i == WEAR_FEET))
            send_to_char("<worn on hooves>     ", ch);
        else if ((IS_DRACONIAN(ch) || IS_DRAGON(ch))
                && ((i == WEAR_FINGER_R) || (i == WEAR_FINGER_L)))
            send_to_char("<worn on claw>       ", ch);
        else if ((i == WEAR_WIELD) && affected_by_spell(ch, SKILL_DISARM))
            send_to_char("<fumbling>           ", ch);
        else
            sendChar(ch, "%s", wear_where[i]);
        sendChar(ch, "Something.\r\n");
	found = TRUE;
      }
    }
    else if (i < WEAR_ORBIT) /* Suppress messages if they cant wear an item there. */
	sendChar( ch, "%s Nothing.\r\n", wear_where[i] );
  }
}

ACMD(do_time)
{
  static const char *month_name[] = {
    "the Winter Wolf", "the Frost Giant",
    "the Old Forces", "the Grand Struggle",
    "the Spring", "Nature",
    "Fertility", "the Sun",
    "the Dragon", "the Heat",
    "the Rains", "the Dark Battle",
    "the Autumn", "the Shadows",
    "the Long Shadows", "the Ancient Darkness"
  };

  static const char *weekdays[] = {
    "the Moon", "the Bull", "the Deception",
    "the Waning Crescent", "Freedom",
    "the Great Gods", "the Sun"
  };

  char *suf;
  int weekday, day;

  sprintf(buf, "It is %d o'clock %s, on the Day of ",
	  ((time_info.hours % 12 == 0) ? 12 : ((time_info.hours) % 12)),
	  ((time_info.hours >= 12) ? "pm" : "am"));

  /* 35 days in a month */
  weekday = ((35 * time_info.month) + time_info.day + 1) % 7;

  strcat(buf, weekdays[weekday]);
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  day = time_info.day + 1;	/* day in [1..35] */

  if (day == 1) suf = "st";
  else if (day == 2) suf = "nd";
  else if (day == 3) suf = "rd";
  else if (day < 20) suf = "th";
  else if ((day % 10) == 1) suf = "st";
  else if ((day % 10) == 2) suf = "nd";
  else if ((day % 10) == 3) suf = "rd";
  else suf = "th";

  sprintf(buf, "The %d%s Day of the Month of %s, Year %d.\r\n",
	  day, suf, month_name[(int) time_info.month], time_info.year);

  send_to_char(buf, ch);

  if(GET_LEVEL(ch) >= LVL_IMMORT)
      sendChar(ch, "Date: %d%s of the %d%s, %d\r\n",
                time_info.day, ndth(time_info.day),
                time_info.month, ndth(time_info.month),
                time_info.year);
}


ACMD(do_weather)
{
  const char *sky_look[] = {
    "&03cloudless&00",
    "&06cloudy&00",
    "&11rainy&00",
    "&10lit by flashes of lightning&00"
  };

  if (OUTSIDE(ch))
    {
    sendChar(ch, "The sky is %s and %s.\r\n", sky_look[weather_info.sky],
	    weather_info.change >= 0 ? "you feel a warm wind from south" :
	     "your foot tells you bad weather is due");
    if (GET_LEVEL(ch) >= LVL_CREATOR)
      sendChar(ch, "Pressure: %d (change: %d), Sky: %d (%s)\r\n",
                 weather_info.pressure,
                 weather_info.change,
                 weather_info.sky,
                 sky_look[weather_info.sky]);
    }
  else
    sendChar(ch, "You have no feeling about the weather at all.\r\n");
}

/* puts -'s instead of spaces */
void space_to_minus(char *str)
{
  while ((str = strchr(str, ' ')) != NULL)
    *str = '-';
}

int search_help(const char *argument, int level)
{
  int chk, bot, top, mid, minlen;

   bot = 0;
   top = top_of_helpt;
   minlen = strlen(argument);

  while (bot <= top) {
    mid = (bot + top) / 2;

    if (!(chk = strn_cmp(argument, help_table[mid].keywords, minlen)))  {
      while ((mid > 0) && !strn_cmp(argument, help_table[mid - 1].keywords, minlen))
         mid--;

      while (level < help_table[mid].min_level && mid < (bot + top) / 2)
        mid++;

      if (strn_cmp(argument, help_table[mid].keywords, minlen) || level < help_table[mid].min_level)
	      break;

      return mid;
    }
    else if (chk > 0)
      bot = mid + 1;
    else
      top = mid - 1;
  }
  return NOWHERE;
}

ACMD(do_help)
{
  int mid = 0;
  int i, found = 0;

    if (!ch->desc)
    return;

  skip_spaces(&argument);

  if (!help_table) {
    sendChar(ch, "No help available.\r\n");
    return;
  }

  if (!*argument) {
    if (GET_LEVEL(ch) < LVL_IMMORT)
      page_string(ch->desc, help, 0);
    else
      page_string(ch->desc, ihelp, 0);
    return;
  }

  space_to_minus(argument);

  if ((mid = search_help(argument, GET_LEVEL(ch))) == NOWHERE) {
    sendChar(ch, "There is no help on that word.\r\n");
    mudlog(NRM, MAX(LVL_IMPL, GET_INVIS_LEV(ch)), TRUE,
      "%s tried to get help on %s", GET_NAME(ch), argument);
    for (i = 0; i < top_of_helpt; i++)  {
      if (help_table[i].min_level > GET_LEVEL(ch))
        continue;
      /* To help narrow down results, if they don't start with the same letters, move on. */
      if (*argument != *help_table[i].keywords)
        continue;
      if (levenshtein_distance(argument, help_table[i].keywords) <= 2) {
        if (!found) {
          sendChar(ch, "\r\nDid you mean:\r\n");
          found = 1;
        }
        sendChar(ch, "  %s\r\n", help_table[i].keywords);
      }
    }
    return;
  }
  page_string(ch->desc, help_table[mid].entry, 0);
}

#define WHO_FORMAT \
"format: who [-n name] [-s] [-o] [-q] [-r] [-z] [-e]\r\n"

ACMD(do_who)
{
  DescriptorData *d;
  CharData *tch;
  char name_search[80];
  char mode;
#if 0
  int i = 0;
#endif
  int low = 0, high = LVL_IMPL, localwho = 0, questwho = 0, explorewho = 0;
  int showclass = 0, short_list = 0, outlaws = 0, num_can_see = 0;
  int who_room = 0, noafk_who = 0;
  static int max_players_today = 0;  /* most players received this reboot. */

  skip_spaces(&argument);
  strcpy(buf, argument);
  name_search[0] = '\0';

  while (*buf) {
    half_chop(buf, arg, buf1);
    if (isdigit(*arg)) {
#if 0
      sscanf(arg, "%d-%d", &low, &high);
      strcpy(buf, buf1);
#else
      sendChar(ch, "That who feature has been disabled for now.\r\n");
      return;
#endif
    } else if (*arg == '-') {
      mode = *(arg + 1);	/* just in case; we destroy arg in the switch */
      switch (mode) {
      case 'o':
      case 'k':
	outlaws = 1;
	strcpy(buf, buf1);
	break;
      case 'z':
	localwho = 1;
	strcpy(buf, buf1);
	break;
      case 's':
	short_list = 1;
	strcpy(buf, buf1);
	break;
      case 'q':
	questwho = 1;
	strcpy(buf, buf1);
	break;
      case 'e':
        explorewho = 1;
        strcpy(buf, buf1);
        break;
/*      case 'l':
	half_chop(buf1, arg, buf);
	sscanf(arg, "%d-%d", &low, &high);
	break;
*/
      case 'n':
	half_chop(buf1, name_search, buf);
	break;
      case 'r':
	who_room = 1;
	strcpy(buf, buf1);
	break;
      case 'a':
        noafk_who = 1;
        strcpy(buf, buf1);
        break;
      default:
	send_to_char(WHO_FORMAT, ch);
	return;
	break;
      }				/* end of switch */

    } else {			/* endif */
      send_to_char(WHO_FORMAT, ch);
      return;
    }
  }				/* end while (parser) */


  send_to_char("\r\n--- &10Immortals&00 ---\r\n", ch);

  for (d = descriptor_list; d; d = d->next) {
    if (d->connected)
      continue;

    if (GET_LEVEL(d->character) < LVL_IMMORT)
      continue;

    if (PRF_FLAGGED(d->character, PRF_NOWHO)
            && GET_LEVEL(ch) < GET_LEVEL(d->character))
      continue;

    if (d->original)
      tch = d->original;
    else if (!(tch = d->character))
      continue;

    if (*name_search && str_cmp(GET_NAME(tch), name_search) &&
        !strstr(GET_TITLE(tch), name_search))
      continue;
    if( !CAN_SEE(ch, tch)      ||
         GET_LEVEL(tch) < low  ||
         GET_LEVEL(tch) > high)
      continue;
    if (outlaws && !PLR_FLAGGED(tch, PLR_KILLER) &&
        !PLR_FLAGGED(tch, PLR_THIEF) && !PLR_FLAGGED(tch, PLR_HUNTED))
      continue;
    if (questwho && !PRF_FLAGGED(tch, PRF_QUEST))
      continue;
    if (localwho && world[ch->in_room].zone != world[tch->in_room].zone)
      continue;
    if (who_room && (tch->in_room != ch->in_room))
      continue;
    if (noafk_who && IS_AFK(tch))
      continue;
    if (showclass && !(showclass & (1 << GET_CLASS(tch))))
      continue;
    if (explorewho && !PLR_IS_EXPLORER(tch))
        continue;
    if (short_list) {
       if (!PRF_FLAGGED(tch,PRF_NOTANON)) {
          if (GET_LEVEL(ch) >= LVL_IMMORT) {
             sprintf(buf, "%s[%2d %2s %3s] %-12.12s%s%s",
                     "&11", GET_LEVEL(tch), CLASS_ABBR(tch), RACE_ABBR(tch),
                     GET_NAME(tch), "&00",
                     ((!(++num_can_see % 4)) ? "\r\n" : ""));
          }
          else {
             sprintf(buf, "[ ------- ] %-12.12s%s",
                     GET_NAME(tch), ((!(++num_can_see % 4)) ? "\r\n" : ""));
          }
       }
       else {
          sprintf(buf, "[%2d %2s %3s] %-12.12s%s",
                  GET_LEVEL(tch), CLASS_ABBR(tch), IS_ELEMENTAL(tch)? SUBRACE_ABBR(tch) : RACE_ABBR(tch), GET_NAME(tch),
                  ((!(++num_can_see % 4)) ? "\r\n" : ""));
       }
       send_to_char(buf, ch);
    }
    else {
      num_can_see++;
      if (!PRF_FLAGGED(tch,PRF_NOTANON)) {
         if (GET_LEVEL(ch) >= LVL_IMMORT) {
            sprintf(buf, "%s[%6s %3s]%s %s %s",
                    "&11", god_labels[GET_LEVEL(tch)-LVL_IMMORT], RACE_ABBR(tch),
                    "&00", GET_NAME(tch), GET_TITLE(tch));
         }
         else {
            sprintf(buf, "[ -------- ] %s %s",
                    GET_NAME(tch), GET_TITLE(tch));
         }
      }
      else {
          sprintf(buf, "[%6s %3s] %s %s",
                  god_labels[GET_LEVEL(tch)-LVL_IMMORT], RACE_ABBR(tch),
                  GET_NAME(tch), GET_TITLE(tch));
      }

      if (GET_INVIS_LEV(tch))
        sprintf(buf, "%s (i%d)", buf, GET_INVIS_LEV(tch));
      else if (IS_AFFECTED(tch, AFF_INVISIBLE))
        strcat(buf, " (invis)");

      if (IS_AFK(tch))
        strcat(buf, " (AFK)");
      if (PRF_FLAGGED(tch, PRF_NOWIZ))
	strcat(buf, " (nowiz)");
      if (PLR_FLAGGED(tch, PLR_MAILING))
        strcat(buf, " (mailing)");
      else if (PLR_FLAGGED(tch, PLR_WRITING))
        strcat(buf, " (writing)");

      if (PRF_FLAGGED(tch, PRF_DEAF))
        strcat(buf, " (deaf)");
      if (PRF_FLAGGED(tch, PRF_NOTELL))
        strcat(buf, " (notell)");
      if (PRF_FLAGGED(tch, PRF_QUEST))
        strcat(buf, " (quest)");
      if (PRF_FLAGGED(tch, PRF_GOLD_TEAM))
        strcat(buf, " &03(Gold Team)&00");
      if (PRF_FLAGGED(tch, PRF_BLACK_TEAM))
        strcat(buf, " &05(Black Team)&00");
      if (PRF_FLAGGED(tch, PRF_ROGUE_TEAM))
        strcat(buf, "&08(Rogue Team)&00");
      if (PLR_FLAGGED(tch, PLR_THIEF))
        strcat(buf, " (THIEF)");
      if (PLR_FLAGGED(tch, PLR_KILLER))
        strcat(buf, " (KILLER)");
      if (PLR_FLAGGED(tch, PLR_HUNTED))
        strcat(buf, " &25&01(HUNTED)&00");
      if (PLR_FLAGGED(tch, PLR_JAILED))
        strcat(buf, " (JAILED)");
      if (PLR_FLAGGED(tch, PLR_REIMBURSED) && GET_LEVEL(ch) == LVL_IMPL)
        strcat(buf, " &08(Fixed)&00");

      if (PLR_IS_LEGEND(tch))
          strcat(buf, " &14(Legend)&00");
      else if (PLR_IS_VETERAN(tch))
          strcat(buf, " (Veteran)");
      else if (PLR_IS_EXPLORER(tch))
          strcat(buf, " (Explorer)");

      strcat(buf, CCNRM(ch, C_SPR));
      strcat(buf, "&00\r\n");
      send_to_char(buf, ch);
    }                           /* endif shortlist */
  }                             /* end of for */

  send_to_char("\r\n --- &10Players&00 ---\r\n", ch);

  for (d = descriptor_list; d; d = d->next) {
    if (d->connected)
      continue;

    if (GET_LEVEL(d->character) >= LVL_IMMORT)
      continue;

    if (PRF_FLAGGED(d->character, PRF_NOWHO)
            && GET_LEVEL(ch) < GET_LEVEL(d->character))
      continue;

    if (d->original)
      tch = d->original;
    else if (!(tch = d->character))
      continue;

    if (*name_search && str_cmp(GET_NAME(tch), name_search) &&
	!strstr(GET_TITLE(tch), name_search))
      continue;
    if (!CAN_SEE(ch, tch) || GET_LEVEL(tch) < low || GET_LEVEL(tch) > high)
      continue;
    if (outlaws && !PLR_FLAGGED(tch, PLR_KILLER) &&
	!PLR_FLAGGED(tch, PLR_THIEF) && !PLR_FLAGGED(tch, PLR_HUNTED))
      continue;
    if (questwho && !PRF_FLAGGED(tch, PRF_QUEST))
      continue;
    if (localwho && world[ch->in_room].zone != world[tch->in_room].zone)
      continue;
    if (who_room && (tch->in_room != ch->in_room))
      continue;
    if (noafk_who && IS_AFK(tch))
        continue;
    if (showclass && !(showclass & (1 << GET_CLASS(tch))))
      continue;
    if (explorewho && !PLR_IS_EXPLORER(tch))
        continue;
    if (short_list) {
       if (!PRF_FLAGGED(tch,PRF_NOTANON)) {
          if (GET_LEVEL(ch) >= LVL_IMMORT) {
             sprintf(buf, "%s[%2d %2s %3s] %-12.12s%s%s",
                     "&11", GET_LEVEL(tch), CLASS_ABBR(tch), RACE_ABBR(tch),
                     GET_NAME(tch), "&00",
                     ((!(++num_can_see % 4)) ? "\r\n" : ""));
          }
          else {
             sprintf(buf, "[ ------- ] %-12.12s%s",
                     GET_NAME(tch), ((!(++num_can_see % 4)) ? "\r\n" : ""));
          }
       }
       else {
          sprintf(buf, "[%2d %2s %3s] %-12.12s%s",
                  GET_LEVEL(tch), CLASS_ABBR(tch), RACE_ABBR(tch), GET_NAME(tch),
                  ((!(++num_can_see % 4)) ? "\r\n" : ""));
       }
       send_to_char(buf, ch);
    }
    else {
      num_can_see++;
      if (!PRF_FLAGGED(tch,PRF_NOTANON)) {
         if (GET_LEVEL(ch) >= LVL_IMMORT) {
            sprintf(buf, "%s[ %2d %2s %3s]%s %s %s",
                    "&11", GET_LEVEL(tch), CLASS_ABBR(tch), RACE_ABBR(tch),
                    "&00", GET_NAME(tch), GET_TITLE(tch));
         }
         else {
          sprintf(buf, "[ -------- ] %s %s",
                  GET_NAME(tch), GET_TITLE(tch));
         }
      }
      else {
          sprintf(buf, "[ %2d %2s %3s] %s %s",
	          GET_LEVEL(tch), CLASS_ABBR(tch), RACE_ABBR(tch),
		  GET_NAME(tch), GET_TITLE(tch));
      }

      if (GET_INVIS_LEV(tch))
	sprintf(buf, "%s (i%d)", buf, GET_INVIS_LEV(tch));
      else if (IS_AFFECTED(tch, AFF_INVISIBLE))
	strcat(buf, " (invis)");

      if (PLR_FLAGGED(tch, PLR_MAILING))
	strcat(buf, " (mailing)");
      else if (PLR_FLAGGED(tch, PLR_WRITING))
	strcat(buf, " (writing)");
      if (IS_AFK(tch)) {
        if( strcmp(GET_NAME(tch), "Karias" ) == 0 )
            strcat(buf, " (&10Nachos&00)");
        else
            strcat(buf, " (AFK)");
      }
      if (PRF_FLAGGED(tch, PRF_NOWIZ))
	strcat(buf, " (nowiz)");
      if (PRF_FLAGGED(tch, PRF_DEAF))
	strcat(buf, " (deaf)");
      if (PRF_FLAGGED(tch, PRF_NOTELL))
	strcat(buf, " (notell)");
      if (PRF_FLAGGED(tch, PRF_QUEST))
	strcat(buf, " (quest)");
      if (PRF_FLAGGED(tch, PRF_GOLD_TEAM))
	strcat(buf, " &03(Gold Team)&00");
      if (PRF_FLAGGED(tch, PRF_BLACK_TEAM))
	strcat(buf, " &05(Black Team)&00");
      if (PRF_FLAGGED(tch, PRF_ROGUE_TEAM))
        strcat(buf, "&08(Rogue Team)&00");
      if (PLR_FLAGGED(tch, PLR_THIEF))
	strcat(buf, " (THIEF)");
      if (PLR_FLAGGED(tch, PLR_KILLER))
	strcat(buf, " (KILLER)");
      if (PLR_FLAGGED(tch, PLR_HUNTED))
	strcat(buf, " (HUNTED)");
      if (PLR_FLAGGED(tch, PLR_JAILED))
	strcat(buf, " (JAILED)");
      if (PLR_FLAGGED(tch, PLR_GRIEF))
        strcat(buf, " (GRIEF)");
      if (PLR_FLAGGED(tch, PLR_REIMBURSED) && GET_LEVEL(ch) == LVL_IMPL)
        strcat(buf, " &08(Fixed)&00");

      if (PLR_IS_LEGEND(tch))
          strcat(buf, " &14(Legend)&00");
      else if (PLR_IS_VETERAN(tch))
          strcat(buf, " (Veteran)");
      else if (PLR_IS_EXPLORER(tch))
          strcat(buf, " (Explorer)");

      strcat(buf, CCNRM(ch, C_SPR));
      strcat(buf, "&00\r\n");
      send_to_char(buf, ch);
    }				/* endif shortlist */
  }				/* end of for */
  if (short_list && (num_can_see % 4))
    send_to_char("\r\n", ch);
  if (num_can_see == 1)
    sprintf(buf, "\r\nYou seem to be alone here!\r\n");
  else
    sprintf(buf, "\r\n%d characters displayed.\r\n", num_can_see);
  send_to_char(buf, ch);

  if (num_can_see > max_players_today)
    max_players_today = num_can_see;
  sendChar(ch, "Most players online today: %d.\r\n", max_players_today);
}

#define USERS_FORMAT \
"format: users [-l minlevel[-maxlevel]] [-n name] [-h host] [-r racelist] [-c classlist] [-o] [-p]\r\n"

ACMD(do_users)
{
  char line[200], line2[220], idletime[10], classname[20];
  char state[30], *timeptr, mode;
  char name_search[MAX_INPUT_LENGTH], host_search[MAX_INPUT_LENGTH];
  CharData *tch;
  DescriptorData *d;
  int low = 0, high = LVL_IMPL, num_can_see = 0;
  int showclass = 0, showrace = 0, outlaws = 0, playing = 0, deadweight = 0;
  char buf[MAX_INPUT_LENGTH], arg[MAX_INPUT_LENGTH];

  host_search[0] = name_search[0] = '\0';

  strcpy(buf, argument);	/* strcpy: OK (sizeof: argument == buf) */
  while (*buf) {
    char buf1[MAX_INPUT_LENGTH];

    half_chop(buf, arg, buf1);
    if (*arg == '-') {
      mode = *(arg + 1);  /* just in case; we destroy arg in the switch */
      switch (mode) {
      case 'o':
      case 'k':
	outlaws = 1;
	playing = 1;
	strcpy(buf, buf1);	/* strcpy: OK (sizeof: buf1 == buf) */
	break;
      case 'p':
	playing = 1;
	strcpy(buf, buf1);	/* strcpy: OK (sizeof: buf1 == buf) */
	break;
      case 'd':
	deadweight = 1;
	strcpy(buf, buf1);	/* strcpy: OK (sizeof: buf1 == buf) */
	break;
      case 'l':
	playing = 1;
	half_chop(buf1, arg, buf);
	sscanf(arg, "%d-%d", &low, &high);
	break;
      case 'n':
	playing = 1;
	half_chop(buf1, name_search, buf);
	break;
      case 'h':
	playing = 1;
	half_chop(buf1, host_search, buf);
	break;
      case 'c':
	playing = 1;
	half_chop(buf1, arg, buf);
	showclass = find_class_bitvector(arg);
	break;
      case 'r':
	playing = 1;
	half_chop(buf1, arg, buf);
	showrace = find_race_bitvector(arg);
	break;
      default:
	sendChar(ch, "%s", USERS_FORMAT);
	return;
      }				/* end of switch */

    } else {			/* endif */
      sendChar(ch, "%s", USERS_FORMAT);
      return;
    }
  }				/* end while (parser) */
  sendChar(ch,
	 "Num Class/Race  Name         State          Idl   Login@   Site\r\n"
	 "--- ----------- ------------ -------------- ----- -------- ------------------------\r\n");

  one_argument(argument, arg);

  for (d = descriptor_list; d; d = d->next) {
    if (STATE(d) != CON_PLAYING && playing)
      continue;
    if (STATE(d) == CON_PLAYING && deadweight)
      continue;
    if (IS_PLAYING(d)) {
      if (d->original)
	tch = d->original;
      else if (!(tch = d->character))
	continue;

      if (*host_search && !strstr(d->host, host_search))
	continue;
      if (*name_search && str_cmp(GET_NAME(tch), name_search))
	continue;
      if (!CAN_SEE(ch, tch) || GET_LEVEL(tch) < low || GET_LEVEL(tch) > high)
	continue;
      if (outlaws && !PLR_FLAGGED(tch, PLR_KILLER) &&
	  !PLR_FLAGGED(tch, PLR_THIEF))
	continue;
      if (showrace && !(showrace & (1 << GET_RACE(tch))))
	continue;
      if (showclass && !(showclass & (1 << GET_CLASS(tch))))
	continue;
      if (GET_INVIS_LEV(tch) > GET_LEVEL(ch))
	continue;

      if (d->original)
      	sprintf(classname, "[%2d %s %s]", GET_LEVEL(d->original),
                RACE_ABBR(d->original), CLASS_ABBR(d->original));
      else {
        static char *levClr[] = { "00", "09", "13", "12", "08", "01" };
        static char *immClr   = "22";
        int chLev = GET_LEVEL(d->character);
        char *clrPtr = ( chLev < LVL_IMMORT ? levClr[chLev/10] : immClr );
        sprintf( classname, "[&%s%2d&00 %s %s]", clrPtr, chLev,
		CLASS_ABBR(d->character), RACE_ABBR(d->character));
      }
    } else
      strcpy(classname, "   -   ");

    timeptr = asctime(localtime(&d->login_time));
    timeptr += 11;
    *(timeptr + 8) = '\0';

    if (STATE(d) == CON_PLAYING && d->original)
      strcpy(state, "Switched");
    else
      strcpy(state, connected_types[STATE(d)]);

    if (d->character && STATE(d) == CON_PLAYING)
      sprintf(idletime, "%5d", d->character->char_specials.timer *
                            SECS_PER_MUD_HOUR / SECS_PER_REAL_MIN);
    else if( d->connected != CON_PLAYING )
      sprintf( idletime, "&08%5d&00", d->dcTimer * SECS_PER_MUD_HOUR / SECS_PER_REAL_MIN);
    else
      strcpy(idletime, "     ");

    sprintf(line, "%3d %-11s %-12s %-14s %-3s %-8s ", d->desc_num, classname,
	d->original && d->original->player.name ? d->original->player.name :
	d->character && d->character->player.name ? d->character->player.name :
	"UNDEFINED",
	state, idletime, timeptr);

    if (d->host && *d->host)
      sprintf(line + strlen(line), "[%s]\r\n", d->host);
      else
        strcat(line, "[Hostname unknown]\r\n");

    if (STATE(d) != CON_PLAYING) {
      sprintf(line2, "%s%s%s", CCGRN(ch, C_SPR), line, CCNRM(ch, C_SPR));
      strcpy(line, line2);
    }
    if (STATE(d) != CON_PLAYING ||
		(STATE(d) == CON_PLAYING && CAN_SEE(ch, d->character))) {
      sendChar(ch, "%s", line);
      num_can_see++;
    }
  }

  sendChar(ch, "\r\n%d visible sockets connected.\r\n", num_can_see);
}

/* Generic page_string function for displaying text */
ACMD(do_gen_ps)
{
  if (IS_NPC(ch)) {
    sendChar(ch, "Not for mobiles!\r\n");
    return;
  }

  switch (subcmd) {
  case SCMD_CREDITS:
    page_string(ch->desc, credits, 0);
    break;
  case SCMD_NEWS:
    page_string(ch->desc, news, 0);
    break;
  case SCMD_INFO:
    page_string(ch->desc, info, 0);
    break;
  case SCMD_WIZLIST:
    page_string(ch->desc, wizlist, 0);
    break;
  case SCMD_IMMLIST:
    page_string(ch->desc, immlist, 0);
    break;
  case SCMD_HANDBOOK:
    page_string(ch->desc, handbook, 0);
    break;
  case SCMD_POLICIES:
    page_string(ch->desc, policies, 0);
    break;
  case SCMD_MOTD:
    page_string(ch->desc, motd, 0);
    break;
  case SCMD_IMOTD:
    page_string(ch->desc, imotd, 0);
    break;
  case SCMD_CLEAR:
    sendChar(ch, "\033[H\033[J");
    break;
  case SCMD_VERSION:
    sendChar(ch, "%s\r\n", circlemud_version);
    break;
  case SCMD_WHOAMI:
    sendChar(ch, "%s\r\n", GET_NAME(ch));
    break;
  default:
    mlog("SYSERR: Unhandled case in do_gen_ps. (%d)", subcmd);
    /* SYSERR_DESC: General page string function for such things as 'credits',
     * 'news', 'wizlist', 'clear', 'version'.  This occurs when a call is made
     * to this routine that is not one of the predefined calls.  To correct it,
     * either a case needs to be added into the function to account for the
     * subcmd that is being passed to it, or the call to the function needs to
     * have the correct subcmd put into place. */
    return;
  }
}

static void perform_mortal_where(CharData *ch, char *arg)
{
  CharData       *i;
  DescriptorData *d;
  int j;

  j = world[(IN_ROOM(ch))].zone;
  sendChar(ch, "Players in %s.\r\n--------------------\r\n", zone_table[j].name);
  for (d = descriptor_list; d; d = d->next) {
      if (STATE(d) != CON_PLAYING)
          continue;
      if ((i = (d->original ? d->original : d->character)) == NULL)
          continue;
      if (IN_ROOM(i) == NOWHERE || !CAN_SEE(ch, i))
          continue;
      if (world[IN_ROOM(ch)].zone != world[IN_ROOM(i)].zone)
          continue;

      sendChar(ch, "%-20s%s\r\n", GET_NAME(i), QNRM);
  }
}

static void print_object_location(int num, ObjData * obj, CharData * ch, int recur)
{
  if (num > 0)
    sendChar(ch, "O%3d. %-25s%s - ", num, obj->short_description, QNRM);
  else
    sendChar(ch, "%33s", " - ");

  if (SCRIPT(obj)) {
    if (!TRIGGERS(SCRIPT(obj))->next)
      sendChar(ch, "[T%d] ", GET_TRIG_VNUM(TRIGGERS(SCRIPT(obj))));
    else
      sendChar(ch, "[TRIGS] ");
  }

  if (IN_ROOM(obj) != NOWHERE)
    sendChar(ch, "[%5d] %s%s\r\n", GET_ROOM_VNUM(IN_ROOM(obj)), world[IN_ROOM(obj)].name, QNRM);
  else if (obj->carried_by)
    sendChar(ch, "carried by %s%s\r\n", PERS(obj->carried_by, ch), QNRM);
  else if (obj->worn_by)
    sendChar(ch, "worn by %s%s\r\n", PERS(obj->worn_by, ch), QNRM);
  else if (obj->in_obj) {
    sendChar(ch, "inside %s%s%s\r\n", obj->in_obj->short_description, QNRM, (recur ? ", which is" : " "));
    if (recur)
      print_object_location(0, obj->in_obj, ch, recur);
  } else
    sendChar(ch, "in an unknown location\r\n");
  }

static void perform_immort_where(CharData *ch, char *arg)
{
    CharData *i;
    ObjData  *k;
    DescriptorData *d;
    int num = 0, found = 0;

  if (!*arg) {
    sendChar(ch, "Players         Room    Location                            Zone\r\n");
    sendChar(ch, "--------------- ------- ----------------------------------- -------------------\r\n");
    for (d = descriptor_list; d; d = d->next)
      if (IS_PLAYING(d)) {
	i = (d->original ? d->original : d->character);
        if (i && CAN_SEE(ch, i) && (IN_ROOM(i) != NOWHERE)) {
	  if (d->original)
            sendChar(ch, "%-15s%s - [%5d] %s%s (in %s%s)\r\n",
              GET_NAME(i), QNRM, GET_ROOM_VNUM(IN_ROOM(d->character)),
              world[IN_ROOM(d->character)].name, QNRM, GET_NAME(d->character), QNRM);
	  else
            sendChar(ch, "%-15s%s %s[%s%5d%s]%s %-*s%s %s%s\r\n", GET_NAME(i), QNRM,
              QCYN, QYEL, GET_ROOM_VNUM(IN_ROOM(i)), QCYN, QNRM,
              35+count_color_chars(world[IN_ROOM(i)].name), world[IN_ROOM(i)].name, QNRM,
              zone_table[(world[IN_ROOM(i)].zone)].name, QNRM);
	}
      }
  } else {
    for (i = character_list; i; i = i->next)
      if (CAN_SEE(ch, i) && IN_ROOM(i) != NOWHERE && isname(arg, i->player.name)) {
	found = 1;
        sendChar(ch, "M%3d. %-25s%s - [%5d] %-25s%s", ++num, GET_NAME(i), QNRM,
               GET_ROOM_VNUM(IN_ROOM(i)), world[IN_ROOM(i)].name, QNRM);
        if (SCRIPT(i) && TRIGGERS(SCRIPT(i))) {
          if (!TRIGGERS(SCRIPT(i))->next)
            sendChar(ch, "[T%d] ", GET_TRIG_VNUM(TRIGGERS(SCRIPT(i))));
          else
            sendChar(ch, "[TRIGS] ");
      }
      sendChar(ch, "%s\r\n", QNRM);
      }
    for (num = 0, k = object_list; k; k = k->next)
      if (CAN_SEE_OBJ(ch, k) && isname(arg, k->name)) {
	found = 1;
	print_object_location(++num, k, ch, TRUE);
      }
    if (!found)
      sendChar(ch, "Couldn't find any such thing.\r\n");
  }
}

ACMD(do_where)
{
  char arg[MAX_INPUT_LENGTH];

  one_argument(argument, arg);

  if (GET_LEVEL(ch) >= LVL_IMMORT)
    perform_immort_where(ch, arg);
  else
    perform_mortal_where(ch, arg);
}

ACMD(do_levels)
{
  int i;

  if (IS_NPC(ch)) {
    send_to_char("You ain't nothin' but a hound-dog.\r\n", ch);
    return;
  }
  *buf = '\0';

  for (i = 1; i < LVL_IMMORT; i++) {
    sprintf(buf + strlen(buf), "[%2d] %8d-%-8d : ", i,
	    titles[(int) GET_CLASS(ch)][i].exp, titles[(int) GET_CLASS(ch)][i + 1].exp);
    switch (GET_SEX(ch)) {
    case SEX_MALE:
      strcat(buf, titles[(int) GET_CLASS(ch)][i].title_m);
      break;
    case SEX_FEMALE:
      strcat(buf, titles[(int) GET_CLASS(ch)][i].title_f);
      break;
    default:
      send_to_char("Oh dear.\r\n", ch);
      break;
    }
    strcat(buf, "\r\n");
  }
  send_to_char(buf, ch);
}

/* This function calculates how many rounds it would take for
** victim to kill ch. Vex.
*/
int rounds_to_die(CharData *ch, CharData *victim)
{

    int victim_damage;
    int victim_chance; /* Chance victim will hit ch */
    int victim_attacks;
    int victim_rounds; /* rounds victim will take to kill ch */
    int real_ac;

    real_ac = realAC(ch);

    victim_attacks = calculate_attack_count(victim);
    victim_chance = 100 - ((thacoCalc(victim) - (real_ac/10)) * 5);
    if (victim_chance < 5)
	victim_chance = 5;
    if (victim_chance > 95)
	victim_chance = 95;
    victim_damage = (calculateSimpleDamage(victim) *
		    victim_attacks * victim_chance)/100;
    if (IS_SET_AR(AFF_FLAGS(ch), AFF_SANCTUARY))
	victim_damage = victim_damage >> 1;
    if (victim_damage < 1)  victim_damage =  1;
    victim_rounds = (GET_HIT(ch)/victim_damage);
    return victim_rounds;
}

ACMD(do_consider)
{

    CharData *victim;
    int ch_rounds, victim_rounds, diff;

    one_argument(argument, buf);

    if( !(victim = get_char_room_vis(ch, buf)) ){
        sendChar( ch, "Consider killing who?\r\n" );
        return;
    }

    if( victim == ch ){
        sendChar( ch, "Easy! REAL easy...\r\n" );
        return;
    }

    ch_rounds = rounds_to_die(victim, ch);
    victim_rounds = rounds_to_die(ch, victim);
    diff = ch_rounds - victim_rounds;

    if (diff == 0)
	sendChar(ch, "The perfect match!\r\n");
    else if ((diff < 0) && (ch_rounds <= 1))
	sendChar(ch, "You could squash them like a bug!\r\n");
    else if ((diff > 0) && (victim_rounds <= 1))
	sendChar(ch, "They would squash you like a bug!\r\n");
    else if (diff < -100)
	sendChar(ch, "You could take on an army of them and walk away unscathed.\r\n");
    else if (diff <= -80)
	sendChar(ch, "You could take on a score of them at a time.\r\n");
    else if (diff <= -60)
	sendChar(ch, "You could kill them, their uncles, aunts, sisters, brothers, mother and father - all at once.\r\n");
    else if (diff <= -40)
	sendChar(ch, "If the gods smiled down upon them, they would STILL lose.\r\n");
    else if (diff <= -20)
	sendChar(ch, "They wouldn't have a hope in hell.\r\n");
    else if (diff <= -10)
	sendChar(ch, "Piece of cake.\r\n");
    else if (diff <= -7)
	sendChar(ch, "Easy.\r\n");
    else if (diff <= -5)
	sendChar(ch, "No problem.\r\n");
    else if (diff <= -3)
	sendChar(ch, "You would probably win.\r\n");
    else if (diff <= -2)
	sendChar(ch, "You have a distinct advantage over them.\r\n");
    else if (diff <= -1)
	sendChar(ch, "It'd be real close, but you have the edge.\r\n");
    else if (diff > 100)
	sendChar(ch, "They could take on army of your kind, and walk away unscathed.\r\n");
    else if (diff >= 80)
	sendChar(ch, "They take on a score of your ilk, and win.\r\n");
    else if (diff >= 60)
	sendChar(ch, "They'd kill you, your uncles, aunts, brothers, sisters, mother and father - all at once.\r\n");
    else if (diff >= 40)
	sendChar(ch, "If the gods smiled down upon you, you'd still lose.\r\n");
    else if (diff >= 20)
	sendChar(ch, "You wouldn't have a hope in hell.\r\n");
    else if (diff >= 10)
	sendChar(ch, "Dig your grave first. You'll need it.\r\n");
    else if (diff >= 7)
	sendChar(ch, "They'd slay you with ease.\r\n");
    else if (diff >= 5)
	sendChar(ch, "They would have no problem against you.\r\n");
    else if (diff >= 3)
	sendChar(ch, "They would probably win.\r\n");
    else if (diff == 2)
	sendChar(ch, "They have a distinct advantage over you.\r\n");
    else if (diff == 1)
	sendChar(ch, "It'd be real close and they have the edge...\r\n");
    else
	mudlog(NRM, LVL_IMMORT, TRUE, "CON LOGIC ERROR: diff = %d.", diff);
}

ACMD(do_diagnose)
{
  char buf[MAX_INPUT_LENGTH];
  CharData *vict;

  one_argument(argument, buf);

  if (*buf) {
    if (!(vict = get_char_room_vis(ch, buf)))
      sendChar(ch, "%s", CONFIG_NOPERSON);
    else
      diag_char_to_char(vict, ch);
  } else {
    if (FIGHTING(ch))
      diag_char_to_char(FIGHTING(ch), ch);
    else
      sendChar(ch, "Diagnose who?\r\n");
  }
}
/* This function is really doing two different things. Number one it is displaying
 * the available toggles. Number two, it's shooting out a message and setting a
 * toggle in the charcter specials data strcture. I admit, it should be broke
 * into two seperate functions. However, for the present I've dropped a few
 * comments in to help explain things along the way -Xiuh 08.19.09 */
ACMD(do_toggle)
{
    char buf2[4], arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    int toggle, tp, wimp_lev, result = 0, len = 0;
    const char *types[] = { "off", "brief", "normal", "on", "\n" };

    const struct {
        char *command;
        bitvector_t toggle; /* this needs changing once hashmaps are implemented */
        char min_level;
        char *disable_msg;
        char *enable_msg;
    } tog_messages[] = {
        {"summonable", PRF_SUMMONABLE, 0,
            "You are now safe from summoning by other players.\r\n",
            "You may now be summoned by other players.\r\n"},
        {"nohassle", PRF_NOHASSLE, LVL_IMMORT,
            "Nohassle disabled.\r\n",
            "Nohassle enabled.\r\n"},
        {"brief", PRF_BRIEF, 0,
            "Brief mode off.\r\n",
            "Brief mode on.\r\n"},
        {"compact", PRF_COMPACT, 0,
            "Compact mode off.\r\n",
            "Compact mode on.\r\n"},
        {"notell", PRF_NOTELL, 0,
            "You can now hear tells.\r\n",
            "You are now deaf to tells.\r\n"},
        {"noauction", PRF_NOAUCT, 0,
            "You can now hear auctions.\r\n",
            "You are now deaf to auctions.\r\n"},
        {"noshout", PRF_DEAF, 0,
            "You can now hear shouts.\r\n",
            "You are now deaf to shouts.\r\n"},
        {"noroleplay", PRF_NORPLAY, 0,
            "You can now hear roleplays.\r\n",
            "You are now deaf to roleplays.\r\n"},
        {"nograts", PRF_NOGRATZ, 0,
            "You can now hear gratz.\r\n",
            "You are now deaf to gratz.\r\n"},
        {"noarena", PRF_NOARENA, 0,
            "You will now see the arena results.\r\n",
            "You will not receive arena results.\r\n"},
        {"quest", PRF_QUEST, 0,
            "You are no longer part of the Quest.\r\n",
            "Okay, you are part of the Quest.\r\n"},
        {"showvnums", PRF_SHOWVNUMS, LVL_IMMORT,
            "You will no longer see the vnums.\r\n",
            "You will now see the vnums.\r\n"},
        {"norepeat", PRF_NOREPEAT, 0,
            "You will now have your communication repeated.\r\n",
            "You will no longer have your communication repeated.\r\n"},
        {"holylight", PRF_HOLYLIGHT, LVL_IMMORT,
            "HolyLight mode off.\r\n",
            "HolyLight mode on.\r\n"},
        {"slownameserver", 0, LVL_IMPL,
            "Nameserver_is_slow changed to OFF; IP addresses will now be resolved.\r\n",
            "Nameserver_is_slow changed to ON; sitenames will no longer be resolved.\r\n"},
        {"autoexits", PRF_AUTOEXIT, 0,
            "Autoexits disabled.\r\n",
            "Autoexits enabled.\r\n"},
        {"noanon", PRF_NOTANON, 0,
            "Your character information is now hidden.\r\n",
            "Your character information is now visible.\r\n"},
        {"consent", PRF_CONSENT, 0,
            "You have rescinded your consent.\r\n",
            "You have given consent.\r\n"},
        {"contag", 0, LVL_IMMORT,
            "Plagues are not contagious.\r\n",
            "Plagues will spread.\r\n"},
        {"rawlog", 0, LVL_IMPL,
            "Raw I/O logging disabled.\r\n",
            "Raw I/O logging enabled.\r\n"},
        {"nospam", PRF_NOSPAM, 0,
            "You will receive all character actions (SPAM).\r\n",
            "You will only receive pertinant character action (SPAM LITE).\r\n"},
        {"olcverbose", PRF_OLCV, LVL_BUILDER,
            "You will now use simple OLC entries.\r\n",
            "You will now use verbose OLC entries.\r\n"},
        {"noooc", PRF_NOOOC, 0,
            "You can now hear OOCs.\r\n",
            "You are now deaf to OOCs.\r\n"},
        {"noquery", PRF_NOQUERY, 0,
            "You can now hear queries.\r\n",
            "You are now deaf to queries.\r\n"},
        {"noguild", PRF_NOGUILD, 0,
            "You can now hear guild conversations.\r\n",
            "You are now deaf to guild conversations.\r\n"},
        {"recallable", PRF_NORECALL, 0,
            "You are now safe from recall by other players.\r\n",
            "You may now be recalled by other players.\r\n"},
        {"autosplit", PRF_AUTOSPLIT, 0,
            "Autosplit disabled.\r\n",
            "Autosplit enabled.\r\n"},
        {"autoloot", PRF_AUTOLOOT, 0,
            "Autoloot disabled.\r\n",
            "Autoloot enabled.\r\n"},
        {"noclan", PRF_NOCLAN, 0,
            "You can now hear the clan channel.\r\n",
            "You can no longer hear the clan channel.\r\n"},
        {"autogold", PRF_AUTOGOLD, 0,
            "Autogold disabled.\r\n",
            "Autogold enabled.\r\n"},
        {"nopkill", 0, LVL_IMPL,
            "Player killing disabled.\r\n",
            "Player killing enabled.\r\n"},
        {"nopsteal", 0, LVL_IMPL,
            "Player stealing disabled.\r\n",
            "Player stealing enabled.\r\n"},
        {"nowho", PRF_NOWHO, LVL_IMMORT,
            "You will now appear on the WHO list.\r\n",
            "You are now hidden from the WHO list.\r\n"},
        {"showdam", PRF_SHOWDAM, 0,
            "You will no longer see numeric damage.\r\n",
            "You will now see numeric damage.\r\n"},
        {"clsolc", PRF_CLS, LVL_BUILDER,
            "You will no longer clear screen in OLC.\r\n",
            "You will now clear screen in OLC.\r\n"},
        {"nowiz", PRF_NOWIZ, LVL_IMMORT,
            "You can now hear the Wiz-channel.\r\n",
            "You are now deaf to the Wiz-channel.\r\n"},
        {"afk", PRF_AFK, 0,
            "AFK is now Off.\r\n",
            "AFK is now On.\r\n"},
        {"color", 0, 0, "\n", "\n"},
        {"syslog", 0, LVL_IMMORT, "\n", "\n"},
        {"wimpy", 0, 0, "\n", "\n"},
        {"passivepet", PRF_PASSIVE_PET, 0,
                "Those under your control will no longer act passively.\r\n",
                "Those under your control will act passively.\r\n"},
        {"\n", 0, -1, "\n", "\n"} /* must be last */
    };

    if (IS_NPC(ch))
        return;

    argument = one_argument(argument, arg);
    any_one_arg(argument, arg2); /* so that we don't skip 'on' */

    if (!*arg) {
        if (!GET_WIMP_LEV(ch))
            strcpy(buf2, "OFF"); /* strcpy: OK */
        else
            sprintf(buf2, "%-3.3d", GET_WIMP_LEV(ch)); /* sprintf: OK */

        if (GET_LEVEL(ch) >= LVL_IMMORT) {
            sendChar(ch,
                    "&13IMMortal Preferences&00\r\n"
                    "          NoWiz: %-3s    "
                    "         ClsOLC: %-3s    "
                    "       NoHassle: %-3s\r\n"

                    "          NoWho: %-3s    "
                    "         Contag: %-3s    "
                    "      Holylight: %-3s\r\n"

                    "     olcverbose: %-3s    "
                    "      ShowVnums: %-3s    "
                    "         Syslog: %-3s\r\n",
                    /* show immortal only toggles */
                    ONOFF(PRF_FLAGGED(ch, PRF_NOWIZ)),
                    ONOFF(PRF_FLAGGED(ch, PRF_CLS)),
                    ONOFF(PRF_FLAGGED(ch, PRF_NOHASSLE)),

                    ONOFF(PRF_FLAGGED(ch, PRF_NOWHO)),
                    ONOFF(CONFIG_PLAGUE_IS_CONTAGIOUS),
                    ONOFF(PRF_FLAGGED(ch, PRF_HOLYLIGHT)),

                    ONOFF(PRF_FLAGGED(ch, PRF_OLCV)),
                    ONOFF(PRF_FLAGGED(ch, PRF_SHOWVNUMS)),
                    types[(PRF_FLAGGED(ch, PRF_LOG1) ? 1 : 0) + (PRF_FLAGGED(ch, PRF_LOG2) ? 2 : 0)]);
        }

                if (GET_LEVEL(ch) == LVL_IMPL) {
            sendChar(ch,
                    "        NoPKill: %-3s    "
                    "       NoPSteal: %-3s    "
                    " SlowNameserver: %-3s\r\n"

                    "         RawLog: %-3s\r\n",
                    /* show IMP only toggles */
                    ONOFF(CONFIG_PK_ALLOWED),
                    ONOFF(CONFIG_PT_ALLOWED),
                    ONOFF(CONFIG_NS_IS_SLOW),

                    ONOFF(raw_input_logging));
        }

        sendChar(ch,
                "\r\n&13Prompt Preferences&00\r\n"
                "             HP: %-3s    "
                "           Mana: %-3s    "
                "          Vigor: %-3s\r\n"
                "            Exp: %-3s    "
	        "                        "
                "           Tank: %-3s\r\n"
                /* show prompt toggles */

               "\r\n&13Play Preferences&00\r\n"
                "          Brief: %-3s    "
                "       NoRepeat: %-3s    "
                "        Compact: %-3s\r\n"

                "            AFK: %-3s    "
                "         NoSpam: %-3s    "
                "           Anon: %-3s\r\n"

                "     Recallable: %-3s    "
                "     Summonable: %-3s    "
                "        Consent: %-3s\r\n"

                "       AutoLoot: %-3s    "
                "       AutoGold: %-3s    "
                "      AutoSplit: %-3s\r\n"

                "          Wimpy: %-3s    "
                "      AutoExits: %-3s    "
                "          Color: %s\r\n"

                "     PassivePet: %-3s    \r\n"

                "\r\n&13Channel Preferences&00\r\n"
                "          Quest: %-3s    "
                "     NoRoleplay: %-3s    "
                "          NoOoc: %-3s\r\n"

                "        NoQuery: %-3s    "
                "        NoGuild: %-3s    "
                "        NoShout: %-3s\r\n"

                "        NoGrats: %-3s    "
                "      NoAuction: %-3s    "
                "        NoArena: %-3s\r\n"

                "        NoTells: %-3s    "
	        "                        "
                "         NoClan: %-3s\r\n",

                /* show global player toggles */
                /* show prompt toggles */
                ONOFF(PRF_FLAGGED(ch, PRF_DISPHP)),
                ONOFF(PRF_FLAGGED(ch, PRF_DISPMANA)),
                ONOFF(PRF_FLAGGED(ch, PRF_DISPMOVE)),
                ONOFF(PRF_FLAGGED(ch, PRF_DISPEXP)),
                ONOFF(PRF_FLAGGED(ch, PRF_SHOWTANK)),

                ONOFF(PRF_FLAGGED(ch, PRF_BRIEF)),
                ONOFF(PRF_FLAGGED(ch, PRF_NOREPEAT)),
                ONOFF(PRF_FLAGGED(ch, PRF_COMPACT)),

                ONOFF(PRF_FLAGGED(ch, PRF_AFK)),
                ONOFF(PRF_FLAGGED(ch, PRF_NOSPAM)),
                ONOFF(!PRF_FLAGGED(ch, PRF_NOTANON)),

                ONOFF(PRF_FLAGGED(ch, PRF_NORECALL)),
                ONOFF(PRF_FLAGGED(ch, PRF_SUMMONABLE)),
                ONOFF(PRF_FLAGGED(ch, PRF_CONSENT)),

                ONOFF(PRF_FLAGGED(ch, PRF_AUTOLOOT)),
                ONOFF(PRF_FLAGGED(ch, PRF_AUTOGOLD)),
                ONOFF(PRF_FLAGGED(ch, PRF_AUTOSPLIT)),

                buf2,
                ONOFF(PRF_FLAGGED(ch, PRF_AUTOEXIT)),
                types[COLOR_LEV(ch)],

                ONOFF(PRF_FLAGGED(ch, PRF_PASSIVE_PET)),

                /* show channel toggles */
                ONOFF(PRF_FLAGGED(ch, PRF_QUEST)),
                ONOFF(PRF_FLAGGED(ch, PRF_NORPLAY)),
                ONOFF(PRF_FLAGGED(ch, PRF_NOOOC)),

                ONOFF(PRF_FLAGGED(ch, PRF_NOQUERY)),
                ONOFF(PRF_FLAGGED(ch, PRF_NOGUILD)),
                ONOFF(PRF_FLAGGED(ch, PRF_DEAF)),

                ONOFF(PRF_FLAGGED(ch, PRF_NOGRATZ)),
                ONOFF(PRF_FLAGGED(ch, PRF_NOAUCT)),
                ONOFF(PRF_FLAGGED(ch, PRF_NOARENA)),

                ONOFF(PRF_FLAGGED(ch, PRF_NOTELL)),
                ONOFF(PRF_FLAGGED(ch, PRF_NOCLAN)));

        if (PLR_IS_EXPLORER(ch)) {
            sendChar(ch,
                    "\r\n&13Status Preferences&00\r\n"

                    "        ShowDam: %-3s\r\n",
                    /* show status toggles */
                    ONOFF(PRF_FLAGGED(ch, PRF_SHOWDAM)));
        }
        return; /* This ends our toggle display output */
    }

    len = strlen(arg);
    for (toggle = 0; *tog_messages[toggle].command != '\n'; toggle++)
        if (!strncmp(arg, tog_messages[toggle].command, len))
            break;

    if (*tog_messages[toggle].command == '\n' || tog_messages[toggle].min_level > GET_LEVEL(ch)) {
        sendChar(ch, "You can't toggle that!\r\n");
        return;
    }

    switch (toggle) { /* Now let's find special toggle casses */
        case SCMD_COLOR:
            if (!*arg2) {
                sendChar(ch, "Your current color level is %s.\r\n", types[COLOR_LEV(ch)]);
                return;
            }

            if (((tp = search_block(arg2, types, FALSE)) == -1)) {
                sendChar(ch, "Usage: toggle color { Off | Brief | Normal | On }\r\n");
                return;
            }
            REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_COLOR_1);
            REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_COLOR_2);
            if (tp & 1) SET_BIT_AR(PRF_FLAGS(ch), PRF_COLOR_1);
            if (tp & 2) SET_BIT_AR(PRF_FLAGS(ch), PRF_COLOR_2);

            sendChar(ch, "Your %scolor%s is now %s.\r\n", CCRED(ch, C_SPR), CCNRM(ch, C_OFF), types[tp]);
            return;
        case SCMD_SYSLOG:
            if (!*arg2) {
                sendChar(ch, "Your syslog is currently %s.\r\n",
                        types[(PRF_FLAGGED(ch, PRF_LOG1) ? 1 : 0) + (PRF_FLAGGED(ch, PRF_LOG2) ? 2 : 0)]);
                return;
            }
            if (((tp = search_block(arg2, types, FALSE)) == -1)) {
                sendChar(ch, "Usage: toggle syslog { Off | Brief | Normal | On }\r\n");
                return;
            }
            REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_LOG1);
            REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_LOG2);
            if (tp & 1) SET_BIT_AR(PRF_FLAGS(ch), PRF_LOG1);
            if (tp & 2) SET_BIT_AR(PRF_FLAGS(ch), PRF_LOG2);

            sendChar(ch, "Your syslog is now %s.\r\n", types[tp]);
            return;
        case SCMD_SLOWNS:
            result = (CONFIG_NS_IS_SLOW = !CONFIG_NS_IS_SLOW);
            break;
        case SCMD_RAWLOG:
            result = (raw_input_logging = !raw_input_logging);
            break;
        case SCMD_PLAGUE:
            result = (CONFIG_PLAGUE_IS_CONTAGIOUS = !CONFIG_PLAGUE_IS_CONTAGIOUS);
            break;
        case SCMD_PSTEAL:
            result = (CONFIG_PT_ALLOWED = !CONFIG_PT_ALLOWED);
            break;
        case SCMD_PKILL:
            result = (CONFIG_PK_ALLOWED = !CONFIG_PK_ALLOWED);
            break;
        case SCMD_AFK:
            if ((result = PRF_TOG_CHK(ch, PRF_AFK)))
                act("$n is now away from $s keyboard.", TRUE, ch, 0, 0, TO_ROOM);
            else {
                act("$n has returned to $s keyboard.", TRUE, ch, 0, 0, TO_ROOM);
                if (has_mail(GET_IDNUM(ch)))
                    sendChar(ch, "You have mail waiting.\r\n");
            }
            break;
        case SCMD_WIMPY:
            if (!*arg2) {
                if (GET_WIMP_LEV(ch)) {
                    sendChar(ch, "Your current wimp level is %d hit points.\r\n", GET_WIMP_LEV(ch));
                    return;
                } else {
                    sendChar(ch, "At the moment, you're not a wimp.  (sure, sure...)\r\n");
                    return;
                }
            }
            if (isdigit(*arg2)) {
                if ((wimp_lev = atoi(arg2)) != 0) {
                    if (wimp_lev < 0)
                        sendChar(ch, "Heh, heh, heh.. we are jolly funny today, eh?\r\n");
                    else if (wimp_lev > GET_MAX_HIT(ch))
                        sendChar(ch, "That doesn't make much sense, now does it?\r\n");
                    else if (wimp_lev > (GET_MAX_HIT(ch) / 2))
                        sendChar(ch, "You can't set your wimp level above half your hit points.\r\n");
                    else {
                        sendChar(ch, "Okay, you'll wimp out if you drop below %d hit points.", wimp_lev);
                        GET_WIMP_LEV(ch) = wimp_lev;
                    }
                } else {
                    sendChar(ch, "Okay, you'll now tough out fights to the bitter end.");
                    GET_WIMP_LEV(ch) = 0;
                }
            } else
                sendChar(ch, "Specify at how many hit points you want to wimp out at.  (0 to disable)\r\n");
            break;
        default:
            if (!*arg2) {
                TOGGLE_BIT_AR(PRF_FLAGS(ch), tog_messages[toggle].toggle);
                result = (PRF_FLAGGED(ch, tog_messages[toggle].toggle));
            } else if (!strcmp(arg2, "on")) {
                SET_BIT_AR(PRF_FLAGS(ch), tog_messages[toggle].toggle);
                result = 1;
            } else if (!strcmp(arg2, "off")) {
                REMOVE_BIT_AR(PRF_FLAGS(ch), tog_messages[toggle].toggle);
            } else {
                sendChar(ch, "Value for %s must either be 'on' or 'off'.\r\n", tog_messages[toggle].command);
                return;
            }
    }
    if (result)
        sendChar(ch, "%s", tog_messages[toggle].enable_msg);
    else
        sendChar(ch, "%s", tog_messages[toggle].disable_msg);
}

struct sort_struct {
  int sort_pos;
  byte is_social;
}          *cmd_sort_info = NULL;

int num_of_cmds;

void sort_commands(void)
{
  int a, b, tmp;

  ACMD(do_action);

  num_of_cmds = 0;

  /*
   * first, count commands (num_of_commands is actually one greater than the
   * number of commands; it inclues the '\n'.
   */
  while (*cmd_info[num_of_cmds].command != '\n')
    num_of_cmds++;

  /* create data array */
  CREATE(cmd_sort_info, struct sort_struct, num_of_cmds);

  /* initialize it */
  for (a = 1; a < num_of_cmds; a++) {
    cmd_sort_info[a].sort_pos = a;
    cmd_sort_info[a].is_social = (cmd_info[a].command_pointer == do_action);
  }

  /* the infernal special case */
  cmd_sort_info[find_command("insult")].is_social = TRUE;

  /* Sort.  'a' starts at 1, not 0, to remove 'RESERVED' */
  for (a = 1; a < num_of_cmds - 1; a++)
    for (b = a + 1; b < num_of_cmds; b++)
      if (strcmp(cmd_info[cmd_sort_info[a].sort_pos].command,
		 cmd_info[cmd_sort_info[b].sort_pos].command) > 0) {
	tmp = cmd_sort_info[a].sort_pos;
	cmd_sort_info[a].sort_pos = cmd_sort_info[b].sort_pos;
	cmd_sort_info[b].sort_pos = tmp;
      }
}



ACMD(do_commands)
{
  int no, i, cmd_num;
  int wizhelp = 0, socials = 0;
  CharData *vict;

  if (!IS_NPC(ch) && IS_SET_AR(PLR_FLAGS(ch), PLR_NOCOMM)) {
      send_to_char("You seem unable to do that just now.\r\n", ch);
      return;
  }

  one_argument(argument, arg);

  if (*arg) {
    if (!(vict = get_char_vis(ch, arg, 1)) || IS_NPC(vict)) {
      send_to_char("Who is that?\r\n", ch);
      return;
    }
    if (GET_LEVEL(ch) < GET_LEVEL(vict)) {
      send_to_char("You can't see the commands of people above your level.\r\n", ch);
      return;
    }
  } else
    vict = ch;

  if (subcmd == SCMD_SOCIALS)
    socials = 1;
  else if (subcmd == SCMD_WIZHELP)
    wizhelp = 1;

  sprintf(buf, "The following %s%s are available to %s:\r\n",
	  wizhelp ? "privileged " : "",
	  socials ? "socials" : "commands",
	  vict == ch ? "you" : GET_NAME(vict));

  /* cmd_num starts at 1, not 0, to remove 'RESERVED' */
  for (no = 1, cmd_num = 1; cmd_num < num_of_cmds; cmd_num++) {
    i = cmd_sort_info[cmd_num].sort_pos;
    if ((cmd_info[i].minimum_level > 0 ||  cmd_sort_info[i].is_social)&&
	GET_LEVEL(vict) >= cmd_info[i].minimum_level &&
	(cmd_info[i].minimum_level >= LVL_IMMORT) == wizhelp &&
	(wizhelp || socials == cmd_sort_info[i].is_social)) {
      sprintf(buf + strlen(buf), "%-11s", cmd_info[i].command);
      if (!(no % 7))
	strcat(buf, "\r\n");
      no++;
    }
  }

  strcat(buf, "\r\n");
  send_to_char(buf, ch);
}


/* JC Oct 24/94
 * Throw -- throw a weapon at a target in an adjacent room
 *
 * ripped some of this off from do_scan :-)  thanx Dig.
 *
 * command : throw <weapon> <direction> <target>
 */
ACMD(do_old_throw)
{
    char	     dir[255], target[255], weapon[255], temp[255];

    ObjData  *obj;
    CharData *vict;

    char tmpname[MAX_INPUT_LENGTH];
    char *tmp = tmpname;

    static const char *throw_directions[] = { "north", "east", "south", "west", "up", "down", "\n"};
    int throw_dir                   = NOWHERE;

#   define CAN_THROW( room, door ) \
        ((world[room].dir_option[door]) &&                     \
         (world[room].dir_option[door]->to_room != NOWHERE) && \
         (!IS_SET(world[room].dir_option[door]->exit_info, EX_CLOSED)))

    half_chop (argument, weapon, temp);

    two_arguments(temp, dir, target);

    if( !*weapon || !*target || !*dir ) {
        send_to_char( "Try : throw <weapon> <direction> <target>\n\r", ch );
        return;
    }/* if */

    if (!(obj = get_obj_in_list_vis(ch, weapon, ch->carrying))) {
	sprintf (buf, "You don't seem to have a %s to throw.\n\r", weapon);
	send_to_char (buf, ch);
	return;
    }

    if( !CAN_WEAR( obj, ITEM_WEAR_HOLD ) ){
	send_to_char( "You can't throw that!\n\r", ch );
	return;
    }

    if(( throw_dir = search_block( dir, throw_directions, FALSE )) < 0 ) {
	send_to_char( "Throw in what direction???\n\r", ch );
	return;
    }
    else {

	if (CAN_THROW(ch->in_room, throw_dir )) {
	    CharData *throw_mob = NULL;
            int 	dest_room;
	    int		num;
	    int		j = 0;
	    int		dam;

	    dest_room = world [ch->in_room].dir_option[throw_dir]->to_room;

	    strcpy (tmp, target);
	    num = get_number(&tmp);

	    vict = NULL;

	    for (throw_mob  = world[ dest_room ].people; throw_mob && j <= num; throw_mob = throw_mob->next_in_room ) {
		if (isname(tmp, throw_mob->player.name))
		    if (CAN_SEE(ch, throw_mob))
		        if (++j >= num) {
                            vict = throw_mob;
                            break;
		        }
	    }/* for */

	    if (!vict) {
		sprintf (buf, "You can't see %s in that direction!\n\r", target);
		send_to_char ( buf, ch);
		return;
	    }/* if */

	    if(( GET_LEVEL(vict) >= LVL_IMMORT) && (GET_LEVEL(ch) < LVL_IMMORT )) {
		send_to_char ("Maybe in the after life!\n\r", ch);
		return;
	    }

	    /*
	     * Ok - at this point, we've got the weapon, the target
	     * and the room
	     */

	    obj_from_char ( obj );

	    /*
	     *	JC XXX Hardcoded shit below
	     *
	     *  There's 50% chance that the mob picks up the weapon
	     *
	     */
	    if ( number(1, 100 ) > 50 ) {
		obj_to_room ( obj, vict->in_room );
	    }
	    else {
		obj_to_char ( obj, vict );
	    }

	    if ( GET_SKILL(ch, SKILL_THROW ) > number (1, 101) ) {
		char	c[10] = " ";

		/*
		 * Damage is dice of weapon and any +DAM on the weapon --
		 * nothing else affects this
		 */
#ifdef OLD_THROW
		dam = dice(GET_OBJ_VAL(obj, 1), GET_OBJ_VAL(obj, 2));
#else
                dam = 1;
#endif
		if (obj->affected[0].location == APPLY_DAMROLL)
		{
		    dam += obj->affected[0].modifier;
		}
		else if (obj->affected[1].location == APPLY_DAMROLL)
		{
		    dam += obj->affected[1].modifier;
		}

		/* Ick -- hardcoded number 4 is direction "down"
		 *
		 * Send some kinda message to the room
		 *
		 */

		if ( throw_dir < 4 )
		    strcpy (c, " to the ");

	        sprintf (buf,"$n throws $o%s%s", c, throw_directions[throw_dir] );
		act (buf, FALSE, ch, obj, 0, TO_ROOM);

		sprintf (buf, "You throw $o%s%s",c, throw_directions[throw_dir] );
		act (buf, FALSE, ch, obj, 0, TO_CHAR);

		/*
		 * Send a message to the room with the vict
		 *
		 */
		damage (ch, vict, dam, SKILL_THROW);
		WAIT_STATE(vict, PULSE_VIOLENCE*2);
	    }
	    else {
		damage (ch, vict, 0, SKILL_THROW);
	    }
	    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
	}
	else {
	    send_to_char ("You can't throw in that direction!\n\r", ch);
	    return;
	}
    }
}

void perform_immort_stat( CharData *ch, char *arg )
{
  CharData *victim = 0;
  ObjData *object = 0;
  struct char_file_u tmp_store;


  half_chop(arg, buf1, buf2);

  if (IS_NPC(ch))
    return;

  if (!*buf1) {
    send_to_char("Stats on who or what?\r\n", ch);
    return;
  } else if (is_abbrev(buf1, "room")) {
    do_stat_room(ch);
  } else if (is_abbrev(buf1, "mob")) {
    if (!*buf2)
      send_to_char("Stats on which mobile?\r\n", ch);
    else {
      if ((victim = get_char_vis(ch, buf2, 0)))
	do_stat_character(ch, victim);
      else
	send_to_char("No such mobile around.\r\n", ch);
    }
  } else if (is_abbrev(buf1, "player")) {
    if (!*buf2) {
      send_to_char("Stats on which player?\r\n", ch);
    } else {
      if ((victim = get_player_vis(ch, buf2)))
	do_stat_character(ch, victim);
      else
	send_to_char("No such player around.\r\n", ch);
    }
  } else if (is_abbrev(buf1, "file")) {
    if (!*buf2) {
      send_to_char("Stats on which player?\r\n", ch);
    } else {
      CREATE(victim, CharData, 1);
      clear_char(victim);
      if (load_char(buf2, &tmp_store) > -1) {
	store_to_char(&tmp_store, victim);
	if (GET_LEVEL(victim) > GET_LEVEL(ch))
	  send_to_char("Sorry, you can't do that.\r\n", ch);
	else
        {
          victim->in_room = 0; /* Digger 5/5/98 */
	  do_stat_character(ch, victim);
        }
	free_char(victim);
      } else {
	send_to_char("There is no such player.\r\n", ch);
	free(victim);
      }
    }
  } else if (is_abbrev(buf1, "object")) {
    if (!*buf2)
      send_to_char("Stats on which object?\r\n", ch);
    else {
      if ((object = get_obj_vis(ch, buf2)))
	do_stat_object(ch, object);
      else
	send_to_char("No such object around.\r\n", ch);
    }
  } else {
    if ((victim = get_char_vis(ch, buf1, 0)))
      do_stat_character(ch, victim);
    else if ((object = get_obj_vis(ch, buf1)))
      do_stat_object(ch, object);
    else
      send_to_char("Nothing around by that name.\r\n", ch);
  }
}

void perform_mortal_stat(CharData *ch, char *argument)
{
	int found, i;
	OBJ_DATA *obj;

	one_argument(argument, arg);

	if( !*arg )
	{
		sendChar( ch, "Examine stats of what item?\r\n" );
		return;
	}

	if( !(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
		// If no item prevails in inventory, check worn shit.
		found = 0;
		for( i = 0; i < NUM_WEARS; i++ )
			if( ch->equipment[i] && CAN_SEE_OBJ(ch, ch->equipment[i]) &&
				isname(arg, ch->equipment[i]->name))
			{
				obj = ch->equipment[i];
				found = 1;
			}
	}

	if ( !obj )
	{
		sendChar( ch, "You don't seem to have that item.\r\n");
		return;
	}

	if( !IS_OBJ_STAT(obj, ITEM_IDENTIFIED) )
	{
		send_to_char("This item has not yet been identified.\r\n", ch);
		return;
	}

	if( (IS_OBJ_STAT(obj, ITEM_ARTIFACT))){
		send_to_char( "This object is beyond your comprehension.\r\n", ch);
		return;
    }

	send_to_char("You feel informed:\r\n", ch);
    sprintf(buf, "Object '%s', Item type: ", obj->short_description);
    sprinttype(GET_OBJ_TYPE(obj), item_types, buf2, sizeof(buf2));
    strcat(buf, buf2);
    strcat(buf, "\r\n");
    send_to_char(buf, ch);

	if (obj->obj_flags.bitvector) {
		send_to_char("Item will give you following abilities:  ", ch);
		sprintbitarray(obj->obj_flags.bitvector, affected_bits, AF_ARRAY_MAX, buf);
		strcat(buf, "\r\n");
		send_to_char(buf, ch);
    }

	send_to_char("Item is: ", ch);
    sprintbitarray(GET_OBJ_EXTRA(obj), extra_bits, EF_ARRAY_MAX, buf);
    strcat(buf, "\r\n");
    send_to_char(buf, ch);

	    sprintf(buf, "Weight: %d, Value: %d, Rent: %d\r\n",
        GET_OBJ_WEIGHT(obj), GET_OBJ_COST(obj), GET_OBJ_RENT(obj));
    send_to_char(buf, ch);

    /* show them item values. */
    oPrintValues(obj, buf);
    sendChar(ch, "%s\r\n", buf);

    /* And now applies. */
    found = FALSE;
    for (i = 0; i < MAX_OBJ_AFFECT; i++) {
      if ((obj->affected[i].location != APPLY_NONE) &&
      (obj->affected[i].modifier != 0)) {
        if (!found) {
          send_to_char("Can affect you as :\r\n", ch);
          found = TRUE;
        }
        sprinttype(obj->affected[i].location, apply_types, buf2, sizeof(buf2));
        sprintf(buf, "   Affects: %s By %d\r\n", buf2, obj->affected[i].modifier);
        send_to_char(buf, ch);
      }
    }
    /* Now special procedures. */
    if (IS_MAGIC_USER(ch)) {
    	if ( obj->item_number >= 0 &&
	     obj_index[obj->item_number].combatSpec &&
	     GET_LEVEL(ch) >= 30 &&
	     skillSuccess(ch, SPELL_IDENTIFY)) {
	    combSpecName(obj_index[obj->item_number].combatSpec, buf);
	    sendChar(ch, "You sense this item has the following arcane power: %s\r\n", buf);
	}
    } /* lvl 30 mages can detect special routines. */

}

ACMD(do_stat)
{
	if (GET_LEVEL(ch) >= LVL_IMMORT)
		perform_immort_stat(ch, argument);
	else
		perform_mortal_stat(ch, argument);

}
