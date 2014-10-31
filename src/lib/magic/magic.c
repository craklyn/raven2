/*******************************************************************************
 *  newmagic.c
 *  Written by Fred Merkel
 *  Part of JediMUD
 *  Copyright (C) 1993 Trustees of The Johns Hopkins Unversity
 *  All Rights Reserved.
 *  Based on DikuMUD, Copyright (C) 1990, 1991.
 *******************************************************************************/

#include "general/conf.h"
#include "general/sysdep.h"
#include "general/db.h"
#include "general/structs.h"
#include "util/utils.h"
#include "general/comm.h"
#include "general/class.h"
#include "magic/spells.h"
#include "magic/magic.h"
#include "general/handler.h"
#include "actions/interpreter.h"
#include "magic/skills.h"
#include "actions/fight.h"
#include "util/weather.h"
#include "scripts/dg_scripts.h"
#include "magic/sing.h"
#include "actions/act.h"          /* ACMDs located within the act*.c files */
#include "specials/special.h"
#include "magic/spell_parser.h"

/* external functions */
ACMD (do_flee);
ASPELL (spell_cleanse);
int dice (int number, int size);
int carrying_quest_item (CharData *ch);
int aff_level (CharData *ch, int skill);
int begin_singing (CharData *ch, CharData *victim, int songnum);
CharData *read_mobile (int, int);
ObjData *die (CharData *ch, CharData *killer, int pkill);
void awkward_dismount (CharData *ch);
void weight_change_object (ObjData * obj, int weight);

/*
 * Saving throws for:
 * MCTW
 *   PARA, ROD, PETRI, BREATH, SPELL
 *     Levels 0-40
 */

/* negative apply_saving_throw values make saving throws better! */


#define NO_APPLY APPLY_NONE
#define NO_BIT   0
#define NO_MOD   0

int applyDurationBonus (CharData *caster, int duration)
{
  { /* Do they have an amulet to increase this spells duration? */
    int increase = 0;
    int i;
    for (i = 0; i < MAX_OBJ_AFFECT; i++)
      {
        if (caster->equipment[WEAR_NECK] && caster->equipment[WEAR_NECK]->affected[i].location == APPLY_SPELL_DURATION)
          increase += caster->equipment[WEAR_NECK]->affected[i].modifier;
        if (caster->equipment[WEAR_ORBIT] && caster->equipment[WEAR_ORBIT]->affected[i].location == APPLY_SPELL_DURATION)
          increase += caster->equipment[WEAR_ORBIT]->affected[i].modifier;
      }
    if (IS_HOLY_PRIEST (caster) && GET_ADVANCE_LEVEL (caster) >= 2)
      increase += 15;
    duration += (int) (((duration + 1) * increase) / 100);

    return duration;
  } /* end of amulet check. */
}

void
add_affect (CharData * caster,
            CharData * target,
            int spell_num,
            int level,
            int location,
            int modifier,
            int duration,
            int bitvector,
            bool add_dur,
            bool avg_dur,
            bool add_mod,
            bool avg_mod)
{
  static struct affected_type affect;

  /* Any special effects on these kind of spells . */
  if (spell_num <= MAX_SPELLS && duration > 0 && caster)
    duration = applyDurationBonus (caster, duration);

  /* Resist poison gives you a good chance to ignore a poison */
  if (bitvector == AFF_POISON &&
      affected_by_spell (target, SPELL_RESIST_POISON) &&
      percentSuccess (70))
    {
      sendChar (target, "You shrug off the poison.\r\n");
      return;
    }

  affect.type = spell_num;
  affect.location = location;
  affect.modifier = modifier;
  affect.duration = duration;
  affect.bitvector = bitvector;
  affect.level = level;
  affect_join (target, &affect, add_dur, avg_dur, add_mod, avg_mod);

}/* add_affect */

/*
 ** This function will randomly pick a worn piece of equipment
 ** that is affected by the ITEM_CURSED flag.
 */
ObjData *
getCursedObj (CharData *ch)
{
  int oCount = 0;
  int target = 0;
  int i;

  /*
   ** Total up the number of pieces of cursed worn EQ.
   */
  for (i = 0; i < NUM_WEARS; i++)
    if ((ch->equipment[i]) && IS_OBJ_STAT (ch->equipment[i], ITEM_CURSED))
      oCount += 1;

  if (oCount == 0) return NULL;

  target = number (1, oCount);

  for (i = 0; i < NUM_WEARS; i++)
    if ((ch->equipment[i]) && IS_OBJ_STAT (ch->equipment[i], ITEM_CURSED))
      if (--target == 0)
        {
          return ( ch->equipment[i]);
        }
  /*
   ** If we get here something went wrong.
   */
  return NULL;
}

/* TODO: There are still quite a few things in the game that call this function
 * instead of magic_savingthrow. Perhaps at some point we can update all the
 * calls for this old function. -Xiuh 03.26.09 */

int
mag_savingthrow (CharData *ch, int type)
{
  int save;

  /* Vex Febuary 11
   ** This used to make ALL NPC's use mage saving throws. They now use
   ** the same save as a PC of their class.
   */
  save = saving_throws[(int) GET_CLASS (ch)][type][(int) GET_LEVEL (ch)];

  if (IS_AFFECTED (ch, AFF_CRUSADE)) save -= 5;
  if (IS_AFFECTED (ch, AFF_FEEBLE)) save += 10;
  if (IS_AFFECTED (ch, AFF_LORE)) save -= (aff_level (ch, AFF_LORE) - 15) / 10;

  save += GET_SAVE(ch, type);
  /* Vex Febuary 11
   ** Ensured that immortals, ALWAYS make their save(which can include some mobs)
   */
  if (GET_LEVEL (ch) >= LVL_IMMORT)
    return TRUE;
  else
    return ((MAX (1, save) < number (0, 99)) ? TRUE : FALSE);

}/* mag_savingthrow */

/* This replaces the mag_savingthrow function. Since caster is also passed
 ** in, we are able to compare caster and victim etc. Vex.
 */
int
magic_savingthrow (CharData *caster, CharData *victim, int type)
{
  int save, i, level;
  int favored_soul_level = 0;

  if (!victim)
  {
      mudlog (NRM, LVL_IMMORT, TRUE, "SYSERR: NULL victim passed to magic_savingthrow!");
      return TRUE;
  }
  
  if(IS_SHADE(caster) && skillSuccess(caster, SKILL_PENUMBRAE))
  {
      level = MIN(spell_level(caster, SKILL_ADUMBRATION) + 1, 120);
      affect_from_char(caster, SKILL_ADUMBRATION);
      add_affect(caster, caster, SKILL_ADUMBRATION, level + 15, APPLY_SPELL_DAMAGE, level + 15, 40, 0, FALSE, FALSE, FALSE, FALSE);
  }
  
  /* Gods(including some mobs) never fail. */
  if (GET_LEVEL (victim) >= LVL_IMMORT || affected_by_spell (victim, SKILL_PHASESHIFT))
      return TRUE;

  // spells from feebleminded casters fail 50% of the time, immediately.
  if (IS_AFFECTED (caster, AFF_FEEBLE) && number (0, 1)) return TRUE;

  // focused mages will often succeed
  if (skillSuccess (caster, SKILL_FOCUS) && number (1, 4) == 1) return FALSE;

  save = saving_throws[(int) GET_CLASS (victim)][type][(int) GET_LEVEL (victim)];
  save += GET_SAVE (victim, type); /* due to any items the victim is wearing. */

  // Feebleminded victims suffer a save penalty
  if (IS_AFFECTED (victim, AFF_FEEBLE)) save += 7;

  // Drow get a bonus
  if (IS_DROW (victim)) save -= 5;

  // Crusading victims score a bonus
  if (IS_AFFECTED (victim, AFF_CRUSADE)) save -= 5;

  // Forest Lore gives a bonus
  if (IS_AFFECTED (victim, AFF_LORE))
    save -= (aff_level (victim, AFF_LORE) - 15) / 10;

  /* Now look for anything the caster has that could affect the victims save. */
  if (caster)
  { /* There isn't neccessarily going to be a caster. */
      if (GET_CLASS (caster) == CLASS_MAGIC_USER && WIELDING(caster))
      { /* Is this caster a mage with a staff to enhance his power? */
          for (i = 0; i < MAX_OBJ_AFFECT; i++)
              if (caster->equipment[WEAR_WIELD]->affected[i].location == APPLY_SPELL_SAVE)
                  break;
          if (i < MAX_OBJ_AFFECT) /* we found a save affect. */
              save += caster->equipment[WEAR_WIELD]->affected[i].modifier;
      } /* end of mage staff check. */
      /* this affects everyone on an ORBIT */
      if (caster->equipment[WEAR_ORBIT])
      {
          for (i = 0; i < MAX_OBJ_AFFECT; i++)
              if (caster->equipment[WEAR_ORBIT]->affected[i].location == APPLY_SPELL_SAVE)
                  break;
          if (i < MAX_OBJ_AFFECT)
              save += caster->equipment[WEAR_ORBIT]->affected[i].modifier;
      }
  } /* end of caster affects on the victims save. */

  /* Wraithform makes you harder to resist
      ONLY if you are a necro */
  if (IS_AFFECTED (caster, AFF_WRAITHFORM) &&
      GET_CLASS (caster) == CLASS_NECROMANCER)
    save += aff_level (caster, AFF_WRAITHFORM) / 5 + 2;

  /* And makes you better at resisting
      ONLY if you are a necro */
  if (IS_AFFECTED (victim, AFF_WRAITHFORM) &&
      GET_CLASS (victim) == CLASS_NECROMANCER)
    save -= aff_level (caster, AFF_WRAITHFORM) / 10;


  if ((favored_soul_level = spell_level (caster, SPELL_FAVORED_SOUL)) > 0)
  {
      save += favored_soul_level;
      affect_from_char (caster, SPELL_FAVORED_SOUL);
  }

  if (MAX (1, save) < number (0, 99))
  {
      if (type == SAVING_SPELL && IS_DEFILER (caster))
      {
          send_to_char ("You reclaim some dark energies for future use.\r\n", caster);
          GET_MANA (caster) = MIN (GET_MAX_MANA (caster), GET_MANA (caster) + number (GET_LEVEL (caster) / 9, GET_LEVEL (caster) / 5));
      }
      if ((IS_DARK_PRIEST (caster) || IS_HOLY_PRIEST (caster))
              && GET_ADVANCE_LEVEL (caster) >= 3)
      {
          add_affect (caster, caster, SPELL_FAVORED_SOUL, favored_soul_level + 5, APPLY_SPELL_SAVE, 0,
                  -1, NO_BIT, FALSE, FALSE, TRUE, FALSE);
      }
      return TRUE;
  }
  else if (IS_PRESTIDIGITATOR(victim) && GET_ADVANCE_LEVEL(victim) >= THIRD_ADVANCE_SKILL && !COOLDOWN(victim, SLOT_SLIPPERY_MIND))
  {
      sendChar(victim, "The darkness adjusts the odds in your favor.\r\n");
      COOLDOWN(victim, SLOT_SLIPPERY_MIND) = 2;
      return magic_savingthrow(caster, victim, type);
  }
  else
  {
      return FALSE;
  }

}/* magic_savingthrow */

/* affect_update: called from comm.c (causes spells to wear off) */
void
affect_update (void)
{
  static struct affected_type *af, *next;
  static CharData *i, *nextchar;

  for (i = character_list; i; i = nextchar)
    {
      nextchar = i->next;
      i->call_to_corpse = 1;
      for (af = i->affected; af; af = next)
        {
          next = af->next;
          if (af->duration >= 1)
            af->duration--;
          else if (af->duration == -1)
            /* No action */
            af->duration = -1; /* GODs only! unlimited- possibly items.(vex) */
          else
            {
              if ((af->type > 0) && (af->type <= MAX_SPELLS))
                if (!af->next || (af->next->type != af->type) ||
                    (af->next->duration > 0))
                  if (*spell_wear_off_msg[af->type])
                    {
                      sendChar (i, "%s\r\n", spell_wear_off_msg[af->type]);
                    }
              affect_remove (i, af, FALSE); /* FALSE is for fall rooms */
            }
        }
    }
}

void
random_dispel (CharData * ch)
{
  struct affected_type *af, *remove = NULL;
  int r = 0;

  /* select a random spell affect */
  for (af = ch->affected; af; af = af->next)
    {
      if ((af->type > 0) && (af->type <= MAX_SPELLS) && (number (1, ++r) == 1))
        remove = af;
    }

  /* if one was found, blow it away */
  if (remove)
    {
      if (*spell_wear_off_msg[remove->type])
        {
          sendChar (ch, "%s\r\n", spell_wear_off_msg[remove->type]);
        }
      affect_remove (ch, remove, FALSE); /* FALSE is for fall rooms */
    }
}

int
debuff_protections (CharData *ch, CharData * victim)
{
  struct affected_type *af, *remove = NULL;
  int r = 0;

  /* select a random spell affect */
  for (af = victim->affected; af; af = af->next)
    {
      if ((af->type > 0) && (af->type <= MAX_SPELLS) && (number (1, ++r) == 1) &&
          affect_type_is_a_protection (af->type))
        remove = af;
    }

  /* if one was found, blow it away */
  if (remove)
    {
      struct affected_type af2;

      af2.type = remove->type;
      af2.duration = remove->duration / (1 TICKS); // Shortlived buff to caster
      af2.modifier = remove->modifier;
      af2.location = remove->location;
      af2.bitvector = remove->bitvector;

      affect_to_char (ch, &af2);

      if (*spell_wear_off_msg[remove->type])
        {
          sendChar (victim, "%s\r\n", spell_wear_off_msg[remove->type]);
        }
      affect_remove (victim, remove, FALSE); /* FALSE is for fall rooms */
      return TRUE;
    }
  else return FALSE;
}

int affect_type_is_a_protection(int type)
{
  switch (type)
    {
    case SPELL_SHIELD:
    case SPELL_SANCTUARY:
    case SPELL_ARMOR:
    case SPELL_BARKSKIN:
    case SPELL_AWAKEN:
    case SPELL_BLUR:
    case SPELL_HOLY_ARMOR:
    case SPELL_ARMOR_OF_CHAOS:
    case SPELL_NOXIOUS_SKIN:
    case SPELL_FORTIFY:
    case SPELL_CRUSADE:
    case SPELL_APOCALYPSE:
    case SPELL_MISSION:
      return TRUE;
    default:
      return FALSE;
    }
}

/*
  mag_materials:
  Checks for up to 3 vnums in the player's inventory.
 */
int
mag_materials (CharData * ch, int item0, int item1, int item2,
               int extract, int verbose)
{
    ObjData *tobj;
    ObjData *obj0, *obj1, *obj2;
    
    for (tobj = ch->carrying; tobj; tobj = tobj->next)
    {
        if ((item0 > 0) && (GET_OBJ_VNUM (tobj) == item0))
        {
            obj0 = tobj;
            item0 = -1;
        }
        else if ((item1 > 0) && (GET_OBJ_VNUM (tobj) == item1))
        {
            obj1 = tobj;
            item1 = -1;
        }
        else if ((item2 > 0) && (GET_OBJ_VNUM (tobj) == item2))
        {
            obj2 = tobj;
            item2 = -1;
        }
    }
    if ((item0 > 0) || (item1 > 0) || (item2 > 0))
    {
        if (verbose)
        {
            switch (number (0, 2))
            {
            case 0:
                send_to_char ("A wart sprouts on your nose.\r\n", ch);
                break;
            case 1:
                    send_to_char ("Your hair falls out in clumps.\r\n", ch);
                    break;
            case 2:
                send_to_char ("A huge corn develops on your big toe.\r\n", ch);
                break;
            }
        }
        return (FALSE);
    }
    if (extract)
    {
        if (item0 < 0)
        {
            obj_from_char (obj0);
            extract_obj (obj0);
        }
        if (item1 < 0)
        {
            obj_from_char (obj1);
            extract_obj (obj1);
        }
        if (item2 < 0)
        {
            obj_from_char (obj2);
            extract_obj (obj2);
        }
    }
    if (verbose)
    {
        send_to_char ("A puff of smoke rises from your pack.\r\n", ch);
        act ("A puff of smoke rises from $n's pack.", TRUE, ch, NULL, NULL, TO_ROOM);
    }
    return (TRUE);
}

/* ============================================================================
artFlower
This function performs the skill "art of the flower" for the shou-lin class.
The function is designed to reduce spell damage the shou-lin may suffer.
ch - the attacker
victim - the shou-lin
theSpell - spell being avoided
theDam - the damage
============================================================================ */
void
artFlower (CharData *ch, CharData *victim, int *theSpell, int *theDam, int savetype)
{
  if (GET_ASPECT (victim) != ASPECT_FLOWER)
    return;

  if (magic_savingthrow (ch, victim, savetype))
    {
      if (skillSuccess (victim, SKILL_ART_FLOWER))
        {
          sendChar (ch, "%s serenely deflects your %s!\r\n", GET_NAME (victim), spells[*theSpell]);
          sendChar (victim, "Harnessing your inner energy, you deflect %s's %s.\r\n", GET_NAME (ch), spells[*theSpell]);
          sprintf (buf, "%s serenely deflects %s's %s!", GET_NAME (victim), GET_NAME (ch), spells[*theSpell]);
          act (buf, FALSE, ch, 0, victim, TO_NOTVICT);
          *theDam = 0; /* Shou-lin suffers no damage! */
          *theSpell = TYPE_SPECIAL; /* suppress damage messages */
        }
      else
        *theDam >>= 1;
    }
  else if (skillSuccess (victim, SKILL_ART_FLOWER))
    {
      /* Caster and observors will know no different. */
      sendChar (victim, "Your strength of spirit protects you from the full force of %s's %s.\r\n", GET_NAME (ch), spells[*theSpell]);
      if (skillSuccess (ch, SKILL_ART_FLOWER))
        {
          // Chance to absorb some of the damage as mana.  50% chance of getting 1/4th, 1/8th, or 1/16th the damage as mana.
          sendChar (victim, "You absorb some energy from the attack.\r\n");
          GET_MANA (victim) = MIN (GET_MANA (victim) + (*theDam >> number (2, 4)), GET_MAX_MANA (victim));
        }
      *theDam >>= 1;

    }
  /* Otherwise they still take full damage. */
}

