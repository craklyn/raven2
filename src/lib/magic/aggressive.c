/* ============================================================================ 
aggressive.c
Copyright 1997 Jeremy Wright(a.k.a. Vex)
============================================================================ */

#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/class.h"
#include "util/utils.h"
#include "magic/aggressive.h"
#include "actions/interpreter.h"
#include "magic/spells.h"
#include "magic/skills.h"
#include "actions/act.h"          /* ACMDs located within the act*.c files */
#include "util/weather.h"
#include "magic/sing.h"
#include "general/handler.h"

/* ============================================================================
This function simply sets the characters aggressive preferences.
============================================================================ */
#define THIS_SKILL SKILL_AGGRESSIVE
ACMD(do_aggressive)
{
    char theArg[MAX_INPUT_LENGTH];
    int i;
    char *aggr_pref[NUM_AGGR_PREF + 1] = 
    {
	"OFF",		/* 0 - AGGR_OFF */
	"MONSTERS",	/* 1 - AGGR_MONSTERS */
	"PLAYERS",	/* 2 - AGGR_PLAYERS */
	"BOTH",		/* 3 - AGGR_BOTH */
	"\n"		/* end */
    };

    IF_UNLEARNED_SKILL   ( "You have no ability in this area." );

    /* ensure preference is valid. */
    if ( ch->player_specials->saved.aggr_pref < 0
         || ch->player_specials->saved.aggr_pref >= NUM_AGGR_PREF)
	ch->player_specials->saved.aggr_pref = AGGR_OFF;

    if (!*argument) {
	sendChar(ch, "Possible options are:\r\n");
	sendChar(ch, "OFF - you will not be aggressive.\r\n");
	sendChar(ch, "MONSTERS - you will attack aggressive monsters.\r\n");
	sendChar(ch, "PLAYERS - you will attack players you are not grouped with.\r\n");
	sendChar(ch, "BOTH - You will attack aggressive monsters and players.\r\n");
	sendChar(ch, "Your current setting is: &08%s&00.\r\n", aggr_pref[ch->player_specials->saved.aggr_pref]);
	return;
    }

    one_argument(argument, theArg);

    for (i = 0; i < NUM_AGGR_PREF; i++)
	if ( !strncasecmp(theArg, aggr_pref[i], strlen(theArg)) )
	    break; /* found a valid preference */

    /* The preference they entered wasn't valid. */
    if ( i < 0 || i >= NUM_AGGR_PREF) {
	sendChar(ch, "%s isn't a valid aggressive preference.\r\n", theArg);
	sendChar(ch, "Type &10aggressive&00 with no arguments for help.\r\n");
	return;
    }

    sendChar(ch, "Your aggressive preference is now: %s.\r\n", aggr_pref[i]);
    ch->player_specials->saved.aggr_pref = i;
}

/* ============================================================================ 
This function determines if a player, by virtue of having the aggressive skill
and the "desire", will automatically attack the victim. It returns 1 if they
will, 0 if they will not.
============================================================================ */
int pcAttackVictim(CharData *attacker, CharData *victim)
{
    int aggro = 0;

    if (IS_NPC(attacker)) {
	mudlog(NRM, LVL_IMMORT, TRUE, "pcAttackVictim invoked for NPC attacker %s!", GET_NAME(attacker));
	return 0;
    }

	if( STUNNED(attacker) )
	return 0;

    if (GET_AGGR_PREF(attacker) == AGGR_OFF)
	return 0;

    if (GET_POS(attacker) != POS_STANDING)
	return 0;

    if (attacker == victim)
	return 0;

    if (!skillSuccess(attacker, SKILL_AGGRESSIVE))
	return 0;

    if (!CAN_SEE(attacker, victim))
	return 0;
    
    if (FIGHTING(attacker))
	return 0;

    if (IS_AFFECTED(attacker, AFF_PARALYZE))
	return 0;

    // Recognize team mates for flag games
    if (PRF_FLAGGED(attacker, PRF_GOLD_TEAM) && PRF_FLAGGED(victim, PRF_GOLD_TEAM)) 
    return 0;
    if (PRF_FLAGGED(attacker, PRF_BLACK_TEAM) && PRF_FLAGGED(victim, PRF_BLACK_TEAM)) 
    return 0;
    if (PRF_FLAGGED(attacker, PRF_ROGUE_TEAM) && PRF_FLAGGED(victim, PRF_ROGUE_TEAM)) 
    return 0;

    /* Perhaps they are after monsters */
    if ( MOB_FLAGGED(victim, MOB_AGGRESSIVE) &&
         (GET_AGGR_PREF(attacker) == AGGR_MONSTERS ||
          GET_AGGR_PREF(attacker) == AGGR_BOTH) )
	aggro = 1;

    /* How about players? */
    if ( !IS_NPC(victim) &&
	 !chGroupedWithTarget(attacker, victim) &&
	 (GET_LEVEL(victim) < LVL_IMMORT) &&
         (GET_AGGR_PREF(attacker) == AGGR_PLAYERS ||
          GET_AGGR_PREF(attacker) == AGGR_BOTH) )
	aggro = 1;

    if (aggro && IS_SET_AR(ROOM_FLAGS(attacker->in_room), ROOM_PEACEFUL)) {
        sendChar( attacker, "Your stupidity has been squelched.\r\n" );
        return 0;
    }

    /* advance the skill, but slow like */
    if (aggro && percentSuccess(40)) {
        advanceSkill(attacker, SKILL_AGGRESSIVE,
                (GET_LEVEL(attacker) - 35) * 2 + 60,
                "Your spirit is at one with the tiger!",
                FALSE, FALSE, FALSE);
    }

    return aggro; /* Not who they are after. */
} /* pcAttackVictim */
