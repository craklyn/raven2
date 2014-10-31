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
    return 1;
}

/* Procedure for Baseball Bat. */
int baseball(void *me, CharData *victim, int *damage, int *attacktype)
{
    return 1;
}

/* Procedure for Starlight Sword. */
int healingSurgeSword(void *me, CharData *victim, int *damage, int *attacktype)
{
    return 1;
}

/* Procedure for Starlight Sword. */
int holyAvenger(void *me, CharData *victim, int *damage, int *attacktype)
{
     return 1;
}

/* Procedure for Sword of Shadows. */
int unholyAvenger(void *me, CharData *victim, int *damage, int *attacktype)
{
    return 1;
}

/*Procedure for Avernus*/
int hellSword(void *me, CharData *victim, int *damage, int *attacktype)
{
    return 1;
}

/*Fake Avernus, made by Xiuhtecuhtli for Xandor*/
int trickXandor(void *me, CharData *victim, int *damage, int *attacktype)
{
    return 1;
}

/*Procedure for Soul Reaver*/
int betrayImmort(void *me, CharData *victim, int *damage, int *attacktype)
{
 	return 1;
}

/*Procedure for MageBane*/
int mageBane(void *me, CharData *victim, int *damage, int *attacktype)
{
     return 1;
}

/*Procedure for cursed "backbiter" weapons*/
int backbiter(void *me, CharData *victim, int *dam, int *attacktype)
{
    return 1;
}

/* Procedure for the Hellfire sword. */
int Hellfire(void *me, CharData *victim, int *damage, int *attacktype)
{
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
    return 1;
}

/*Procedure for DragonSlayer*/
int dragonSlayer(void *me, CharData *victim, int *damage, int *attacktype)
{
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

