/*
 * newspellparser.c Portions from DikuMUD, Copyright (C) 1990, 1991. Written
 * by Fred Merkel and Jeremy Elson Part of JediMUD Copyright (C) 1993
 * Trustees of The Johns Hopkins Unversity All Rights Reserved.
 */

#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/class.h"
#include "general/comm.h"
#include "general/handler.h"
#include "actions/interpreter.h"
#include "actions/outlaw.h"
#include "magic/skills.h"
#include "magic/spells.h"
#include "util/utils.h"
#include "specials/special.h"
#include "scripts/dg_scripts.h"
#include "magic/spell_parser.h"
#include "magic/missile.h"
#include "specials/flag_game.h"
#include "specials/muckle.h"

struct spell_info_type spell_info[TOP_SPELL_DEFINE + 1];

#define SINFO spell_info[spellnum]

#define MANUAL_SPELL(spellname) spellname(level, caster, cvict, ovict, castobj, casttype);

#define SKILL_PENUMBRAE_LEARN 75

/*
 * This arrangement is pretty stupid, but the number of skills is limited by
 * the playerfile.  We can arbitrarily increase the number of skills by
 * increasing the space in the playerfile. Meanwhile, this should provide
 * ample slots for skills.
 */


char *utterance[NUM_CLASSES + 1] = {
      "utters",   // Mu
      "prays",    // Cl (or invoke??)
      "sputters", // Th, shouldn't be used
      "sputters", // Wa, shouldn't be used
      "utters",   // Ra
      "sputters", // As, shouldn't be used
      "sputters", // Sl, shouldn't be used
      "prays",    // Kn
      "curses",   // Dk
      "whispers", // Sd
      "curses",   // Nm
      "spatters"  // Error!
};

struct syllable {
  char *org;
  char *new;
};

struct syllable syls[] = {
  {" ", " "},
  {"ar", "abra"},
  {"ate", "i"},
  {"cau", "kada"},
  {"blind", "nose"},
  {"bur", "mosa"},
  {"cu", "judi"},
  {"de", "oculo"},
  {"dis", "mar"},
  {"ect", "kamina"},
  {"en", "uns"},
  {"gro", "cra"},
  {"light", "dies"},
  {"lo", "hi"},
  {"magi", "kari"},
  {"mon", "bar"},
  {"mor", "zak"},
  {"move", "sido"},
  {"ness", "lacri"},
  {"ning", "illa"},
  {"per", "duda"},
  {"ra", "gru"},
  {"re", "candus"},
  {"son", "sabru"},
  {"tect", "infra"},
  {"tri", "cula"},
  {"ven", "nofo"},
  {"word of", "inset"},
  {"a", "i"}, {"b", "v"}, {"c", "q"}, {"d", "m"}, {"e", "o"}, {"f", "y"}, {"g", "t"},
  {"h", "p"}, {"i", "u"}, {"j", "y"}, {"k", "t"}, {"l", "r"}, {"m", "w"}, {"n", "b"},
  {"o", "a"}, {"p", "s"}, {"q", "d"}, {"r", "f"}, {"s", "g"}, {"t", "h"}, {"u", "e"},
  {"v", "z"}, {"w", "x"}, {"x", "n"}, {"y", "l"}, {"z", "k"}, {"", ""}
};

int mag_manacost(CharData *ch, int spellnum, int spellCostImmutable)
{
    int mana;
    int i;

    mana = MAX(SINFO.mana_max - (SINFO.mana_change *
                (GET_LEVEL(ch) - SINFO.min_level[(int) GET_CLASS(ch)])),
            SINFO.mana_min);

    /* remort spells done differently */
    if (IS_REMORT_SPELL(spellnum))
        mana = MAX(SINFO.mana_max - (SINFO.mana_change * (GET_LEVEL(ch)-1)),
                SINFO.mana_min);

  if(!spellCostImmutable) {

    /* Mana item effects here. */
    if (ch->equipment[WEAR_NECK] || ch->equipment[WEAR_ORBIT]) {
        int increase = 0;
        for (i = 0; i < MAX_OBJ_AFFECT; i++) {
            if ((GET_CLASS(ch) == CLASS_MAGIC_USER
                        || GET_CLASS(ch) == CLASS_NECROMANCER) &&
                    ch->equipment[WEAR_NECK] &&
            ch->equipment[WEAR_NECK]->affected[i].location == APPLY_SPELL_COST)
                increase += ch->equipment[WEAR_NECK]->affected[i].modifier;
            if (ch->equipment[WEAR_ORBIT] &&
            ch->equipment[WEAR_ORBIT]->affected[i].location == APPLY_SPELL_COST)
                increase += ch->equipment[WEAR_ORBIT]->affected[i].modifier;
        }
        mana += (int)((mana * increase)/100);
    }

    /* fervor makes your spells cheaper */
    mana -= (int)((mana * GET_SKILL(ch, SKILL_FERVOR))/400);

    /* shadowform reduces spell cost */
    if(affected_by_spell(ch, SKILL_SHADOWFORM))
        mana -= mana * 20 / 100;

    if(affected_by_spell(ch, SKILL_PENUMBRAE))
        mana -= mana * spell_level(ch, SKILL_PENUMBRAE)/100;

    /* energy drain costs you extra mana */
    if (affected_by_spell(ch, SPELL_ENERGY_DRAIN))
        mana = (mana * spell_level(ch, SPELL_ENERGY_DRAIN)) / 30;

  } /* !spell cost immutable */

    // While affected by potency, spells cost 3x as much mana.
    if(affected_by_spell(ch, SKILL_POTENCY))
        mana *= 3;

    if (mana < 1) mana = 1;
    return mana;
}


bool is_chant( CharData * ch, int spellnum, CharData * tch, ObjData * tobj)
{    // Some spells can be cast, but no standard cast message.

	switch (spellnum) {

	case SPELL_REVIVE:
	if ( !IS_NPC(ch) && (ch == tch) )    
			return TRUE;
		else return FALSE;
		break;

	case SPELL_DANCE_SHADOWS:
		if (!IS_NPC(ch) && affected_by_spell(tch, spellnum)  && 
			(GET_LEVEL(ch) > 35) )
			return TRUE;
		else return FALSE;
		break;
	
	default:
		return FALSE;
	}

	return FALSE;
}


/* say_spell erodes buf, buf1, buf2 */
void
say_spell( CharData * ch,
           int spellnum,
           CharData * tch,
	   ObjData * tobj)
{
    CharData *i;
    char              lbuf[256];
    int               j, ofs = 0;

    if(( spellnum >= SPELL_FIRE_BREATH ) && ( spellnum <= SPELL_KNOWLEDGE ))
        return; /* Just bail on breath weapons, and misc obj spells. */

	if ( is_chant(ch, spellnum, tch, tobj) )
		return;   // Let's not announce to rooms when some chants start.

    *buf = '\0';
    strcpy( lbuf, spells[spellnum] );

    while( *(lbuf + ofs) )
        for( j = 0; *(syls[j].org); j++ )

            if (!strncmp(syls[j].org, lbuf + ofs, strlen(syls[j].org))) {
                strcat(buf, syls[j].new);
                ofs += strlen(syls[j].org);
            }/* if */

  if (tch != NULL && tch->in_room == ch->in_room) {
    if (tch == ch)
      sprintf(lbuf, "$n closes $s eyes and %s the words, '%%s'.", utterance[GET_CLASS(ch)]);
    else
      sprintf(lbuf, "$n stares at $N and %s the words, '%%s'.", utterance[GET_CLASS(ch)]);
  } else if (tobj != NULL && tobj->in_room == ch->in_room)
    sprintf(lbuf, "$n stares at $p and %s the words, '%%s'.", utterance[GET_CLASS(ch)]);
  else
    sprintf(lbuf, "$n %s the words, '%%s'.", utterance[GET_CLASS(ch)]);

  sprintf(buf1, lbuf, spells[spellnum]);
  sprintf(buf2, lbuf, buf);

  for (i = world[ch->in_room].people; i; i = i->next_in_room) {
    if (i == ch || i == tch || !i->desc || !AWAKE(i))
      continue;
    if (GET_CLASS(ch) == GET_CLASS(i))
      perform_act(buf1, ch, tobj, tch, i);
    else
      perform_act(buf2, ch, tobj, tch, i);
  }

  if (tch != NULL && tch != ch) {
    sprintf(buf1, "$n stares at you and %s the words, '%s'.",
            utterance[GET_CLASS(ch)], 
	    GET_CLASS(ch) == GET_CLASS(tch) ? spells[spellnum] : buf);
    act(buf1, FALSE, ch, NULL, tch, TO_VICT);
  }

}/* say_spell */



/* the other is_abbrev works both ways, which is broken behaviour for
 * find_skill_num! */
static int is_REAL_abbrev(char *one, char *two)
{
  if (!*one) return 0;
  for (; *one; one++, two++)
     if (LOWER(*one) != LOWER(*two))
       return 0;
  return 1;
}

int find_skill_num(char *name)
{
  int index = 0, ok;
  char *temp, *temp2;
  char first[256], first2[256];

  while (*spells[++index] != '\n') {
    if (is_REAL_abbrev(name, spells[index]))
      return index;

    ok = 1;
    temp = any_one_arg(spells[index], first);
    temp2 = any_one_arg(name, first2);
    while (*first && *first2 && ok) {
      if (!is_REAL_abbrev(first2, first))
	ok = 0;
      temp = any_one_arg(temp, first);
      temp2 = any_one_arg(temp2, first2);
    }

    if (ok && !*first2)
      return index;
  }

  return -1;
}





/*
 * All invocations of any spell must come through this function,
 * call_magic(). This is also the entry point for non-spoken or unrestricted
 * spells. Spellnum 0 is legal but silently ignored here, to make callers
 * simpler.
 */