static void
range_spell_damage (CharData *ch,
                    CharData *victim,
                    int damage,
                    int spellnum)
{
  int old_room;

  /* the victim gets a chance to reduce the damage taken here */
  damage = damage_affects (ch, victim, damage, spellnum);
  damage = damage_limit (ch, victim, damage, spellnum);

  if (IS_PVP (ch, victim))
    damage /= pvpFactor ();
  else if (!(IS_NPC (ch) || IS_NPC (victim)))
    damage /= pvpFactor ();


  GET_HIT (victim) -= damage;

  /* have to be in the same room for messages to work! */
  old_room = ch->in_room;
  ch->in_room = victim->in_room;
  damage_display (ch, victim, damage, spellnum);
  ch->in_room = old_room;

  protect_unlinked_victim (victim);
  if (GET_POS (victim) == POS_DEAD)
    {
      die (victim, NULL, 0);
    }
}

/*
** Every spell that does damage comes through here.  This calculates the
** amount of damage, adds in any modifiers, determines what the saves are,
** tests for save and calls damage();
*/
void mag_damage( int lvl,
                 CharData *ch,
                 CharData *victim,
                 int spellnum,
                 int savetype )
{
    int dam = 0;

    if( victim == NULL || ch == NULL ) return;

    switch (spellnum) {
    case SPELL_EXPLODE_CORPSE: dam =  50 + dice(4, lvl) + (0 * lvl); break;
    case SPELL_MAGIC_MISSILE:  dam =   0 + dice(1,  20) + (1 * lvl); break;
    case SPELL_BLACK_DART:     dam =   0 + dice(8,   5) + (1 * lvl); break;
    case SPELL_CHILL_TOUCH:    dam =   0 + dice(2,  25) + (1 * lvl); break;
    case SPELL_BURNING_HANDS:  dam =   0 + dice(2,  40) + (1 * lvl); break;
    case SPELL_CAUSE_WOUND:    dam =   0 + dice(4,   8) + (1 * lvl); break;
    case SPELL_MALEDICT:       dam =   0 + dice(lvl, 2) + (-1* lvl); break;
    case SPELL_CAUSE_CRITIC:   dam =   0 + dice(12,  8) + (0 * lvl); break;
    case SPELL_CHAIN_LIGHTNING:dam =  50 + dice(lvl, 4) + (1 * lvl);
    case SPELL_SHOCKING_GRASP: dam =   5 + dice(10, 12) + (1 * lvl); break;
    case SPELL_CALL_LIGHTNING: dam =   7 + dice(7,   8) + (1 * lvl); break;
    case SPELL_COLOR_SPRAY:    dam =  45 + dice(2,  90) + (1 * lvl);
                               dam *= CAN_SEE(victim, ch) && FIGHTING(victim) == ch? 2:1;      
                                                                     break;
    case SPELL_BLACK_BREATH:   dam =  40 + dice(2,  80) + (2 * lvl); break;
    case SPELL_SHRIEK:         dam =  40 + dice(2, 100) + (2 * lvl); break;
    case SPELL_SHADOW_BLADES:  dam =  50 + dice(3, lvl) + (5 * lvl); break;
    case SPELL_FIREBALL:
		if (!IS_NPC(ch) && (savetype == SAVING_SPELL) ) {
				begin_singing(ch, victim, spellnum);
				ch->target = victim;
				return;
		}
		dam =  70 + dice(2, 100) + (5 * lvl); break;
    case SPELL_LIGHTNING_BOLT: dam =  90 + dice(2, 100) + (3 * lvl);
    case SPELL_DEATH_TOUCH:    dam =  90 + dice(2, 100) + (3 * lvl); break;
    case SPELL_DOOM_BOLT:      dam =  30 + dice(2, 120) + (5 * lvl); break;
    case SPELL_HARM:           dam = 200 + dice(4,   5) + (0 * lvl); break;
    case SPELL_EARTHQUAKE:     dam =  00 + dice(7,   7) + (2 * lvl); break;
    case SPELL_ICE_STORM:      dam =  00 + dice(10, 14) + (2 * lvl); break;
    case SPELL_METEOR_SWARM:   dam =  00 + dice(10, 23) + (3 * lvl); break;
    case SPELL_PESTILENCE:     dam =  10 + dice(1, lvl) + (0 * lvl); break;
    case SPELL_WALL_OF_FIRE:   dam =  10 + dice(2,  14) + (3 * lvl); break;
    case SPELL_TREMOR:         dam =  00 + dice(6,   7) + (2 * lvl); break;
    case SPELL_TSUNAMI:        dam =  20 + dice(6,   7) + (2 * lvl); break;
    case SPELL_TYPHOON:        dam =  00 + dice(7,   7) + (2 * lvl); break;
    case SPELL_WRATH_OF_THE_ANCIENTS:
                               dam =  40 + dice(2, 120) + (4 * lvl); break;
    case SPELL_ENTANGLE:       dam =  00 + dice(1,  10) + (1 * lvl); break;
    case SPELL_SWARM:          dam =  00 + dice(6,   7) + (2 * lvl); break;
    case SPELL_TERROR:         dam =  00 + dice(6,   6) + (1 * lvl); break;
    case SPELL_SOUL_PIERCE:    dam =  10 + dice(lvl, 9) + (2 * lvl); break;
    case SPELL_DISPEL_EVIL:    dam =  50 + dice(8,  20) + (2 * lvl);
         if( IS_EVIL(ch) ){
             victim = ch;
             dam = GET_HIT(ch) - 1;
         }
         else if( IS_GOOD(victim) ){
            act("The gods protect $N.", FALSE, ch, 0, victim, TO_CHAR);
            dam = 0;
            return;
         }
         break;
    case SPELL_DISPEL_GOOD:    dam =  50 + dice(8,  20) + (2 * lvl);
         if (IS_GOOD(ch)) {
             victim = ch;
             dam = GET_HIT(ch) - 1;
         }
         else if( IS_EVIL(victim) ){
             act("The gods protect $N.", FALSE, ch, 0, victim, TO_CHAR);
             dam = 0;
             return;
         }
         break;

  case SPELL_HOLY_WORD:
    if (IS_EVIL(victim) )
    {
        dam = dice(2,100) + (4 * lvl) + 40;
        if (GET_LEVEL(ch) > GET_LEVEL(victim))
          mag_affects(lvl, ch, victim, SPELL_SLOW, savetype);
          if ((GET_LEVEL (victim) < 25) || (GET_LEVEL (ch) >= LVL_IMMORT))
          mag_affects(lvl, ch, victim, SPELL_PARALYZE, savetype);
        break;
    }
    else  /* No effect on a non-evil opponent */
    {
      dam = 0;
      return;
    }
    break;
  case SPELL_UNHOLY_WORD:
    if (IS_GOOD(victim) )
    {
      dam = dice(2,100) + (4 * lvl) + 40;
      if (GET_LEVEL(ch) > GET_LEVEL(victim))
        mag_affects(lvl, ch, victim, SPELL_SLOW, savetype);
          if ((GET_LEVEL (victim) < 25) || (GET_LEVEL (ch) >= LVL_IMMORT))
	mag_affects(lvl, ch, victim, SPELL_PARALYZE, savetype);
      break;
    }
    else /* No effect on non-good opponent */
    {
      dam = 0;
      return;
    }
    break;
  case SPELL_DEMON_FIRE:
    if (IS_EVIL(ch) )
    {
      dam = 50 + dice(5,lvl) + 3 * lvl;
      act("You utter an evil incantation and summon up the fires of hell!", FALSE, ch, 0, 0, TO_CHAR);
      act("$n utters an evil incantation and summons up the fires of hell!", FALSE, ch, 0, 0, TO_ROOM);
      break;
    }
    else
    {
      send_to_char ("You must completely surrender your soul to evil before you can summon the\n\rfires of hell!\n\r",ch);
      return;
    }
    break;
  case SPELL_FLAME_STRIKE:
    if (IS_GOOD(ch) )
    {
      act("You raise your hands to the heavens and call down a purifying column of flame!", FALSE, ch, 0, 0, TO_CHAR);
      act("$n raises $s hands to the heavens and calls down a purifying column of flame!", FALSE, ch, 0, 0, TO_ROOM);
      dam = 50 + dice(5,lvl) + 3 * lvl;
      break;
    }
    else
    {
      send_to_char ("You must completely dedicate your soul to goodness before you can summon\n\rthe power of the heavens!\n\r",ch);
      return;
    }
    break;

    case SPELL_FIRE_BREATH:
    case SPELL_FROST_BREATH:
    case SPELL_ACID_BREATH:
    case SPELL_GAS_BREATH:
    case SPELL_LIGHTNING_BREATH: dam = ( GET_HIT(ch) < 60 ? 30 : 150 ); break;
    }

    if(spellnum == SPELL_CALL_LIGHTNING ||
            spellnum == SPELL_LIGHTNING_BOLT ||
            spellnum == SPELL_CHAIN_LIGHTNING) {
        weather_change();
        dam = OUTSIDE(ch) ? dam * (1 + weather_info.sky) / 2 : dam/2;
    }

    if(spellnum == SPELL_SHRIEK) {
        act("A piercing shriek fills the air!", FALSE, ch, 0, 0, TO_CHAR);
        act("A piercing shriek fills the air!", FALSE, ch, 0, 0, TO_ROOM);
        project_sound(ch, "You hear a distant shriek from %s.", "You hear a distant shriek.");
        
        add_affect(ch, victim, spellnum, GET_LEVEL(ch), APPLY_HITROLL, GET_LEVEL(victim) < LVL_IMMORT? -1:number(-1, 0),
                16, NO_BIT, FALSE, FALSE, FALSE, FALSE );
    }

    if(spellnum == SPELL_SHOCKING_GRASP) {
        weather_change();
        if(AFF_FLAGGED(victim, AFF_SANCTUARY))
            dam *= 2;
        else if(AFF_FLAGGED(victim, AFF_WARD))
            dam = dam * 4 / 3;

        if(AFF_FLAGGED(victim, AFF_SHIELD)) {
            sendChar(ch, "Your shocking grasp penetrates %s's force field.\r\n", GET_NAME(victim));
            dam = dam * 3 / 2;
        }
    }
    else {
        if( IS_SHOU_LIN(victim) )
            artFlower(ch, victim, &spellnum, &dam, savetype);
        else if( magic_savingthrow(ch, victim, savetype) )
            dam = dam * 75 / 100;

        /* Evasion allows some people to leap aside! */
        if (skillSuccess(victim, SKILL_EVASION) && (
                spellnum == SPELL_MAGIC_MISSILE ||
                spellnum == SPELL_BLACK_DART ||
                spellnum == SPELL_COLOR_SPRAY ||
                spellnum == SPELL_DOOM_BOLT))
        {
            sendChar(victim, "You nimbly dodge aside!\r\n");
            act("$n nimbly leaps aside!", TRUE, victim, 0, 0, TO_ROOM);
            dam = 0;
        }

        if (affected_by_spell(victim, SPELL_REFLECTION) && percentSuccess(15)) {
            sprintf(buf, "Your %s is thrown back at you!", spells[spellnum]);
            act(buf, TRUE, ch, 0, victim, TO_CHAR);
            sprintf(buf, "$n's %s is thrown back at $m!", spells[spellnum]);
            act(buf, TRUE, ch, 0, victim, TO_ROOM);
            victim = ch;
        }
    }

        if (ch->in_room != victim->in_room &&
                IS_SET(spell_info[spellnum].targets, TAR_CHAR_DIR)) {
            range_spell_damage(ch, victim, dam, spellnum);
        }
        else damage(ch, victim, dam, spellnum);
}

void project_sound(CharData *ch, char *from_dir_sound, char *other_sound)
{
    int i;
    int orig_room;
    char *rev_dirs[] = {
        "the south",
        "the west",
        "the north",
        "the east",
        "below",
        "above"
    };

    orig_room = ch->in_room;

    for (i = 0; i < NUM_OF_DIRS; i++)
    {
        ch->in_room = orig_room;
        if(CAN_GO(ch, i))
        {
            ch->in_room = world[orig_room].dir_option[i]->to_room;
            if (ch->in_room != orig_room)
            {
                if (CAN_GO(ch,rev_dir[i]))
                {
                    sprintf(buf, from_dir_sound, rev_dirs[i]);
                    act(buf,FALSE, ch, 0, 0, TO_ROOM);
                }
                else
                    act(other_sound,FALSE, ch, 0, 0, TO_ROOM);
            }
        }
    }
    ch->in_room = orig_room;
}

