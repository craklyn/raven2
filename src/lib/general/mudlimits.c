/* ************************************************************************
*   File: limits.c                                      Part of CircleMUD *
*  Usage: limits & gain funcs for HMV, exp, hunger/thirst, idle time      *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "util/utils.h"
#include "general/objsave.h"
#include "magic/spells.h"
#include "general/comm.h"
#include "general/handler.h"
#include "util/weather.h"
#include "specials/portal.h"
#include "scripts/dg_scripts.h"
#include "general/class.h"
#include "magic/magic.h"
#include "actions/fight.h"
#include "actions/outlaw.h"       /* For unset_hunted_player */


/* prototypes of local functions */
/* Infectious global world plague */
static void spreadInfection(CharData *theInfected);
static void infectious(CharData *ch);


#define READ_TITLE(ch) (GET_SEX(ch) == SEX_MALE ?   \
	titles[(int)GET_CLASS(ch)][(int)GET_LEVEL(ch)].title_m :  \
	titles[(int)GET_CLASS(ch)][(int)GET_LEVEL(ch)].title_f)

/*
 * Begin all disease/plague code.
 */
//FIXME: Calculate for new tick updates.
#define PLAGUE_MAX 48

typedef struct {
    int   infectionLevel;  /* And integer max value for this stage of plague. */
    int   infectionDamage; /* How much damage can be suffered at this stage.  */
    int   howContagious;   /* The percent change that it will spread.         */
    char *sufferCharAct;   /* The action message to the char.                 */
    char *sufferRoomAct;   /* The action message to the room.                 */
} plague_entry;

plague_entry standardBubonic[] = {
 /*  lvl dmg   %  message_char                         message_room           */
    {  4, 0,  80, NULL,                                NULL                  },
    {  8, 0,  25, "You feel feverish.",                "$n looks flush."     },
    { 16, 6,  50, "You cough.",                        "$n coughs."          },
    { 24, 9,  70, "You sneeze.",                       "$n sneezes."         },
    { 36, 12, 85, "You vomit.",                        "$n vomits."          },
    { 44, 0,   0, "You feel your strength returning.", "$n looks healthier." },
    { 48, 0,   0, "It's all over.",                    "$n looks whole."     }
};

static void spreadInfection(CharData *theInfected)
{
    CharData *theVictim = world[theInfected->in_room].people;
    struct affected_type af;

    if( !CONFIG_PLAGUE_IS_CONTAGIOUS ) return;

    if( theVictim ){
        if( !IS_AFFECTED(theVictim, AFF_PLAGUE) ){
            af.type      = SPELL_PLAGUE;
            af.duration  = 48 TICKS;
            af.modifier  = -2;
            af.location  = APPLY_STR;
            af.bitvector = AFF_PLAGUE;
            affect_join( theVictim, &af, 0, 0, 0, 0 );
            mudlog( BRF, LVL_DEITY, TRUE, "(INFECTED) %s with the plague.", GET_NAME(theVictim));
        }
    }
}

static void infectious(CharData *ch)
{
    int theRoll     = number( 1, 100 );
    int i           = 0;
    int stage       = 0;
    int howAdvanced = -1;
    struct affected_type *aff;

    for( aff = ch->affected; aff && howAdvanced < 0; aff = aff->next ){
        if( aff->type == SPELL_PLAGUE ){
            //FIXME: Giving howAdvanced improper calculation.
            howAdvanced = PLAGUE_MAX - aff->duration;
            if( howAdvanced < 0 ) return;
        }
    }

    //FIXME: Refer to standardBubonic structure above for a viable calculation #
    while( howAdvanced > standardBubonic[i].infectionLevel ){
        stage = i;
        i++;
    }

    if( standardBubonic[stage].sufferCharAct != NULL ){
        sendChar(ch, standardBubonic[stage].sufferCharAct);
        act( standardBubonic[stage].sufferRoomAct, TRUE, ch, 0, 0, TO_ROOM );
    }

    if( theRoll < standardBubonic[stage].howContagious )
        spreadInfection( ch );

    if( theRoll > 90 ){
        sendChar(ch, "You faint.");
        GET_POS(ch) = POS_STUNNED;
        WAIT_STATE( ch, 5*PULSE_VIOLENCE );
    }
}

/* When age < 15 return the value p0 */
/* When age in 15..29 calculate the line between p1 & p2 */
/* When age in 30..44 calculate the line between p2 & p3 */
/* When age in 45..59 calculate the line between p3 & p4 */
/* When age in 60..79 calculate the line between p4 & p5 */
/* When age >= 80 return the value p6 */
int graf(int age, int p0, int p1, int p2, int p3, int p4, int p5, int p6)
{

  if (age < 15)
    return (p0);		/* < 15   */
  else if (age <= 29)
    return (int) (p1 + (((age - 15) * (p2 - p1)) / 15));	/* 15..29 */
  else if (age <= 44)
    return (int) (p2 + (((age - 30) * (p3 - p2)) / 15));	/* 30..44 */
  else if (age <= 59)
    return (int) (p3 + (((age - 45) * (p4 - p3)) / 15));	/* 45..59 */
  else if (age <= 79)
    return (int) (p4 + (((age - 60) * (p5 - p4)) / 20));	/* 60..79 */
  else
    return (p6);		/* >= 80 */
}



/*
 * Amount of H/M/V to be taken away in SUFFER rooms.  Must be a negative value.
 * Kaidon 5/23/97
*/
#define SUFFER_RATE -20

int calcManaBase(CharData *ch) {
    int base = 0;

    /* Set base regeneration */
    if (IS_NPC(ch)) {
        base += GET_LEVEL(ch) * 2;
    }
    else {
        base += graf(age(ch).year, 34, 32, 24, 26, 30, 41, 42);

        /* Class bonuses */
        switch(GET_CLASS(ch)) {
            case CLASS_MAGIC_USER:
            case CLASS_NECROMANCER:
                base += GET_LEVEL(ch);
                break;
            case CLASS_CLERIC:
            case CLASS_SHADOW_DANCER:
                base += GET_LEVEL(ch)*2/3;
                break;
            case CLASS_SOLAMNIC_KNIGHT:
            case CLASS_DEATH_KNIGHT:
                base += GET_LEVEL(ch)/2;
                break;
            case CLASS_SHOU_LIN:
            case CLASS_RANGER:
                base += GET_LEVEL(ch)/3;
                break;
            case CLASS_WARRIOR:
            case CLASS_THIEF:
            case CLASS_ASSASSIN:
                break;
            default:
                mudlog(NRM, LVL_IMMORT, TRUE, "ERROR calcManaBase: %s's class not recognized!", GET_NAME(ch));
                break;
        }
        
        /* Race bonuses*/
        if(IS_DROW(ch))
            base += (GET_LEVEL(ch)/8);

        /* Position calculations    */
        switch (GET_POS(ch)) {
            case POS_MEDITATING:
                if(IS_CHI_WARRIOR(ch) && GET_ADVANCE_LEVEL(ch) >= SECOND_ADVANCE_SKILL)
                    base += GET_LEVEL(ch)/10;
            case POS_SLEEPING:
                base += 16;
                break;
            case POS_RESTING:
                base += 8;	/* Divide by 2 */
                break;
        }
    }

    /* Int/wis bonus */
    base += (GET_WIS(ch) + GET_INT(ch) - 30);
    
    if(!IS_NPC(ch) && IS_VAMPIRE(ch) && IS_SUNLIGHT(IN_ROOM(ch)) && !IS_AFFECTED(ch, AFF_SHADOW_SPHERE))
        base = 0;

    return base;
}