int call_magic( CharData *caster,
                CharData *cvict,
                ObjData  *ovict,
                             int  spellnum,
                             int  level,
                ObjData  *castobj,
                             int  casttype )
{
    int savetype;

    if( spellnum < 1 || spellnum > TOP_SPELL_DEFINE ) return 0;

//     This check is breaking peace rooms for some reason
//    if (caster->nr != real_mobile(DG_CASTER_PROXY)) {
      if( ROOM_FLAGGED(caster->in_room, ROOM_NOMAGIC) ){
          sendChar( caster, "Your magic fizzles out and dies.\r\n" );
          WAIT_STATE(caster, PULSE_VIOLENCE);
          return 0;
      }

      /* disruptive rooms don't handle magic so well */
      if (ROOM_FLAGGED(caster->in_room, ROOM_DISRUPTIVE)) {
          switch (number(1,10)) {
              case 1:   /* 1 in 10 chance to fizzle */
                  sendChar(caster, "Your magic fizzles out and dies.\r\n");
                  WAIT_STATE(caster, PULSE_VIOLENCE);
                  return 0;
              case 2:  /* 1 in 10 chance to misdirect */
                  if (cvict && cvict->in_room != -1)
                      cvict = random_victim(cvict->in_room);
                  sendChar(caster, "You feel confused!\r\n");
                  break;
              case 3:  /* 1 in 10 chance to "feel strange" */
                  sendChar(caster, "Magic feels strange here ...\r\n");
                  break;
          }
      }
    
      /* While confused, who knows what will happen.*/
      if (affected_by_spell(caster, SPELL_CONFUSION)) {
          switch (number(1,10)) {
              case 1:   /* 1 in 10 chance to fizzle */
                  sendChar(caster, "Your magic fizzles out and dies.\r\n");
                  WAIT_STATE(caster, PULSE_VIOLENCE);
                  return 0;
              case 2:  /* 1 in 10 chance to misdirect */
                  if (cvict && cvict->in_room != -1)
                      cvict = random_victim(cvict->in_room);
                  sendChar(caster, "You feel confused!\r\n");
                  break;
              case 3:  /* 1 in 10 chance to "feel strange" */
                  sendChar(caster, "Something seems wrong...\r\n");
                  break;
          }
      }

      // Prevent items being used for offense - this is definitely one quick hack
      // good thing I re-writing all this stuff huh?
      if( ROOM_FLAGGED(caster->in_room, ROOM_PEACEFUL) ||
              (cvict && ROOM_FLAGGED(cvict->in_room, ROOM_PEACEFUL))) {
          if (SINFO.violent || IS_SET(SINFO.routines, MAG_DAMAGE)){
              sendChar( caster, "A flash of white light fills the room, dispelling your " "violent magic!\r\n" );
              WAIT_STATE(caster, PULSE_VIOLENCE);
              return FALSE;
          }
      }

      if( caster != cvict ){
          if( (cvict) && !IS_NPC(cvict) && !IS_NPC(caster) &&
              (SINFO.violent || IS_SET(SINFO.routines, MAG_DAMAGE))){
              /*
              ** Special pkilling checks done here. -digger
              */
              WAIT_STATE(caster, PULSE_VIOLENCE);
              
              player_attack_victim(caster, cvict);
          }

          if( (IS_SET_AR(ROOM_FLAGS(caster->in_room), ROOM_PEACEFUL ) ||
              (cvict && ROOM_FLAGGED(cvict->in_room, ROOM_PEACEFUL))) &&
             (SINFO.violent || IS_SET(SINFO.routines, MAG_DAMAGE))) {
              sendChar( caster, "A flash of white light fills the room, dispelling your " "violent magic!\r\n" );
              WAIT_STATE(caster, PULSE_VIOLENCE);
              return FALSE;
          }
      }
//  }
// end of cut - memnoch

  // Can't overly punish players who are brained.
  if(cvict && (SINFO.violent || IS_SET(SINFO.routines, MAG_DAMAGE)))
      affect_from_char(cvict, SKILL_BRAIN);

  /* determine the type of saving throw */
  switch (casttype) {
  case CAST_STAFF:
  case CAST_SCROLL:
  case CAST_POTION:
  case CAST_FOOD:
  case CAST_WAND:
    savetype = SAVING_ROD;
    break;
  case CAST_SPELL:
    savetype = SAVING_SPELL;
    break;
  default:
    savetype = SAVING_BREATH;
    break;
  }


  if( IS_SET(spell_info[spellnum].targets, TAR_CHAR_GROUP) && (cvict == NULL) && (caster != NULL)) {
    mag_groups(level, caster, spellnum, savetype); /* This has been cast in its group form. */
    return 1; /* If we dropped down further, people could get hit more than once. */
  }

  if (IS_SET(SINFO.routines, MAG_DAMAGE))
    mag_damage(level, caster, cvict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_AFFECTS))
    mag_affects(level, caster, cvict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_UNAFFECTS))
    mag_unaffects(level, caster, cvict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_POINTS))
    mag_points(level, caster, cvict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_ALTER_OBJS))
    mag_alter_objs(level, caster, ovict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_GROUPS))
    mag_groups(level, caster, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_MASSES))
    mag_masses(level, caster, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_AREAS))
    mag_areas(level, caster, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_SUMMONS))
    mag_summons(level, caster, ovict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_CREATIONS))
    mag_creations(level, caster, spellnum);

  if (IS_SET(SINFO.routines, MAG_MANUAL))
    switch (spellnum) {
    case SPELL_YOUTHEN:
      MANUAL_SPELL(spell_youthen);
      break;
    case SPELL_KNOWLEDGE:
      MANUAL_SPELL(spell_knowledge);
      break;
    case SPELL_RECHARGE:
      MANUAL_SPELL(spell_recharge);
      break;
    case SPELL_TELEPORT:
      MANUAL_SPELL(spell_teleport);
      break;
    case SPELL_ENCHANT_WEAPON:
      MANUAL_SPELL(spell_enchant_weapon);
      break;
    case SPELL_CHARM:
      MANUAL_SPELL(spell_charm);
      break;
    case SPELL_WORD_OF_RECALL:
      MANUAL_SPELL(spell_recall);
      break;
    case SPELL_IDENTIFY:
      MANUAL_SPELL(spell_identify);
      break;
    case SPELL_SUMMON:
      MANUAL_SPELL(spell_summon);
      break;
    case SPELL_RELOCATE:
      MANUAL_SPELL(spell_relocate);
      break;
    case SPELL_LOCATE_OBJECT:
      MANUAL_SPELL(spell_locate_object);
      break;
    case SPELL_CREATE_WATER:
      MANUAL_SPELL(spell_create_water);
      break;
    case SPELL_PORTAL:
      MANUAL_SPELL(spell_portal);
      break;
	case SPELL_MINOR_CREATION:
	  break;
	case SPELL_CALM:
	  MANUAL_SPELL(spell_calm);
	  break;
	case SPELL_FORGET:
	  MANUAL_SPELL(spell_forget);
	  break;
	case SPELL_BANISH:
	  MANUAL_SPELL(spell_banish);
	  break;
	case SPELL_SAND_STORM:
	  MANUAL_SPELL(spell_sand_storm);
	  break;
    case SPELL_HANDS_OF_WIND:
          MANUAL_SPELL(spell_hands_of_wind);
	  break;
    case SPELL_LIFE_DRAIN:
          MANUAL_SPELL(spell_life_drain);
	  break;
    case SPELL_NEXUS:
          MANUAL_SPELL(spell_nexus);
          break;
    case SPELL_CLEANSE:
      MANUAL_SPELL(spell_cleanse);
      break;
    case SPELL_SACRIFICE:
      MANUAL_SPELL(spell_sacrifice);
      break;
    case SPELL_CONSUME_CORPSE:
      MANUAL_SPELL(spell_consume_corpse);
      break;
    case SPELL_EXPLODE_CORPSE:
      MANUAL_SPELL(spell_explode_corpse);
      break;
    case SPELL_BONE_WALL:
      MANUAL_SPELL(spell_bone_wall);
      break;
    case SPELL_SUMMON_CORPSE:
      MANUAL_SPELL(spell_summon_corpse);
      break;
    case SPELL_CHARM_CORPSE:
      MANUAL_SPELL(spell_charm_corpse);
      break;
    case SPELL_CALL_STEED:
      MANUAL_SPELL(spell_call_steed);
      break;
    case SPELL_FLETCH:
      MANUAL_SPELL(spell_fletch);
      break;
    case SPELL_EMBALM:
      MANUAL_SPELL(spell_embalm);
      break;
    case SPELL_ENTOMB_CORPSE:
      MANUAL_SPELL(spell_entomb_corpse);
      break;
    }

  return 1;
}

/*
 * mag_objectmagic: This is the entry-point for all magic items.
 *
 * staff  - [0]	level	[1] max charges	[2] num charges	[3] spell num
 * wand   - [0]	level	[1] max charges	[2] num charges	[3] spell num
 * scroll - [0]	level	[1] spell num	[2] spell num	[3] spell num
 * potion - [0] level	[1] spell num	[2] spell num	[3] spell num
 * food   - [0] full    [1] level       [2] spell num   [3] poison?
 * dust   - [0] level	[1] spell num	[2] spell num	[3] spell num
 * Food works a bit like a potion. Its extracted in do_eat, stoopid but
 * simpler. Vex.
 *
 * Staves and wands will default to level 14 if the level is not specified.
 */

