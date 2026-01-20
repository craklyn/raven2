/* ============================================================================
combspec.c
Special procedures for mobs and weapons that are invoked only during combat.
Written by Vex of RavenMUD for RavenMUD.
Notes on use:
As soon as the procedure returns, "damage" is  subtracted from the victims
hit point total, and the appropriate message for the attacktype and the
damage will be printed.
You can alter the attacktype to "TYPE_SPECIAL", and this will suppress the
usual damage message being printed. This can be useful if the procedure is
going to transport the victim or something.
Be sure to note that if the damage would cause the victims position to
change, the effects of that will immediately occur after the procedure
returns. e.g.  To "kill" the victim, all you need to do is make sure their
hps will be below -10, and they will "die" after the procedure returns.

Steps to make a new procedure:
------------------------------
1) write the procedure itself. Note the parameters MUST have the same types
as the others.
2) Class the procedure as defensive(will be invoked for the victim of an
attack) or offensive(will be invoked for the attacker when they make an attack)
All this requires is for you to add a reference to the procedure in the
appropriate function -> see "isOffensiveObj" for typical examples. The other
functions are "isDefensiveObj", "isOffensiveMob" and "isDefensiveMob".
3) Assign the procedure to the obj/mob thats going to have it in
assignCombatSpecials.
If the procedure is for an object, use "specialObjCombat" to assign it,
otherwise use "specialMobCombat".
4) The last finishing touch is to add the procedures name to "combSpecName"
============================================================================ */
#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "util/utils.h"
#include "specials/combspec.h"
#include "specials/classes.h"
#include "general/class.h"
#include "magic/spells.h"
#include "general/comm.h"
#include "magic/magic.h"
#include "actions/fight.h"
#include "actions/offensive.h"
#include "general/handler.h"
#include "scripts/dg_scripts.h"
#include "olc/oedit.h"
#include "magic/skills.h"

/* ============================================================================
attackerSpecial
This routine invokes a combatSpec function for the attacker. All offensive
procedures must come through here.
Note: only 1 combatSpec function can be invoked SUCCESSFULLY at a time.
============================================================================ */
/* proto-types used in this file. */
int isOffensiveObj(ObjData *obj);
int isOffensiveMob(CharData *mob);
int combspec(CharData *ch);

int attackerSpecial(CharData *attacker, CharData *victim, int *damage, int *attacktype)
{
  int i;

  /* Check for combatSpec on equipment. */
  for (i = 0; i < NUM_WEARS; i++)
      if (
	  attacker->equipment[i] &&
	  isOffensiveObj(attacker->equipment[i])
	 )
	    if ( obj_index[attacker->equipment[i]->item_number].combatSpec(attacker->equipment[i], victim, damage, attacktype) )
		return 1;

  /* Check for combatSpec on mobile. */
  if (
      IS_NPC(attacker) &&
      isOffensiveMob(attacker)
     )
	if ( mob_index[attacker->nr].combatSpec(attacker, victim, damage, attacktype) )
	    return 1;

  return 0; /* no effect */
} /* attackerSpecial */

/* ============================================================================
This routine invokes a special routine for an item being worn by the victim
of an attack, or on the victim them self.
============================================================================ */
/* proto-types used in this function. */
int isDefensiveObj(ObjData *obj);
int isDefensiveMob(CharData *mob);

int victimSpecial(CharData *attacker, CharData *victim, int *damage, int *attacktype)
{
  int i;

  /* TYPE_SPECIAL indicates something particularly unusual is happening */
  /* It's probably wiser to not interfere further... e.g. deathWand procedure */
  /* uses TYPE_SPECIAL when it outright slays the victim. */
  if (*attacktype == TYPE_SPECIAL)
	return 0;

  /* Check for combatSpec on equipment. */
  for (i = 0; i < NUM_WEARS; i++)
      if (
	  victim->equipment[i] &&
	  isDefensiveObj(victim->equipment[i])
	 )
	    if ( obj_index[victim->equipment[i]->item_number].combatSpec(victim->equipment[i], attacker, damage, attacktype) )
		return 1;

  /* Check for combatSpec on mobile. */
  if (
      IS_NPC(victim) &&
      isDefensiveMob(victim)
     )
	if ( mob_index[victim->nr].combatSpec(victim, attacker, damage, attacktype) )
	    return 1;

  return 0; /* no effect */
} /* victimSpecial */

/* ============================================================================
Offensive combat special procedures for items. These are invoked when the
attacker damages their opponent.
============================================================================ */