/*
** Every spell that does an affect comes through here.  This determines
** the effect, whether it is added or replacement, whether it is legal or
** not, etc.
**
*/
void mag_affects( int level,
                  CharData *ch,
                  CharData *victim,
                  int spellnum,
                  int savetype )
{
    int i;
    int duration  = 0;
    int maxLvl24  = MAX(24, level);

#define EXIT_IF_ALREADY_AFFECTED \
if (affected_by_spell(victim, spellnum)) { send_to_char("Nothing seems to happen.\r\n", ch); return; }

    if( victim == NULL || ch == NULL ) return;

    switch( spellnum ){
    case SPELL_CHILL_TOUCH:
        duration = ( magic_savingthrow( ch, victim, savetype ) ? 0 : 2 );
        add_affect( ch, victim, spellnum, level, APPLY_STR, -1, duration TICKS + number(15, 30), NO_BIT, TRUE, TRUE, TRUE, FALSE );
        send_to_char("You feel your strength wither!\r\n", victim);
        break;

    case SPELL_ARMOR:
		REMOVE_WEAK_PROTECTION(victim);
		EXIT_IF_STRONGLY_PROTECTED(victim);
		if(( affected_by_spell(victim, SPELL_HOLY_ARMOR))      ||
           ( affected_by_spell(victim, SPELL_BLUR))           ||
           ( affected_by_spell(victim, SPELL_ARMOR_OF_CHAOS)) ||
           ( affected_by_spell(victim, SPELL_ARMOR )))   {
            sendChar( ch, "Your attempt to armor has failed.\r\n" );
        }
        else {
            add_affect( ch, victim, spellnum, level, APPLY_AC, -20, 24 TICKS, NO_BIT, TRUE, FALSE, FALSE, FALSE );
            sendChar( victim, "You feel someone protecting you.\r\n" );
        }
        break;

    case SPELL_BLACK_DART:  /* Exactly the same as poison - Vex */
        if( magic_savingthrow(ch, victim, savetype )) return;
        add_affect( ch, victim, SPELL_POISON, level, APPLY_STR, -2, GET_LEVEL(ch) TICKS, AFF_POISON, FALSE, FALSE, FALSE, FALSE);
        sendChar( victim, "You feel very sick.\r\n" );
        act( "$N gets violently ill!", TRUE, ch, NULL, victim, TO_ROOM );
        break;

    case SPELL_BLESS:
        affect_from_char( victim, SPELL_BLESS);
        add_affect( ch, victim, spellnum, level, APPLY_SAVING_SPELL, -1 * (level/10), 12 TICKS, NO_BIT, FALSE, FALSE, FALSE, FALSE);
        sendChar( victim, "You feel righteous.\r\n" );
        break;

    case SPELL_BLINDNESS: EXIT_IF_ALREADY_AFFECTED;
        if( IS_SET_AR(MOB_FLAGS(victim), MOB_NOBLIND) ||
            magic_savingthrow(ch, victim, savetype )){
            act( "Your attempt to blind $N has failed.", FALSE, ch, 0, victim, TO_CHAR );
            return;
        }
        act( "$n seems to be blinded!", TRUE, victim, 0, 0, TO_ROOM );
        sendChar( victim, "You have been blinded!\r\n" );
        add_affect( ch, victim, spellnum, level, NO_APPLY, NO_MOD, 2 TICKS, AFF_BLIND, FALSE, FALSE, FALSE, FALSE);
        break;

    case SPELL_HARM:
        i = MIN(spell_level(victim, SPELL_HARM), 15);
        affect_from_char(victim, spellnum);
        add_affect(ch, victim, spellnum, i + 3, NO_APPLY, NO_MOD, 20 , 0, FALSE, FALSE, FALSE, FALSE);
        mag_damage(spellnum, ch, victim, spellnum, SAVING_SPELL);
        break;

    case SPELL_MALEDICT:
        add_affect(ch, victim, SPELL_MALEDICT, level, NO_APPLY, NO_MOD, 3 , 0, FALSE, FALSE, FALSE, FALSE);
        add_affect(ch, ch, SPELL_MALEDICT2, level, NO_APPLY, NO_MOD, 3, 0, FALSE, FALSE, FALSE, FALSE);
        TARGET(ch) = victim;
        act("A dark curse is woven on $N.", FALSE, ch, 0, victim, TO_CHAR);
        act("You feel influenced by a dark curse.", FALSE, ch, 0, victim, TO_VICT);
        act("$n weaves a dark curse on $N.", FALSE, ch, 0, victim, TO_NOTVICT);
        mag_damage(spellnum, ch, victim, spellnum, SAVING_SPELL);
        break;

    case SPELL_CURSE:
        if( savetype != SAVING_SPELL && magic_savingthrow( ch, victim, savetype )){
            act( "Your attempt to put a curse on $N has failed.", TRUE, ch, NULL, victim, TO_CHAR );
            return;
        }
        duration = 1 + (GET_LEVEL(ch)/15);
        if(IS_IMMORTAL(victim)) {
            add_affect(ch, victim, spellnum, level, APPLY_SKILL_SUCCESS, -5, duration TICKS, AFF_CURSE, FALSE, FALSE, FALSE, FALSE);
        }
        else {
            if(GET_ADVANCE_LEVEL(ch) > THIRD_ADVANCE_SKILL && IS_REVENANT(ch))
                add_affect(ch, victim, spellnum, level, APPLY_SKILL_SUCCESS, -15, duration TICKS, AFF_CURSE, FALSE, FALSE, FALSE, FALSE);
            else
                add_affect(ch, victim, spellnum, level, APPLY_SKILL_SUCCESS, -10, duration TICKS, AFF_CURSE, FALSE, FALSE, FALSE, FALSE);
        }
        act( "$n briefly glows red!", FALSE, victim, 0, 0, TO_ROOM);
        act( "You feel very uncomfortable.", FALSE, victim, 0, 0, TO_CHAR);

        set_fighting(victim, ch);
        set_fighting(ch, victim);
        break;

    case SPELL_DETECT_ALIGN: EXIT_IF_ALREADY_AFFECTED;
        add_affect( ch, victim, spellnum, level, NO_APPLY, NO_MOD, (level/5) TICKS, AFF_DETECT_ALIGN, TRUE, FALSE, FALSE, FALSE);
        send_to_char("Your eyes tingle.\r\n", victim);
        break;

    case SPELL_DETECT_INVIS:
	if( affected_by_spell(victim, SPELL_DETECT_INVIS) )
		if( aff_level(victim, AFF_DETECT_INVIS) <= level )
			affect_from_char( victim, SPELL_DETECT_INVIS);
	EXIT_IF_ALREADY_AFFECTED;
	add_affect( ch, victim, spellnum, level, NO_APPLY, NO_MOD, (level + 12) TICKS, AFF_DETECT_INVIS, TRUE, FALSE, FALSE, FALSE);
        send_to_char("Your eyes tingle.\r\n", victim);
        break;

    case SPELL_DETECT_MAGIC: EXIT_IF_ALREADY_AFFECTED;
        add_affect( ch, victim, spellnum, level, NO_APPLY, NO_MOD, (level/5) TICKS, AFF_DETECT_MAGIC, TRUE, FALSE, FALSE, FALSE);
        send_to_char("Your eyes tingle.\r\n", victim);
        break;

    case SPELL_DETECT_POISON: /* this spell don't belong here. */
        if( victim == ch ){
            if( IS_AFFECTED( victim, AFF_POISON ))
                sendChar( ch, "You can sense poison in your blood.\r\n" );
            else
                sendChar( ch, "You feel healthy.\r\n" );
        }
        else if( IS_AFFECTED(victim, AFF_POISON ))
            act( "You sense that $E is poisoned.", FALSE, ch, 0, victim, TO_CHAR);
        else
            act( "You sense that $E is healthy.", FALSE, ch, 0, victim, TO_CHAR);
        break;

    case SPELL_INFRAVISION:
        if(GET_CLASS(ch) == CLASS_ASSASSIN && (savetype == SAVING_SPELL)) {
            sendChar(ch, "You lost your concentration!  Use the 'innereye' command.\r\n");
            return;
        }
	if(affected_by_spell(victim, SPELL_INFRAVISION) && aff_level(victim, AFF_INFRAVISION) <= level)
            affect_from_char(victim, SPELL_INFRAVISION);
	EXIT_IF_ALREADY_AFFECTED;
        add_affect( ch, victim, spellnum, level, NO_APPLY, NO_MOD, (level + 12) TICKS, AFF_INFRAVISION, TRUE, FALSE, FALSE, FALSE);
        sendChar( victim, "Your eyes glow red.\r\n" );
        act("$n's eyes glow red.", TRUE, victim, 0, 0, TO_ROOM);
        break;

    case SPELL_INVISIBLE: EXIT_IF_ALREADY_AFFECTED;
        if (!victim) victim = ch;
        act("$n slowly fades out of existence.", TRUE, victim, 0, 0, TO_NOTVICT);
        sendChar( victim, "You vanish.\r\n" );
        duration = 12 + (GET_LEVEL(ch) >> 2);
        add_affect( ch, victim, spellnum, level, NO_APPLY, NO_MOD, duration TICKS, AFF_INVISIBLE, TRUE, FALSE, FALSE, FALSE);
        break;

    case SPELL_POISON:
        if( magic_savingthrow( ch, victim, savetype )) return;
        add_affect( ch, victim, spellnum, level, APPLY_STR, -2, GET_LEVEL(ch) TICKS, AFF_POISON, FALSE, FALSE, FALSE, FALSE);
        act("You feel very sick.", TRUE, ch, NULL, victim, TO_VICT);
        act("$N gets violently ill!", TRUE, ch, NULL, victim, TO_NOTVICT);
        act("$N gets violently ill!", TRUE, ch, NULL, victim, TO_CHAR);
        break;

    case SPELL_SANCTUARY:
	if( affected_by_spell( victim, SPELL_SANCTUARY) )
		affect_from_char( victim, SPELL_SANCTUARY);

        act( "$n is surrounded by a white aura.", TRUE, victim, 0, 0, TO_ROOM );
        sendChar( victim, "You start glowing.\r\n");
        add_affect( ch, victim, spellnum, level, NO_APPLY, NO_MOD, 4 TICKS, AFF_SANCTUARY, TRUE, FALSE, FALSE, FALSE);
        break;

    case SPELL_SLEEP: EXIT_IF_ALREADY_AFFECTED;
      if (!CONFIG_PK_ALLOWED && !IS_NPC (ch) && !IS_NPC (victim))
            return;

        if (!ZONE_FLAGGED(world[ch->in_room].zone, ZONE_SLEEPTAG)) {
            if( magic_savingthrow( ch, victim, savetype )) return;
        } else if (number(1, 100) > 80)
            break;

	if (!ZONE_FLAGGED(world[ch->in_room].zone, ZONE_SLEEPTAG))
            if (GET_LEVEL(victim) > 25) {
                sendChar( victim, "You start to feel groggy but manage to snap out of it." );
                return;
            }
        if( IS_NPC(victim) && IS_SET_AR( MOB_FLAGS( victim ), MOB_NOSLEEP )) return;
        duration = 4 + (level >> 2);
        add_affect( ch, victim, spellnum, level, NO_APPLY, NO_MOD, duration TICKS, AFF_SLEEP, FALSE, FALSE, FALSE, FALSE);

        if (GET_POS(victim) > POS_SLEEPING) {
            sendChar( victim, "You feel very sleepy...zzzzzz" );
            act( "$n falls to the floor in a magic induced sleep.", TRUE, victim, 0, 0, TO_ROOM);
            if (IS_NPC(victim) && victim->rider)
                awkward_dismount(victim->rider);
            if (!IS_NPC(victim) && victim->mount)
                awkward_dismount(victim);
            GET_POS(victim) = POS_SLEEPING;
        }
        break;

    case SPELL_DANCE_DREAMS: EXIT_IF_ALREADY_AFFECTED;
    if( IS_NPC(victim) && IS_SET_AR( MOB_FLAGS( victim ), MOB_NOSLEEP )) return;

    if( magic_savingthrow( ch, victim, savetype) ) {
        if(IS_NPC(victim)) {
            return;
        }
        else if( !OFF_BALANCE(victim) || !percentSuccess(fleeValue * 2))
        {
            act("$N dances hypnotically, but you shake off the need to sleep.", TRUE,
                    victim, 0, ch, TO_CHAR);
            act("$N dances hypnotically, but to no avail.", TRUE,
                    victim, 0, ch, TO_NOTVICT);
            act("You fail to dance $n to sleep.", TRUE, victim, 0, ch, TO_VICT);
            return;
        }
    }

    if( IS_NPC(victim) )
        add_affect(ch, victim, spellnum, level, NO_APPLY, NO_MOD, 2 TICKS,
                AFF_SLEEP, FALSE, FALSE, FALSE, FALSE);
    else
    {
        add_affect(ch, victim, spellnum, level, NO_APPLY, NO_MOD, 2 TICKS,
                AFF_SLEEP, FALSE, FALSE, FALSE, FALSE);
        OFF_BALANCE(victim) = 0;
    }
    if (GET_POS(victim) > POS_SLEEPING) {
        act("$N dances hypnotically, sending you into a deep sleep.", TRUE,
                victim, 0, ch, TO_CHAR);
        act("$N dances hypnotically, sending $n into a deep sleep.", TRUE,
                victim, 0, ch, TO_NOTVICT);
        act("You dance $n into a deep sleep.", TRUE, victim, 0, ch, TO_VICT);
        if (IS_NPC(victim) && victim->rider)
            awkward_dismount(victim->rider);
        if (!IS_NPC(victim) && victim->mount)
            awkward_dismount(victim);
        GET_POS(victim) = POS_SLEEPING;
        end_fight(victim);
    }
    break;

    case SPELL_DANCE_MISTS:
		EXIT_IF_ALREADY_AFFECTED;
        if (magic_savingthrow(ch, victim, savetype)) return;
        add_affect(ch, victim, spellnum, level, APPLY_HITROLL, -4, 1 TICKS, NO_BIT,
            FALSE, FALSE, FALSE, FALSE);
        act("$N weaves around you in a strange dance.", TRUE,
            victim, 0, ch, TO_CHAR);
        act("$N begins to weave around $n in a strange dance.", TRUE,
            victim, 0, ch, TO_NOTVICT);
        act("You dance around $n, confusing their attacks.", TRUE, victim, 0,
            ch, TO_VICT);
        break;

    case SPELL_DANCE_SHADOWS:
        act("Your movements flow within the shadows.", TRUE, ch, 0, victim, TO_VICT);
        act("$N's movements flow into the shadows.", TRUE, ch, 0, victim, TO_ROOM);
        if (affected_by_spell(victim, spellnum) && ( savetype == SAVING_SPELL )  &&
                (GET_LEVEL(ch) > 35) ) {
            begin_singing(ch, ch, spellnum);
        }
        affect_from_char(ch, SPELL_DANCE_SHADOWS);
        add_affect(ch, victim, spellnum, level, APPLY_NONE, NO_MOD, (level/10) TICKS,
                AFF_HIDE, FALSE, FALSE, FALSE, FALSE);
            
        break;

    case SPELL_STRENGTH:
        if( affected_by_spell(victim, spellnum) >= 8 ) EXIT_IF_ALREADY_AFFECTED;
        duration = (level >> 1) + 4;
        add_affect( ch, victim, spellnum, level, APPLY_STR, (1 + (level > 18)), duration TICKS, NO_MOD, TRUE, TRUE, TRUE, FALSE );
        send_to_char("You feel stronger!\r\n", victim);
        break;

    case SPELL_SENSE_LIFE:
        if(GET_CLASS(ch) == CLASS_ASSASSIN && (savetype == SAVING_SPELL)) {
            sendChar(ch, "You lost your concentration!  Use the 'innereye' command.\r\n");
            return;
        }
	if( affected_by_spell( victim, SPELL_SENSE_LIFE) )
		if( aff_level( victim, AFF_SENSE_LIFE) <= level)
			affect_from_char( victim, SPELL_SENSE_LIFE);

	EXIT_IF_ALREADY_AFFECTED;
        add_affect( ch, victim, spellnum, level, NO_APPLY, NO_MOD, level TICKS, AFF_SENSE_LIFE, TRUE, FALSE, FALSE, FALSE );
        send_to_char("You feel your awareness improve.\r\n", ch);
        //if (GET_SENSE_LVL(victim) < level) GET_SENSE_LVL(victim) = level;
        break;

    case SPELL_FLY:
        affect_from_char(victim, SPELL_FLY);
        add_affect(ch, victim, spellnum, level, NO_APPLY, NO_MOD, maxLvl24 TICKS, AFF_FLY, TRUE, FALSE, FALSE, FALSE);
        sendChar(victim, "You float into the air.\r\n");
        act("$n rises into the air.", TRUE, victim, 0, 0, TO_ROOM);
        break;

    case SPELL_AIRSPHERE:
	if( affected_by_spell(victim, SPELL_AIRSPHERE))
		affect_from_char( victim, SPELL_AIRSPHERE);

        add_affect( ch, victim, spellnum, level, NO_APPLY, NO_MOD, 3 TICKS, AFF_AIRSPHERE, TRUE, FALSE, FALSE, FALSE );
        sendChar( victim, "You are surrounded by a large bubble.\r\n" );
        act("$n is surrounded by a large bubble.", TRUE, victim, 0, 0, TO_ROOM);
	WATER_COUNTER(victim) = 0; /* Reset counter */
        break;

    case SPELL_SHADOW_WALK :
        affect_from_char(victim, SPELL_SHADOW_WALK);
        if(savetype == SAVING_SPELL)
            add_affect(ch, victim, spellnum, level, NO_APPLY, NO_MOD, -1, 
                    AFF_SHADOW_WALK, TRUE, FALSE, FALSE, FALSE );
	else
            add_affect( ch, victim, spellnum, level, NO_APPLY, NO_MOD, (level/2) TICKS, 
                    AFF_SHADOW_WALK, TRUE, FALSE, FALSE, FALSE );
	
        
	sendChar( victim, "You feel at one with the shadows.\r\n" );
	break;

    case SPELL_BARKSKIN:
	REMOVE_WEAK_PROTECTION(victim);
	REMOVE_STRONG_PROTECTION(victim);
        if(savetype == SAVING_SPELL)
            add_affect( ch, victim, spellnum, level, NO_APPLY, 0,
		-1, NO_MOD, TRUE, FALSE, FALSE, FALSE );
        else
            add_affect( ch, victim, spellnum, level, NO_APPLY, 0,
		level/10 TICKS, NO_MOD, TRUE, FALSE, FALSE, FALSE );
        sendChar( victim, "Needle-sharp thorns erupt from your pores.\r\n");
        act( "$n's skin takes on a woody texture and sprouts thorns at the surface.", TRUE, victim, 0, 0, TO_ROOM );
        break;

    case SPELL_AWAKEN:
	REMOVE_WEAK_PROTECTION(victim);
	REMOVE_STRONG_PROTECTION(victim);
        if(savetype == SAVING_SPELL && ch == victim)
            add_affect(ch, victim, spellnum, level, APPLY_SPELL_DAMAGE, 45, 
                    -1, NO_MOD, TRUE, FALSE, FALSE, FALSE );        
	else
            add_affect(ch, victim, spellnum, level, APPLY_SPELL_DAMAGE, 45, 
                    (level TICKS)/3, NO_MOD, TRUE, FALSE, FALSE, FALSE );
        
        sendChar( victim, "Your sense for the arcane comes into focus.\r\n" );
        break;

    case SPELL_REGENERATE:
        affect_from_char(victim, SPELL_REGENERATE);
        add_affect(ch, victim, spellnum, GET_LEVEL(ch), NO_APPLY, 0, 40, AFF_REGENERATE, FALSE, FALSE, FALSE, FALSE);
        sendChar( victim, "Your body seems to be regenerating.\r\n" );
        act("$n looks invigorated.", TRUE, victim, 0, 0, TO_ROOM);
        break;

    case SPELL_PROT_FROM_EVIL: EXIT_IF_ALREADY_AFFECTED;
        if( !IS_GOOD(victim) ){
            sendChar( victim, "Your god is displeased!\r\n" );
            GET_HIT(victim) = MAX(1,GET_HIT(victim) - 100 );
            return;
        }
        add_affect( ch, victim, spellnum, level, NO_APPLY, NO_MOD, 4 TICKS, AFF_PROTECT_EVIL, TRUE, FALSE, FALSE, FALSE);
        sendChar( victim, "You feel invulnerable!\r\n" );
        break;

    case SPELL_PROT_FROM_GOOD: EXIT_IF_ALREADY_AFFECTED;
        if( !IS_EVIL( victim )){
            sendChar( victim, "Your god is displeased!\r\n" );
            GET_HIT(victim) = MAX(1,GET_HIT(victim) - 100 );
            return;
        }
        add_affect( ch, victim, spellnum, level, NO_APPLY, NO_MOD, 4 TICKS, AFF_PROTECT_GOOD, TRUE, FALSE, FALSE, FALSE);
        send_to_char("You feel invulnerable!\r\n", victim);
        break;

    case SPELL_PARALYZE: EXIT_IF_ALREADY_AFFECTED;
        if ((GET_HIT(victim) > 100*GET_LEVEL(ch)) || GET_LEVEL(victim) > GET_LEVEL(ch))
	{
	    sendChar( ch, "They are immune to your power!\r\n");
	    set_fighting(victim, ch);
	    set_fighting(ch, victim);
	    return;
	}
        if( magic_savingthrow(ch, victim, savetype)) {
            sendChar( ch, "Your attempt to paralyze failed.\r\n" );
	    set_fighting(victim, ch);
	    set_fighting(ch, victim);
            return;
        }
        act( "$n seems to be paralyzed!", TRUE, victim, 0, 0, TO_ROOM);
        sendChar( victim, "You have been paralyzed!\r\n" );
        add_affect( ch, victim, spellnum, level, APPLY_AC, 20, 1 TICKS, AFF_PARALYZE, FALSE, FALSE, FALSE, FALSE);
        break;

    case SPELL_HOLY_ARMOR:
	REMOVE_WEAK_PROTECTION(victim)
	REMOVE_STRONG_PROTECTION(victim)
      if( !IS_GOOD(ch) || (victim && victim != ch && !IS_GOOD(victim))){
        send_to_char("Your deity does not feel particularly inclined to perform this task.\r\n", ch);
        }
    else {
	add_affect( ch, victim, spellnum, level, APPLY_AC, -50, 24 TICKS, NO_BIT, TRUE, FALSE, FALSE, FALSE);
        send_to_char("You feel a divine influence wash over you, coating you in a shimmering armor\nstronger than steel.\r\n", victim);
    }
    break;

  case SPELL_ARMOR_OF_CHAOS:
	REMOVE_WEAK_PROTECTION(victim)
	REMOVE_STRONG_PROTECTION(victim)
	if ( (victim && victim != ch && !IS_EVIL(victim)) || !IS_EVIL(ch)) {
        send_to_char("Your deity is displeased with your decision and refuses you.\r\n", ch);
    }
    else
	{
		add_affect( ch, victim, spellnum, level, APPLY_AC, -50, 24 TICKS, NO_BIT, TRUE, FALSE, FALSE, FALSE);
        send_to_char("You feel surrounded in an unholy light, its reddish glow ready to repel\n all attacks.\r\n", victim);
    }
    break;

  case SPELL_HASTE:
	if (affected_by_spell(victim, SPELL_SLOW))
	{
		mag_unaffects(level, ch, victim, spellnum, savetype);
		return;
	}
	act("$N starts moving at a blinding speed!", TRUE, ch, 0, victim, TO_NOTVICT);
	if (victim != ch)
	    act("$N starts moving at a blinding speed!", TRUE, ch, 0, victim, TO_CHAR);
	send_to_char("You begin to move at a blinding speed!\r\n", victim);
        affect_from_char(victim, SPELL_HASTE);
        add_affect(ch, victim, spellnum, level, APPLY_NONE, NO_MOD, 3 TICKS, AFF_HASTE, FALSE, FALSE, FALSE, FALSE);
	break;

  case SPELL_SLOW:
	EXIT_IF_ALREADY_AFFECTED;
	if (magic_savingthrow(ch, victim, savetype)) {
		send_to_char("You fail.\r\n", ch);
		return;
	}
	if (affected_by_spell(victim, SPELL_HASTE)) {
		mag_unaffects(level, ch, victim, spellnum, savetype);
		return;
	}
	if (victim != ch)
	    act("$N's movements slow to a crawl.", TRUE, ch, 0, victim, TO_CHAR);
	act("$N's movements slow to a crawl.", TRUE, ch, 0, victim, TO_NOTVICT);
	send_to_char("You feel your movements slowing down at a dramatic rate.\r\n", victim);
	add_affect( ch, victim, spellnum, level, APPLY_NONE, NO_MOD, 3 TICKS, AFF_HASTE, FALSE, FALSE, FALSE, FALSE);
	// NFI why it used to affect haste - Craklyn Because it worked - Bean
	// add_affect( ch, victim, spellnum, level, APPLY_NONE, NO_MOD, 3 TICKS, NO_MOD, FALSE, FALSE, FALSE, FALSE);
	break;

  case SPELL_SHIELD:
	  if (affected_by_spell(victim, spellnum) && ( savetype == SAVING_SPELL )  &&
		  victim == ch && GET_CLASS(ch) == CLASS_MAGIC_USER ) {
		  begin_singing(ch, ch, spellnum);
		  break;
	  }

	  if (victim != ch)
              act("$N is surrounded by a shimmering globe.", TRUE, ch, 0, victim, TO_CHAR);
	  act("$N is surrounded by a shimmering globe.", TRUE, ch, 0, victim, TO_NOTVICT);
	  act("You are surrounded by a shimmering globe.", TRUE, ch, 0, victim, TO_VICT);
          affect_from_char(victim, SPELL_SHIELD);
	  add_affect( ch, victim, spellnum, level, APPLY_AC, -10, (level/3) TICKS, AFF_SHIELD, FALSE, FALSE, FALSE, FALSE);
	  break;

  case SPELL_WARD:
	if (affected_by_spell(victim, SPELL_WARD) )
		affect_from_char(victim, SPELL_WARD);

	if (victim !=ch)
	  act("$N is protected by your diety.", TRUE, ch, 0, victim, TO_CHAR);
	act("You feel protected.", TRUE, ch, 0, victim, TO_VICT);
	add_affect( ch, victim, spellnum, level, NO_APPLY, NO_MOD, ((level/3) + 2) TICKS, AFF_WARD, FALSE, FALSE, FALSE, FALSE);
	break;

    case SPELL_FEAR:
        if (IS_DEFILER(ch) && GET_ADVANCE_LEVEL(ch) >= THIRD_ADVANCE_SKILL && !number(0, 2)) {
            affect_from_char(victim, SPELL_PARANOIA);
            add_affect( ch, victim, SPELL_PARANOIA, level, 0, 0, 2 TICKS, NO_BIT, FALSE, FALSE, FALSE, FALSE );
            send_to_char("Something seems terribly wrong!\r\n", victim);
            act("$N is disoriented.", TRUE, ch, NULL, victim, TO_NOTVICT);
            act("$N is disoriented.", TRUE, ch, NULL, victim, TO_CHAR);
        }
        // Intentionally left out (break;) so that SPELL_FEAR gives same behvaior as SPELL_TERROR:
    case SPELL_TERROR: /* this spell doesn't belong here, either? */
        if( magic_savingthrow(ch, victim, savetype) ||
          ( IS_NPC(victim) && IS_SET_AR(MOB_FLAGS(victim), MOB_SENTINEL))){
            sendChar( ch, "You fail.\n\r" );
            if( !FIGHTING(victim) ) set_fighting(victim, ch);
            return;
        }
        do_flee(victim, "", 0, 0);
        break;

    case SPELL_FLEET_FOOT: EXIT_IF_ALREADY_AFFECTED;
        add_affect( ch, victim, spellnum, level, APPLY_DEX, 2, level TICKS, NO_BIT, FALSE, FALSE, FALSE, FALSE );
        act("You feel light and fast on your feet.", TRUE, ch,0 ,victim, TO_VICT);
        break;

    case SPELL_TRUE_SIGHT:
	if( affected_by_spell( victim, SPELL_DETECT_INVIS) )
		if( aff_level(victim, AFF_DETECT_INVIS) <= level)
			affect_from_char(victim, SPELL_DETECT_INVIS);
	if( affected_by_spell( victim, SPELL_SENSE_LIFE) )
		if(aff_level(victim, AFF_SENSE_LIFE) <= level)
			affect_from_char(victim, SPELL_SENSE_LIFE);

        if( !affected_by_spell( victim, SPELL_DETECT_INVIS ))
            add_affect( ch, victim, SPELL_DETECT_INVIS, level, APPLY_NONE, 0, maxLvl24 TICKS,
                        AFF_DETECT_INVIS, FALSE, FALSE, FALSE, FALSE);
	if( !affected_by_spell(victim, SPELL_SENSE_LIFE ))
            add_affect( ch, victim, SPELL_SENSE_LIFE, level, APPLY_NONE, 0, maxLvl24 TICKS,
                        AFF_SENSE_LIFE, FALSE, FALSE, FALSE, FALSE);
        act("Your eyesight feels keener.", TRUE, ch, 0, victim, TO_VICT);
	break;

    case SPELL_EYES_OF_THE_DEAD: EXIT_IF_ALREADY_AFFECTED;
	if( !IS_EVIL(ch) )
	{
          sendChar (ch, "You must be wholly evil to see as the undead do.\r\n");
		break;
	}

	if( affected_by_spell(victim, SPELL_INFRAVISION) )
		if( aff_level(victim, AFF_INFRAVISION) <= level)
			affect_from_char( victim, SPELL_INFRAVISION);
	if(affected_by_spell(victim, SPELL_SENSE_LIFE) )
		if( aff_level( victim, AFF_SENSE_LIFE) <= level)
			affect_from_char( victim, SPELL_SENSE_LIFE);

        if( !affected_by_spell( victim, SPELL_INFRAVISION ))
            add_affect( ch, victim, SPELL_INFRAVISION, level, APPLY_NONE, 0, maxLvl24 TICKS,
                        AFF_INFRAVISION, FALSE, FALSE, FALSE, FALSE);
        if( !affected_by_spell(victim, SPELL_SENSE_LIFE ))
            add_affect( ch, victim, SPELL_SENSE_LIFE, level, APPLY_NONE, 0, maxLvl24 TICKS,
                        AFF_SENSE_LIFE, FALSE, FALSE, FALSE, FALSE);
        act("$N's eyes flicker and roll over.", TRUE, ch, 0, victim, TO_ROOM);
      sendChar (ch, "Your eyes flicker and roll over.\r\n");
        break;

    case SPELL_RIGHTEOUS_VISION: EXIT_IF_ALREADY_AFFECTED;
	if( !IS_GOOD( ch ))
	{
          sendChar (ch, "You must dedicate yourself to goodness to see as the righteous see.\r\n");
		break;
	}

	if(affected_by_spell(victim, SPELL_INFRAVISION) )
		if(aff_level(victim, AFF_INFRAVISION) <= level)
			affect_from_char(victim, SPELL_INFRAVISION);
	if( affected_by_spell(victim, SPELL_SENSE_LIFE) )
		if(aff_level(victim, AFF_SENSE_LIFE) <= level)
			affect_from_char(victim, SPELL_SENSE_LIFE);

        if (!affected_by_spell(victim, SPELL_INFRAVISION))
          add_affect( ch, victim, SPELL_INFRAVISION, level, APPLY_NONE, 0, maxLvl24 TICKS,
                      AFF_INFRAVISION, FALSE, FALSE, FALSE, FALSE);
        if (!affected_by_spell(victim, SPELL_SENSE_LIFE))
          add_affect( ch, victim, SPELL_SENSE_LIFE, level, APPLY_NONE, 0, maxLvl24 TICKS,
                      AFF_SENSE_LIFE, FALSE, FALSE, FALSE, FALSE);
        act("$N's eyes glow brightly.", TRUE, ch, 0, victim, TO_ROOM);
      sendChar (ch, "Your eyes glow brightly.\r\n");
        break;

  case SPELL_SHADOW_VISION:
	  if(ch->in_room == victim->in_room) {
		if( affected_by_spell(victim, SPELL_INFRAVISION) )
			if( aff_level(victim, AFF_INFRAVISION) <= level)
				affect_from_char(victim, SPELL_INFRAVISION);
		if( affected_by_spell(victim, SPELL_DETECT_INVIS) )
			if( aff_level(victim, AFF_DETECT_INVIS) <= level )
				affect_from_char(victim, SPELL_DETECT_INVIS);

		  act("Your eyes darken.", TRUE, ch, 0, victim, TO_VICT);
		  act("$N's eyes turn black.", TRUE, ch, 0, victim, TO_NOTVICT);
		  if (!affected_by_spell(victim, SPELL_INFRAVISION))
			  add_affect(ch, victim, SPELL_INFRAVISION, level, APPLY_NONE, NO_MOD, maxLvl24 TICKS, AFF_INFRAVISION, FALSE, FALSE, FALSE, FALSE);
		  if (!affected_by_spell(victim, SPELL_DETECT_INVIS))
			  add_affect(ch, victim, SPELL_DETECT_INVIS, level, APPLY_NONE, NO_MOD, maxLvl24 TICKS, AFF_DETECT_INVIS, FALSE, FALSE, FALSE, FALSE);
		  break;
	  }
	  else 
          {
              if ( (!IS_LIGHT(victim->in_room) && !IS_SET_AR(ROOM_FLAGS(victim->in_room), ROOM_CLAN) &&
                      GET_LEVEL(victim) < LVL_IMMORT) || IS_HUNTED (victim))
              {
                  act( "You peer into the shadows and see:", FALSE, ch, NULL, victim, TO_CHAR );

                  int temp_room;
                  temp_room = ch->in_room;
                  ch->in_room = victim->in_room;
                  look_at_room(ch, 1);
                  act( "You get the feeling you are being watched.", TRUE, ch, NULL, victim, TO_ROOM );
                  ch->in_room = temp_room;
              }
              else {
                  act( "You are dazzled with radiance and can make out nothing.",
                  FALSE, ch, NULL, victim, TO_CHAR );
              }
              break;
          }
  case SPELL_SHADOW_SPHERE:
        EXIT_IF_ALREADY_AFFECTED;
        if(!IS_NPC(ch) && savetype == SAVING_SPELL)
            add_affect( ch, victim, spellnum, level, APPLY_NONE, NO_MOD, -1,
                    AFF_SHADOW_SPHERE, FALSE, FALSE, FALSE, FALSE );
        else
            add_affect( ch, victim, spellnum, level, APPLY_NONE, NO_MOD, (level / 2) TICKS,
                    AFF_SHADOW_SPHERE, FALSE, FALSE, FALSE, FALSE );

	act("A globe of darkness surrounds you.", TRUE, ch, 0, victim, TO_VICT);
	act("$N is surrounded by a globe of darkness.", TRUE, ch, 0, victim, TO_ROOM);
	break;

    case SPELL_PLAGUE:
        if( !magic_savingthrow(ch, victim, savetype) ){
            add_affect( ch, victim, SPELL_PLAGUE, level, APPLY_STR, -2, 48 TICKS, AFF_PLAGUE, FALSE, FALSE, FALSE, FALSE );
            sendChar( victim, "You feel very sick.\r\n" );
            act( "$N gets violently ill!", TRUE, ch, NULL, victim, TO_NOTVICT );
        }
        break;

  case SPELL_BLACK_BREATH:   /* Exactly the same as plague - Vex */
      if (IS_DEFILER(ch) && GET_ADVANCE_LEVEL(ch) >= 2 && !magic_savingthrow(ch, victim, savetype)) {
          add_affect( ch, victim, SPELL_CONFUSION, level, 0, 0, 1 TICKS, NO_BIT, FALSE, FALSE, FALSE, FALSE );
          send_to_char("You feel disoriented and confused.\r\n", victim);
          act("$N is disoriented.", TRUE, ch, NULL, victim, TO_NOTVICT);
      }
      if (!magic_savingthrow(ch, victim, savetype)) {
          add_affect( ch, victim, SPELL_PLAGUE, level, APPLY_STR, -2, 48 TICKS, AFF_PLAGUE, FALSE, FALSE, FALSE, FALSE );
          send_to_char("You feel very sick.\r\n", victim);
          act("$N gets violently ill!", TRUE, ch, NULL, victim, TO_NOTVICT);
      }
      break;

  case SPELL_WEB: EXIT_IF_ALREADY_AFFECTED;
      if (!magic_savingthrow(ch, victim, savetype))
      {
          add_affect( ch, victim, spellnum, level, APPLY_DEX, (((level/10) * -1) - 1), 2 TICKS, AFF_WEB, FALSE, FALSE, FALSE, FALSE );
          GET_MOVE(victim) -= (GET_MOVE(victim)/2);
          send_to_char("Pale white tendrils of webbing wrap themselves around you!\r\n", victim);
          send_to_char("Your tendrils of webbing do their work.\r\n", ch);
          act("Pale white tendrils of webbing wrap themselves around $N!\r\n", TRUE, ch, NULL, victim, TO_NOTVICT);
          set_fighting(victim, ch);
          set_fighting(ch, victim);
      }
      else {
          send_to_char("White tendrils of webbing hurl through the air, but miss you.\r\n", victim);
          sendChar(ch, "You fail to entangle %s in your web.\r\n", GET_NAME(victim));
          act("Pale white tendrils fly across the room, but miss $N.\r\n", TRUE, ch, NULL, victim, TO_NOTVICT);
      }
      break;

  case SPELL_DEATH_TOUCH:
    if (!magic_savingthrow(ch, victim, savetype))
    {
      add_affect( ch, victim, spellnum, level, APPLY_AGE, 100, 6 TICKS, NO_BIT, TRUE, FALSE, FALSE, FALSE );
      send_to_char("You feel old.\r\n", victim);
      act("$N looks much older!", TRUE, ch, NULL, victim, TO_NOTVICT);
    }
    break;

  case SPELL_BLUR:
        REMOVE_WEAK_PROTECTION(victim)
	REMOVE_STRONG_PROTECTION(victim)
    if( affected_by_spell(victim, SPELL_BLUR)) {
        send_to_char( "If your form blurred any more, you'd fade from existence.\r\n", victim );
    }
    else {
	if (!IS_NPC(ch) && (savetype == SAVING_SPELL) )
		add_affect( ch, victim, spellnum, level, APPLY_AC, -40,
		  (1 + GET_LEVEL(ch)/5) TICKS, NO_BIT, TRUE, FALSE, FALSE, FALSE);
	else
		add_affect( ch, victim, SPELL_ARMOR, level, APPLY_AC, -20,
		  (1 + GET_LEVEL(ch)/5) TICKS, NO_BIT, TRUE, FALSE, FALSE, FALSE);
	// If you cast it, you get -40 armor, if you don't, -20 armor.

	send_to_char("You become very blurry and hard to see.\r\n", victim);
        act("$n's form wavers and becomes hard to see.", TRUE, victim, 0, 0, TO_ROOM);
    }
    break;

  case SPELL_SILENCE:
    EXIT_IF_ALREADY_AFFECTED;

    if(magic_savingthrow(ch, victim, savetype)) {
       send_to_char("You fail.\r\n", ch);
       return;
    }
    act( "$n becomes very silent.", TRUE, victim, 0, 0, TO_ROOM);
    send_to_char( "You feel at a loss for words.\r\n", victim);
    add_affect(ch, victim, spellnum, level, NO_APPLY, NO_MOD, 2 TICKS, AFF_SILENCE, FALSE, FALSE, FALSE, FALSE);
    break;

  case SPELL_NO_HOT:
     EXIT_IF_ALREADY_AFFECTED;
     if (victim !=ch)
         act("$N is surrounded by a shield of &11ice&00.", TRUE, ch, 0, victim, TO_CHAR);

     act("You are surrounded by a shield of &11ice&00.", TRUE, ch, 0, victim, TO_VICT);
     add_affect( ch, victim, spellnum, level, NO_APPLY, NO_MOD, 5 TICKS, AFF_NO_HOT, FALSE, FALSE, FALSE, FALSE);
        break;

  case SPELL_NO_DRY:
     EXIT_IF_ALREADY_AFFECTED;
     if (victim !=ch)
         act("$N is surrounded by a shield of &04water&00.",
	      TRUE, ch, 0, victim, TO_CHAR);

     act("You are surrounded by a shield of &04water&00.",
          TRUE, ch, 0, victim, TO_VICT);

     add_affect(ch, victim, spellnum, level, NO_APPLY, NO_MOD, 5 TICKS, AFF_NO_DRY,
	        FALSE, FALSE, FALSE, FALSE);
     break;

  case SPELL_NO_COLD:
     EXIT_IF_ALREADY_AFFECTED;
     if (victim !=ch)
         act("$N is surrounded by a shield of &08fire&00.",
              TRUE, ch, 0, victim, TO_CHAR);

     act("You are surrounded by a shield of &08fire&00.",
          TRUE, ch, 0, victim, TO_VICT);
     add_affect(ch, victim, spellnum, level, NO_APPLY, NO_MOD, 5 TICKS, AFF_NO_COLD,
                FALSE, FALSE, FALSE, FALSE);
     break;

  case SPELL_BLINK:
     act("You feel a little &25&12jumpy&00.",
       TRUE, ch, 0, victim, TO_CHAR);
     affect_from_char(ch, SPELL_BLINK);
     if (GET_HITROLL(ch) >= 5) {
     add_affect( ch, victim, spellnum, level, APPLY_HITROLL, -3, 4 TICKS, AFF_BLINK,
       TRUE, FALSE, FALSE, FALSE);}
     else if (GET_HITROLL(ch) < 5) {
     add_affect(ch, victim, spellnum, level, NO_APPLY, NO_MOD, 4 TICKS, AFF_BLINK,
       TRUE, FALSE, FALSE, FALSE);}
     break;

  case SPELL_ASSISTANT:
     if(!(IS_DARK_PRIEST(ch) && GET_ADVANCE_LEVEL(ch) >= 2))
          EXIT_IF_ALREADY_AFFECTED;
     act("You feel as if someone was watching over you.",
       TRUE,ch, 0, victim, TO_CHAR);
     affect_from_char(ch, SPELL_ASSISTANT);
     add_affect(ch, victim, spellnum, level, NO_APPLY, NO_MOD, 10 TICKS, AFF_ASSISTANT,
         TRUE, FALSE, FALSE, FALSE);
     GET_ASSIST_HP(ch) = level * 50;
     break;

  case SPELL_PULSE_HEAL:
     EXIT_IF_ALREADY_AFFECTED;

     act("Raw healing power courses through you!", FALSE, ch, 0, victim,
         TO_VICT);
     act("$N glows with raw healing power!", FALSE, ch, 0, victim, TO_NOTVICT);
     act("You infuse $N with raw healing power!", FALSE, ch, 0, victim,
         TO_CHAR);

      add_affect( ch, victim, spellnum, level, APPLY_NONE, 0, 7 TICKS,
          AFF_PULSE_HIT, FALSE, FALSE, FALSE, FALSE);
      break;

  case SPELL_PULSE_GAIN:
     EXIT_IF_ALREADY_AFFECTED;

     act("Raw magical power courses through you!", FALSE, ch, 0, victim,
         TO_VICT);
     act("$N glows with raw magical power!", FALSE, ch, 0, victim, TO_NOTVICT);
     act("You infuse $N with raw magical power!", FALSE, ch, 0, victim,
         TO_CHAR);

      add_affect( ch, victim, spellnum, level, APPLY_NONE, 0, 7 TICKS,
          AFF_PULSE_MANA, FALSE, FALSE, FALSE, FALSE);
      break;

  case SPELL_DISHEARTEN:
     EXIT_IF_ALREADY_AFFECTED;

    if(magic_savingthrow(ch, victim, savetype)) {
		set_fighting(victim, ch);
		set_fighting(ch, victim);
		send_to_char("You fail.\r\n", ch);
		return;
	}

    act("You are disheartened by $n's power.", FALSE, ch, 0, victim, TO_VICT);
    act("$N is disheartened by $n's power.", FALSE, ch, 0, victim, TO_NOTVICT);
    act("You dishearten $N.", FALSE, ch, 0, victim, TO_CHAR);
    {
      int amount = -8;
      if (IS_EVIL(ch)) {
        if (IS_EVIL(victim)) amount >>= 1;
      } else if (IS_GOOD(ch)) {
        if (IS_GOOD(victim)) amount >>= 1;
      } else {
        if (!IS_EVIL(victim) && !IS_GOOD(victim)) amount >>= 1;
      }

      add_affect( ch, victim, spellnum, level, APPLY_DAMROLL, amount, 10 TICKS,
          NO_BIT, FALSE, FALSE, FALSE, FALSE);
    }
    set_fighting(victim, ch);
    set_fighting(ch, victim);

    break;
  case SPELL_FORTIFY:
     if (ch->fortifiee) {
       act("You are already fortifying $N!", TRUE, ch, 0, ch->fortifiee,
           TO_CHAR);
       break;
     }

     if (victim->fortifier) {
       act("$N is already being fortified!", TRUE, ch, 0, victim, TO_CHAR);
       break;
     }

     if(ch != victim) {
         act("Your gods extend their protection to $N.", TRUE, ch, 0, victim, TO_CHAR);
         act("You are protected by $N's gods!", TRUE, victim, 0, ch, TO_CHAR);
     }
     else
         act("Your gods extend their protection to you.", TRUE, ch, 0, victim, TO_CHAR);

     add_affect(ch, victim, spellnum, level, NO_APPLY, NO_MOD, (GET_LEVEL(ch) / 7) TICKS,
         AFF_FORTIFY, FALSE, FALSE, FALSE, FALSE);
     ch->fortifiee = victim;
     victim->fortifier = ch;
     break;

  case SPELL_SAGACITY:
      act("You feel very wise.", TRUE,ch, 0, victim, TO_VICT);
      affect_from_char(victim, SPELL_SAGACITY);
      affect_from_char(victim, SPELL_SAGACITY2);
      add_affect( ch, victim, SPELL_SAGACITY, level, APPLY_WIS, 2, level TICKS, NO_BIT, FALSE, FALSE, FALSE, FALSE );
      add_affect( ch, victim, SPELL_SAGACITY2, level, APPLY_INT, 2, level TICKS, NO_BIT, FALSE, FALSE, FALSE, FALSE );
      break;

  case SPELL_CRUSADE:
      if (!IS_GOOD(ch)) {
          send_to_char("Your deity does not feel particularly inclined to perform this task.\r\n", ch);
          return;
      }
      affect_from_char(victim, SPELL_CRUSADE);
      affect_from_char(victim, SPELL_CRUSADE2);
      add_affect( ch, victim, SPELL_CRUSADE, level, APPLY_MANA, level, 4 TICKS, AFF_CRUSADE, FALSE, FALSE, FALSE, FALSE );
      add_affect( ch, victim, SPELL_CRUSADE2, level, APPLY_AC, -30, 4 TICKS, AFF_WARD, FALSE, FALSE, FALSE, FALSE );
      send_to_char("You take heart in your righteous crusade.\r\n", victim);
      break;

  case SPELL_APOCALYPSE:
     if (!IS_EVIL(ch)) {
         send_to_char("Your deity does not feel particularly inclined to perform this task.\r\n", ch);
         return;
     }
     affect_from_char(ch, SPELL_APOCALYPSE);
     add_affect(ch, victim, spellnum, level, APPLY_AC, -25, 4 TICKS,
             AFF_APOCALYPSE, FALSE, FALSE, FALSE, FALSE);
     send_to_char("A chill vision of terror settles around you.\r\n", victim);
     break;

  case SPELL_MISSION:
     if (!IS_GOOD(ch)) {
         send_to_char("Your deity does not feel particularly inclined to perform this task.\r\n", ch);
         return;
     }
     affect_from_char(ch, SPELL_MISSION);
     add_affect(ch, victim, spellnum, level, APPLY_AC, -25, 4 TICKS,
             AFF_MISSION, FALSE, FALSE, FALSE, FALSE);
     send_to_char("You shine by the glory of your chosen deity.\r\n", victim);
     break;
  case SPELL_FOREST_LORE:
     EXIT_IF_ALREADY_AFFECTED;
     add_affect(ch, victim, spellnum, level, APPLY_CON, (level >= 35) ? 2 : 1,
             level TICKS, AFF_LORE, FALSE, FALSE, FALSE, FALSE);
     send_to_char("Your forest lore teaches you well.\r\n", victim);
     break;

        case SPELL_FEEBLEMIND:
            EXIT_IF_ALREADY_AFFECTED;
            if(magic_savingthrow(ch, victim, savetype)) {
                send_to_char("You fail.\r\n", ch);
                return;
            }
            act("$n's eyes glaze over as you drain their intelligence away.",
                    FALSE, victim, 0, 0, TO_CHAR);
            act("$n's eyes glaze over, and starts to drool feebly.",
                    TRUE, victim, 0, 0, TO_ROOM);
            send_to_char("You feel your intelligence begin to drain away.\r\n",
                    victim);
            add_affect( ch, victim, spellnum, level, APPLY_INT,
                    ((GET_INT(victim)/2) * -1), 4 TICKS, AFF_FEEBLE,
                    TRUE, FALSE, FALSE, FALSE);
            break;

        case SPELL_ENTANGLE:
            EXIT_IF_ALREADY_AFFECTED;
            if (magic_savingthrow(ch, victim, savetype) || percentSuccess(15)) return;
            act("You scream as $n's entanglement traps your legs.",
                    FALSE, ch, 0, victim, TO_VICT);
            act("$N screams as $n's entanglement traps $M.",
                    FALSE, ch, 0, victim, TO_NOTVICT);
            act("$N screams as your entanglement traps $M.",
                    FALSE, ch, 0, victim, TO_CHAR);
            add_affect(ch, victim, spellnum, level, NO_APPLY, NO_MOD, 1 TICKS,
                    AFF_HAMSTRUNG, FALSE, FALSE, FALSE, FALSE);
            break;

        case SPELL_SWARM:
            EXIT_IF_ALREADY_AFFECTED;
            if (magic_savingthrow(ch, victim, savetype)) return;
            act("Your sting wounds swell up and start to burn.",
                    FALSE, ch, 0, victim, TO_VICT);
            act("$N's sting wounds swell up and turn a nasty red colour.",
                    FALSE, ch, 0, victim, TO_NOTVICT);
            act("$N's sting wounds look swollen and very uncomfortable.",
                    FALSE, ch, 0, victim, TO_CHAR);
            add_affect(ch, victim, spellnum, level, NO_APPLY, NO_MOD, 3 TICKS,
                    AFF_POISON, FALSE, FALSE, FALSE, FALSE);
            break;

        case SPELL_FAST_LEARNING:
            affect_from_char(victim, SKILL_SHOWDAM);
            affect_from_char(victim, SPELL_FAST_LEARNING);
            act("A soft blue aura surrounds $n's head.", TRUE, victim, 0, 0,
                    TO_ROOM);
            sendChar(victim, "You suddenly gain a clearer understanding "
                    "of the world.\r\n");

            add_affect(ch, victim, spellnum, level, NO_APPLY, NO_MOD,
                    8 TICKS, AFF_LEARNING, TRUE, FALSE, FALSE, FALSE);
            break;
        case SKILL_SHOWDAM:
            affect_from_char(victim, SPELL_FAST_LEARNING);
            act("A dark red aura surrounds $n's head.", TRUE, victim, 0, 0, TO_ROOM);
            sendChar(victim, "You suddenly gain a keener understanding of combat.\r\n");
            affect_from_char(ch, SKILL_SHOWDAM);;
            add_affect(ch, victim, spellnum, level, NO_APPLY, NO_MOD,
                       25 TICKS, 0, TRUE, FALSE, FALSE, FALSE);
            break;
        case SPELL_DISPEL_MAGIC:
            if (magic_savingthrow(ch, victim, savetype)) return;
            debuff_protections(ch, victim);
			//random_dispel(victim);
            break;
	case SPELL_FLAME_BLADE:
            if(WIELDING(victim)) {
                act("$p is surrounded by fire.", FALSE, victim,
                        victim->equipment[WEAR_WIELD], 0, TO_ROOM);
                act("$p is surrounded by fire.", FALSE, victim,
                        victim->equipment[WEAR_WIELD], 0, TO_CHAR);
            } else {
                act("Your fists are surrounded by fire.", FALSE, victim,
                        0, 0, TO_CHAR);
                act("$n's fists are surrounded by fire.", FALSE, victim,
                        0, 0, TO_ROOM);
            }
            affect_from_char(victim, SPELL_FLAME_BLADE);
            add_affect(ch, victim, spellnum, level, NO_APPLY, NO_MOD,
                    12 TICKS, AFF_FLAME_BLADE, FALSE, FALSE, FALSE, FALSE);
            break;
        case SPELL_WRAITHFORM:
            act("$n fades to a translucent shadow.", TRUE, victim, 0, 0,
                    TO_ROOM);
            sendChar(victim, "Your body shifts out of the physical "
                    "plane.\r\n");
            affect_from_char(victim, SPELL_WRAITHFORM);
            add_affect(ch, victim, spellnum, level, NO_APPLY, NO_MOD,
                    (level/2+4) TICKS, AFF_WRAITHFORM, TRUE, FALSE, FALSE, FALSE);
            break;
        case SPELL_NOXIOUS_SKIN:
	    if ( victim != ch &&
		( (victim->master != ch) ||
		  !IS_AFFECTED(victim, AFF_CHARM) ||
		  !affected_by_spell(victim, SPELL_CHARM_CORPSE) ) )
	    {
		sendChar(ch, "You cannot cast noxious skin on this target.\r\n");
		sendChar(victim, "Nothing happens.\r\n");
		return;
	    }
            else if(affected_by_spell(victim, SPELL_NOXIOUS_SKIN)) {
                act("The blisters on $n's skin dissipate.", TRUE, victim,
                        0, 0, TO_ROOM);
                sendChar(victim, "Your skin is cleansed of its blisters.\r\n");
                affect_from_char(victim, SPELL_NOXIOUS_SKIN);
                break;
            }

            REMOVE_WEAK_PROTECTION(victim);
	    REMOVE_STRONG_PROTECTION(victim);
            if (savetype == SAVING_SPELL)
	       add_affect( ch, victim, spellnum, level, NO_APPLY, 0,
			-1, NO_MOD, FALSE, FALSE, FALSE, FALSE);
	    else
		 add_affect( ch, victim, spellnum, level, NO_APPLY, 0,
			(level/6) TICKS, AFF_DISEASE, FALSE, FALSE, FALSE, FALSE);

            act("$n's skin erupts in hideous boils.", TRUE, victim,
		 0, 0, TO_ROOM);
            sendChar(victim, "Your skin is covered in noxious blisters.\r\n");

            break;
        case SPELL_DISEASE: EXIT_IF_ALREADY_AFFECTED;
            if( magic_savingthrow( ch, victim, savetype )) return;
            add_affect( ch, victim, spellnum, level, APPLY_STR, -2,
                    (level/10) TICKS, AFF_DISEASE, FALSE, FALSE, FALSE, FALSE);
            act("You feel deathly sick.", TRUE, ch, NULL, victim, TO_VICT);
            act("$N gets dangerously ill!", TRUE, ch, NULL, victim, TO_NOTVICT);
            act("$N gets dangerously ill!", TRUE, ch, NULL, victim, TO_CHAR);
            break;
        case SPELL_REFLECTION: EXIT_IF_ALREADY_AFFECTED;
            act("The air around $n begins to shimmer.", TRUE, victim, 0, 0,
                    TO_ROOM);
            sendChar(victim, "The air around you begins to shimmer.\r\n");
            add_affect(ch, victim, spellnum, level, NO_APPLY, NO_MOD, (level/2) TICKS,
                    NO_MOD, TRUE, FALSE, FALSE, FALSE);
            break;
        case SPELL_RESIST_POISON: EXIT_IF_ALREADY_AFFECTED;
            act("$n's veins briefly glow green.", TRUE, victim, 0, 0, TO_ROOM);
            sendChar(victim, "Your veins briefly glow green.\r\n");
            add_affect(ch, victim, spellnum, level, NO_APPLY, NO_MOD, (level/2) TICKS,
                    NO_MOD, TRUE, FALSE, FALSE, FALSE);
            break;
        case SPELL_QUICKEN:
            if(affected_by_spell(victim, SPELL_QUICKEN)) {
                sendChar(victim, "You dispel the protective runes from the air.\r\n");
                affect_from_char(victim, SPELL_QUICKEN);
                break;
            }
            if( COOLDOWN(ch, SLOT_QUICKEN) || savetype != SAVING_SPELL || GET_CLASS(ch) != CLASS_NECROMANCER) {
                sendChar(ch, "You are not ready to prevent yourself from death.\r\n");
                return;
            }
            act("$n draws runes in the air.", FALSE, ch, 0, victim, TO_NOTVICT);
            sendChar(ch, "You draw runes in the air to prevent your death.\r\n");
            add_affect(ch, victim, spellnum, level, NO_APPLY, NO_MOD, -1,
                    NO_MOD, TRUE, FALSE, FALSE, FALSE);
            break;
        case SPELL_AGE:
            if (affected_by_spell(victim, spellnum) >= 20)
                EXIT_IF_ALREADY_AFFECTED;
            add_affect(ch, victim, spellnum, level, APPLY_AGE, 5, (level/2) TICKS,
                    NO_MOD, TRUE, TRUE, TRUE, FALSE);
            send_to_char("You feel a little closer to the grave.\r\n", victim);
            break;
        case SPELL_ENERGY_DRAIN: EXIT_IF_ALREADY_AFFECTED;
            if ( !magic_savingthrow(ch, victim, savetype) ||
               number(1, 25) > MIN(24, (GET_INT(victim) + 5*(IS_IMMORTAL(victim))) ))
	    {
                act("$n looks wearier.", TRUE, victim, 0, 0, TO_ROOM);
                sendChar(victim, "You find it harder to concentrate.\r\n");
                add_affect(ch, victim, spellnum, level, APPLY_INT, -1, (level/5) TICKS,
                        NO_MOD, TRUE, FALSE, FALSE, FALSE);
            }
            else
            {
                send_to_char("You fail.\r\n", ch);
                return;
            }
            break;
	case SPELL_CALL_TO_CORPSE: EXIT_IF_ALREADY_AFFECTED;
           act("$n calls you to the grave.",
                   FALSE, ch, 0, victim, TO_VICT);
           act("$N screams as $n calls $M to the grave.",
                   FALSE, ch, 0, victim, TO_NOTVICT);
           act("$N screams as you call $M to the grave.",
                   FALSE, ch, 0, victim, TO_CHAR);
           add_affect( ch, victim, spellnum, level, NO_APPLY, NO_MOD, 20,
			NO_MOD, TRUE, FALSE, FALSE, FALSE);
		damage(ch, victim, 0, SPELL_CALL_TO_CORPSE);
		victim->call_to_corpse = 5;
		break;

        case SPELL_DEBILITATE:
            if(magic_savingthrow(ch, victim, savetype)) {
                send_to_char("You fail.\r\n", ch);
                return;
            }
            act("$n's skin takes on a gray hue.", TRUE, victim, 0, 0, TO_ROOM);
            sendChar(victim, "You feel horribly weakened.\r\n");
            add_affect(ch, victim, spellnum, level, APPLY_STR, -3, 4 TICKS,
                    NO_MOD, TRUE, FALSE, FALSE, FALSE);
            break;
        case SKILL_BATTLECRY: EXIT_IF_ALREADY_AFFECTED;
            sendChar(victim, "Your blood sings to you of battle!\r\n");
            add_affect(ch, victim, spellnum, level, APPLY_DAMROLL, level/10,
                    2 TICKS, NO_MOD, FALSE, FALSE, FALSE, FALSE);
            break;
    }
}


