/**************************************************************************
 *  File: bloodbowl.c                                     Part of RavenMUD *
 *  Usage: Special procedures for bloodbowl     area.                      *
 *                                                                         *
 *  All rights reserved.  See license for complete information.            *
 *                                                                         *
 *  Special procedures for bloodbowl by unknown RavenMUD author.           *
 **************************************************************************/
#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "specials/bloodbowl.h"
#include "general/class.h"
#include "general/comm.h"
#include "general/handler.h"
#include "actions/interpreter.h"
#include "util/utils.h"

/* Local functions */
void    bloodLog( CharData*, int );
ObjData *chIsBallCarrier( CharData* ch );
void    send_to_questors(char *msg);

char *refText[] ={
    "The Referee swats you on the can and sends you back into the fray.\n",
    "The Referee says, 'Go win one for The Gipper!'\n",
    "The Referee says, 'Look at it this way - you can't do much worse!'\n",
    "The Referee does a quick cup check, snickers, and sends you back in.\n"
};
#define REFTEXT_ENTRIES (sizeof( refText ) / sizeof( refText[0] ))

int
bloodref(CharData *ref, void *me, int cmd, char *argument) {
    CharData* victim;

    // We do not process commands in this loop.
    //
    if (cmd != 0) {
        return (0);
    }

    // Check the rooms for players and their current tick delays.
    //
    for (victim = world[ref->in_room].people;
            victim != NULL;
            victim = victim->next_in_room) {
        if (victim != ref && GET_LEVEL(victim) <= LVL_IMMORT) {
            mudlog(NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(ref)), TRUE, "BLOODBOWL: referee active - [%s:%d]",
                    GET_NAME(victim), ARENA_DLY(victim));
            if (ARENA_DLY(victim) <= 0) {
                int refMsg = (int) (random() % REFTEXT_ENTRIES);
                ARENA_DLY(victim) = 0;
                GET_HIT(victim) = GET_MAX_HIT(victim);
                GET_MANA(victim) = GET_MAX_MANA(victim);
                GET_MOVE(victim) = GET_MAX_MOVE(victim);
                GET_POS(victim) = POS_STANDING;
                sendChar(victim, refText[refMsg]);
                char_from_room(victim);
                char_to_room(victim, real_room(BLOODBOWL_START));
                look_at_room(victim, 0);

                // Modifying one of the loop control vars is typically something to
                // avoid, but in this case if we don't start the room search over,
                // we could end up in an endless loop.
                //
                victim = world[ref->in_room].people;
            } else {
                ARENA_DLY(victim) -= 1;
            }
        }
    }

    return (0);
}
/* Check and see if they're in the zone */
int
inBloodBowl(CharData* ch) {
    if (IN_ROOM(ch) >= real_room(BLOODBOWL_TOP) &&
            IN_ROOM(ch) <= real_room(BLOODBOWL_BOT)) {
        return (1);
    }
    else {
        return (0);
    }
}

/* check for blood ball */
ObjData*
chIsBallCarrier(CharData* ch) {
    return ( get_obj_in_list_vis(ch, "bloodball", ch->carrying));
}

void
bloodLog(CharData *victim, int ballCarrier) {
    static char *black = "&05(black)&00";
    static char *gold = "&03(gold)&00";
    static char *none = "&10(no team)&00";

    DescriptorData *questor;
    char qBuf[256] = "";

    char *lColor = (PRF_FLAGGED(victim, PRF_GOLD_TEAM) ? gold : black);

    // Check to make certain they were on teams.
    //
    if (!PRF_FLAGGED(victim, PRF_GOLD_TEAM) &&
            !PRF_FLAGGED(victim, PRF_BLACK_TEAM)) lColor = none;

    if (ballCarrier) {
        sprintf(qBuf, "&08BLOODBOWL:&00 %s is tackled, %s fumbled.&00\r\n\r\n",
                GET_NAME(victim), lColor);
    } else {
        sprintf(qBuf, "&08BLOODBOWL:&00 %s is tackled %s.&00\r\n\r\n",
                GET_NAME(victim), lColor);
    }
    /*
     ** Provide a little quest commentary.
     */

    for (questor = descriptor_list; questor; questor = questor->next) {
        if (!questor->connected &&
                questor->character &&
                PRF_FLAGGED(questor->character, PRF_QUEST))
            send_to_char(qBuf, questor->character);
    }
}

void
dieInBloodBowl(CharData* victim) {
    // Returns a pointer to the bloodball if this character
    // is the ball carrier.
    //
    ObjData* ball = chIsBallCarrier(victim);

    int targetRoom = real_room(30314);

    bloodLog(victim, (ball != NULL));

    if (ball != NULL) {
        mudlog(BRF, LVL_IMMORT, TRUE, "BLOODBOWL: %s fumbles the ball", GET_NAME(victim));

        /* Mortius, remove ballcarrier from title */
        set_title(victim, "");
        obj_from_char(ball);
        obj_to_room(ball, IN_ROOM(victim));
    }

    GET_HIT(victim) = 1;
    GET_MANA(victim) = 1;
    GET_MOVE(victim) = 1;
    GET_POS(victim) = POS_RESTING;

    char_from_room(victim);
    char_to_room(victim, targetRoom);
    look_at_room(victim, 0);

    ARENA_DLY(victim) = 10;
}

