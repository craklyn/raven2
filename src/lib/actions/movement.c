 /* ************************************************************************
*   File: movement.c                                    Part of CircleMUD *
*  Usage: movement commands, door handling, & sleep/rest/etc state        *
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
#include "actions/fight.h"
#include "actions/interpreter.h"
#include "general/handler.h"
#include "general/class.h"
#include "magic/spells.h"
#include "specials/house.h"
#include "util/weather.h"
#include "actions/act.h"          /* ACMDs located within the act*.c files */
#include "specials/mobact.h"
#include "magic/backstab.h"
#include "magic/skills.h"
#include "magic/trap.h"
#include "magic/ambush.h"
#include "magic/aggressive.h"
#include "scripts/dg_scripts.h"
#include "specials/muckle.h"
#include "magic/sing.h"
#include "magic/knock.h"
#include "specials/portal.h"

/* local file functs */
void checkSuperAgg( CharData *ch );
int check_one_person(CharData *ch, int to_room);

/* ============================================================================ 
This function checks if anything in the room should attack ch. If so, then it
starts the fight.
============================================================================ */
void
aggroRoomCheck( CharData *ch, int sneaking )
{	
	CharData *attacker;

	for( attacker = world[ch->in_room].people; attacker; attacker = attacker->next_in_room )
	{
            /* Mortius, this should stop mobs being aggro if switched */
            if (IS_NPC(attacker))
                if (attacker->desc != NULL)
                    continue;

	    if (sneaking) sneaking = 1;  //Sanity.
            
            if(number(1, 2) - sneaking)  // 50% chance of catching sneakers
                if ( !IS_NPC(attacker) && ambushVictim(attacker, ch) )
                {
                    act("You leap out of hiding at $N!", TRUE, attacker, 0, ch, TO_CHAR);
                    act("$n leaps out of hiding at $N!", TRUE, attacker, 0, ch, TO_NOTVICT);
                    //act("$n leaps out of hiding at you!", TRUE, attacker, 0, ch, TO_VICT);
                    do_backstab(attacker, attacker->ambushing, 0, 0);
                    free(attacker->ambushing); attacker->ambushing = NULL;
                }

            if( sneaking ) /* Perhaps they noticed ch arrive? */
            {
                if (attacker == ch) continue; /* You can already hear _yourself_ */
                if (GET_INVIS_LEV(ch) > GET_LEVEL(attacker)) continue;

                if (skillSuccess(attacker, SKILL_HEIGHTENED_SENSES)) {
                    act("You hear $N arrive.", TRUE, attacker, 0, ch, TO_CHAR);
                    advanceSkill(attacker, SKILL_HEIGHTENED_SENSES,
                            (GET_LEVEL(attacker) - 35) * 2 + 60, NULL,
                            FALSE, FALSE, FALSE);
                } else
                    continue; /* ch has the drop on this one. */
            }

            if( !IS_NPC(attacker) && pcAttackVictim(attacker, ch) )
            {
                act("You react swiftly as $N arrives!", TRUE, attacker, 0, ch, TO_CHAR);
                act("$n turns on you with lightning speed!", TRUE, attacker, 0, ch, TO_VICT);
                act("$n reacts with lightning reflexes as $N arrives!", TRUE, attacker, 0, ch, TO_NOTVICT);
                hit(attacker, ch, TYPE_UNDEFINED);
            }

            if( IS_NPC(attacker) && !mobAttackVictim(attacker, ch) ) continue;

            /*
             ** 1 in 5 chance mob gets a shot at ch.
             */
            else if( IS_NPC(attacker) && (number(1, 5) == 1) )
            {
                mob_attack(attacker, ch);
            } /* else if */
        } /* for loop */
} /* aggroRoomCheck */


void deathTrapKill(CharData *ch)
{
	if (IS_NPC(ch)) {
	    mudlog(NRM, LVL_IMMORT, TRUE, "SYSERR: deathTrapKill: mob %s sent to die?", GET_NAME(ch));
	    return;
	}
    death_cry(ch);
        mudlog( BRF, LVL_IMMORT, TRUE, "%s hit death trap #%d (%s)", GET_NAME(ch),
	world[ch->in_room].number, world[ch->in_room].name);
        death_cry(ch);
	ZKILL_CNT(ch);
        GET_HIT(ch) = GET_MANA(ch) = 0;
	if (IN_QUEST_FIELD(ch) && (PRF_FLAGGED(ch, PRF_GOLD_TEAM) || PRF_FLAGGED(ch, PRF_BLACK_TEAM) || PRF_FLAGGED(ch, PRF_ROGUE_TEAM)))
	    jail_char(ch, TRUE);
	else
	    make_ghost(ch);
}