void mag_group_switch(int level, CharData * ch, CharData * tch,
		           int spellnum, int savetype)
{
  switch (spellnum) {
  case SPELL_HEAL:
  case SPELL_GROUP_HEAL:
    mag_points(level, ch, tch, SPELL_HEAL, savetype);
    break;
  case SPELL_CURE_CRITIC:
    mag_points(level, ch, tch, SPELL_CURE_CRITIC, savetype);
    break;
  case SPELL_CURE_LIGHT:
    mag_points(level, ch, tch, SPELL_CURE_LIGHT, savetype);
    break;
  case SPELL_ARMOR:
  case SPELL_GROUP_ARMOR:
    mag_affects(level, ch, tch, SPELL_ARMOR, savetype);
    break;
  case SPELL_BLESS:
    mag_affects(level, ch, tch, SPELL_BLESS, savetype);
    break;
  case SPELL_CURE_BLIND:
    mag_unaffects(level, ch, tch, SPELL_CURE_BLIND, savetype);
    break;
  case SPELL_DETECT_ALIGN:
    mag_affects(level, ch, tch, SPELL_DETECT_ALIGN, savetype);
    break;
  case SPELL_DETECT_INVIS:
    mag_affects(level, ch, tch, SPELL_DETECT_INVIS, savetype);
    break;
  case SPELL_STRENGTH:
    mag_affects(level, ch, tch, SPELL_STRENGTH, savetype);
    break;
  case SPELL_AWAKEN:
    mag_affects(level, ch, tch, SPELL_AWAKEN, savetype);
    break;
  case SPELL_REGENERATE:
    mag_affects(level, ch, tch, SPELL_REGENERATE, savetype);
    break;
  case SPELL_SHIELD:
    mag_affects(level, ch, tch, SPELL_SHIELD, savetype);
    break;
  case SPELL_REMOVE_PARALYSIS:
    mag_unaffects(level, ch, tch, SPELL_REMOVE_PARALYSIS, savetype);
    break;
  case SPELL_REFRESH:
    mag_points(level, ch, tch, SPELL_REFRESH, savetype);
    break;
  case SPELL_TRUE_SIGHT:
    mag_affects(level, ch, tch, SPELL_TRUE_SIGHT, savetype);
    break;
  case SPELL_FLEET_FOOT:
    mag_affects(level, ch, tch, SPELL_FLEET_FOOT, savetype);
    break;
  case SPELL_CURE_PLAGUE:
    mag_unaffects(level, ch, tch, SPELL_CURE_PLAGUE, savetype);
    break;
  case SPELL_WORD_OF_RECALL:
  case SPELL_GROUP_RECALL:
    spell_recall(level, ch, tch, NULL, NULL, CAST_SPELL);
    break;
  case SPELL_SANCTUARY:
  case SPELL_GROUP_SANCTUARY:
    mag_affects(level, ch, tch, SPELL_SANCTUARY, savetype);
    break;
  case SPELL_WARD:
  case SPELL_GROUP_WARD:
    mag_affects(level, ch, tch, SPELL_WARD, savetype);
    break;
  case SPELL_SHADOW_VISION:
  case SPELL_GROUP_SHADOW_VISION:
    mag_affects(level, ch, tch, SPELL_SHADOW_VISION, savetype);
    break;
  case SPELL_FLY:
  case SPELL_GROUP_FLY:
	mag_affects(level, ch, tch, SPELL_FLY, savetype);
	break;
  case SPELL_AIRSPHERE:
        mag_affects(level, ch, tch, SPELL_AIRSPHERE, savetype);
	break;
  case SPELL_INVISIBLE:
  case SPELL_GROUP_INVISIBILITY:
	mag_affects(level, ch, tch, SPELL_INVISIBLE, savetype);
	break;
  case SPELL_HASTE:
  case SPELL_GROUP_HASTE:
    mag_affects(level, ch, tch, SPELL_HASTE, savetype);
    break;
  case SPELL_CLEANSE:
    spell_cleanse(level, ch, tch, 0, 0, savetype);
    break;
  case SPELL_FLAME_BLADE:
    mag_affects(level, ch, tch, SPELL_FLAME_BLADE, savetype);
    break;
  case SPELL_RESIST_POISON:
    mag_affects(level, ch, tch, SPELL_RESIST_POISON, savetype);
    break;
  case SPELL_NOXIOUS_SKIN:
    mag_affects(level, ch, tch, SPELL_NOXIOUS_SKIN, savetype);
    break;
  case SKILL_BATTLECRY: /* shh! it is a spell! shh! */
    mag_affects(level, ch, tch, SKILL_BATTLECRY, savetype);
    break;
  default:
      mudlog (NRM, LVL_IMMORT, TRUE, "SYSERR: Unknown spell(%d) passed to mag_group_switch.", spellnum);
  }
}

