/**************************************************************************
*   File: suffer.c                                      Part of RavenMUD  *
*  Usage: Source file for Suffering code                                  *
*   Date: 09-Aug-00                                                       *
*  All rights reserved.  Based on CircleMUD for RavenMUD by Mortius       *
**************************************************************************/

#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "util/utils.h"
#include "general/comm.h"
#include "actions/interpreter.h"
#include "general/handler.h"
#include "general/color.h"
#include "general/class.h"
#include "actions/act.h"          /* ACMDs located within the act*.c files */
#include "actions/suffer.h"

/* external functions */
ObjData *die( CharData *ch, CharData *killer, int pkill);


void suffer_activity(int pulse)
{
  struct char_data *ch, *next_ch;
  char *desc;
  char dam_msg1[120] = "msg1 for suffer room not defined, report this to an imm!\r\n";
  char dam_msg2[120] = "msg2 for suffer room not defined, report this to an imm!\r\n";
  char dam_msg3[120] = "msg3 for suffer room not defined, report this to an imm!\r\n";
  char dam_msg4[120] = "msg4 for suffer room not defined, report this to an imm!\r\n";
  char death_char[120] = "You drift into coma and die.";
  char death_room[120] = "$n has drifted into coma and dies.";
  char suffer_msg[120] = "$n is suffering.";
  int percent_hp;
  int min_dam = 10, max_dam = 20, dam_amount;
  int hot = FALSE, cold = FALSE , dry = FALSE;

  for (ch = character_list; ch; ch = next_ch) {
       next_ch = ch->next;

      if (GET_LEVEL(ch) >= LVL_IMMORT) /* Lets not hurt our immortals */
          continue;

      if (IS_NPC(ch))
          continue;

      // I'm adding in a check to make outlaws "suffer" if in a peace room
      // Added by Sanji
      if( (ROOM_FLAGGED(IN_ROOM(ch), ROOM_PEACEFUL) || IN_ARENA(ch) ||  IS_SET_AR(ROOM_FLAGS((ch)->in_room), ROOM_CLAN)) &&
        (IS_SET_AR(PLR_FLAGS(ch), PLR_HUNTED) || IS_SET_AR(PLR_FLAGS(ch), PLR_THIEF))) {
            int to_room = 18001;	// Samsera center

            // hurt the little cheater
            GET_HIT(ch)  = MAX(1, GET_HIT(ch) - GET_MAX_HIT(ch)/8);
            GET_MANA(ch) = MAX(0, GET_MANA(ch) - GET_MAX_MANA(ch)/8);
            GET_MOVE(ch) = MAX(0, GET_MOVE(ch) - GET_MAX_MOVE(ch)/8);

            // where to send the victim?
            //do
            //{
            //    to_room = number( 0, top_of_world );
            //} while (IS_SET_AR(world[to_room].room_flags, ROOM_PRIVATE) ||
            //        IS_SET_AR(world[to_room].room_flags, ROOM_DEATH) ||
            //        IS_SET_AR(world[to_room].room_flags, ROOM_PEACEFUL));

            // Lets send out a message, more can be added later.
            //
            send_to_char("Naughty naughty.  No hiding!\r\n", ch);

            // Time to move the victim
            char_from_room(ch);
            char_to_room(ch, real_room(to_room));
            look_at_room(ch, 0);
      }

      hot = (ROOM_FLAGGED(IN_ROOM(ch), ROOM_HOT));
      cold = (ROOM_FLAGGED(IN_ROOM(ch), ROOM_COLD));
      dry = (ROOM_FLAGGED(IN_ROOM(ch), ROOM_DRY));

      if (!hot && !cold && !dry)
          continue;


      /* Getting ready to pull the damage amount out of the extra desc in the room */
      if ((desc = find_exdesc("min_dam", world[ch->in_room].ex_description)) != NULL)
           min_dam = atoi(desc);
      else
           min_dam = 0;

      if ((desc = find_exdesc("max_dam", world[ch->in_room].ex_description)) != NULL)
          max_dam = atoi(desc);
      else
          max_dam = 25;

     if (max_dam < min_dam) {
         send_to_char("SYSERR: Max damage lower then Min damage.  Report to an Immortal.\r\n", ch);
         return;
     }

     dam_amount = number(min_dam, max_dam);

     if ((desc = find_exdesc("suffer1", world[ch->in_room].ex_description)) != NULL)
          sprintf(dam_msg1, "%s", desc);
     if ((desc = find_exdesc("suffer2", world[ch->in_room].ex_description)) != NULL)
          sprintf(dam_msg2, "%s", desc);
     if ((desc = find_exdesc("suffer3", world[ch->in_room].ex_description)) != NULL)
          sprintf(dam_msg3, "%s", desc);
     if ((desc = find_exdesc("suffer4", world[ch->in_room].ex_description)) != NULL)
	  sprintf(dam_msg4, "%s", desc);
     if ((desc = find_exdesc("death_char", world[ch->in_room].ex_description)) != NULL)
          sprintf(death_char, "%s", desc);
     if ((desc = find_exdesc("death_room", world[ch->in_room].ex_description)) != NULL)
          sprintf(death_room, "%s", desc);
     if ((desc = find_exdesc("suffer_msg", world[ch->in_room].ex_description)) != NULL)
          sprintf(suffer_msg, "%s", desc);


     if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_COLD) && IS_AFFECTED(ch, AFF_NO_COLD))
         continue;
     else if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_HOT) && IS_AFFECTED(ch, AFF_NO_HOT))
         continue;
     else if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_HOT) && IS_AFFECTED(ch, AFF_NO_DRY))
         continue;

    // DUH! added a check for soak
     if (((ROOM_FLAGGED(IN_ROOM(ch), ROOM_HOT) || ROOM_FLAGGED(IN_ROOM(ch), ROOM_DRY)) && (GET_COND(ch, THIRST) > 0)))
         GET_COND(ch, THIRST) = 0;


     if (dam_amount >= GET_HIT(ch))
         dam_amount = GET_HIT(ch) + number(4, 7);
     else
        act(suffer_msg, FALSE, ch, 0, 0, TO_ROOM);

     GET_HIT(ch) -= dam_amount;

     percent_hp = (100 * GET_HIT(ch)) / GET_MAX_HIT(ch);
// need a check here to see if the char is still in the room!
   if ((ROOM_FLAGGED(IN_ROOM(ch), ROOM_HOT)) || (ROOM_FLAGGED(IN_ROOM(ch), ROOM_COLD)) || (ROOM_FLAGGED(IN_ROOM(ch), ROOM_DRY))){
     if (percent_hp > 75)
         send_to_char(dam_msg1, ch);
     else if (percent_hp > 50)
              send_to_char(dam_msg2, ch);
     else if (percent_hp > 30)
              send_to_char(dam_msg3, ch);
     else if (percent_hp >= 15)
              send_to_char(dam_msg4, ch);
     else if (percent_hp >= 1) {
	      GET_HIT(ch) = number (-3, -5);
              send_to_char("You have passed out.\r\n", ch);
	      act("$n has passed out and fallen to the ground.",
		   FALSE, ch, 0, 0, TO_ROOM);
              update_pos(ch);
     } else {
          GET_HIT(ch) = number(-7, -10);
          act(death_char, FALSE, ch, 0, 0, TO_CHAR);
          act(death_room, FALSE, ch, 0, 0, TO_ROOM);
          update_pos(ch);
          GET_MANA(ch) = 1;
          die(ch, NULL, 0);
     }
    }
  } /* End of for loop */
}
