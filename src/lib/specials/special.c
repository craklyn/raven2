/* ************************************************************************
*   File: spec_procs.c                                  Part of CircleMUD *
*  Usage: implementation of special procedures for mobiles/objects/rooms  *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

/* For more examples:
 * ftp://ftp.circlemud.org/pub/CircleMUD/contrib/snippets/specials */

#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "util/utils.h"
#include "actions/interpreter.h"
#include "general/class.h"
#include "general/comm.h"
#include "general/handler.h"
#include "magic/skills.h"
#include "magic/spells.h"
#include "specials/special.h"
#include "util/weather.h"
#include "specials/beholder.h"
#include "specials/castle.h"
#include "specials/guard.h"
#include "specials/healer.h"
#include "specials/mail.h"
#include "specials/metaphys.h"
#include "specials/portal.h"
#include "specials/reward.h"
#include "specials/superAggr.h"
#include "specials/unicorn.h"
#include "general/objsave.h"
#include "specials/boards.h"
#include "specials/shop.h"
#include "specials/perceptory.h"
#include "specials/lockers.h"
#include "specials/qpshop.h"
#include "specials/forger.h"
#include "specials/torment.h"
#include "specials/elie.h"
#include "magic/sing.h"
#include "scripts/dg_scripts.h"
#include "magic/magic.h"
#include "actions/fight.h"


/* ============================================================================ 
This functions prints the name of ANY special procedure that is passed in.
============================================================================ */
void specialName(int (*theFunc)(CharData *ch, void *me, int cmd, char *argument), char *theName)
{
    if (!theFunc)
	sprintf(theName, "None");
    else if (theFunc == clan_bank)
	sprintf(theName, "clan_bank");
    else if (theFunc == cityguard)
	sprintf(theName, "cityguard");
    else if (theFunc == cryogenicist)
	sprintf(theName, "cryogenicist");
    else if (theFunc == janitor)
        sprintf(theName, "janitor");
    else if (theFunc == metaphysician)
        sprintf(theName, "metaphysician");
    else if (theFunc == fido)
        sprintf(theName, "fido");
    else if (theFunc == guild)
        sprintf(theName, "guild");
    else if (theFunc == guild_guard)
        sprintf(theName, "guild_guard");
    else if (theFunc == null_proc)
        sprintf(theName, "null_proc");
    else if (theFunc == postmaster)
        sprintf(theName, "postmaster");
    else if (theFunc == receptionist)
        sprintf(theName, "receptionist");
    else if (theFunc == snake)
        sprintf(theName, "snake");
    else if (theFunc == thief)
        sprintf(theName, "thief");
    else if (theFunc == unicorn)
        sprintf(theName, "unicorn");
    else if (theFunc == superAggr)
        sprintf(theName, "superAggr");
    else if (theFunc == beholder)
        sprintf(theName, "beholder");
    else if (theFunc == bank)
        sprintf(theName, "bank");
    else if (theFunc == gen_board)
        sprintf(theName, "gen_board");
    else if (theFunc == portal_proc)
        sprintf(theName, "portal_proc");
#ifdef DO_DUMP
    else if (theFunc == dump)
        sprintf(theName, "dump");
#endif
    else if (theFunc == roomFall)
        sprintf(theName, "roomFall");
    else if (theFunc == roomAggro)
        sprintf(theName, "roomAggro");
    else if (theFunc == pet_shops)
        sprintf(theName, "pet_shops");
    else if (theFunc == qpshop)
        sprintf(theName, "QPshop");
    else if (theFunc == CastleGuard)
        sprintf(theName, "CastleGuard");
    else if (theFunc == James)
        sprintf(theName, "James");
    else if (theFunc == cleaning)
        sprintf(theName, "cleaning");
    else if (theFunc == DicknDavid)
        sprintf(theName, "DicknDavid");
    else if (theFunc == tim)
        sprintf(theName, "tim");
    else if (theFunc == tom)
        sprintf(theName, "tom");
    else if (theFunc == king_welmar)
        sprintf(theName, "king_welmar");
    else if (theFunc == training_master)
        sprintf(theName, "training_master");
    else if (theFunc == peter)
        sprintf(theName, "peter");
    else if (theFunc == jerry)
        sprintf(theName, "jerry");
    else if (theFunc == reward)
        sprintf(theName, "reward");
    else if (theFunc == healer)
        sprintf(theName, "healer");
    else if (theFunc == shop_keeper)
        sprintf(theName, "shop_keeper");
    else if (theFunc == heavens_door)
        sprintf(theName, "heavens_door");
    else if (theFunc == stjohn)
        sprintf(theName, "stjohn");
    else if (theFunc == ibm)
        sprintf(theName, "ibm");
    else if (theFunc == mobLauncher)
        sprintf(theName, "mobLauncher");
    else if (theFunc == locker_manager)
        sprintf(theName, "locker_manager");
    else if (theFunc == rogue_receptionist)
        sprintf(theName, "rogue_receptionist");
    else if (theFunc == rogue_cryogenicist)
        sprintf(theName, "rogue_cryogenicist");
	else if (theFunc == forgeshop)
		sprintf(theName, "forgeshop");
    else
        sprintf(theName, "&08Unknown -> specialName needs to be updated.&00");
}

