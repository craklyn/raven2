
#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "util/utils.h"
#include "specials/classes.h"
#include "magic/spells.h"
#include "magic/magic.h"
#include "actions/offensive.h"
#include "general/comm.h"
#include "general/class.h"
#include "general/handler.h"

#define NO_MOD   0

// thieves get a specproc wield that has a chance to slow victims temporarily on a stab
int thSpec(void *me, CharData *victim, int *damage, int *attacktype)
{
  ObjData *obj = (ObjData *)me;

  if (*damage > 0 && *attacktype == SKILL_BACKSTAB && number(1,100) <= 60) {
    act("$p severs critical nerves in $N's back!", FALSE, obj->worn_by, obj,
        victim, TO_ROOM);
    act("$p severs critical nerves in $N's back!", FALSE, obj->worn_by, obj,
        victim, TO_CHAR);
   	add_affect( obj->worn_by, victim, SPELL_SLOW, GET_LEVEL(obj->worn_by), 
		APPLY_NONE, NO_MOD, number(3, 6), NO_MOD, FALSE, FALSE, FALSE, FALSE);
	return 1;
  }

  if (*damage > 0 && *attacktype == SKILL_CIRCLE && number(1,100) <= 15) {
    act("$p severs critical nerves in $N's back!", FALSE, obj->worn_by, obj,
        victim, TO_ROOM);
    act("$p severs critical nerves in $N's back!", FALSE, obj->worn_by, obj,
        victim, TO_CHAR);
   	add_affect( obj->worn_by, victim, SPELL_SLOW, GET_LEVEL(obj->worn_by), 
		APPLY_NONE, NO_MOD, number(3, 4), NO_MOD, FALSE, FALSE, FALSE, FALSE);
	return 1;
  }

  return 0;
}

// ranger boots will do 10x damage kicks 50% of the time
// they will also set HAMSTRUNG for 1 tick for 33% of trips
int raSpec(void *me, CharData *victim, int *damage, int *attacktype)
{
  ObjData *obj = (ObjData *)me;

  /* kicks are SILENT damage! */
  if (*damage > 0 && *attacktype == SKILL_KICK && number(1,100) <= 50) {
    *damage *= 6;
    return 1;
  }

  if (*damage > 0 && *attacktype == SKILL_TRIP && number(1,100) <= 33) {
    act("Blood sprays from $N's legs as $p slice them!", FALSE,
            obj->worn_by, obj, victim, TO_NOTVICT);
    act("Blood sprays from your legs as $p slice them!", FALSE,
            obj->worn_by, obj, victim, TO_VICT);
    act("Blood sprays from $N's legs as $p slice them!", FALSE,
            obj->worn_by, obj, victim, TO_CHAR);
    add_affect(obj->worn_by, victim, SKILL_TRIP, 0, APPLY_NONE, 0, 1 TICKS,
            AFF_HAMSTRUNG, FALSE, FALSE, FALSE, FALSE);
    return 1;
  }
  return 0;
}

// shou-lin gloves will do a shou-lin specproc 5% of the time
int slSpec(void *me, CharData *victim, int *damage, int *attacktype)
{
  ObjData *obj = (ObjData *)me;

  // If gloves aren't being worn by anybody, bail.
  if (!obj->worn_by)
	return 0;
  
  //if (*damage > 0 && !number(0,19) && *attacktype == TYPE_HIT) {
  if (*damage > 0 && !number(0,21) && GET_CLASS(obj->worn_by) == CLASS_SHOU_LIN &&
          (*attacktype == TYPE_HIT || *attacktype == TYPE_CLAW) )
  {
      if (!WIELDING(obj->worn_by)) {
          if ( number(0, 9) ) {
              act("$n's fists burn with power!", FALSE, obj->worn_by,
                      obj, victim, TO_ROOM);
              act("Your fists burn with power!", FALSE, obj->worn_by,
                      obj, victim, TO_CHAR);
              *damage *= number(2,3);
          }
          else {
              act("Shou-lin spirits radiate from $p!", FALSE, victim,
                      obj, obj->worn_by, TO_ROOM);
              act("Shou-lin spirits radiate from $p!", FALSE, victim,
                      obj, obj->worn_by, TO_CHAR);
              *damage *= number(3,3);
          }
      }
      return 1;
  }
  return 0;
}

/* clerics get on body armor that absorbs magical damage */
int clSpec(void *me, CharData *victim, int *damage, int *attacktype)
{
  ObjData *obj = (ObjData *)me;

  /* 1/4 of spells will be 1/4 damage, and 50 to 100% of the damage
   * done will be added to the cleric's mana */
  if (*attacktype < MAX_SPELLS && *damage > 0 && number(1,4) == 1) {
    act("Arcs of lightning web over $p!", FALSE, obj->worn_by, obj, victim,
        TO_ROOM);
    act("Arcs of lightning web over $p!", FALSE, obj->worn_by, obj, victim,
        TO_CHAR);
    GET_MANA(obj->worn_by) += *damage * number(50,100) / 100;
    *damage /= 4;
  }
}

int muSpec(void *me, CharData *victim, int *damage, int *attacktype)
{
    /* determine if this is a physical attack or a spell attack */
    if (*attacktype >= TYPE_HIT && *attacktype <= TYPE_HIT + NUM_WEAPON_TYPES) {
    } else if (*attacktype <= MAX_SPELLS) {
    }
}

