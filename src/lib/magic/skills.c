/*
**++
**  RCSID:     $Id: skills.c,v 1.11 2005/03/04 21:39:36 raven Exp $
**
**  FACILITY:  RavenMUD
**
**  LEGAL MUMBO JUMBO:
**
**      This is based on code developed for DIKU and Circle MUDs.
**
**  MODULE DESCRIPTION:
**
**  AUTHORS:
**
**      Digger from RavenMUD, Vex from RavenMUD
**
**  NOTES:
**
**      Use 132 column editing in here.
**
*/

/*
** STANDARD U*IX INCLUDES
*/
#include "general/conf.h"
#include "general/sysdep.h"


/* #define SHOW_AVOIDS */

#ifdef SHOW_AVOIDS
char *skill_name[] = { "UNDEFINED",
  "BACKSTAB", "BASH", "HIDE", "KICK", "PICK_LOCK", "PUNCH",
  "RESCUE", "SNEAK", "STEAL", "TRACK", "DISARM", "SECOND_ATTACK",
  "THIRD_ATTACK", "SCAN", "LAY_HANDS", "FISTS_OF_FURY", "THROW",
  "SHOOT", "KNOCK", "TRIP", "BLINDING_STRIKE", "HAMSTRING",
  "ENHANCED_DAMAGE", "RETREAT", "TURN", "BUTCHER", "DODGE", "TRAP",
  "DISARM_TRAP", "PALM", "FIND_WEAKNESS", "SKIN", "FEIGN_DEATH",
  "ART_DRAGON", "ART_SNAKE", "ART_TIGER", "ART_CRANE", "CIRCLE",
  "DUST", "STALK", "ART_WIND", "ENVENOM", "PARRY", "SWEEP",
};
#endif

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
#include "util/weather.h"
#include "magic/sing.h"