/* ============================================================================ 
This function returns 1 if the character passed in should
not be in a fight, by virtue of having a special procedure
essential for game play.
============================================================================ */
int chIsNonCombatant(CharData *ch)
{
    if (!IS_NPC(ch)) return 0; /* killing players is fine! */

    if (ch->nr < 0) return 0; /* not a "real" mob */

    /* if there is no special procedure, go ahead and kill it. */
    if (!mob_index[ch->nr].func) return 0;

    /* Ok, its a mob and it has a special procedure. */
    if (mob_index[ch->nr].func == guild) return 1; /* its a guild master */

    if (mob_index[ch->nr].func == shop_keeper) return 1;

    if (mob_index[ch->nr].func == metaphysician) return 1; 

    if (mob_index[ch->nr].func == receptionist) return 1;

    if (mob_index[ch->nr].func == cryogenicist) return 1;

    if (mob_index[ch->nr].func == rogue_receptionist) return 1;

    if (mob_index[ch->nr].func == rogue_cryogenicist) return 1;

    if (mob_index[ch->nr].func == postmaster) return 1;

    if (mob_index[ch->nr].func == identifyshop) return 1;
    
    if (mob_index[ch->nr].func == remort_questmaster) return 1;    

    /* It's got a special procedure, but fighting it is ok. */
    return 0;
}

/* ********************************************************************
*  Special procedures for rooms                                       *
******************************************************************** */

const char *prac_types[] = {
  "spell",
  "skill"
};

#define LEARNED_LEVEL	0	/* % known which is considered "learned" */
#define MAX_PER_PRAC	1	/* max percent gain in skill per practice */
#define MIN_PER_PRAC	2	/* min percent gain in skill per practice */
#define PRAC_TYPE	3	/* should it say 'spell' or 'skill'?	 */

/* actual prac_params are in class.c */
#define MINGAIN(ch) (prac_params[MIN_PER_PRAC][(int)GET_CLASS(ch)])
#define MAXGAIN(ch) (prac_params[MAX_PER_PRAC][(int)GET_CLASS(ch)])
#define SPLSKL(ch) (prac_types[prac_params[PRAC_TYPE][(int)GET_CLASS(ch)]])

char *exitIsClanGuarded(int cmd, CharData *ch)
{
    int i, j;

    if (cmd < CMD_NORTH || cmd > CMD_DOWN)
        return NOBODY;

    for (i = 0; i < clan_count; i++) {
        for (j = 0; j < 3; j++) {
            if (ch->in_room == real_room(clan_list[i].guards[j].room) &&
                    cmd == clan_list[i].guards[j].dir) {
                if (GET_CLAN(ch) != i)
                    return clan_list[i].guards[j].name;
            }
        }
    }

    return NULL;
}

int
exitIsGuarded( guarded_exit_t *guard_list,
               int guard_type,
               int cmd, 
               CharData *ch )
{
  int guard_index = 0;
  int guard_id    = -1;

  if(( cmd < CMD_NORTH ) || ( cmd > CMD_DOWN )) return guard_id;

  while(( guard_id < 0 ) && ( guard_list->guarded_room != 0 ))
  {
    if(( ch->in_room == real_room( guard_list->guarded_room )) &&  /* If it's a guarded room ...                   */
       ( cmd == guard_list->guarded_exit ))                        /* AND the ch tries to use the guarded exit ... */
    {
      switch( guard_type )
      {
      case GUARD_CLASS:
        if(( GET_CLASS( ch ) != guard_list->allowed_passage ))
          guard_id = guard_index;
        break;

      case GUARD_LEVEL:
        if( guard_list->allowed_passage < 0 )
        {
          if( GET_LEVEL(ch) > abs( guard_list->allowed_passage ))
            guard_id = guard_index;
        }/* if */
        else if(( GET_LEVEL(ch) < guard_list->allowed_passage ))
          guard_id = guard_index;
        break;

      default:
        break;
      }
      /* Ok.. lets not let Hunted people past either - Vex. */
//128      if( !IS_NPC(ch) && IS_SET(PLR_FLAGS(ch), PLR_HUNTED) )
      if (!IS_NPC(ch) && IS_SET_AR(PLR_FLAGS(ch), PLR_HUNTED))
        guard_id = guard_index;
    }

    guard_list++;
    guard_index++;
  }/* while */

  return( guard_id );

}

