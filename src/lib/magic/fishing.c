
#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/comm.h"
#include "actions/interpreter.h"
#include "general/handler.h"
#include "actions/act.clan.h"
#include "actions/commact.h"
#include "magic/fishing.h"
#include "magic/spells.h"
#include "specials/boards.h"
#include "specials/mail.h"
#include "util/utils.h"
#include "util/weather.h"

/* Is the Great Raven Fish-Off in progress? */
int fishoff = 0;

ACMD(do_castout)
{
    ObjData *pole;
    int fail;

    if (PLR_FLAGGED(ch, PLR_FISHING)) {
        send_to_char("Your line is already in the water.\r\n", ch);
        return;
    }

    if (!(pole = GET_EQ(ch, WEAR_HOLD)) || GET_OBJ_TYPE(pole) != ITEM_POLE) {
        send_to_char("You need to be holding a fishing pole.\r\n", ch);
        return;
    }

    if (!ROOM_FLAGGED(IN_ROOM(ch), ROOM_SALTWATER_FISH) &&
        !ROOM_FLAGGED(IN_ROOM(ch), ROOM_FRESHWATER_FISH)) {
        send_to_char("This is not a good spot to fish from.\r\n", ch);
        return;
    }

    fail = number(1,10);
    if (fail <= 3) {
        send_to_char("You pull your arm back and try to cast out your line, "
                "but it gets all \r\ntangled up.\r\n", ch);
        act("$n pulls $s arm back, trying to cast $s fishing line out into the"
                "\r\nwater, but ends up just a bit tangled.\r\n",
                FALSE, ch, 0, 0, TO_ROOM);
        return;
    }

    /* Ok, now they've gone through the checks, now set them fishing */
    SET_BIT_AR(PLR_FLAGS(ch), PLR_FISHING);
    send_to_char("You cast your line out into the water.\r\n", ch);
    act("$n casts $s line out into the water.", FALSE, ch, 0, 0, TO_ROOM);
}

struct fish_type {
    char *name;
    int spell_lvl;
    int spell_num;
} salt_fishies[] = {
    {"halibut", 0, 0},
    {"trout", 0, 0},
    {"cod", 0, 0},
    {"snapper", 0, 0},
    {"sole", 0, 0},
    {"kipper", 0, 0},
    {"whiting", 0, 0},
    {"mackerel", 0, 0},
    {"bass", 0, 0},
    {"flathead", 0, 0},
    {"puffer", 50, SPELL_POISON},
    {"rainbow tuna", 50, SPELL_HEAL},
};

struct fish_type fresh_fishies [] = {
    {"red herring", 50, SPELL_HASTE},
    {"perch", 0, 0},
    {"herring", 0, 0},
    {"trout", 0, 0},
    {"catfish", 0, 0},
    {"cod", 0, 0},
    {"bass", 0, 0},
    {"sunfish", 0, 0},
    {"walleye", 0, 0},
    {"mullet", 0, 0},
    {"sturgeon", 0, 0},
    {"calico", 0, 0},
};

#define MAX_SALT_FISH   ((sizeof(salt_fishies)/sizeof(struct fish_type))-1)
#define MAX_FRESH_FISH  ((sizeof(fresh_fishies)/sizeof(struct fish_type))-1)