int calcManaMulti(CharData *ch) {
    int multi = 100;

    if(ROOM_FLAGGED(ch->in_room, ROOM_MANA_REGEN))
        multi += 50;
    
    if(IS_AFFECTED(ch, AFF_WRAITHFORM))
        multi += 25;

    if((GET_CLASS(ch) == CLASS_SHADOW_DANCER) && (IS_DARK(ch->in_room) || IS_AFFECTED(ch, AFF_SHADOW_SPHERE)))
        multi += 50;

    if(IS_AFFECTED(ch, AFF_POISON))
        multi -= 100;

    if(!IS_NPC(ch)) {
        if( !IS_NPC(ch) && ROOM_FLAGGED(ch->in_room, ROOM_SUFFER) && ( GET_LEVEL(ch) < LVL_IMMORT ))
            multi -= 90;
        else if (!GET_COND(ch, THIRST) && IS_AMARA(ch))
            multi -= 90;
        else if (IS_IZARTI(ch) && IS_EVIL(ch))
            multi -= 90;
        else if (IS_DEMON(ch) && IS_GOOD(ch))
            multi -= 90;
    }

    if(GET_RACE(ch) == RACE_STROLL)
        multi += 100;

    /* Position calculations    */
    switch (GET_POS(ch)) {
        case POS_MEDITATING:
        case POS_SLEEPING:
            multi += 120;
            break;
        case POS_RESTING:
            multi += 60;
            break;
    }

    multi += (int)GET_SKILL(ch, SKILL_DEVOTION);

    // Don't let a player's regeneration completely stagnate...
    multi = MAX(multi, 25);
    return multi;
}

int calcManaBonus(CharData *ch) {
    int bonus = 0;
    
    return bonus;
}


/* manapoint gain per game hour */
int mana_gain(CharData * ch)
{
    int malnourishCount = 0;
    struct affected_type *hjp, *next;

    if (IN_ROOM(ch) == -1) return 0;

    if(!IS_NPC(ch) && ((GET_COND(ch, HUNGER) == 0 && !IS_VAMPIRE(ch)) || GET_COND(ch, THIRST) == 0)) {
        if(ch->desc && percentSuccess(2)) {
            if(!affected_by_spell(ch, SKILL_EMACIATED))
                add_affect( ch, ch, SKILL_EMACIATED, GET_LEVEL(ch), 0, 0, -1,
                        0, FALSE, FALSE, FALSE, FALSE);

            for (hjp = ch->affected; hjp; hjp = next) {
                next = hjp->next;
                if (hjp->type == SKILL_EMACIATED_MANA)
                    malnourishCount++;
            }

            if(malnourishCount < 18)
                add_affect( ch, ch, SKILL_EMACIATED_MANA, GET_LEVEL(ch), APPLY_MANA, -MIN(50, GET_MANA(ch)/20), -1,
                        0, FALSE, FALSE, FALSE, FALSE);
        }
    }

    int base = calcManaBase(ch);
    int multiplier = calcManaMulti(ch);
    int bonus = calcManaBonus(ch);
    
    int gain = base*multiplier/100 + bonus;
    if (affected_by_spell(ch, SKILL_POTENCY)) gain *= 5;
    return (gain);
}


/*
** doPoison
*/
void
doPoison(CharData* vict)
{
    int poisonDamage = aff_level(vict, AFF_POISON) + number(1,10);

    if (affected_by_spell(vict, SPELL_RESIST_POISON)) {
        switch (number(1,10)) {
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
                poisonDamage = 1;
                break;
            case 6:
                if (affected_by_spell(vict, SPELL_POISON)) {
                    sendChar(vict, "You fight off the poison and recover.\r\n");
                    affect_from_char(vict, SPELL_POISON);
                    return;
                }
                break;
            case 7:
            case 8:
            case 9:
            case 10:
                break;
        }
    }
    damage(vict, vict, poisonDamage/3, SPELL_POISON);
}

/*
** doInvigorate
*/
void
doInvigorate(CharData* ch)
{
   int gain = 0;

   gain += number(1, GET_LEVEL(ch)/3);
   gain += number(1, (GET_MAX_HIT(ch) - GET_LEVEL(ch)*12)/15);

   if (GET_HIT(ch) < GET_MAX_HIT(ch)) 
       GET_HIT(ch) = MIN( GET_MAX_HIT(ch), GET_HIT(ch) + gain );
}

/*
** doDisease
*/
void
doDisease(CharData* vict)
{
    int poisonDamage = aff_level(vict, AFF_DISEASE) + number(1,10);
    damage(vict, vict, poisonDamage, SPELL_DISEASE);
}


int calcHitBase(CharData *ch) {
    int base = 0;

    if(IS_NPC(ch))
        base += 18;
    else {
        base += graf(age(ch).year, 37, 38, 43, 46, 40, 34, 32);
    }

    /* Level/Class bonuses */
    switch(GET_CLASS(ch)) {
        case CLASS_WARRIOR:
            base += GET_LEVEL(ch)*5/3;
            break;
        case CLASS_RANGER:
        case CLASS_SOLAMNIC_KNIGHT:
        case CLASS_DEATH_KNIGHT:
            base += GET_LEVEL(ch)*4/3;
            break;
        case CLASS_THIEF:
        case CLASS_ASSASSIN:
            base += GET_LEVEL(ch);
            break;
        case CLASS_SHOU_LIN:
        case CLASS_CLERIC:
        case CLASS_SHADOW_DANCER:
            base += GET_LEVEL(ch)*2/3;
            break;
        case CLASS_MAGIC_USER:
        case CLASS_NECROMANCER:
            base += GET_LEVEL(ch)/3;
            break;
        default:
            mudlog(NRM, LVL_IMMORT, TRUE, "ERROR calcManaBase: %s's class not recognized!", GET_NAME(ch));
            break;
    }

    if(affected_by_spell(ch, SKILL_ADRENALINE)) {
        struct affected_type *af;
        for (af = ch->affected; af; af = af->next) {
            if(af->type == SKILL_ADRENALINE)
                base += af->modifier;
        }
    }

    /* Position calculations    */
    switch (GET_POS(ch)) {
        case POS_MEDITATING:
            if(IS_CHI_WARRIOR(ch) && GET_ADVANCE_LEVEL(ch) >= SECOND_ADVANCE_SKILL)
                base += GET_LEVEL(ch)/10;
        case POS_SLEEPING:
            base += 20;    /* Divide by 2 */
            break;
        case POS_RESTING:
            base += 12;	/* Divide by 4 */
            break;
    }

    // Naturalist hunters get 1% bonus regen to pets per advance level.
    if (affected_by_spell(ch, SPELL_CALL_OF_THE_WILD) && ch->master && IS_NATURALIST(ch->master))
        base += GET_MAX_HIT(ch)*GET_ADVANCE_LEVEL(ch->master)/100;
    
    return base;
}