static int epic_skills[NUM_CLASSES][2] = 
{
    {SPELL_FOUNT_OF_YOUTH,      SKILL_FOCUS          }, /* Magic User      */
    {SKILL_DEVOTION,            SKILL_FERVOR         }, /* Cleric          */
    {SKILL_FALSE_TRAIL,         SKILL_EVASION        }, /* Thief           */
    {SKILL_BATTLECRY,           SKILL_WARCRY         }, /* Warrior         */
    {SPELL_FLETCH,              SKILL_BEFRIEND       }, /* Ranger          */
    {SKILL_ADRENALINE,          SKILL_CRITICAL_HIT   }, /* Assassin        */
    {SKILL_STANCE,              SKILL_POWERSTRIKE    }, /* Shou-Lin        */
    {SPELL_CALL_STEED,          SKILL_CHARGE         }, /* Solamnic Knight */
    {SPELL_CALL_STEED,          SPELL_REFLECTION     }, /* Death Knight    */
    {SKILL_ENHANCED_STEALTH,    SKILL_SHADOW_JUMP    }, /* Shadow Dancer   */
    {SPELL_SUMMON_CORPSE,       SPELL_QUICKEN           }, /* Necromancer     */
};

void list_skills(struct char_data * ch)
{
    int i, cnt;
    int conjFound = 0;
    char buf[MAX_STRING_LENGTH];

    sendChar( ch, "You know of the following:\r\n" );

    for( i = 1, cnt = 0; i < MAX_SKILLS; i++ ){
        if( GET_LEVEL(ch) >= spell_info[i].min_level[(int) GET_CLASS(ch)]) {
            cnt += 1;
	    sendChar(ch, "[%2d]%-20s%20s%s",
                         spell_info[i].min_level[(int)GET_CLASS(ch)],
                         spells[i], 
                         competenceDescription(GET_SKILL(ch, i)),
                         (cnt % 2 ? " | " : "\r\n"));
        }
    }
    if((cnt % 2))
        sendChar(ch, "\r\n");

    if ((GET_CLASS(ch) == CLASS_WARRIOR) && GET_SKILL(ch, SKILL_WEAPON_MASTERY) && (GET_LEVEL(ch) < LVL_IMMORT))
        sendChar(ch, "\r\nWeapon speciality: %s\r\n", mastery_types[GET_MASTERY(ch)]);

    sprintf(buf, "");

    if(GET_LEVEL(ch) >= spell_info[SPELL_MONSTER_SUMMON].min_level[GET_CLASS(ch)] && GET_SKILL(ch, SPELL_MONSTER_SUMMON)) {
        sprintf(buf, "%s  Monster [%2d]", buf, GET_CONJ_CNT(ch, TIMER_MONSTER));
        conjFound += 1;
    }
    if(GET_LEVEL(ch) >= spell_info[SPELL_CONJURE_ELEMENTAL].min_level[GET_CLASS(ch)] && GET_SKILL(ch, SPELL_CONJURE_ELEMENTAL)) {
        sprintf(buf, "%s  Elemental [%2d]", buf, GET_CONJ_CNT(ch, TIMER_ELEMENTAL));
        conjFound += 1;
    }
    if(GET_LEVEL(ch) >= spell_info[SPELL_GATE].min_level[GET_CLASS(ch)] && GET_SKILL(ch, SPELL_GATE)) {
        sprintf(buf, "%s  Demon [%2d]", buf, GET_CONJ_CNT(ch, TIMER_GATE));
        conjFound += 1;
    }
    if(GET_LEVEL(ch) >= spell_info[SPELL_CALL_OF_THE_WILD].min_level[GET_CLASS(ch)] && GET_SKILL(ch, SPELL_CALL_OF_THE_WILD)) {
        sprintf(buf, "%s  Wild [%2d]", buf, GET_CONJ_CNT(ch, TIMER_WILD));
        conjFound += 1;
    }
    if(GET_LEVEL(ch) >= spell_info[SKILL_BLACKJACK].min_level[GET_CLASS(ch)] && GET_SKILL(ch, SKILL_BLACKJACK)) {
        sprintf(buf, "%s  Blackjack [%2d]", buf, COOLDOWN(ch, SLOT_BLACKJACK));
        conjFound += 1;
    }
    if(GET_LEVEL(ch) >= spell_info[SKILL_REDOUBT].min_level[GET_CLASS(ch)] && GET_SKILL(ch, SKILL_REDOUBT)) {
        sprintf(buf, "%s  Redoubt [%2d]", buf, COOLDOWN(ch, SLOT_REDOUBT));
        conjFound += 1;
    }
    if(IS_DEFENDER(ch)) {
        sprintf(buf, "%s  Commanding Shout [%2d]", buf, COOLDOWN(ch, SLOT_COMMANDING_SHOUT));
        conjFound += 1;
    }
    if(GET_SKILL(ch, SPELL_QUICKEN) && IS_NECROMANCER(ch) && PLR_IS_LEGEND(ch)) {
        sprintf(buf, "%s  Quicken [%2d]", buf, COOLDOWN(ch, SLOT_QUICKEN));
        conjFound += 1;
    }
    if(IS_WITCH_DOCTOR(ch)) {
        sprintf(buf, "%s  Metamorphosis [%2d]", buf, COOLDOWN(ch, SLOT_METAMORPHOSIS));
        conjFound += 1;
    }
    if(IS_PRESTIDIGITATOR(ch) && GET_ADVANCE_LEVEL(ch) >= THIRD_ADVANCE_SKILL) {
        sprintf(buf, "%s  Slippery Mind [%2d]", buf, COOLDOWN(ch, SLOT_SLIPPERY_MIND));
        conjFound += 1;
    }
    if(IS_SHADE(ch) && GET_ADVANCE_LEVEL(ch) >= SECOND_ADVANCE_SKILL) {
        sprintf(buf, "%s  Nodeshift [%2d]", buf, COOLDOWN(ch, SLOT_NODESHIFT));
        conjFound += 1;
    }
    if((IS_BRIGAND(ch) || IS_BOUNTY_HUNTER(ch)) && GET_ADVANCE_LEVEL(ch) >= SECOND_ADVANCE_SKILL) {
        sprintf(buf, "%s  Deterrence [%2d]", buf, COOLDOWN(ch, SLOT_DETERRENCE));
        conjFound += 1;
    }
        
    if(conjFound) {
        sendChar(ch, "\r\nCooldown timer%s:%s \r\n", SINGPLUR(conjFound), buf);
    }


    if (IS_MINOTAUR(ch) && (GET_LEVEL(ch) < LVL_IMMORT)) {
	sendChar(ch, "\r\nYour race gives you knowledge of:\r\n");
	sendChar(ch, "%-20s%20s\r\n", spells[SKILL_GORE],
		 competenceDescription(GET_SKILL(ch, SKILL_GORE)));
    }
    else if (IS_DRACONIAN(ch) && (GET_LEVEL(ch) < LVL_IMMORT)) {
	sendChar(ch, "\r\nYour race gives you knowledge of:\r\n");
	sendChar(ch, "%-20s%20s\r\n", spells[SKILL_BREATHE],
		 competenceDescription(GET_SKILL(ch, SKILL_BREATHE)));
    }
    else if (IS_ELEMENTAL(ch) && (GET_LEVEL(ch) < LVL_IMMORT)) {
        int spell = 0;

        switch (GET_SUBRACE(ch)) {
            case FIRE_ELEMENTAL: spell = SPELL_WALL_OF_FIRE; break;
            case WATER_ELEMENTAL: spell = SPELL_TSUNAMI; break;
            case EARTH_ELEMENTAL: spell = SPELL_TREMOR; break;
            case AIR_ELEMENTAL: spell = SPELL_TYPHOON; break;
        }
        if (spell) {
            sendChar(ch, "\r\nYour race gives you knowledge of:\r\n");
            sendChar(ch, "%-20s%20s\r\n", spells[spell],
                   competenceDescription(GET_SKILL(ch, spell)));
        }
    }
    else if (IS_VAMPIRE(ch) && (GET_LEVEL(ch) < LVL_IMMORT))
    {
	sendChar(ch, "\r\nYour race gives you knowledge of:\r\n");
	sendChar(ch, "%-20s%20s\r\n", spells[SKILL_FEED],
		 competenceDescription(GET_SKILL(ch, SKILL_FEED)));
    }
    else if (IS_DEMON(ch) && (GET_LEVEL(ch) < LVL_IMMORT))
    {
	sendChar(ch, "\r\nYour race gives you knowledge of:\r\n");
	sendChar(ch, "%-20s%20s\r\n", spells[SKILL_MIST],
		 competenceDescription(GET_SKILL(ch, SKILL_MIST)));
    }
    else if (IS_IZARTI(ch) && (GET_LEVEL(ch) < LVL_IMMORT))
    {
	sendChar(ch, "\r\nYour race gives you knowledge of:\r\n");
	sendChar(ch, "%-20s%20s\r\n", spells[SKILL_CALM],
		 competenceDescription(GET_SKILL(ch, SKILL_CALM)));
    }
    else if (IS_AMARA(ch) && (GET_LEVEL(ch) < LVL_IMMORT))
    {
	sendChar(ch, "\r\nYour race gives you knowledge of:\r\n");
	sendChar(ch, "%-20s%20s\r\n", spells[SKILL_STING],
		 competenceDescription(GET_SKILL(ch, SKILL_STING)));
    }
    else if (IS_UNDEAD(ch) && (GET_LEVEL(ch) < LVL_IMMORT))
    {
	sendChar(ch, "\r\nYour race gives you knowledge of:\r\n");
	sendChar(ch, "%-20s%20s\r\n", spells[SPELL_TERROR],
		 competenceDescription(GET_SKILL(ch, SPELL_TERROR)));
    }

    if (PLR_IS_VETERAN(ch)) {
        int vetskill = epic_skills[GET_CLASS(ch)][0];
        int legskill = epic_skills[GET_CLASS(ch)][1];

        if (!PLR_IS_LEGEND(ch)) legskill = SKILL_NULL;

        if (vetskill != SKILL_NULL || legskill != SKILL_NULL) {
            sendChar(ch, "\r\nYour status gives you knowledge of:\r\n");
            if (vetskill != SKILL_NULL)
                sendChar(ch, "%-20s%20s\r\n", spells[vetskill],
                        competenceDescription(GET_SKILL(ch, vetskill)));
            if (legskill != SKILL_NULL)
                sendChar(ch, "%-20s%20s\r\n", spells[legskill],
                        competenceDescription(GET_SKILL(ch, legskill)));
        }
    }

    sendChar( ch, "\r\nYou have %d practice session%s remaining.\r\n",
    GET_PRACTICES(ch), GET_PRACTICES(ch) == 1 ? "" : "s");

}

