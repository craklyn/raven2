
#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/handler.h"
#include "general/structs.h"
#include "general/class.h"
#include "general/comm.h"
#include "actions/interpreter.h"
#include "magic/skills.h"
#include "magic/spells.h"
#include "util/utils.h"
#include "util/weather.h"
#include "magic/sing.h"

extern void look_at_char(CharData * i, CharData * ch);

ACMD(do_spy)
{
  CharData *found_char = NULL;
  CharData *witness = world[ch->in_room].people;
  char arg2[MAX_INPUT_LENGTH];

  half_chop(argument, arg, arg2);

  if (GET_POS(ch) < POS_SLEEPING)
    send_to_char("You can't see anything but stars!\r\n", ch);
  else if (IS_AFFECTED(ch, AFF_BLIND))
    send_to_char("You can't see a damned thing, you're blind!\r\n", ch);
  else if (!*arg)
    send_to_char("Spy on who?\r\n", ch);
  else {
    if (!GET_SKILL(ch, SKILL_SPY)) {
      send_to_char("Maybe you should just use look.\r\n", ch);
      return;
    }
    if (GET_MOVE(ch) < 5) {
      send_to_char("You are too tired to be furtive.\r\n", ch);
      return;
    }
    GET_MOVE(ch) -= 5;
    advanceSkill(ch, SKILL_SPY, 90, "You feel more secretive!", 0, 0, 0);

    found_char = get_char_room_vis(ch, arg);

    if (found_char == NULL)
      send_to_char("You don't see that person here.\r\n", ch);
    else {
      char msgbuf[500];

      sprintf(msgbuf, "You notice that %s is eyeing up %s suspiciously.\r\n",
          GET_NAME(ch), GET_NAME(found_char));
      look_at_char(found_char, ch);
      while (witness) {
        if ((!skillSuccess(ch, SKILL_SPY) || number(6,40) < GET_WIS(witness)) && AWAKE(witness) && CAN_SEE(witness, ch)) {
          REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_HIDE);
          if (witness == found_char)
            act("You notice that $N is eyeing you up suspiciously.",
                TRUE, found_char, 0, ch, TO_CHAR);
          else if (witness != ch) send_to_char(msgbuf, witness);
        }
        witness = witness->next_in_room;
      }
    }
  }
}