ACMD(do_reelin)
{
    int success;
    ObjData *fish = NULL;
    struct fish_type *template = NULL;

    if (!PLR_FLAGGED(ch, PLR_FISHING)) {
        send_to_char("You aren't even fishing!\r\n", ch);
        return;
    }
    if (!PLR_FLAGGED(ch, PLR_FISH_ON)) {
        send_to_char("You reel in your line, but alas... nothing on the end."
                "\r\n", ch);
        REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_FISHING);
        act("$n reels $s line in, but with nothing on the end.\r\n",
                FALSE, ch, 0, 0, TO_ROOM);
        return;
    }

    /* Ok, they are fishing and have a fish on */
    success = number(1, 10);

    REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_FISHING);
    REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_FISH_ON);

    if (success <= 6) {
        send_to_char("You reel in your line, putting up a good fight, but you "
                "lose him!\r\n", ch);
        act("$n reels $s line in, fighting with whatever is on the end, but "
                "loses the catch.\r\n", FALSE, ch, 0, 0, TO_ROOM);
        return;
    }

    if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_SALTWATER_FISH)) {
        template = &salt_fishies[number(0, MAX_SALT_FISH)];
    } else {
        template = &fresh_fishies[number(0, MAX_FRESH_FISH)];
    }

    fish = read_perfect_object(BASE_FISH, VIRTUAL);
    fish->in_room = NOWHERE;

    sprintf(buf, "fish %s", template->name);
    fish->name = str_dup(buf);

    sprintf(buf, "A fish flops helplessly on the ground, suffocating.");
    fish->description = str_dup(buf);

    sprintf(buf, "a %s", template->name);
    fish->short_description = str_dup(buf);

    GET_OBJ_TYPE(fish) = ITEM_FOOD;
    GET_OBJ_COST(fish) = number(10,40);
    GET_OBJ_RENT(fish) = 0;
    GET_OBJ_WEIGHT(fish) = number(1,18);
    GET_OBJ_TIMER(fish) = 50;

    SET_BIT_AR(GET_OBJ_WEAR(fish), ITEM_WEAR_TAKE);

    /* the Red Herring is special ... */
    if (template == fresh_fishies) {
        SET_BIT_AR(GET_OBJ_WEAR(fish), ITEM_WEAR_HOLD);
        SET_BIT_AR(GET_OBJ_AFFECT(fish), AFF_NOTRACK);
    }

    /* During a Fish-Off, fish get tagged */
    if (fishoff) {
        struct extra_descr_data *new_descr;

        CREATE(new_descr, struct extra_descr_data, 1);
        new_descr->keyword = str_dup("angler_tag");
        sprintf(buf, "Caught by %s", GET_NAME(ch));
        new_descr->description = str_dup(buf);
        new_descr->next = NULL;
        fish->ex_description = new_descr;
    }

    SET_BIT_AR(GET_OBJ_EXTRA(fish), ITEM_NOSELL);
    SET_BIT_AR(GET_OBJ_EXTRA(fish), ITEM_TIMED);
    SET_BIT_AR(GET_OBJ_EXTRA(fish), ITEM_NORENT);

    GET_OBJ_VAL(fish, 0) = GET_OBJ_WEIGHT(fish);
    GET_OBJ_VAL(fish, 1) = template->spell_lvl;
    GET_OBJ_VAL(fish, 2) = template->spell_num;
    GET_OBJ_VAL(fish, 3) = 0;

    sprintf(buf, "You reel in %s! Nice catch!\r\n",
        fish->short_description);
    act("Wow! $n reels in a helluva catch! Looks like $p!\r\n",
        FALSE, ch, fish, 0, TO_ROOM);
    send_to_char(buf, ch);
    obj_to_char(fish, ch);
}

void check_fishing(void)
{
    DescriptorData *d;
    int bite;

    for (d = descriptor_list; d; d = d->next) {
        CharData *ch = d->character;
        if (d->connected) continue;

        if (PLR_FLAGGED(ch, PLR_FISHING)) {
            if (!PLR_FLAGGED(ch, PLR_FISH_ON)) {
                bite = number(1, 10);

                if (bite >= 7 && bite <= 8) {
                    send_to_char("Time goes by... not even a nibble.\r\n", ch);
                } else if (bite >= 6) {
                    send_to_char("You feel a small jiggle on your line.\r\n",
                        ch);
                } else if (bite >= 4) {
                    send_to_char("You feel a solid pull on your line.\r\n", ch);
                    SET_BIT_AR(PLR_FLAGS(ch), PLR_FISH_ON);
                } else if (bite >= 2) {
                    send_to_char("Your line suddenly jumps to life, FISH ON!"
                        "\r\n", ch);
                    SET_BIT_AR(PLR_FLAGS(ch), PLR_FISH_ON);
                }
            } else {
                bite = number(1,10);

                if (bite < 4) {
                    send_to_char("Oh dear, looks like this one got away.\r\n",
                            ch);
                    REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_FISH_ON);
                }
            }
        }
    }
}