void mag_objectmagic(CharData * ch, ObjData * obj,
		          char *argument)
{
  int i, k;
  CharData *tch = NULL, *next_tch;
  CharData *caster = NULL;
  ObjData *tobj = NULL;

  one_argument(argument, arg);

	/* Removal of PVP restrictions by Craklyn - Bean
	if (FIGHTING(ch) && IS_PVP(ch, FIGHTING(ch))) {
	sendChar( ch, "You can't use magical impliments against such a mighty opponent!\r\n" );
	return;
	}
	*/
  // Instant Poison restricts use of items
  if(affected_by_spell(ch, SKILL_INSTANT_POISON)){
	  sendChar( ch, "The poison makes it too hard to concentrate...\r\n" );
	  return;
  }

  if(the_muckle == ch) {
      sendChar(ch, "The muckle may not use magical items.\r\n");
      return;
  }
      

  k = generic_find(arg, FIND_CHAR_ROOM | FIND_OBJ_INV | FIND_OBJ_ROOM |
		   FIND_OBJ_EQUIP, ch, &tch, &tobj);

  switch (GET_OBJ_TYPE(obj)) {
  case ITEM_STAFF:
    act("You tap $p three times on the ground.", FALSE, ch, obj, 0, TO_CHAR);
    if (obj->action_description)
      act(obj->action_description, FALSE, ch, obj, 0, TO_ROOM);
    else
      act("$n taps $p three times on the ground.", FALSE, ch, obj, 0, TO_ROOM);

    if (GET_OBJ_VAL(obj, 2) <= 0) {
      act("It seems powerless.", FALSE, ch, obj, 0, TO_CHAR);
      act("Nothing seems to happen.", FALSE, ch, obj, 0, TO_ROOM);
    } else {
      GET_OBJ_VAL(obj, 2)--;
      for (tch = world[ch->in_room].people; tch; tch = next_tch) {
	next_tch = tch->next_in_room;
	if (ch == tch)
	  continue;

	
	if (GET_OBJ_VAL(obj, 0))
	  call_magic(ch, tch, NULL, GET_OBJ_VAL(obj, 3),
    	     GET_OBJ_VAL(obj, 0), obj, CAST_STAFF);
	else
		call_magic(ch, tch, NULL, GET_OBJ_VAL(obj, 3),
		DEFAULT_STAFF_LVL, obj, CAST_STAFF);
      }
    }
	
	/* Don't allow them to burn staff in just one round. */
    WAIT_STATE(ch, PULSE_VIOLENCE);

    break;
  case ITEM_WAND:
    if (k == FIND_CHAR_ROOM) {
      if (tch == ch) {
	act("You point $p at yourself.", FALSE, ch, obj, 0, TO_CHAR);
	act("$n points $p at $mself.", FALSE, ch, obj, 0, TO_ROOM);
      } else {
	act("You point $p at $N.", FALSE, ch, obj, tch, TO_CHAR);
	if (obj->action_description != NULL)
	  act(obj->action_description, FALSE, ch, obj, tch, TO_ROOM);
	else
	  act("$n points $p at $N.", TRUE, ch, obj, tch, TO_ROOM);
      }
    } else if (tobj != NULL) {
      if(!IS_SET(spell_info[GET_OBJ_VAL(obj, 3)].targets,
                  TAR_OBJ_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP)) {
          act("You can't use $p on that.", FALSE, ch, obj, tobj, TO_CHAR);
          return;
      }
      act("You point $p at $P.", FALSE, ch, obj, tobj, TO_CHAR);
      if (obj->action_description != NULL)
	act(obj->action_description, FALSE, ch, obj, tobj, TO_ROOM);
      else
	act("$n points $p at $P.", TRUE, ch, obj, tobj, TO_ROOM);
    } else {
      act("At what should $p be pointed?", FALSE, ch, obj, NULL, TO_CHAR);
      return;
    }

    if (GET_OBJ_VAL(obj, 2) <= 0) {
      act("It seems powerless.", FALSE, ch, obj, 0, TO_CHAR);
      return;
    }
    GET_OBJ_VAL(obj, 2)--;
    if (GET_OBJ_VAL(obj, 0))
      call_magic(ch, tch, tobj, GET_OBJ_VAL(obj, 3),
		 GET_OBJ_VAL(obj, 0), obj, CAST_WAND);
    else
      call_magic(ch, tch, tobj, GET_OBJ_VAL(obj, 3),
		 DEFAULT_WAND_LVL, obj, CAST_WAND);
    /* Don't allow them to burn wand in just one round. */
    WAIT_STATE(ch, PULSE_VIOLENCE);
    break;
  case ITEM_SCROLL:
    if (*arg) {
      if (!k) {
	act("There is nothing to here to affect with $p.", FALSE,
	    ch, obj, NULL, TO_CHAR);
	return;
      }
    } else
      tch = ch;

    act("You recite $p which dissolves.", TRUE, ch, obj, 0, TO_CHAR);
    if (obj->action_description)
      act(obj->action_description, FALSE, ch, obj, NULL, TO_ROOM);
    else
      act("$n recites $p.", FALSE, ch, obj, NULL, TO_ROOM);

    for (i = 1; i < 4; i++){
        char thecardstring[51];
        char thecardstring2[51];
        strcpy(thecardstring, "thecard");
        sprintf(thecardstring2, "thecard%s", GET_NAME(ch));
        char scribestring[51];
        char scribestring2[51];
        strcpy(scribestring, "scribedscroll");
        sprintf(scribestring2, "scribedscroll%s", GET_NAME(ch));

        // If the scroll is one of TheCards, then it is cast as a spell for improved DK benefit.
        // Otherwise, it's cast as a scroll.
        if(isname(thecardstring, obj->name)) {
            if(isname(thecardstring2, obj->name)) {
                if (!(call_magic(ch, tch, tobj, GET_OBJ_VAL(obj, i), GET_OBJ_VAL(obj, 0), obj, CAST_SPELL)))
                    break;
            }
            else { act("Only the creator of a tarot card can read its magic.", TRUE, ch, obj, 0, TO_CHAR); break; }
        }
        else if(isname(scribestring, obj->name)) {
            if(isname(scribestring2, obj->name)) {
                if (!(call_magic(ch, tch, tobj, GET_OBJ_VAL(obj, i), GET_OBJ_VAL(obj, 0), obj, CAST_SCROLL)))
                    break;
            }
            else { act("Only the creator of a scribed scroll can read its magic.", TRUE, ch, obj, 0, TO_CHAR); break; }
        }
        else if (!(call_magic(ch, tch, tobj, GET_OBJ_VAL(obj, i), GET_OBJ_VAL(obj, 0), obj, CAST_SCROLL)))
            break;
    }

    /* Dissallow multiple scroll use in a round. */
    WAIT_STATE(ch, PULSE_VIOLENCE);

    if (obj != NULL)
      extract_obj(obj);
    break;
  case ITEM_DUST:
    tch = ch;
    act("You toss $p into the air.", FALSE, ch, obj, NULL, TO_CHAR);
    if (obj->action_description)
        act(obj->action_description, FALSE, ch, obj, NULL, TO_ROOM);
    else
        act("$n tosses $p into the air.", TRUE, ch, obj, NULL, TO_ACTSPAM);

	caster = read_mobile(DG_CASTER_PROXY, VIRTUAL);
    if (!caster) {
      script_log("dg_cast: Cannot load the caster mob!");
      return;
    }
    
    caster->player.short_descr = str_dup("The gods");
    char_to_room( caster, ch->in_room );

	for (i = 1; i < 4; i++) {
		if ( is_chant(ch, GET_OBJ_VAL(obj, 1), tch, obj) ) {
			if (!(call_magic(caster, tch, NULL, GET_OBJ_VAL(obj, i), GET_OBJ_VAL(obj, 0),
				obj, CAST_POTION)))
				break;
		} else {
			if (!(call_magic(ch, ch, NULL, GET_OBJ_VAL(obj, i),
				GET_OBJ_VAL(obj, 0), obj, CAST_POTION)))
				break;
		}
	}
	
	extract_char(caster);

    /* Disallow multiple dust use in a round. */
    WAIT_STATE(ch, PULSE_VIOLENCE);
    if (obj != NULL) extract_obj(obj);
    break;
  case ITEM_POTION:
    tch = ch;

    act("You quaff $p.", FALSE, ch, obj, NULL, TO_CHAR);
    if (obj->action_description)
      act(obj->action_description, FALSE, ch, obj, NULL, TO_ROOM);
    else
    { /* The TO_ used to be TO_ROOM */
      act("$n quaffs $p.", TRUE, ch, obj, NULL, TO_ACTSPAM);
    }

	caster = read_mobile(DG_CASTER_PROXY, VIRTUAL);
	if (!caster) {
		script_log("dg_cast: Cannot load the caster mob!");
		return;
	}
	
	caster->player.short_descr = str_dup("The gods");
	char_to_room( caster, ch->in_room );
		
	
	for (i = 1; i < 4; i++) {			
		if(is_chant( ch, GET_OBJ_VAL(obj, i), tch, obj)) {
			if (!(call_magic(caster, ch, NULL, GET_OBJ_VAL(obj, i),
				GET_OBJ_VAL(obj, 0), obj, CAST_POTION)))
				break;
			}
		else if (!(call_magic(ch, tch, NULL, GET_OBJ_VAL(obj, i), GET_OBJ_VAL(obj, 0),
			obj, CAST_POTION)))
			break;
	}

		extract_char(caster);
		
		/* Dissallow multiple potion use in a round. */
    WAIT_STATE(ch, PULSE_VIOLENCE);
    if (obj != NULL)
		extract_obj(obj);
    break;
  case ITEM_FOOD:
	  tch = ch; /* You already see them eat it in do_eat. */

	  if(is_chant(ch, GET_OBJ_VAL(obj, 2), tch, obj))
	  {
		  caster = read_mobile(DG_CASTER_PROXY, VIRTUAL);
		  if (!caster) {
			  script_log("dg_cast: Cannot load the caster mob!");
			  return;
		  }
		  
		  caster->player.short_descr = str_dup("The gods");
		  char_to_room( caster, ch->in_room );

		  call_magic(caster, ch, NULL, GET_OBJ_VAL(obj, 2), GET_OBJ_VAL(obj, 1), obj, CAST_FOOD);
		  extract_char(caster);
	  }
	  else 
		  call_magic(ch, ch, NULL, GET_OBJ_VAL(obj, 2), GET_OBJ_VAL(obj, 1), obj, CAST_FOOD);

	break;
  default:
    mlog("SYSERR: Unknown object_type in mag_objectmagic");
    break;
  }
}

/*
 * cast_spell is used generically to cast any spoken spell, assuming we
 * already have the target char/obj and spell number.  It checks all
 * restrictions, etc., prints the words, etc.
 *
 * Entry point for NPC casts.
 */

int cast_spell(CharData * ch, CharData * tch,
	           ObjData * tobj, int spellnum)
{
  if (GET_POS(ch) < SINFO.min_position) {
    switch (GET_POS(ch)) {
      case POS_SLEEPING:
      send_to_char("You dream about great magical powers.\r\n", ch);
      return 0;
      break;
    case POS_RESTING:
      send_to_char("You cannot concentrate while resting.\r\n", ch);
      return 0;
      break;
    case POS_SITTING:
      send_to_char("You can't do this sitting!\r\n", ch);
      return 0;
      break;
    case POS_FIGHTING:
      send_to_char("Impossible!  You can't concentrate enough!\r\n", ch);
      return 0;
      break;
    default:
      send_to_char("You can't do much of anything like this!\r\n", ch);
      return 0;
      break;
    }
    return 0;
  }
  if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == tch)) {
    send_to_char("You are afraid you might hurt your master!\r\n", ch);
    return 0;
  }
  if ((tch != ch) && IS_SET(SINFO.targets, TAR_SELF_ONLY)) {
    send_to_char("You can only cast this spell upon yourself!\r\n", ch);
    return 0;
  }
  if ((tch == ch) && IS_SET(SINFO.targets, TAR_NOT_SELF)) {
    send_to_char("You cannot cast this spell upon yourself!\r\n", ch);
    return 0;
  }
/* Kaidon: added check to see if caster affected by silence */
  if (IS_AFFECTED(ch, AFF_SILENCE)) {
    send_to_char("Your lips move, but no sound issues forth!\r\n", ch);
    return 0;
  }
  
/* Vex Febuary 17, 1997
** NPC's previously did not use mana, now I'm going to make sure
** they will. Immortal level PC's don't have to worry about mana, all
** NPC's do.
*/
  if IS_NPC(ch) /* Just in case... */
  {
      int mana = mag_manacost(ch, spellnum, FALSE);
      // If mobs cast for reduced spell damage, let their spells cost less mana -Craklyn
      if( mana > 0 )
          mana = MAX( 1, MIN(mana, mana * GET_MOB_SPELL_DAM(ch) / 100) );

      if ((mana > 0) && (GET_MANA(ch) < mana)) {
          send_to_char("You haven't the energy to cast that spell!\r\n", ch);
          return 0;
      }
      else /* Had enough mana to cast the spell, so deduct the mana. */
      {
          if (mana > 0)
              GET_MANA(ch) = MAX(0, MIN(GET_MAX_MANA(ch), GET_MANA(ch) - mana));
      }
  }
  sendChar( ch, CONFIG_OK );
  say_spell(ch, spellnum, tch, tobj);
  WAIT_STATE(ch, PULSE_VIOLENCE);

  return (call_magic(ch, tch, tobj, spellnum, GET_LEVEL(ch), NULL, CAST_SPELL));
}


