
#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/handler.h"
#include "actions/act.h"          /* ACMDs located within the act*.c files */
#include "general/comm.h"
#include "actions/interpreter.h"
#include "magic/spells.h"
#include "util/utils.h"
#include "specials/special.h"
#include "actions/fight.h"
#include "actions/holidays.h"

extern int zombies;

/* On St. Valentine's Day only, hugs might get you something nice */
ACMD(do_hug)
{
    time_t temp = time(NULL);
    struct tm *now = localtime(&temp);
    CharData *vict = NULL;
    ObjData *obj = NULL;
    int i;

    /* Always do the hug first */
    do_action(ch, argument, cmd, subcmd);

    /* End now if it's not V.Day, or if a mob hugged */
    if (IS_NPC(ch) || now->tm_mday != 14 || now->tm_mon != 1) return;

    /* Has this player loved this tick? */
    if (ch->tickstate & STATE_LOVER) return;

    /* Look for the named target */
    one_argument(argument, buf);
    if (*buf) vict = get_char_room_vis(ch, buf);

    /* Not found?  Not an NPC?  No love?  Abort! */
    if (!vict || !IS_NPC(vict) || percentSuccess(50)) return;

	if (chIsNonCombatant(vict))
	{
		sprintf(buf2, "Your idealist mind tricks won't work on me, %s.  Only money works on me.",
                    GET_NAME(ch));
				do_say(vict, buf2, 0, 0);
		return;
	}


    /* The player is now a lover! */
    ch->tickstate |= STATE_LOVER;

    /* randomly pick something to do: */
    switch (number(1,5)) {
        case 1: /* In rare cases, the mob will befriend you */
            if (percentSuccess(15) && !IS_AFFECTED(vict, AFF_CHARM) &&
                    !MOB_FLAGGED(vict, MOB_SENTINEL)) {
                act("$N gazes at $n adoringly.", TRUE, ch, 0, vict, TO_ROOM);
                act("$N gazes at you adoringly.", TRUE, ch, 0, vict, TO_CHAR);
                if (vict->master) stop_follower(vict);
                add_follower(vict, ch);
                add_affect(ch, vict, SKILL_BEFRIEND, GET_LEVEL(ch),
                        APPLY_NONE, 0, 2 TICKS, 0, FALSE, FALSE, FALSE, FALSE);
                REMOVE_BIT_AR(MOB_FLAGS(vict), MOB_SPEC);
                break;
            }
            /* else, fall through */
        case 2: /* They'll often want to give you all their money */
            if (GET_GOLD(vict) > 0) {
                act("$N gives $n all $S gold!", TRUE, ch, 0, vict, TO_ROOM);
                act("$N gives you all $S gold!", TRUE, ch, 0, vict, TO_CHAR);
                sendChar(ch, "There were %d coins.\r\n", GET_GOLD(vict));
                GET_GOLD(ch) += GET_GOLD(vict);
                GET_GOLD(vict) = 0;
                break;
            }
            /* else, fall through */
        case 3: /* They could just giggle and blush */
            act("$n giggles and blushes.", TRUE, vict, 0, 0, TO_ROOM);
            break;
        case 4: /* They might try to give you some of their eq */
            if (vict->carrying) {
                obj_from_char(obj = vict->carrying);
            } else {
                for (i = 0; !obj && i < NUM_WEARS; i++) {
                    if (vict->equipment[i]) {
                        obj = unequip_char(vict, i);
                    }
                }
            }
            /* Fall through ... */
        case 5: /* They might give you candy hearts */
            if (!obj) {
                obj = read_perfect_object(1650, VIRTUAL);
                if (!obj) return;
                SET_BIT_AR(obj->obj_flags.extra_flags, ITEM_TIMED);
                GET_OBJ_TIMER(obj) = 5;
            }
            obj_to_char(obj, ch);
            act("$N gives $n $p.", TRUE, ch, obj, vict, TO_ROOM);
            act("$N gives you $p.", TRUE, ch, obj, vict, TO_CHAR);
            break;
    }
}


bool shouldMakeZombie() {
    time_t temp = time(NULL);
    struct tm *now = localtime(&temp);

    // If an imm turns on zombies...
    if(number(1,100) <= zombies) {
        return 1;
    }

    /* End now if it's not Halloween */
    if(now->tm_mday != 31 || now->tm_mon != 9) {
        return 0;
    }
    else {
        if(percentSuccess(15)) {
            return 1;
        }
    }

    return 0;
}