/* Proceduer for the dk special weapon */
int dkSpec(void *me, CharData *victim, int *damage, int *attacktype)
{
	ObjData *dksword = me;
	CharData *ch = dksword->worn_by;

	if( ch->equipment[WEAR_SHIELD] || 
	    ch->equipment[WEAR_LIGHT]  ||
	    ch->equipment[WEAR_HOLD]	) {
		obj_to_char( unequip_char(ch, WEAR_WIELD), ch);
		send_to_char("Your mighty two-handed weapon slips from your fingers.\r\n", ch);
		WAIT_STATE( ch, SET_STUN(3) );
	}

	return 1;
}

/* Procedure for the wand of death. */
int deathWand(void *me, CharData *victim, int *damage, int *attacktype)
{
    ObjData *wand;

    wand = me;

    /* If the wand isn't being worn, bail. */
    if (!wand->worn_by)
	return 0;

    /* If the victim wasn't hit with the wand itself, just bail. */
	// Also consider attack type burn due to flame blade
    if (*damage <= 0 || 
		(*attacktype != (wand->obj_flags.value[3] + TYPE_HIT) && *attacktype != TYPE_BURN ))
	return 0;

    /* 1% chance of making them save vs. death or die. */
    if (number(1, 100) != 1)
	return 0;

    act("$p is surrounded by a nimbus of emerald light!", FALSE, victim, wand, wand->worn_by, TO_ROOM);
    act("$p is surrounded by a nimbus of emerald light!", FALSE, victim, wand, wand->worn_by, TO_CHAR);
    if ( !(GET_LEVEL(wand->worn_by) >= LVL_IMMORT) && /* more effective in hands of a god, like Orcus for example. */
         magic_savingthrow(wand->worn_by, victim, SAVING_PARA)) {
	*damage *= 2; /* Even if they save, take extra damage. */
	return 1;    /* They made a successful save, so they live. */
    }

    /* They die. */
    act("$n turns deathly pale as $N touches $m with $p!", FALSE, victim, wand, wand->worn_by, TO_ROOM);
    act("As $N touches you with $p, you feel your soul wrenched from your body!", FALSE, victim, wand, wand->worn_by, TO_CHAR);
    GET_HIT(victim) = -100; /* Time to die! */
    *damage = 0; /* Make sure nothing funky happens, they WILL die. */
    *attacktype = TYPE_SPECIAL; /* turn off normal damage messages. */

    /* 1 in 1000 chance of wand dissapearing after slaying something outright. */
    if (GET_LEVEL(wand->worn_by) < LVL_IMMORT && number(1, 1000) == 1) {
	act("$p fades from existence!", FALSE, victim, wand, wand->worn_by, TO_ROOM);
	unequip_char(wand->worn_by, wand->worn_at);
	extract_obj(wand);
    }

    return 1;
}

/* Procedure for Baseball Bat. */
int baseball(void *me, CharData *victim, int *damage, int *attacktype)
{
  ObjData *bat = (ObjData *)me;
  int to_room;

  /* If the bat isn't being wielded, bail. */
  if (!bat->worn_by) return 0;

  /* If the victim wasn't hit with the bat itself, just bail. */
  if (*damage <= 0 || 
	  ( *attacktype != (bat->obj_flags.value[3] + TYPE_HIT) && *attacktype != TYPE_BURN ))
    return 0;

  /* 5% chance of batting them away, 20% for immortals */
  if (number(1, 100) > (GET_LEVEL(bat->worn_by) > MAX_MORTAL ? 20: 5))
    return 0;

  act("$N bats $n clean out of the room!", FALSE, victim, bat, bat->worn_by,
      TO_NOTVICT);
  act("$N bats $n clean out of the room!", FALSE, victim, bat, bat->worn_by,
      TO_VICT);
  act("$N bats you clean out of the room!", FALSE, victim, bat, bat->worn_by,
      TO_CHAR);

  do {
    to_room = number(0, top_of_world);
  } while (IS_SET_AR(world[to_room].room_flags, ROOM_CLAN | ROOM_PRIVATE | ROOM_DEATH));

  end_fight(victim);
  char_from_room(victim);
  char_to_room(victim, to_room);
  look_at_room(victim, 0);
  entry_memory_mtrigger(victim);
  greet_mtrigger(victim, -1);
  greet_memory_mtrigger(victim);

  *damage = -1;

  return 1;    /* They made a successful save, so they stay in combat. */
}

