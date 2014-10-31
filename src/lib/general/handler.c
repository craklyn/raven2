/* ************************************************************************
*   File: handler.c                                     Part of CircleMUD *
*  Usage: internal funcs: moving and finding chars/objs                   *
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
#include "general/comm.h"
#include "general/handler.h"
#include "actions/interpreter.h"
#include "general/class.h"
#include "general/objsave.h"
#include "magic/spells.h"
#include "magic/magic.h"
#include "util/weather.h"
#include "scripts/dg_scripts.h"
#include "specials/flag_game.h"
#include "magic/sing.h"
#include "actions/fight.h"
#include "actions/act.h"          /* ACMDs located within the act*.c files */
#include "specials/mobact.h"      /* for the clearMemory function */


/* local file scope functions */
static void affect_modify(struct char_data *ch, byte loc, sbyte mod, long bitv, bool add);


char *fname(const char *namelist)
{
  static char holder[READ_SIZE];
  char *point;

  for (point = holder; isalpha(*namelist); namelist++, point++)
        *point = *namelist;

    *point = '\0';

    return (holder);
}


int isname(char *str, char *namelist)
{
    register char *curname, *curstr;

    curname = curstr = namelist;
    for(;;){

        if( !curname ) return(0);

        if( strncasecmp( str, curname, strlen( str )) == 0 )
            return( 1 );

        /* skip to next name */

        for (; isalpha(*curname); curname++);
        if( !*curname ) return(0);
        curname++;			/* first char of new name */
    }
}
void aff_apply_modify(struct char_data *ch, byte loc, sbyte mod, char *msg)
{

  switch (loc) {
  case APPLY_NONE:
    break;

  case APPLY_STR:
    GET_STR(ch) += mod;
    break;
  case APPLY_DEX:
    GET_DEX(ch) += mod;
    break;
  case APPLY_INT:
    GET_INT(ch) += mod;
    break;
  case APPLY_WIS:
    GET_WIS(ch) += mod;
    break;
  case APPLY_CON:
    GET_CON(ch) += mod;
    break;
  case APPLY_CHA:
    GET_CHA(ch) += mod;
    break;

  case APPLY_CLASS:
    /* ??? GET_CLASS(ch) += mod; */
    break;

  case APPLY_LEVEL:
    /* ??? GET_LEVEL(ch) += mod; */
    break;

  case APPLY_AGE:
    ch->player.time.birth -= (mod * SECS_PER_MUD_YEAR);
    break;

  case APPLY_CHAR_WEIGHT:
    GET_WEIGHT(ch) += mod;
    break;

  case APPLY_CHAR_HEIGHT:
    GET_HEIGHT(ch) += mod;
    break;

  case APPLY_MANA:
    GET_MAX_MANA(ch) += mod;
    break;

  case APPLY_HIT:
    GET_MAX_HIT(ch) += mod;
    break;

  case APPLY_MOVE:
    GET_MAX_MOVE(ch) += mod;
    break;

  case APPLY_GOLD:
    break;

  case APPLY_EXP:
    break;

  case APPLY_AC:
    GET_AC(ch) += mod;
    break;

  case APPLY_HITROLL:
    GET_HITROLL(ch) += mod;
    break;

  case APPLY_DAMROLL:
    if(GET_DAMROLL(ch) == ch->points.damroll)
        if (mod + (int)GET_DAMROLL(ch) > 127)
            mod = (int)GET_DAMROLL(ch) - 127;
    SET_DAMROLL(ch, GET_DAMROLL(ch) + mod);
    break;

  case APPLY_SAVING_PARA:
    GET_SAVE(ch, SAVING_PARA) += mod;
    break;

  case APPLY_SAVING_ROD:
    GET_SAVE(ch, SAVING_ROD) += mod;
    break;

  case APPLY_SAVING_PETRI:
    GET_SAVE(ch, SAVING_PETRI) += mod;
    break;

  case APPLY_SAVING_BREATH:
    GET_SAVE(ch, SAVING_BREATH) += mod;
    break;

  case APPLY_SAVING_SPELL:
    GET_SAVE(ch, SAVING_SPELL) += mod;
    break;

  case APPLY_POISON:
  case APPLY_PLAGUE:
  case APPLY_SPELL_COST:
  case APPLY_SPELL_SAVE:
  case APPLY_SPELL_DAMAGE:
  case APPLY_SPELL_DURATION:
  case APPLY_SKILL_SUCCESS:
  case APPLY_USELEVEL:
  case APPLY_SKILL_SUCCESS_DEPRECATED:
    break;

  default:
    mlog("SYSERR: Unknown apply adjust attempt (handler.c, %s).", msg);
    break;

  } /* switch */
}

static void affect_modify(struct char_data * ch, byte loc, sbyte mod, long bitv, bool add)
{
  if (add) {
    SET_BIT_AR(AFF_FLAGS(ch), bitv);
  } else {
    REMOVE_BIT_AR(AFF_FLAGS(ch), bitv);
    mod = -mod;
  }

  aff_apply_modify(ch, loc, mod, "affect_modify");
}

void affect_modify_ar(struct char_data * ch, byte loc, sbyte mod, int bitv[], bool add)
{
  int i , j;

  if (add) {
    for(i = 0; i < AF_ARRAY_MAX; i++)
      for(j = 0; j < 32; j++)
        if(IS_SET_AR(bitv, (i*32)+j))
          SET_BIT_AR(AFF_FLAGS(ch), (i*32)+j);
  } else {
    for(i = 0; i < AF_ARRAY_MAX; i++)
      for(j = 0; j < 32; j++)
        if(IS_SET_AR(bitv, (i*32)+j))
          REMOVE_BIT_AR(AFF_FLAGS(ch), (i*32)+j);
    mod = -mod;
  }

  aff_apply_modify(ch, loc, mod, "affect_modify_ar");
}



/* This updates a character by subtracting everything he is affected by */
/* restoring original abilities, and then affecting all again           */
void affect_total(CharData * ch)
{
  struct affected_type *af;
  int i, j;

  for (i = 0; i < NUM_WEARS; i++) {
    if (GET_EQ(ch, i))
      for (j = 0; j < MAX_OBJ_AFFECT; j++){
        if( GET_EQ(ch, i)->affected[j].location >= NUM_APPLIES){
            mlog("SYSERR: Unknown apply %s affect %d.",
                     GET_NAME(ch), GET_EQ(ch, i)->affected[j].location );
        }
        else {
	    affect_modify_ar(ch, GET_EQ(ch, i)->affected[j].location,
		      GET_EQ(ch, i)->affected[j].modifier,
		      GET_OBJ_AFFECT(GET_EQ(ch, i)), FALSE);
        }
      }
  }

  for (af = ch->affected; af; af = af->next)
    affect_modify(ch, af->location, af->modifier, af->bitvector, FALSE);

  // This resets the character to its natural abilities.
  ch->aff_abils = ch->real_abils;

  for (i = 0; i < NUM_WEARS; i++) {
    if (GET_EQ(ch, i))
      for (j = 0; j < MAX_OBJ_AFFECT; j++)
	affect_modify_ar(ch, GET_EQ(ch, i)->affected[j].location,
		      GET_EQ(ch, i)->affected[j].modifier,
		      GET_OBJ_AFFECT(GET_EQ(ch, i)), TRUE);
  }

  for (af = ch->affected; af; af = af->next)
    affect_modify(ch, af->location, af->modifier, af->bitvector, TRUE);

  /* Make certain values are between 0..25, not < 0 and not > 25! */
  i = (IS_NPC(ch) || GET_LEVEL(ch) >= LVL_GRGOD) ? 25 : 18;
  

  GET_DEX(ch) = calcMaxStat(ch, DEXTERITY_INDEX);
  GET_INT(ch) = calcMaxStat(ch, INTELLIGENCE_INDEX);
  GET_WIS(ch) = calcMaxStat(ch, WISDOM_INDEX);
  GET_CON(ch) = calcMaxStat(ch, CONSTITUTION_INDEX);
  GET_CHA(ch) = calcMaxStat(ch, CHARISMA_INDEX);

  if(GET_STR(ch) < 0) 
    GET_STR(ch) = 0;
  
  if (IS_NPC(ch)) {
    GET_STR(ch) = MIN(GET_STR(ch), i);
  } else {
    if (GET_STR(ch) > 18) {
      i = GET_ADD(ch) + ((GET_STR(ch) - 18) * 10);
      GET_ADD(ch) = MIN(i, race_stat_limits[(int)GET_RACE(ch)][STRENGTH_ADD_INDEX]);
      if (i <= 100)
        GET_STR(ch) = MIN(18, race_stat_limits[(int)GET_RACE(ch)][STRENGTH_INDEX]);
      else {
        i = ((i - 100)/10);
        GET_STR(ch) = MIN(18 + i, race_stat_limits[(int)GET_RACE(ch)][STRENGTH_INDEX]);
      }
    }
  }
  
  GET_STR(ch) = calcMaxStat(ch, STRENGTH_INDEX);

}