int calcHitMulti(CharData *ch) {
    int multi = 100;

    if (GET_RACE(ch) == RACE_TROLL)
        multi += 200;	/* Trolls regenerate VERY fast. */
    if (GET_RACE(ch) == RACE_STROLL)
        multi += 300;

    if(IS_AFFECTED(ch, AFF_PLAGUE))
        multi -= 50;

    if (IS_AFFECTED(ch, AFF_POISON) || IS_AFFECTED(ch, AFF_DISEASE))
        multi -= 75;

    if( ROOM_FLAGGED( ch->in_room, ROOM_HEALTH_REGEN ))
        multi += 50;

    /* Position calculations    */
    switch (GET_POS(ch)) {
        case POS_MEDITATING:
        case POS_SLEEPING:
            multi += 120;
            break;
        case POS_RESTING:
            multi += 60;
            break;
    }

    // During the day, vampires have a difficult time regenerating mana
    if(!IS_NPC(ch) && ((GET_COND(ch, HUNGER) == 0 && !IS_VAMPIRE(ch)) || (GET_COND(ch, THIRST) == 0))) {
        multi = 25;
        return multi;
    }

    // Don't let a player's regeneration completely stagnate...
    multi = MAX(multi, 25);
    return multi;
}

int calcHitBonus(CharData *ch) {
    int bonus = 0;
    
    if(GET_CON(ch) > 18)
        bonus += (GET_CON(ch) - 18)*5;

    if(affected_by_spell(ch, SPELL_REGENERATE))
        bonus += spell_level(ch, SPELL_REGENERATE) + GET_MAX_HIT(ch)/8;        
    
    if(IS_DEFENDER(ch))
        bonus += 30 + 10*GET_ADVANCE_LEVEL(ch);
    
    return bonus;
}

/* Hitpoint gain per game hour */
int hit_gain(CharData * ch)
{    
    // Character is messed
    if (IN_ROOM(ch) == -1) return 0;

    // Delusion health deflates fairly quickly
    if(affected_by_spell(ch, SKILL_DELUSION))
        return -(GET_MAX_HIT(ch) * spell_level(ch, SKILL_DELUSION) / 9);

    /* Are they rotting away ? */
    if (!IS_NPC(ch) && IS_UNDEAD(ch)) {
        return ( (GET_LEVEL(ch) < 50) && (ch->desc) ) ? -GET_LEVEL(ch) : 0;
    }

    /* Race calculations */
    /* vampires don't regenerate in sunlight, unless they're shadow sphered */
    if (!IS_NPC(ch) && IS_VAMPIRE(ch) &&
            IS_SUNLIGHT(IN_ROOM(ch)) && !IS_AFFECTED(ch, AFF_SHADOW_SPHERE))
        return 0;

    if(!IS_NPC(ch)) {
        int suffer = 0;

        if(ROOM_FLAGGED(ch->in_room, ROOM_SUFFER) && ( GET_LEVEL(ch) < LVL_IMMORT ))
            suffer = 1;
        else if(!GET_COND(ch, THIRST) && IS_AMARA(ch))
            suffer = 1;
        else if (IS_IZARTI(ch) && IS_EVIL(ch))
            suffer = 1;
        else if (IS_DEMON(ch) && IS_GOOD(ch))
            suffer = 1;

        if(suffer == 1) {
            if(percentSuccess(2))
                damage(ch, ch, -SUFFER_RATE, TYPE_ROOM_SUFFER);
            return 0;
        }
    }

    int base = calcHitBase(ch);
    int multiplier = calcHitMulti(ch);
    int bonus = calcHitBonus(ch);

    int gain = base*multiplier/100 + bonus;
    return (gain);
}/* hit_gain */

int calcMoveBase(CharData *ch) {
    int base = 0;

    base += graf(age(ch).year, 40, 39, 34, 30, 26, 24, 23);

    /* Class/Level calculations */
    if (GET_CLASS(ch) == CLASS_RANGER)
        base += GET_LEVEL(ch);
    else if (GET_CLASS(ch) == CLASS_ASSASSIN || GET_CLASS(ch) == CLASS_THIEF)
        base += GET_LEVEL(ch)*2/3;
    else
        base += GET_LEVEL(ch)/3;
        
    return base;
}

int calcMoveMulti(CharData *ch) {
    int multi = 100;
    
    if(IS_AFFECTED(ch, AFF_PLAGUE) > 0)
        multi -= 50;

    /* vampires barely regen vigor in sunlight, unless they're shadow sphered */
    if (!IS_NPC(ch) && IS_VAMPIRE(ch) && IS_SUNLIGHT(IN_ROOM(ch)) && !IS_AFFECTED(ch, AFF_SHADOW_SPHERE))
        multi -= 90;
    else if (IS_AFFECTED(ch, AFF_POISON))
        multi -= 90;
    else if ((GET_COND(ch, HUNGER) == 0 && !IS_VAMPIRE(ch)) || (GET_COND(ch, THIRST) == 0))
        multi -= 90;

    if( ROOM_FLAGGED(ch->in_room, ROOM_SUFFER) && ( GET_LEVEL(ch) < LVL_IMMORT ))
        multi -= 90;
    else if (!GET_COND(ch, THIRST) && IS_AMARA(ch))
        multi -= 90;
    else if (IS_IZARTI(ch) && IS_EVIL(ch))
        multi -= 90;
    else if (IS_DEMON(ch) && IS_GOOD(ch))
        multi -= 90;

    /* Position calculations    */
    switch (GET_POS(ch)) {
        case POS_MEDITATING:
        case POS_SLEEPING:
            multi += 120;
            break;
        case POS_RESTING:
            multi += 80;
            break;
        case POS_SITTING:
            multi += 40;	/* Divide by 8 */
            break;
    }

    // Don't let a player's regeneration completely stagnate...
    multi = MAX(multi, 25);
    return multi;
}
int calcMoveBonus(CharData *ch) {
    int bonus = 0;

    if (affected_by_spell(ch, SKILL_ADRENALINE))
        bonus += GET_LEVEL(ch);
    
    if(GET_RACE(ch) == RACE_STROLL)
        bonus += 20 + GET_LEVEL(ch)/2;
    
    if( GET_DEX(ch) > 18 )
        bonus += (GET_DEX(ch) - 18)*3;

    if(affected_by_spell(ch, SPELL_REGENERATE))
        bonus += spell_level(ch, SPELL_REGENERATE)/3 + GET_MAX_MOVE(ch)/6;
    
    return bonus;
}

/* move gain per game hour */
int move_gain(CharData * ch)
{
    int base, multiplier, bonus;

    if (IN_ROOM(ch) == -1) return 0;

    /* Neat and fast */
    if (IS_NPC(ch)) {
        return (GET_LEVEL(ch));
    }

    base = calcMoveBase(ch);
    multiplier = calcMoveMulti(ch);
    bonus = calcMoveBonus(ch);

  int gain = base*multiplier/100 + bonus;
  return (gain);
}

