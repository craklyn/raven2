
#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/class.h"
#include "general/comm.h"
#include "general/handler.h"
#include "actions/interpreter.h"
#include "util/utils.h"
#include "specials/special.h"
#include "magic/spells.h"
#include "util/weather.h"
#include "magic/backstab.h"
#include "magic/sing.h"
#include "actions/act.h"          /* ACMDs located within the act*.c files */
#include "actions/fight.h"

/*
** IS_TARGET will return a true if v(ictim) is a valid target for
** the super aggressor. The following conditions must be met
** for the aggressor to attack:
**    v cannot be null
**    v cannot be a mob
**    a must be able to see v
**    v cannot be nohassle
*/
#define IS_TARGET(a,v) ((v != NULL) &&    \
                        !IS_MOB(v) &&     \
                         CAN_SEE(a, v) && \
                        !PRF_FLAGGED(v, PRF_NOHASSLE))

CharData *
pickVictim( CharData *ch )
{
    CharData *victim = NULL;
    int       pCount = 0;
    int       target = 0;

    if( FIGHTING(ch) ) return NULL;
    if (ch->in_room == NOWHERE) return NULL;

    for( victim = world[ch->in_room].people;
         victim != NULL;
         victim = victim->next_in_room )
        if( IS_TARGET(ch,victim) )
        {
            pCount += 1;
        }

    if( pCount == 0 ) return NULL;

    target = number( 1, pCount );

    for( victim = world[ch->in_room].people; target; victim = victim->next_in_room) {
        if( IS_TARGET(ch,victim) ){
            target -= 1;
            if( target == 0 ) return( victim );
        }
    }
    /*
    ** If we get here something went wrong.
    */
    return NULL;
}

SPECIAL(superAggr)
{
    /*
    ** First, let's locate a random target.
    */
    CharData *attacker = me;
    CharData *victim   = pickVictim( attacker );

    /*
    ** Mortius, lets stop aggro mobs attack if switched
    */
    if (attacker->desc != NULL)
        return FALSE;

    if(GET_LEVEL(ch) >= LVL_IMMORT)
        return FALSE;

    if( victim == NULL ){
        smart_NPC_combat(ch);
        return FALSE;
    }
    if (STUNNED(attacker) || GET_POS(attacker) < POS_STANDING)
	return FALSE;
    mudlog(NRM, LVL_IMMORT, TRUE, "AGGRO: %s attacking %s", GET_NAME(attacker), GET_NAME(victim));

	  if( !mobCanBackstab( attacker ) || victim->mount )
          {
            hit( attacker, victim, TYPE_UNDEFINED );
          }
	  else
          {
            do_backstab( attacker, victim->player.name, 0, 0 );
          }

    smart_NPC_combat( attacker );

    return( TRUE );
}


/*
** Copy of aggroCh from mobact except it's race based.
*/
int
aggroRace( CharData *ch,
           int aggRace )
{
  CharData *vict;
  int found = 0;

  if( FIGHTING(ch) ) return FALSE;

  for( vict = world[ch->in_room].people;
       vict && !found;
       vict = vict->next_in_room)
  {
    if( GET_RACE(vict) != aggRace ) continue;

    if( ch == vict ) continue;

    if( !mobCanBackstab(ch) || vict->mount ) {
      mob_talk(ch, vict, 4);
      hit(ch, vict, TYPE_UNDEFINED);
    }
    else
    {
     do_backstab( ch, vict->player.name, 0, 0 );
    }
    return FALSE;
  }
  return FALSE;
}

SPECIAL(humanAggr)
{
  return( aggroRace(me, RACE_HUMAN ));
}