/* Mortius, adding a spec proc to the ball itself.  This will do the following.
 *
 * 1. When the ball is carried from room 30046 to 30305 a quest echo will say
 *    Mortius has scored a touchdown.
 * 2. When the ball is picked up off the ground a quest echo will say
 *    Mortius has recovered the ball.
 *
 * Defines will have to be added for what the status of the ball is at any given
 * time.
 *
 * When I have something other then PC telnet I will rewrite this.  God this is
 * driving me nuts.  I have 40 lines and a max of 80 chars *roll*
 *
 */

/* Note: the echo for pick up and drop have been moved to be more in line with how
 *       flag works
 */

/* Ball handling status */
#define NO_STATUS	0
#define TAKE_BALL	1
#define DROP_BALL	2
#define GIVE_BALL	3
#define THROW_BALL	4
#define ON_WAY_TO_TD	5

SPECIAL(blood_ball) {
    char playername[30] = "";
    char objname[30] = "";
    char victim_name[30] = "";
    CharData *victim = NULL;
    ObjData *obj = NULL;
    int ball_status = NO_STATUS;
    static char *black = "&05(black)&00";
    static char *gold = "&03(gold)&00";
    static char *none = "&10(no team)&00";

    char *lColor = (PRF_FLAGGED(ch, PRF_GOLD_TEAM) ? gold : black);
    if (!PRF_FLAGGED(ch, PRF_GOLD_TEAM) && !PRF_FLAGGED(ch, PRF_BLACK_TEAM))
        lColor = none;

    argument = one_argument(argument, objname);
    /* First work out who the player is */
    sprintf(playername, "%s",
            GET_NAME(ch));

    /* This is a god damn mess.  This 80 char windows telnet crap is really
       starting to piss me off */

    if (CMD_IS("give"))
        ball_status = GIVE_BALL;
    else if (CMD_IS("throw"))
        ball_status = THROW_BALL;
    else if (CMD_IS("north") || CMD_IS("south") || CMD_IS("west") ||
            CMD_IS("east") || CMD_IS("up") || CMD_IS("down")) {
        if (EXIT(ch, cmd - 1) != NULL) {
            if (world[EXIT(ch, cmd - 1)->to_room].number == 30046) {

                obj = get_obj_in_list_vis(ch, "ball", ch->carrying);

                if (obj == NULL)
                    return FALSE;

                obj->obj_flags.value[0] = ON_WAY_TO_TD;
                sprintf(buf, "&08BLOODBOWL:&00 %s %s MAY GO ALL THE WAY!\r\n",
                        playername, lColor);
                send_to_questors(buf);
                return FALSE;
            } else if (world[EXIT(ch, cmd - 1)->to_room].number == 30305) {

                obj = get_obj_in_list_vis(ch, "ball", ch->carrying);

                if (obj == NULL)
                    return FALSE;

                if (obj->obj_flags.value[0] != ON_WAY_TO_TD)
                    return FALSE;

                sprintf(buf, "&08BLOODBOWL:&00 %s %s just scored a TOUCH DOWN!!!\r\n",
                        playername, lColor);
                send_to_questors(buf);

                obj->obj_flags.value[0] = NO_STATUS;
                return FALSE;
            }
        }
    } else
        return FALSE;



    /* Time to do something with the ball */

    if (ball_status > 0) {

        /* Lets check its the ball and nothing else */
        if (ball_status == TAKE_BALL)
            obj = get_obj_in_list_vis(ch, objname, world[ch->in_room].contents);
        else
            obj = get_obj_in_list_vis(ch, objname, ch->carrying);

        if (obj == NULL)
            return FALSE;

        /* Check the vnum to see if it matches the balll */
        if (GET_OBJ_VNUM(obj) != 30000)
            return FALSE;

        /* We know its the ball that the player is trying to do something with */
        if (ball_status == GIVE_BALL) {
            argument = one_argument(argument, victim_name);
            victim = get_char_room_vis(ch, victim_name);

            if (victim == NULL)
                return TRUE;

            set_title(ch, "");
            set_title(victim, "&08(BallCarrier)&00");

            sprintf(buf, "&08BLOODBOWL;&00 %s %s has handed off the ball to %s.\r\n",
                    playername, lColor, GET_NAME(victim));
            send_to_questors(buf);
            return FALSE;
        } else
            if (ball_status == THROW_BALL) {
            argument = one_argument(argument, victim_name);
            victim = get_char_vis(ch, victim_name, 0);

            if (victim == NULL)
                return TRUE;

            set_title(ch, "");
            set_title(victim, "&08(BallCarrier)&00");

            sprintf(buf, "&08BLOODBOWL;&00 %s %s passes the ball to %s.\r\n",
                    playername, lColor, GET_NAME(victim));
            send_to_questors(buf);
            return FALSE;
        }
    }
    return FALSE;
}

/* Mortius, send a message to all playes on quest channel */

void send_to_questors(char *msg) {
    DescriptorData *questor;

    for (questor = descriptor_list; questor; questor = questor->next) {
        if (!questor->connected &&
                questor->character &&
                PRF_FLAGGED(questor->character, PRF_QUEST))
            send_to_char(msg, questor->character);
    }
}