static void fishing_reward(char *who, int place)
{
    struct char_file_u tmp_store;
    CharData *i, *cbuf;
    int qp;

    if (place == 1) qp = 5;
    else if (place == 2) qp = 3;
    else qp = 1;

    /* find if this person is currently online */
    for (i = character_list; i; i = i->next)
        if (!IS_NPC(i) && !str_cmp(i->player.name, who)) break;

    if (i) {
        GET_QP(i) += qp;
        sendChar(i, "&10You came %d%s in the Great Fish-Off, and receive "
                "%dqp!\r\n&00", place, ndth(place), qp);
    } else {
        int player_i = load_char(who, &tmp_store);
        if (player_i == -1) {
            mudlog(NRM, LVL_IMMORT, TRUE, "Player %s doesn't exist to reward!", who);
        } else {
            CREATE(cbuf, CharData, 1);
            clear_char(cbuf);
            store_to_char(&tmp_store, cbuf);
            GET_QP(cbuf) += qp;
            char_to_store(cbuf, &tmp_store);
            fseek(player_fl, (player_i) * sizeof(struct char_file_u), SEEK_SET);
            fwrite(&tmp_store, sizeof(struct char_file_u), 1, player_fl);
            free_char(cbuf);
        }
    }
}

/* invariant: angler list remains sorted, highest weights first */
struct fishoff_angler {
    char *name;
    int totalweight;
    int fishies;
    struct fishoff_angler *next, *prev;
} *anglers = NULL;

void final_fishoff_score(void)
{
    struct fishoff_angler *angler, *last;
    char msg[MAX_MESSAGE_LENGTH];
    int ofs = 0, count = 0;
    ObjData *trophy;

    /* noone fished, just pack in */
    if (!anglers) {
        return;
    }

    ofs += sprintf(msg, "The Great Fish-Off for the year %d is now over.\r\n",
            time_info.year);
    ofs += sprintf(msg + ofs, "\r\nThe top ten fishers this year were:\r\n");
    ofs += sprintf(msg + ofs, "&13");
    ofs += sprintf(msg + ofs, "         Name         | Caught | Weight\r\n");
    ofs += sprintf(msg + ofs, "----------------------+--------+--------\r\n");
    ofs += sprintf(msg + ofs, "&00");
    for (angler = anglers; angler; angler = angler->next) {
        ofs += sprintf(msg + ofs, " %-20s &13|&00", angler->name);
        ofs += sprintf(msg + ofs, " %4d   &13|&00", angler->fishies);
        ofs += sprintf(msg + ofs, "%4d lb\r\n", angler->totalweight);
        fishing_reward(angler->name, ++count);
        if (count == 10) break;
    }

    /* create a board message on the mortal board (board #2) */
    board_add_message(2, "Fish-Off results", msg, "Fishmaster");

    /* create the new trophy */
    trophy = read_perfect_object(FISH_TROPHY, VIRTUAL);

    trophy->in_room = NOWHERE;
    trophy->worn_by = trophy->carried_by = NULL;
 
    /* ensure the types and values are correct */
    SET_BIT_AR(trophy->obj_flags.extra_flags, ITEM_TROPHY);
    GET_OBJ_TIMER(trophy) = TICKS_SO_FAR + TICKS_PER_MUD_YEAR;        /* expiry day */

    /* mail it to the rightful recipient */
    if (!postmaster_anonymous_mail(anglers->name, trophy)) {
        mudlog(NRM, LVL_IMMORT, TRUE, "Can't mail trophy to '%s'!", anglers->name);
        extract_obj(trophy);
    }

    /* and now free up the anglers list */
    for (angler = anglers; angler; angler = last) {
        last = angler->next;
        free(angler->name);
        free(angler);
    }
    anglers = NULL;
}

/* used to construct Fish-Off messages */
#define ANNOUNCE "&10The Fishmaster announces: &13"
#define ENDLINE "&00\r\n"

#define FO_ENDS (ANNOUNCE "The Great Fish-Off is now closed!" ENDLINE)
#define FO_WARN (ANNOUNCE "The Great Fish-Off closes in %d hours!" ENDLINE)
#define FO_1WARN (ANNOUNCE "The Great Fish-Off closes in one hour!" ENDLINE)
#define FO_START (ANNOUNCE "The Great Fish-Off has BEGUN!" ENDLINE)
#define FO_PREP (ANNOUNCE "The Great Fish-Off begins in %d hours!" ENDLINE)
#define FO_1PREP (ANNOUNCE "The Great Fish-Off begins in one hour!" ENDLINE)
#define FO_DAYS (ANNOUNCE "%d days remain in the Great Fish-Off!" ENDLINE)
#define FO_1DAY (ANNOUNCE "One day remains in the Great Fish-Off!" ENDLINE)