int calcMaxStat(CharData *ch, int stat) {
    int val = 0, maxVal = 0;
    
    switch(stat) {
        case STRENGTH_INDEX:
            val = MAX(0, GET_STR(ch));
            maxVal = race_stat_limits[(int)GET_RACE(ch)][STRENGTH_INDEX];
            break;
        case INTELLIGENCE_INDEX:
            val = MAX(0, GET_INT(ch));
            maxVal = race_stat_limits[(int)GET_RACE(ch)][INTELLIGENCE_INDEX];
            break;
        case WISDOM_INDEX:
            val = MAX(0, GET_WIS(ch));
            maxVal = race_stat_limits[(int)GET_RACE(ch)][WISDOM_INDEX];
            break;
        case DEXTERITY_INDEX:
            val = MAX(0, GET_DEX(ch));
            maxVal = race_stat_limits[(int)GET_RACE(ch)][DEXTERITY_INDEX];
            if(affected_by_spell(ch, SPELL_FLEET_FOOT))
                maxVal += 1;
            break;
        case CONSTITUTION_INDEX:
            val = MAX(0, GET_CON(ch));
            maxVal = race_stat_limits[(int)GET_RACE(ch)][CONSTITUTION_INDEX];
            break;
        case CHARISMA_INDEX:
            val = MAX(0, GET_CHA(ch));
            maxVal = race_stat_limits[(int)GET_RACE(ch)][CHARISMA_INDEX];
            break;
        default:
            // TODO: Output mudlog error message
            break;
    }
    
    return (val <= maxVal) ? val : maxVal;    
}

/* Insert an affect_type in a char_data structure
   Automatically sets apropriate bits and apply's */
void affect_to_char(CharData * ch, AffectedType * af)
{
  struct affected_type *affected_alloc;

  CREATE(affected_alloc, struct affected_type, 1);

  *affected_alloc = *af;
  affected_alloc->next = ch->affected;
  ch->affected = affected_alloc;

  affect_modify(ch, af->location, af->modifier, af->bitvector, TRUE);

  if(af->location == APPLY_HIT)
      GET_HIT(ch) += af->modifier;
  if(af->location == APPLY_MANA)
      GET_MANA(ch) += af->modifier;

  affect_total(ch);
}



/*
 * Remove an affected_type structure from a char (called when duration
 * reaches zero). Pointer *af must never be NIL!  Frees mem and calls
 * affect_location_apply
 */
void affect_remove(CharData * ch, struct affected_type * af, int save)
{
    struct affected_type *temp;
    int berserk_remove = 0;
    int fortify_remove = 0;
    int spell = FALSE;

    if( !ch->affected ) {
        /* OLD WAY: assert(ch->affected); */
        mudlog(NRM, LVL_IMMORT, TRUE, "SYSERR: affect_remove called with no affects for %s",
                GET_NAME(ch));
        return;
    }

    /* -2 duration means we have been called from char_to_store in db.c */
    /* This is a hack in every sense of the word :p Vex. */
    if ((af->bitvector == AFF_BERSERK) && !(af->duration == -2)) {
        berserk_remove = 1;
    }
    if (af->bitvector == AFF_FORTIFY && af->duration != -2)
        fortify_remove = 1;

    if(af->location == APPLY_HIT)
        //GET_HIT(ch) = MAX(1, GET_HIT(ch) - af->modifier);
        GET_HIT(ch) = GET_HIT(ch) - af->modifier;
    if(af->location == APPLY_MANA)
        GET_MANA(ch) = MAX(0, GET_MANA(ch) - af->modifier);

    /* Ok if we are not about to rent or doing a save then do this other
     wise we can crash here *shudder* */

    if (save == FALSE)  {
        if (af->bitvector == AFF_FLY && ROOM_FLAGGED(IN_ROOM(ch), ROOM_FALL))
            spell = SPELL_FLY;
        if (af->type == SPELL_CURE_CRITIC)
            spell = SPELL_CURE_CRITIC;
    }
    
    /* If we're not saving/renting, a charm corpse running will kill the mob */
    if (save == FALSE && af->type == SPELL_CHARM_CORPSE) spell = af->type;

    affect_modify(ch, af->location, af->modifier, af->bitvector, FALSE);
    if (berserk_remove) {
        sendChar(ch, "You don't feel so angry now.\r\n");
        if (GET_HIT(ch) <= 0) {
            if (GET_HIT(ch) < -9) GET_HIT(ch) = -9;
            sendChar(ch, "You are overcome by your wounds!\r\n");
            update_pos(ch);
            switch (GET_POS(ch)) {
                case POS_MORTALLYW:
                    act("$n is mortally wounded, and will die soon, if not aided.", TRUE, ch, 0, 0, TO_ROOM);
                    send_to_char("You are mortally wounded, and will die soon, if not aided.\r\n", ch);
                    break;
                case POS_INCAP:
                    act("$n is incapacitated and will slowly die, if not aided.", TRUE, ch, 0, 0, TO_ROOM);
                    send_to_char("You are incapacitated an will slowly die, if not aided.\r\n", ch);
                    break;
                case POS_STUNNED:
                    act("$n is stunned, but will probably regain consciousness again.", TRUE, ch, 0, 0, TO_ROOM);
                    send_to_char("You're stunned, but will probably regain consciousness again.\r\n", ch);
                    break;
                default:
                    /* Ack! This should'nt happen. */
                    mudlog(NRM, LVL_IMMORT, TRUE, "SYSERR: Invalid updated position for %s in affect_remove(result from BERSERK)", GET_NAME(ch));
                    break;
                }
        }
    }

    /* check to see if a player tried to make a mob's affect vanish by
    * casting the spell for that affect */
    if (IS_NPC(ch) && ch->nr != -1 && af->bitvector != AFF_DONTSET &&
            IS_SET_AR(AFF_FLAGS(mob_proto + ch->nr), af->bitvector))
        SET_BIT_AR(AFF_FLAGS(ch), af->bitvector);

    if( spell == SPELL_CURE_CRITIC && save == FALSE) {
        // We heal more for longer duration spells.  This information is encapsulated in the spell level over 45.
        GET_HIT(ch) += (dice(8, 8)+number(GET_LEVEL(ch), GET_LEVEL(ch)*2))*spell_level(ch, SPELL_CURE_CRITIC)/45;
        GET_HIT(ch) = MIN(GET_HIT(ch), GET_MAX_HIT(ch));
    }

    REMOVE_FROM_LIST(af, ch->affected, next);
    free(af);
    affect_total(ch);

    if( spell == SPELL_FLY && IS_NOT_FLYING(ch) && save == FALSE) {
        doFallRoom(ch);
    }
    
    if (fortify_remove) {
        if (ch->fortifier) ch->fortifier->fortifiee = NULL;
        ch->fortifier = NULL;
    }

    if(spell == SPELL_CHARM_CORPSE && IS_NPC(ch) && save == FALSE) {
        act("The life fades from $n as the foul magic around him unravels.",
                FALSE, ch, 0, 0, TO_ROOM);
        make_corpse(ch);
        extract_char(ch);
    }
}


#define DOWN  5	/* Direction 5 is down */