/*
** advanceSkill
*/
void advanceSkill( CharData *caster,
              const int skill_id,
              const int max_learn,
              char *advancement_string,
              const int dex_affect, const int int_affect, const int wis_affect )
{
#   define MIN_INT_TO_AFFECT  15
#   define MIN_DEX_TO_AFFECT  13
#   define MIN_WIS_TO_AFFECT  14
#   define MIN_STR_TO_AFFECT  17

#   define MIN_SKILL_TO_AUTO  60
#   define USES_PER_AUTO_ADV  100
#   define USE_AFFECT_MAX     3
#   define USE_AFFECT_MIN     1

    int dex_bonus    = ( dex_affect ? GET_DEX(caster) - MIN_DEX_TO_AFFECT : 0 );
    int int_bonus    = ( int_affect ? GET_INT(caster) - MIN_INT_TO_AFFECT : 0 );
    int wis_bonus    = ( wis_affect ? GET_WIS(caster) - MIN_WIS_TO_AFFECT : 0 );
    int total_affect = USE_AFFECT_MIN + dex_bonus + int_bonus + wis_bonus;

    if (IS_NPC(caster)) return;  /* NPC skills don't get better with use! */
    
    if( ROOM_FLAGGED( caster->in_room, ROOM_NOMAGIC )) return;

    if(( GET_SKILL( caster, skill_id ) <  MIN_SKILL_TO_AUTO ) ||
       ( GET_SKILL( caster, skill_id ) >= max_learn ))
        return;

    if( total_affect > USE_AFFECT_MAX )
        total_affect = USE_AFFECT_MAX;
    if( total_affect < USE_AFFECT_MIN )
	total_affect = USE_AFFECT_MIN;

    SET_USAGE( caster, skill_id, GET_USAGE( caster, skill_id ) + total_affect );

    if( GET_USAGE( caster, skill_id ) > USES_PER_AUTO_ADV ){
        SET_USAGE( caster, skill_id, 0 );
        SET_SKILL( caster, skill_id, GET_SKILL( caster, skill_id ) + 1 );
        if( GET_SKILL( caster, skill_id ) % 5 == 0 ){
            mudlog(NRM, LVL_IMMORT, TRUE, "(ADVANCE) %s just advanced %s to %d",
                      GET_NAME(caster), spells[skill_id], GET_SKILL(caster, skill_id));
            if( advancement_string != NULL ){
                sendChar( caster, "%s\r\n", advancement_string );
            }
        }/* if */
    }/* if */

    if(affected_by_spell(caster, SPELL_FAST_LEARNING) && percentSuccess(50))
        advanceSkill(caster, skill_id, max_learn, advancement_string, dex_affect, int_affect, wis_affect);

    // Some skills level up more quickly automatically..
    if((skill_id == SKILL_BLACKJACK || skill_id == SKILL_REDOUBT) && percentSuccess(75))
        advanceSkill(caster, skill_id, max_learn, advancement_string, dex_affect, int_affect, wis_affect);
    
    if(spell_duration(caster, SPELL_FAST_LEARNING) > 10 TICKS)
        affect_from_char(caster, SPELL_FAST_LEARNING);
}/* advanceSkill */



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      competenceDescription -
**
**  FORMAL PARAMETERS:
**
**      learned:
**          The percentage that a skill has been learned.
**
**  RETURN VALUE:
**
**      Character pointer to a static string to display.
**
**  DESIGN:
**
**      This
** 
*/
char
*competenceDescription( int learned )
{
    static char *competence_string[] = {
        /* 00 */ "(&14not learned)&00",     /* 05 */ "(&08inept&00)",
        /* 10 */ "(&08pathetic&00)",        /* 15 */ "(&08horrendous&00)",
        /* 20 */ "(&08pitiful&00)",         /* 25 */ "(&08awful&00)",
        /* 30 */ "(&12bad&00)",             /* 35 */ "(&12pretty bad&00)",
        /* 40 */ "(&12poor&00)",            /* 45 */ "(&10below average&00)",
        /* 50 */ "(&10average&00)",         /* 55 */ "(&10above average&00)",
        /* 60 */ "(&11fair&00)",            /* 65 */ "(&11promising&00)",
        /* 70 */ "(&11good&00)",            /* 75 */ "(&13very good&00)",
        /* 80 */ "(&13superb&00)",          /* 85 */ "(&13excellent&00)",
        /* 90 */ "(&09adept&00)",           /* 95 */ "(&09true meaning&00)",
        /* XX */ "(&22godlike&00)"
    };
    int competence_level = 0;

    learned = CLAMP_VALUE( 0, learned, 99 );

    if( learned > 0 )
        competence_level = CLAMP_VALUE( 0, ((learned+5)/5), 20 );

    return( competence_string[ competence_level ] );

}/* competenceDescription */


  // Changed the way spell success is calculated.  Now, each +x spell success
  // buff will give it a x/100 chance of being a guarenteed success.  For
  // example, if a skill would succeed 70% of the time, and the player has
  // two pieces with +5 spell success on, it would increase the chance to
  // 73% chance of success.  This has the affect of giving diminishing
  // returns on +spell success bonuses.
  // Skills that reduce spellcasting success are considered at the bottom.
  // They will additively take away from the success of casting.  For
  // example, if you have a 70% chance of reduced casting and a spell
  // on you reduces casting by 5%, you succeed 65% of the time.
int equipmentSkillSuccess(CharData *ch) {
    int j, i;

    // 10% chance of success for aspect of the monkey
    if(GET_ASPECT(ch) == ASPECT_MONKEY && percentSuccess(10))
        return TRUE;

    for( j = 0; j < NUM_WEARS; j++ )
    {
        if( ch->equipment[j])
            for (i = 0; i < MAX_OBJ_AFFECT; i++)
            {
                if (ch->equipment[j] &&
                        (ch->equipment[j]->affected[i].location == APPLY_SKILL_SUCCESS ||
                        ch->equipment[j]->affected[i].location == APPLY_SKILL_SUCCESS_DEPRECATED) )
                {
                    if (ch->equipment[j]->affected[i].location < 0) {
                        // Do nothing.  Add something later?
                    }
                    else if (percentSuccess(ch->equipment[j]->affected[i].location))
                        return TRUE;
                }
            }
    }

    // If we get this far, the equipment didn't help out :(
    return FALSE;
}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      skillSucess -
**
**  FORMAL PARAMETERS:
**
**      ch:
**          A pointer to the character that is trying to use the skill/spell.
**
**      skill_id:
**          The skill that is being used (ie, SKILL_BASH, SKILL_DISARM).
**
**  RETURN VALUE:
**
**      Boolean
**
**  DESIGN:
**
**    Rather than look at the same definition of this code OVER and OVER
**    again it's been made into a function that will make the skill code
**    more readable and generic. Basically, this function is called with
**    the character record and the skill id and then the roll is made and
**    compared. If the random roll is higher than the char's learned ability
**    then the skill failed and a false is returned.
*/