SPECIAL(remort_questmaster)
{
    return( FALSE );

}/* null_proc */

SPECIAL(null_proc)
{
    /* Stubbed */
    return( FALSE );

}/* null_proc */

SPECIAL(guild)
{
#   define INT_LEARNING_RATIO 150.0

    int skill_num, percent;
    int skill_delta;
    int can_learn;

    extern struct spell_info_type spell_info[];

    if (IS_NPC(ch) || (!CMD_IS("practice") && !CMD_IS("unlearn"))) return 0;

    if (GET_POS(ch) != POS_STANDING) {
        send_to_char("You must be standing to learn from a guildmaster.\r\n", ch);
	return 0;
    }

    skip_spaces(&argument);

    if (!*argument) {
        list_skills(ch);
        return 1;
    }

    skill_num = find_skill_num(argument);
    if (skill_num < 1) {
        sendChar(ch, "There is no such %s.\r\n", SPLSKL(ch));
        return 1;
    }

    can_learn = 0;
    if (GET_LEVEL(ch) >= spell_info[skill_num].min_level[GET_CLASS(ch)])
        can_learn = 1;
    if (PLR_IS_VETERAN(ch) && skill_num == epic_skills[GET_CLASS(ch)][0])
        can_learn = 1;
    if (PLR_IS_LEGEND(ch) && skill_num == epic_skills[GET_CLASS(ch)][1])
        can_learn = 1;

    if (!can_learn) {
        sprintf(buf, "You do not know of that %s.\r\n", SPLSKL(ch));
        send_to_char(buf, ch);
        return 1;
    }

    if(CMD_IS("practice")) {
        if(GET_PRACTICES(ch) <= 0) {
            sendChar(ch, "You lack the capacity to learn more %ss right now.\r\n", SPLSKL(ch));
            return 1;
        }

        if( GET_SKILL(ch, skill_num) >= MAX_PRACTICE_LEVEL ) {
            send_to_char("Go out and use thy skills to improve.\r\n", ch);
            return 1;
        }

        send_to_char("You practice for a while...\r\n", ch);
        GET_PRACTICES(ch) -= 1;

        skill_delta = (int)(((float)GET_INT(ch)/(float)INT_LEARNING_RATIO)*100.0);
        percent     = GET_SKILL(ch, skill_num);

        percent += MIN(MAXGAIN(ch), MAX(MINGAIN(ch), skill_delta ));
        
        SET_SKILL(ch, skill_num, MIN(MAX_PRACTICE_LEVEL, percent));

        if (GET_SKILL(ch, skill_num) >= MAX_PRACTICE_LEVEL)
            send_to_char("You are now learned in that area.\r\n", ch);

        return 1;
    }

    if(CMD_IS("unlearn")) {
        int orig = GET_SKILL(ch, skill_num);
        int prac_count = 0;

        if(GET_QP(ch) < 25) {
            sendChar(ch, "You need the blessing of the Gods to forget what you knew.  Return with 25 quest points.\r\n");
            return 0;
        }

        SET_SKILL(ch, skill_num, find_initial_skills(ch, skill_num));

        // Let's figure out how many practices to give the player...
        while(GET_SKILL(ch, skill_num) < orig && GET_SKILL(ch, skill_num) < MAX_PRACTICE_LEVEL) {
            skill_delta = (int)((race_stat_limits[(int)GET_RACE(ch)][INTELLIGENCE_INDEX]/(float)INT_LEARNING_RATIO)*100.0);
            percent     = GET_SKILL(ch, skill_num);

            percent += MIN(MAXGAIN(ch), MAX(MINGAIN(ch), skill_delta ));

            SET_SKILL(ch, skill_num, MIN(MAX_PRACTICE_LEVEL, percent));

            prac_count++;
        }
        
        SET_SKILL(ch, skill_num, find_initial_skills(ch, skill_num));
        
        if(GET_SKILL(ch, skill_num) < MAX_PRACTICE_LEVEL && GET_SKILL(ch, skill_num) < orig) {
            send_to_char("You feel as though you've lost someone once dear to you...\r\n", ch);
            GET_PRACTICES(ch) += prac_count;
            GET_QP(ch) -= 25;
        }
        else {
            send_to_char("You can't unlearn that.\r\n", ch);
        }
    }
}



