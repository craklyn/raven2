/*
**++
**  RCSID:     $Id: epic.c,v 1.2 2003/11/07 02:48:43 raven Exp $
**
**  FACILITY:  RavenMUD
**
**  LEGAL MUMBO JUMBO:
**
**      This is based on code developed for DIKU and Circle MUDs.
**
**  MODULE DESCRIPTION:
**
**      Epic skills and spells for Veteran and Legend characters.
**
**  AUTHORS:
**
**      Digger from RavenMUD
**      Imhotep from RavenMUD
**
**  NOTES:
**
**      Use 132 column editing in here.
**
**--
*/

#include "general/conf.h"
#include "general/sysdep.h"

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
#include "actions/fight.h"
#include "actions/act.h"          /* ACMDs located within the act*.c files */

#define SKILL_MAX_LEARN        90
#define DEX_AFFECTS            FALSE
#define INT_AFFECTS            FALSE
#define WIS_AFFECTS            FALSE

#define STUN_MIN               1
#define STUN_MAX               1

#define THIS_SKILL              SKILL_BATTLECRY
#define SKILL_ADVANCE_STRING    "The earth trembles with the strength of " \
                                "your battlecry."
ACMD(do_battlecry)
{
    int move_cost = 50;

    IF_UNLEARNED_SKILL("You let out a hoarse shout.\r\n" );

    if (IS_NPC(ch)) return;

    if (GET_MOVE(ch) < move_cost) {
        send_to_char("You're too exhausted to do that.\r\n", ch);
        return;
    }

    GET_MOVE(ch) -= move_cost;

    if (skillSuccess(ch, THIS_SKILL)) {
        act("$n lets forth a mighty roar!", FALSE, ch, 0, 0, TO_ROOM);
        send_to_char("You let forth a mighty roar!\r\n", ch);
        advanceSkill(ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING,
                DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS);
        /* we will just pretend it's a spell, sshh now */
        mag_groups(GET_LEVEL(ch), ch, THIS_SKILL, 0);
    } else {
        send_to_char("You fail to think of anything to shout.\r\n", ch);
    }

    STUN_USER_RANGE;
}

#undef THIS_SKILL
#undef SKILL_ADVANCE_STRING

#define THIS_SKILL              SKILL_WARCRY
#define SKILL_ADVANCE_STRING    "Your fierce warcry is heard in Valhalla!"
ACMD(do_warcry)
{
    int move_cost = 10;
    CharData *v;

    IF_UNLEARNED_SKILL("You let out a hoarse shout.\r\n" );

    if (IS_NPC(ch)) return;

    if (IS_SET_AR(ROOM_FLAGS(ch->in_room), ROOM_PEACEFUL)) {
        send_to_char("You shout and shout, but everyone ignores you.\r\n", ch);
        return;
    }

    if (GET_MOVE(ch) < move_cost) {
        send_to_char("You're too exhausted to do that.\r\n", ch);
        return;
    }

    GET_MOVE(ch) -= move_cost;

    if (skillSuccess(ch, THIS_SKILL)) {
        act("$n lets forth a fierce roar!", FALSE, ch, 0, 0, TO_ROOM);
        send_to_char("You let forth a fierce roar!\r\n", ch);
        advanceSkill(ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING,
                DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS);

        /* anyone not grouped now fights you! */
        for (v = world[ch->in_room].people; v; v = v->next_in_room) {
            if (in_same_group(ch, v)) continue;
            if (GET_POS(v) <= POS_SLEEPING) continue;
            if (!IS_NPC(v) && GET_LEVEL(v) > MAX_MORTAL) continue;
            if (skillSuccess(ch, THIS_SKILL)) {
                act("$N responds to your warcry!", FALSE, ch, 0, v, TO_CHAR);
                act("You respond to $N's warcry!", FALSE, v, 0, ch, TO_CHAR);
                damage(ch, v, 0, THIS_SKILL);
                FIGHTING(v) = ch;
            }
        }
    } else {
        send_to_char("Your shout sounds more like a whimper.\r\n", ch);
    }

    STUN_USER_RANGE;
}

#undef THIS_SKILL
#undef SKILL_ADVANCE_STRING

#define THIS_SKILL              SKILL_STANCE
#define SKILL_ADVANCE_STRING    "Assuming a stance feels more natural now."
ACMD(do_stance)
{
    char arg[MAX_INPUT_LENGTH];
    int move_cost = 15;
    int i;

    IF_UNLEARNED_SKILL("Huh!?\r\n" );

    if (IS_NPC(ch)) return;

    one_argument(argument, arg);

    if (!*arg) {
        sendChar(ch, "Your stance is currently %s.\r\n", stances[ch->stance]);
        return;
    }

    for (i = 0; i < NUM_STANCES; i++)
        if (is_abbrev(arg, stances[i])) break;

    if (i < NUM_STANCES) {
        if (ch->stance == STANCE_NEUTRAL) move_cost = 0;
        if (GET_MOVE(ch) < move_cost) {
            sendChar(ch, "You are too tired to assume that stance.\r\n");
            return;
        }
        if (ch->stance == i) {
            sendChar(ch, "You are already in that stance!\r\n");
            return;
        }
        GET_MOVE(ch) -= move_cost;
        if (skillSuccess(ch, THIS_SKILL)) {
            ch->stance = i;
            sendChar(ch, "Your stance is now %s.\r\n", stances[i]);
            advanceSkill(ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING,
                    DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS);
        } else {
            sendChar(ch, "You fail to assume the stance.\r\n");
        }
        STUN_USER_MIN;
    } else {
        sendChar(ch, "What stance is that?  Your choices are:\r\n");
        for (i = 0; i < NUM_STANCES; i++) {
            sendChar(ch, "  %s\r\n", stances[i]);
        }
    }
}

#undef THIS_SKILL
#undef SKILL_ADVANCE_STRING

#define THIS_SKILL              SKILL_POWERSTRIKE
#define SKILL_ADVANCE_STRING    "You gain a deeper understanding of your chi."
ACMD(do_powerstrike)
{
    char arg[MAX_INPUT_LENGTH];

    IF_UNLEARNED_SKILL("Huh!?\r\n" );

    if (IS_NPC(ch)) return;

    one_argument(argument, arg);

    if (!*arg) {
        if (ch->powerstrike) {
            sendChar(ch, "Your powerstrike level is currently %d.\r\n",
                    ch->powerstrike);
        } else {
            sendChar(ch, "You are not currently using powerstrike.\r\n");
        }
        return;
    }

    if (!strcmp(arg, "off") || !strcmp(arg, "0")) {
        sendChar(ch, "You will no longer use powerstrike.\r\n");
        ch->powerstrike = 0;
    } else if (isdigit(*arg)) {
        int level = atoi(arg);
        if (level > 50 || level > GET_MANA(ch)) {
            sendChar(ch, "You cannot call on that much power.\r\n");
        } else if (skillSuccess(ch, THIS_SKILL)) {
            sendChar(ch, "You concentrate your chi into your fists!\r\n");
            advanceSkill(ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING,
                    DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS);
            ch->powerstrike = level;
            GET_MANA(ch) -= level;
        } else {
            sendChar(ch, "You fail to concentrate your chi.\r\n");
            GET_MANA(ch) -= level;
        }
        STUN_USER_MIN;
    } else {
        sendChar(ch, "What level of powerstrike do you want?\r\n");
    }
}