/*
  Every spell that affects the group should run through here
  mag_group_switch contains the switch statement to send us to the right
  magic.

  group spells affect everyone grouped with the caster who is in the room.
*/

void mag_groups(int level, CharData * ch, int spellnum, int savetype)
{
    CharData *tch, *leader;
    struct follow_type *f, *f_next;

    if( ch == NULL ) return;
    if( !IS_AFFECTED(ch, AFF_GROUP)) return;

    leader = ( ch->master != NULL ? ch->master : ch ); /* Get the leader */

    for( f = leader->followers; f; f = f_next  ){
        f_next = f->next;
        tch    = f->follower;
        if( tch->in_room != ch->in_room ) continue;
        if( !IS_AFFECTED(tch, AFF_GROUP)) continue;
        if( tch != ch )  /* Affect the caster LAST */
            mag_group_switch(level, ch, tch, spellnum, savetype);
        /* affect mounts too, please */
        if (tch->mount && tch->mount->in_room == tch->in_room)
            mag_group_switch(level, ch, tch->mount, spellnum, savetype);
    }

    /* Digger - bug in remote heals. */
    if(( leader != ch ) && ( leader->in_room == ch->in_room )) {
        mag_group_switch(level, ch, leader, spellnum, savetype);
        if (leader->mount)
            mag_group_switch(level, ch, leader->mount, spellnum, savetype);
    }

    /* Finally ... cleric, heal thine self. */
    mag_group_switch(level, ch, ch, spellnum, savetype);

    /* and thy mount */
    if (ch->mount) mag_group_switch(level, ch, ch->mount, spellnum, savetype);
}