//-------------------------------------------------------------------------
//
// This function performs the actual movement of ch to a new room. ch is
// the creature thats doing the moving. dir is the direction that ch is
// moving in.
//
// Mortius : added the trigging in here for traps.  This is the best
// place I could think of to add it.
// Imhotep : Removed Mortius' trap code in favour of new methods
//
int
doTheMove( CharData *ch, int dir )
{
  CharData *rider = ch->rider;
  int mounted     = (rider != NULL ? 1 : 0);
  int sneaking    = (IS_AFFECTED( ch, AFF_SNEAK ));
  int toRoom      = EXIT(ch,dir)->to_room;
  int fromRoom    = ch->in_room;
  int tmpRoom     = 0;
  int dam;
  RoomDirectionData *to_wld, *from_wld;

  /* if they're singing, and it's broken by movement, shut them up */
  if (SINGING(ch) && SONG_FLAGS(ch, SST_MOVEMENT)) {
      stop_singing(ch);
  }

  /* check for traps
   */
  if (GET_TRAP(EXIT(ch,dir)) > 0) {
    trigger_wld_trap(ch, dir);
    return 0;
  }

    /* moving into a death trap! */
    if (IS_SET_AR(ROOM_FLAGS(EXIT(ch, dir)->to_room), ROOM_DEATH)) {
        /* halflings are lucky buggers, sometimes */
        if ((IS_HALFLING(ch) && number(1,100) < 60) ||
                (GET_RACE(ch) == RACE_SHALFLING && number(1, 100) < 60)) {
            send_to_char("You trip and fall flat on your face!\r\n", ch);
            GET_POS(ch) = POS_SITTING;
            return 0;
        }
    }

  /*
  ** Some races can sneak in certain terrain:
  **   Elves can sneak in forests and fields and Halflings can sneak
  **   if they are barefooted.  Drow sneak indoors. Amara sneak
  **   on the 4 different water sectors.
  */
  if((IS_HALFLING(ch) && !ch->equipment[WEAR_FEET]) ||
     ( IS_ELF(ch)      &&
     ((world[toRoom].sector_type == SECT_FOREST) ||
      (world[toRoom].sector_type == SECT_FIELD) ||
      (world[toRoom].sector_type == SECT_LIGHT_FOREST) ||
      (world[toRoom].sector_type == SECT_THICK_FOREST)) &&
     ((world[(ch)->in_room].sector_type == SECT_FOREST) ||
      (world[(ch)->in_room].sector_type == SECT_FIELD) ||
      (world[(ch)->in_room].sector_type == SECT_LIGHT_FOREST) ||
      (world[(ch)->in_room].sector_type == SECT_THICK_FOREST))) ||
     (IS_AMARA(ch)      &&
      ((world[toRoom].sector_type == SECT_UNDERWATER) ||
      (world[toRoom].sector_type == SECT_UNDERWATER_RIVER)) &&
     ((world[(ch)->in_room].sector_type == SECT_UNDERWATER) ||
      (world[(ch)->in_room].sector_type == SECT_UNDERWATER_RIVER))) ||
     (IS_DROW(ch)      &&
     (world[toRoom].sector_type == SECT_INSIDE ||
     ROOM_FLAGGED(toRoom, ROOM_INDOORS)) &&
     (world[(ch)->in_room].sector_type == SECT_INSIDE ||
     ROOM_FLAGGED((ch)->in_room, ROOM_INDOORS))) ||
     IS_IZARTI(ch))
    sneaking = 1;

    /*
    ** They leave the room
    */
    if( !sneaking ) {
        if( ch->rider ) {
            sprintf(buf2, "$n rides off %swards.", dirs[dir]);
            act(buf2, TRUE, ch->rider, 0, 0, TO_ROOM);
        } else {
            sprintf(buf2, "$n departs %s.", dirs[dir]);
            act(buf2, TRUE, ch, 0, 0, TO_ROOM);
        }
    } else {
        /* check all other people in the room for heightened senses */
        CharData *ch2;
        for (ch2 = world[ch->in_room].people; ch2; ch2 = ch2->next_in_room) {
            if (skillSuccess(ch2, SKILL_HEIGHTENED_SENSES) && ch2 != ch) {
                if (mounted) {
                    sprintf(buf2, "You hear $N ride off to the %s.", dirs[dir]);
                    act(buf2, TRUE, ch2, 0, ch, TO_CHAR);
                } else {
                    sprintf(buf2, "You hear $N depart %s.", dirs[dir]);
                    act(buf2, TRUE, ch2, 0, ch, TO_CHAR);
                }
            }
        }
  }

  if (IN_ROOM(ch) != -1) {
    /* see if an entry trigger disallows the move */
    if (!entry_mtrigger(ch))
      return 0;
    if (!enter_wtrigger(&world[EXIT(ch, dir)->to_room], ch, dir))
      return 0;
  }

  /* the entry trigger might have caused a death! */
  if (ch->in_room < 0) return 0;

  tmpRoom = ch->in_room;
  char_from_room(ch);
  char_to_room(ch, world[tmpRoom].dir_option[dir]->to_room);

  if( ch->rider )
  {
    tmpRoom = rider->in_room;
    char_from_room( rider );
    char_to_room( rider, world[tmpRoom].dir_option[dir]->to_room );
  }

  /*
  ** They enter the room
  */
  if( !sneaking )
  {
    if( IS_AFFECTED(ch, AFF_HIDE) ) /* hide broken if not sneaking */
      REMOVE_BIT_AR( AFF_FLAGS(ch), AFF_HIDE );

    if( ch->rider )
      act("$n has arrived riding $N.", TRUE, ch->rider, 0, ch, TO_ROOM );
    else
      act("$n has arrived.", TRUE, ch, 0, 0, TO_ROOM);
  }

  look_at_room(ch, 0);

  if(muckle_active)
  {
      if(!the_muckle) {
          if(ch->rider && !IS_NPC(ch->rider) && IN_ARENA(ch->rider))
              the_muckle = ch->rider;
          else if(!IS_NPC(ch) && IN_ARENA(ch))
              the_muckle = ch;

          if(the_muckle){
              sprintf(buf, "&08MUCKLE:&00 &14%s is now the muckle!&00\r\n", GET_NAME(the_muckle));
              quest_echo(buf);
          }
      }
  }

  if(ch->rider) look_at_room(rider, 0);

  if( IS_SET_AR( ROOM_FLAGS(ch->in_room), ROOM_DEATH ) && IS_MORTAL(ch) )
  {
    if( ch->rider && !IS_NPC(ch->rider))
    {
      deathTrapKill(rider);
      ch->rider->mount = NULL;
      ch->rider = NULL;
    }

    if( !IS_NPC(ch) ) deathTrapKill(ch);

    return;
  }

  // This code below is terrible.  I should remember to clean it up some day.
  // (Yeah right, nine years from now someone will find it and hate me.)  - Arbaces 4/29/09
   if (EXIT(ch, rev_dir[dir]) && GET_TRAP(EXIT(ch,rev_dir[dir])) > 0) {
       to_wld = world[ch->in_room].dir_option[rev_dir[dir]];
       char_from_room(ch);
       char_to_room(ch, fromRoom);
       from_wld = world[ch->in_room].dir_option[dir];
       GET_TRAP(from_wld) = GET_TRAP(to_wld);
       trigger_wld_trap(ch, dir);
       GET_TRAP(from_wld) = GET_TRAP(to_wld) = 0;
       if(ch && ch->in_room != NOWHERE) {
           char_from_room(ch);
           char_to_room(ch, toRoom);
       }
       return 0;
   }

  /*
  ** perform aggro checks here.
  */
  if( ch->rider )
  {
    aggroRoomCheck(ch->rider, sneaking);
  }
  else
  {
    aggroRoomCheck(ch, sneaking);
  }

  // If their skill in poultice > 0 and they entered a forest room, they might
  // find a magic mushroom!
  if (IN_ROOM(ch) != -1 && GET_SKILL(ch, SKILL_POULTICE) > 0 &&
        HAS_HERBS(ch) < MAX_HERBS(ch) &&
        world[ch->in_room].sector_type == SECT_FOREST &&
        !affected_by_spell(ch, SKILL_POULTICE)) {
      if (skillSuccess(ch, SKILL_POULTICE) && percentSuccess(5)) {
          HAS_HERBS(ch) += 1;
          send_to_char("You discover some moss and lichen appropriate for your poultices!\r\n", ch);
          sprintf(buf, "You now have enough materials for %d bandage%s.\r\n", HAS_HERBS(ch), HAS_HERBS(ch)>1?"s":"");
          send_to_char(buf, ch);
          add_affect(ch, ch, SKILL_POULTICE, GET_LEVEL(ch), 0, 0, 
                  number(5, (1+HAS_HERBS(ch)) TICKS), 0, FALSE, FALSE, FALSE, FALSE);
      }
  }
  // It wasn't checking for fields.  Code's messy enough, so copied the previous
  // block to consider fields.  -Craklyn 7/29/05
  if (IN_ROOM(ch) != -1 && GET_SKILL(ch, SKILL_POULTICE) > 0 &&
        HAS_HERBS(ch) < MAX_HERBS(ch) &&
        world[ch->in_room].sector_type == SECT_FIELD &&
        !affected_by_spell(ch, SKILL_POULTICE))
  {
      if (skillSuccess(ch, SKILL_POULTICE) && percentSuccess(5))
      {
          HAS_HERBS(ch) += 1;
          send_to_char("You spot some herbs you need for poultices!\r\n", ch);
          sprintf(buf, "You now have enough materials for %d bandage%s.\r\n", HAS_HERBS(ch), HAS_HERBS(ch)>1?"s":"");
          send_to_char(buf, ch);
          add_affect(ch, ch, SKILL_POULTICE, GET_LEVEL(ch), 0, 0,
                  number(5, (1+HAS_HERBS(ch)) TICKS), 0, FALSE, FALSE, FALSE, FALSE);
      }
  }

  // Below is Call to Corpse spell's effects.
  if (IN_ROOM(ch) != -1) {
      if ( affected_by_spell(ch, SPELL_CALL_TO_CORPSE) && (ch->call_to_corpse == 1) ) {
          // Hurt them about 4% for each room they move
          dam = GET_MAX_HIT(ch) * 4 / number(95, 105);
          dam = MAX(1, MIN(dam, 100));
          ch->call_to_corpse = 0;
          GET_HIT(ch) -= dam;

          update_pos(ch);
          
          if(GET_POS(ch) == POS_DEAD && !pvpHoliday(ch))
          {
              send_to_char("The sound of your heartbeat fades away...\r\n", ch);
              act("$N slows and fades away...", FALSE, ch, 0, ch, TO_NOTVICT);
              ch_kill_victim(ch, ch);
          }
          
          if(IN_ROOM(ch) != -1)
              send_to_char("You feel one step closer to the grave.\r\n", ch);
          
      }
  }

  // 0.4 second stun when instant poisoned.  0.4 seconds for hamstrung.  Extra stun for being the muckle
  int thestun = affected_by_spell(ch, SKILL_INSTANT_POISON)? PULSE_VIOLENCE / 2:0
                  + IS_AFFECTED(ch, AFF_HAMSTRUNG)? PULSE_VIOLENCE / 2:0
                  + the_muckle == ch ? ((ch->muckleTime)/60 + 1)*4 * (pvpFactor()+1) / 2 : 0;
  if (thestun > 0) {
      WAIT_STATE(ch, thestun);
  }

    // NOTE: ch may be dead now!
  if (IN_ROOM(ch) != -1) {
      if (ch->rider) {
          entry_memory_mtrigger(ch->rider);
          if (IN_ROOM(ch) != -1)
              if (!greet_mtrigger(ch->rider, dir)) {
                  char_from_room(ch);
                  char_to_room(ch, tmpRoom);
                  look_at_room(ch, 0);
              } else {
                  greet_memory_mtrigger(ch->rider);
              }
      } else {
          entry_memory_mtrigger(ch);
          if (!greet_mtrigger(ch, dir)) {
              char_from_room(ch);
              char_to_room(ch, tmpRoom);
              look_at_room(ch, 0);
          } else greet_memory_mtrigger(ch);
      }
      // remaining careful, because they may have died from those triggers
      if(IN_ROOM(ch) != -1 &&  ROOM_FLAGGED(IN_ROOM(ch), ROOM_FALL)
              && !IS_AFFECTED(ch, AFF_FLY))
      {
          doFallRoom(ch);
      }
  } else return 0;

  return 1;
} /* doTheMove */