/* Procedure for Starlight Sword. */
int healingSurgeSword(void *me, CharData *victim, int *damage, int *attacktype)
{
    ObjData *sword;

    sword = me;

    /* If the sword isn't being worn, bail. */
    if (!sword->worn_by)
	return 0;

    /* If the victim wasn't hit with the sword itself, just bail. */
    // Also consider attack type burn due to flame blade
    if (*damage <= 0 ||
            (*attacktype != (sword->obj_flags.value[3] + TYPE_HIT) && *attacktype != TYPE_BURN ))
	return 0;

    /* If the wielder is not pure of heart, or the victim isn't evil, bail. */
    if((GET_CLASS(sword->worn_by) != CLASS_SOLAMNIC_KNIGHT) || !IS_GOOD(sword->worn_by))
        return 0;

    /* 8% chance of something happening. Can only proc when wearer has less than 50% health. */
    if (number(1, 100) > 8 && GET_HIT(sword->worn_by) * 100 / GET_MAX_HIT(sword->worn_by) < 50)
	return 0;

    act("$p glows with a holy light!", FALSE, victim, sword, sword->worn_by, TO_ROOM);
    act("You glow with a holy light!", FALSE, victim, sword, sword->worn_by, TO_CHAR);
    if(GET_HIT(victim) < GET_MAX_HIT(victim))
      GET_HIT(victim) = MIN(GET_MAX_HIT(victim), GET_HIT(victim) + number(45, 90));

    return 1;
}

/* Procedure for Starlight Sword. */
int holyAvenger(void *me, CharData *victim, int *damage, int *attacktype)
{
    ObjData *avenger;

    avenger = me;

    /* If the avenger isn't being worn, bail. */
    if (!avenger->worn_by)
	return 0;

    /* If the victim wasn't hit with the avenger itself, just bail. */
	// Also consider attack type burn due to flame blade
    if (*damage <= 0 || 
		(*attacktype != (avenger->obj_flags.value[3] + TYPE_HIT) && *attacktype != TYPE_BURN ))
	return 0;

    /* If the wielder is not pure of heart, or the victim isn't evil, bail. */
    if ( (GET_CLASS(avenger->worn_by) != CLASS_SOLAMNIC_KNIGHT) ||
         !IS_GOOD(avenger->worn_by) ||
	 !IS_EVIL(victim)
       )
	return 0;

    /* 5% chance of something happening. */
    if (number(1, 100) > 5)
	return 0;

    act("$p is surrounded by a holy light!", FALSE, victim, avenger, avenger->worn_by, TO_ROOM);
    act("$p is surrounded by a holy light!", FALSE, victim, avenger, avenger->worn_by, TO_CHAR);
    *damage *= 2; /* double damage */

    /* Evil victim must save or flee from the holy light. */
    if ( !magic_savingthrow(avenger->worn_by, victim, SAVING_PETRI) ) {
        sendChar(victim, "Your evil soul quails before the holy aura!");
        do_flee(victim, "", 0, 0);
    }
    return 1;
}

/* Procedure for Sword of Shadows. */
int unholyAvenger(void *me, CharData *victim, int *damage, int *attacktype)
{
    ObjData *avenger;

    avenger = me;

    /* If the avenger isn't being worn, bail. */
    if (!avenger->worn_by)
	return 0;

    /* If the victim wasn't hit with the avenger itself, just bail. */
	// Also consider attack type burn due to flame blade
    if (*damage <= 0 || 
		(*attacktype != (avenger->obj_flags.value[3] + TYPE_HIT) && *attacktype != TYPE_BURN ))
	return 0;

    /* If the wielder is not a champion of evil, or the victim isn't good, bail. */
    if ( (GET_CLASS(avenger->worn_by) != CLASS_DEATH_KNIGHT) ||
         !IS_EVIL(avenger->worn_by) ||
	 !IS_GOOD(victim)
       )
	return 0;

    /* 8% chance of something happening. */
    if (number(1, 100) > 8)
	return 0;

    act("$p is surrounded by a cloud of darkness!", FALSE, victim, avenger, avenger->worn_by, TO_ROOM);
    act("$p is surrounded by a cloud of darkness!", FALSE, victim, avenger, avenger->worn_by, TO_CHAR);
    *damage *= 2; /* double damage */

    /* Good victim must save or flee from the unholy light. */
    if ( !magic_savingthrow(avenger->worn_by, victim, SAVING_PETRI) ) {
        sendChar(victim, "Your pure soul quails before the unholy aura!");
        do_flee(victim, "", 0, 0);
    }
    return 1;
}