/* This is defined in spells.h, it's just a static array of memory
 * locations, basically, that can be passed as ObjData pointers via
 * do_cast.  It's probably a bad idea to think there's useful data in
 * them. :) */
ObjData cast_directions[NUM_OF_DIRS];

/*
 * do_cast is the entry point for PC-casted spells.  It parses the arguments,
 * determines the spell number and finds a target, throws the die to see if
 * the spell can be cast, checks for sufficient mana and subtracts it, and
 * passes control to cast_spell().
 */

ACMD(do_cast)
{
  static const char *castDirs[] = { "north", "east", "south", "west", "up",
      "down", "\n"};
  CharData *tch = NULL;
  ObjData *tobj = NULL;
  char *s, *t, *d;
  int mana, spellnum, i, target = 0, group = 0, level, dir_num;

  if (IS_NPC(ch))
      if (ch->desc == NULL)
          return;

  /* get: blank, spell name, target name */
  s = strtok(argument, "'");

  if (s == NULL) {
    send_to_char("Cast what where?\r\n", ch);
    return;
  }
  s = strtok(NULL, "'");
  if (s == NULL) {
    send_to_char("Spell names must be enclosed in the Holy Magic Symbols: '\r\n", ch);
    return;
  }
  t = strtok(NULL, "\0");

  /* spellnum = search_block(s, spells, 0); */
  spellnum = find_skill_num(s);

  if ((spellnum < 1) || (spellnum > MAX_SPELLS)) {
    send_to_char("Cast what?!?\r\n", ch);
    return;
  }
  if ( GET_SKILL(ch, spellnum) == 0 &&
          GET_LEVEL(ch) < SINFO.min_level[(int) GET_CLASS(ch)] ) 
  {
    send_to_char("You do not know that spell!\r\n", ch);
    return;
  }

  if (!IS_NPC(ch))
      if (GET_SKILL(ch, spellnum) == 0) {
          send_to_char("You are unfamiliar with that spell.\r\n", ch);
          return;
      }
  /* Find the target */
  if (t != NULL) {
    d = one_argument(strcpy(arg, t), t);
    skip_spaces(&t);
    skip_spaces(&d);
  }
  if (IS_SET(SINFO.targets, TAR_IGNORE)) {
    target = TRUE;
  }
  else if (t != NULL && *t) {
    if (!target && (IS_SET(SINFO.targets, TAR_CHAR_GROUP))) {
      if (!strncasecmp("group", t, strlen(t)))
	target = group = TRUE;
    }
    if (!target && (IS_SET(SINFO.targets, TAR_CHAR_ROOM))) {
      if ((tch = get_char_room_vis(ch, t)) != NULL)
	target = TRUE;
    }
    if (!target && (IS_SET(SINFO.targets, TAR_CHAR_DIR)) && *d) {
        if (( dir_num = search_block( d, castDirs, FALSE )) < 0) {
            send_to_char("What direction is that?\r\n", ch);
            return;
        }
        tch = find_vict_dir( ch, t, 4, dir_num );
        if (tch) target = TRUE;
    }
    if (!target && IS_SET(SINFO.targets, TAR_CHAR_WORLD))
      if ((tch = get_char_vis(ch, t, 0)))
	target = TRUE;

    if (!target && IS_SET(SINFO.targets, TAR_OBJ_INV))
      if ((tobj = get_obj_in_list_vis(ch, t, ch->carrying)))
	target = TRUE;

    if (!target && IS_SET(SINFO.targets, TAR_OBJ_EQUIP)) {
      for (i = 0; !target && i < NUM_WEARS; i++)
	if (ch->equipment[i] && !str_cmp(t, ch->equipment[i]->name)) {
	  tobj = ch->equipment[i];
	  target = TRUE;
	}
    }
    if (!target && IS_SET(SINFO.targets, TAR_OBJ_ROOM))
      if ((tobj = get_obj_in_list_vis(ch, t, world[ch->in_room].contents)))
	target = TRUE;

    if (!target && IS_SET(SINFO.targets, TAR_OBJ_WORLD))
      if ((tobj = get_obj_vis(ch, t)))
	target = TRUE;

	if (!target && IS_SET(SINFO.targets, TAR_CHAR_ZONE))
		if ((tch = get_char_vis(ch, t, 0)))
			if ( world[ch->in_room].zone == world[tch->in_room].zone )
			{
				if (spellnum != SPELL_MAGIC_MISSILE || IS_HUNTED(tch) )
					target = TRUE;
                                else tch = NULL;
			}

    if (!target && IS_SET(SINFO.targets, TAR_DIRECTION)) {
        if (( dir_num = search_block(t, castDirs, FALSE )) < 0) {
            send_to_char("What direction is that?\r\n", ch);
            return;
        }
        tobj = cast_directions + dir_num;
        target = TRUE;
    }

  } else {			/* if target string is empty */
    if (!target && IS_SET(SINFO.targets, TAR_FIGHT_SELF))
      if (FIGHTING(ch) != NULL) {
	tch = ch;
	target = TRUE;
      }
    if (!target && IS_SET(SINFO.targets, TAR_FIGHT_VICT))
      if (FIGHTING(ch) != NULL) {
	tch = FIGHTING(ch);
	target = TRUE;
      }
    /* if no target specified, and the spell isn't violent, default to self */
    if (!target && IS_SET(SINFO.targets, TAR_CHAR_ROOM) && !SINFO.violent) {
      tch = ch;
      target = TRUE;
    }
    if (!target) {
      sprintf(buf, "Upon %s should the spell be cast?\r\n",
	 IS_SET(SINFO.targets, TAR_OBJ_ROOM | TAR_OBJ_INV | TAR_OBJ_WORLD) ?
	      "what" : "who");
      send_to_char(buf, ch);
      return;
    }
  }

  if (target && (tch == ch) && SINFO.violent) {
    send_to_char("You shouldn't cast that on yourself -- could be bad for your health!\r\n", ch);
    return;
  }

  /* If they are trying to cast an aggressive spell on a non-combatant, stop them. */
  if (target && tch && SINFO.violent && chIsNonCombatant(tch)) {
	sendChar(ch, "%s is a non-combatant, you can't use violent magic on them.\r\n", GET_NAME(tch));
	return;
  }
  
  if (!target) {
    send_to_char("Cannot find the target of your spell!\r\n", ch);
    return;
  }
  if (group && ((SINFO.min_level[(int)GET_CLASS(ch)] + SINFO.grouplvl) > GET_LEVEL(ch)) && (GET_LEVEL(ch) < LVL_IMMORT)) {
    sendChar(ch, "You are not experienced enough to cast this spell on your group.\r\n");
    return;
  }
  mana = mag_manacost(ch, spellnum, FALSE);
  if (group && ((SINFO.min_level[(int)GET_CLASS(ch)] + (SINFO.grouplvl*2)) <= GET_LEVEL(ch)) )
    mana *= 2;
  else if (group)
    mana *= 3;
  if ((mana > 0) && (GET_MANA(ch) < mana) && (GET_LEVEL(ch) < LVL_IMMORT)) {
    send_to_char("You haven't the energy to cast that spell!\r\n", ch);
    return;
  }

  /* You throw the dice and you takes your chances.. 101% is total failure */
  if (!skillSuccess(ch, spellnum)) {
      WAIT_STATE(ch, PULSE_VIOLENCE);
      if (!tch || !skill_message(0, ch, tch, spellnum))
          send_to_char("You lost your concentration!\r\n", ch);
      if (mana > 0) {          
          GET_MANA(ch) = MAX(0, MIN(GET_MAX_MANA(ch), GET_MANA(ch) - (mana >> 1)));
      }
  }
  else {
      if (cast_spell(ch, tch, tobj, spellnum)) {
          advanceSkill( ch, spellnum, 90, "Your knowledge of the arcane has suddenly become more acute.", TRUE, TRUE, TRUE );
          if (!CHECK_WAIT(ch)) WAIT_STATE(ch, PULSE_VIOLENCE);
          if (mana > 0) {
              /* focus makes your spells randomly cheaper */
              if (GET_SKILL(ch, SKILL_FOCUS) > 0) {
                  int reduct = number(1, GET_SKILL(ch, SKILL_FOCUS) / 2);
                  /* plus, if the skill succeeds, you get a further boost */
                  if (skillSuccess(ch, SKILL_FOCUS) && percentSuccess(25)) {
                      reduct += number(1, GET_SKILL(ch, SKILL_FOCUS) / 2);
                      advanceSkill(ch, SKILL_FOCUS, 90, NULL, FALSE, FALSE, FALSE);
                  }
                  mana -= (mana * reduct) / 100;
              }

              GET_MANA(ch) = MAX(0, MIN(GET_MAX_MANA(ch), GET_MANA(ch) - mana));
          }
          if(skillSuccess(ch, SKILL_PENUMBRAE) && GET_CLASS(ch) == CLASS_SHADOW_DANCER) {
              level = MIN(spell_level(ch, SKILL_PENUMBRAE), 40);
              affect_from_char(ch, SKILL_PENUMBRAE);
              add_affect(ch, ch, SKILL_PENUMBRAE, level + 5, 0, 0, 25, 0, FALSE, FALSE, FALSE, FALSE);
              if(percentSuccess(5))
                  advanceSkill( ch, SKILL_PENUMBRAE, SKILL_PENUMBRAE_LEARN, "You and your shadow have more and more in common.", TRUE, TRUE, FALSE );
          }
      }
  }
}

/* Assign the spells on boot up */

void spello(int spl, int lv0, int lv1, int lv2, int lv3, int lv4, int lv5, int lv6, int lv7, int lv8, int lv9, int lv10,
	         int max_mana, int min_mana, int mana_change, int minpos,
	         int targets, int violent, int routines, byte grouplvl)
{
  spell_info[spl].min_level[CLASS_MAGIC_USER] = lv0;
  spell_info[spl].min_level[CLASS_CLERIC] = lv1;
  spell_info[spl].min_level[CLASS_THIEF] = lv2;
  spell_info[spl].min_level[CLASS_WARRIOR] = lv3;
  spell_info[spl].min_level[CLASS_RANGER] = lv4;
  spell_info[spl].min_level[CLASS_ASSASSIN] = lv5;
  spell_info[spl].min_level[CLASS_SHOU_LIN] = lv6;
  spell_info[spl].min_level[CLASS_SOLAMNIC_KNIGHT] = lv7;
  spell_info[spl].min_level[CLASS_DEATH_KNIGHT] = lv8;
  spell_info[spl].min_level[CLASS_SHADOW_DANCER] = lv9;
  spell_info[spl].min_level[CLASS_NECROMANCER] = lv10;
  spell_info[spl].mana_max = max_mana;
  spell_info[spl].mana_min = min_mana;
  spell_info[spl].mana_change = mana_change;
  spell_info[spl].min_position = minpos;
  spell_info[spl].targets = targets;
  spell_info[spl].violent = violent;
  spell_info[spl].routines = routines;
  spell_info[spl].grouplvl = grouplvl; /* Added by Vex. */
}