void set_title(CharData * ch, char *title)
{
    if (GET_TITLE(ch) != NULL)
        free(GET_TITLE(ch));

    if (title == NULL){
        char *the = "the ";
        title = calloc( 1, (strlen( the ) + strlen( READ_TITLE(ch))+1));
        strcpy( title, the );
        GET_TITLE(ch) = strcat( title, READ_TITLE(ch));
    }/* if */

    else {
        GET_TITLE(ch) = str_dup(title);
    }/* else */

}/* set_title */

void run_autowiz(void)
{
#if defined(CIRCLE_UNIX) || defined(CIRCLE_WINDOWS)
  if (CONFIG_USE_AUTOWIZ) {
    size_t res;
    char buf[256];
    int i;

#if defined(CIRCLE_UNIX)
    res = snprintf(buf, sizeof(buf), "nice ../bin/autowiz %d %s %d %s %d &",
	CONFIG_MIN_WIZLIST_LEV, WIZLIST_FILE, LVL_IMMORT, IMMLIST_FILE, (int) getpid());
#elif defined(CIRCLE_WINDOWS)
    res = snprintf(buf, sizeof(buf), "autowiz %d %s %d %s",
	CONFIG_MIN_WIZLIST_LEV, WIZLIST_FILE, LVL_IMMORT, IMMLIST_FILE);
#endif /* CIRCLE_WINDOWS */

    /* Abusing signed -> unsigned conversion to avoid '-1' check. */
    if (res < sizeof(buf)) {
      mudlog(CMP, LVL_IMMORT, FALSE, "Initiating autowiz.");
      i = system(buf);
      reboot_wizlists();
    } else
      mlog("Cannot run autowiz: command-line doesn't fit in buffer.");
  }
#endif /* CIRCLE_UNIX || CIRCLE_WINDOWS */
}

void gain_exp_unchecked(CharData *ch, int gain)
{
    // PC mobs do NOT gain exps at all.
    if (IS_AFFECTED(ch, AFF_CHARM) || MOB_FLAGGED(ch, MOB_CONJURED))
        return;
 
    // stop xps clocking over.
    if ((GET_EXP(ch) > 2000000000) && gain > 0)
        return;

    if (!IS_NPC(ch) && ((GET_LEVEL(ch) < 1 || GET_LEVEL(ch) > MAX_MORTAL)))
        return;

    if (IN_ARENA(ch)) return;
    if (ZONE_FLAGGED(world[ch->in_room].zone, ZONE_ARENA)) return;
    if (ZONE_FLAGGED(world[ch->in_room].zone, ZONE_SLEEPTAG)) return;

    /* NPCs just get their exp mod and leave */
    if (IS_NPC(ch)) {
        GET_EXP(ch) += gain;
        return;
    }

    if (gain > 0) {
        if ((GET_EXP(ch) + gain ) > INT_MAX)
            return;

        GET_EXP(ch) += gain;

        /* Did the player just earn a level? */
        if( GET_LEVEL(ch) < MAX_MORTAL &&
           GET_EXP(ch) >= titles[(int) GET_CLASS(ch)][GET_LEVEL(ch) + 1].exp)
        {
            send_to_char("You rise a level!\r\n", ch);
            GET_EXP(ch) = titles[(int) GET_CLASS(ch)][GET_LEVEL(ch) + 1].exp;
            GET_LEVEL(ch) += 1;
            level_up(ch);
            set_title(ch, NULL);
        }
    } else if (gain < 0) {
        long thisLvl = (titles[(int) GET_CLASS(ch)][GET_LEVEL(ch) + 0].exp);
        long nextLvl = (titles[(int) GET_CLASS(ch)][GET_LEVEL(ch) + 1].exp);
        long neg100 = thisLvl - nextLvl + thisLvl;

        GET_EXP(ch) += gain;

        /* uh oh spaghettios! */
        if (GET_EXP(ch) < 0 && IS_MORTAL(ch)) {
            mudlog(NRM, LVL_IMMORT, TRUE, "SOUL DEATH: %s reduced to 0 xps by death, deleting!",
                    GET_NAME(ch));   
            sendChar(ch, "Your soul is too weak to return to the world...");
            sendChar(ch, "Utter darkness surrounds you as you slowly slip "
                         "into oblivion...");
            act("A look of utter despair crosses $n's face as their very soul"
                " is destroyed.", TRUE, ch, 0, 0, TO_ROOM );
            SET_BIT_AR(PLR_FLAGS(ch), PLR_DELETED);
            if (ch->desc) SET_DCPENDING(ch->desc);
        }

        /* Did the player just lose a level? */
        if (GET_LEVEL(ch) > 1 && IS_MORTAL(ch) && GET_EXP(ch) <= neg100) {
            send_to_char("You have lost a level!\r\n", ch);
            GET_LEVEL(ch) -= 1;
            retreat_level(ch);
            set_title(ch, NULL);

            /* make sure they don't drop below 0% at their new level */
            if (GET_EXP(ch) < titles[(int)GET_CLASS(ch)][GET_LEVEL(ch)].exp)
                GET_EXP(ch) = titles[(int)GET_CLASS(ch)][GET_LEVEL(ch)].exp;
        }
    }
}

void gain_exp(CharData *ch, int gain)
{
    gain = MAX(-CONFIG_MAX_EXP_LOSS * GET_LEVEL(ch), gain);
    gain = MIN(CONFIG_MAX_EXP_GAIN, gain);
    gain_exp_unchecked(ch, gain);
}


void gain_exp_regardless( CharData * ch, int gain)
{
    int is_altered = FALSE;

    GET_EXP(ch) += gain;
    if( GET_EXP(ch) < 0 )
        GET_EXP(ch) = 0;

    if( !IS_NPC(ch) ){
        while( GET_LEVEL(ch) < LVL_IMPL &&
               GET_EXP(ch) >= titles[(int) GET_CLASS(ch)][GET_LEVEL(ch) + 1].exp) {
            send_to_char("You rise a level!\r\n", ch);
            GET_LEVEL(ch) += 1;
            level_up(ch);
            is_altered = TRUE;
        }

        if( is_altered ){
            set_title(ch, NULL);
            run_autowiz();
        }
    }
}/* gain_exp_regardless */