#define MOB_BASE_PERCENT         35
#define MOB_PERCENT_ADD           5

int
skillSuccess( CharData *ch, int skill_id )
{
  int chance;
  struct affected_type *af;

  /* Vex - I'm paranoid, ok? */
  if( skill_id > MAX_SKILLS || skill_id < 0 )
  {
      mudlog(NRM, LVL_IMMORT, TRUE, "SYSERR: Invalid skill %d passed to skillSuccess!", skill_id);
      return 0;
  }

  if( IS_NPC(ch))
  {
      int min_level = spell_info[ skill_id ].min_level[ (int)GET_CLASS(ch) ];
      if( GET_LEVEL(ch) < min_level ) return 0;
      chance = GET_MOB_SKILL_SUC(ch);
  }
  else
      chance = GET_SKILL(ch, skill_id);

  // Special checks done here. Vex.
  if (chance > 0) 
  {
      /* Do they have an item to improve their skills? */
      if(equipmentSkillSuccess(ch))
          return TRUE;

      // Spells can positively or negatively affect skill_success.  This calculates
      // similarly to equipment
      for (af = ch->affected; af; af = af->next) {
          if(af->location == APPLY_SKILL_SUCCESS) {
              if(af->modifier > 0 && percentSuccess(af->modifier)) {
                  return TRUE;
              }
              else if(af->modifier < 0 && percentSuccess(-(af->modifier))) {
                  return FALSE;
              }
          }
      }

      /* Fervor gives you more success rate */
      chance += (int)(GET_SKILL(ch, SKILL_FERVOR)/12);

      // Some classes get a bonus chance to succeed at skills.
      if( IS_CHI_WARRIOR(ch) && percentSuccess(GET_ADVANCE_LEVEL(ch) *3/2))
          return TRUE;
  }

  /* Check for AFF_APOCALYPSE */
  if (FIGHTING(ch) && IS_AFFECTED(FIGHTING(ch), AFF_APOCALYPSE)) {
      chance -= MAX(10, MIN(GET_LEVEL(FIGHTING(ch)) - GET_LEVEL(ch) + 2, 25));
  }

  /* Check for AFF_MISSION */
  if (FIGHTING(ch) && IS_AFFECTED(FIGHTING(ch), AFF_MISSION)) {
    if (spell_info[skill_id].violent && IS_EVIL(ch))
      chance -= MAX(10, MIN(GET_LEVEL(FIGHTING(ch)) - GET_LEVEL(ch) + 2, 25));
    else if (spell_info[skill_id].violent && IS_NEUTRAL(ch))
      chance -= MAX(10, MIN(GET_LEVEL(FIGHTING(ch)) - GET_LEVEL(ch) + 2, 25))/2;
  }

  /* Check for SPELL_ENERGY_DRAIN */
  if (affected_by_spell(ch, SPELL_ENERGY_DRAIN) &&
          (skill_id <= MAX_SPELLS) && (skill_id > 0))
      chance -= 5;

  if(GET_COND(ch, DRUNK) > 8)
      chance -= GET_COND(ch, DRUNK);

  // Non-necromancers are next to useless in Necropolis.  Pets ARE useless
  if(IN_NECROPOLIS(ch)) {
      if( !IS_NPC(ch) && GET_CLASS(ch) != CLASS_NECROMANCER)
          chance = MIN(chance, 10);
      else if( IS_NPC(ch) && ch->master && !IS_NPC(ch->master) &&
              GET_CLASS(ch->master) != CLASS_NECROMANCER)
          chance = MIN(chance, 5);
  }

  chance = (chance > 99 ? 99 : chance);

  return( percentSuccess( chance ));
}