#ifdef DO_DUMPS
SPECIAL(dump)
{
  struct obj_data *k;
  int value = 0;

  ACMD(do_drop);
  char *fname(char *namelist);

  for (k = world[ch->in_room].contents; k; k = world[ch->in_room].contents) {
    /* Avoid extracting player corpses. Vex. */
    if ( !( (GET_OBJ_TYPE(k) == ITEM_CONTAINER) && !IS_SET(GET_OBJ_WEAR(k), ITEM_WEAR_TAKE) ) ) {
        act("$p vanishes in a puff of smoke!", FALSE, 0, k, 0, TO_ROOM);
        extract_obj(k);
    }
  }

  if (!CMD_IS("drop"))
    return (FALSE);

  do_drop(ch, argument, cmd, SCMD_DROP);

  for (k = world[IN_ROOM(ch)].contents; k; k = world[IN_ROOM(ch)].contents) {
    act("$p vanishes in a puff of smoke!", FALSE, 0, k, 0, TO_ROOM);
    value += MAX(1, MIN(50, GET_OBJ_COST(k) / 10));
    extract_obj(k);
  }

  if (value) {
    sendChar(ch, "You are awarded for outstanding performance.\r\n");
    act("$n has been awarded for being a good citizen.", TRUE, ch, 0, 0, TO_ROOM);

    if (GET_LEVEL(ch) < 3)
      gain_exp(ch, value);
    else
      GET_GOLD(ch) += value;
  }
  return (TRUE);
}
#endif