/*Procedure for Avernus*/
int hellSword(void *me, CharData *victim, int *damage, int *attacktype)
{
    ObjData *avernus;
    int num;

    avernus = me;

    /* If avernus isn't being worn, bail. */
    if (!avernus->worn_by)
	return 0;

    /* If the victim wasn't hit with avernus itself, just bail. */
	// Also consider attack type burn due to flame blade.
    if (*damage <= 0 || 
		(*attacktype != (avernus->obj_flags.value[3] + TYPE_HIT) && *attacktype != TYPE_BURN) )
	return 0;

    /* 8% chance of doing something. */
    if (number(1,100) > 8)
	return 0;

    /* wielder is corrupted by the swords power */
    if ( GET_ALIGNMENT(avernus->worn_by) >= -990 )
	GET_ALIGNMENT(avernus->worn_by) -= 10;
    else
	GET_ALIGNMENT(avernus->worn_by) = -1000;

    num = number(1, 2);
    switch (num) {
    case 1: /* life drain */
	*attacktype = TYPE_DRAIN;
        act("$p emits a ghostly wail!", FALSE, victim, avernus, avernus->worn_by, TO_ROOM);
        act("$p emits a ghostly wail!", FALSE, victim, avernus, avernus->worn_by, TO_CHAR);
        GET_HIT(avernus->worn_by) = MIN(GET_MAX_HIT(avernus->worn_by) * 6 / 5, GET_HIT(avernus->worn_by) + *damage);
	break;
    case 2: /* burns them */
	*attacktype = TYPE_BURN;
        act("$p glows with a hellish light!", FALSE, victim, avernus, avernus->worn_by, TO_CHAR);
        act("$p glows with a hellish light!", FALSE, victim, avernus, avernus->worn_by, TO_ROOM);
	*damage *= number(2, 4); /* ouchie! */
	break;
    default:
		break;
    }
    return 1;
}

/*Fake Avernus, made by Xiuhtecuhtli for Xandor*/
int trickXandor(void *me, CharData *victim, int *damage, int *attacktype)
{
    ObjData *avernus;
    int num;

    avernus = me;

    /* If avernus isn't being worn, bail. */
    if (!avernus->worn_by)
	return 0;

    /* If the victim wasn't hit with avernus itself, just bail. */
	// Also consider attack type burn due to flame blade.
    if (*damage <= 0 || 
		(*attacktype != (avernus->obj_flags.value[3] + TYPE_HIT) && *attacktype != TYPE_BURN) )
	return 0;

    /* 8% chance of doing something. */
    if (number(1,100) > 8)
	return 0;

    /* instead of going evil this version of the sword will turn their alignment to holy */
    if ( GET_ALIGNMENT(avernus->worn_by) <= 990 )
	GET_ALIGNMENT(avernus->worn_by) += 10;
    else
	GET_ALIGNMENT(avernus->worn_by) = 1000;

    num = number(1, 2);
    switch (num) {
    case 1: /* life drain */
	*attacktype = TYPE_DRAIN;
        act("$p emits an embryonic whine!", FALSE, victim, avernus, avernus->worn_by, TO_ROOM);
        act("$p emits an embryonic whine!", FALSE, victim, avernus, avernus->worn_by, TO_CHAR);
	GET_HIT(avernus->worn_by) += *damage;
	break;
    case 2: /* burns them */
	*attacktype = TYPE_BURN;
        act("$p glows with a soft light!", FALSE, victim, avernus, avernus->worn_by, TO_CHAR);
        act("$p glows with a soft light!", FALSE, victim, avernus, avernus->worn_by, TO_ROOM);
	*damage *= number(2, 4); /* ouchie! */
	break;
    default:
		break;
    }
    return 1;
}

/*Procedure for Soul Reaver*/
int betrayImmort(void *me, CharData *victim, int *damage, int *attacktype)
{
    ObjData *betrayal;

    betrayal = me;

    /* If betrayal isn't being worn, bail. */
    if (!betrayal->worn_by)
	return 0;

    /* If the victim wasn't hit with betrayal itself, just bail. */
    if (*damage <= 0 || *attacktype != (betrayal->obj_flags.value[3] + TYPE_HIT))
	return 0;

    /* 8% chance of doing something. */
    if (number(1,100) > 8)
	return 0;

	*attacktype = TYPE_DRAIN;
        act("$p wails, rejuvenating its master!", FALSE, victim, betrayal, betrayal->worn_by, TO_ROOM);
        act("$p wails, rejuvenating its master!", FALSE, victim, betrayal, betrayal->worn_by, TO_CHAR);
	GET_MANA(betrayal->worn_by) += *damage;

	return 1;
}

/*Procedure for MageBane*/
int mageBane(void *me, CharData *victim, int *damage, int *attacktype)
{
    ObjData *mb;
    int fraction;

    mb = me;

    /* If mageBane isn't being worn, bail. */
    if (!mb->worn_by)
        return 0;

    /* If the victim wasn't hit with mageBane itself, just bail. */
    if (*damage <= 0 || *attacktype != (mb->obj_flags.value[3] + TYPE_HIT))
        return 0;

    fraction=0;
    if (GET_CLASS(mb->worn_by) == CLASS_CLERIC) {
      if (IS_GOOD(mb->worn_by)) {
	if (GET_CLASS(victim) == CLASS_MAGIC_USER)
	  fraction=3;
	else
	  fraction=10;
      } else if (IS_NEUTRAL(mb->worn_by)) {
        if (GET_CLASS(victim) == CLASS_MAGIC_USER)
          fraction=10;
        else
          fraction=0;
      } else {
        if (GET_CLASS(victim) == CLASS_MAGIC_USER)
          fraction=10;
        else
          fraction=-10;
      }
    } else if (GET_CLASS(mb->worn_by)== CLASS_MAGIC_USER) {
        if (GET_CLASS(victim) == CLASS_MAGIC_USER)
          fraction=-10;
        else
          fraction=-1;
    }
    if (fraction)
      GET_MANA(mb->worn_by) += *damage/fraction;
    if (GET_MANA(mb->worn_by)<0)
      GET_MANA(mb->worn_by)=0;

    if (GET_CLASS(victim) == CLASS_MAGIC_USER) {
      GET_MANA(victim) -= *damage;
      if (GET_MANA(victim)<0)
	GET_MANA(victim)=0;
    }
    return 1;
}