#define WATERY(t) (t == SECT_WATER_SWIM || t == SECT_WATER_NOSWIM || \
                   t == SECT_UNDERWATER || t == SECT_UNDERWATER_RIVER)

/* do_simple_move assumes
 *	1. That there is no master and no followers.
 *	2. That the direction exists.
 *
 *   Returns :
 *   1 : If succes.
 *   0 : If fail
*/
int
do_simple_move( CharData * ch, int dir, int following)
{
  static const int movement_loss[NUM_ROOM_SECTORS] = {
    1,   /* 0 Inside     	*/ 	
    1,   /* 1 City       	*/
    2,   /* 2 Field      	*/ 	
    3,   /* 3 Forest     	*/
    4,   /* 4 Hills      	*/ 	
    6,   /* 5 Mountains  	*/
    4,   /* 6 Swimming   	*/ 	
    1,   /* 7 Unswimable 	*/
    4,   /* 8 Underwater 	*/ 	
    1,   /* 9 Flying     	*/
    0,   /* 10 Underwater River */ 	
    4,   /* 11 Corpse room 	*/ 
    2,	 /* 12 Road		*/	
    2,   /* 13 Plain		*/
    5,   /* 14 Rocky		*/      
    6,   /* 15 Muddy		*/
    5,   /* 16 Sand		*/	
    4,   /* 17 Thin Forest  	*/
    6	 /* 18 Thick Forest 	*/
  };

  int moveCost;
  ObjData *obj;
  int chRoom = ch->in_room;

  int special(CharData * ch, int cmd, char *arg);

  // Check for special routines (North is 1 in command list, but 0 here) Note
  // -- only check if following; this avoids 'double spec-proc' bug
  //
  if( following && special( ch, dir+1, "" )) return 0;

  if( IS_AFFECTED(ch, AFF_CHARM) && ch->master && chRoom == ch->master->in_room)
  {
    sendChar( ch, "The thought of leaving your master makes you weep.\r\n" );
    return 0;
  }

  // The following line is what is wrong with coders. Digger 1/1/2000
  //
  moveCost =
    (movement_loss[world[chRoom].sector_type] +
     movement_loss[world[world[chRoom].dir_option[dir]->to_room].sector_type]) >> 1;

# ifdef ARENA_MOVE_FREE
  if( IN_ARENA(ch) ) moveCost = 0;
# endif

  if( IS_AFFECTED(ch, AFF_SHADOW_WALK) &&
      IS_DARK(world[chRoom].dir_option[dir]->to_room))
    moveCost = 0;

  else if( CAN_FLY(ch) )
    moveCost = ( moveCost <= 4 ? 1 : moveCost - 3 );

  else if (affected_by_spell(ch, SPELL_FLEET_FOOT))
    moveCost = ( moveCost <= 3 ? 1 : moveCost - 2 );

  if((world[EXIT(ch, dir)->to_room].sector_type == SECT_FLYING) &&
     (!CAN_FLY(ch)))
  {
    sendChar( ch, "You must be flying to go there.\r\n" );
    return 0;
  }

  if( ((world[chRoom].sector_type == SECT_WATER_NOSWIM) ||
       (world[EXIT(ch, dir)->to_room].sector_type == SECT_WATER_NOSWIM)) &&
       (!CAN_FLY(ch)) && ( !( (GET_SUBRACE(ch) == WATER_ELEMENTAL) || IS_AMARA(ch)) ))
  {
    int hasBoat = 0;
    for( obj = ch->carrying; (obj && !hasBoat); obj = obj->next_content)
    {
      if( GET_OBJ_TYPE(obj) == ITEM_BOAT )
        hasBoat = 1;
    }

    if( !hasBoat )
    {
      sendChar( ch, "You need a boat to go there.\r\n" );
      return 0;
    }
  }

  /* Amara get free movement going from water to water */
  if (WATERY(world[chRoom].sector_type) &&
      WATERY(world[EXIT(ch, dir)->to_room].sector_type) &&
      IS_AMARA(ch))
      moveCost = 0;

  if( GET_RACE(ch) == RACE_DROW &&
      IS_SUNLIGHT(world[chRoom].dir_option[dir]->to_room))
    moveCost = moveCost*2;

  if(GET_MOVE(ch) < moveCost && !IS_NPC(ch))
  {
    if(following && ch->master) {
      sendChar( ch, "You are too exhausted to follow.\r\n" );
      return 0;
    }
    else {
      sendChar( ch, "You are exhausted and move quite slowly.\r\n" );
      GET_MOVE(ch) = GET_MOVE(ch) + moveCost;
      WAIT_STATE(ch, PULSE_VIOLENCE*(1 + moveCost)/3);
    }
  }

#ifdef USE_HOUSES
    if (IS_SET_AR(ROOM_FLAGS(chRoom), ROOM_ATRIUM)) {
    if( !House_can_enter(ch, world[EXIT(ch, dir)->to_room].number) ){
      send_to_char("That's private property -- no trespassing!\r\n", ch);
    return 0;
    }
  }
#endif

#ifdef CLAN_WALK
  if (IS_SET_AR(ROOM_FLAGS(EXIT(ch, dir)->to_room), ROOM_CLAN) &&
      GET_LEVEL(ch) < LVL_LRGOD )
  {
  if (PLR_FLAGGED(ch, PLR_KILLER))
    {
      sendChar( ch, "Killers can't hide in clan halls." );
      return 0;
    }
    else if( world[EXIT(ch,dir)->to_room].clan_id != GET_CLAN(ch) )
    {
      sendChar( ch, "A force field repulses you backwards.\r\n" );
      return 0;
    }
  }
#endif

  // HUNTED players cannot enter peace or arena rooms - Vex.
  //
  if (PLR_FLAGGED(ch, PLR_HUNTED) &&
    ( IS_SET_AR(ROOM_FLAGS(EXIT(ch, dir)->to_room), ROOM_PEACEFUL) ||
    ( zone_table[world[EXIT(ch,dir)->to_room].zone].tournament_room != 0) ||
    IS_SET_AR(ROOM_FLAGS(EXIT(ch, dir)->to_room), ROOM_CLAN)))
  {
    sendChar( ch, "It's just no fun at all if you hide in there." );
    return 0;
  }

  if( IS_MORTAL(ch) && !IS_NPC(ch)) GET_MOVE(ch) -= moveCost;

  /* Mortius */
  if (!UNDERWATER(ch)) 
      WATER_COUNTER(ch) = 0; /* Reset the counter if the player hits air */

  /* One Person Only - Mortius 16/05/2000 (UK Date Not US :P) */

  if( IS_SET_AR(ROOM_FLAGS(EXIT(ch, dir)->to_room), ROOM_ONE_PERSON) &&
      IS_MORTAL(ch))
  {
    if( !check_one_person(ch, real_room(world[EXIT(ch, dir)->to_room].number)))
    {
      return FALSE;
    }
  }

    /* make sure no bone wall is blocking that direction */
    obj = world[ch->in_room].contents;
    while (obj) {
        if (GET_OBJ_VNUM(obj) == BONE_WALL_OBJ_VNUM &&
                GET_OBJ_VAL(obj, 0) == dir) {
            sendChar(ch, "A wall of bone fragments is blocking your path!\r\n");
            return FALSE;
        }
        obj = obj->next_content;
    }

  if (PLR_FLAGGED(ch, PLR_FISHING)) {
      send_to_char("You pack up your fishing gear and move on.\r\n\r\n", ch);
      REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_FISHING);
      REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_FISH_ON);
  }

  return doTheMove(ch, dir);
}