#define SAYS "The Fishmaster says, '"
#define ENDSAYS "'\r\n"
#define NTHFISH "That's your %d%s fish!"
#define TOTALWEIGHT "  A total weight of %d lbs!"

#define FO_WELCOME (SAYS "Welcome to the Great Fish-Off!" ENDSAYS)
#define FO_WEIGHT (SAYS NTHFISH TOTALWEIGHT ENDSAYS)
#define FO_FIRST (SAYS "You're now in first place!" ENDSAYS)

/* called once per tick to check on the fishoff status */
void fishoff_ticker(void)
{
    
    // If the fishoff will begin soon, let everyone know.
    if (!fishoff && (time_info.month % 4) == 3 && time_info.day == 34) {
        switch (time_info.hours) {
            case 23:
                gecho(FO_1PREP);
                break;
            case 22:
            case 21:
            case 20:
                gecho(FO_PREP, 24 - time_info.hours);
                break;
        }
        return;
    }
    
    // If it's the first month of the season, then begin a fishoff.
    if(!fishoff && (time_info.month % 4) == 0 && 
            time_info.day == 0 && time_info.hours == 0) {
        fishoff = 24;
        gecho(FO_START);
        return;
    }
    
    if(fishoff > 0) {
        fishoff -= 1;

        switch (fishoff) {
            case 4:
            case 3:
            case 2:
                gecho(FO_WARN, fishoff);
                break;
            case 1:
                gecho(FO_1WARN);
                break;
            case 0:
                gecho(FO_ENDS);
                fishoff = 0;
                final_fishoff_score();
                break;
        }      
    }
}



int check_fishing_give(CharData *from, CharData *to, ObjData *obj)
{
    struct fishoff_angler *angler = NULL, *last = NULL;
    char *name;

    /* make sure the fishoff is on (!) */
    if (!fishoff) return 0;

    /* make sure it's a fishmaster, a fish, and a trophy fish at that */
    if (GET_MOB_VNUM(to) != FISH_MASTER) return 0;

    if (GET_OBJ_VNUM(obj) != BASE_FISH) {
        do_say(to, "Um, no thanks!", 0, 0);
        return 0;
    }

    if (!obj->ex_description ||
            strcmp(obj->ex_description->keyword, "angler_tag")) {
        do_say(to, "Nice fish, but it was caught outside the tournament.", 0, 0);
        extract_obj(obj);
        return 1;
    }

    /* find out who caught it */
    name = obj->ex_description->description + 10;

    if (strcmp(name, GET_NAME(from))) {
        do_say(to, "Nice try, but you didn't catch THIS fish!", 0, 0);
        extract_obj(obj);
        return 1;
    }

    /* find or add them to the Great Fish-Off */
    for (angler = anglers; angler; angler = angler->next) {
        if (!strcmp(angler->name, GET_NAME(from))) break;
        last = angler;
    }

    /* best add 'em new */
    if (!angler) {
        CREATE(angler, struct fishoff_angler, 1);
        angler->prev = last;
        angler->next = NULL;
        angler->fishies = angler->totalweight = 0;
        angler->name = str_dup(GET_NAME(from));

        /* link them in */
        if (last) {
            last->next = angler;
        } else {
            anglers = angler;
        }
        sendChar(from, FO_WELCOME);
    }

    /* give them their new fish */
    angler->fishies++;
    angler->totalweight += GET_OBJ_WEIGHT(obj);

    /* tell them their status */
    sendChar(from, FO_WEIGHT, angler->fishies, ndth(angler->fishies),
            angler->totalweight);

    /* move them up in the rankings, if they deserve it */
    while (angler->prev && angler->prev->totalweight < angler->totalweight) {
        last = angler->prev->prev;
        angler->prev->prev = angler;
        angler->prev->next = angler->next;
        angler->next = angler->prev;
        angler->prev = last;
	if (angler->prev) angler->prev->next = angler;
        if (angler->next->next) angler->next->next->prev = angler->next;
    }

    /* and update the anglers pointer, if this is a new first place */
    if (!angler->prev) {
        anglers = angler;
        sendChar(from, FO_FIRST);
    }

    /* get rid of the evidence */
    extract_obj(obj);
    return 1;
}

