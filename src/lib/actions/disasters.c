/**************************************************************************
*   File: disaster.c                                    Part of RavenMUD  *
*  Usage: Disaster rooms/zones                                            *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*									  *
* Base On CircleMUD By Mortius of RavenMUD : 08-Aug-00			  *
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
#include "actions/disasters.h"

/* external functions */
ObjData *die( CharData *ch, CharData *killer, int pkill);

void disaster_activity(int pulse)
{
  struct char_data *ch, *next_ch;
  int dam;

  for (ch = character_list; ch; ch = next_ch) {
       next_ch = ch->next;

       if (!ch)         /* If no char lets check the next one */
           continue;

       if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_NO_DISASTER)) /* Safe Room? */
           continue;

       if (!OUTSIDE(ch)) /* We don't have disasters indoors */
           continue;

       dam = number(1, (GET_MAX_HIT(ch) + 73));  /* Lets work out damage */

       if (IS_AFFECTED(ch, AFF_SANCTUARY))
           dam = MIN(dam, 18);  /* Max 18 damage when sanctuary */
           dam = MIN(dam, 300);  /* no more than 100 hp per round */
           dam = MAX(dam, 20);    /* no less than 0 hp per round */

       /* We don't damage immortals or mobs, maybe we should make it damage
          charmed mobs??? */

       if (((GET_LEVEL(ch) >= LVL_IMMORT) && !IS_NPC(ch)))
             continue; /* Lets do nothing to imms */

       if (IS_NPC(ch))
           dam = 0;

       /* These send the player into the disaster funcation, along with dam */

       if (ZONE_FLAGGED(world[ch->in_room].zone, ZONE_DIS_FIREBALL) || 
	   ROOM_FLAGGED(IN_ROOM(ch), ROOM_DISASTER_FIREBALL))
           disaster_fireball(ch, dam);

       if (ZONE_FLAGGED(world[ch->in_room].zone, ZONE_DIS_LIGHTNING) ||
           ROOM_FLAGGED(IN_ROOM(ch), ROOM_DISASTER_LIGHTNING))
	   disaster_lightning(ch, dam);
  }
}

void disaster_fireball(struct char_data *ch, int dam)
{
  int num = number(1, 100), close = FALSE, hit = FALSE;
  int knock_down = 15, near_miss = 35, random = number(1, 100);

  if (random < 40) /* Make it a little more random */
      return;

  if (num < knock_down)
      hit = TRUE;
  else if (num < near_miss)
           close = TRUE;

  if (hit == FALSE)  {
      if (close == TRUE) {
          act("&03A &01fireball&03 hits the ground bursting into &01flames&03.&00",
               FALSE, ch, 0, 0, TO_CHAR);
          return;
      } else {
          send_to_char("&03You see a &00glowing&03 ball of &01flames&03 fly past.&00\r\n", ch);
          return;
      }
  }

  GET_HIT(ch) -= dam;

  act("$n &03is knocked to the ground by a small &01fireball&03.&00",
       TRUE, ch, 0, 0, TO_ROOM);
  act("&03You are knocked to the ground by a small &01fireball&03.&00",
       FALSE, ch, 0, 0, TO_CHAR);

  if (dam > (GET_MAX_HIT(ch) >> 2))
      send_to_char("&03That really did HURT!&00\r\n", ch);

  GET_POS(ch) = POS_SITTING;
  update_pos(ch);

  if (GET_POS(ch) == POS_DEAD) {
      mlog("Fireball killed %s", GET_NAME(ch));
      die(ch, NULL, 0);
      return;
  }
}

void disaster_lightning(struct char_data *ch, int dam)
{
  int num = number(1, 100), close = FALSE, hit = FALSE;
  int knock_down = 10, near_miss = 25, random = number(1, 100);

  if (random < 40) /* Make it a little more random */
      return;

  if (num < knock_down)
      hit = TRUE;
  else if (num < near_miss)
           close = TRUE;

  if (hit == FALSE)  {
      if (close == TRUE) {
          act("&03KAZAK! a lightning &04bolt &03hits nearby.&00",
               FALSE, ch, 0, 0, TO_CHAR);
          return;
      } else {
          send_to_char("&03You hear the clap of distant thunder.&00\r\n", ch);
          return;
      }
  }

  GET_HIT(ch) -= dam;

  act("&03KAZAK! a lightning &04bolt&03 hits $n&03.  You hear a sick sizzle.&00",
       TRUE, ch, 0, 0, TO_ROOM);
  act("&03KAZAK! a lightning &03bolt &03hits you.  You hear a sick sizzle.&00",
       FALSE, ch, 0, 0, TO_CHAR);

  if (dam > (GET_MAX_HIT(ch) >> 2))
      send_to_char("&03That really did HURT!&00\r\n", ch);

  update_pos(ch);

  if (GET_POS(ch) == POS_DEAD) {
      mlog("Lightning killed %s", GET_NAME(ch));
      die(ch, NULL, 0);
      return;
  }
}
