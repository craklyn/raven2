
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

#define STUN_MIN        2
#define STUN_MAX        4
#define THIS_SKILL      SKILL_DISTRACT

#define SKILL_ADVANCE_STRING "You have become even more distracting!"

extern void look_at_char(CharData * i, CharData * ch);

ACMD(do_distract)
{
  CharData *victim = NULL;
  char arg2[MAX_INPUT_LENGTH];

  IF_UNLEARNED_SKILL("You make a lot of noise, without any effect.");

  half_chop(argument, arg, arg2);

  if (FIGHTING(ch))
    send_to_char("You are too busy fighting to distract anyone.\r\n", ch);
  else if (!*arg)
    send_to_char("Who do you want to distract?\r\n", ch);
  else {
    victim = get_char_room_vis(ch, arg);

    if (victim == NULL)
      send_to_char("You don't see that person here.\r\n", ch);
    else if (victim == ch)
      send_to_char("You wonder who made that loud noise.\r\n", ch);
    else if (IS_AFFECTED(victim, AFF_DISTRACT))
      send_to_char("They already seem to be distracted.\r\n", ch);
    else {
      if (GET_POS(ch) <= POS_SLEEPING) {
        send_to_char("That person isn't even awake!\r\n", ch);
      } else if (CAN_SEE(victim, ch) && skillSuccess(ch, SKILL_DISTRACT)) {
        int flags = MOB_PREDATOR | MOB_AGGRESSIVE;

        advanceSkill(ch, THIS_SKILL, 90, SKILL_ADVANCE_STRING, 0, 0, 0);
        add_affect(ch, victim, SKILL_DISTRACT, 0, APPLY_NONE, 0, 1 TICKS,
            AFF_DISTRACT, FALSE, FALSE, FALSE, FALSE);
        add_affect(ch, ch, SKILL_DISTRACT, 0, APPLY_NONE, 0, 1 TICKS,
            AFF_DISTRACT, FALSE, FALSE, FALSE, FALSE);
        act("You make a loud noise, distracting $N's attention.", FALSE, ch, 0,
            victim, TO_CHAR);
        act("You are distracted by a loud noise nearby.", FALSE, ch, 0,
            victim, TO_VICT);
        act("$N is distracted by a loud noise nearby.", FALSE, ch, 0,
            victim, TO_NOTVICT);
		if(IS_TRICKSTER(ch) && GET_ADVANCE_LEVEL(ch) >= 3 && percentSuccess(20)) {
			add_affect(ch, victim, SPELL_CONFUSION, 0, APPLY_NONE, 0, 1 TICKS, 0, FALSE, FALSE, FALSE, FALSE);
			sendChar(victim, "You aren't sure, but something just isn't right.  It's going to bother you until you figure it out...\r\n");
		}
		if (IS_GOOD(ch)) flags |= MOB_AGGR_GOOD;
        else if (IS_EVIL(ch)) flags |= MOB_AGGR_EVIL;
        else flags |= MOB_AGGR_NEUTRAL;
        if (MOB_FLAGGED(victim, flags))
          damage(ch, victim, 0, THIS_SKILL);
        STUN_USER_MIN;
      } else {
        act("You make a loud noise.", FALSE, ch, 0, victim, TO_CHAR);
        STUN_USER_MAX;
      }
    }
  }
}