/* Lets check there are no other players in the room.  If there are the player
 * can't get in
*/

int check_one_person(CharData *ch, int to_room)
{
  CharData *mob;

  if (ch->rider || ch->mount) {
      send_to_char("You cannot enter there while mounted...\r\n", ch);
      return FALSE;
  }

  if (world[to_room].people > 0)
      for (mob = world[to_room].people; mob; mob = mob->next_in_room) {
           if (IS_NPC(mob))
               continue;
           else {
             send_to_char("Alas, there is not enough room...\r\n", ch);
             return FALSE;
           }
     }
  return TRUE;
}

/* Imhotep: this should be checked for different ways of moving */
int exit_guarded( CharData *ch, int dir)
{
  CharData *mob;

  /* Mortius: New way for creating guards that block */
  for( mob = world[ch->in_room].people; mob; mob = mob->next_in_room)
  {
    if( IS_NPC(mob) &&
      ( MOB_FLAGGED(mob, MOB_GUARD_CLASS) ||
        MOB_FLAGGED(mob, MOB_GUARD_RACE ) ||
        MOB_FLAGGED(mob, MOB_GUARD_BOTH )))
    {
      if( mob_block(ch, mob, dir) )
      {
        return TRUE;
      }
    }
    else
      continue;
  }
  return FALSE;
}

int
perform_move( CharData *ch, int dir, int following)
{
  int was_in;
  FollowType *k, *next;
  CharData *mover = ch;

  if (!ch || dir < 0 || dir >= NUM_OF_DIRS) return 0;

  if (!EXIT(ch, dir) || EXIT(ch, dir)->to_room == NOWHERE){
    send_to_char("Alas, you cannot go that way...\r\n", ch);
    return 0;
  }
#ifdef NAEMOBS
  if (IS_NPC(ch) && IS_SET_AR(ROOM_FLAGS(EXIT(ch, dir)->to_room), ROOM_NOMOB)) {
    send_to_char("Alas, you cannot go that way...\r\n", ch);
    return 0;
  }
#endif

#ifdef CLAN_WALK
  if (IS_SET_AR(ROOM_FLAGS(EXIT(ch, dir)->to_room), ROOM_CLAN) && (GET_LEVEL(ch) < LVL_LRGOD)) {
      if( world[EXIT(ch,dir)->to_room].clan_id != GET_CLAN(ch) ){
          send_to_char("Alas, you cannot go that way...\r\n", ch);
          return 0;
      }
  }
#endif

  if( exit_guarded(ch, dir))
      return FALSE;

  if( IS_SET(EXIT(ch, dir)->exit_info, EX_CLOSED))
  {
      if( EXIT(ch, dir)->keyword)
      {
          sendChar( ch, "The %s seems to be closed.\r\n",
                  fname(EXIT(ch, dir)->keyword));
      }
      else
      {
          sendChar( ch, "It seems to be closed.\r\n" );
      }
      return 0;
  }
  
  /* Tunnel support - Kaidon 5/23/97 */
  if( IS_SET_AR(ROOM_FLAGS(EXIT(ch, dir)->to_room), ROOM_TUNNEL) &&
      IS_MORTAL(ch))
  {
    if( world[EXIT(ch,dir)->to_room].people > 0  || ch->mount || ch->rider)
    {
      sendChar( ch, "Alas, there is not enough room...\r\n" );
      return 0;
    }
  }

  /* One Person Only - Mortius 16/05/2000 (UK Date Not US :P) */
  if( IS_SET_AR(ROOM_FLAGS(EXIT(ch, dir)->to_room), ROOM_ONE_PERSON) &&
      IS_MORTAL(ch))
  {
    if( !check_one_person(ch, real_room(world[EXIT(ch, dir)->to_room].number)))
    {
      return FALSE;
    }
  }

  if( IS_MORTAL(ch) )
  {
    if( ZONE_FLAGGED(world[EXIT(ch, dir)->to_room].zone, ZONE_NOMORTAL ))
    {
      sendChar( ch, "You are not godly enough to go there!\r\n" );
      return FALSE;
    }

    if( ZONE_FLAGGED(world[EXIT(ch, dir)->to_room].zone, ZONE_CLOSED))
    {
      sendChar( ch, "That zone is closed right now.  Try again later.\r\n" );
      return FALSE;
    }
  } /* End of immortal check */

  /* Small folks only please - Mortius 09-Aug-2000 */
  if( IS_MORTAL(ch) )
  {
      if (ROOM_FLAGGED(EXIT(ch, dir)->to_room, ROOM_SMALL))
          if (!IS_GNOME(ch))  {
              send_to_char("Your bulk will not allow you to enter.", ch);
              return FALSE;
          }
  }

  if (ch && ch->mount && ch->mount->in_room != ch->in_room) {
      sendChar(ch, "You have become separated from your mount!\r\n");
	  if (ch->mount != NULL)
		  ch->mount->rider = NULL;
      ch->mount = NULL;
  }

  if (ch && ch->rider && ch->rider->in_room != ch->in_room) {
      sendChar(ch, "You have become separated from your rider!\r\n");
      if (ch->rider != NULL)
		  ch->rider->mount = NULL;
      ch->rider = NULL;
  }

  if( !IS_NPC(ch) && IS_MORTAL(ch) &&
    ( zone_table[world[EXIT(ch,dir)->to_room].zone].reset_mode == 3)) {
    sendChar(ch, "Mere mortals may not enter there.\r\n");
    return 0;
  }
  else {
    if(ch->mount != NULL) {
      mover = ch->mount;
    }

    if(ch->followers == NULL) {
      return( do_simple_move( mover, dir, following ));
    }

    was_in = ch->in_room;
    if(!do_simple_move(mover, dir, following))
      return 0;

    for( k = ch->followers; k; k = next)
    {
      next = k->next;
      if((was_in == k->follower->in_room) &&
         (GET_POS(k->follower) >= POS_STANDING))
      {
	act( "You follow $N.\r\n", FALSE, k->follower, 0, ch, TO_CHAR );
	perform_move(k->follower, dir, TRUE);
      }
      /* Make charmed mobs run away if master leaves them. */
      else if(( was_in == k->follower->in_room) &&
	      ( GET_POS(k->follower) == POS_FIGHTING) &&
	        IS_AFFECTED(k->follower, AFF_CHARM) &&
	      ( number(1, 10) > 7 )) /* 30% chance mob leaves as well */
      {
	sendChar( k->follower, "Oh no! Your master has left you to die!\r\n" );
	do_flee(k->follower, "", 0, 0);
      }
    }
    return 1;
  }

  return 0;
}


ACMD(do_move)
{
  /*
   * This is basically a mapping of cmd numbers to perform_move indexes.
   * It cannot be done in perform_move because perform_move is called
   * by other functions which do not require the remapping.
   */
  perform_move(ch, cmd - 1, 0);
}


