/* ************************************************************************
*  File: chores.c                                       Part of  RavenMUD *
*  Author: Imhotep of RavenMUD						  *
*  Usage: Random tasks for characters to achieve                          *
*                                                                         *
*  RavenMUD is derived from CircleMUD, so the CircleMUD license applies.  *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/handler.h"
#include "actions/interpreter.h"
#include "util/utils.h"
#include "general/class.h"        /* for IS_AFFECTED see note in class.c */

#define CHORE_VISIT     0
#define CHORE_KILL      1
#define CHORE_QUEST     2

typedef struct player_chore {
    unsigned char type;
    int vnum;
} PlayerChore;

static PlayerChore chores[] = {
	
	{CHORE_KILL,  18001},
	{CHORE_KILL,  18002},
	{CHORE_KILL,  18003},
	{CHORE_KILL,  18004},
	{CHORE_KILL,  18005},
	{CHORE_KILL,  18006},
	{CHORE_KILL,  18007},
	{CHORE_KILL,  18008},
	{CHORE_KILL,  18009},
	{CHORE_KILL,  18010},
	{CHORE_KILL,  18011},
	{CHORE_KILL,  18012},
	{CHORE_KILL,  18013},
	{CHORE_KILL,  18014},
	{CHORE_KILL,  18015},
	{CHORE_KILL,  18016},
	{CHORE_KILL,  18017},
	{CHORE_KILL,  18018},
	{CHORE_KILL,  18019},
	{CHORE_KILL,  18020},
};

#define CHORE_COUNT (sizeof(chores)/sizeof(PlayerChore))

#define CHORE(ch, i) (ch->player_specials->saved.chores[i])

/* assign a random set of chores to a player */
void chore_initialise(CharData *ch)
{
    int i, j, c;

    for (i = 0; i < MAX_CHORES; i++) {
        do {
            c = number(1, CHORE_COUNT) - 1;
            for (j = 0; j < i; j++) if (CHORE(ch, j) == c) c = -1;
        } while (c == -1);
        CHORE(ch, i) = c;
    }
    ch->player_specials->saved.chore_count = 0;
}

static void check_one_kill(CharData *ch, int vnum)
{
    int i;

    /* mobs need not apply */
    if (IS_NPC(ch)) return;

    if(!PRF_FLAGGED(ch, PRF_BRIEF))
        sendChar(ch, "&05This kill counts toward status.&00\r\n");

    for (i = 0; i < MAX_CHORES; i++) {
        if (CHORE(ch, i) >= 0 && chores[CHORE(ch, i)].type == CHORE_KILL &&
                chores[CHORE(ch, i)].vnum == vnum) {
            CHORE(ch, i) = -CHORE(ch, i);
        }
    }
}

void chore_check_kill(CharData *ch, int vnum)
{
    FollowType *fol;

    // If ch is null, bail
    if (!ch)
        return;

    /* make sure the char gets it */
    check_one_kill(ch, vnum);

    /* if not grouped, go away */
    // This code is disabled because mobiles can be unofficially
    // grouped but contribute...
    //if (!IS_AFFECTED(ch, AFF_GROUP)) return;

    /* For every grouped follower in the same room, check it also */
    for (fol = ch->followers; fol; fol = fol->next) {
        if (IS_AFFECTED(fol->follower, AFF_GROUP) &&
                fol->follower->in_room == ch->in_room)
            check_one_kill(fol->follower, vnum);
    }

        /* If the char has a master in the same room, switch to them */
    if(ch->master && ch->master->in_room == ch->in_room) {
        ch = ch->master;
        chore_check_kill(ch, vnum);
    }
    else if(ch->master)
        ch = ch->master;

}

void chore_check_quest(CharData *ch, int vnum)
{
    int i;

    /* A completed chore is a negative one */
    for (i = 0; i < MAX_CHORES; i++) {
        // Somewhere along the line, we started letting chore 0 be allowed.  This
        // makes it impossible to mark the chore as completed (-0).  To work
        // around this, we're just going to change it to chore = 1.
        if(CHORE(ch, i) == 0) {
            mudlog(NRM, LVL_IMPL, TRUE, "Fixing player %s's chores to be completable!", GET_NAME(ch));
            CHORE(ch, i) = 8;
        }

        if (CHORE(ch, i) >= 0 && chores[CHORE(ch, i)].type == CHORE_QUEST &&
                chores[CHORE(ch, i)].vnum == vnum) {
            CHORE(ch, i) = -CHORE(ch, i);
        }
    }
}