void gain_condition(CharData * ch, int condition, int value)
{
  bool intoxicated = (GET_COND(ch, DRUNK) > 0);

  if (GET_COND(ch, condition) == -1)	/* No change */
    return;

  GET_COND(ch, condition) += value;

  GET_COND(ch, condition) = MAX(0, GET_COND(ch, condition));
  if(condition == THIRST && IS_VAMPIRE(ch))
      GET_COND(ch, condition) = MIN(48, GET_COND(ch, condition));
  else
      GET_COND(ch, condition) = MIN(24, GET_COND(ch, condition));

  // Do not spam hungry players who are trying to write.
  if (PLR_FLAGGED(ch, PLR_WRITING))
    return;

  if( GET_COND(ch, condition) == 2){
      switch (condition) {
          case HUNGER:
              send_to_char("You begin to experience hunger pangs.\r\n", ch);
              return;
          case THIRST:
              send_to_char("Your mouth is as dry as a desert.\r\n", ch);
              return;
          default:
              return;
      }
  }

  if(GET_COND(ch, condition) >= 1)
      return;

  switch (condition) {
      case HUNGER:
          send_to_char("Your stomach grumbles loudly.\r\n", ch);
          return;
      case THIRST:
          if (IS_VAMPIRE(ch)) {
              send_to_char("Your desire for &08blood&00 consumes you.\r\n", ch);
              return;
          } else {
              send_to_char("You are parched.\r\n", ch);
              return;
          }
      case DRUNK:
          if(intoxicated)
              send_to_char("You are now sober.\r\n", ch);
          return;
      default:
          break;
  }
}


void
check_idling( CharData * ch )
{
#define VOID_TIME 8
#define EXTRACT_TIME 32
  void Crash_rentsave(CharData *ch, int cost);
  void Crash_cryosave(CharData *ch, int cost);

  /*
  ** CONJURED timer
  */
  int i = 0;

  for(;i < 4;i++) {
      if( GET_CONJ_CNT(ch, i) > 0)
      {
          SET_CONJ_CNT(ch, i) -= 1;
          
          if(SET_CONJ_CNT(ch, i) == 0)
              switch(GET_CLASS(ch)) {
                  case CLASS_RANGER:
                      sendChar( ch, "The creatures of the wild will answer your call again.\r\n" );
                      break;
                  case CLASS_NECROMANCER:
                      sendChar(ch, "The dead will heed your summons once more.\r\n");
                  case CLASS_MAGIC_USER:
                      switch(i) {
                          case TIMER_GATE: sendChar(ch, "Demons will answer your summons again.\r\n"); break;
                          case TIMER_MONSTER: sendChar(ch, "Monsters will answer your summons again.\r\n"); break;
                          case TIMER_ELEMENTAL: sendChar(ch, "Elementals will answer your summons again.\r\n"); break;
                      }
                      break;
                  default:
                      sendChar( ch, "Conjured creatures will answer your summons again.\r\n" );
              }
      }
  }
  
  /*
  ** HUNTED timer
  */
  if( IS_SET_AR( PLR_FLAGS(ch), PLR_HUNTED ) && ch->desc &&
    (--(ch)->player_specials->saved.phunt_countdown <= 0))
  {
    sendChar( ch, "You are no longer hunted.\r\n" );
    unset_hunted_player(ch);
  }
  /*
  ** THIEF timer
  */
  if( IS_SET_AR(PLR_FLAGS(ch), PLR_THIEF) && ch->desc &&
    (--(ch)->player_specials->saved.pthief_countdown <= 0))
  {
    REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_THIEF);
    send_to_char("You are no longer a registered thief.\r\n", ch);
    (ch)->player_specials->saved.pthief_countdown = 0;
  }
  /* KILLER timer */
  if (IS_SET_AR(PLR_FLAGS(ch), PLR_KILLER)
     && (--(ch)->player_specials->saved.pkill_countdown <= 0))
  {
    REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_KILLER);
    send_to_char("You are no longer a registered killer.\r\n", ch);
    (ch)->player_specials->saved.pkill_countdown = 0;
  }
  /* JAILED timer */
  if (IS_SET_AR(PLR_FLAGS(ch), PLR_JAILED)
     && (--(ch)->player_specials->saved.jail_timer <= 0))
  {
    int jail_exit_room;
    REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_JAILED);

    if (PRF_FLAGGED(ch, PRF_GOLD_TEAM) && IN_ROOM(ch) == real_room(GOLD_TEAM_JAIL))
    {
      jail_exit_room = real_room(GOLD_TEAM_START_ROOM);
      GET_HIT(ch) = GET_MAX_HIT(ch);
      GET_MANA(ch) = GET_MAX_MANA(ch);
      GET_MOVE(ch) = GET_MAX_MOVE(ch);
      char_from_room(ch);
      char_to_room(ch, jail_exit_room);
      look_at_room(ch, 0);
    }
    if (PRF_FLAGGED(ch, PRF_BLACK_TEAM) && IN_ROOM(ch) == real_room(BLACK_TEAM_JAIL))
    {
      jail_exit_room = real_room(BLACK_TEAM_START_ROOM);
      GET_HIT(ch) = GET_MAX_HIT(ch);
      GET_MANA(ch) = GET_MAX_MANA(ch);
      GET_MOVE(ch) = GET_MAX_MOVE(ch);
      char_from_room(ch);
      char_to_room(ch, jail_exit_room);
      look_at_room(ch, 0);
    }    
    if (PRF_FLAGGED(ch, PRF_ROGUE_TEAM) && IN_ROOM(ch) == real_room(ROGUE_TEAM_JAIL))
    {
      jail_exit_room = real_room(ROGUE_TEAM_START_ROOM);
      GET_HIT(ch) = GET_MAX_HIT(ch);
      GET_MANA(ch) = GET_MAX_MANA(ch);
      GET_MOVE(ch) = GET_MAX_MOVE(ch);
      char_from_room(ch);
      char_to_room(ch, jail_exit_room);
      look_at_room(ch, 0);
    }    
    sendChar(ch, "Your imprisonement is over.\r\n");
//    if (PRF_FLAGGED(ch, PRF_GOLD_TEAM))
//	jail_exit_room = IN_ROOM(ch);
//    else if (PRF_FLAGGED(ch, PRF_BLACK_TEAM))
//	jail_exit_room = IN_ROOM(ch);
//	else if (PRF_FLAGGED(ch, PRF_ROGUE_TEAM))
//	jail_exit_room = IN_ROOM(ch);
//    else
//	jail_exit_room = getStartRoom(ch);

//    char_to_room(ch, jail_exit_room);
//    look_at_room(ch, 0);
  }
  /*
  ** If your hunted there is NO escape.
  */
  if(( ++(ch->char_specials.timer) > VOID_TIME ) &&
    !IS_SET_AR(PLR_FLAGS(ch), PLR_HUNTED))
  {
    if (GET_WAS_IN(ch) == NOWHERE && ch->in_room != NOWHERE)
    {
      GET_WAS_IN(ch) = ch->in_room;
      end_fight(ch);
      act("$n disappears into the void.", TRUE, ch, 0, 0, TO_ROOM);
      send_to_char("You have been idle, and are pulled into a void.\r\n", ch);
      save_char(ch, NOWHERE);
      Crash_crashsave(ch);
      GET_WAS_IN(ch) = ch->in_room;
      char_from_room(ch);
      char_to_room(ch, 1);
    }
    //else if (ch->char_specials.timer > EXTRACT_TIME)
    //{
      //if (ch->in_room != NOWHERE)
	//char_from_room(ch);
      //char_to_room(ch, 1);
      //if (ch->desc)
	//SET_DCPENDING(ch->desc);
      //ch->desc = NULL;

      //Crash_idlesave(ch);	/* apparently causing problems? */
      //crashRentSave(ch, -1);

      //mudlog( NRM, LVL_LRGOD, TRUE, "%s force-rented and extracted (idle).", GET_NAME(ch));
      //extract_char(ch);
    //}
  }