#define SPINE_MSG "You land with such force you have broken your spine.\r\n"
#define SPINE_ACT "$n lands with a sickening *crunch* and does not move."
#define BREAK_MSG "You slam to the ground breaking many of your bones.\r\n"
#define BREAK_ACT "$n almost lands on top of you as $e falls from above."

void
doFallRoom( CharData* ch )
{
  ObjData* obj;
  int fall_room, was_in_room, fall_loop = 0;
  int number_of_rooms = 0, damage = 0;
  CharData *rider = ch->rider;

  // Ignore NULL char pointers.
  //
  if( ch == NULL ){ return; }

  // Mounted people don't fall unless their mount does!
  if (ch->mount) return;

  // Imms are impervious - maybe not so in Arena?
  //
  if( IS_IMMORTAL(ch) ) return;

  // If this is not a fall room then get out now.
  //
  if( !ROOM_FLAGGED(IN_ROOM(ch), ROOM_FALL)) return;

  // If there is no way down then bail.
  //
  if( !CAN_GO( ch, DOWN )) return;

  // If the character is flying then the rest is moot.
  //
  if( IS_FLYING(ch) ) return;

  // If the character is Draconian or Faerie and they're not wearing anything
  // about their body then they will automatically fly.
  if( IS_DRACONIAN(ch) && !ch->equipment[WEAR_ABOUT] ) return;
  if( IS_FAERIE(ch)&& !ch->equipment[WEAR_ABOUT] ) return;

  // If the room dictates a boat, and the char has one, exit.
  //
  if( SECT(IN_ROOM(ch)) == SECT_WATER_NOSWIM)
  {
    for( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
      if( GET_OBJ_TYPE(obj) == ITEM_BOAT ) return;
    }
    
    if ((GET_SUBRACE(ch) == WATER_ELEMENTAL) || IS_AMARA(ch)) return;
  }  

  fall_room = real_room(GET_ROOM_VNUM(EXIT(ch, DOWN)->to_room));
  if( fall_room < 0 )
  {
    mudlog(NRM, LVL_IMMORT, TRUE, "SYSERR: Fall room exit is whacked in room %d", IN_ROOM(ch) );
    return;
  }

  /* Uhoh, your rider just fell off! */
  if (ch->rider) {
      awkward_dismount(rider = ch->rider);
  }

  if( EXIT(ch, DOWN) )
  {
    bool fallAgain = FALSE;

    if( GET_ROOM_VNUM(EXIT(ch, DOWN)->to_room > 0))
    {
      was_in_room = ch->in_room;
      ch->in_room = real_room(GET_ROOM_VNUM(EXIT(ch, DOWN)->to_room));
      if (CAN_GO(ch, DOWN) && ROOM_FLAGGED(IN_ROOM(ch), ROOM_FALL))
      fallAgain = TRUE;


      ch->in_room = was_in_room;
    }

    sendChar(ch, "You try and flap your arms as you start to fall!\r\n");
    act( "$n tries to flap $s arms as $e starts to fall.",
                                FALSE, ch, 0, 0, TO_ROOM);

    number_of_rooms = 1;
    char_from_room(ch);
    char_to_room(ch, fall_room);
    look_at_room(ch, 0);

    if( fallAgain )
    {
      for( fall_loop = 0; fallAgain != FALSE; fall_loop++ )
      {
        fall_room = real_room(world[EXIT(ch, DOWN)->to_room].number);

        sendChar( ch, "\r\nYou fall faster and faster.\r\n\r\n" );
        act("$n falls quickly past you.", FALSE, ch, 0, 0, TO_ROOM);

        number_of_rooms++;
        char_from_room(ch);               
        char_to_room(ch, fall_room); 
        look_at_room(ch, 0); 

        fallAgain = (CAN_GO(ch, DOWN) && ROOM_FLAGGED(IN_ROOM(ch), ROOM_FALL));

        if( UNDERWATER(ch) )
        {
          sendChar(ch, "You land in the water creating a huge splash.\r\n");
          act( "$n plunges into the depths with a loud splash.",
               FALSE, ch, 0, 0, TO_ROOM);
        }
        else
        {
          damage = number(10, 25) * number_of_rooms; 
          if( GET_HIT(ch) < damage )
          {
            GET_HIT(ch) = number(-10, -2);
            update_pos(ch);
            sendChar( ch, SPINE_MSG );
            act( SPINE_ACT, FALSE, ch, 0, 0, TO_ROOM);
            if( IS_SET_AR( ROOM_FLAGS(ch->in_room), ROOM_DEATH )) 
             {
              deathTrapKill(ch);
             }
          }
          else
          {
            GET_HIT(ch) -= damage;
            sendChar( ch, BREAK_MSG );
            act( BREAK_ACT, FALSE, ch, 0, 0, TO_ROOM);
            GET_POS(ch) = POS_SITTING;
            update_pos(ch);
            if( IS_SET_AR( ROOM_FLAGS(ch->in_room), ROOM_DEATH )) 
             {
              deathTrapKill(ch);
             }
            }
        }
      }
    }
  }

  else
  {
    char_from_room(ch);
    char_to_room(ch, fall_room);
    damage = number(10, 25);

    if( !UNDERWATER(ch) )
    {
      if( GET_HIT(ch) < damage )
      {
        GET_HIT(ch) = number(-10, -2);
        GET_POS(ch) = POS_SITTING;
        update_pos(ch);
        sendChar( ch, SPINE_MSG );
        act( SPINE_ACT, FALSE, ch, 0, 0, TO_ROOM);
           if( IS_SET_AR( ROOM_FLAGS(ch->in_room), ROOM_DEATH )) 
             {
              deathTrapKill(ch);
             }
      }
      else
      {
        GET_HIT(ch) -= damage;
        sendChar( ch, BREAK_MSG );
        act( BREAK_ACT, FALSE, ch, 0, 0, TO_ROOM);
        GET_POS(ch) = POS_SITTING;
        update_pos(ch);
            if( IS_SET_AR( ROOM_FLAGS(ch->in_room), ROOM_DEATH )) 
             {
              deathTrapKill(ch);
             }
      }
    }

    else
    {
      sendChar( ch, "You land in the water creating a huge splash.\r\n");
      act( "$n plunges into the depths with a loud splash.",
            FALSE, ch, 0, 0, TO_ROOM);
    }

    if (ch->in_room != -1) look_at_room(ch, 0);
    
  }

  if (rider) doFallRoom(rider);
}



/*
** Call affect_remove with every spell of spelltype "skill"
*/
/* Mort : Added in FALSE to affect_remove, this is for fall rooms.  */
void affect_from_char( CharData *ch, int skill )
{
  struct affected_type *hjp, *next;

  for (hjp = ch->affected; hjp; hjp = next) {
    next = hjp->next;
    if (hjp->type == skill) {
        affect_remove(ch, hjp, FALSE);
        break;
    }
  }
}



/* Return if a char is affected by a spell (SPELL_XXX), NULL indicates
   not affected  */
bool affected_by_spell(CharData * ch, int skill)
{
  struct affected_type *hjp;

  for (hjp = ch->affected; hjp; hjp = hjp->next)
    if (hjp->type == skill)
      return (hjp->modifier ? hjp->modifier : TRUE);

  return (FALSE);
}

/* return the level of the spell affecting ch, or -1 if no such spell */
int spell_level(CharData *ch, int spell)
{
  struct affected_type *hjp;
  int level = -1;

  for (hjp = ch->affected; hjp; hjp = hjp->next)
    if (hjp->type == spell) level = MAX(level, hjp->level);

  return level;
}

/* return the duration of the spell affecting ch, or -2 if no such spell */
int spell_duration(CharData *ch, int spell)
{
  struct affected_type *hjp;
  int duration = -2;

  for (hjp = ch->affected; hjp; hjp = hjp->next)
    if (hjp->type == spell) duration = MAX(duration, hjp->duration);

  return duration;
}