/* ********************************************************************
*  General special procedures for mobiles                             *
******************************************************************** */


void npc_steal(struct char_data * ch, struct char_data * victim)
{
  int gold;

  if (IS_NPC(victim))
    return;
  if (GET_LEVEL(victim) >= LVL_IMMORT)
    return;

  if (AWAKE(victim) && (number(0, GET_LEVEL(ch)) == 0)) {
    act("You discover that $n has $s hands in your wallet.", FALSE, ch, 0, victim, TO_VICT);
    act("$n tries to steal gold from $N.", TRUE, ch, 0, victim, TO_NOTVICT);
  } else {
    /* Steal some gold coins */
    gold = (int) ((GET_GOLD(victim) * number(1, 10)) / 100);
    if (gold > 0) {
      GET_GOLD(ch) += gold;
      GET_GOLD(victim) -= gold;
    }
  }
}

/* Quite lethal to low-level characters. */
SPECIAL(snake)
{
  if (cmd || GET_POS(ch) != POS_FIGHTING || !FIGHTING(ch))
    return (FALSE);

  if (IN_ROOM(FIGHTING(ch)) != IN_ROOM(ch) || number(0, GET_LEVEL(ch)) != 0)
    return (FALSE);

    act("$n rears back its ugly head and sprays $N with a mist of &09venom&00!", 1, ch, 0, FIGHTING(ch), TO_NOTVICT);
    act("$n sprays you with a mist of &09venom&00!", 1, ch, 0, FIGHTING(ch), TO_VICT);
    call_magic(ch, FIGHTING(ch), 0, SPELL_POISON, GET_LEVEL(ch), NULL, CAST_SPELL);
  return (TRUE);
  }

SPECIAL(thief)
{
  struct char_data *cons;

  if (cmd || GET_POS(ch) != POS_STANDING)
    return (FALSE);

  for (cons = world[IN_ROOM(ch)].people; cons; cons = cons->next_in_room)
    if (!IS_NPC(cons) && GET_LEVEL(cons) < LVL_IMMORT && !number(0, 4)) {
      npc_steal(ch, cons);
      return (TRUE);
    }

  return (FALSE);
}

SPECIAL(fido)
{

  struct obj_data *i, *temp, *next_obj;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  for (i = world[ch->in_room].contents; i; i = i->next_content) {
    if (GET_OBJ_TYPE(i) == ITEM_CONTAINER && GET_OBJ_VAL(i, 3) && CAN_WEAR(i, ITEM_WEAR_TAKE)) {
      act("$n savagely devours a corpse.", FALSE, ch, 0, 0, TO_ROOM);
      for (temp = i->contains; temp; temp = next_obj) {
	next_obj = temp->next_content;
	obj_from_obj(temp);
	obj_to_room(temp, ch->in_room);
      }
      extract_obj(i);
      return (TRUE);
    }
  }
  return (FALSE);
}