#undef VOID_TIME
#undef EXTRACT_TIME
}

void regen_update( void ) {
    CharData *i, *next_char;
    int hit, mana, move;
    int vivify_level;

    for( i = character_list; i; i = next_char )
    {
        next_char = i->next;

        if(affected_by_spell(i, SKILL_EMACIATED) && (GET_COND(i, HUNGER) != 0 || IS_VAMPIRE(i)) && GET_COND(i, THIRST) != 0) {
            if(!affected_by_spell(i, SKILL_EMACIATED_MANA) && !affected_by_spell(i, SKILL_EMACIATED_HIT))
                affect_from_char(i, SKILL_EMACIATED);
            else if(percentSuccess(4)) {
                affect_from_char(i, SKILL_EMACIATED_MANA);
                affect_from_char(i, SKILL_EMACIATED_HIT);
            }
        }

        // Dwarves sober up 3x faster than most races (unless they drink more...)
        if(IS_DWARF(i) && !number(0, 35))
            gain_condition(i, DRUNK, -1);

        vivify_level = spell_level(i, SPELL_VIVIFY);
        affect_from_char(i, SPELL_VIVIFY);
        if(GET_POS(i) <= POS_MEDITATING) {
            add_affect(i, i, SPELL_VIVIFY, MIN(vivify_level + 1, 100), APPLY_NONE, 0, -1,
                    0, FALSE, FALSE, FALSE, FALSE);
        }

        if(GET_POS(i) >= POS_MORTALLYW)
        {
            hit = hit_gain(i);
            mana = mana_gain(i);
            move = move_gain(i);

            if(affected_by_spell(i, SPELL_VIVIFY)) {
                if(hit > 0)
                    hit += spell_level(i, SPELL_VIVIFY) * GET_MAX_HIT(i)/600;
                if(mana > 0)
                    mana += spell_level(i, SPELL_VIVIFY) * GET_MAX_MANA(i)/600;
                if(move > 0)
                    move += spell_level(i, SPELL_VIVIFY) * GET_MAX_MOVE(i)/600;
            }

            // Regeneration is increased on pvp holidays.  If you're incapactitated, you will
            // eventually recover.
            if(pvpHoliday(i)) {
                if (GET_POS(i) < POS_SLEEPING)
                    hit = 20;
                else {
                    hit  = (hit  > 0) ? hit *3 : 20;
                    mana = (mana > 0) ? mana*2 : 40;
                    move = (move > 0) ? move*2 : 40;
                }
            }
                

            // 72 seconds per tick...
            if(hit < 0 || GET_HIT(i) < GET_MAX_HIT(i)) {
                if(hit>0)
                    GET_HIT(i) += hit/72 + (hit % 72 > number(0, 71)? 1:0);
                else
                    GET_HIT(i)  = MAX(GET_HIT(i) + hit/72 + (-hit % 72 > number(0, 71)?-1:0), -9);
            }
            else {
                int surplus = (GET_HIT(i) - GET_MAX_HIT(i));
                GET_HIT(i) -= surplus / 72 + (surplus % 72 > number(0, 71)? 1:0);
            }


            if(mana>0)
                GET_MANA(i) = MAX(MIN(GET_MANA(i) + mana/72 + (mana % 72 > number(0, 71)?1:0), GET_MAX_MANA(i)), 0);
            else
                GET_MANA(i) = MIN(GET_MANA(i) + mana/72 + (-mana % 72 > number(0, 71)?-1:0), GET_MAX_MANA(i));

            if(move>0)
                GET_MOVE(i) = MIN(GET_MOVE(i) + move/72 + (move % 72 > number(0, 71)?1:0), GET_MAX_MOVE(i));
            else
                GET_MOVE(i) = MAX(MIN(GET_MOVE(i) + move/72 + (-move % 72 > number(0, 71)?-1:0), GET_MAX_MOVE(i)), 0);

            update_pos(i);
        }
    }
}