/*Procedure for cursed "backbiter" weapons*/
int backbiter(void *me, CharData *victim, int *dam, int *attacktype)
{
    ObjData *bb;

    bb = me;

    // Is it used?
    if (!bb->worn_by)
        return 0;

    // Did you hit with it?
    if (*dam <= 0 || *attacktype != (bb->obj_flags.value[3] + TYPE_HIT))
        return 0;

    // There's a 2% chance that the backbite will strike
    if (number(1,100)>2)
	return 0;
    // After the  bickbite the weapon is cursed :)
    SET_BIT_AR( bb->obj_flags.extra_flags, ITEM_CURSED );

    act("$p crackles with power as $N falls down.", FALSE, bb->worn_by, bb, bb->worn_by, TO_ROOM);
    act("$p crackles with power as you fall down.", FALSE, bb->worn_by, bb, bb->worn_by, TO_CHAR);
    *dam += 10;
    *dam *= 5;  // (damage+10)x5 OUCH!
    /* *attacktype = TYPE_SPECIAL; */

    // damage the wielder - and make him sit :)
    GET_POS(bb->worn_by) = POS_SITTING;
    damage(bb->worn_by, bb->worn_by, *dam, TYPE_SUFFERING);
    *dam=0; // We don't want to hurt the target :) :)
    return 1;
}

/* Procedure for the Hellfire sword. */
int Hellfire(void *me, CharData *victim, int *damage, int *attacktype)
{
  ObjData *sword = (ObjData *)me;

  /* If sword isn't being worn, bail. */
  if (!sword->worn_by)
    return 0;

  /* if it wasn't the sword that did the hit, bail */
  if (*damage <= 0 || *attacktype != (sword->obj_flags.value[3] + TYPE_HIT))
    return 0;

  /* 4% chance of doing something. */
  if (number(1, 100) > 4)
    return 0;

  // it's bad to actually use the mag_damage() routine to kill them
  if (IS_EVIL(sword->worn_by)) {
    act("You utter an evil incantation and summon up the fires of hell!",
        FALSE, sword->worn_by, 0, 0, TO_CHAR);
    act("$n utters an evil incantation and summons up the fires of hell!",
        FALSE, sword->worn_by, 0, 0, TO_ROOM);
    *attacktype = SPELL_DEMON_FIRE;
    *damage = *damage * 7 / 2;
  } else if (IS_GOOD(sword->worn_by)) {
    act("You raise your hands to the heavens and call down a purifying column "
        "of flame!", FALSE, sword->worn_by, 0, 0, TO_CHAR);
    act("$n raises $s hands to the heavens and calls down a purifying column "
        "of flame!", FALSE, sword->worn_by, 0, 0, TO_ROOM);
    *attacktype = SPELL_FLAME_STRIKE;
    *damage = *damage * 7 / 2;
  } else {
      // neutral are SOL.
  }

  return 1;
}

char *ACTIONS[] = {
    "none", "blindness", "silence", "haste", "revive", "dispel",
    "curse", "+dam", "-dam", "?dam", "sting", "spray", "fear",
    "cleanse", "disrobe", "dream",
};