/* ============================================================================ 
artMonkey
This function performs the skill "art of the monkey" for the shou-lin class.
You can invoke this function for any skill the shou-lin might be able to avoid
with the art of the monkey. The parameters are:
ch - the attacker
victim - the shou-lin
skill_id - skill being avoided
============================================================================ */
int
artMonkey( CharData *ch, CharData *victim, int skill_id )
{
  /* Mortius : This is working way too much for players, lets set it a bit
     lower as a test to see if players notice */
  if (!IS_NPC(victim) && number(1, 100) > 75) return 0;
  
  /* Craklyn : Imm mobs are much too good at blocking.  We'll give them a shot
   *  of getting it through at least. */
  if (number(1, 100) > 95) return 0;  

  if( !AWAKE(victim)) return 0;

  if( !CAN_SEE(victim, ch)) return 0;

  if( skillSuccess(victim, SKILL_ART_MONKEY) && GET_ASPECT(victim) == ASPECT_MONKEY &&
     !(skillSuccess(ch, SKILL_ART_MONKEY) && GET_ASPECT(ch) == ASPECT_MONKEY))
  {
      sprintf(buf, "%s swiftly blocks your %s.", GET_NAME(victim),spells[skill_id]);
      act( buf, FALSE, ch, 0, victim, TO_CHAR);
      sprintf(buf, "You swiftly block %s's %s.", GET_NAME(ch),spells[skill_id]);
      act( buf, FALSE, ch, 0, victim, TO_VICT );
      sprintf(buf, "$N swiftly blocks $n's %s.", spells[skill_id]);
      act( buf, FALSE, ch, 0, victim, TO_NOTVICT );
      return 1;
  }

  return 0;

}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      victimAvoidsSkill -
**
**  FORMAL PARAMETERS:
**
**      ch:
**          A pointer to the character structure that is trying to use the skill/spell.
**
**      victim:
**          A pointer to the character structure that is the target of the skill.
**
**      skill_id:
**          The skill that is being used (ie, SKILL_BASH, SKILL_DISARM).
**
**  RETURN VALUE:
**
**      Boolean
**
**  DESIGN:
**
**      In order to add an enhanced version of combat we must check for other parameters
**	during skill usage. A victim's level, strength, dexterity,
**	may all play a role in avoiding a bash or a trip.
** 
*/

int
victimAvoidsSkill( CharData *ch, CharData *victim, int skill_id )
{
    int attacker_roll;
    int attacker_base;
    int attacker_adjust;

    int victim_roll = number(1, 101);
    int evasive_mob = 0;
    
    if( IS_NPC(ch) ){
        int min_level = spell_info[skill_id].min_level[(int)GET_CLASS(ch)];

        if (GET_LEVEL(ch) < min_level ) return 0;
        attacker_base = MIN( (MOB_BASE_PERCENT + ( (GET_LEVEL(ch) - min_level )*MOB_PERCENT_ADD )) , MAX_MOB_SKILL_PERCENT );
    }
    else {
        attacker_base = GET_SKILL(ch,skill_id);
    }

    /*
    ** Now, let's make adjustments based on levels and stats.
    */
    attacker_base    = ( attacker_base < 50 ? 0 : attacker_base - 50 );

    attacker_adjust  = 3 * ( GET_LEVEL(ch) - GET_LEVEL(victim) );
    attacker_adjust += 2 * ( GET_DEX(ch) - GET_DEX(victim) );
    attacker_adjust += STRENGTH_APPLY_INDEX(ch) - STRENGTH_APPLY_INDEX(victim); 

    /* Make any adjustments for particular skills that are unusually effective/ineffective */
    switch(skill_id) {
        case SKILL_BASH:
            attacker_adjust += 15;
            break;
        case SKILL_FISTS_OF_FURY:
            attacker_adjust += 12;
            break;
        case SKILL_SWEEP:
        case SKILL_HAMSTRING:
        case SKILL_TRIP:
            attacker_adjust += 9;
        case SKILL_KNOCK:
        case SKILL_SHADOW_DANCE:
        case SKILL_CIRCLE:
        case SKILL_GORE:
            attacker_adjust += 7;
            break;
        default:
            attacker_adjust +=5;
            break;
    }

    attacker_roll = attacker_base + (attacker_adjust * MOB_PERCENT_ADD);

    if(IS_NPC(victim) && IS_SET_AR(MOB_FLAGS( victim ), MOB_EVASIVE))
        evasive_mob = percentSuccess(75);

    if((victim_roll > attacker_roll) || evasive_mob) {
	sprintf(buf, "$N evades your attempted %s.\r\n", spells[skill_id]);
        CAP(buf);
        act( buf, FALSE, ch, 0, victim, TO_CHAR);
	
        sprintf(buf, "You manage to evade $n's %s.\r\n", spells[skill_id]);
        act( buf, FALSE, ch, 0, victim, TO_VICT );

        sprintf(buf, "$N evades $n's %s.", spells[skill_id]);
        act( buf, FALSE, ch, 0, victim, TO_NOTVICT );
	return 1;
    }

    if (artMonkey(ch, victim, skill_id))
	return 1;

    return 0;

}/* victimAvoidsSkill */