/*
** Update PCs, NPCs, and objects
*/
void
point_update( void )
{
    int slot;

    void update_char_objects(CharData * ch); /* handler.c */
    void extract_obj(ObjData * obj);	     /* handler.c */
    void update_char_quests(CharData * ch);  /* quest.c */
    CharData *i, *next_char;
    ObjData  *j, *next_thing, *jj, *next_thing2, *debugnext;
    int loopvar;

    /* characters */
    for( i = character_list; i; i = next_char )
    {
        next_char = i->next;

        // state flags
        i->tickstate = 0;

        /* dismount anyone who's gotten separated from their steed */
        /* Note that it's superfluous to check for both rider AND mount */
        if (i->rider && i->rider->in_room != i->in_room) {
            i->rider->mount = NULL;
            i->rider = NULL;
        }

        /* Prayer timer */
        if (i->player_specials->saved.prayer_time > 0) {
            if (i->player_specials->saved.prayer_time == 1) {
                i->player_specials->saved.prayer_time = 0;
                send_to_char("Your prayers will be heard once again.\r\n", i);
            } else
                i->player_specials->saved.prayer_time -= 1;
        }

        for(slot = 0; slot<4; slot++) {
            if (COOLDOWN(i, slot) ) {
                COOLDOWN(i, slot) -= 1;
                if (!COOLDOWN(i, slot) ) {
                    switch( GET_CLASS(i) ) {
                        case CLASS_DEATH_KNIGHT:
                            break;
                        case CLASS_SOLAMNIC_KNIGHT:
                            break;
                        case CLASS_MAGIC_USER:
                            break;
                        case CLASS_SHADOW_DANCER:
                            if(slot == SLOT_SLIPPERY_MIND)
                                break;
                            else if(slot == SLOT_NODESHIFT)
                                sendChar(i, "You may once again shift your spectrum.\r\n");
                            else sendChar(i, "ERROR!\r\n");
                            break;
                        case CLASS_THIEF:
                            if(slot== SLOT_BLACKJACK) {
                                sendChar(i, "You are able to use blackjack again.\r\n");
                                break;
                            }
                            else sendChar(i, "ERROR!\r\n");
                        case CLASS_ASSASSIN:
                            if(slot == SLOT_DETERRENCE)
                                sendChar(i, "You are able to use deterrence again.\r\n");
                            else sendChar(i, "ERROR!\r\n");
                            break;
                        case CLASS_CLERIC:
                            if(slot == SLOT_SHADOW_FORM)
                                sendChar(i, "You are ready to enter shadow form again..\r\n");
                            else sendChar(i, "ERROR!\r\n");
                        case CLASS_WARRIOR:
                            if(slot == SLOT_REDOUBT)
                                sendChar(i, "You can shield yourself again.\r\n");
                            else if(slot == SLOT_COMMANDING_SHOUT)
                                sendChar(i, "You can shout commands again.\r\n");
                            else sendChar(i, "ERROR!\r\n");
                            break;
                        case CLASS_SHOU_LIN:
                            break;
                        case CLASS_RANGER:
                            break;
                        case CLASS_NECROMANCER:
                            if(slot == SLOT_QUICKEN)
                                sendChar(i, "You may once again rise from the grave.\r\n");
                            else if(slot == SLOT_METAMORPHOSIS)
                                sendChar(i, "You may once again metamorphisize.\r\n");
                            else sendChar(i, "ERROR!\r\n");
                            break;

                        default:
                            sendChar(i, "ERROR!\r\n");
                            break;
                    }
                }
            }
        }

        if( IS_AFFECTED(i, AFF_PLAGUE )) infectious(i);

        if( !IS_NPC(i) )
        {
            update_char_objects(i);
            if( GET_LEVEL(i) < LVL_GOD )
                check_idling(i);
            update_char_quests(i);
        }
        gain_condition(i, HUNGER, IS_AFFECTED(i, AFF_REGENERATE)? -2:-1);
        gain_condition(i, DRUNK, -1);

        /* Amara get thirsty in different ways */
        if (IS_AMARA(i)) {
            if (IN_ROOM(i) >= 0 && IN_ROOM(i) <= top_of_world) {
                switch (SECT(IN_ROOM(i))) {
                    case SECT_WATER_SWIM:
                    case SECT_WATER_NOSWIM:
                        gain_condition(i, THIRST, 1);
                        break;
                    case SECT_UNDERWATER:
                    case SECT_UNDERWATER_RIVER:
                        gain_condition(i, THIRST, 24);
                        break;
                    default:
                        gain_condition(i, THIRST, -2);
                        break;
                }
            } else gain_condition(i, THIRST, -2);
        } else gain_condition(i, THIRST, -1);
    }/* for */

    debugnext = NULL;
    /* objects */
    for( j = object_list; j; j = next_thing )
    {
        next_thing = j->next;	/* Next in object list */
        debugnext = j;		// we didn't crash if we got here

        if( IS_SET_AR( j->obj_flags.extra_flags, ITEM_TIMED ))
        {
            if( GET_OBJ_TIMER(j) > 0 )
                GET_OBJ_TIMER(j)--;

            if (GET_OBJ_TIMER(j) == 0) {
                if(contains_soulbound(j)) {
                    GET_OBJ_TIMER(j) = 1;
                    continue;
                }

                if (SCRIPT_CHECK(j, OTRIG_TIMER)) {
                    REMOVE_BIT_AR(j->obj_flags.extra_flags, ITEM_TIMED);
                    timer_otrigger(j);
                    continue;    // don't do anything more with this
                }
            }

            if( GET_OBJ_TYPE(j) == ITEM_KEY )
            {
                static char *keyVaporMsgs[] = {
                    "$p vanishes with a flash.",
                    "$p begins to shake violently.",
                    "$p begins to vibrate.",
                    "$p begins to hum.",
                    "$p begins to glow."
                };

                if( GET_OBJ_TIMER(j) < (sizeof( keyVaporMsgs )/sizeof( *keyVaporMsgs )))
                {
                    int vaporMsg = GET_OBJ_TIMER(j);

                    if( j->carried_by )
                        act( keyVaporMsgs[ vaporMsg ], FALSE, j->carried_by, j, 0, TO_CHAR);

                    else if( j->worn_by )
                    {
                        act( keyVaporMsgs[ vaporMsg ], FALSE, j->worn_by, j, 0, TO_CHAR);
                        for( loopvar = 0; loopvar < NUM_WEARS; loopvar++ )
                        {
                            if( j->worn_by->equipment[loopvar] == j )
                                j->worn_by->equipment[loopvar] = 0;
                        }
                    }
                    else if( j->in_room != NOWHERE && world[j->in_room].people )
                    {
                        act( keyVaporMsgs[ vaporMsg ], FALSE, world[j->in_room].people, j, 0, TO_CHAR);
                        act( keyVaporMsgs[ vaporMsg ], FALSE, world[j->in_room].people, j, 0, TO_ROOM);
                    }
                    extract_obj(j);
                    continue;
                }/* ITEM_KEY has timed out */
            }/* if ITEM_KEY */
            else if (GET_OBJ_TYPE(j) == ITEM_AFFECT)
            {
                if (!GET_OBJ_TIMER(j)) {
                    if (j->in_room != NOWHERE && world[j->in_room].people) {
                        act(j->action_description, FALSE, world[j->in_room].people,
                                j, 0, TO_CHAR);
                        act(j->action_description, FALSE, world[j->in_room].people,
                                j, 0, TO_ROOM);
                    }
                    extract_obj(j);
                    continue; // object gone, don't act further on it!
                }
            }
            else if( !GET_OBJ_TIMER(j) )
            {
                /* The object timed out - delete it */
                if( j->carried_by )
                    act( "$p crumbles to dust and is blown away.", FALSE, j->carried_by, j, 0, TO_CHAR );

                else if( j->worn_by )
                {
                    act("$p crumbles to dust and is blown away.", FALSE, j->worn_by, j, 0, TO_CHAR);
                    unequip_char( j->worn_by, j->worn_at );
                }
                else if( j->in_room != NOWHERE && world[j->in_room].people )
                {
                    act( "$p crumbles to dust and is blown away.",
                            FALSE, world[j->in_room].people, j, 0, TO_CHAR);
                    act( "$p crumbles to dust and is blown away.",
                            FALSE, world[j->in_room].people, j, 0, TO_ROOM);
                }
                extract_obj(j);
                continue; // object gone, don't act further on it!
            }
        } /* if OBJ_TIMED */

        /* if this looks like a portal */
        if ( (GET_OBJ_RNUM(j) >= 0 && GET_OBJ_RNUM(j) <= top_of_objt) &&
                obj_index[GET_OBJ_RNUM(j)].func == portal_proc &&
                GET_OBJ_TYPE(j) == ITEM_OTHER )
        { /* Mage created portals are type other, permanent portals are type portal. */
            /* Permanent portals thus don't decay. */
            if (GET_OBJ_VAL(j, 2) > 0)
                GET_OBJ_VAL(j,2)--;
        }
        /*
         ** Digger
         */
        /* If this is a corpse */
        if ((GET_OBJ_TYPE(j) == ITEM_CONTAINER) && GET_OBJ_VAL(j, 3)) {
            /* timer count down */
            if (GET_OBJ_TIMER(j) > 0)
                GET_OBJ_TIMER(j)--;

            // PC corpses which are empty will decay eventually..
            if(!CAN_WEAR(j, ITEM_WEAR_TAKE) && !(j->contains))
            {
                GET_OBJ_TIMER(j) = MAX(GET_OBJ_TIMER(j) / 3, 0);
            }

            if (!GET_OBJ_TIMER(j)) {
                if(contains_soulbound(j)) {
                    GET_OBJ_TIMER(j) = 1;
                    continue;
                }

                if (j->carried_by)
                    act("$p decays in your hands.", FALSE, j->carried_by, j, 0, TO_CHAR);
                
                else if ((j->in_room != NOWHERE) && (world[j->in_room].people)) {
                    static char *decay_messages[] = {
                        "A quivering hoard of maggots consumes $p.",
                        "A flock of vultures swoop down from the sky to devour $p.",
                        "The $p rots and decays as the shards of bone are blown to the four winds.",
                        "The $p rots and decays leaving behind the pungent stench of death.",
                        "A bolt of holy fire streaks from the heavens to burn the corpse of $p to ash.",
                        "The $p rots to ash and is swept away by the winds of time."
                    };
                    int decay_idx = (int)( random() % ( sizeof( decay_messages ) / sizeof( *decay_messages )));

                    act( decay_messages[ decay_idx ], TRUE, world[j->in_room].people, j, 0, TO_ROOM);
                    act( decay_messages[ decay_idx ], TRUE, world[j->in_room].people, j, 0, TO_CHAR);
                }/* JBP */
                
                for (jj = j->contains; jj; jj = next_thing2) {
                    next_thing2 = jj->next_content;	/* Next in inventory */
                    obj_from_obj(jj);

                    if (j->in_obj) {
                        if ( GET_OBJ_TYPE(j) != ITEM_KEY    &&
                                GET_OBJ_TYPE(j) != ITEM_SCROLL &&
                                GET_OBJ_TYPE(j) != ITEM_POTION &&
                                GET_OBJ_TYPE(j) != ITEM_DUST   &&
                                (GET_OBJ_VNUM(j) == 1460 ||
                                GET_OBJ_VNUM(j) == 1461 ||
                                GET_OBJ_VNUM(j) == 1462   ) )
                            continue;  // Refrigeration to keep food from rotting.
                        obj_to_obj(jj, j->in_obj);
                    }
                    else if (j->carried_by)
                        obj_to_room(jj, j->carried_by->in_room);
                    else if (j->in_room != NOWHERE)
                        obj_to_room(jj, j->in_room);
                    else
                    {
                        /* OLD WAY: assert(FALSE); */
                        mudlog(NRM, LVL_IMMORT, TRUE, "SYSERR: Something is wrong with a container." );
                        obj_to_room(jj, real_room(1201));
                    }
                }
                extract_obj(j);
            }
        }

        /* Imhotep: Added support for ITEM_TROPHY pieces that decay after
         * a given MUD date */        
        if(IS_OBJ_STAT(j, ITEM_TROPHY)) {
            if(GET_OBJ_TIMER(j) < TICKS_SO_FAR) {
                if (j->carried_by)
                    act("$p shimmers and vanishes out of your hands!", FALSE, j->carried_by, j, 0,
                            TO_CHAR);
                else if (j->worn_by) {
                    act("$p shimmers and vanishes!", FALSE, j->worn_by, j, 0, TO_CHAR);
                    unequip_char(j->worn_by, j->worn_at);
                } else if (j->in_room != NOWHERE && world[j->in_room].people) {
                    act("$p shimmers and vanishes!", FALSE,
                            world[j->in_room].people, j, 0, TO_CHAR);
                    act("$p shimmers and vanishes!", FALSE,
                            world[j->in_room].people, j, 0, TO_ROOM);
                }
                extract_obj(j);
                continue;
            }
        }
    }
}/* point_update */