/*
  mass spells affect every creature in the room except the caster.
*/

void mag_masses(int level, CharData * ch, int spellnum, int savetype)
{
  CharData *tch, *tch_next;

  for (tch = world[ch->in_room].people; tch; tch = tch_next) {
    tch_next = tch->next_in_room;
    if (tch == ch)
      continue;

    switch (spellnum) {
    }
  }
}


/*
  Every spell that affects an area (room) runs through here.  These are
  generally offensive spells.  This calls mag_damage to do the actual
  damage.
  area spells have limited targets within the room.
*/

void mag_areas(int level, CharData * ch, int spellnum, int savetype)
{
    CharData *tch, *next_tch;
    int skip, trip;

    if (ch == NULL) return;

    switch( spellnum ) {
        case SPELL_EARTHQUAKE:
            act("The ground shakes and shivers!", FALSE, ch, 0, 0, TO_CHAR);
            act("The ground shakes and shivers!", FALSE, ch, 0, 0, TO_ROOM);
            break;
        case SPELL_TREMOR:
            act("The ground bucks and heaves!", FALSE, ch, 0, 0, TO_CHAR);
            act("The ground bucks and heaves!", FALSE, ch, 0, 0, TO_ROOM);
            break;
        case SPELL_WALL_OF_FIRE:
            act("A wall of fire races across the ground!",
                    FALSE, ch, 0, 0, TO_CHAR);
            act("A wall of fire races across the ground!",
                    FALSE, ch, 0, 0, TO_ROOM);
            break;
        case SPELL_TYPHOON:
            act("A sudden storm churns the air violently!",
                    FALSE, ch, 0, 0, TO_CHAR);
            act("A sudden storm churns the air violently!",
                    FALSE, ch, 0, 0, TO_ROOM);
            break;
        case SPELL_TSUNAMI:
            act("A wall of water races across the ground!",
                    FALSE, ch, 0, 0, TO_CHAR);
            act("A wall of water races across the ground!",
                    FALSE, ch, 0, 0, TO_ROOM);
            break;
        case SPELL_ICE_STORM:
            act("You call down a hail of ice and snow!", FALSE, ch, 0, 0, TO_CHAR);
            act("$n calls down a hail of ice and snow!", FALSE, ch, 0, 0, TO_ROOM);
            break;
        case SPELL_METEOR_SWARM:
            act("You summon forth a swarm of flaming meteors!", FALSE, ch, 0, 0, TO_CHAR);
            act("$n summons forth a swarm of flaming meteors!", FALSE, ch, 0, 0, TO_ROOM);
            break;
        case SPELL_CHAIN_LIGHTNING:
            act("You conjure wicked forks of lightning which snake wildly in all directions!", FALSE, ch, 0, 0, TO_CHAR);
            act("$n conjures wicked forks of lightning which snake wildly in all directions!", FALSE, ch, 0, 0, TO_ROOM);
            break;
        case SPELL_WRATH_OF_THE_ANCIENTS:
            act("Tendrils of purple energy snake forth from your person to destroy your foes!", FALSE, ch, 0, 0, TO_CHAR);
            act("Tendrils of purple energy snake forth from $n's body to touch $s enemies!", FALSE, ch, 0, 0, TO_ROOM);
            break;
        case SPELL_HOLY_WORD:
            if( IS_GOOD(ch) ){
                act("You speak the word that is the bane of evil!", FALSE, ch, 0, 0, TO_CHAR);
                act("$n speaks the word that is the bane of evil!", FALSE, ch, 0, 0, TO_ROOM);
            }
            else {
                act("You must be pure of heart to speak the holy word!", FALSE, ch, 0, 0, TO_CHAR);
                return;
            }
            break;
        case SPELL_UNHOLY_WORD:
            if( IS_EVIL(ch) ){
                act("You speak the word that is the bane of good!", FALSE, ch, 0, 0, TO_CHAR);
                act("$n speaks the word that is the bane of good!", FALSE, ch, 0, 0, TO_ROOM);
            }
            else {
                act("The gods of evil sneer at you in contempt.", FALSE, ch, 0, 0, TO_CHAR);
                return;
            }
            break;
        case SPELL_FIRE_BREATH:
            act("You belch forth a roiling ball of fire!", FALSE, ch, 0, 0, TO_CHAR);
            act("$n scorches you with fire!", FALSE, ch, 0, 0, TO_ROOM);
            break;
        case SPELL_FROST_BREATH:
            act("You spray the room with shard of ice!", FALSE, ch, 0, 0, TO_CHAR);
            act("$n blasts you with shards of ice!", FALSE, ch, 0, 0, TO_ROOM);
            break;
        case SPELL_ACID_BREATH:
            act("You spray a stream of vile acid about the room!", FALSE, ch, 0, 0, TO_CHAR);
            act("$n's acid breath BURNS!", FALSE, ch, 0, 0, TO_ROOM);
            break;
        case SPELL_GAS_BREATH:
            act("You spew forth a noxious cloud that fills the room!", FALSE, ch, 0, 0, TO_CHAR);
            act("$n's green cloud of gas causes you to gasp and wheeze for air!", FALSE, ch, 0, 0, TO_ROOM);
            break;
        case SPELL_LIGHTNING_BREATH:
            act("A large bolt of electricity explodes from your mouth!", FALSE, ch, 0, 0, TO_CHAR);
            act("$n's bolt of electricity causes your hair to stand on end!", FALSE, ch, 0, 0, TO_ROOM);
            break;
    }

    for( tch = world[ch->in_room].people; tch; tch = next_tch ){
        next_tch = tch->next_in_room;

        skip = 0;
        /*
        ** Make certain mobs that summon mobs don't attack them.
        */
#ifdef VEX
        if( IS_NPC(ch) && IS_NPC(tch) && IS_AFFECTED(tch, AFF_CHARM)){
            if( tch->master && (tch->master == ch))
                skip = 1;
        }
#endif
	/*
	** Vex Febuary 12 1997
	** Stop mobiles blasting friendly mobiles all together.
	*/
	if (IS_NPC(ch) && IS_NPC(tch)
	   && !(IS_AFFECTED(ch, AFF_CHARM)       /* Could be player mob */
	   && ch->master && !IS_NPC(ch->master)))
	{
	    if (FIGHTING(tch) && (FIGHTING(tch) == ch))
		skip = 0;
	    else if (IS_AFFECTED(tch, AFF_CHARM)
		    && tch->master && !IS_NPC(tch->master))
		skip = 0;
	    else
		skip = 1;
	}

        /* but mounts need to be blasted! */
        if (IS_NPC(ch) && ch->rider && !IS_NPC(ch->rider)) skip = 0;

	if (tch == ch) skip = 1;

      if (!IS_NPC (tch) && GET_LEVEL (tch) >= LVL_IMMORT) skip = 1;

        /*  Jan 25, 1995 Liam
        **
        **  Changed it so that damage spells damage everyone not in your group
        **
        **  Dec 31, 1996 Digger
        **
        **  The following if statement used to have !IS_NPC(ch) at the end but
        **  since Mu mobs can summon and gate I removed it.
        */
//        if( IS_AFFECTED(tch, AFF_GROUP) && IS_AFFECTED(ch, AFF_GROUP) ) {
//            /* Skip the group leader */
//            if( tch->master && (tch->master == ch) ) skip = 1;
//            /* Skip the group members */
//            if( tch->master && ch->master && (ch->master == tch->master) ) skip = 1;
//            /* Skip the group followers */
//           if( ch->master  && (ch->master == tch ) ) skip = 1;
//        }
        if(chGroupedWithTarget(tch, ch))
            skip = 1;
        /* Stop charmed creatures blasting their _masters_ group */
        else if( IS_AFFECTED(tch, AFF_GROUP)
	    && IS_AFFECTED(ch, AFF_CHARM)
	    && (ch->master))
	{
            /* Skip the group leader */
            if( tch->master && (tch->master == ch->master) ) skip = 1;
            /* Skip the group members */
            if( tch->master && ch->master->master && (ch->master->master == tch->master) ) skip = 1;
            /* Skip the group followers */
            if( ch->master->master  && (ch->master->master == tch ) ) skip = 1;
        }
        // Area affect spells don't hit teammates
        if (PRF_FLAGGED(ch, PRF_GOLD_TEAM) && PRF_FLAGGED(tch, PRF_GOLD_TEAM)) skip = 1;
        if (PRF_FLAGGED(ch, PRF_BLACK_TEAM) && PRF_FLAGGED(tch, PRF_BLACK_TEAM)) skip = 1;
        if (PRF_FLAGGED(ch, PRF_ROGUE_TEAM) && PRF_FLAGGED(tch, PRF_ROGUE_TEAM)) skip = 1;

        /* Area affect spells shouldn't affect mounts of groupies/teamies */
        if (IS_NPC(tch) && tch->rider) {
            if (PRF_FLAGGED(tch->rider, PRF_GOLD_TEAM) &&
                    PRF_FLAGGED(ch, PRF_GOLD_TEAM)) skip = 1;
            if (PRF_FLAGGED(tch->rider, PRF_BLACK_TEAM) &&
                    PRF_FLAGGED(ch, PRF_BLACK_TEAM)) skip = 1;
            if (PRF_FLAGGED(tch->rider, PRF_ROGUE_TEAM) &&
                    PRF_FLAGGED(ch, PRF_ROGUE_TEAM)) skip = 1;

            if (tch->rider == ch) skip = 1;

            /* Skip if grouped with the rider */
            if (tch->rider->master && (tch->rider->master == ch)) skip = 1;
            if (tch->rider->master && ch->master &&
                    (ch->master == tch->rider->master)) skip = 1;
            if (ch->master  && (ch->master == tch->rider)) skip = 1;
        }

        // Non-combatants cannot be attacked
        if (chIsNonCombatant(tch)) skip = 1;

        if (skip) continue;

        switch (spellnum) {
            case SPELL_EARTHQUAKE:
                trip = GET_LEVEL(ch) - GET_LEVEL(tch) + 2;
                if (IS_FLYING(tch)) trip -= 5;
                if (percentSuccess(trip)) {
                  act("You are thrown off your feet by $N's earthquake!",
                      FALSE, tch, 0, ch, TO_CHAR);
                  act("$n is thrown off his feet by $N's earthquake!",
                      FALSE, tch, 0, ch, TO_NOTVICT);
                  act("You throw $N off $S feet with your earthquake!",
                      FALSE, ch, 0, tch, TO_CHAR);
                  GET_POS(tch) = POS_SITTING;
                  WAIT_STATE(tch, SET_STUN(2));
                }
                mag_damage(level, ch, tch, spellnum, 1);
                break;
            case SPELL_TREMOR:
                trip = GET_LEVEL(ch) - GET_LEVEL(tch) + 7;
                if (IS_FLYING(tch)) trip -= 5;
                if (percentSuccess(trip)) {
                  act("You are thrown off your feet by $N's tremor!",
                      FALSE, tch, 0, ch, TO_CHAR);
                  act("$n is thrown off his feet by $N's tremor!",
                      FALSE, tch, 0, ch, TO_NOTVICT);
                  act("You throw $N off $S feet with your tremor!",
                      FALSE, ch, 0, tch, TO_CHAR);
                  GET_POS(tch) = POS_SITTING;
                  WAIT_STATE(tch, SET_STUN(2));
                }
                mag_damage(level, ch, tch, spellnum, 1);
                break;
            case SPELL_TSUNAMI:
                trip = GET_LEVEL(ch) - GET_LEVEL(tch) + 7;
                if (IS_FLYING(tch)) trip -= 5;
                if (percentSuccess(trip)) {
                  act("You are knocked over by $N's tsunami!",
                      FALSE, tch, 0, ch, TO_CHAR);
                  act("$n is knocked over by $N's tsunami!",
                      FALSE, tch, 0, ch, TO_NOTVICT);
                  act("You knock $N over with your tsunami!",
                      FALSE, ch, 0, tch, TO_CHAR);
                  GET_POS(tch) = POS_SITTING;
                  WAIT_STATE(tch, SET_STUN(2));
                }
                mag_damage(level, ch, tch, spellnum, 1);
                break;
            case SPELL_METEOR_SWARM:
                if( IS_ARCANIST(ch) && GET_ADVANCE_LEVEL(ch) &&
                        !magic_savingthrow(ch, tch, SAVING_SPELL) && percentSuccess(40)) {
                    WAIT_STATE(tch, SET_STUN(2));
                    act("You are knocked senseless by $N's meteor swarm!",
                            FALSE, tch, 0, ch, TO_CHAR);
                    act("$n is knocked senseless by $N's meteor swarm!",
                            FALSE, tch, 0, ch, TO_NOTVICT);
                    act("You knock $N senseless with your meteor swarm!",
                            FALSE, ch, 0, tch, TO_CHAR);
                }
            case SPELL_EXPLODE_CORPSE:
            case SPELL_WALL_OF_FIRE:
            case SPELL_WRATH_OF_THE_ANCIENTS:
            case SPELL_HOLY_WORD:
            case SPELL_UNHOLY_WORD:
            case SPELL_CHAIN_LIGHTNING:
                mag_damage(level, ch, tch, spellnum, 1);
                break;
            case SPELL_TYPHOON:
                mag_damage(level, ch, tch, spellnum, 1);
                if (IN_ROOM(tch) != -1 &&
                        !magic_savingthrow(ch, tch, SAVING_SPELL)) {
                    int dir_to_go = number(0, 5);
                    if (!CAN_GO(tch, dir_to_go)) {
                        act("You are thrown around the room!", FALSE, tch,
                                0, 0, TO_CHAR);
                        act("$n is thrown around the room!", FALSE, tch, 0,
                                0, TO_ROOM);
                        WAIT_STATE(tch, SET_STUN(2));
                    } else {
                        sprintf(buf, "You are picked up and hurled %swards!",
                                dirs[dir_to_go]);
                        act(buf, FALSE, tch, 0, 0, TO_CHAR);
                        sprintf(buf, "$n is picked up and hurled %swards!",
                                dirs[dir_to_go]);
                        act(buf, FALSE, tch, 0, 0, TO_ROOM);
                        do_simple_move(tch, dir_to_go, 0);
                    }
                }
                break;

            case SPELL_ICE_STORM:
                mag_affects(level, ch, tch, SPELL_BLINDNESS, savetype);
                mag_damage(level, ch, tch, spellnum, 1);
                break;
            case SPELL_PESTILENCE:
                mag_affects(level, ch, tch, SPELL_POISON, savetype);
                if (GET_LEVEL(ch) >= 37) {
                    mag_affects(level, ch, tch, SPELL_BLINDNESS, savetype);
                    }
                if (GET_LEVEL(ch) >= 47) {
                    mag_affects(level, ch, tch, SPELL_PLAGUE, savetype);
                }
                if (GET_LEVEL(ch) >= 51) {
                  mag_affects(level, ch, tch, SPELL_DEATH_TOUCH, savetype);
                }
                mag_damage(level, ch, tch, spellnum, 1);
                break;
            case SPELL_FIRE_BREATH:
            case SPELL_FROST_BREATH:
            case SPELL_ACID_BREATH:
            case SPELL_GAS_BREATH:
            case SPELL_LIGHTNING_BREATH:
                mag_damage(GET_LEVEL(ch), ch, tch, spellnum, 1);
                break;

            default:
                return;
        }
    }
}