/*
 * Arguments for spello calls:
 *
 * spellnum, levels (MCTW), maxmana, minmana, manachng, minpos, targets,
 * violent?, routines.
 *
 * spellnum:  Number of the spell.  Usually the symbolic name as defined in
 * spells.h (such as SPELL_HEAL). levels  :  Minimum level (mage, cleric,
 * thief, warrior) a player must be to cast this spell.  Use 'X' for immortal
 * only. maxmana :  The maximum mana this spell will take (i.e., the mana it
 * will take when the player first gets the spell). minmana :  The minimum
 * mana this spell will take, no matter how high level the caster is.
 * manachng:  The change in mana for the spell from level to level.  This
 * number should be positive, but represents the reduction in mana cost as
 * the caster's level increases.
 *
 * minpos  :  Minimum position the caster must be in for the spell to work
 * (usually fighting or standing). targets :  A "list" of the valid targets
 * for the spell, joined with bitwise OR ('|'). violent :  TRUE or FALSE,
 * depending on if this is considered a violent spell and should not be cast
 * in PEACEFUL rooms or on yourself. routines:  A list of magic routines
 * which are associated with this spell. Also joined with bitwise OR ('|').
 *
 * See the CircleMUD documentation for a more detailed description of these
 * fields.
 */

#define XX LVL_IMMORT
#define ZZ (LVL_IMPL + 1)

void mag_assign_spells(void)
{
  int i;

  for (i = 1; i <= TOP_SPELL_DEFINE; i++)
    spello(i,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,0,0,0,0,0,0,0,ZZ);

  spello(SPELL_ARMOR,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX,  3, XX, XX, XX, XX, XX,  7,  7, XX, XX,
         30, 15, 3, POS_FIGHTING, TAR_CHAR_ROOM | TAR_CHAR_GROUP, FALSE, MAG_AFFECTS, 12);

  spello(SPELL_TELEPORT,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         400, 300, 10, POS_FIGHTING, TAR_CHAR_ROOM, TRUE, MAG_MANUAL, ZZ);

  spello(SPELL_BLESS,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX,  5, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         35, 5, 3, POS_FIGHTING, TAR_CHAR_ROOM | TAR_CHAR_GROUP, FALSE, MAG_AFFECTS, 15);

  spello(SPELL_BLINDNESS,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
          9, 11, XX, XX, XX, XX, XX, XX, 33, 13, 12,
         35, 25, 1, POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_NOT_SELF, TRUE, MAG_AFFECTS, ZZ);

  spello(SPELL_AIRSPHERE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
	 35, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         100, 65, 5, POS_STANDING, TAR_CHAR_ROOM | TAR_CHAR_GROUP, FALSE, MAG_AFFECTS, 8);

  spello(SPELL_BURNING_HANDS,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         40, 15, 5, POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE, ZZ);

  spello(SPELL_CALL_LIGHTNING,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, 18, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         40, 25, 3, POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE, ZZ);

  spello(SPELL_CHARM,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
          8, XX, XX, XX, XX, XX, XX, XX, XX, 30, XX,
         75, 50, 2, POS_FIGHTING, TAR_CHAR_ROOM | TAR_NOT_SELF, TRUE, MAG_MANUAL, ZZ);

  spello(SPELL_CHILL_TOUCH,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
          5, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         40, 10, 5, POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE | MAG_AFFECTS, ZZ);

  spello(SPELL_CLONE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         80, 65, 5, POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_MANUAL, ZZ);

  spello(SPELL_COLOR_SPRAY,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         20, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         45, 15, 3, POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE, ZZ);

  spello(SPELL_CONTROL_WEATHER,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         75, 25, 5, POS_STANDING, TAR_IGNORE, FALSE, MAG_MANUAL, ZZ);

  spello(SPELL_CREATE_FOOD,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX,  1, XX, XX, XX, XX, XX,  2, XX, XX, XX,
         30, 5, 4, POS_STANDING, TAR_IGNORE, FALSE, MAG_CREATIONS, ZZ);

  spello(SPELL_CREATE_WATER,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX,  2, XX, XX,  5, XX, XX,  2, XX, XX, XX,
         30, 5, 4, POS_STANDING, TAR_OBJ_INV | TAR_OBJ_EQUIP, FALSE, MAG_MANUAL, ZZ);

  spello(SPELL_CURE_BLIND,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, 11, XX, XX, XX, XX, XX, 33, XX, XX, XX,
         30, 5, 2, POS_FIGHTING, TAR_CHAR_ROOM | TAR_CHAR_GROUP, FALSE, MAG_UNAFFECTS, 15);

  spello(SPELL_CURE_CRITIC,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX,  8, XX, XX, XX, XX, XX, 35, XX, XX, XX,
         40, 20, 5, POS_FIGHTING, TAR_CHAR_ROOM | TAR_CHAR_GROUP, FALSE, MAG_POINTS, 10);

  spello(SPELL_CURE_LIGHT,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, 10, XX, XX, XX,
         40, 20, 4, POS_FIGHTING, TAR_CHAR_ROOM | TAR_CHAR_GROUP, FALSE, MAG_POINTS, 10);

  spello(SPELL_CURSE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, 23, 33, 15,
         80, 40, 4, POS_FIGHTING, TAR_CHAR_ROOM | TAR_OBJ_INV, TRUE, MAG_AFFECTS, ZZ);

  spello(SPELL_DETECT_ALIGN,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX,  4, XX, XX, XX, XX, XX,  5,  5, XX, XX,
         20, 10, 2, POS_FIGHTING, TAR_CHAR_ROOM | TAR_CHAR_GROUP, FALSE, MAG_AFFECTS, 15);

  spello(SPELL_DETECT_INVIS,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
          2,  9, XX, XX, XX, XX, XX, 13, 13, XX, 10,
         20, 10, 2, POS_FIGHTING, TAR_CHAR_ROOM | TAR_CHAR_GROUP, FALSE, MAG_AFFECTS, 15);

  spello(SPELL_DETECT_MAGIC,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
          2, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         20, 10, 2, POS_FIGHTING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS, ZZ);

  spello(SPELL_DETECT_POISON,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         15, 5, 1, POS_STANDING, TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE, MAG_MANUAL, ZZ);

  spello(SPELL_DISPEL_EVIL,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, 22, XX, XX, XX, XX, XX, 30, XX, XX, XX,
         40, 25, 3, POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE, ZZ);

  spello(SPELL_DISPEL_GOOD,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, 22, XX, XX, XX, XX, XX, XX, 30, XX, XX,
         40, 25, 3, POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE, ZZ);

  spello(SPELL_EARTHQUAKE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, 20, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         40, 25, 3, POS_FIGHTING, TAR_IGNORE, TRUE, MAG_AREAS, ZZ);

  spello(SPELL_ENCHANT_WEAPON,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         20, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         150, 100, 10, POS_STANDING, TAR_OBJ_INV | TAR_OBJ_EQUIP, FALSE, MAG_MANUAL, ZZ);

  spello(SPELL_LIFE_DRAIN,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         23, XX, XX, XX, XX, XX, XX, XX, XX, XX,  1,
         40, 25, 1, POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_NOT_SELF, TRUE, MAG_MANUAL, ZZ);

  spello(SPELL_GROUP_ARMOR,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         50, 30, 2, POS_STANDING, TAR_IGNORE, FALSE, MAG_GROUPS, ZZ);

  spello(SPELL_FIREBALL,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         45, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         50, 30, 5, POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE, ZZ);

  spello(SPELL_GROUP_HEAL,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         80, 60, 5, POS_FIGHTING, TAR_IGNORE, FALSE, MAG_GROUPS, ZZ);

  spello(SPELL_HARM,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, 25, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         75, 50, 3, POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_AFFECTS, ZZ);

  spello(SPELL_HEAL,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, 20, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         60, 40, 3, POS_FIGHTING, TAR_CHAR_ROOM | TAR_CHAR_GROUP, FALSE, MAG_POINTS | MAG_AFFECTS | MAG_UNAFFECTS, 15);

  spello(SPELL_INFRAVISION,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         11, 17, XX, XX,  9, 15, XX, XX, XX, XX, 17,
         25, 10, 1, POS_FIGHTING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS, ZZ);

  spello(SPELL_INVISIBLE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
          2, XX, XX, XX, XX, XX, XX, XX, XX,  5, 18,
         35, 25, 1, POS_STANDING, TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM | TAR_CHAR_GROUP, FALSE, MAG_AFFECTS, 15);

  spello(SPELL_LIGHTNING_BOLT,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         35, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         60, 40, 4, POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE, ZZ);

  spello(SPELL_LOCATE_OBJECT,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         18, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         80, 40, 4, POS_STANDING, TAR_OBJ_WORLD, FALSE, MAG_MANUAL, ZZ);

  spello(SPELL_MAGIC_MISSILE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
          1, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         40, 10, 10, POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_CHAR_DIR | TAR_CHAR_ZONE, TRUE, MAG_DAMAGE, ZZ);

  spello(SPELL_POISON,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, 23,  7,
         50, 20, 3, POS_STANDING, TAR_CHAR_ROOM | TAR_NOT_SELF | TAR_FIGHT_VICT | TAR_OBJ_INV, TRUE, MAG_AFFECTS, ZZ);

  spello(SPELL_PROT_FROM_EVIL,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, 10, XX, XX, XX,
         40, 10, 3, POS_FIGHTING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS, ZZ);

  spello(SPELL_REMOVE_CURSE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         15, 20, XX, XX, XX, XX, XX, 23, XX, XX, XX,
         45, 25, 5, POS_STANDING, TAR_CHAR_ROOM | TAR_OBJ_INV, FALSE, MAG_UNAFFECTS|MAG_ALTER_OBJS, ZZ);

  spello(SPELL_SANCTUARY,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, 30, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         110, 85, 5, POS_STANDING, TAR_CHAR_ROOM | TAR_CHAR_GROUP, FALSE, MAG_AFFECTS, 10);

  spello(SPELL_SHOCKING_GRASP,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         10, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         45, 20, 5, POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE, ZZ);

  spello(SPELL_SLEEP,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
          7, XX, XX, XX, XX, XX, XX, XX, XX, 13, XX,
         40, 25, 5, POS_STANDING, TAR_CHAR_ROOM, TRUE, MAG_AFFECTS, ZZ);

  spello(SPELL_STRENGTH,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
          9, XX, XX, XX, 21, XX, XX, XX, XX, XX, XX,
         40, 20, 2, POS_STANDING, TAR_CHAR_ROOM | TAR_CHAR_GROUP, FALSE, MAG_AFFECTS, 15);

  spello(SPELL_SUMMON,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
          5, 16, XX, XX, XX, XX, XX, XX, XX, XX, 14,
         75, 50, 3, POS_STANDING, TAR_CHAR_WORLD | TAR_NOT_SELF, FALSE, MAG_MANUAL, ZZ);

  spello(SPELL_WORD_OF_RECALL,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX,  6, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         20, 10, 2, POS_FIGHTING, TAR_CHAR_ROOM | TAR_CHAR_GROUP, FALSE, MAG_MANUAL, 20);

  spello(SPELL_REMOVE_POISON,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, 14, XX, XX,  9, XX, XX, 18, XX, XX, XX,
         40, 8, 4, POS_STANDING, TAR_CHAR_ROOM | TAR_OBJ_INV, FALSE, MAG_UNAFFECTS, ZZ);

  spello(SPELL_SENSE_LIFE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, 13, XX, XX, 18, 23, XX, XX, XX, 28, 21,
         20, 10, 2, POS_FIGHTING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS, ZZ);

  spello(SPELL_FLY,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         12, 18, XX, XX, XX, XX, XX, XX, XX, XX, 11,
         30, 15, 3, POS_STANDING, TAR_CHAR_ROOM | TAR_CHAR_GROUP, FALSE, MAG_AFFECTS, 10);

  spello(SPELL_BARKSKIN,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, 12, XX, XX, XX, XX, XX, XX,
         35, 20, 3, POS_FIGHTING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS, 10);

  spello(SPELL_AWAKEN,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         21, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         55, 25, 3, POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS, 15);

  spello(SPELL_REGENERATE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, 18, XX, XX, XX, XX, XX, 28, 28, XX, XX,
         50, 40, 5, POS_STANDING, TAR_CHAR_ROOM | TAR_CHAR_GROUP, FALSE, MAG_AFFECTS, 15);

  spello(SPELL_ICE_STORM,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         25, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         80, 50, 3, POS_FIGHTING, TAR_IGNORE, TRUE, MAG_AREAS, ZZ);

  spello(SPELL_METEOR_SWARM,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         40, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         90, 60, 5, POS_FIGHTING, TAR_IGNORE, TRUE, MAG_AREAS, ZZ);

  spello(SPELL_PORTAL,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         37, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
#ifdef CAN_PORTAL_TO_OBJ
         250, 200, 10, POS_STANDING, TAR_CHAR_WORLD|TAR_OBJ_WORLD, FALSE, MAG_MANUAL, ZZ);
#else
         250, 200, 10, POS_STANDING, TAR_CHAR_WORLD, FALSE, MAG_MANUAL, ZZ);