int find_door(CharData * ch, char *type, char *dir)
{
  int door;

  if (dir && *dir) {			/* a direction was specified */
    if ((door = search_block(dir, dirs, FALSE)) == -1) {	/* Partial Match */
      send_to_char("That's not a direction.\r\n", ch);
      return -1;
    }
    if (EXIT(ch, door))
      if (EXIT(ch, door)->keyword)
	if (isname(type, EXIT(ch, door)->keyword))
	  return door;
	else {
	  sprintf(buf2, "I see no %s there.\r\n", type);
	  send_to_char(buf2, ch);
	  return -1;
	}
      else
	return door;
    else {
      send_to_char("I really don't see how you can close anything there.\r\n", ch);
      return -1;
    }
  } else {			/* try to locate the keyword */
    for (door = 0; door < NUM_OF_DIRS; door++)
      if (EXIT(ch, door))
	if (EXIT(ch, door)->keyword)
	  if (isname(type, EXIT(ch, door)->keyword))
	    return door;

    sprintf(buf2, "There doesn't seem to be %s %s here.\r\n", AN(type), type);
    send_to_char(buf2, ch);
    return -1;
  }
}


/* Mortius : Added trigging into here for if a player opens a trap.  Also added
	     IS_CONTAINER macro so we can open traps */
/* Imhotep : Removed checks, blah blah */

/* Imhotep: I don't like externs, but I'm lazy today */
extern int tics;
ACMD(do_open)
{
  int door, other_room;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  struct room_direction_data *back;
  static int last_lucky = -10000000;
  ObjData *obj = NULL;
  CharData *victim = NULL;

  if (IS_AFFECTED(ch, AFF_BLIND)) {
      send_to_char("You can't see, let alone open something.\r\n", ch);
      return;
  }

  two_arguments(argument, type, dir);

  /* Mortius: if there is no type we should exit the function */

  if (!*type) {
    send_to_char("Open what?\r\n", ch);
    return;
  } else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
		          ch, &victim, &obj))

    /* this is an object */

    if (!IS_CONTAINER(obj))
      send_to_char("That's not a container.\r\n", ch);
    else if (!IS_SET(GET_OBJ_VAL(obj, 1), CONT_CLOSED))
      send_to_char("But it's already open!\r\n", ch);
    else if (!IS_SET(GET_OBJ_VAL(obj, 1), CONT_CLOSEABLE))
      send_to_char("You can't do that.\r\n", ch);
    else if ((IS_SET(GET_OBJ_VAL(obj, 1), CONT_LOCKED)) && (GET_LEVEL(ch) < LVL_CREATOR))
      send_to_char("It seems to be locked.\r\n", ch);
    else if ((IS_SET(GET_OBJ_VAL(obj, 1), CONT_LOCKER)) && (GET_LOCKER(ch) != GET_OBJ_VNUM(obj)) && (GET_LEVEL(ch) < LVL_IMMORT))
      send_to_char("That's not your locker.\r\n", ch);
    else if (GET_TRAP(obj) > 0) {
      trigger_obj_trap(ch, obj);
    } else {
      REMOVE_BIT(GET_OBJ_VAL(obj, 1), CONT_CLOSED);
      sendChar( ch, CONFIG_OK );
      act("$n opens $p.", FALSE, ch, obj, 0, TO_ROOM);
    }
  else if ((door = find_door(ch, type, dir)) >= 0)
  {
    /* perhaps it is a door */

    if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR))
      send_to_char("That's impossible, I'm afraid.\r\n", ch);
    else if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
      send_to_char("It's already open!\r\n", ch);
    else if ((IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED)) && (GET_LEVEL(ch) < LVL_CREATOR))
      /* halflings are sometimes really lucky ... */
      if (IS_HALFLING(ch)
              && !IS_SET(EXIT(ch, door)->exit_info, EX_PICKPROOF)
              && (last_lucky + 15 * SECS_PER_MUD_HOUR SEC < tics)) {
          if (number(1,100) < 15) {
            send_to_char("You try to open it, and hear a *click* as the lock "
                    "falls open!\r\n", ch);
            REMOVE_BIT(EXIT(ch, door)->exit_info, EX_LOCKED);
          }
          last_lucky = tics;
      } else {
        send_to_char("It seems to be locked.\r\n", ch);
      }
    else {
      REMOVE_BIT(EXIT(ch, door)->exit_info, EX_CLOSED);
      if (EXIT(ch, door)->keyword)
	act("$n opens the $F.", FALSE, ch, 0, EXIT(ch, door)->keyword, TO_ROOM);
      else
	act("$n opens the door.", FALSE, ch, 0, 0, TO_ROOM);
      sendChar( ch, CONFIG_OK );
      /* now for opening the OTHER side of the door! */
      if ((other_room = EXIT(ch, door)->to_room) != NOWHERE)
	if ((back = world[other_room].dir_option[rev_dir[door]]))
	  if (back->to_room == ch->in_room) {
	    REMOVE_BIT(back->exit_info, EX_CLOSED);
	    if (back->keyword) {
	      sprintf(buf, "The %s is opened from the other side.\r\n",
		      fname(back->keyword));
	      send_to_room(buf, -EXIT(ch, door)->to_room);
	    } else
	      send_to_room("The door is opened from the other side.\r\n",
			   -EXIT(ch, door)->to_room);
	  }
    }
  }
}

/* Mortius : added IS_CONTAINER macro which includes traps so we can close em*/

ACMD(do_close)
{
  int door, other_room;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  struct room_direction_data *back;
  ObjData *obj;
  CharData *victim;

  if (IS_AFFECTED(ch, AFF_BLIND)) {
      send_to_char("You can't see, let alone close something.\r\n", ch);
      return;
  }

  two_arguments(argument, type, dir);

  if (!*type)
    send_to_char("Close what?\r\n", ch);
  else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj))
    /* this is an object */
    if (!IS_CONTAINER(obj))
      send_to_char("That's not a container.\r\n", ch);
    else if (IS_SET(GET_OBJ_VAL(obj, 1), CONT_CLOSED))
      send_to_char("But it's already closed!\r\n", ch);
    else if (!IS_SET(GET_OBJ_VAL(obj, 1), CONT_CLOSEABLE))
      send_to_char("That's impossible.\r\n", ch);
    else {
      SET_BIT(GET_OBJ_VAL(obj, 1), CONT_CLOSED);
      sendChar( ch, CONFIG_OK );
      act("$n closes $p.", FALSE, ch, obj, 0, TO_ROOM);
    }
  else if ((door = find_door(ch, type, dir)) >= 0)
  {
    /* Or a door */
    if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR))
      send_to_char("That's absurd.\r\n", ch);
    else if (IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
      send_to_char("It's already closed!\r\n", ch);
    else {
      SET_BIT(EXIT(ch, door)->exit_info, EX_CLOSED);
      if (EXIT(ch, door)->keyword)
	act("$n closes the $F.", 0, ch, 0, EXIT(ch, door)->keyword,
	    TO_ROOM);
      else
	act("$n closes the door.", FALSE, ch, 0, 0, TO_ROOM);
      sendChar( ch, CONFIG_OK );
      /* now for closing the other side, too */
      if ((other_room = EXIT(ch, door)->to_room) != NOWHERE)
	if ((back = world[other_room].dir_option[rev_dir[door]]))
	  if (back->to_room == ch->in_room) {
	    SET_BIT(back->exit_info, EX_CLOSED);
	    if (back->keyword) {
	      sprintf(buf, "The %s closes quietly.\r\n", fname(back->keyword));
	      send_to_room(buf, -EXIT(ch, door)->to_room);
	    } else
	      send_to_room("The door closes quietly.\r\n", -EXIT(ch, door)->to_room);
	  }
    }
  }
}


int has_key(CharData * ch, int key, ObjData **keyptr)
{
  ObjData *o;

  for (o = ch->carrying; o; o = o->next_content)
    if (GET_OBJ_VNUM(o) == key){
      *keyptr = o;
      return 1;
    }

  if (ch->equipment[WEAR_HOLD]){
    if (GET_OBJ_VNUM(ch->equipment[WEAR_HOLD]) == key){
      *keyptr = ch->equipment[WEAR_HOLD];
      return 1;
    }
  }

  return 0;
}

/* Mortius : Added IS_CONTAINER macro so we can lock traps */