/* return the level of affect the character has or -1 if not affected */
int aff_level(CharData *ch, int skill)
{
  struct affected_type *hjp;
  int level = -LVL_IMMORT;

  // check racial abilities
  if (has_native_aff(ch, skill)) level = GET_LEVEL(ch);
	  

  // don't bother checking any further if they don't even have the affect
  if (!AFF_FLAGGED(ch, skill)) return level;

  // check spell affects
  for (hjp = ch->affected; hjp; hjp = hjp->next)
    if (hjp->bitvector ==  skill) level = MAX(level, hjp->level);

#ifdef DO_USELEVEL_AFFECT_LEVEL
  // check eq affects
  for (i = 0; i < NUM_WEARS; i++) {
    if (ch->equipment[i])
      if (IS_SET_AR(ch->equipment[o].obj_flags.bitvector, skill)) {
        l = GET_LEVEL(ch);
        for (j = 0; j < MAX_OBJ_AFFECT; j++)
          if( ch->equipment[i]->affected[j].location == APPLY_USELEVEL)
            l = ch->equipment[i]->affected[j].modifier;
        level = MAX(level, l);
      }
  }
#else
  if (level < 0) level = GET_LEVEL(ch);
#endif

  /* mobs will have the spell at a minimum of their own level */
  if (IS_NPC(ch)) level = MAX(level, GET_LEVEL(ch));

  /* for certain affects, SKILL_ENHANCED_STEALTH boosts the level */
  if (!IS_NPC(ch)) switch (skill) {
      case AFF_INVISIBLE:
      case AFF_HIDE:
      case AFF_SHADOW_SPHERE:
          level += GET_SKILL(ch, SKILL_ENHANCED_STEALTH)/6;
          break;
      default:
          break;
  }

  return level;
}

/* Mort : added FALSE to affect_remove, this is for fall rooms */
void affect_join(CharData * ch, struct affected_type * af,
		      bool add_dur, bool avg_dur, bool add_mod, bool avg_mod)
{
  struct affected_type *hjp;
  bool found = FALSE;

  for (hjp = ch->affected; !found && hjp; hjp = hjp->next) {
    if (hjp->type == af->type) {

      if (add_dur)
		  af->duration += hjp->duration;
      if (avg_dur)
		  af->duration >>= 1;

      if (add_mod)
		  af->modifier += hjp->modifier;
      if (avg_mod)
		  af->modifier >>= 1;

      /* hackity, not more than +10dr for adrenaline surges! */
      if (af->type == SKILL_ADRENALINE) af->modifier = MIN(af->modifier, 10);

      if(add_dur || avg_dur || add_mod || avg_mod)
          affect_remove(ch, hjp, FALSE);

      affect_to_char(ch, af);
      found = TRUE;
      break;
    }
  }
  if (!found)
    affect_to_char(ch, af);
}


/* move a player out of a room */
void char_from_room(CharData * ch)
{
  CharData *temp;

  if (ch == NULL) {
    mlog("SYSERR: NULL character in %s, char_from_room", __FILE__);
    exit(1);
  }

  if(IN_ROOM(ch) == NOWHERE) {
    mlog("SYSERR: Character is NOWHERE in %s, char_from_room", __FILE__);
    exit(1);
  }


  /* make sure that they don't stay fishin' as they move ... */
  if (PLR_FLAGGED(ch, PLR_FISHING)) {
      REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_FISHING);
      REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_FISH_ON);
  }

  if (GET_EQ(ch, WEAR_LIGHT) != NULL)
    if (GET_OBJ_TYPE(GET_EQ(ch, WEAR_LIGHT)) == ITEM_LIGHT)
      if (GET_OBJ_VAL(GET_EQ(ch, WEAR_LIGHT), 2))	/* Light is ON */
	world[IN_ROOM(ch)].light--;

  REMOVE_FROM_LIST(ch, world[IN_ROOM(ch)].people, next_in_room);
  IN_ROOM(ch) = NOWHERE;
  ch->next_in_room = NULL;
}


/*
** place a character in a room
*/
void
char_to_room( CharData *ch, int room )
{
  if (ch == NULL || room == NOWHERE || room > top_of_world)
    mlog("SYSERR: Illegal value(s) passed to char_to_room. (Room: %d/%d Ch: %p",
		room, top_of_world, ch);
  else {
  ch->next_in_room = world[room].people;
  world[room].people = ch;
  ch->in_room = room;
  
  if (ch->equipment[WEAR_LIGHT])
    if (GET_OBJ_TYPE(ch->equipment[WEAR_LIGHT]) == ITEM_LIGHT)
      if (GET_OBJ_VAL(ch->equipment[WEAR_LIGHT], 2))	/* Light ON */
        world[room].light++;
  }
}


/* give an object to a char   */
void obj_to_char(ObjData * object, CharData * ch)
{
    if (!object) {
	mudlog(NRM, LVL_IMMORT, TRUE, "SYSERR: NULL object passed to obj_to_char!");
	return;
    }
    if (!ch) {
	mudlog(NRM, LVL_IMMORT, TRUE, "SYSERR: NULL ch passed to obj_to_char!");
	return;
    }
    object->next_content = ch->carrying;
    /* ^^^^ The line above caused a crash. *CRASH* Vex. */
    ch->carrying = object;
    object->carried_by = ch;
    object->in_room = NOWHERE;
    object->worn_at = NOWHERE;
    GET_OBJ_WEIGHT(object) = (GET_OBJ_WEIGHT(object) < 0 ? 1 : GET_OBJ_WEIGHT(object) );
    IS_CARRYING_W(ch) += GET_OBJ_WEIGHT(object);
    IS_CARRYING_N(ch)++;

    /* Mortis : Don't set this if its a mob it sets the STAY_ZONE FLAG
                set flag for crash-save system */
    if (!IS_NPC(ch))
	SET_BIT_AR(PLR_FLAGS(ch), PLR_CRASH);
}


/*
** take an object from a char
*/
void
obj_from_char( ObjData *object )
{
  ObjData *temp;

  if (object == NULL) {
    mlog("SYSERR: NULL object passed to obj_from_char.");
    return;
  }
  REMOVE_FROM_LIST(object, object->carried_by->carrying, next_content);

  /* set flag for crash-save system, but not on mobs! */
  if (!IS_NPC(object->carried_by))
  SET_BIT_AR(PLR_FLAGS(object->carried_by), PLR_CRASH);

  IS_CARRYING_W(object->carried_by) -= GET_OBJ_WEIGHT(object);
  IS_CARRYING_N(object->carried_by)--;
  object->carried_by = NULL;
  object->next_content = NULL;
}

/* Return the effect of a piece of armor in position eq_pos */
/* I've substantially modified this function. This is what it is doing:
** First it gets the ac from an item, multiplies it by 8.
** This value is then modified by the location the armor is worn on.
** It is then divided by 8. For NPCs, this is then the final value.
** For pcs, the remainder is kept and used in their player_specials
** structure, so even an item that is ac apply 1 on a location that is
** divided by 8 will count for SOMETHING.
** The new parameter add is needed to properly deal with pc remainders.
** If add is true, then we are equiping the item, otherwise we are taking
** it off.
*/
int
apply_ac( CharData *ch,
          int eq_pos,
          bool add )
{
# define AC_REMAIN_FACTOR 3 /* Highest bit shift */
  int ac;

  if( !ch->equipment[eq_pos] )
  {
    mudlog(NRM, LVL_IMMORT, TRUE, "apply_ac in handler.c unk eq pos %d, %s",
              eq_pos, GET_NAME(ch));
    return(0);
  }

  if (!(GET_OBJ_TYPE(GET_EQ(ch, eq_pos)) == ITEM_ARMOR))
    return (0);

  ac = GET_OBJ_VAL( ch->equipment[eq_pos], 0 ) << AC_REMAIN_FACTOR;

  switch( eq_pos )
  {
  case WEAR_BODY:     /* triple */
    ac *= 3;
    break;
  case WEAR_SHIELD:   /* double */
  case WEAR_HEAD:
    ac *= 2;
    break;
  case WEAR_ABOUT:    /* one and a half times */                               
    ac += ac >> 1;
    break;
  case WEAR_ARMS:	/* Unmodified */
  case WEAR_LEGS:
  case WEAR_CLOAK:
    break;
  case WEAR_HANDS:   /* one half */
  case WEAR_FEET:
  case WEAR_WAIST:
  case WEAR_WRIST_R:
  case WEAR_WRIST_L:
  case WEAR_FINGER_R:
  case WEAR_FINGER_L:
  case WEAR_NECK:
  case WEAR_FACE:
  case WEAR_ANKLES:
    ac = ac >> 1;
    break;
  case WEAR_WIELD:
  case WEAR_LIGHT:
  case WEAR_HOLD:
  case WEAR_ORBIT:
  case WEAR_EARS:
    ac = 0;
    break;
  default:
    mudlog(NRM, LVL_IMMORT, TRUE, "handler.c: unknown eq position %d passed to apply_ac", eq_pos);
    ac = 0;
    break;
  }

  if( !IS_NPC(ch) )
  {
    if( add )
    {
      ac += ch->player_specials->saved.ac_remain;
    }
    else
    {
      ac -= ch->player_specials->saved.ac_remain;
    }
    ch->player_specials->saved.ac_remain =
      ac - ((ac >> AC_REMAIN_FACTOR) << AC_REMAIN_FACTOR);
    if (!add)
      ch->player_specials->saved.ac_remain =
        - ch->player_specials->saved.ac_remain;
  }

  ac = ac >> AC_REMAIN_FACTOR;

  return ac;
}