/*
  Every spell which summons/gates/conjures a mob comes through here.
  ... except SPELL_CALL_STEED, which doesn't bring forth a charmed creature!

  define MOB_CLONE 69
*/

#define MOB_NONE			-1
#define MOB_AERIALSERVANT	99
#define MOB_ZOMBIE			100
#define MOB_SKELETON		110
#define MOB_GHOUL			120
#define MOB_GHAST           111
#define MOB_WIGHT           112
#define MOB_WRAITH          113
#define MOB_GHOST           114
#define MOB_SPECTRE         115
#define MOB_VAMPIRE         116
#define MOB_MONSUM_I		130
#define MOB_MONSUM_II		140
#define MOB_MONSUM_III		150
#define MOB_MONSUM_IV		160
#define MOB_MONSUM_V		170
#define MOB_GATE_I			180
#define MOB_GATE_II			190
#define MOB_GATE_III		200
#define MOB_ELEM_I          210
#define MOB_ELEM_II         220
#define MOB_ELEM_III        230
#define MOB_ELEM_IV         240
#define MOB_WILD_I          250
#define MOB_WILD_II         254
#define MOB_WILD_III        258
#define MOB_WILD_IV         262
#define MOB_WILD_V          266
#define MOB_WILD_VI         270
#define MOB_NM_SKELETON     280

typedef struct summon_struct {
	int mob_base_num;
	int choices;
	int min_level;
	int attack_lev;
	int percent_fail;
	char *summ_fail_mesg;
	char *summ_succ_mesg;
} summons;

#define SUMM_FAIL_DEF 		"You failed."
#define SUMM_SUCC_DEF 		"$N appears out of a swirling mist."

void mag_summons(int level, CharData * ch, ObjData * obj,
		      int spellnum, int savetype)
{
  CharData *mob;
  CharData *pet = NULL;
  ObjData *tobj, *next_obj;
  int num = 1;
  int i, j;
  int mob_num = 0;
  int handle_corpse = 0;
  int mag_followers = 0;
  int mag_max       = 0;
  int summons_ok = TRUE;
  int ch_level   = GET_LEVEL(ch);
  summons *summs;
  struct follow_type *f;
  int timer = -1;

  summons animate_summons[] = {
    { MOB_SKELETON, 1,  6,  6, 15,  "You fail to bring any life to it.",
                                    "$N rises up out of the earth!"               },
    { MOB_ZOMBIE,   1, 12, 12, 15,  "You fail to bring any life to it.",
                                    "$N rises up out of the earth!"               },
    { MOB_GHOUL,    1, 18, 18, 20,  "You fail to bring any life to it.",
                                    "$N rises up out of the earth!"               },
    { MOB_GHAST,    1, 25, 25, 20,  "You fail to bring any life to it.",
                                    "$N rises up out of the earth!"               },
    { MOB_WIGHT,    1,  30, 30, 25, "You fail to bring any life to it.",
                                    "$N rises up out of the earth!"               },
    { MOB_WRAITH,   1,  35, 35, 25, "You fail to summon the spirit of the dead.",
                                    "$N materializes from the nether world!"      },
    { MOB_GHOST,    1,  40, 40, 25, "You fail to summon the spirit of the dead.",
                                    "$N materializes from the nether world!"      },
    { MOB_SPECTRE,  1,  45, 45, 30, "You fail to summon the spirit of the dead.",
                                    "$N materializes from the nether world!"      },
    { MOB_VAMPIRE,  1,  50, 50, 30, "You fail to empower its blood.",
                                    "$N rises from the dead!"                     },
    { MOB_NONE,     1, -1, -1,  0,  NULL,  NULL }
  };

  summons element_summons[] = {
    { MOB_ELEM_I,       4,  30, 30, 20,  "The elements resist!", SUMM_SUCC_DEF },
    { MOB_ELEM_II,      4,  38, 33, 20,  "The elements resist!", SUMM_SUCC_DEF },
    { MOB_ELEM_III,     4,  48, 36, 20,  "The elements resist!", SUMM_SUCC_DEF },
    { MOB_NONE,         1,  -1, -1,  0,  NULL,                   NULL          }
  };

    /* Vex January 1, 1997
    ** I've altered the chances of failure slightly for Monster summon
    ** to make it a bit less harsh.
    */

    summons monster_summons[] = {
        { MOB_MONSUM_I,   5,   5,   5,  10,  SUMM_FAIL_DEF,    SUMM_SUCC_DEF },
        { MOB_MONSUM_II,  5,  15,  10,  10,  SUMM_FAIL_DEF,    SUMM_SUCC_DEF },
        { MOB_MONSUM_III, 5,  25,  15,  15,  SUMM_FAIL_DEF,    SUMM_SUCC_DEF },
        { MOB_MONSUM_IV,  5,  35,  20,  15,  SUMM_FAIL_DEF,    SUMM_SUCC_DEF },
        { MOB_MONSUM_V,   5,  45,  25,  20,  SUMM_FAIL_DEF,    SUMM_SUCC_DEF },
        { MOB_NONE,       1,  -1,  -1,   0,  NULL,             NULL }
    };

    /* Vex January 20, 1997
    ** I've collapsed gate and gate major into the one spell (gate).
    */

    summons gate_I_summons[] = {
        { MOB_GATE_I,   4, 40, 40, 20, SUMM_FAIL_DEF, SUMM_SUCC_DEF },
        { MOB_GATE_II,  4, 45, 45, 20, SUMM_FAIL_DEF, SUMM_SUCC_DEF },
        { MOB_GATE_III, 4, 50, 50, 20, SUMM_FAIL_DEF, SUMM_SUCC_DEF },
        { MOB_NONE,    1, -1, -1,  0, NULL,          NULL}
    };

    /* Vex January 1, 1997
    ** I've extended the variety of major demons that can be summoned and
    ** made the type of demons you summon progressive in a similiar fashion
    ** to monster summon.
    ** I also reduced the chance of failure from 35% to 20%, since I reckon
    ** mages ought to be fairly competent at summoning things.
    */

  summons gate_II_summons[] = {
    { MOB_GATE_II,  4, 45, 45, 20, SUMM_FAIL_DEF, SUMM_SUCC_DEF },
    { MOB_GATE_III, 4, 50, 50, 20, SUMM_FAIL_DEF, SUMM_SUCC_DEF },
	{ MOB_NONE,    1, -1, -1,  0, NULL,          NULL}
  };

    /* Vex January 13, 1997
    ** Implemented a new skill for rangers call of the wild.
    */

    summons wild_summons[] = {
        { MOB_WILD_I,   4, 22, 22, 20, SUMM_FAIL_DEF, SUMM_SUCC_DEF },
        { MOB_WILD_II,  4, 26, 26, 20, SUMM_FAIL_DEF, SUMM_SUCC_DEF },
        { MOB_WILD_III, 4, 30, 30, 25, SUMM_FAIL_DEF, SUMM_SUCC_DEF },
        { MOB_WILD_IV,  4, 35, 35, 25, SUMM_FAIL_DEF, SUMM_SUCC_DEF },
        { MOB_WILD_V,   4, 40, 40, 30, SUMM_FAIL_DEF, SUMM_SUCC_DEF },
        { MOB_WILD_VI,  4, 45, 45, 30, SUMM_FAIL_DEF, SUMM_SUCC_DEF },
        { MOB_NONE,     1, -1, -1, 0, NULL, NULL}
    };

    if (ch == NULL) return;

    mob = 0;
    tobj = 0;
    next_obj = 0;
    summs = 0;

    switch( spellnum ){
        case SPELL_ANIMATE_DEAD:
            if(( obj == NULL ) || (GET_OBJ_TYPE(obj) != ITEM_CONTAINER ) || (!GET_OBJ_VAL(obj, 3)) || (!CAN_WEAR(obj,  ITEM_WEAR_TAKE))) {
                act( SUMM_FAIL_DEF, FALSE, ch, 0, 0, TO_CHAR );
                return;
            }
            handle_corpse = 1;
            summs = animate_summons;
            timer = TIMER_ANIMATE;
            break;
        case SPELL_MONSTER_SUMMON:
            summs = monster_summons;
            timer = TIMER_MONSTER;
            break;
        case SPELL_CONJURE_ELEMENTAL:
            summs = element_summons;
            timer = TIMER_ELEMENTAL;
            break;
        case SPELL_GATE:
            summs = gate_I_summons;
            timer = TIMER_GATE;
            break;
        case SPELL_GATE_MAJOR_DEMON:
            summs = gate_II_summons;
            timer = TIMER_GATE;
            break;
        case SPELL_CALL_OF_THE_WILD:
            summs = wild_summons;
            timer = TIMER_WILD;
            break;
        default:
            return;
    }

    /* Restricted how often creatures may be summoned */
    if (spellnum != SPELL_ANIMATE_DEAD
	&& !IS_NPC(ch)
	&& GET_CONJ_CNT(ch, timer)
      && GET_LEVEL (ch) < LVL_GRGOD) /* because check_idling won't be called... */
    {
	switch (GET_CLASS(ch)) {
	case CLASS_RANGER:
	    act( "The creatures of the wild aren't responding to you right now.", FALSE, ch, 0, 0, TO_CHAR );
	    break;
        case CLASS_NECROMANCER:
            act("The dead are ignoring your call right now.", FALSE, ch, 0,
                    0, TO_CHAR);
            break;
	default:
	    act( "No conjured creature will answer your summons at the moment!", FALSE, ch, 0, 0, TO_CHAR );
	}
	return;
    }
    else if (spellnum == SPELL_ANIMATE_DEAD && weather_info.sunlight != SUN_DARK)
    {
	act( "Undead creatures may not be created while the sun shines!", FALSE, ch, 0, 0, TO_CHAR );
	return;
    }

    if (!summs) {
        send_to_char("The moons aren't in alignment.\r\n", ch);
        return;
    }

    if( IS_AFFECTED(ch, AFF_CHARM )) {
        send_to_char("You are too giddy to have any followers!\r\n", ch);
        return;
    }

    // Vex - only  _one_ mob at a time!
    mag_followers = 0;
    summons_ok = TRUE;
    f = ch->followers;
    while( f ){
        if( IS_AFFECTED( f->follower, AFF_CHARM ) && ( f->follower->master == ch )){
            mag_followers += 1;
            if(f->follower->in_room != ch->in_room)
                pet = f->follower;
        }
        f = f->next;
    }

    // Imm mobs get around this.
    if( IS_NPC(ch) && GET_LEVEL(ch) >= LVL_IMMORT )
        mag_followers = 0;

    // If the player already has a summoned pet, the player summons the pet instead.
    if(mag_followers > 0 && savetype != -1)
    {
        if(pet)
        {
            act("$n dissapears.", TRUE, pet, 0, 0, TO_ROOM);
            char_from_room(pet);
            char_to_room(pet, ch->in_room);
            act("$n appears!", TRUE, pet, 0, 0, TO_ROOM);
            act("$n has summoned you!", FALSE, ch, 0, pet, TO_VICT);
            look_at_room(pet, 0);

            // Let's reimburse them something which is about their mana cost.
            sendChar(ch, "You regain some spent mana.\r\n");
            GET_MANA(ch) = MIN(GET_MAX_MANA(ch), GET_MANA(ch) + mag_manacost(ch, spellnum, 0));
        }
        else
            sendChar(ch, "You can only control one conjured creature at a time!\r\n");

        return;
    }

    /* Liam Nov 17, 1996
     * Summon spells not working the first level you get them
     * changed < to <=.
     */
    for( i = 0; summs[i].min_level <= level && summs[i].mob_base_num != MOB_NONE; i++) ;

    i--;

    if( i < 0 || number( 0, 101 ) < summs[i].percent_fail || summs[i].min_level > ch_level) {
        if (i < 0)
            act(SUMM_FAIL_DEF, FALSE, ch, 0, 0, TO_CHAR);
        else
            act(summs[i].summ_fail_mesg, FALSE, ch, 0, 0, TO_CHAR);
        return;
    }

    mob_num = ((summs[i].choices - 1) == 0 ? 0 : number(0,summs[i].choices-1));
    mob_num += summs[i].mob_base_num;

    for (j = 0; j < num; j++) {
        mob = read_mobile(mob_num, VIRTUAL);
        if (!mob) {
            act(SUMM_FAIL_DEF, FALSE, ch, 0, 0, TO_CHAR);
            return;
        }
        char_to_room(mob, ch->in_room);
        if (spellnum == SPELL_CLONE) {
            strcpy(GET_NAME(mob), GET_NAME(ch));
            strcpy(mob->player.short_descr, GET_NAME(ch));
        }
        IS_CARRYING_W(mob) = 0;
        IS_CARRYING_N(mob) = 0;
        act(summs[i].summ_succ_mesg, FALSE, ch, 0, mob, TO_CHAR);
        act(summs[i].summ_succ_mesg, FALSE, ch, 0, mob, TO_ROOM);

        mag_followers = 0;
        summons_ok = TRUE;
        f = ch->followers;
        while( f ){
            if( IS_AFFECTED( f->follower, AFF_CHARM ) && ( f->follower->master == ch )){
                mag_followers += 1;
            }
            f = f->next;
        }
		// Imm mobs get around this.
      if (IS_NPC (ch) && GET_LEVEL (ch) >= LVL_IMMORT)
			mag_followers = 0;

        /* Liam Nov 17, 1996
         * reduced probability of failing to 15% per mob for mages, 25% for Ra
         * This would fail 50% of the time the first level you
         * get the spell, which is too harsh.
         */
#ifdef USE_CHARISMA
        if( GET_CLASS(ch) == CLASS_RANGER ){
            mag_max = GET_LEVEL(ch)/12 + GET_CHA(ch)/6;
        }
        else if( GET_CLASS(ch) == CLASS_MAGIC_USER ){
            mag_max = GET_LEVEL(ch)/10 + GET_CHA(ch)/6;
        }
        else if( GET_CLASS(ch) == CLASS_CLERIC ){
            mag_max = GET_LEVEL(ch)/10 + GET_CHA(ch)/8;
        }
#endif
        mag_max = (GET_CLASS(ch) == CLASS_MAGIC_USER ? 20 : 25 ) * mag_followers;
        if( number(0,99) < mag_max ) summons_ok = 0;

        if(summons_ok && (savetype == -1 ||
            (((ch_level >= summs[i].attack_lev && (summs[i].attack_lev != -1))) ||
             (ch_level >= GET_LEVEL(mob) || number(1,10) < 5))) )
        {
            // Okay, it's summoned.  Let's make sure everything is set correctly and renormalize mage pets
            add_affect( ch, mob, spellnum, level, NO_APPLY, NO_MOD, (24 * (GET_LEVEL(ch)/15) + number(1,10)) TICKS, AFF_CHARM, TRUE, FALSE, FALSE, FALSE);
            SET_BIT_AR(MOB_FLAGS(mob), MOB_CONJURED);
            
            if(!IS_SET_AR(MOB_FLAGS(mob), MOB_NOCHARM))
              mlog("Mob #%d must be made !charm.", GET_MOB_VNUM(mob));
            
            SET_BIT_AR(MOB_FLAGS(mob), MOB_NOCHARM);
            /* HALVE CONJURED MANA HERE - VEX */
            GET_MANA(mob) = GET_MAX_MANA(mob) >>= 1;
            GET_EXP(mob) = 1; /* Should'nt conjure stuff just to kill it.. */
            GET_GOLD(mob) = 0; /* shouldn't conjure stuff to make a profit */
            load_mtrigger(mob);
            add_follower(mob, ch);
            mob->timer  = timer;

            // 2% bonus hitpoints for hunter pets per advance.  1% bonus hitpoints for enchanter pets.
            if(IS_HUNTER(ch))
                GET_MAX_HIT(mob) += GET_MAX_HIT(mob)*(2*GET_ADVANCE_LEVEL(ch))/100;
            if(IS_ENCHANTER(ch))
                GET_MAX_HIT(mob) += GET_MAX_HIT(mob)*GET_ADVANCE_LEVEL(ch)/100;
            GET_HIT(mob) = GET_MAX_HIT(mob);
        }
        /* Vex Febuary 11 1997
         *  Rather than having the mob turn on the mage _ALL_ the time
         *  gave mages a chance to scare the mob away before it attacks
         *  them.
         */
        else {
            SET_BIT_AR(MOB_FLAGS(mob), MOB_CONJURED);
            if ((GET_CLASS(ch) == CLASS_MAGIC_USER) && (number(1, 55) <= ch_level))
            {
                act("$N quakes in terror at the thought of attacking $n!", FALSE, ch, 0, mob, TO_ROOM);
                act("$N arrives determined to destroy you, but you frighten them away!", FALSE, ch, 0, mob, TO_CHAR);
                do_flee(mob, "", 0, 0);
            }
            else
            {
                act("$N roars and turns on $n!", FALSE, ch, 0, mob, TO_ROOM);
                act("$N roars and turns on you!", FALSE, ch, 0, mob, TO_CHAR);
                hit(mob, ch, TYPE_UNDEFINED);
            }
        }

	}
	if (handle_corpse) {
		for (tobj = obj->contains; tobj; tobj = next_obj) {
			next_obj = tobj->next_content;
			obj_from_obj(tobj);
			obj_to_char(tobj, mob);
		}
		extract_obj(obj);
	}


    if(IS_ENCHANTER(ch) && GET_ADVANCE_LEVEL(ch) >= 8 && percentSuccess(5) && savetype != -1) {
        mag_summons(level, ch, obj, spellnum, -1);
        mag_summons(level, ch, obj, spellnum, -1);
    }
    else if(IS_ENCHANTER(ch) && GET_ADVANCE_LEVEL(ch) >= 5 && percentSuccess(10) && savetype != -1) {
        mag_summons(level, ch, obj, spellnum, -1);
    }
    else if(IS_ENCHANTER(ch) && GET_ADVANCE_LEVEL(ch) >= 3 && percentSuccess(5) && savetype != -1) {
        mag_summons(level, ch, obj, spellnum, -1);
    }

}