SPECIAL(janitor)
{
  struct obj_data *i;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  for (i = world[IN_ROOM(ch)].contents; i; i = i->next_content) {
    if (!CAN_WEAR(i, ITEM_WEAR_TAKE))
      continue;
    if (GET_OBJ_TYPE(i) != ITEM_DRINKCON && GET_OBJ_COST(i) >= 15)
      continue;
    act("$n picks up some trash.", FALSE, ch, 0, 0, TO_ROOM);
    obj_from_room(i);
    obj_to_char(i, ch);
    return (TRUE);
  }
  return (FALSE);
}

SPECIAL(cityguard)
{
  struct char_data *tch, *evil;
  int max_evil;

  if (cmd || !AWAKE(ch) || (GET_POS(ch) == POS_FIGHTING))
    return (FALSE);

  max_evil = 1000;
  evil = 0;

  for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
//    if (!IS_NPC(tch) && CAN_SEE(ch, tch) && IS_SET(PLR_FLAGS(tch), PLR_HUNTED)) {
    if (!IS_NPC(tch) && CAN_SEE(ch, tch) && IS_SET_AR(PLR_FLAGS(tch), PLR_HUNTED)) {
      act("$n screams 'HEY!!!  You're one of those PLAYER KILLERS!!!!!!'", FALSE, ch, 0, 0, TO_ROOM);
      hit(ch, tch, TYPE_UNDEFINED);
      return (TRUE);
    }
  }

  for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
//128    if (!IS_NPC(tch) && CAN_SEE(ch, tch) && IS_SET(PLR_FLAGS(tch), PLR_THIEF)) {
     if (!IS_NPC(tch) && CAN_SEE(ch, tch) && IS_SET_AR(PLR_FLAGS(tch), PLR_THIEF)) {
      act("$n screams 'HEY!!!  You're one of those PLAYER THIEVES!!!!!!'", FALSE, ch, 0, 0, TO_ROOM);
      hit(ch, tch, TYPE_UNDEFINED);
      return (TRUE);
    }
  }

  for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
    if (FIGHTING(tch)) {
      if ((GET_ALIGNMENT(tch) < max_evil) &&
	  (IS_NPC(tch) || IS_NPC(FIGHTING(tch)))) {
	max_evil = GET_ALIGNMENT(tch);
	evil = tch;
      }
    }
  }

    if( !STUNNED(ch) && ( GET_POS(ch) == POS_STANDING ) &&
         evil && !SEEKING(evil) && ( GET_ALIGNMENT(FIGHTING(evil)) >= 0 )) {
        act("$n screams 'PROTECT THE INNOCENT!  BANZAI!  CHARGE!  ARARARAGGGHH!'", FALSE, ch, 0, 0, TO_ROOM);
        hit(ch, evil, TYPE_UNDEFINED);
        return (TRUE);
    }

    return( FALSE );
}

int
count_charms( struct char_data *ch ){
    int count = 0;

    if( ch && ch->followers ){
        struct follow_type *f = ch->followers;

        while( f ){
            if( IS_AFFECTED( f->follower, AFF_CHARM ) && ( f->follower->master == ch )){
                count += 1;
            }
            f = f->next;
        }
    }
    return( count );

}/* count_charms */

SPECIAL(roomAggro)
{
    struct char_data *aggch  = NULL;
    struct char_data *nextCh = NULL;

    for( aggch = world[ch->in_room].people; aggch; aggch = nextCh) {
#if 0
        mudlog( NRM, LVL_IMMORT, TRUE, "AGGRO: %s ", GET_NAME(aggch));
#endif
        nextCh = aggch->next_in_room;
    }
    return 0;
}

SPECIAL(roomFall)
{
#   define STUN_MIN 1
#   define STUN_MAX 3
    struct char_data *victim = ch;

    if( !EXIT(ch, DOWN) || EXIT(ch, DOWN)->to_room == NOWHERE){
        /* Nowhere to fall. */
        return 0;
    }
#if 0
    send_to_char( "\nYou fall downwards and land with a thump! OUCH!!!\n\n", ch );
#endif
    /* randomly damage */
    GET_POS(victim) = POS_SITTING;
    damage( victim, victim, dice( 6, 10 ), TYPE_FALLEN);
    update_pos( victim );

    /* randomly stun */
    STUN_VICTIM_RANGE;

    perform_move( ch, DOWN, 0 );
    return 1;
}

/* Send players who try to cheat the petshops to the sinbin */
#define NAUGHTY_PLAYER_ROOM 1490
/* Quick price calculation for pet prices. */
#define PET_PRICE(pet) (GET_EXP(pet) * 3)