ACMD(do_lock)
{
  int door, other_room;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  struct room_direction_data *back;
  ObjData *obj, *key;
  CharData *victim;


  two_arguments(argument, type, dir);

  if (!*type)
    send_to_char("Lock what?\r\n", ch);
  else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
			ch, &victim, &obj))
    /* this is an object */

    if (!IS_CONTAINER(obj))
      send_to_char("That's not a container.\r\n", ch);
    else if (!IS_SET(GET_OBJ_VAL(obj, 1), CONT_CLOSED))
      send_to_char("Maybe you should close it first...\r\n", ch);
    else if (GET_OBJ_VAL(obj, 2) < 0)
      send_to_char("That thing can't be locked.\r\n", ch);
    else if (!has_key(ch, GET_OBJ_VAL(obj, 2), &key))
      send_to_char("You don't seem to have the proper key.\r\n", ch);
    else if (IS_SET(GET_OBJ_VAL(obj, 1), CONT_LOCKED))
      send_to_char("It is locked already.\r\n", ch);
    else {
      SET_BIT(GET_OBJ_VAL(obj, 1), CONT_LOCKED);
      send_to_char("*Cluck*\r\n", ch);
      act("$n locks $p - 'cluck', it says.", FALSE, ch, obj, 0, TO_ROOM);
    }
  else if ((door = find_door(ch, type, dir)) >= 0)
  {
    /* a door, perhaps */
    if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR))
      send_to_char("That's absurd.\r\n", ch);
    else if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
      send_to_char("You have to close it first, I'm afraid.\r\n", ch);
    else if (EXIT(ch, door)->key < 0)
      send_to_char("There does not seem to be a keyhole.\r\n", ch);
    else if (!has_key(ch, EXIT(ch, door)->key, &key) && GET_LEVEL(ch) < LVL_SAINT)
      send_to_char("You don't have the proper key.\r\n", ch);
    else if (IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED))
      send_to_char("It's already locked!\r\n", ch);
    else {
      SET_BIT(EXIT(ch, door)->exit_info, EX_LOCKED);
      if (EXIT(ch, door)->keyword)
	act("$n locks the $F.", 0, ch, 0, EXIT(ch, door)->keyword,
	    TO_ROOM);
      else
	act("$n locks the door.", FALSE, ch, 0, 0, TO_ROOM);
      send_to_char("*Click*\r\n", ch);
      /* now for locking the other side, too */
      if ((other_room = EXIT(ch, door)->to_room) != NOWHERE)
	if ((back = world[other_room].dir_option[rev_dir[door]]))
	  if (back->to_room == ch->in_room)
	    SET_BIT(back->exit_info, EX_LOCKED);
    }
  }
}

/* Mortius : Added IS_CONTAINER macro so we can lock traps */

ACMD(do_unlock)
{
  int door, other_room;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  struct room_direction_data *back;
  ObjData *obj;
  ObjData *key = NULL;
  CharData *victim;


  two_arguments(argument, type, dir);

  if (!*type)
    send_to_char("Unlock what?\r\n", ch);
  else if ((door = find_door(ch, type, dir)) >= 0)
    /* it is a door */

    if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR))
      send_to_char("That's absurd.\r\n", ch);
    else if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
      send_to_char("Heck.. it ain't even closed!\r\n", ch);
    else if (EXIT(ch, door)->key < 0)
      send_to_char("You can't seem to spot any keyholes.\r\n", ch);
    else if (!has_key(ch, EXIT(ch, door)->key, &key) && GET_LEVEL(ch) < LVL_SAINT)
      send_to_char("You do not have the proper key for that.\r\n", ch);
    else if (!IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED))
      send_to_char("It's already unlocked, it seems.\r\n", ch);
    else {
      /* Digger - Timed keys ... Muhahahahahahaaaaaaa! */
      if( key != NULL ){
          SET_BIT_AR(key->obj_flags.extra_flags, ITEM_TIMED);
          GET_OBJ_TIMER(key)=number(24,48);
      }
      REMOVE_BIT(EXIT(ch, door)->exit_info, EX_LOCKED);
      if (EXIT(ch, door)->keyword)
	act("$n unlocks the $F.", 0, ch, 0, EXIT(ch, door)->keyword,
	    TO_ROOM);
      else
	act("$n unlocks the door.", FALSE, ch, 0, 0, TO_ROOM);
      send_to_char("*click*\r\n", ch);
      /* now for unlocking the other side, too */
      if ((other_room = EXIT(ch, door)->to_room) != NOWHERE)
	if ((back = world[other_room].dir_option[rev_dir[door]]))
	  if (back->to_room == ch->in_room)
	    REMOVE_BIT(back->exit_info, EX_LOCKED);

    }
  else if (generic_find(argument, FIND_OBJ_ROOM | FIND_OBJ_INV,
			ch, &victim, &obj))
  {
    /* this is an object */
    if (!IS_CONTAINER(obj))
      send_to_char("That's not a container.\r\n", ch);
    else if (!IS_SET(GET_OBJ_VAL(obj, 1), CONT_CLOSED))
      send_to_char("Silly - it ain't even closed!\r\n", ch);
    else if (GET_OBJ_VAL(obj, 2) < 0)
      send_to_char("Odd - you can't seem to find a keyhole.\r\n", ch);
    else if (!has_key(ch, GET_OBJ_VAL(obj, 2), &key))
      send_to_char("You don't seem to have the proper key.\r\n", ch);
    else if (!IS_SET(GET_OBJ_VAL(obj, 1), CONT_LOCKED))
      send_to_char("Oh.. it wasn't locked, after all.\r\n", ch);
    else {
		/* Craklyn - Timed keys to objects too.  ... Muhahahahahahaaaaaaa! */
      if( key != NULL ){
          SET_BIT_AR(key->obj_flags.extra_flags, ITEM_TIMED);
          GET_OBJ_TIMER(key)=number(24,48);
      }
      REMOVE_BIT(GET_OBJ_VAL(obj, 1), CONT_LOCKED);
      send_to_char("*Click*\r\n", ch);
      act("$n unlocks $p.", FALSE, ch, obj, 0, TO_ROOM);
    }
  }
}



/* Mortius : Added IS_CONTAINER macro so we can pick a trap */

ACMD(do_pick)
{
  byte percent;
  int door, other_room;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  struct room_direction_data *back;
  ObjData *obj;
  CharData *v;

  two_arguments(argument, type, dir);

  percent = number(1, 101);	/* 101% is a complete failure */

  if (!*type)
    send_to_char("Pick what?\r\n", ch);
  else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &v, &obj)) {
    /* this is an object */
    if (!IS_CONTAINER(obj))
      send_to_char("That's not a container.\r\n", ch);
    else if (!IS_SET(GET_OBJ_VAL(obj, 1), CONT_CLOSED))
      send_to_char("Silly - it isn't even closed!\r\n", ch);
    else if (GET_OBJ_VAL(obj, 2) < 0)
      send_to_char("Odd - you can't seem to find a keyhole.\r\n", ch);
    else if (!IS_SET(GET_OBJ_VAL(obj, 1), CONT_LOCKED))
      send_to_char("Oho! This thing is NOT locked!\r\n", ch);
    else if (IS_SET(GET_OBJ_VAL(obj, 1), CONT_PICKPROOF))
      send_to_char("It resists your attempts at picking it.\r\n", ch);
    else if (percent > GET_SKILL(ch, SKILL_PICK_LOCK))
      send_to_char("You failed to pick the lock.\r\n", ch);
    else {
      REMOVE_BIT(GET_OBJ_VAL(obj, 1), CONT_LOCKED);
      send_to_char("*Click*\r\n", ch);
      act("$n fiddles with $p.", FALSE, ch, obj, 0, TO_ROOM);
    }
  }
  else if ((door = find_door(ch, type, dir)) >= 0)
  {
    if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR))
      send_to_char("That's absurd.\r\n", ch);
    else if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
      send_to_char("You realize that the door is already open.\r\n", ch);
    else if (EXIT(ch, door)->key < 0)
      send_to_char("You can't seem to spot any lock to pick.\r\n", ch);
    else if (!IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED))
      send_to_char("Oh.. it wasn't locked at all.\r\n", ch);
    else if (IS_SET(EXIT(ch, door)->exit_info, EX_PICKPROOF))
      send_to_char("You seem to be unable to pick this lock.\r\n", ch);
    else if (percent > GET_SKILL(ch, SKILL_PICK_LOCK))
      send_to_char("You failed to pick the lock.\r\n", ch);
    else if (percent > GET_SKILL(ch, SKILL_PICK_LOCK)) /* New door pick level XXXXX */
      send_to_char("You lack the skill to pick this lock.\r\n", ch);
    else {
      REMOVE_BIT(EXIT(ch, door)->exit_info, EX_LOCKED);
      if (EXIT(ch, door)->keyword)
	act("$n skillfully picks the lock of the $F.", 0, ch, 0,
	    EXIT(ch, door)->keyword, TO_ROOM);
      else
	act("$n picks the lock of the door.", TRUE, ch, 0, 0, TO_ROOM);
      send_to_char("The lock quickly yields to your skills.\r\n", ch);
      /* now for unlocking the other side, too */
      if ((other_room = EXIT(ch, door)->to_room) != NOWHERE)
	if ((back = world[other_room].dir_option[rev_dir[door]]))
	  if (back->to_room == ch->in_room)
	    REMOVE_BIT(back->exit_info, EX_LOCKED);
    }
  }
}