#endif

  spello(SPELL_CAUSE_WOUND,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX,  2, XX, XX,
         20, 8, 2, POS_FIGHTING, TAR_CHAR_ROOM|TAR_FIGHT_VICT,TRUE, MAG_DAMAGE, ZZ);

  spello(SPELL_MALEDICT,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, 4, XX, XX, XX, XX, XX, XX, XX, XX, 4,
         45, 40, 1, POS_FIGHTING, TAR_CHAR_ROOM|TAR_FIGHT_VICT,TRUE, MAG_AFFECTS, ZZ);

  spello(SPELL_CAUSE_CRITIC,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         20, 10, 2, POS_FIGHTING, TAR_CHAR_ROOM|TAR_FIGHT_VICT,TRUE, MAG_DAMAGE, ZZ);

  spello(SPELL_CURE_SERIOUS,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX,  XX, XX, XX, XX, XX, XX, 25, XX, XX, XX,
         40, 20, 5, POS_FIGHTING, TAR_CHAR_ROOM, FALSE, MAG_POINTS, ZZ);

  spello(SPELL_CALM,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, 37, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         120, 80, 4, POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_MANUAL, ZZ);

  spello(SPELL_CHAIN_LIGHTNING,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, 33, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         100, 80, 5, POS_FIGHTING, TAR_IGNORE, TRUE, MAG_AREAS, ZZ);

  spello(SPELL_BANISH,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, 30, XX, XX, XX, XX, XX, 33, XX, XX, XX,
         110, 50, 3, POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_MANUAL, ZZ);

  /* Jan 24, 1995 Liam
   *
   * Made demon fire and flame strike single target spells
   *  
   */
  spello(SPELL_DEMON_FIRE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, 40, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         100, 50, 5, POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE, ZZ);

  spello(SPELL_FLAME_STRIKE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, 40, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         100, 50, 5, POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE, ZZ);

  spello(SPELL_PROT_FROM_GOOD,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, 10, XX, XX,
         40, 10, 3, POS_FIGHTING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS, ZZ);

  spello(SPELL_SHIELD,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         10, XX, XX, XX, 18, XX, XX, XX, XX, XX, 9,
         40, 20, 2, POS_FIGHTING, TAR_CHAR_ROOM | TAR_CHAR_GROUP, FALSE, MAG_AFFECTS, 15);

  spello(SPELL_ARMOR_OF_CHAOS,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, 30, XX, XX,
         50, 20, 5, POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS, ZZ);

  spello(SPELL_HOLY_ARMOR,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, 30, XX, XX, XX,
         50, 20, 5, POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS, ZZ);

  spello(SPELL_CREATE_SPRING,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         10, 35, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         70, 40, 5, POS_STANDING, TAR_IGNORE, FALSE, MAG_CREATIONS, ZZ);

  spello(SPELL_MINOR_CREATION,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         50, 10, 5, POS_STANDING, TAR_IGNORE, FALSE, MAG_MANUAL, ZZ);

  spello(SPELL_SAND_STORM,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         29, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         110, 50, 3, POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_MANUAL, ZZ);

  spello(SPELL_SILENCE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, 33,
         70, 30, 5, POS_STANDING, TAR_CHAR_ROOM, TRUE, MAG_AFFECTS, ZZ);

  spello(SPELL_WEB,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         6, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         70, 40, 5, POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_AFFECTS, ZZ);
         
  spello(SPELL_PARALYZE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         100, 70, 5, POS_STANDING, TAR_CHAR_ROOM, TRUE, MAG_AFFECTS, ZZ);

  spello(SPELL_HASTE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         33, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         150, 100, 2, POS_STANDING, TAR_CHAR_ROOM |  TAR_CHAR_GROUP,
         FALSE, MAG_AFFECTS, 5);

  spello(SPELL_SLOW,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, 32,
         80, 40, 5, POS_STANDING, TAR_CHAR_ROOM, TRUE, MAG_AFFECTS, ZZ);

  spello(SPELL_SHRIEK,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         30, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         50, 30, 4, POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE, ZZ);

  spello(SPELL_MONSTER_SUMMON,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
          5, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         120, 90, 5, POS_FIGHTING, TAR_IGNORE, FALSE, MAG_SUMMONS, ZZ);

  spello(SPELL_REMOVE_PARALYSIS,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, 27, XX, XX, XX, XX, XX, 35, XX, XX, XX,
         50, 20, 5, POS_FIGHTING, TAR_CHAR_ROOM | TAR_CHAR_GROUP, FALSE, MAG_UNAFFECTS, ZZ);

  spello(SPELL_CONJURE_ELEMENTAL,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         30, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         120, 80, 5, POS_FIGHTING, TAR_IGNORE, FALSE, MAG_SUMMONS, ZZ);

  spello(SPELL_GATE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         40, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         300, 200, 10, POS_FIGHTING, TAR_IGNORE, FALSE, MAG_SUMMONS, ZZ);

  spello(SPELL_GATE_MAJOR_DEMON,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         200, 150, 10, POS_FIGHTING, TAR_IGNORE, FALSE, MAG_SUMMONS, ZZ);

  spello(SPELL_ANIMATE_DEAD,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX,  6, XX, XX,
         120, 80, 1, POS_FIGHTING, TAR_OBJ_ROOM, FALSE, MAG_SUMMONS, ZZ);

  spello(SPELL_FORGET,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         50, 30, 5, POS_STANDING, TAR_CHAR_ROOM, TRUE, MAG_MANUAL, ZZ);

  spello(SPELL_GROUP_RECALL,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         80, 60, 5, POS_FIGHTING, TAR_IGNORE, FALSE, MAG_GROUPS, ZZ);

  spello(SPELL_GROUP_SANCTUARY,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         200, 140, 5, POS_STANDING, TAR_IGNORE, FALSE, MAG_GROUPS, ZZ);

  spello(SPELL_REFRESH,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, 35, XX, XX, XX, XX, XX, XX,
         50, 30, 2, POS_STANDING, TAR_CHAR_ROOM | TAR_CHAR_GROUP, FALSE, MAG_POINTS, 10);

  spello(SPELL_TRUE_SIGHT,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         31, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         70, 40, 2, POS_FIGHTING, TAR_CHAR_ROOM | TAR_CHAR_GROUP, FALSE, MAG_AFFECTS, 15);

  spello(SPELL_BALL_OF_LIGHT,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         01, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         50, 20, 5, POS_STANDING, TAR_IGNORE, FALSE, MAG_CREATIONS, ZZ);

  spello(SPELL_FEAR,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, 33, XX, XX,
         70, 30, 4, POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_AFFECTS, ZZ);

  spello(SPELL_FLEET_FOOT,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, 36, XX, XX, XX, XX, XX, XX,
         50, 30, 2, POS_STANDING, TAR_CHAR_ROOM | TAR_CHAR_GROUP, FALSE, MAG_AFFECTS, 10);

  spello(SPELL_HANDS_OF_WIND,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, 45, XX, XX, XX, XX, XX, XX,
         55, 35, 4, POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_MANUAL, ZZ);

  spello(SPELL_FIRE_BREATH,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         50, 40, 5, POS_FIGHTING, TAR_IGNORE, TRUE, MAG_AREAS, ZZ);

  spello(SPELL_FROST_BREATH,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         50, 40, 5, POS_FIGHTING, TAR_IGNORE, TRUE, MAG_AREAS, ZZ);

  spello(SPELL_ACID_BREATH,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         50, 40, 5, POS_FIGHTING, TAR_IGNORE, TRUE, MAG_AREAS, ZZ);

  spello(SPELL_GAS_BREATH,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         50, 40, 5, POS_FIGHTING, TAR_IGNORE, TRUE, MAG_AREAS, ZZ);

  spello(SPELL_LIGHTNING_BREATH,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         50, 40, 5, POS_FIGHTING, TAR_IGNORE, TRUE, MAG_AREAS, ZZ);

  spello(SPELL_YOUTHEN,
  /*    Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
        XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
        50, 40, 5, POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_MANUAL, ZZ);

  spello(SPELL_KNOWLEDGE,
  /*    Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
        XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
        50, 40, 5, POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_MANUAL, ZZ);

  spello(SPELL_RECHARGE,
  /*    Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
        XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
        50, 40, 5, POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_MANUAL, ZZ);
  
  spello(SPELL_PLAGUE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         50, 20, 3, POS_STANDING, TAR_CHAR_ROOM | TAR_NOT_SELF | TAR_OBJ_INV, TRUE, MAG_AFFECTS, ZZ);

  spello(SPELL_CURE_PLAGUE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, 35, XX, XX, XX, XX, XX, 37, XX, XX, XX,
         100, 25, 5, POS_STANDING, TAR_CHAR_ROOM | TAR_CHAR_GROUP, FALSE, MAG_UNAFFECTS, 10);

  spello(SPELL_RELOCATE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, 45, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         200, 20, 2, POS_STANDING, TAR_CHAR_WORLD | TAR_NOT_SELF, FALSE, MAG_MANUAL, ZZ);

  spello(SPELL_GROUP_HASTE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         100, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         500, 300, 11, POS_STANDING, TAR_IGNORE, FALSE, MAG_GROUPS, ZZ);

  spello(SPELL_BLUR,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, 19, XX,
         45, 15, 1, POS_FIGHTING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS, ZZ);

  spello(SPELL_REVIVE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, 45, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         75, 50, 5, POS_FIGHTING, TAR_CHAR_ROOM, FALSE, MAG_POINTS | MAG_AFFECTS | MAG_UNAFFECTS, ZZ);
  
  spello(SPELL_CHANGE_ALIGN,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         80, 65, 5, POS_STANDING, TAR_SELF_ONLY, FALSE, MAG_POINTS, ZZ);

  spello(SPELL_CALL_OF_THE_WILD,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, 22, XX, XX, XX, XX, XX, XX,
         120, 80, 2, POS_FIGHTING, TAR_IGNORE, FALSE, MAG_SUMMONS, ZZ);

  spello(SPELL_DOOM_BOLT,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         50, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         25, 25, 5, POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE, ZZ);

  spello(SPELL_WRATH_OF_THE_ANCIENTS,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         50, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         50, 50, 5, POS_FIGHTING, TAR_IGNORE, TRUE, MAG_AREAS, ZZ); 

  spello(SPELL_PRAYER_OF_LIFE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, 45, XX, XX, XX,
         60, 30, 6, POS_FIGHTING, TAR_CHAR_ROOM, FALSE, MAG_POINTS, ZZ);

  spello(SPELL_HOLY_WORD,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, 43, XX, XX, XX, XX, XX, 45, XX, XX, XX,
         140, 90, 10, POS_FIGHTING, TAR_IGNORE, TRUE, MAG_AREAS, ZZ);

  spello(SPELL_UNHOLY_WORD,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, 43, XX, XX, XX, XX, XX, XX, 45, XX, XX,
         140, 90, 10, POS_FIGHTING, TAR_IGNORE, TRUE, MAG_AREAS, ZZ);

  spello(SPELL_BLACK_DART,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, 10, XX, XX,
         40, 20, 4, POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_CHAR_DIR, TRUE, MAG_DAMAGE | MAG_AFFECTS, ZZ);

  spello(SPELL_BLACK_BREATH,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, 35, XX, XX,
         60, 30, 5, POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE | MAG_AFFECTS, ZZ);

  spello(SPELL_DEATH_TOUCH,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, 45, XX, XX,
         40, 40, 5, POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE | MAG_AFFECTS, ZZ);

  spello(SPELL_EYES_OF_THE_DEAD,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, 40, XX, XX,
         70, 40, 3, POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS, ZZ);

  spello(SPELL_RIGHTEOUS_VISION,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, 40, XX, XX, XX,
         70, 40, 3, POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS, ZZ);

  spello(SPELL_WARD,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, 10, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         60, 40, 4, POS_STANDING, TAR_CHAR_ROOM | TAR_CHAR_GROUP, FALSE, MAG_AFFECTS, 10);

  spello(SPELL_GROUP_WARD,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         120, 80, 4, POS_STANDING, TAR_IGNORE, FALSE, MAG_GROUPS, ZZ);

  spello(SPELL_SHADOW_VISION,
  /*	 Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
	 XX, XX, XX, XX, XX, XX, XX, XX, XX,  8, XX,
	 60, 30, 2, POS_FIGHTING, TAR_CHAR_ROOM | TAR_CHAR_GROUP | TAR_CHAR_WORLD, FALSE, MAG_AFFECTS, 27);

  spello(SPELL_SHADOW_BLADES,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
	 XX, XX, XX, XX, XX, XX, XX, XX, XX,  4, XX,
	 60, 30, 4, POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE, ZZ);

  spello(SPELL_GROUP_SHADOW_VISION,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
	 XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	 100, 60, 5, POS_STANDING, TAR_IGNORE, FALSE, MAG_GROUPS, ZZ);

  spello(SPELL_SHADOW_SPHERE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
	 XX, XX, XX, XX, XX, XX, XX, XX, XX, 15, XX,
	 95, 25, 2, POS_FIGHTING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS, ZZ);

  spello(SPELL_SHADOW_WALK,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, 20, XX,
         30, 15, 3, POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS, ZZ);

  spello(SPELL_IDENTIFY,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         11, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         60,  20,  2, POS_STANDING,  TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE, MAG_MANUAL, ZZ);

  spello(SPELL_GROUP_FLY,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
	 XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	 100, 60, 5, POS_STANDING, TAR_IGNORE, FALSE, MAG_GROUPS, ZZ);

  spello(SPELL_GROUP_INVISIBILITY,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
	 XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	 100, 60, 5, POS_STANDING, TAR_IGNORE, FALSE, MAG_GROUPS, ZZ);
	 
  spello(SPELL_BLINK,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         41, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         90, 50, 10, POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE,
         MAG_AFFECTS, ZZ);
         
  spello(SPELL_PESTILENCE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, 30, XX, XX,
         100, 50, 5, POS_FIGHTING, TAR_IGNORE, TRUE, MAG_AREAS, ZZ);
  
  spello(SPELL_ENTANGLE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, 31, XX, XX, XX, XX, XX, XX,
         60, 35, 5, POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE | MAG_AFFECTS, ZZ);

  spello(SPELL_FEEBLEMIND,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         48, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         90, 20, 50, POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_AFFECTS, ZZ);

  spello(SPELL_FLAME_BLADE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         17, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         80, 30, 5, POS_STANDING, TAR_CHAR_ROOM | TAR_CHAR_GROUP, FALSE, MAG_AFFECTS, 10);
          
  spello(SPELL_NEXUS,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         35, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         200, 20, 100, POS_STANDING, TAR_CHAR_WORLD | TAR_NOT_SELF, FALSE, MAG_MANUAL, ZZ);

  spello(SPELL_ASSISTANT,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, 34, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         150, 100, 5, POS_FIGHTING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS, ZZ);
  
  spello(SPELL_SAGACITY,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, 38, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         90, 20, 50, POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS, ZZ);

  spello(SPELL_CLEANSE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, 41, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         130, 80, 10, POS_FIGHTING, TAR_CHAR_ROOM | TAR_CHAR_GROUP, FALSE, MAG_MANUAL, 9);

  spello(SPELL_FORTIFY,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, 31, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         180, 130, 10, POS_STANDING, TAR_CHAR_ROOM , FALSE, MAG_AFFECTS, ZZ);

  spello(SPELL_DISHEARTEN,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, 39, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         50, 25, 5, POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT |TAR_NOT_SELF, TRUE, MAG_AFFECTS, ZZ);

  spello(SPELL_SACRIFICE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, 48, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         300, 300, 0, POS_FIGHTING, TAR_CHAR_ROOM | TAR_NOT_SELF, FALSE, MAG_MANUAL, ZZ);

  spello(SPELL_PULSE_HEAL,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         100, 100, 0, POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS, ZZ);

  spello(SPELL_PULSE_GAIN,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         100, 100, 0, POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS, ZZ);

  spello(SPELL_DANCE_SHADOWS,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, 19, XX,
	 40, 20, 2, POS_FIGHTING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS, ZZ);

  spello(SPELL_DANCE_DREAMS,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, 29, XX,
         40, 25, 5, POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_AFFECTS, ZZ);

  spello(SPELL_DANCE_MISTS,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, 39, XX,
         35, 25, 1, POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_NOT_SELF, TRUE, MAG_AFFECTS, ZZ);

  spello(SPELL_CRUSADE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, 42, XX, XX, XX,
         50, 25, 4, POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS, ZZ);

  spello(SPELL_UNUSED,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         100, 75, 5, POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS, ZZ);

  spello(SPELL_APOCALYPSE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, 27, XX, XX,
         75, 50, 5, POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS, ZZ);

  spello(SPELL_MISSION,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, 27, XX, XX, XX,
         75, 50, 5, POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS, ZZ);

  spello(SPELL_FOREST_LORE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, 25, XX, XX, XX, XX, XX, XX,
         40, 40, 5, POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS, ZZ);

  spello(SPELL_SWARM,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, 27, XX, XX, XX, XX, XX, XX,
         60, 30, 2, POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE | MAG_AFFECTS, ZZ);

  spello(SPELL_WALL_OF_FIRE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         60, 10, 2, POS_FIGHTING, TAR_IGNORE, TRUE, MAG_AREAS, ZZ);

  spello(SPELL_TREMOR,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         60, 10, 2, POS_FIGHTING, TAR_IGNORE, TRUE, MAG_AREAS, ZZ);

  spello(SPELL_TSUNAMI,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         60, 10, 2, POS_FIGHTING, TAR_IGNORE, TRUE, MAG_AREAS, ZZ);

  spello(SPELL_TYPHOON,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         60, 10, 2, POS_FIGHTING, TAR_IGNORE, TRUE, MAG_AREAS, ZZ);

  spello(SPELL_TERROR,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         60, 10, 2, POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_AFFECTS | MAG_DAMAGE, ZZ);

  spello(SPELL_FAST_LEARNING,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         10, 10, 0, POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS, ZZ);

  spello(SKILL_SHOWDAM,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         10, 10, 0, POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS, ZZ);

  spello(SPELL_DISPEL_MAGIC,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         10, 10, 0, POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_AFFECTS, ZZ);

  spello(SPELL_CONSUME_CORPSE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, 25,
         90, 70, 3, POS_STANDING, TAR_OBJ_ROOM, TRUE, MAG_MANUAL, ZZ);

  spello(SPELL_EXPLODE_CORPSE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, 41,
         90, 60, 4, POS_FIGHTING, TAR_OBJ_ROOM, TRUE, MAG_MANUAL, ZZ);

  spello(SPELL_CREATE_WARDING,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, 37,
         70, 50, 5, POS_FIGHTING, TAR_IGNORE, FALSE, MAG_CREATIONS, ZZ);

  spello(SPELL_BONE_WALL,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, 43,
         200, 100, 15, POS_FIGHTING, TAR_DIRECTION, FALSE, MAG_MANUAL, ZZ);

  spello(SPELL_WRAITHFORM,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, 38,
         200, 120, 20, POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS, ZZ);

  spello(SPELL_NOXIOUS_SKIN,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, 28,
         45, 35, 1, POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS, 8);

  spello(SPELL_DISEASE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, 45,
         90, 80, 5, POS_STANDING, TAR_CHAR_ROOM | TAR_NOT_SELF, TRUE, MAG_AFFECTS, ZZ);

  spello(SPELL_SUMMON_CORPSE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         200, 200, 0, POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_MANUAL, 15);

  spello(SPELL_EMBALM,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, 26,
         40, 25, 1, POS_FIGHTING, TAR_OBJ_ROOM, FALSE, MAG_MANUAL, ZZ);

  spello(SPELL_CHARM_CORPSE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, 23,
         65, 35, 3, POS_FIGHTING, TAR_OBJ_ROOM, FALSE, MAG_MANUAL, ZZ);

  spello(SPELL_ENTOMB_CORPSE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, 40,
         125, 75, 5, POS_STANDING, TAR_OBJ_ROOM | TAR_CHAR_ROOM, FALSE, MAG_MANUAL, ZZ);

    spello(SPELL_CALL_TO_CORPSE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, 47,
         100, 50, 5, POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_AFFECTS, ZZ);

  spello(SPELL_RESIST_POISON,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,  8,
         40, 20, 4, POS_FIGHTING, TAR_CHAR_ROOM | TAR_CHAR_GROUP, FALSE, MAG_AFFECTS, 15);

  spello(SPELL_AGE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, 20,
         40, 30, 1, POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS, 20);

  spello(SPELL_QUICKEN,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         125, 125, 1, POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS, ZZ);

  spello(SPELL_ENERGY_DRAIN,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, 31,
         40, 25, 5, POS_STANDING, TAR_CHAR_ROOM, TRUE, MAG_AFFECTS, ZZ);

  spello(SPELL_SOUL_PIERCE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, 3,
         40, 30, 1, POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE, ZZ);

  spello(SPELL_DEBILITATE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, 35,
         80, 40, 5, POS_STANDING, TAR_CHAR_ROOM | TAR_NOT_SELF, TRUE, MAG_AFFECTS, ZZ);

  spello(SPELL_FOUNT_OF_YOUTH,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         200, 200, 0, POS_STANDING, TAR_IGNORE, FALSE, MAG_CREATIONS, ZZ);

  spello(SPELL_CALL_STEED,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         60, 60, 0, POS_STANDING, TAR_IGNORE, FALSE, MAG_MANUAL, ZZ);

  spello(SPELL_REFLECTION,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         35, 35, 0, POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS, ZZ);

  spello(SPELL_FLETCH,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
         15, 15, 0, POS_STANDING, TAR_IGNORE, FALSE, MAG_MANUAL, ZZ);


  /*
   *
   *
   * SKILLS
   * 
   * The only parameters needed for skills are only the minimum levels for each
   * class.  The remaining 8 fields of the structure should be filled with
   * 0's, except the VIOLENT field.
   */

  /* Ma  Cl  Th  Wa  */
  spello(SKILL_BACKSTAB,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX,  5, XX, XX, 1, XX, XX, XX, 10, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_BASH,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX,  1, XX, XX, XX,  9,  9, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_HIDE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX,  4, XX,  3,  1, XX, XX, XX,  3, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_KICK,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, 15,  1, 12,  9,  1,  1,  1, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_PICK_LOCK,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX,  5, XX, XX, 20, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_RESCUE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX,  2, 20, XX, XX,  5,  5, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_SNEAK,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX,  2, XX,  2,  5,  2, XX, XX,  1, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_STEAL,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX,  1, XX, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_MUG,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, 33, XX, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_TRACK,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, 30, XX,  1, 17, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_DISARM,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, 10, 10, 24, XX, 15, 20, 20, 22, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_SECOND_ATTACK,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, 22,  5, 15,  8,  9, 15, 15, 25, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_THIRD_ATTACK,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, 35, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_SCAN,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         35, 35, 10, 35, 20,  9, 35, 35, 35, 30, 35,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_LAY_HANDS,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, 01, XX, XX, XX, XX,
         60, 40, 3, POS_FIGHTING, TAR_CHAR_ROOM, FALSE, MAG_POINTS | MAG_AFFECTS | MAG_UNAFFECTS, ZZ);

  spello(SKILL_FISTS_OF_FURY,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, 33, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_THROW,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, 15, XX, XX, 18, XX, XX, XX, 15, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_SHOOT,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, 10,  2, 39, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_KNOCK,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, 23, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_TRIP,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX,  3, XX, 15, 13, XX, XX, XX, 16, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_BLINDING_STRIKE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, 18, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_HAMSTRING,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, 12, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_ENHANCED_DAMAGE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, 25, 25, 20, XX, 25, 25, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_RETREAT,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, 15,  5, XX,  5, 15, 15, XX, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_TURN,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX,  6, XX, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_BUTCHER,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX,  3, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_DODGE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, 18, XX, 14, 15,  9, XX, XX, 18, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_TRAP,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, 16, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_BLACKJACK,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, 42, XX, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_SEARCH_TRAP,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, 38, XX, XX, 16, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_PALM,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, 20, XX, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_FIND_WEAKNESS,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, 13, XX, XX, 10, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_SKIN,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_FEIGN_DEATH,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, 40, XX, XX, XX, 27, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_ART_WIND,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, 20, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_ART_TIGER,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, 25, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_ART_SNAKE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, 30, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_ART_MONKEY,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, 35, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_ART_CRANE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, 40, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_ART_FLOWER,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, 45, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_ART_DRAGON,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, 50, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_CIRCLE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, 25, XX, XX, 25, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_DUST,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, 25, XX, XX, 29, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_STALK,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, 20, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_ENVENOM,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, 35, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_PARRY,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, 30, 28, 40, 28, XX, 38, 38, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_RIPOSTE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, 50, 50, XX, XX, 50, 50, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_SWEEP,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, 12, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_DOORBASH,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, 38, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_PENUMBRAE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, 6, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_PICKPOCKET,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_APPRAISE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_DELUSION,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, 35, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_CUTPURSE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_COWER,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, 35, XX, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_DANGER_SENSE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_THIEF_SENSE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_ASSASSINATION,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, 40, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_DIRTY_TACTICS,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, 29, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_AGGRESSIVE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, 30, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_SHADOW_DANCE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
	 XX, XX, XX, XX, XX, XX, XX, XX, XX, 40, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_SHADOW_STEP,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */  
	 XX, XX, XX, XX, XX, XX, XX, XX, XX, 42, XX,
         0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_SHADOW_MIST,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, 44, XX,
         0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_GORE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
	 XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_BREATHE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
	 XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_BERSERK,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
	 XX, XX, XX, 45, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_GUARD,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
	 XX, XX, XX, 20, XX, XX, XX, 20, 20, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_HEIGHTENED_SENSES,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
	 XX, XX, XX, XX, 35, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

   spello(SKILL_AMBUSH,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, 43, XX, XX, XX, XX, XX,
         0, 0, 0, 0, 0, TRUE, 0, ZZ);

 spello(SKILL_CUT_THROAT,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, 30, XX, XX, XX, XX, XX, XX, XX, XX,
         0, 0, 0, 0, 0, TRUE, 0, ZZ);
         
  spello(SKILL_CONVERT,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, 25, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);
         
  spello(SKILL_FAMILIARIZE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, 32, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_ESCAPE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, 38, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_DANCE_DEATH,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, 49, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_SHADOWBOX,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, 46, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_FENCE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, 16, XX, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_SPY,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, 29, XX, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_DISTRACT,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, 27, XX, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_RETARGET,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, 47, XX, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_DEVOUR,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, 42, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_BLOCK,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, 36, 36, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_SHIELD_BASH,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, 32, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_INVIGORATE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, 20, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_REDOUBT,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, 48, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_WEAPON_MASTERY,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, 47, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_STEADFASTNESS,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, 40, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_SCOUT,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, 33, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_BULLSEYE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, 27, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_EXPOSE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, 41, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_CAMP,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, 46, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_POULTICE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, 28, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_CALM,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
	 XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_FEED,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
	 XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_STING,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
	 XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_MIST,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
	 XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_BATTLECRY,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
	 XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_WARCRY,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
	 XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_STANCE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
	 XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_POWERSTRIKE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
	 XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_FOCUS,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
	 XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_DEVOTION,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
	 XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_FERVOR,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
	 XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_FALSE_TRAIL,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
	 XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_ENHANCED_STEALTH,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
	 XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_SHADOW_JUMP,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
	 XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_EVASION,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
	 XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_CRITICAL_HIT,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
	 XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_ADRENALINE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
	 XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_BEFRIEND,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
	 XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, FALSE, 0, ZZ);

  spello(SKILL_CHARGE,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
	 XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);

  spello(SKILL_FLASHBANG,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX,  45, XX, XX, XX, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);
  
  spello(SKILL_INSTANT_POISON,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, 45, XX, XX, XX, XX, XX,
	 0, 0, 0, 0, 0, TRUE, 0, ZZ);
  /*
   *
   * SONGS
   * 
   * For songs, the minimum levels should be filled out as per skills
   * and spells.  The maximum mana becomes initial move cost, and the
   * minimum mana the continuing move cost.  The manachg field is
   * unused, and should be set to zero.  The minpos is actually a
   * maxpos field - all songs can be sung while resting or standing,
   * but only songs with a maxpos of POS_FIGHTING can be sung while
   * fighting.  The violent field is used to prevent aggressive songs
   * being sung where they should not.  The routines field is used to
   * flag what can end this song, according to the SST_ constants,
   * and the groups field has the same meaning as for spells.
   *
   * The following values are acceptable in the target field:
   * TAR_CHAR_ROOM      - can be sung to a character in this room
   * TAR_CHAR_WORLD     - can be sung to a character in the realm
   * TAR_CHAR_GROUP     - can be sung to the whole group
   * TAR_SELF_ONLY      - can only be sung to oneself
   * TAR_NOT_SELF       - cannot be sung to oneself
   */

  spello(SONG_MINOR_REFRESHMENT,
  /*     Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  ??  ??  ??  ??  ?? */
         XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,  2,
	 10, 3, 0, POS_STANDING, TAR_CHAR_ROOM | TAR_CHAR_GROUP, FALSE,
         SST_FIGHTING | SST_MOVEMENT, 18);

}