SPECIAL(pet_shops)
{
    char buf[MAX_STRING_LENGTH], pet_name[256];
    room_rnum pet_room;
    struct char_data *pet;

    /* Gross. */
    pet_room = IN_ROOM(ch) + 1;

    if (CMD_IS("list"))
    {
        sendChar(ch, "Available pets are:\r\n");
        for (pet = world[pet_room].people; pet; pet = pet->next_in_room)
        {
            /* No, you can't have the Implementor as a pet if he's in there. */
            if (!IS_NPC(pet))
                continue;
            sendChar(ch, "%8d - %s\r\n", PET_PRICE(pet), GET_NAME(pet));
        }
        return (TRUE);
    }
    else if (CMD_IS("buy"))
    {

        two_arguments(argument, buf, pet_name);


        if (!(pet = get_char_room(buf, pet_room)))
        {
            sendChar(ch, "There is no such pet!\r\n");
            return (TRUE);
        }

        /* check that the "pet" really exists, and that its exp hasn't
         * wrapped to some stupidly negative number, and it's a MOB not a
         * PC */
        if (GET_MOB_RNUM(pet) < 1 || PET_PRICE(pet) < 0 || !IS_NPC(pet))
        {
            sendChar(ch, "&14*&13*&10&20FOOF&00&13*&14*&00\r\n");
            sendChar(ch, "You feel screwed.\r\n");
            char_from_room(ch);
            char_to_room(ch, real_room(NAUGHTY_PLAYER_ROOM));
            if (GET_LEVEL(ch) < LVL_IMMORT) WAIT_STATE(ch, PULSE_VIOLENCE * 20);
            return (TRUE);
        }

        if (GET_GOLD(ch) < PET_PRICE(pet))
        {
            sendChar(ch, "You don't have enough gold!\r\n");
            return (TRUE);
        }
        if (count_charms(ch) > 2)
        {
            sendChar(ch, "The shopkeeper says, 'You have enough helpers for now.'");
            return (TRUE);
        }
        GET_GOLD(ch) -= PET_PRICE(pet);

        if (PLR_FLAGGED(ch, PLR_SHUNNED))
        {
            sendChar(ch, "The shopkeepr looks appalled as the pet you picked runs back into its cage.\r\n");
            sendChar(ch, "The shopkeeper says, 'I'm truly sorry, I've never seen one do THAT before....but no refunds!'\r\n");
            return (TRUE);
        }

        pet = read_mobile(GET_MOB_RNUM(pet), REAL);
        GET_EXP(pet) = 0;
        SET_BIT_AR(AFF_FLAGS(pet), AFF_CHARM);

        if (*pet_name)
        {
            snprintf(buf, sizeof (buf), "%s %s", pet->player.name, pet_name);
            /* free(pet->player.name); don't free the prototype! */
            pet->player.name = strdup(buf);

            snprintf(buf, sizeof (buf), "%sA small sign on a chain around the neck says 'My name is %s'\r\n",
                     pet->player.description, pet_name);
            /* free(pet->player.description); don't free the prototype! */
            pet->player.description = strdup(buf);
        }
        char_to_room(pet, IN_ROOM(ch));
        add_follower(pet, ch);
        load_mtrigger(pet);

        /* Be certain that pets can't get/carry/use/wield/wear items */
        IS_CARRYING_W(pet) = 1000;
        IS_CARRYING_N(pet) = 100;

        sendChar(ch, "May you enjoy your pet.\r\n");
        act("$n buys $N as a pet.", FALSE, ch, 0, pet, TO_ROOM);

        return (TRUE);
    }

    /* All commands except list and buy */
    return (FALSE);
}



/* ********************************************************************
*  Special procedures for objects                                     *
******************************************************************** */



SPECIAL(bank)
{
  int amount;

  if (CMD_IS("balance")) {
    if (GET_BANK_GOLD(ch) > 0)
      sprintf(buf, "Your current balance is %d coins.\r\n",
	      GET_BANK_GOLD(ch));
    else
      sprintf(buf, "You currently have no money deposited.\r\n");
    send_to_char(buf, ch);
    return 1;
  } else if (CMD_IS("deposit")) {
    if ((amount = atoi(argument)) <= 0) {
      send_to_char("How much do you want to deposit?\r\n", ch);
      return 1;
    }
    if (GET_GOLD(ch) < amount) {
      send_to_char("You don't have that many coins!\r\n", ch);
      return 1;
    }
    GET_GOLD(ch) -= amount;
    GET_BANK_GOLD(ch) += amount;
    sprintf(buf, "You deposit %d coins.\r\n", amount);
    send_to_char(buf, ch);
    act("$n makes a bank transaction.", TRUE, ch, 0, FALSE, TO_ROOM);
    return 1;
  } else if (CMD_IS("withdraw")) {
    if ((amount = atoi(argument)) <= 0) {
      send_to_char("How much do you want to withdraw?\r\n", ch);
      return 1;
    }
    if (GET_BANK_GOLD(ch) < amount) {
      send_to_char("You don't have that many coins deposited!\r\n", ch);
      return 1;
    }
    GET_GOLD(ch) += amount;
    GET_BANK_GOLD(ch) -= amount;
    sprintf(buf, "You withdraw %d coins.\r\n", amount);
    send_to_char(buf, ch);
    act("$n makes a bank transaction.", TRUE, ch, 0, FALSE, TO_ROOM);
    return 1;
  } else
    return 0;
}