/* ============================================================================ 
This function handles any racial skill avoidance stuff.
============================================================================ */
#define IS_BIG(ch)    ( IS_DRAGON(ch) || IS_GIANT(ch) )
int raceSkillAvoid(CharData *ch, CharData *vict, int the_skill)
{
    int chance;

    if(GET_HIT(ch) <= 0) return 0;

    /* this is %chance victim might avoid ch's skill. */
    chance = 
	     IS_BIG(vict) ? 20 : 0 +
	     HAS_WINGS(vict) ? 20 : 0 +
	     (2 * (STRENGTH_APPLY_INDEX(vict) - STRENGTH_APPLY_INDEX(ch)) ) +
	     GET_DEX(vict) - GET_DEX(ch) +
	     (3 * (GET_LEVEL(vict) - GET_LEVEL(ch)) ) +
	     (int)( GET_HIT(vict)/GET_HIT(ch) );

    if (the_skill == SKILL_BASH || the_skill == SKILL_FISTS_OF_FURY)
	chance -=10; /* bash and fists a bit more effective. */

    /* giant players are more likely to succeed at this */
    if (IS_GIANT(vict) && !IS_NPC(vict)) chance *= 3;

    switch(the_skill) {
    case SKILL_SWEEP:
    case SKILL_TRIP:
    case SKILL_BASH:
    case SKILL_SHIELD_BASH:
    case SKILL_HAMSTRING:
	if ( HAS_WINGS(vict) && (number(1, 100) < chance) ) {
	    sprintf(buf, "%s stabilizes $Mself against your %s with $S wings.\r\n", GET_NAME(vict), spells[the_skill]);
	    act( buf, FALSE, ch, 0, vict, TO_CHAR);
	    sendChar(vict, "You stabilize yourself with your wings against %s's %s.\r\n", GET_NAME(ch), spells[the_skill]);
	    sprintf(buf, "%s stabilizes $Mself against %s's %s with $S wings.", GET_NAME(vict), GET_NAME(ch), spells[the_skill]);
            act( buf, FALSE, ch, 0, vict, TO_NOTVICT );
	    return 1;
	}
	/* fall through */
    case SKILL_KICK:
    case SKILL_FISTS_OF_FURY:
    case SKILL_KNOCK:
	if ( IS_BIG(vict) && !IS_BIG(ch) && (number(1, 100) < chance) ) {
	    sprintf(buf, "%s shrugs off your %s.\r\n", GET_NAME(vict), spells[the_skill]);
	    act( buf, FALSE, ch, 0, vict, TO_CHAR);
	    sendChar(vict, "You shrug off %s's %s.\r\n", GET_NAME(ch), spells[the_skill]);
	    sprintf(buf, "%s shrugs off %s's %s.", GET_NAME(vict), GET_NAME(ch), spells[the_skill]);
            act( buf, FALSE, ch, 0, vict, TO_NOTVICT );
	    return 1;
	}
	break;
    default:
	mudlog(NRM, LVL_IMMORT, TRUE, "Unknown skill %d passed to raceSkillAvoid.", the_skill);
    }
    return 0; /* they don't avoid it. */
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      victimIsAngry -
**
**  FORMAL PARAMETERS:
**
**      chanceToAnger:
**          The percent chance of angering the target.
**
**  RETURN VALUE:
**
**      Boolean
**
**  DESIGN:
**
**      This func is used to generate a random result to see if the target of
**      a skill was angered by its use.
** 
*/
int
victimIsAngry( int chanceToAnger )
{
    return( percentSuccess( chanceToAnger ));

}/* victimIsAngry */