void
equip_char( CharData *ch,
            ObjData  *obj,
            int       pos )
{
  int j;
  int modifier; // Used when checking Level restricted EQ (APPLY_USELEVEL)

  if( pos < 0 || pos >= NUM_WEARS )
  {
    mudlog(NRM, LVL_IMMORT, TRUE, "handler.c illegal eq pos %d, %s", pos, GET_NAME(ch));
    return;
  }

  if( ch->equipment[pos] )
  {
    mudlog(NRM, LVL_IMMORT, TRUE, "SYSERR: Char is already equipped: %s, %s",
              GET_NAME(ch), obj->short_description);
    obj_to_char(obj,ch);
    return;
  }

  if( obj->carried_by )
  {
    mlog("SYSERR: EQUIP: Obj is carried_by when equip.");
    return;
  }

  if( obj->in_room != NOWHERE )
  {
    mlog("SYSERR: EQUIP: Obj is in_room when equip.");
    return;
  }

  modifier=0;
  if (!IS_NPC(ch)) { /* Mobs don't care about level restrictions on EQ
                      * Might have to change this to restrict charmies also */
      for (j=0; j<MAX_OBJ_AFFECT; j++)
          if ((obj->affected[j].location == APPLY_USELEVEL) && (modifier == 0) )
              modifier=obj->affected[j].modifier;
      if ((modifier<0) && (GET_LEVEL(ch)>(-modifier))) { // Char too high level
          act("You are too powerful for $p and lets go of it.", FALSE, ch, obj, 0, TO_CHAR);
          act("$n flinches and lets go of $p.", FALSE, ch, obj, 0, TO_ROOM);
          obj_to_char(obj, ch);
          return;
      } else if ((modifier>0) && (GET_LEVEL(ch)<modifier)) { // Char is too low level
          act("You are not powerful enough for $p and let go of it.", FALSE, ch, obj, 0, TO_CHAR);
          act("$n flinches and lets go of $p.", FALSE, ch, obj, 0, TO_ROOM);
          obj_to_char(obj, ch);
          return;
      }
  }
  if( !IS_NPC(ch) &&    /* Lets not have mobs zapped by their own stuff. */
     ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch)) ||
      invalid_race(ch, obj) ||
      invalid_class(ch, obj))) {
      act("You are zapped by $p and instantly let go of it.", FALSE, ch, obj, 0, TO_CHAR);
      act("$n is zapped by $p and instantly lets go of it.", FALSE, ch, obj, 0, TO_ROOM);
      obj_to_char(obj, ch);	/* changed to drop in inventory instead of ground */
      return;
  }

  if(has_owner(obj)) {
      if(!owns_item(ch, obj)) {
          send_to_char ("This equipment does not belong to you.\r\n", ch);
          obj_to_char(obj, ch);
          return;
      }
  }

  if(!IS_NPC(ch) && (obj->item_number > -1) && obj_index[obj->item_number].combatSpec != NULL || pos == WEAR_ORBIT) {
      SET_BIT_AR(GET_OBJ_EXTRA(obj), ITEM_SOULBOUND);
  }

  ch->equipment[pos] = obj;
  obj->worn_by       = ch;
  obj->worn_at       = pos;

  if( GET_OBJ_TYPE(obj) == ITEM_ARMOR )
    GET_AC(ch) -= apply_ac(ch, pos, TRUE);

  if( ch && ch->in_room > 0 )
  {
    if( pos == WEAR_LIGHT && GET_OBJ_TYPE(obj) == ITEM_LIGHT )
    {
      if (GET_OBJ_VAL(obj, 2))	/* if light is ON */
      {
	if(( ch->in_room > top_of_world) || (ch->in_room < 0 ))
	    mudlog(NRM, LVL_IMMORT, TRUE, "SYSERR: Light bug: Character %s in invalid room %d.", GET_NAME(ch), ch->in_room);
	else
	    world[ch->in_room].light++;
      }
    }
  }

  for( j = 0; j < MAX_OBJ_AFFECT; j++ )
//128    affect_modify(ch, obj->affected[j].location,
       affect_modify_ar(ch, obj->affected[j].location,
		  obj->affected[j].modifier,
		  obj->obj_flags.bitvector, TRUE);
  affect_total(ch);
}



ObjData *unequip_char(CharData * ch, int pos)
{
  int j;
  ObjData *obj;

  if( pos < 0 || pos >= NUM_WEARS )
  {
    /* OLD WAY: assert(pos >= 0 && pos < NUM_WEARS); */
    mudlog(NRM, LVL_IMMORT, TRUE, "SYSERR: unequip_char illegal pos %d for %s", pos, GET_NAME(ch));
    return( NULL );
  }

  if( ch == NULL )
  {
    mudlog(NRM, LVL_IMMORT, TRUE, "SYSERR: unequip_char was passed a NULL char pointer" );
    return( NULL );
  }

  if( !ch->equipment[pos] )
  {
    /* OLD WAY: assert(ch->equipment[pos]); */
    mudlog(NRM, LVL_IMMORT, TRUE, "SYSERR: unequip_char nothing in slot %d on %s", pos, GET_NAME(ch));
    return( NULL );
  }

  obj          = ch->equipment[pos];
  obj->worn_by = NULL;
  obj->worn_at = NOWHERE;

  if (GET_OBJ_TYPE(obj) == ITEM_ARMOR)
    GET_AC(ch) += apply_ac(ch, pos, FALSE);

  /* If you remove your fishing rod, you can't fish so well */
  if (pos == WEAR_HOLD && PLR_FLAGGED(ch, PLR_FISHING)) {
      REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_FISHING);
      REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_FISH_ON);
  }

  if (ch->in_room != NOWHERE) {
    if (pos == WEAR_LIGHT && GET_OBJ_TYPE(obj) == ITEM_LIGHT)
      if (GET_OBJ_VAL(obj, 2))	/* if light is ON */
	world[ch->in_room].light--;
  }

  ch->equipment[pos] = NULL;

  for (j = 0; j < MAX_OBJ_AFFECT; j++)
//128    affect_modify(ch, obj->affected[j].location,
       affect_modify_ar(ch, obj->affected[j].location,
		  obj->affected[j].modifier,
		  obj->obj_flags.bitvector, FALSE);

  affect_total(ch);

  return (obj);
}


int get_number(char **name)
{
  int i;
  char *ppos;
  char number[MAX_INPUT_LENGTH];

  *number = '\0';

  if ((ppos = strchr(*name, '.'))) {
    *ppos++ = '\0';
    strcpy(number, *name);
    strcpy(*name, ppos);

    for (i = 0; *(number + i); i++)
      if (!isdigit(*(number + i)))
	return 0;

    return (atoi(number));
  }
  return 1;
}



/* Search a given list for an object number, and return a ptr to that obj */
ObjData *get_obj_in_list_num(int num, ObjData * list)
{
  ObjData *i;

  for (i = list; i; i = i->next_content)
    if (GET_OBJ_RNUM(i) == num)
      return i;

  return NULL;
}



