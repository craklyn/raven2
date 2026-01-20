//
//++
//  RCSID:     $Id: backstab.c,v 1.8 2003/11/14 01:42:42 raven Exp $
//
//  FACILITY:  RavenMUD
//
//  LEGAL MUMBO JUMBO:
//
//      This is based on code developed for DIKU and Circle MUDs.
//
//  MODULE DESCRIPTION:
//
//  AUTHORS:
//
//      Digger from RavenMUD
//
//  NOTES:
//
//--
//
//


// STANDARD U*IX INCLUDES
//
#include "general/conf.h"
#include "general/sysdep.h"

#define THIS_SKILL                SKILL_BACKSTAB

#define SKILL_ADVANCES_WITH_USE   TRUE
#define SKILL_ADVANCE_STRING \
       "The connections of the vertebrae suddenly seem clearer."

static int max_skill_lvls[] =
{
// Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd
   00, 00, 90, 00, 00, 90, 00, 00, 00, 90,

// XX  XX  XX  XX  XX  XX  XX  XX  XX  XX
   00, 00, 00, 00, 00, 00, 00, 00, 00, 00
};

#define SKILL_MAX_LEARN           max_skill_lvls[ (int)GET_CLASS(ch) ]

#define DEX_AFFECTS               TRUE
#define INT_AFFECTS               TRUE
#define WIS_AFFECTS               FALSE

#define STUN_MIN                  2
#define STUN_MAX                  3


// MUD SPECIFIC INCLUDES
//
#include "general/db.h"
#include "general/structs.h"
#include "general/class.h"
#include "general/comm.h"
#include "general/handler.h"
#include "actions/interpreter.h"
#include "magic/skills.h"
#include "magic/spells.h"
#include "util/utils.h"
#include "magic/backstab.h"
#include "specials/special.h"
#include "actions/fight.h"

//-------------------------------------------------------------------------
//
ACMD(do_backstab)
{
  CharData* victim;
  char      arg[100];
  int       assassinated;

  one_argument( argument, arg );

  IF_UNLEARNED_SKILL   ( "You wouldn't know where to begin.\r\n" );

  IF_CH_CANT_SEE_VICTIM( "Backstab who?\r\n" );

  IF_CH_CANT_BE_VICTIM ( "How can you sneak up on yourself?\r\n" );

  IF_ROOM_IS_PEACEFUL  ( "A sense of well being overwhelms you.\r\n" );

  IF_CH_NOT_WIELDING   ( 
	  "You need to wield a weapon to make it a success.\r\n" );

  // If you're laying on your face, you can be stabbed.
  if ( !FLEEING(victim) ) 
  {
	if( FIGHTING(ch) )
	{
		sendChar(ch, "No way!  You're fighting for your life!\r\n");
		return;
	}

	IF_VICTIM_CANT_BE_FIGHTING(
		"You can't backstab a fighting person -- they're too alert!\r\n" );
  }

  IF_CANT_HURT_VICTIM;

  if (!IS_NPC(victim) && victim->mount) {
      sendChar(ch, "You cannot backstab a mounted person.\r\n");
      return;
  }

  if( GET_OBJ_VAL(ch->equipment[WEAR_WIELD], 3) != TYPE_PIERCE - TYPE_HIT )
  {
    sendChar( ch, "Only piercing weapons can be used for backstabbing.\r\n" );
    return;
  }

 
  if( IS_NPC( victim ) &&
      IS_SET_AR( MOB_FLAGS( victim ), MOB_AWARE ) &&
      AWAKE(victim))
  {
    act( "$N is far too alert to catch off guard.\r\n",
         FALSE, ch, 0, victim, TO_CHAR );
    damage(ch, victim, 0, THIS_SKILL);
    STUN_USER_RANGE;
    return;
  }

  // assassination gives you a chance to defeat monkeying
  assassinated = skillSuccess( ch, SKILL_ASSASSINATION);
  if (AWAKE(victim))
	  if( !assassinated && artMonkey(ch, victim, THIS_SKILL) ) {
		  STUN_USER_RANGE;
		  return;
	  }

  // if you're hiding, you have a 5% bonus to land your stab
  if( assassinated || (skillSuccess( ch, THIS_SKILL) || 
      (IS_AFFECTED(ch, AFF_HIDE) && percentSuccess(5))))
  {
    advanceSkill( ch, THIS_SKILL,
                      SKILL_MAX_LEARN,
                      SKILL_ADVANCE_STRING,
                      DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS );
    hit(ch, victim, THIS_SKILL);
    STUN_VICTIM_MIN;
    STUN_USER_RANGE;
  }
  else
  {
    damage(ch, victim, 0, THIS_SKILL);
    WAIT_STATE(ch, SET_STUN(4));
  }

}


/* this function determines if an NPC is able to backstab. */
int mobCanBackstab(CharData *ch)
{
  int class = GET_CLASS(ch);
  int level = GET_LEVEL(ch);
  int has_back_wpn;

  if (WIELDING(ch) &&
     (GET_OBJ_VAL(ch->equipment[WEAR_WIELD], 3) == (TYPE_PIERCE - TYPE_HIT)))
    has_back_wpn = TRUE;
  else
    has_back_wpn = FALSE;

  if ((class == CLASS_ASSASSIN) && has_back_wpn)
	return 1;
  else if ((class == CLASS_THIEF) && (level >= 5) && has_back_wpn)
	return 1;
  else if ((class == CLASS_SHADOW_DANCER) && (level >= 10) && has_back_wpn)
	return 1;

  return 0; /* can't backstab */
}

int backstab_multiplier(CharData *ch)
{
	switch(GET_CLASS(ch)) {
	case CLASS_THIEF:
		if (GET_LEVEL(ch) < 5)
			return 1;
		if (GET_LEVEL(ch) <= 16)
			return 2;
		if (GET_LEVEL(ch) <= 28)
			return 3;
		if (GET_LEVEL(ch) <= 40)
			return 4;
		if (GET_LEVEL(ch) <= 49)
			return 5;
		return 6;
	case CLASS_ASSASSIN:
		if (GET_LEVEL(ch) < 1)
			return 1;
		if (GET_LEVEL(ch) <= 10)
			return 2;
		if (GET_LEVEL(ch) <= 20)
			return 3;
		if (GET_LEVEL(ch) <= 30)
			return 4;
		if (GET_LEVEL(ch) <= 40)
			return 5;
		if (GET_LEVEL(ch) <= 49)
			return 6;
		return 7;
	case CLASS_SHADOW_DANCER:
		if (GET_LEVEL(ch) < 10)
			return 1;
		if (GET_LEVEL(ch) <= 25)
			return 2;
		if (GET_LEVEL(ch) <= 40)
			return 3;
		if (GET_LEVEL(ch) <=49)
			return 4;
		return 5;
	default:
		break;
	};
	return 1;
}