// do_enter
// 
ACMD(do_enter)
{
  int door;
  ObjData *obj = NULL;

  one_argument(argument, buf);

  /* Mortius : Added this so ports don't need to be added in specials */
  if (*buf) { 
     if ((obj = get_obj_in_list_vis(ch, buf, world[ch->in_room].contents))) {
      if (CAN_SEE_OBJ(ch, obj))
        if (GET_OBJ_TYPE(obj) == ITEM_PORTAL)
            enter_portal(ch, obj);
            return;
    }
  }

  if( *buf ) // an argument was supplied, search for door keyword
  {
    for( door = 0; door < NUM_OF_DIRS; door++ )
      if( EXIT(ch, door)) 
        if( EXIT(ch, door)->keyword )
          if( !str_cmp(EXIT(ch, door)->keyword, buf) )
          {
            perform_move(ch, door, 0);
            return;
          }

    sendChar( ch, "There is no %s here.\r\n", buf );
  }

  else if (IS_SET_AR(ROOM_FLAGS(ch->in_room), ROOM_INDOORS))
    sendChar( ch, "You are already indoors.\r\n" );

  else //try to locate an entrance
  {
    for (door = 0; door < NUM_OF_DIRS; door++)
      if (EXIT(ch, door))
        if (EXIT(ch, door)->to_room != NOWHERE)
          if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) &&
              IS_SET_AR(ROOM_FLAGS(EXIT(ch, door)->to_room), ROOM_INDOORS))
          {
            perform_move(ch, door, 0);
            return;
          }

    sendChar( ch, "You can't seem to find anything to enter.\r\n" );
  }
}


ACMD(do_leave)
{
  int door;

  if (!IS_SET_AR(ROOM_FLAGS(ch->in_room), ROOM_INDOORS))
    send_to_char("You are outside.. where do you want to go?\r\n", ch);
  else {
    for (door = 0; door < NUM_OF_DIRS; door++)
      if (EXIT(ch, door))
	if (EXIT(ch, door)->to_room != NOWHERE)
	  if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) &&
              !IS_SET_AR(ROOM_FLAGS(EXIT(ch, door)->to_room), ROOM_INDOORS)) {
	    perform_move(ch, door, 0);
	    return;
	  }
    send_to_char("I see no obvious exits to the outside.\r\n", ch);
  }
}


ACMD(do_stand)
{

	// They can't sit when they're on a mount.  If they're sitting, it's due
	// to something having forced them to sit.  Now, they can unsit. Craklyn

    /* 
	if (!IS_NPC(ch) && ch->mount) {
        sendChar(ch, "You cannot do this while mounted.\r\n");
        return;
    }
	*/

    switch (GET_POS(ch)) {
        case POS_MORTALLYW:
            act("You are in pretty bad shape, and unable to do anything!", FALSE, ch, 0, 0, TO_CHAR);
            act("$n thrashes violently.", TRUE, ch, 0, 0, TO_ROOM);
            break;
        case POS_INCAP:
            act("You are in pretty bad shape, and unable to do anything!", FALSE, ch, 0, 0, TO_CHAR);
            act("$n twitches violently.", TRUE, ch, 0, 0, TO_ROOM);
            break;
        case POS_STANDING:
            act("You are already standing.", FALSE, ch, 0, 0, TO_CHAR);
            break;
        case POS_SITTING:
            act("You stand up.", FALSE, ch, 0, 0, TO_CHAR);
            act("$n clambers to $s feet.", TRUE, ch, 0, 0, TO_ROOM);
            GET_POS(ch) = POS_STANDING;
            break;
        case POS_RESTING:
            act("You stop resting, and stand up.", FALSE, ch, 0, 0, TO_CHAR);
            act("$n stops resting, and clambers to $s feet.", TRUE, ch, 0, 0, TO_ROOM);
            GET_POS(ch) = POS_STANDING;
            break;
        case POS_SLEEPING:
            act("You have to wake up first!", FALSE, ch, 0, 0, TO_CHAR);
            break;
        case POS_FIGHTING:
            act("Do you not consider fighting as standing?", FALSE, ch, 0, 0, TO_CHAR);
            break;
        default:
            act("You stop floating around, and put your feet on the ground.", FALSE, ch, 0, 0, TO_CHAR);
            act("$n stops floating around, and puts $s feet on the ground.", TRUE, ch, 0, 0, TO_ROOM);
            GET_POS(ch) = POS_STANDING;
            break;
    }
    if( FIGHTING(ch) )
        GET_POS(ch) = POS_FIGHTING;
}