/* search the entire world for an object number, and return a pointer  */
ObjData *get_obj_num(int nr)
{
  ObjData *i;

  for (i = object_list; i; i = i->next)
    if (GET_OBJ_RNUM(i) == nr)
      return i;

  return NULL;
}


/* search a room for a char, and return a pointer if found..  */
CharData *get_char_room(char *name, int room)
{
  CharData *i;
  int j = 0, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp = tmpname;

  strcpy(tmp, name);
  if (!(number = get_number(&tmp)))
    return NULL;

  for (i = world[room].people; i && (j <= number); i = i->next_in_room)
    if (isname(tmp, i->player.name))
      if (++j == number)
	return i;

  return NULL;
}



/* search all over the world for a char num, and return a pointer if found */
CharData *get_char_num(int nr)
{
  CharData *i;

  for (i = character_list; i; i = i->next)
    if (GET_MOB_RNUM(i) == nr)
      return i;

  return NULL;
}



/* put an object in a room */
void
obj_to_room(ObjData * object, room_rnum room)
{
  if (!object || room == NOWHERE || room > top_of_world)
    mlog("SYSERR: Illegal value(s) passed to obj_to_room. (Room #%d/%d, obj %p)",
	room, top_of_world, object);
  else {

    /* Digger added exploding EQ here. */
    if( IS_OBJ_STAT( object, ITEM_EXPLODES )){
        act( "$p explodes with a blinding flash as it touches the floor!", FALSE, 0, object, 0, TO_ROOM );
        extract_obj(object);
        return;
    }

    object->next_content = world[room].contents;
    world[room].contents = object;
    IN_ROOM(object)      = room;
    object->carried_by   = NULL;
    object->worn_at      = NOWHERE;
    if (ROOM_FLAGGED(room, ROOM_HOUSE))
	SET_BIT_AR(ROOM_FLAGS(room), ROOM_HOUSE_CRASH);

    /* add any affects this obj gives that the room doesn't have */
    /* and update GET_OBJ_VAL(object, 3) to reflect what we set */
    if (GET_OBJ_TYPE(object) == ITEM_AFFECT) {
        GET_OBJ_VAL(object, 3) = 0;
        if (GET_OBJ_VAL(object, 0) &&

                !ROOM_FLAGGED(IN_ROOM(object), GET_OBJ_VAL(object, 0) - 1)) {
            SET_BIT_AR(ROOM_FLAGS(IN_ROOM(object)), GET_OBJ_VAL(object, 0) - 1);
            GET_OBJ_VAL(object, 3) |= 1;
        }
        if (GET_OBJ_VAL(object, 1) &&
                !ROOM_FLAGGED(IN_ROOM(object), GET_OBJ_VAL(object, 1) - 1)) {
            SET_BIT_AR(ROOM_FLAGS(IN_ROOM(object)), GET_OBJ_VAL(object, 1) - 1);
            GET_OBJ_VAL(object, 3) |= 2;
        }
        if (GET_OBJ_VAL(object, 2) &&
                !ROOM_FLAGGED(IN_ROOM(object), GET_OBJ_VAL(object, 2) - 1)) {
            SET_BIT_AR(ROOM_FLAGS(IN_ROOM(object)), GET_OBJ_VAL(object, 2) - 1);
            GET_OBJ_VAL(object, 3) |= 4;
        }
    }
  }
}

/* Take an object from a room */
void obj_from_room(ObjData * object)
{
    ObjData *temp;

  if (!object || IN_ROOM(object) == NOWHERE) {
    mlog("SYSERR: NULL object (%p) or obj not in a room (%d) passed to obj_from_room",
	object, IN_ROOM(object));
        return;
    }

      REMOVE_FROM_LIST(object, world[IN_ROOM(object)].contents, next_content);

  if (ROOM_FLAGGED(IN_ROOM(object), ROOM_HOUSE))
    SET_BIT_AR(ROOM_FLAGS(IN_ROOM(object)), ROOM_HOUSE_CRASH);

    /* remove any affects this object gives the room, that it shouldn't have */
    /* GET_OBJ_VAL(object, 3) should tell us which to unset */
    if (GET_OBJ_TYPE(object) == ITEM_AFFECT) {
        if (GET_OBJ_VAL(object, 3) & 1)
            REMOVE_BIT_AR(ROOM_FLAGS(IN_ROOM(object)),
                    GET_OBJ_VAL(object, 0) - 1);
        if (GET_OBJ_VAL(object, 3) & 2)
            REMOVE_BIT_AR(ROOM_FLAGS(IN_ROOM(object)),
                    GET_OBJ_VAL(object, 1) - 1);
        if (GET_OBJ_VAL(object, 3) & 4)
            REMOVE_BIT_AR(ROOM_FLAGS(IN_ROOM(object)),
                    GET_OBJ_VAL(object, 2) - 1);
    }

    IN_ROOM(object) = NOWHERE;
    object->next_content = NULL;
}

/* recursively add "by" to this object's reference count */
static void obj_mod_count(ObjData *obj, int by)
{
    ObjData *c;
    obj_index[obj->item_number].number += by;
    for (c = obj->contains; c; c = c->next_content) {
        obj_mod_count(c, by);
    }
}

/* put an object in an object (quaint)  */
void obj_to_obj( ObjData * obj,
                 ObjData * obj_to )
{
    ObjData *tmp_obj;

    obj->next_content = obj_to->contains;
    obj_to->contains  = obj;
    obj->in_obj       = obj_to;
    obj->worn_at      = NOWHERE;

    for( tmp_obj = obj->in_obj; tmp_obj->in_obj; tmp_obj = tmp_obj->in_obj )
        GET_OBJ_WEIGHT(tmp_obj) += GET_OBJ_WEIGHT(obj);

    /* top level object.  Subtract weight from inventory if necessary. */
    GET_OBJ_WEIGHT(tmp_obj) += GET_OBJ_WEIGHT(obj);
    if (tmp_obj->carried_by)
        IS_CARRYING_W(tmp_obj->carried_by) += GET_OBJ_WEIGHT(obj);

    /* if the object is a locker, "take" it out of the mud */
    if (IS_SET(GET_OBJ_VAL(obj_to, 1), CONT_LOCKER)) {
        obj_mod_count(obj, -1);
        SET_BIT(GET_OBJ_VAL(obj_to, 1), CONT_DIRTY);
    }

}/* obj_to_obj */


/* remove an object from an object */
void obj_from_obj(ObjData * obj)
{
  ObjData *temp, *obj_from;

  if (obj->in_obj == NULL) {
    mlog("error (handler.c): trying to illegally extract obj from obj");
    return;
  }
  obj_from = obj->in_obj;
  REMOVE_FROM_LIST(obj, obj_from->contains, next_content);

  /* Subtract weight from containers container */
  for (temp = obj->in_obj; temp->in_obj; temp = temp->in_obj)
    GET_OBJ_WEIGHT(temp) -= GET_OBJ_WEIGHT(obj);

  /* Subtract weight from char that carries the object */
  GET_OBJ_WEIGHT(temp) -= GET_OBJ_WEIGHT(obj);
  if (temp->carried_by)
    IS_CARRYING_W(temp->carried_by) -= GET_OBJ_WEIGHT(obj);

  obj->in_obj = NULL;
  obj->next_content = NULL;

  /* if the container is a locker, "put" this item into the mud */
  if (IS_SET(GET_OBJ_VAL(obj_from, 1), CONT_LOCKER)) {
    obj_mod_count(obj, 1);
    SET_BIT(GET_OBJ_VAL(obj_from, 1), CONT_DIRTY);
  }
}


/* Set all carried_by to point to new owner */
void object_list_new_owner(ObjData * list, CharData * ch)
{
  if (list) {
    object_list_new_owner(list->contains, ch);
    object_list_new_owner(list->next_content, ch);
    list->carried_by = ch;
  }
}