ACMD(do_showchores)
{
    CharData *victim;
    int i, c;
    one_argument(argument, buf);

    if (!*buf) {
        sendChar(ch, "Show whose goals?\r\n");
        return;
    }

    if ((victim = get_char_vis(ch, buf, 1)) == NULL) {
        sendChar(ch, "Who is that?\r\n");
        return;
    }

    if (IS_NPC(victim)) {
        sendChar(ch, "Goal #1: Become a real person.\r\n");
        return;
    }

    sendChar(ch, "Goals for %s:\r\n", GET_NAME(victim));
    for (i = 0; i < MAX_CHORES; i++) {
        c = CHORE(victim, i);
        sendChar(ch, "#%2d %c ", i, c < 0 ? '*' : ' ');
        if (c < 0) c = -c;
        if (c >= CHORE_COUNT) {
            sendChar(ch, "Invalid chore %d!\r\n", c);
        } else if (chores[c].type == CHORE_VISIT) {
            sendChar(ch, "Visit room #%d\r\n", chores[c].vnum);
        } else if (chores[c].type == CHORE_KILL) {
            sendChar(ch, "Kill mob #%d, ", chores[c].vnum);
            c = real_mobile(chores[c].vnum);
            if (c == -1) sendChar(ch, "&08unknown!&00\r\n");
            else sendChar(ch, "%s\r\n", mob_proto[c].player.short_descr);
        } else if (chores[c].type == CHORE_QUEST) {
            sendChar(ch, "Complete quest #%d", chores[c].vnum);
            c = real_quest(chores[c].vnum);
            if (c == -1) sendChar(ch, ", &08unknown!&00\r\n");
            else sendChar(ch, "\r\n");
        }
    }
}

#undef CHORE
#define CHORE(ch, i) (ch->player_specials_saved.chores[i])
void chore_update(CharFileU *ch)
{
    int i, c = 0;

    for (i = 0; i < MAX_CHORES; i++) {
        if (CHORE(ch, i) < 0) c++;
    }

    ch->player_specials_saved.chore_count = c;
}

/* This function type, the kind that is no longer called, with no comments
 * why it's not longer called nor removed from the source, are scattered all over
 * this file. I've grouped them and commented them out for reference purposes.
 *
// assign a random set of chores to a player
void chore_reinitialise(CharData *ch)
{
    int i, j, c;

    for (i = 0; i < MAX_CHORES; i++) {

        if( CHORE(ch, i) < 0)
            continue;

        do {
            c = number(1, CHORE_COUNT) - 1;
            for (j = 0; j < i; j++) if (CHORE(ch, j) == c) c = -1;
        } while (c == -1);
        CHORE(ch, i) = c;
    }

}
End of comment chore_reinitialize */

/* Another Chore un-re-un-Init function with no notes and no calls in the code.
void chore_pfile_reinit(CharFileU *ch)
{
    int i, j, c;

    for (i = 0; i < MAX_CHORES; i++) {
        for (j = i; j < MAX_CHORES; j++) if (CHORE(ch, j) == CHORE(ch, i)) {
        }
    }
    ch->player_specials_saved.chore_count = 0;
}
End of comment chore_pfile_reinit */

/*

//  Reset chores is a function to re-scramble everyone's chores.  
//  It was last used when Eff got a copy of the code.  It can be
//  called in the boot_db function after boot db is finished.
void reset_chores() {

int i;
int player_i;
CharData *cbuf;
struct char_file_u tmp_store;


  for (i = 0; i <= top_of_p_table; i++) {
    CREATE(cbuf, CharData, 1);
    clear_char(cbuf);
    if ((player_i = load_char( (player_table + i)->name , &tmp_store)) > -1)
      store_to_char(&tmp_store, cbuf);

    chore_reinitialise(cbuf);

    char_to_store(cbuf, &tmp_store);
    fseek(player_fl, (player_i) * sizeof(struct char_file_u), SEEK_SET);
    fwrite(&tmp_store, sizeof(struct char_file_u), 1, player_fl);
    free_char(cbuf);
  }
}

End of comment reset_chores */