/* The Touch of Chaos */
int Chaos(void *me, CharData *victim, int *damage, int *attacktype)
{
    ObjData *sword = (ObjData *)me, *obj;
    CharData *target, *ch, *caster;
    int action, r = 0, level;

    if (!sword->worn_by) return 0;

    /* if it wasn't the sword that did the hit, bail */
    if (*damage <= 0 || *attacktype != (sword->obj_flags.value[3] + TYPE_HIT))
        return 0;

    /* 8% chance to have an effect */
    if (number(1,100) > 8) return 0;

    /* pick a random target in the room */
    for (ch = world[victim->in_room].people; ch; ch = ch->next_in_room) {
        if ((IS_NPC(ch) || GET_LEVEL(ch) < LVL_IMMORT) && number(1, ++r) == 1)
            target = ch;
    }

    /* save typing later */
    ch = sword->worn_by;
    level = GET_LEVEL(ch);

    /* see what we're going to do */
    action = number(1,15);

    act("An ear shattering howl turns the world into chaos.",
            TRUE, ch, 0, 0, TO_CHAR);
    act("An ear shattering howl turns the world into chaos.",
            TRUE, ch, 0, 0, TO_ROOM);

    switch (action) {
        case 1:         /* blind */
            mag_affects(level, ch, target, SPELL_BLINDNESS, SAVING_SPELL);
            break;
        case 2:         /* silence */
            mag_affects(level, ch, target, SPELL_SILENCE, SAVING_SPELL);
            break;
        case 3:         /* haste */
            mag_affects(level, ch, target, SPELL_HASTE, SAVING_SPELL);
            break;
        case 4:         /* revive */
            caster = read_mobile(DG_CASTER_PROXY, VIRTUAL);
            // The following is an ugly mess to let this weapon work
            // despite chants.
            if (!caster) {
                script_log("dg_cast: Cannot load the caster mob!");
                return FALSE;
            }

            caster->player.short_descr = str_dup("The gods");
            char_to_room( caster, ch->in_room );

            mag_points(level, caster, target, SPELL_REVIVE, SAVING_SPELL);
            
            extract_char(caster);
            break;
        case 5:         /* dispel magic */
            mag_affects(level, ch, target, SPELL_DISPEL_MAGIC, SAVING_SPELL);
            break;
        case 6:         /* curse */
            mag_affects(level, ch, target, SPELL_CURSE, SAVING_SPELL);
            break;
        case 7:         /* increase damage */
            *damage *= 2;
            break;
        case 8:         /* decrease damage */
            *damage = number(1,4);
            break;
        case 9:         /* random damage */
            *damage = number(1,200);
            break;
        case 10:        /* change attack type */
            *attacktype = TYPE_STING;
            break;
        case 11:        /* change damage type */
            *attacktype = SPELL_COLOR_SPRAY;
            break;
        case 12:        /* fear */
            mag_affects(level, ch, target, SPELL_FEAR, SAVING_SPELL);
            break;
        case 13:        /* cleanse */
            spell_cleanse(level, ch, target, 0, 0, SAVING_SPELL);
            break;
        case 14:        /* disrobe */
            r = number(0, NUM_WEARS - 1);
            if (target->equipment[r]) {
                obj = unequip_char(target, r);
                act("You are zapped by $p and instantly let go of it.",
                        FALSE, target, obj, target, TO_CHAR);
                act("$n is zapped by $p and instantly lets go of it.",
                        FALSE, target, obj, target, TO_ROOM);
                obj_to_char(obj, target);
            }
            break;
        case 15:        /* dance of dreams */
            mag_affects(level, ch, target, SPELL_DANCE_DREAMS, SAVING_SPELL);
            break;
        default:
            return 0;
    }

    return 1;
}

/* This procedure defines which procedures are offensive. */
int isOffensiveObj(ObjData *obj)
{
    /* does this item even have a procedure on it? */
    if ((obj->item_number < 0) || !obj_index[obj->item_number].combatSpec)
	return 0;

	if (obj_index[obj->item_number].combatSpec == baseball)
	    return 1;
	if (obj_index[obj->item_number].combatSpec == deathWand)
	    return 1;
	if (obj_index[obj->item_number].combatSpec == hellSword)
	    return 1;
	if (obj_index[obj->item_number].combatSpec == holyAvenger)
	    return 1;
	if (obj_index[obj->item_number].combatSpec == unholyAvenger)
	    return 1;
	if (obj_index[obj->item_number].combatSpec == backbiter)
	    return 1;
	if (obj_index[obj->item_number].combatSpec == mageBane)
            return 1;
	if (obj_index[obj->item_number].combatSpec == Hellfire)
            return 1;
	if (obj_index[obj->item_number].combatSpec == Chaos)
            return 1;
	if (obj_index[obj->item_number].combatSpec == slSpec)
            return 1;
	if (obj_index[obj->item_number].combatSpec == raSpec)
            return 1;
	if (obj_index[obj->item_number].combatSpec == muSpec)
            return 1;
	if (obj_index[obj->item_number].combatSpec == thSpec)
            return 1;
	if (obj_index[obj->item_number].combatSpec == dkSpec)
	    return 1;
	if (obj_index[obj->item_number].combatSpec == betrayImmort)
	        return 1;
	if (obj_index[obj->item_number].combatSpec == trickXandor)
	    return 1;
	return 0;
}