void mag_points(int level, CharData * ch, CharData * victim,
		     int spellnum, int savetype)
{
  int hit = 0;
  int castermana = 0; // Mana that the caster gets back, after casting.
  int move = 0;

  if (victim == NULL)
    return;

  switch (spellnum) {
  case SPELL_CURE_LIGHT:
    hit = dice(1, 8) + 1 + (level >> 2);
    send_to_char("You feel better.\r\n", victim);
    break;
  case SPELL_CURE_SERIOUS:
	hit = dice(2,8) + 2 + (level >> 2);
	send_to_char("You feel much better.\r\n", victim);
	break;
  case SPELL_CURE_CRITIC:
    hit = dice(3, 8) + 3 + (level);
    send_to_char("You feel a lot better!\r\n", victim);

    // We apply duration bonus so that the longer the players' cast time, the more the second healing pulse is.
    if (ch == victim && savetype == SAVING_SPELL && !affected_by_spell(ch, SPELL_CURE_CRITIC))
        add_affect( ch, victim, spellnum, applyDurationBonus(ch, 45), NO_APPLY, NO_MOD, 45, NO_APPLY, TRUE, FALSE, FALSE, FALSE);
    
    break;
  case SPELL_PRAYER_OF_LIFE:
    hit = dice(1, 20) + 60 + dice(18, 8);
	// If they're not fighting, they lose less mana overall.
	if(FIGHTING(ch))
		castermana = -30;
    send_to_char("You feel your wounds closing.\r\n", victim);
    break;
  case SPELL_HEAL:
	  /*  Turning this off for now.
	  if ( !IS_NPC(ch) && ch == victim) {
		  add_affect( ch, victim, spellnum, level, NO_APPLY, NO_MOD, 3, NO_APPLY, TRUE, FALSE, FALSE, FALSE);
		  return; // no healing immediately for self heal
	  }
	  */
	  hit = 100 + dice(3, 8);
	  send_to_char("A warm feeling floods your body.\r\n", victim);
	  break;
  case SPELL_REVIVE:
	  hit = 200 + dice(4, 5);
	  send_to_char("Your wounds are mended.\r\n", victim);
	  break;
  case SPELL_REFRESH:
	  move = dice(3,10) + (level >> 1);
	  send_to_char("You feel refreshed.\n\r", victim);
	  break;
  case SPELL_CHANGE_ALIGN:
        /*
        ** _XXX_
        */
        GET_ALIGNMENT(victim) = (IS_GOOD(victim) ? -1000 : 1000);
        break;
  }

  //
  if(GET_RACE(ch) == RACE_SELF) {
      hit += hit/10;
      move += move/10;
  }

  if(affected_by_spell(ch, SPELL_HIPPOCRATIC_OATH)) hit += (hit * 2)/10;
  if(savetype == CAST_SPELL && IS_KNIGHT_TEMPLAR(ch) && GET_ADVANCE_LEVEL(ch) >= 5
	  && IS_GOOD(victim))
	  hit += hit / 8;
  if(IS_HOLY_PRIEST(ch))
          hit += hit*GET_ADVANCE_LEVEL(ch)*3/200;
  // Chance for knight templar critical heals
  if(IS_KNIGHT_TEMPLAR(ch) && percentSuccess(2*GET_ADVANCE_LEVEL(ch)))
      hit += hit/2;

  affect_from_char(ch, SKILL_SHADOWFORM);

  if(GET_HIT(victim) < GET_MAX_HIT(victim))
      GET_HIT(victim) = MIN(GET_MAX_HIT(victim), GET_HIT(victim) + hit);
  GET_MOVE(victim) = MIN(GET_MAX_MOVE(victim), GET_MOVE(victim) + move);

  GET_MANA(ch) = MAX(0, GET_MANA(ch) + castermana);
  GET_MANA(ch) = MIN(GET_MAX_MANA(ch), GET_MANA(ch));
}

void mag_unaffects( int level,
                    CharData *ch,
                    CharData *victim,
                    int spellnum,
                    int type )
{
    int spell = 0;
    char *to_vict = NULL, *to_room = NULL;

    if( victim == NULL ) return;

    switch (spellnum) {
    case SPELL_CURE_BLIND:
        spell = SPELL_BLINDNESS;
        break;
    case SPELL_HEAL:
        spell = SPELL_BLINDNESS;
        break;
    case SPELL_GROUP_HEAL:
        spell = SPELL_BLINDNESS;
        break;
    case SPELL_REVIVE:
        spell = SPELL_BLINDNESS;
        break;
    case SPELL_REMOVE_POISON:
        spell = SPELL_POISON;
        to_vict = "A warm feeling runs through your body!";
        to_room = "$n looks better.";
        break;
    case SPELL_CURE_PLAGUE:
        spell = SPELL_PLAGUE;
        to_vict = "You feel cleansed.";
        to_room = "$n looks better.";
        break;
    case SPELL_REMOVE_CURSE:
        spell = SPELL_CURSE;
        //Curse has two curse targets which both need removed
        if( !affected_by_spell( victim, spell )){
            ObjData *rndObj = getCursedObj( victim );
            if( rndObj != NULL ){
                act( "$p briefly glows blue.", TRUE, ch, rndObj, 0, TO_CHAR);
                act( "$n briefly glows blue.", TRUE, victim, NULL, NULL, TO_ROOM);
		REMOVE_BIT_AR(rndObj->obj_flags.extra_flags, ITEM_CURSED);
            }
            return;
        }
        else {
            affect_from_char( victim, spell );
            to_vict = "You feel your luck improve.";
        }
        break;
    case SPELL_HASTE:
        spell = SPELL_SLOW;
        to_vict = "You begin moving at a more normal speed.";
        to_room = "$n begins moving at a more normal speed.";
        break;
    case SPELL_SLOW:
        spell = SPELL_HASTE;
        to_vict = "You slow down to a normal speed.";
        to_room = "$n's movements slow down to normal.";
        break;
    case SPELL_REMOVE_PARALYSIS:
        spell = SPELL_PARALYZE;
        to_vict = "Your movements free up.";
        to_room = "$n is no longer paralyzed.";
        break;
    default:
      mudlog (NRM, LVL_IMMORT, TRUE, "SYSERR: unknown spellnum %d passed to mag_unaffects", spellnum);
        return;
        break;
    }

    if( affected_by_spell( victim, spell )){
        affect_from_char( victim, spell );
        if( to_vict != NULL ){
            sendChar( victim, "%s\r\n", to_vict );
        }
        if( to_room != NULL )
            act(to_room, TRUE, victim, NULL, NULL, TO_ROOM);

        if(!FIGHTING(ch) && !FIGHTING(victim) && percentSuccess(80) &&
           IS_KNIGHT_TEMPLAR(ch) && GET_ADVANCE_LEVEL(ch) >= 3) {
            GET_MANA(ch) += mag_manacost(ch, spellnum, FALSE);
            act("Paladine cleanses $N for you.", FALSE, ch, 0, victim, TO_CHAR);
        }

    }
    else if( to_vict != NULL )
        sendChar( ch, "Nothing seems to happen.\r\n" );
}


ACMD(do_innereye)
{
    if(GET_CLASS(ch) != CLASS_ASSASSIN) {
        sendChar(ch, "Only assassins have the inner eye.\r\n");
        return;
    }
    
    WAIT_STATE(ch, SET_STUN(1));

    // Do they have all the skills on that they could possibly cast?  Then they want to turn them off.
    if( (affected_by_spell(ch, SPELL_INFRAVISION) || !GET_SKILL(ch,SPELL_INFRAVISION))
        && (affected_by_spell(ch, SPELL_SENSE_LIFE) || !GET_SKILL(ch,SPELL_SENSE_LIFE))) {
        sendChar(ch, "You close your inner eye.\r\n");
        affect_from_char(ch, SPELL_INFRAVISION);
        affect_from_char(ch, SPELL_SENSE_LIFE);
        return;
    }

    if(GET_MOVE(ch) < 15)
        sendChar(ch, "You must rest first.\r\n");
    else
        GET_MOVE(ch) -= 15;

    sendChar(ch, "You open your inner eye.\r\n");

    // Otherwise they want to turn on their detection skills.
    if(!affected_by_spell(ch, SPELL_INFRAVISION) && skillSuccess(ch, SPELL_INFRAVISION)) {
        add_affect( ch, ch, SPELL_INFRAVISION, GET_LEVEL(ch), 0, 0, -1, AFF_INFRAVISION, FALSE, FALSE, FALSE, FALSE);
        advanceSkill( ch, SPELL_INFRAVISION, 90, "You see what others do not.", FALSE, FALSE, FALSE );
    }
    if(!affected_by_spell(ch, SPELL_SENSE_LIFE) && skillSuccess(ch, SPELL_SENSE_LIFE)) {
        add_affect(ch, ch, SPELL_SENSE_LIFE, GET_LEVEL(ch), 0, 0, -1,  AFF_SENSE_LIFE, FALSE, FALSE, FALSE, FALSE);
        advanceSkill( ch, SPELL_SENSE_LIFE, 90, "You see what others do not.", FALSE, FALSE, FALSE );
    }

}

void mag_alter_objs( int level,
                     CharData *ch,
                     ObjData  *obj,
                     int spellnum,
                     int savetype )
{
    switch( spellnum ){
    case SPELL_REMOVE_CURSE:
        if( obj ){
            if( IS_SET_AR( obj->obj_flags.extra_flags, ITEM_CURSED )){
                act("$p briefly glows blue.", TRUE, ch, obj, 0, TO_CHAR);
		REMOVE_BIT_AR(obj->obj_flags.extra_flags, ITEM_CURSED);
            }
        }
        break;

    default: break;
    }
}


void mag_objects( int level,
                  CharData * ch,
                  CharData * obj,
                  int spellnum )
{
    switch( spellnum ){
    case SPELL_CREATE_WATER: break;
    }
}




void mag_creations(int level, CharData * ch, int spellnum)
{
    ObjData *tobj;
    int z, t;
    CharData *tch, *next_tch;

    if (ch == NULL) return;

  level = MAX (MIN (level, LVL_IMPL), 1);

    switch (spellnum) {
        case SPELL_CREATE_FOOD:
            z = 7090;
            t = (GET_LEVEL(ch) / 5) + number (1,30);
            break;
        case SPELL_FOUNT_OF_YOUTH:
            sendChar(ch, "The Fountain of Youth shimmers faintly in the air "
                         "before you.\r\n");
            sendChar(ch, "A small spray of water hardens into glass, landing "
                         "neatly in your hand.\r\n");
            z = 894;
            t = 40;
            break;
        case SPELL_CREATE_SPRING:
            z = 1325;
            t = (GET_LEVEL(ch) / 5) + number (1,30);
            break;
        case SPELL_BALL_OF_LIGHT:
            z = 1326;
            t = (GET_LEVEL(ch) / 5) + number (1,30);
            break;
        case SPELL_CREATE_WARDING:
            z = 898;
            t = number(2, 5);
            break;
    }


    if (spellnum == SPELL_CREATE_WARDING) {
        if(ROOM_FLAGGED(ch->in_room, ROOM_NOMOB)) {
            send_to_char("This room already resists monsters' intrusions.\r\n", ch);
            return;
        }

        for( tch = world[ch->in_room].people; tch; tch = next_tch ){
            next_tch = tch->next_in_room;

            if(IS_NPC(tch) && !(IS_AFFECTED(tch, AFF_CHARM) || IS_SET_AR( MOB_FLAGS(tch), MOB_MOUNT))) {
                send_to_char("You cannot create a warding without first clearing the room of mobiles.\r\n", ch);
                return;
            }

        }
    }


    if (!(tobj = read_perfect_object(z, VIRTUAL))) {
        send_to_char("I seem to have goofed.\r\n", ch);
      mlog ("SYSERR: spell_creations, spell %d, obj %d: obj not found",
                spellnum, z);
        return;
    }

    SET_BIT_AR(tobj->obj_flags.extra_flags, ITEM_TIMED);
    GET_OBJ_TIMER(tobj) = t;

    if (spellnum == SPELL_CREATE_SPRING ||
        spellnum == SPELL_CREATE_WARDING)
        obj_to_room(tobj, ch->in_room);
    else
	obj_to_char(tobj, ch);

    act("$n creates $p.", FALSE, ch, tobj, 0, TO_ROOM);
    if (spellnum != SPELL_FOUNT_OF_YOUTH)
        act("You create $p.", FALSE, ch, tobj, 0, TO_CHAR);

    load_otrigger(tobj);
}