/* pulse heal anyone affected by AFF_PULSE_HIT or AFF_PULSE_MANA */
/* nb, also mess up anyone affected by AFF_POISON */
void pulse_heal(void)
{
    CharData *ch;
    int gain = number(18,24);

    for (ch = character_list; ch; ch = ch->next) {
        if(ch->in_room == NOWHERE)
            continue;

        if(!(pvpFactor() > 1)) {
            if( GET_POS(ch) == POS_INCAP )      damage(ch, ch, 1, TYPE_SUFFERING);
            else if( GET_POS(ch) == POS_MORTALLYW )  damage(ch, ch, 2, TYPE_SUFFERING);
            else if( GET_POS(ch) == POS_DEAD) {
                if(IN_ARENA(ch) || IN_QUEST_FIELD(ch) ||
                   ZONE_FLAGGED(world[ch->in_room].zone, ZONE_ARENA) ||
                   ZONE_FLAGGED(world[ch->in_room].zone, ZONE_SLEEPTAG))
                    // If they're dying in the arena, they eventually get better (or killed by someone)
                {
                    GET_HIT(ch) = number(GET_HIT(ch), 1);
                    sendChar(ch, "You slowly recover.\r\n");
                    update_pos(ch);
                }
                else {
                    raw_kill(ch, NULL);
                    continue;
                }
            }
        }

        if (IS_AFFECTED(ch, AFF_PULSE_HIT))
            if (GET_HIT(ch) < GET_MAX_HIT(ch)) GET_HIT(ch) += gain;
        if (IS_AFFECTED(ch, AFF_PULSE_MANA))
            if (GET_MANA(ch) < GET_MAX_MANA(ch)) GET_MANA(ch) += gain;
        if (IS_AFFECTED(ch, AFF_POISON)) doPoison(ch);
        if (IS_AFFECTED(ch, AFF_DISEASE)) doDisease(ch);
	if (affected_by_spell(ch, SKILL_INVIGORATE)) doInvigorate(ch);
        if (IS_BOUNTY_HUNTER(ch) && GET_ADVANCE_LEVEL(ch) >= 1 && IS_AFFECTED(ch, AFF_HIDE)) GET_MOVE(ch) = MIN(GET_MOVE(ch) + 3*gain, GET_MAX_MOVE(ch));
        if (IS_PRESTIDIGITATOR(ch)) GET_MANA(ch) = MIN(GET_MANA(ch) + GET_ADVANCE_LEVEL(ch) * 2, GET_MAX_MANA(ch));
        if (affected_by_spell(ch, SPELL_HIPPOCRATIC_OATH)) GET_MANA(ch) = MIN(GET_MANA(ch) + 25, GET_MAX_MANA(ch));
        if (affected_by_spell(ch, SKILL_PET_MEND)) GET_HIT(ch) = MIN(GET_HIT(ch) * 115 / 100, GET_MAX_HIT(ch));
        if (IS_HOLY_PRIEST(ch)) GET_MANA(ch) = MIN(GET_MANA(ch) + 10 + 2*GET_ADVANCE_LEVEL(ch), GET_MAX_MANA(ch));

        /* The room might be poisoned! (Or later, otherwise dangerous) */
        if (ch->in_room != NOWHERE) {
            if (ROOM_FLAGGED(ch->in_room, ROOM_POISONED)) {
                if (!mag_savingthrow(ch, SAVING_SPELL)) {
                    act("$n chokes and gags!", TRUE, ch, 0, 0, TO_ROOM);
                    act("You choke and gag!", TRUE, ch, 0, 0, TO_CHAR);
                    add_affect( ch, ch, SPELL_POISON, 30, APPLY_NONE, 0, 5 TICKS,
                            AFF_POISON, FALSE, FALSE, FALSE, FALSE);
                }
            }
        }

        if(IS_DEFENDER(ch) && !affected_by_spell(ch, SKILL_DEFENDER_HEALTH))
            add_affect(ch, ch, SKILL_DEFENDER_HEALTH, GET_LEVEL(ch), APPLY_HIT, GET_ADVANCE_LEVEL(ch)*5, -1, FALSE, FALSE, FALSE, FALSE, FALSE);

    }
}