/* ============================================================================
Defensive combat special procedures for items. These are invoked for an
item being worn by someone who has just been hit.
============================================================================ */
/* Procedure for the shield demon fire. */
int demonFire(void *me, CharData *victim, int *damage, int *attacktype)
{
    ObjData *shield;
    int dam;

    shield = me;

    if (mini_mud)
	return 0;

    /* If shield isn't being worn, bail. */
    if (!shield->worn_by)
	return 0;

    if (!IS_GOOD(victim) && IS_EVIL(shield->worn_by))
	return 0;

    /* 4% chance of doing something. */
    if (number(1, 100) > 3)
	return 0;

    if (!IS_EVIL(shield->worn_by)) { /* shield don't like wearer... */
// Debug spam	mudlog(NRM, LVL_IMMORT, TRUE, "DEMON FIRE: Burning %s", GET_NAME(shield->worn_by));
    act("The demonic runes on your $p glow, twisting your sense of faith!", FALSE, shield->worn_by, shield, victim, TO_CHAR);
    act("The demonic runes on $n's $p glow, but something is not right!", FALSE, shield->worn_by, shield, victim, TO_ROOM);
    *attacktype = SPELL_DEMON_FIRE;
    *damage += 500 + dice(5,GET_LEVEL(shield->worn_by));
    }
    else { /* the victim is good aligned, burn them! */
// Debug spam	mudlog(NRM, LVL_IMMORT, TRUE, "DEMON FIRE: Burning %s", GET_NAME(victim));
    act("The demonic runes on your $p glow with a hellish light!", FALSE, shield->worn_by, shield, victim, TO_CHAR);
    act("The demonic runes on $n's $p glow with a hellish light!", FALSE, shield->worn_by, shield, victim, TO_ROOM);
    dam = 50 + dice(5,GET_LEVEL(shield->worn_by));
    GET_HIT(victim) -= dam;
    GET_HIT(victim) = MAX(1, GET_HIT(victim)); // Imhotep sez, "Killing ppl with a shield is a Bad Thing(tm)"
        }

    return 1;
}

/*Procedure for DragonSlayer*/
int dragonSlayer(void *me, CharData *victim, int *damage, int *attacktype)
{
  ObjData *shield = (ObjData *)me;
  int reflect;

  if (!shield->worn_by) return 0;

  // don't glow if there was no damage
  if (*damage == 0) return 0;

  // only glow if this was a normal attack
  if (*attacktype < TYPE_HIT || *attacktype > TYPE_HIT + NUM_WEAPON_TYPES)
    return 0;

  // 5% chance to reflect
  if (number(1, 100) > 5) return 0;

  // 30% chance we'll reflect 90% of damage, otherwise 33%
  if (number(1, 100) <= 30) {
    reflect = *damage * 9 / 10;
  } else {
    reflect = *damage / 3;
  }
  *damage -= reflect;

  // flash brightly for the room, and cause mojo pain to the attacker
  act("$p flashes brightly, reflecting $n's attack!", FALSE, victim, shield, victim, TO_ROOM);
  act("$p flashes brightly, reflecting your attack!", FALSE, victim, shield, victim, TO_CHAR);

  // don't kill them with this
  GET_HIT(victim) -= reflect;
  if (GET_HIT(victim) < 1) GET_HIT(victim) = 1;
  //mudlog(BRF, LVL_IMMORT, FALSE, "Reflecting %d dam, leaving %d dam, victim to %d hp",
      //reflect, *damage, GET_HIT(victim));
  return 1;
}

/* This function defines which combat specials are "defensive" */
int isDefensiveObj(ObjData *obj)
{
    /* does this item even have a procedure on it? */
    if ((obj->item_number < 0) || !obj_index[obj->item_number].combatSpec)
	return 0;

    if (obj_index[obj->item_number].combatSpec == demonFire)
	return 1;
    if (obj_index[obj->item_number].combatSpec == dragonSlayer)
	return 1;
    if (obj_index[obj->item_number].combatSpec == clSpec)
        return 1;

    return 0;
}

/* ============================================================================
Offensive mob procedures.
============================================================================ */

/* This function defines which combat specials on mobs are "offensive" */
int isOffensiveMob(CharData *mob)
{
    if (!IS_NPC(mob)) {
	mudlog(NRM, LVL_IMMORT, TRUE, "SYSERR: PC %s passed to isOffensiveMob!!(combat specials)", GET_NAME(mob));
	return 0;
    }

    /* does this mob even have a procedure on it? */
    if ( mob->nr < 0 || !mob_index[mob->nr].combatSpec)
	return 0;

    /* none defined atm */
    return 0;
}


/* ============================================================================
Defensive mob procedures.
============================================================================ */

/* This function defines which combat specials on mobs are "defensive" */
int isDefensiveMob(CharData *mob)
{
    if (!IS_NPC(mob)) {
	mudlog(NRM, LVL_IMMORT, TRUE, "SYSERR: PC %s passed to isDefensiveMob!!(combat specials)", GET_NAME(mob));
	return 0;
    }

    /* does this mob even have a procedure on it? */
    if ( mob->nr < 0 || !mob_index[mob->nr].combatSpec)
	return 0;

    /* none defined atm */
    return 0;
}