/* Extract an object from the world */
void extract_obj(ObjData * obj)
{
  ObjData *temp;

  if (obj->in_room != NOWHERE)
    obj_from_room(obj);
  else if (obj->carried_by)
    obj_from_char(obj);
  else if (obj->in_obj)
    obj_from_obj(obj);

  /* Get rid of the contents of the object, as well. */
  while (obj->contains)
    extract_obj(obj->contains);

  /* just in case it's a corpse, remove it from the corpse list */
  if (GET_OBJ_TYPE(obj) == ITEM_CONTAINER) del_saved_corpse(obj);

  REMOVE_FROM_LIST(obj, object_list, next);

  if (IS_LOCKER(obj)) {
    LockerList *list = lockers;

    if (list->locker == obj) {
      lockers = lockers->next;
      free(list);
    } else {
      while (list->next && list->next->locker != obj) list = list->next;
      if (list->next) {
        LockerList *tofree = list->next;
        list->next = list->next->next;
        free(tofree);
      }
    }
  }

  if (GET_OBJ_RNUM(obj) >= 0)
    (obj_index[GET_OBJ_RNUM(obj)].number)--;

  if (SCRIPT(obj))
    extract_script(obj, OBJ_TRIGGER);

  free_obj(obj);
}



void update_object(ObjData *obj, int use)
{
  if (obj->contains)
    update_object(obj->contains, use);
  if (obj->next_content)
    update_object(obj->next_content, use);
}

void update_char_objects(CharData *ch)
{
  int i;

  if (ch->equipment[WEAR_LIGHT] != NULL)
    if (GET_OBJ_TYPE(ch->equipment[WEAR_LIGHT]) == ITEM_LIGHT)
      if (GET_OBJ_VAL(ch->equipment[WEAR_LIGHT], 2) > 0) {
	i = --GET_OBJ_VAL(ch->equipment[WEAR_LIGHT], 2);
	if (i == 1) {
	  act("Your light begins to flicker and fade.", FALSE, ch, 0, 0, TO_CHAR);
	  act("$n's light begins to flicker and fade.", FALSE, ch, 0, 0, TO_ROOM);
	} else if (i == 0) {
	  act("Your light sputters out and dies.", FALSE, ch, 0, 0, TO_CHAR);
	  act("$n's light sputters out and dies.", FALSE, ch, 0, 0, TO_ROOM);
	  world[ch->in_room].light--;
	}
      }

  for (i = 0; i < NUM_WEARS; i++)
    if (ch->equipment[i])
      update_object(ch->equipment[i], 2);

  if (ch->carrying)
    update_object(ch->carrying, 1);
}


static CharData *deletelist = NULL;

void delete_extracted(void)
{
  CharData *tmp, *next;

  for (tmp = deletelist; tmp; tmp = next) {
    next = tmp->next;
    free_char(tmp);
  }
  deletelist = NULL;
}

/*
** Extract a ch completely from the world, and leave his stuff behind
*/
void extract_char(CharData * ch)
{
  CharData *temp;
  DescriptorData *t_desc;
  ObjData *obj;
  int i;

  ACMD(do_return);

  if( !ch )
  {
    mudlog(NRM, LVL_IMMORT, TRUE, "SYSERR: extract_char: Can't extract a NULL ch!" );
    return;
  }

  if( ch->in_room == NOWHERE )
  {
    mudlog(NRM, LVL_IMMORT, TRUE, "SYSERR: NOWHERE extracting char %s. (handler.c, extract_char)", GET_NAME(ch));
    return;
  }

  if (SINGING(ch)) stop_singing(ch);


  if( !IS_NPC(ch) && !ch->desc )
  {
    for( t_desc = descriptor_list; t_desc; t_desc = t_desc->next )
      if( t_desc->original == ch )
	do_return( t_desc->character, "", 0, 0 );
  }

  /* On with the character's assets... */
  if (ch->followers || ch->master)
    die_follower(ch);
	  
  /* transfer objects to room, if any */
  while (ch->carrying) {
    obj = ch->carrying;
    obj_from_char(obj);
    obj_to_room(obj, IN_ROOM(ch));
  }

  /* transfer equipment to room, if any */
  for (i = 0; i < NUM_WEARS; i++)
    if (GET_EQ(ch, i))
      obj_to_room(unequip_char(ch, i), IN_ROOM(ch));

  end_fight(ch);

  char_from_room(ch);
  /* Unmount any Riders */
  if( ch->rider != NULL )
  {
    ch->rider->mount = NULL;
    ch->rider = NULL;
  }

  // Remove any fortification
  if (ch->fortifier != NULL) {
    affect_from_char(ch, SPELL_FORTIFY);
    ch->fortifier = NULL;
  }
  if (ch->fortifiee != NULL) {
    affect_from_char(ch->fortifiee, SPELL_FORTIFY);
    ch->fortifiee = NULL;
  }

  /* pull the char from the list */
  REMOVE_FROM_LIST(ch, character_list, next);

  if (ch->desc && ch->desc->original)
    do_return(ch, "", 0, 0);

  if( !IS_NPC(ch) )
  {
    save_char(ch, NOWHERE);
    deleteCrashFile(ch);
    flag_player_from_game(ch);
  }
  else
  {
    if (GET_MOB_RNUM(ch) > -1)		/* if mobile */
      mob_index[GET_MOB_RNUM(ch)].number--;
    clearMemory(ch);		/* Only NPC's can have memory */
    if (SCRIPT(ch))
      extract_script(ch, MOB_TRIGGER);
    if (SCRIPT_MEM(ch))
      extract_script_mem(SCRIPT_MEM(ch));
    // Freeing a char here will make *ch invalid, and this causes problems
    // in many places.  So, we instead add ch to a list of CharDatas pending
    // freeing, and take care of them each tick.
    ch->next = deletelist;
    deletelist = ch;
    //free_char(ch);
  }

  if( ch->desc )
  {
    setConnectState( ch->desc, CON_MENU );
    write_to_output(ch->desc, "%s", CONFIG_MENU);
  }
}



/* ***********************************************************************
   Here follows high-level versions of some earlier routines, ie functions
   which incorporate the actual player-data.
   *********************************************************************** */


CharData *get_player_vis(CharData * ch, char *name)
{
  CharData *i;

  for (i = character_list; i; i = i->next)
    if (!IS_NPC(i) && !str_cmp(i->player.name, name) && CAN_SEE(ch, i))
      return i;

  return NULL;
}


CharData *get_char_room_vis(CharData * ch, char *name)
{
  CharData *i;
  int j = 0, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp = tmpname;

  /* Digger - Maybe, maybe not */
  if(( name == NULL ) || ( *name == 0 )) return(0); 

  /* JE 7/18/94 :-) :-) */
  if (!str_cmp(name, "self") || !str_cmp(name, "me"))
    return ch;

    strcpy( tmp, name );
    if( !(number = get_number(&tmp)) )
#ifdef USE_ZERO_PC_INTERFACE
        /* 0.<name> means PC with name */
        return get_player_vis(ch, tmp);
#else
        number = 1;
#endif

  for (i = world[ch->in_room].people; i && j <= number; i = i->next_in_room)
    if (isname(tmp, i->player.name))
      if (CAN_SEE(ch, i))
	if (++j == number)
	  return i;

  return NULL;
}


CharData *get_char_vis(CharData * ch, char *name, int pc_only)
{
  CharData *i;
  DescriptorData *d;
  int j = 0, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp = tmpname;

  /* check the room first */
  if ((i = get_char_room_vis(ch, name)) != NULL)
    return i;

  /* Check player list next. Vex. */
  for (d = descriptor_list; d; d = d->next) {
    if (d->connected != CON_PLAYING)
      continue;
    // If its their EXACT name.
    if ( (strcasecmp(name, d->character->player.name) == 0) && CAN_SEE(ch, d->character) )
	return d->character;
  }

  strcpy(tmp, name);
  if (!(number = get_number(&tmp)))
    return NULL;

  for (i = character_list; i && (j <= number); i = i->next)
    if(( isname(tmp, i->player.name) && CAN_SEE(ch, i)) && (!IS_NPC(i) || !pc_only))
      if (++j == number)
	return i;

  return NULL;
}