ACMD(do_sit)
{
    if (!IS_NPC(ch) && ch->mount) {
        sendChar(ch, "You cannot do this while mounted.\r\n");
        return;
    }

  switch (GET_POS(ch)) {
  case POS_STANDING:
    act("You sit down.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n sits down.", FALSE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SITTING;
    break;
  case POS_SITTING:
    send_to_char("You're sitting already.\r\n", ch);
    break;
  case POS_RESTING:
    act("You stop resting, and sit up.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops resting.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SITTING;
    break;
  case POS_SLEEPING:
    act("You have to wake up first.", FALSE, ch, 0, 0, TO_CHAR);
    break;
  case POS_FIGHTING:
    act("Sit down while fighting?  Are you MAD?", FALSE, ch, 0, 0, TO_CHAR);
    break;
  default:
    act("You stop floating around, and sit down.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops floating around, and sits down.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SITTING;
    break;
  }
}


ACMD(do_rest)
{
    if (!IS_NPC(ch) && ch->mount) {
        sendChar(ch, "You cannot do this while mounted.\r\n");
        return;
    }

  switch (GET_POS(ch)) {
  case POS_STANDING:
    act("You sit down and rest your tired bones.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n sits down and rests.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_RESTING;
    break;
  case POS_SITTING:
    act("You rest your tired bones.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n rests.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_RESTING;
    break;
  case POS_RESTING:
    act("You are already resting.", FALSE, ch, 0, 0, TO_CHAR);
    break;
  case POS_SLEEPING:
    act("You have to wake up first.", FALSE, ch, 0, 0, TO_CHAR);
    break;
  case POS_FIGHTING:
    act("Rest while fighting?  Are you MAD?", FALSE, ch, 0, 0, TO_CHAR);
    break;
  default:
    act("You stop floating around, and stop to rest your tired bones.",
	FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops floating around, and rests.", FALSE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SITTING;
    break;
  }
}


ACMD(do_meditate)
{
    if( GET_CLASS(ch) != CLASS_SHOU_LIN ){
        sendChar(ch, "You cannot bend your body that way.\r\n");
        return;
    }

    if (!IS_NPC(ch) && ch->mount) {
        sendChar(ch, "You cannot do this while mounted.\r\n");
        return;
    }

  switch (GET_POS(ch)) {
      case POS_STANDING:
          act("You sit in the lotus position and begin chanting your mantra.", FALSE, ch, 0, 0, TO_CHAR);
          act("$n sits in the lotus position and begins chanting.", TRUE, ch, 0, 0, TO_ROOM);
          GET_POS(ch) = POS_MEDITATING;
          break;
      case POS_RESTING:
      case POS_SITTING:
          act("You get into the lotus position and begin chanting your mantra.", FALSE, ch, 0, 0, TO_CHAR);
          act("$n gets into the lotus position and begins chanting.", TRUE, ch, 0, 0, TO_ROOM);
          GET_POS(ch) = POS_MEDITATING;
          break;
      case POS_SLEEPING:
          act("You have to wake up first.", FALSE, ch, 0, 0, TO_CHAR);
          break;
      case POS_FIGHTING:
          act("Meditation takes peace of mind, seek the void instead.", FALSE, ch, 0, 0, TO_CHAR);
          break;
      default:
          break;
  }

}


ACMD(do_sleep)
{
    if (!IS_NPC(ch) && ch->mount) {
        sendChar(ch, "You cannot do this while mounted.\r\n");
        return;
    }

  switch (GET_POS(ch)) {
  case POS_STANDING:
  case POS_SITTING:
  case POS_RESTING:
    send_to_char("You go to sleep.\r\n", ch);
    act("$n lies down and falls asleep.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SLEEPING;
    break;
  case POS_SLEEPING:
    send_to_char("You are already sound asleep.\r\n", ch);
    break;
  case POS_FIGHTING:
    send_to_char("Sleep while fighting?  Are you MAD?\r\n", ch);
    break;
  default:
    act("You stop floating around, and lie down to sleep.",
	FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops floating around, and lie down to sleep.",
	TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SLEEPING;
    break;
  }
}


ACMD(do_wake)
{
  CharData *vict;
  int self = 0;

  one_argument(argument, arg);
  if (*arg) {
    if (GET_POS(ch) == POS_SLEEPING)
      send_to_char("You can't wake people up if you're asleep yourself!\r\n", ch);
    else if ((vict = get_char_room_vis(ch, arg)) == NULL)
      sendChar( ch, CONFIG_NOPERSON );
    else if (vict == ch)
      self = 1;
    else if (GET_POS(vict) > POS_SLEEPING)
      act("$E is already awake.", FALSE, ch, 0, vict, TO_CHAR);
    else if (IS_AFFECTED(vict, AFF_SLEEP))
      act("You can't wake $M up!", FALSE, ch, 0, vict, TO_CHAR);
    else {
      act("You wake $M up.", FALSE, ch, 0, vict, TO_CHAR);
      act("You are awakened by $n.", FALSE, ch, 0, vict, TO_VICT | TO_SLEEP);
      GET_POS(vict) = POS_SITTING;
    }
    if (!self)
      return;
  }
  if (IS_AFFECTED(ch, AFF_SLEEP))
    send_to_char("You can't wake up!\r\n", ch);
  else if (GET_POS(ch) > POS_SLEEPING)
    send_to_char("You are already awake...\r\n", ch);
  else if (GET_POS(ch) == POS_INCAP || GET_POS(ch) == POS_MORTALLYW)
            send_to_char("You are in pretty bad shape, and unable to awaken!\r\n", ch);
  else {
    send_to_char("You awaken, and sit up.\r\n", ch);
    act("$n awakens.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_RESTING;
  }
}


ACMD(do_follow)
{
  CharData *leader;

  one_argument(argument, buf);

  if( *buf )
  {
    if( !(leader = get_char_room_vis(ch, buf)) )
    {
      sendChar( ch, CONFIG_NOPERSON );
      return;
    }
  }

  else
  {
    sendChar( ch, "Whom do you wish to follow?\r\n" );
    return;
  }

  if( ch->master == leader )
  {
    act("You are already following $M.", FALSE, ch, 0, leader, TO_CHAR);
    return;
  }

  if( IS_AFFECTED( ch, AFF_CHARM) && (ch->master) )
  {
    act("But you only feel like following $N!", FALSE, ch, 0, ch->master, TO_CHAR);
    return;
  }

  if( leader == ch )
  {
    if( !ch->master )
    {
      sendChar( ch, "You are already following yourself.\r\n" );
      return;
    }
    stop_follower(ch);
  }

  else
  {
    if( circle_follow(ch, leader) )
    {
      act("Sorry, but following in loops is not allowed.", FALSE, ch, 0, 0, TO_CHAR);
      return;
    }

    if( ch->master )
      stop_follower(ch);
    REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_GROUP);
    add_follower(ch, leader);
  }
}


ACMD(do_mount)
{
  CharData *mount;

  one_argument(argument, buf);

  if( *buf ) {
    if( !(mount = get_char_room_vis(ch, buf)) ) {
      sendChar( ch, CONFIG_NOPERSON );
      return;
    }
  }
  else {
    sendChar( ch, "What did you you intend on riding ?\r\n" );
    return;
  }

  /* For now, only Immortals, SKs and DKs can mount. */
  if (GET_LEVEL(ch) < LVL_IMMORT && GET_CLASS(ch) != CLASS_SOLAMNIC_KNIGHT &&
              GET_CLASS(ch) != CLASS_DEATH_KNIGHT) {
      sendChar(ch, "You are unfamiliar with equestrian tradition.\r\n");
      return;
  }

  if (GET_SKILL(ch, SPELL_CALL_STEED) == 0) {
	  send_to_char("You are unfamiliar with equestrian tradition.\r\n", ch);
	  return;
  }
    
  if(!IS_SET_AR( MOB_FLAGS( mount ), MOB_MOUNT)) {
    sendChar( ch, "You cannot ride that.\r\n" );
    return;
  }

  if(mount->rider != NULL) {
    sendChar( ch, "Mount rustlin' is a hangin offense.\r\n" );
    return;
  }

  // Added this check.  It's actualy not been in for the years we've had mount
  // and nobody noticed you could mount infinite steeds.  Craklyn 9/23/05
  if( ch->mount != NULL)  
  {
	  sendChar( ch, "You shouldn't overextend yourself..\r\n" );
	  return;
  }
  
  ch->mount = mount;
  mount->rider = ch;

  act( "You mount up on $N.", FALSE, ch, 0, mount, TO_CHAR );
  act( "$n mounts $N.", FALSE, ch, 0, mount, TO_ROOM );

  /* From now, you will be considered to be standing */
  GET_POS(ch) = POS_STANDING;
}

void awkward_dismount(CharData *ch)
{
    CharData *tch, *next_tch;

    if (!ch->mount) return;

    ch->mount->rider = NULL;

    act("You are thrown from $N!", FALSE, ch, 0, ch->mount, TO_CHAR);
    act("$n is thrown from $N!", FALSE, ch, 0, ch->mount, TO_ROOM);

    OFF_BALANCE(ch) = 1;
    send_to_char("You find yourself off balance and vulnerable!\r\n",
            ch);
    act("$n is off balance and vulnerable!\r\n", FALSE,	ch, 0, ch, TO_ROOM);
    
    for( tch = world[ch->in_room].people; tch; tch = next_tch ){
        next_tch = tch->next_in_room;
        
        if(FIGHTING(tch) == ch && GET_ASPECT(tch) == ASPECT_MONKEY
                && skillSuccess(tch, SKILL_ART_MONKEY))
            do_knock(tch, ch->player.name, 0, 0);
    }
    
    /* It would be nice here to do damage, too */

    GET_POS(ch) = POS_SITTING;
    WAIT_STATE(ch, SET_STUN(4));

    ch->mount = NULL;
}

ACMD(do_dismount)
{
  if( ch->mount == NULL )
  {
    sendChar( ch, "You are not riding anything.\r\n" );
    return;
  }

  ch->mount->rider = NULL;
  ch->mount = NULL;
  sendChar( ch, "You hop down from your mount.\r\n" );

}