/* ============================================================================
Combat special procedure general utilities.
============================================================================ */

/* Procedure to assign a combat special to a object. */
void specialObjCombat(int obj, int (fname)(void *me, CharData *victim, int *damage, int *attacktype) )
{
  if (real_object(obj) >= 0){
    obj_index[real_object(obj)].combatSpec = fname;
  }
  else {
      if (!mini_mud) {
        mlog("SYSERR: Attempt to assign combat special procedure to non-existant obj #%d",
	        obj);
      }
  }
}

/* Procedure to assign a combat special to a mob */
void specialMobCombat(int mob, int (fname)(void *me, CharData *victim, int *damage, int *attacktype) )
{
  if (real_mobile(mob) >= 0){
    mob_index[real_mobile(mob)].combatSpec = fname;
  }
  else {
      if (!mini_mud) {
        mlog("SYSERR: Attempt to assign combat special procedure to non-existant mob #%d",
	        mob);
      }
  }
}

/* Assign all combat special proedures */
void assignCombatSpecials(void)
{
    /* Objects. */
    specialObjCombat(13201, deathWand);    /* the Wand of Death */
    specialObjCombat(1281, baseball);      /* the baseball bat */
    specialObjCombat(9015, Chaos);         /* the Touch of Chaos */
    specialObjCombat(13230, hellSword);    /* Avernus the life stealer */
    specialObjCombat(1442, unholyAvenger); /* the Sword of Shadows */
    specialObjCombat(20689, demonFire);    /* Demon Fire */
    specialObjCombat(21280, holyAvenger);  /* the Starlight Sword */
    specialObjCombat(23899, backbiter);    /* BackBiter */
    specialObjCombat(23898, mageBane);     /* MageBane*/
    specialObjCombat(40498, dragonSlayer); /*DragonSlayer*/
    specialObjCombat(40497, Hellfire);     /*Hellfire*/
    specialObjCombat(38312, clSpec);
    specialObjCombat(38210, slSpec);
    specialObjCombat(38113, raSpec);
    specialObjCombat(38618, muSpec);
    specialObjCombat(38625, dkSpec);       /* Lochaber axe */
    specialObjCombat(38850, thSpec);       /* Quietus */
    specialObjCombat(43919, betrayImmort); /* Soul Reaver */
    specialObjCombat(25086, trickXandor);  /* Fake Avernus, opposite alignment */
}

/* Figures out what the name of the function that was passed in is. */
void combSpecName(int (*combSpec)(void *, CharData *, int *, int *), char *theName)
{

    if ( !combSpec )
	sprintf(theName, "None");
    else if (combSpec == baseball)
	 sprintf(theName, "&12Baseball Bat&00");
    else if (combSpec == deathWand)
	 sprintf(theName, "&12Death Wand&00");
    else if (combSpec == hellSword)
	sprintf(theName, "&08Hell Sword&00");
    else if (combSpec == trickXandor)
        sprintf(theName, "&08Fake Avernus&00");
    else if (combSpec == unholyAvenger)
	sprintf(theName, "&12Unholy Avenger&00");
    else if (combSpec == holyAvenger)
	sprintf(theName, "&14Holy Avenger&00");
    else if (combSpec == demonFire)
	sprintf(theName, "&01Demon Fire&00");
    else if (combSpec == backbiter)
	sprintf(theName, "&08Backbiter&00");
    else if (combSpec == mageBane)
        sprintf(theName, "&14Magebane&00");
    else if (combSpec == dragonSlayer)
        sprintf(theName, "&11Dragon Slayer&00");
    else if (combSpec == Hellfire)
        sprintf(theName, "&01Hellfire&00");
    else if (combSpec == Chaos)
        sprintf(theName, "&08Chaos&00");
    else if (combSpec == slSpec)
        sprintf(theName, "&03gloves of harmony&00");
    else if (combSpec == clSpec)
        sprintf(theName, "&03the armor of the mind&00");
    else if (combSpec == thSpec)
        sprintf(theName, "&07the quietus&00");
    else if (combSpec == dkSpec)
	sprintf(theName, "&07the great lochaber axe&00");
    else if (combSpec == betrayImmort)
        sprintf(theName, "&12Betrayal of Immort&00");
    else
	sprintf(theName, "Unknown -> combSpecName needs to be updated.");
}

int isUsingLochaber(CharData *ch)
{
	if(WIELDING(ch))
            if(GET_OBJ_VNUM(ch->equipment[WEAR_WIELD]) == 38625)
                return TRUE;

        // If we get this far, it's not a lochaber.
        return FALSE;
}