ObjData *get_obj_in_list_vis(CharData * ch, char *name,
				              ObjData * list)
{
  ObjData *i;
  int j = 0, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp = tmpname;

  strcpy(tmp, name);
  if (!(number = get_number(&tmp)))
    return NULL;

  for (i = list; i && (j <= number); i = i->next_content)
    if (isname(tmp, i->name))
      if (CAN_SEE_OBJ(ch, i))
	if (++j == number)
	  return i;

  return NULL;
}




/* search the entire world for an object, and return a pointer  */
ObjData *get_obj_vis(CharData * ch, char *name)
{
  ObjData *i;
  int j = 0, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp = tmpname;

  /* scan items carried */
  if ((i = get_obj_in_list_vis(ch, name, ch->carrying)))
    return i;

  /* scan room */
  if ((i = get_obj_in_list_vis(ch, name, world[ch->in_room].contents)))
    return i;

  strcpy(tmp, name);
  if (!(number = get_number(&tmp)))
    return NULL;

  /* ok.. no luck yet. scan the entire obj list   */
  for (i = object_list; i && (j <= number); i = i->next)
    if (isname(tmp, i->name))
      if (CAN_SEE_OBJ(ch, i))
	if (++j == number)
	  return i;

  return NULL;
}



ObjData *get_object_in_equip_vis(CharData * ch,
		           char *arg, ObjData * equipment[], int *j)
{
  for ((*j) = 0; (*j) < NUM_WEARS; (*j)++)
    if (equipment[(*j)])
      if (CAN_SEE_OBJ(ch, equipment[(*j)]))
	if (isname(arg, equipment[(*j)]->name))
	  return (equipment[(*j)]);

  return NULL;
}


char *money_desc(int amount)
{
  static char buf[128];

  if (amount <= 0) {
    mlog("SYSERR: Try to create negative or 0 money.");
    return NULL;
  }
  if (amount == 1)
    strcpy(buf, "a gold coin");
  else if (amount <= 10)
    strcpy(buf, "a tiny pile of gold coins");
  else if (amount <= 20)
    strcpy(buf, "a handful of gold coins");
  else if (amount <= 75)
    strcpy(buf, "a little pile of gold coins");
  else if (amount <= 200)
    strcpy(buf, "a small pile of gold coins");
  else if (amount <= 1000)
    strcpy(buf, "a pile of gold coins");
  else if (amount <= 5000)
    strcpy(buf, "a big pile of gold coins");
  else if (amount <= 10000)
    strcpy(buf, "a large heap of gold coins");
  else if (amount <= 20000)
    strcpy(buf, "a huge mound of gold coins");
  else if (amount <= 75000)
    strcpy(buf, "an enormous mound of gold coins");
  else if (amount <= 150000)
    strcpy(buf, "a small mountain of gold coins");
  else if (amount <= 250000)
    strcpy(buf, "a mountain of gold coins");
  else if (amount <= 500000)
    strcpy(buf, "a huge mountain of gold coins");
  else if (amount <= 1000000)
    strcpy(buf, "an enormous mountain of gold coins");
  else
    strcpy(buf, "an absolutely colossal mountain of gold coins");

  return buf;
}


ObjData *create_money(int amount)
{
  ObjData *obj;
  struct extra_descr_data *new_descr;
  char buf[200];
  int y;

  if (amount <= 0) {
    mlog("SYSERR: Try to create negative or 0 money.");
    return NULL;
  }
  obj = create_obj();
  CREATE(new_descr, struct extra_descr_data, 1);

  if (amount == 1) {
    obj->name = str_dup("coin gold");
    obj->short_description = str_dup("a gold coin");
    obj->description = str_dup("One miserable gold coin is lying here.");
    new_descr->keyword = str_dup("coin gold");
    new_descr->description = str_dup("It's just one miserable little gold coin.");
  } else {
    obj->name = str_dup("coins gold");
    obj->short_description = str_dup(money_desc(amount));
    sprintf(buf, "%s is lying here.", money_desc(amount));
    obj->description = str_dup(CAP(buf));

    new_descr->keyword = str_dup("coins gold");
    if (amount < 10) {
      sprintf(buf, "There are %d coins.", amount);
      new_descr->description = str_dup(buf);
    } else if (amount < 100) {
      sprintf(buf, "There are about %d coins.", 10 * (amount / 10));
      new_descr->description = str_dup(buf);
    } else if (amount < 1000) {
      sprintf(buf, "It looks to be about %d coins.", 100 * (amount / 100));
      new_descr->description = str_dup(buf);
    } else if (amount < 100000) {
      sprintf(buf, "You guess there are, maybe, %d coins.",
	      1000 * ((amount / 1000) + number(0, (amount / 1000))));
      new_descr->description = str_dup(buf);
    } else
      new_descr->description = str_dup("There are a LOT of coins.");
  }

  new_descr->next = NULL;
  obj->ex_description = new_descr;

  GET_OBJ_TYPE(obj) = ITEM_MONEY;
  for (y = 0; y < TW_ARRAY_MAX; y++)
       obj->obj_flags.wear_flags[y] = 0;

  SET_BIT_AR(GET_OBJ_WEAR(obj), ITEM_WEAR_TAKE);
  GET_OBJ_VAL(obj, 0) = amount;
  GET_OBJ_COST(obj) = amount;
  obj->item_number = NOTHING;

  return obj;
}


/* Generic Find, designed to find any object/character                    */
/* Calling :                                                              */
/*  *arg     is the sting containing the string to be searched for.       */
/*           This string doesn't have to be a single word, the routine    */
/*           extracts the next word itself.                               */
/*  bitv..   All those bits that you want to "search through".            */
/*           Bit found will be result of the function                     */
/*  *ch      This is the person that is trying to "find"                  */
/*  **tar_ch Will be NULL if no character was found, otherwise points     */
/* **tar_obj Will be NULL if no object was found, otherwise points        */
/*                                                                        */
/* The routine returns a pointer to the next word in *arg (just like the  */
/* one_argument routine).                                                 */

int generic_find(char *arg, int bitvector, CharData * ch,
		     CharData ** tar_ch, ObjData ** tar_obj)
{
  int i, found;
  char name[256];

  one_argument(arg, name);

  if (!*name)
    return (0);

  *tar_ch = NULL;
  *tar_obj = NULL;

  if (IS_SET(bitvector, FIND_CHAR_ROOM)) {	/* Find person in room */
    if ((*tar_ch = get_char_room_vis(ch, name))) {
      return (FIND_CHAR_ROOM);
    }
  }
  if (IS_SET(bitvector, FIND_CHAR_WORLD)) {
    if ((*tar_ch = get_char_vis(ch, name, 0))) {
      return (FIND_CHAR_WORLD);
    }
  }
  if (IS_SET(bitvector, FIND_OBJ_EQUIP)) {
    for (found = FALSE, i = 0; i < NUM_WEARS && !found; i++)
      if (ch->equipment[i] && str_cmp(name, ch->equipment[i]->name) == 0) {
	*tar_obj = ch->equipment[i];
	found = TRUE;
      }
    if (found) {
      return (FIND_OBJ_EQUIP);
    }
  }
  if (IS_SET(bitvector, FIND_OBJ_INV)) {
    if ((*tar_obj = get_obj_in_list_vis(ch, name, ch->carrying))) {
      return (FIND_OBJ_INV);
    }
  }
  if (IS_SET(bitvector, FIND_OBJ_ROOM)) {
    if ((*tar_obj = get_obj_in_list_vis(ch, name, world[ch->in_room].contents))) {
      return (FIND_OBJ_ROOM);
    }
  }
  if (IS_SET(bitvector, FIND_OBJ_WORLD)) {
    if ((*tar_obj = get_obj_vis(ch, name))) {
      return (FIND_OBJ_WORLD);
    }
  }
  return (0);
}


/* a function to scan for "all" or "all.x" */
int find_all_dots(char *arg)
{
    if( !strcmp(arg, "all") ) return FIND_ALL;

    else if( !strncmp(arg, "all.", 4) ){
        strcpy(arg, arg + 4);
        return FIND_ALLDOT;
    }
    else
        return FIND_INDIV;
}







