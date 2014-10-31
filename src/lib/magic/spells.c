/*
 * newmagic.c Written by Fred Merkel Part of JediMUD Copyright (C) 1993
 * Trustees of The Johns Hopkins Unversity All Rights Reserved. Based on
 * DikuMUD, Copyright (C) 1990, 1991.
 */

#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/objsave.h"
#include "util/utils.h"
#include "general/comm.h"
#include "general/class.h"
#include "magic/spells.h"
#include "general/handler.h"
#include "util/weather.h"
#include "olc/oedit.h"
#include "specials/combspec.h"
#include "magic/skills.h"
#include "actions/fight.h"
#include "scripts/dg_scripts.h"
#include "magic/sing.h"

/* 128 BIT */

extern char weapon_verbs[];
extern int *max_ac_applys;

void clearMemory(CharData * ch);

void weight_change_object(ObjData * obj, int weight);
int mag_savingthrow(CharData * ch, int type);
int magic_savingthrow(CharData * caster, CharData * victim, int type);
int read_max_hit(ObjData *corpse);

/*
 * Special spells appear below.
 */

ASPELL(spell_create_water)
{

  void name_to_drinkcon(ObjData * obj, int type);
  void name_from_drinkcon(ObjData * obj);

  if (ch == NULL || obj == NULL)
    return;

  if (GET_OBJ_TYPE(obj) == ITEM_DRINKCON) {
    if ((GET_OBJ_VAL(obj, 2) != LIQ_WATER) && (GET_OBJ_VAL(obj, 1) != 0)) {
      name_from_drinkcon(obj);
      GET_OBJ_VAL(obj, 2) = LIQ_SLIME;
      name_to_drinkcon(obj, LIQ_SLIME);
    } else {
    GET_OBJ_VAL(obj, 2) = LIQ_WATER;
    GET_OBJ_VAL(obj, 1) = GET_OBJ_VAL(obj, 0);
/*    weight_change_object(obj, GET_OBJ_VAL(obj, 1)); */
    name_from_drinkcon(obj);
    name_to_drinkcon(obj, LIQ_WATER);
    act("$p is filled.", FALSE, ch, obj, 0, TO_CHAR);
    }
  }
}


ASPELL(spell_recall)
{
  sh_int target_room;
// Pre-Set the destination room
  if( ch != NULL )
  {
    target_room = getStartRoom( ch );
  }
// Check for scroll
  if(( castobj != NULL ) &&
     ( casttype == CAST_SCROLL ) &&
     ( GET_OBJ_VAL( castobj, 3 ) > 0 )) {
        target_room = real_room( GET_OBJ_VAL( castobj, 3 ));
  }
  // Pre-set the target room when people are casting;
  if ( castobj == NULL ) target_room = getStartRoom( ch );
  if (victim == NULL || IS_NPC(victim)) return;

  if (ZONE_FLAGGED(world[ch->in_room].zone, ZONE_NORECALL)) {
      send_to_char("The magic unravels before you can complete the spell!\r\n", ch);
     return;
  }

  if ( castobj && IS_SET_AR( ROOM_FLAGS( ch->in_room ), ROOM_NORECALL )) {
	sendChar(ch, "Your %s fizzles!\r\n", castobj->short_description);
	return;
  }
  if ( (ch != victim) &&
       !PRF_FLAGGED(victim, PRF_NORECALL) &&
       !IS_SET_AR(PLR_FLAGS(victim), PLR_HUNTED) &&
       !IS_SET_AR(PLR_FLAGS(victim), PLR_GRIEF))
  {
	sendChar(ch, "%s doesn't seem to want to go!\r\n", GET_NAME(victim));
	return;
  }

  // Thief or Hunted Check for peaceroom recalls
  if ((IS_SET_AR(PLR_FLAGS(victim), PLR_HUNTED) ||
      IS_SET_AR(PLR_FLAGS(victim), PLR_THIEF)) &&
      (IS_SET_AR( ROOM_FLAGS( target_room ), ROOM_PEACEFUL))) {
          send_to_char("The magic unravels before you can complete the spell!\r\n", ch);
          return;
        }

  if(( castobj != NULL ) &&
     ( casttype == CAST_SCROLL ) &&
     ( GET_OBJ_VAL( castobj, 3 ) > 0 ))
  {
    /* Let's check for a clan scroll. */
    int secondary_room = real_room( GET_OBJ_VAL( castobj, 3 ));
    if( !IS_SET_AR( ROOM_FLAGS( secondary_room ), ROOM_CLAN ) )
      target_room = secondary_room;
    else
    { /* Clan room is destination, check for membership */
      if( world[secondary_room].clan_id == GET_CLAN(victim) &&
          !IS_SET_AR(PLR_FLAGS(victim), PLR_KILLER))
        target_room = secondary_room;
    }
  }
   if(castobj == NULL && GET_RECALL(ch) != 0 && (IS_SET_AR( ROOM_FLAGS( real_room(GET_RECALL(ch))), ROOM_FAMILIAR))){
    target_room = real_room(GET_RECALL(ch));
  }

  act("$n phases out of existence.", TRUE, victim, 0, 0, TO_ROOM);
  char_from_room(victim);
  if( target_room != victim->in_room )
    stop_fighting(victim);
   if (victim->mount) {
       char_from_room(victim->mount);
       if (target_room != victim->in_room)
           stop_fighting(victim->mount);
       char_to_room(victim->mount, target_room);
   }
  char_to_room(victim, target_room);
  act("$n phases into existence.", TRUE, victim, 0, 0, TO_ROOM);
  look_at_room(victim, 0);
  entry_memory_mtrigger(victim);
  greet_mtrigger(victim, -1);
  greet_memory_mtrigger(victim);
}
// Spell Nexus Swap positions with target

ASPELL(spell_nexus)
{
    int to_room = 0;
    int from_room = 0;
    // make sure caster exists, victim exists, make sure target is a player, and that the player is not an immortal
    if( ch == NULL || victim == NULL || IS_NPC(victim) || (GET_LEVEL(victim) - 5) >= GET_LEVEL(ch)  ) return;
    // Where's the target at?
    to_room = victim->in_room;
    //Where's the caster at?
    from_room = ch->in_room;
    // Don't go to death rooms

    if( IS_SET_AR( world[to_room].room_flags, ROOM_DEATH ))
    {
      act("The raging energies contained in the nexus implode with a pop.", FALSE, ch, NULL, victim, TO_CHAR );
      return;
    }
    // Don't go to norelocate areas
    if (ZONE_FLAGGED(world[victim->in_room].zone, ZONE_NORELOCATE)) {
      send_to_char("The raging energies contained in the nexus implode with a pop.\r\n", ch);
      return;
   }
   if (IS_SET_AR( world[to_room].room_flags, ROOM_NORELOCATE ))
   {
    send_to_char("The raging energies contained in the nexus implode with a pop.\r\n", ch);
    return;
    }
   // Don't allow hunteds in peace rooms
   if ( IS_SET_AR(PLR_FLAGS(ch), PLR_HUNTED) && IS_SET_AR( world[from_room].room_flags, ROOM_PEACEFUL))
   {
	send_to_char("Hunted players are not permitted in peace rooms.\r\n", ch);
	return;
    }
   if ( IS_SET_AR(PLR_FLAGS(victim), PLR_HUNTED) && IS_SET_AR( world[to_room].room_flags, ROOM_PEACEFUL))
   {
	send_to_char("Hunted players are not permitted in peace rooms.\r\n", ch);
	return;
   }

   if (victim->mount || victim->rider) {
       send_to_char("You cannot displace both mount and rider!\r\n", ch);
       return;
   }

   if (!PRF_FLAGGED(victim, PRF_SUMMONABLE) || !victim->desc) {
     send_to_char("Their mental powers prevent you from locking on to their location!\r\n", ch);
     return;
   }
   if ( ch == victim ) {
    act("You feel something go terribly wrong as the nexus begins to form INSIDE your body!", FALSE, ch, NULL, NULL, TO_CHAR);
    act("$n screams in agony as a rift in time and space consumes their body!", TRUE, ch, NULL, NULL, TO_ROOM);
    raw_kill(ch, NULL);

    }
    act("A gaping maelstrom of raging energy pulls you in!", FALSE, ch, NULL, NULL, TO_CHAR);
    act("A gaping maelstrom of raging energy pulls you in!", FALSE, victim, NULL, NULL, TO_CHAR);
    act("$n is sucked into a raging vortex of energy.", TRUE, ch, NULL, NULL, TO_ROOM);
    char_from_room(ch);
    act("$n is sucked into a raging vortex of energy.", TRUE, victim, NULL, NULL, TO_ROOM);
    char_from_room(victim);
    char_to_room(ch, to_room);
    act("$n is violently ejected from the energy vortex.", TRUE, ch, NULL, NULL, TO_ROOM);
    char_to_room(victim, from_room);
    act("$n is violently ejected from the energy vortex.", TRUE, victim, NULL, NULL, TO_ROOM);
    look_at_room(ch, 0);
    look_at_room(victim, 0);
    entry_memory_mtrigger(ch);
    entry_memory_mtrigger(victim);
    greet_mtrigger(ch, -1);
    greet_mtrigger(victim, -1);
    greet_memory_mtrigger(ch);
    greet_memory_mtrigger(victim);
}

ASPELL(spell_relocate)
{
   int to_room = 0;

   if( ch == NULL || victim == NULL || IS_NPC(victim) || GET_LEVEL(victim) > MAX_MORTAL ) return;

   to_room = victim->in_room;

   if (ch->mount) {
       send_to_char("Perhaps you should dismount first.\r\n", ch);
   }

   if (ZONE_FLAGGED(world[victim->in_room].zone, ZONE_NORELOCATE)) {
      send_to_char("The magic unravels before you can complete the spell!\r\n", ch);
      return;
   }
   if (IS_SET_AR( world[to_room].room_flags, ROOM_NORELOCATE ))
   {
    send_to_char("The magic unravels before you can complete the spell!\r\n", ch);
    return;
    }
   if( IS_SET_AR( world[to_room].room_flags, ROOM_CLAN | ROOM_PRIVATE | ROOM_DEATH )){
       act("You are unable to lock in on $N's position.", FALSE, ch, NULL, victim, TO_CHAR );
       return;
   }

   if ( IS_SET_AR(PLR_FLAGS(ch), PLR_HUNTED) && IS_SET_AR( world[to_room].room_flags, ROOM_PEACEFUL))
   {
	send_to_char("Hunted players are not permitted in peace rooms.\r\n", ch);
	return;
    }

   if( ch == victim ){
       act("You temporarily fade from existance.", FALSE, ch, NULL, NULL, TO_CHAR);
       act("$n fades in and out of existance.", TRUE, ch, NULL, NULL, TO_ROOM);
       return;
   }

   act("\r\n$n is pulled through a rift in time and space.", TRUE, ch, NULL, NULL, TO_ROOM);
   act("You are pulled through time and space at incredible speed!\r\n", FALSE, ch, NULL, NULL, TO_CHAR);
   char_from_room(ch);

   char_to_room(ch,victim->in_room);
   act("\r\n$n emerges from a rift in time and space.", TRUE, ch, NULL, NULL, TO_ROOM);
   act("Stopping suddenly, you fall into...\r\n", FALSE, ch, NULL, NULL, TO_CHAR);

   look_at_room(ch, 0);
   entry_memory_mtrigger(ch);
   greet_mtrigger(ch, -1);
   greet_memory_mtrigger(ch);
}


ASPELL(spell_teleport)
{
    int to_room;

    if( victim == NULL ) return;

    
//added check for mob aggro //
    if(MOB_FLAGGED(victim, MOB_AGGRESSIVE) || MOB_FLAGGED(victim, MOB_PREDATOR) || MOB_FLAGGED(victim, MOB_AGGR_GOOD)
       || MOB_FLAGGED(victim, MOB_AGGR_EVIL) || MOB_FLAGGED(victim, MOB_AGGR_NEUTRAL)|| MOB_FLAGGED(victim, MOB_SUPERAGG) ||
          (GET_LEVEL(victim) >= LVL_IMMORT && GET_LEVEL(ch) < LVL_IMMORT))
    {
     act("$n starts to fade out of existence, but screams in rage, and claws its way back!", FALSE, victim, 0, 0, TO_ROOM);
     return;
     }

    do {
        to_room = number(0, top_of_world);
    } while (IS_SET_AR(world[to_room].room_flags, ROOM_CLAN | ROOM_PRIVATE | ROOM_DEATH));

    act("$n slowly fades out of existence and is gone.", FALSE, victim, 0, 0, TO_ROOM);
    char_from_room(victim);
    char_to_room(victim, to_room);
    act("$n slowly fades into existence.", FALSE, victim, 0, 0, TO_ROOM);
    look_at_room(victim, 0);
   entry_memory_mtrigger(victim);
   greet_mtrigger(victim, -1);
   greet_memory_mtrigger(victim);
}

ASPELL(spell_summon)
{
  int room = -1;

  if (ch == NULL || victim == NULL) return;

  room = victim->in_room;

  if (ZONE_FLAGGED(world[ch->in_room].zone, ZONE_NOSUMMON)) {
      send_to_char("The magic unravels before you can complete the spell!\r\n", ch);
     return;
  }

  if (ZONE_FLAGGED(world[ch->in_room].zone, ZONE_NOMORTAL)) {
      send_to_char("The magic unravels before you can complete the spell!\r\n", ch);
      return;
  }

  if( (GET_LEVEL( victim ) > MIN(LVL_IMMORT - 1, level + 5 )) && !PLR_FLAGGED(victim, PLR_HUNTED) ) {
    sendChar( ch, "You failed.\r\n" );
    return;
  }

  if (IN_ARENA(ch) || ZONE_FLAGGED(world[ch->in_room].zone, ZONE_ARENA) ||
      ZONE_FLAGGED(world[ch->in_room].zone, ZONE_SLEEPTAG))
  {
    sendChar( ch, "You cannot summon into an arena!\r\n" );
    return;
  }

  if (!IS_NPC(victim) && PLR_FLAGGED(victim, PLR_JAILED))
  {
    sendChar( ch, "They have been magically imprisoned.\r\n" );
    return;
  }

  if( MOB_FLAGGED(victim, MOB_AGGRESSIVE))
  {
    act( "As the words escape your lips and $N travels\r\n"
         "through time and space towards you, you realize that $E is\r\n"
         "aggressive and might harm you, so you wisely send $M back.",
         FALSE, ch, 0, victim, TO_CHAR);
    return;
  }

  if( IS_NPC( victim ) && IS_SET_AR( MOB_FLAGS( victim ), MOB_NOSUMMON))
  {
    act("$N manages to avoid your feeble attempt of summoning.\r\n", FALSE, ch, 0, victim, TO_CHAR );
    return;
  }

  if( !IS_NPC(victim) &&
       IS_SET_AR(ROOM_FLAGS((ch)->in_room), ROOM_PEACEFUL) &&
       IS_SET_AR(PLR_FLAGS(victim), PLR_HUNTED))
  {
    sendChar( ch, "You may not summon a hunted player into a peace room!" );
    return;
  }

  if( victim->rider != NULL )
  {
    sendChar( ch, "You don't have the power to summon %s and %s rider.\r\n",
                  GET_NAME(victim), HSHR(victim));
    return;
  }

  if( victim->mount != NULL )
  {
    sendChar( ch, "You don't have the power to summon %s and %s mount.\r\n",
                  GET_NAME(victim), HSHR(victim));
    return;
  }

  // You can ALWAYS try and summon a hunted player.
  if( !IS_NPC(victim) &&
      (!PRF_FLAGGED(victim, PRF_SUMMONABLE) || !victim->desc || victim->in_room == 1) &&
      !PLR_FLAGGED(victim, PLR_HUNTED) &&
      !PLR_FLAGGED(victim, PLR_GRIEF))
  {
    sprintf( buf, "%s just tried to summon you to: %s.\r\n"
                  "%s failed because you have summon protection on.\r\n"
                  "Type NOSUMMON to allow other players to summon you.\r\n",
                  GET_NAME(ch), world[ch->in_room].name,
                  (ch->player.sex == SEX_MALE) ? "He" : "She");
    sendChar( victim, buf );

    sendChar( ch, "You failed because %s has summon protection on.\r\n",
                   GET_NAME(victim) );
    if(!IS_NPC(victim) && !victim->desc)
        mudlog(BRF, LVL_IMMORT, TRUE, "%s failed summoning the linkdead player %s to %s.",
              GET_NAME(ch), GET_NAME(victim), world[ch->in_room].name);
    else
        mudlog(NRM, LVL_IMMORT, TRUE, "%s failed summoning %s to %s.",
              GET_NAME(ch), GET_NAME(victim), world[ch->in_room].name);

    return;
  }

  if ( IS_NPC( victim ) && (magic_savingthrow( ch, victim, SAVING_SPELL )
       || (GET_MAX_HIT(victim)  > (GET_LEVEL(ch) * 50))))
  {
    sendChar( ch, "You failed.\r\n" );
    return;
  }

  if( IS_NPC(victim) &&
    (GET_LEVEL(ch) < LVL_IMMORT) &&
    (zone_table[world[victim->in_room].zone].reset_mode == 3))
  {
    sendChar(ch, "Mortals cannot summon monsters from there.\r\n");
    return;
  }

  if (GET_LEVEL(ch) < LVL_IMMORT)
      if (!IS_NPC(victim))
          if (IS_SET_AR(ROOM_FLAGS(ch->in_room), ROOM_ONE_PERSON)) {
              if (world[ch->in_room].people > 0) {
                  send_to_char("There is not enough room to summon anyone else here.\r\n", ch);
                  return;
              }
          }

      CharData *temp_ch, *temp2;
      ObjData *corpse;
      int charmed_corpse = affected_by_spell(ch, SPELL_CHARM_CORPSE);
      if(ch->in_room != victim->in_room &&
              (!IS_NPC(victim) || victim->master)) {
          for (temp2 = world[victim->in_room].people; temp2; temp2 = temp2->next_in_room)
          {
              temp_ch = temp2;
              if (temp_ch == ch || !IS_NPC(temp_ch))
                  continue;
              if(victim && FIGHTING(temp_ch) == victim)
                  hit(temp_ch, victim, TYPE_UNDEFINED);
          }
          if(!victim || IN_ROOM(victim) == -1) {
              sendChar(ch, "You target's lifeforce has been lost.\r\n");
              if(!charmed_corpse) {
                  corpse = world[room].contents;
                  obj_from_room(corpse);
                  obj_to_room(corpse, ch->in_room);
              }
              return;
          }
      }

  act("$n disappears suddenly.", TRUE, victim, 0, 0, TO_ROOM);

  char_from_room(victim);
  char_to_room(victim, ch->in_room);

  act("$n appears with a POP!", TRUE, victim, 0, 0, TO_ROOM);
  act("$n has summoned you!", FALSE, ch, 0, victim, TO_VICT);
  look_at_room(victim, 0);
   entry_memory_mtrigger(victim);
   greet_mtrigger(victim, -1);
   greet_memory_mtrigger(victim);

  if( ROOM_FLAGGED(IN_ROOM(victim), ROOM_FALL) && IS_NOT_FLYING(victim) )
    doFallRoom(victim);

}

ASPELL(spell_youthen)
{
    if (IS_NPC(ch))
	return;
    if (GET_AGE(ch) > 17)
    {
	ch->player.time.birth += (SECS_PER_MUD_YEAR);
	send_to_char("You feel the vigour of youth coursing through your veins!\r\n", ch);
    }
}

ASPELL(spell_knowledge)
{
    if (IS_NPC(ch))
	return;
    if (GET_PRACTICES(ch) < 100)
    {
	//GET_PRACTICES(ch) += 1;
	send_to_char("You feel more learned!\r\n", ch);
        send_to_char("Unfortunately, this has no effect!\r\n", ch);
	mudlog( BRF, LVL_GOD, TRUE, "SPECIAL: %s practices increased by one.", GET_NAME(ch));
    }
}
ASPELL(spell_recharge)
{
    if (IS_NPC(ch))
    return;

    if (GET_AGE(ch) > 17)
    {
    GET_MANA(ch) = GET_MAX_MANA(ch);
	send_to_char("Your veins burn as raw magic power infuses you.\r\n", ch);
	mudlog( BRF, LVL_GOD, TRUE,"SPECIAL: %s Mana recharged", GET_NAME(ch));
   }
}

ASPELL(spell_locate_object)
{
  ObjData *i;
  char name[MAX_INPUT_LENGTH];
  int j;

  if( obj == NULL )
  {
    mudlog(NRM, LVL_IMMORT, TRUE, "[CRASH?] locate failed, ask %s what command they just used.",
                                                            GET_NAME(ch) );
    sendChar( ch,"If you were attempting to intentionally crash the mud then you should fire off a detailed mudmail to Maestro about the command you just issued." );
    return;
  }

  strcpy(name, fname(obj->name));

  j = level >> 1;

  for( i = object_list; i && (j > 0); i = i->next )
  {
    if( !isname( name, i->name )) continue;

    /* Vex - March 1997
    ** Prevented artifacts and nolocate items being located at all.
    */
    if( IS_OBJ_STAT(i, ITEM_ARTIFACT) || IS_OBJ_STAT(i, ITEM_NOLOCATE)) continue;

    if( i->carried_by )
      sprintf(buf, "%s is being carried by %s.\n\r", i->short_description, PERS(i->carried_by, ch));

    else if(( i->in_room != NOWHERE ) &&
                !IS_SET_AR( world[i->in_room].room_flags, ROOM_PRIVATE) &&
                !IS_SET_AR( world[i->in_room].room_flags, ROOM_CLAN ))
      sprintf(buf, "%s is in %s.\n\r", i->short_description, world[i->in_room].name);

    else if (i->in_obj)
      sprintf(buf, "%s is in %s.\n\r", i->short_description, i->in_obj->short_description);

    else if (i->worn_by)
      sprintf(buf, "%s is being worn by %s.\n\r", i->short_description, PERS(i->worn_by, ch));

    else
      sprintf(buf, "%s's location is uncertain.\n\r", i->short_description);

    CAP(buf);
    send_to_char(buf, ch);
    j--;
  }

  if( j == level >> 1 )
    sendChar( ch, "You sense nothing.\n\r" );
}



ASPELL(spell_charm)
{
    struct affected_type af;
    int    chance = 10;
    int    add    = 10;
    struct follow_type *f;
    int    charm_factor = ( GET_CLASS(ch) == CLASS_MAGIC_USER ? 2 : 1 );

    /* Ok, we've already established that mages are the best at  */
    /* charming things. Now let's toss in an int factor to boot. */
    if( GET_INT(ch) > 17 )
        charm_factor += 1;

    if( victim == NULL || ch == NULL )
        return;

    if( victim == ch ){
        send_to_char("You like yourself even better!\r\n", ch);
        return;
    }

    else if( IS_NPC(victim) && IS_SET_AR( MOB_FLAGS( victim ), MOB_NOCHARM ))
        send_to_char("Your victim is impervious to your guile you ugly slug!\r\n", ch );

    else if( IS_AFFECTED(ch, AFF_CHARM ))
        send_to_char("You can't have any followers of your own!\r\n", ch);

    else if (IS_AFFECTED(victim, AFF_CHARM) || level < GET_LEVEL(victim))
        send_to_char("You fail.\r\n", ch);

    /* player charming a BIG critter */
    /* changed to using player's mana instead of hp for charm -Fenrir */
    else if( GET_MAX_MANA(ch)*charm_factor < GET_MAX_HIT(victim) )
        send_to_char("You aren't powerful enough.\r\n", ch);

    /* player charming another player - no legal reason for this */
    else if (!IS_NPC(victim) && !IS_NPC(ch))
        send_to_char("You fail - shouldn't be doing it anyway.\r\n", ch);

    else if (circle_follow(victim, ch))
        send_to_char("Sorry, following in circles can not be allowed.\r\n", ch);

    else if (magic_savingthrow(ch, victim, SAVING_PARA))
    {
        send_to_char("Your victim resists!\r\n", ch);
        if (!FIGHTING(victim)) set_fighting(victim, ch);
    }
    else {

        if( victim->master )
            stop_follower(victim);

        add_follower(victim, ch);

        af.type      = SPELL_CHARM;
        af.duration  = (charm_factor * (GET_LEVEL(ch)/3)) TICKS;
        af.modifier  = 0;
        af.location  = 0;
        af.bitvector = AFF_CHARM;
        af.level     = GET_LEVEL(ch);
        affect_to_char(victim, &af);

        act("Isn't $n just such a nice fellow?", FALSE, ch, 0, victim, TO_VICT);
        if (IS_NPC(victim)) {
	    REMOVE_BIT_AR(MOB_FLAGS(victim), MOB_AGGRESSIVE);
	    REMOVE_BIT_AR(MOB_FLAGS(victim), MOB_SPEC);
		REMOVE_BIT_AR(MOB_FLAGS(victim), MOB_GRENADER);
        }
    }

  /*
   * Liam April 11, 1995
   *
   * Everytime you cast charm, you run the risk of losing control
   * of the mobs that you already have charmed.  If you lose
   * control, they attack you... ain't that great?
   */
    f = ch->followers;
    while( f ){
        /* Sure, you can have multiple "pets", but it's gonna
         * be tougher and tougher to keep them under control!
         */
        if( IS_AFFECTED(f->follower, AFF_CHARM) && (f->follower->master == ch )){
            chance += add;
            add *= 2;
        }
        f = f->next;
    }

    if( number(1, 100) < chance ){
        /* Oh oh */
        f = ch->followers;
        while ( f ) {
            if( IS_AFFECTED(f->follower, AFF_CHARM ) && (f->follower->master == ch )) {
                affect_from_char (f->follower, SPELL_CHARM);
                act ("$n loses control of $N!", FALSE, ch, 0, f->follower, TO_NOTVICT);
                act ("You break free from $n's control!", FALSE, ch, 0, f->follower, TO_VICT);
                act ("You lose control of $N!", FALSE, ch, 0, f->follower, TO_CHAR);
                if(( f->follower->in_room == ch->in_room ) && (!FIGHTING(f->follower))){
                    set_fighting ( f->follower, ch );
                }
            }
            f = f->next;
        }
    }
}



ASPELL(spell_identify)
{
  int i;
  int found;

  struct time_info_data age(CharData * ch);

  if (obj) {
    if( (IS_OBJ_STAT(obj, ITEM_ARTIFACT))){
       send_to_char( "Your magic flows around the object leaving it untouched!\r\n", ch);
       return;
    }
	
	SET_BIT_AR(obj->obj_flags.extra_flags, ITEM_IDENTIFIED);

    send_to_char("You feel informed:\r\n", ch);
    sprintf(buf, "Object '%s', Item type: ", obj->short_description);
    sprinttype(GET_OBJ_TYPE(obj), item_types, buf2, sizeof(buf2));
    strcat(buf, buf2);
    strcat(buf, "\r\n");
    send_to_char(buf, ch);

    if (obj->obj_flags.bitvector) {
      send_to_char("Item will give you following abilities:  ", ch);
//128      sprintbit(obj->obj_flags.bitvector, affected_bits, buf);
      sprintbitarray(obj->obj_flags.bitvector, affected_bits, AF_ARRAY_MAX, buf);
      strcat(buf, "\r\n");
      send_to_char(buf, ch);
    }
    send_to_char("Item is: ", ch);
//128    sprintbit(GET_OBJ_EXTRA(obj), extra_bits, buf);
    sprintbitarray(GET_OBJ_EXTRA(obj), extra_bits, EF_ARRAY_MAX, buf);
    strcat(buf, "\r\n");
    send_to_char(buf, ch);

    sprintf(buf, "Weight: %d, Value: %d, Rent: %d\r\n",
        GET_OBJ_WEIGHT(obj), GET_OBJ_COST(obj), GET_OBJ_RENT(obj));
    send_to_char(buf, ch);

    /* show them item values. */
    oPrintValues(obj, buf);
    sendChar(ch, "%s\r\n", buf);

    /* And now applies. */
    found = FALSE;
    for (i = 0; i < MAX_OBJ_AFFECT; i++) {
      if ((obj->affected[i].location != APPLY_NONE) &&
      (obj->affected[i].modifier != 0)) {
        if (!found) {
          send_to_char("Can affect you as :\r\n", ch);
          found = TRUE;
        }
        sprinttype(obj->affected[i].location, apply_types, buf2, sizeof(buf2));
        sprintf(buf, "   Affects: %s By %d\r\n", buf2, obj->affected[i].modifier);
        send_to_char(buf, ch);
      }
    }
    /* Now special procedures. */
    if (IS_MAGIC_USER(ch)) {
    	if ( obj->item_number >= 0 &&
	     obj_index[obj->item_number].combatSpec &&
	     GET_LEVEL(ch) >= 30 &&
	     skillSuccess(ch, SPELL_IDENTIFY)) {
	    combSpecName(obj_index[obj->item_number].combatSpec, buf);
	    sendChar(ch, "You sense this item has the following arcane power: %s\r\n", buf);
	}
    } /* lvl 30 mages can detect special routines. */

  }

  else if (victim == ch) {        /* victim */
    sprintf(buf, "Name: %s\r\n", GET_NAME(victim));
    send_to_char(buf, ch);
    if (!IS_NPC(victim)) {
      sprintf(buf, "%s is %d years, %d months, %d days and %d hours old.\r\n",
          GET_NAME(victim), age(victim).year, age(victim).month,
          age(victim).day, age(victim).hours);
      send_to_char(buf, ch);
    }
    sprintf(buf, "Height %d cm, Weight %d pounds\r\n", GET_HEIGHT(victim), GET_WEIGHT(victim));
    sprintf(buf, "%sLevel: %d, Hits: %d, Mana: %d, Move: %d\r\n", buf, GET_LEVEL(victim), GET_HIT(victim), GET_MANA(victim), GET_MOVE(victim));

    int i;
    int hit = 0, mana = 0, move = 0;
    for(i = 0; i < 72; i++){
        hit  += hit_gain(ch);
        mana += mana_gain(ch);
        move += move_gain(ch);
    }
    sprintf(buf, "%sRegen -- Hit: Base: %3d  Multi: %3d  Bonus: %3d  Total: %3d\r\n", buf, calcHitBase(ch), calcHitMulti(ch), calcHitBonus(ch), hit/72);
    sprintf(buf, "%s        Mana:       %3d         %3d         %3d         %3d\r\n", buf, calcManaBase(ch), calcManaMulti(ch), calcManaBonus(ch), mana/72);
    sprintf(buf, "%s        Move:       %3d         %3d         %3d         %3d\r\n", buf, calcMoveBase(ch), calcMoveMulti(ch), calcMoveBonus(ch), move/72);

    int skillSuccessMod = 0;
    for(i = 0; i<250; i++)
        if(equipmentSkillSuccess(ch)) {
            skillSuccessMod += 1;
        }

    sprintf(buf, "%sAC: %d, Hitroll: %d, Damroll: %d\r\n",
	buf, realAC(victim), GET_HITROLL(victim),
	GET_DAMROLL(victim));

    sprintf(buf, "%sSpell Damage: %d%, Approx. Skill Success Mod: %d\r\n",
	buf, 100 + spell_damage_gear(victim)/3,
        skillSuccessMod/10);

    sprintf(buf, "%sStr: %d/%d, Int: %d, Wis: %d, Dex: %d, Con: %d, Cha: %d\r\n",
            buf, GET_STR(victim), GET_ADD(victim), GET_INT(victim),
            GET_WIS(victim), GET_DEX(victim), GET_CON(victim), GET_CHA(victim));
    
    sprintf(buf, "%sNaked Stats: Str: %d/%d, Int: %d, Wis: %d, Dex: %d, Con: %d, Cha: %d\r\n",
            buf, ch->real_abils.str, ch->real_abils.str_add, ch->real_abils.intel,
            ch->real_abils.wis, ch->real_abils.dex, ch->real_abils.con, ch->real_abils.cha);
  
    send_to_char(buf, ch);
  }

}



ASPELL(spell_enchant_weapon)
{
  int i;

  if (ch == NULL || obj == NULL) return;

  if ((GET_OBJ_TYPE(obj) == ITEM_WEAPON) &&
      !IS_SET_AR(GET_OBJ_EXTRA(obj), ITEM_MAGIC)) {

    for (i = 0; i < MAX_OBJ_AFFECT; i++)
      if (obj->affected[i].location != APPLY_NONE)
        return;

    SET_BIT_AR(GET_OBJ_EXTRA(obj), ITEM_MAGIC);

    obj->affected[0].location = APPLY_HITROLL;
    obj->affected[0].modifier = 1 + (level >= 25) + (level >= 35) + (level >= 45);

    obj->affected[1].location = APPLY_DAMROLL;
    obj->affected[1].modifier = 1 + (level >= 30) + (level >= 40) + (level >= 50);

    if (IS_GOOD(ch)) {
      SET_BIT_AR(GET_OBJ_EXTRA(obj), ITEM_ANTI_EVIL);
      act("$p glows blue.", FALSE, ch, obj, 0, TO_CHAR);
    } else if (IS_EVIL(ch)) {
      SET_BIT_AR(GET_OBJ_EXTRA(obj), ITEM_ANTI_GOOD);
      act("$p glows red.", FALSE, ch, obj, 0, TO_CHAR);
    } else {
      act("$p glows yellow.", FALSE, ch, obj, 0, TO_CHAR);
    }
  }
}


ASPELL(spell_portal)
{
    CharData *clone;
    CharFileU template;
    char cloneDescr[80] = "";

    ObjData *portal = NULL;
    int no_can_go = FALSE;
    int to_room = NOWHERE;
    int obj_no;

    if (ch == NULL) return;

    if (ZONE_FLAGGED(world[victim->in_room].zone, ZONE_NOPORTAL) || SEEKING(victim)) {
        send_to_char("The magic unravels before you can complete the spell!\r\n", ch);
        return;
    }

    if (victim) {
        if (victim->in_room == NOWHERE || victim->in_room == 1      ||
            IS_SET_AR(world[victim->in_room].room_flags, ROOM_PRIVATE) ||
            IS_SET_AR(world[victim->in_room].room_flags, ROOM_DEATH)   ||
            IS_SET_AR(world[victim->in_room].room_flags, ROOM_GODROOM) ||
            IS_SET_AR(world[victim->in_room].room_flags, ROOM_CLAN )   /*||
	    IS_SET_AR(world[victim->in_room].room_flags, ROOM_PEACEFUL)*/ ||
            (IS_NPC(victim) && IS_SET_AR(MOB_FLAGS(victim), MOB_NOSUMMON)) ||
            (!IS_NPC(victim) && GET_LEVEL(victim) > GET_LEVEL(ch) && GET_LEVEL(victim) > MAX_MORTAL))
        /* if the dude isn't anywhere or is in limbo (protect link-dead) */
            no_can_go = TRUE;
        else
            to_room = victim->in_room;
    }
#ifdef CAN_PORTAL_TO_OBJ
    else if (obj)
    {
        if (obj->in_room == NOWHERE)
            no_can_go = TRUE;
        else
            to_room = obj->in_room;

    }
#endif
    if (no_can_go)
    {
        act("Your magic splutters and dies, as if extinguished by a powerful force.", FALSE, ch, 0, 0, TO_CHAR);
        act("$n's magic splutters and dies.", FALSE, ch, 0, 0, TO_ROOM);
        return;
    }
    else if ((zone_table[world[to_room].zone].reset_mode == 3) && (GET_LEVEL(ch) < LVL_IMMORT)) { /* IMM only zone. */
	sendChar(ch, "Mortals cannot portal to there.\r\n");
	return;
    }

    obj_no = PORTAL_OBJ_VNUM;
    if (!(portal = read_perfect_object(obj_no, VIRTUAL)))
    {
        send_to_char("Oops! report this bug.\n\r", ch);
        mlog("SYSERR : no portal object when creating portal (obj num %d)", obj_no);
        return;
    }

    act("You utter a powerful incantation and stretch the fabric of reality, opening\n$p to your target.",
            FALSE, ch, portal, 0, TO_CHAR);
    act("$n utters a powerful incantation, creating $p!", FALSE, ch, portal, 0, TO_ROOM);

    portal->obj_flags.value[0] = world[to_room].number;
    portal->obj_flags.value[1] = level / 5;
    portal->obj_flags.value[2] = 10;
    GET_OBJ_TIMER(portal) = GET_LEVEL(ch)/2;
    SET_BIT_AR(portal->obj_flags.extra_flags, ITEM_TIMED);
    obj_to_room(portal, ch->in_room);

    // If they are advanced, create a close to make a portal back.
    if(!IS_NPC(ch) && IS_ENCHANTER(ch) && GET_ADVANCE_LEVEL(ch) >= 4 &&
            !IS_SET_AR(MOB_FLAGS(ch), MOB_NOSUMMON)) {

        CREATE( clone, CharData, 1 );
        clear_char( clone );
        
        if( load_char( GET_NAME(ch), &template ) < 0 ){
            sendChar( ch, "No such player - clone aborted.\r\n" );
            free(clone);
            return;
        }
        store_to_char( &template, clone );

        strcpy( cloneDescr, clone->player.name );
        if( clone->player.short_descr != NULL )
            free( clone->player.short_descr );
        clone->player.short_descr = strdup( cloneDescr );

        SET_BIT_AR(MOB_FLAGS(clone), MOB_ISNPC);
        SET_BIT_AR(MOB_FLAGS(clone), MOB_CLONE);
        clone->mobskill_suc = 100;

        char_to_room(clone, victim->in_room);

        cast_spell( clone, ch, NULL, SPELL_PORTAL);
        extract_char(clone);
    }

}


ASPELL(spell_calm)
{
    if (!ch || !victim)
        return;

	if (SINGING(victim) == SKILL_BERSERK)	{
		if (SINGING(victim)) stop_singing(victim);
		act("$N calms down.", FALSE, ch, 0, victim, TO_NOTVICT);
		act("$N calms down.", FALSE, ch, 0, victim, TO_CHAR);
		act("You start to see things from others' point of view.", FALSE, ch, 0, victim, TO_VICT);
	}


    if (magic_savingthrow(ch, victim, SAVING_SPELL))
    {
        send_to_char("You failed.\r\n", ch);
        if (number(0,3)==0)
            hit(victim, ch, TYPE_UNDEFINED);
        return;
    }

	// Either way, don't remove flags from player or end combat.
	if (!IS_NPC(victim))// || !IS_SET_AR(MOB_FLAGS(victim), MOB_AGGRESSIVE))
        return;

    REMOVE_BIT_AR(MOB_FLAGS(victim), MOB_AGGRESSIVE);
	REMOVE_BIT_AR(MOB_FLAGS(victim), MOB_AGGR_EVIL );
	REMOVE_BIT_AR(MOB_FLAGS(victim), MOB_AGGR_GOOD);
	REMOVE_BIT_AR(MOB_FLAGS(victim), MOB_AGGR_NEUTRAL);

    if (FIGHTING(victim))
    {
        end_fight(victim);
    }

    act("You feel calm and peaceful.", FALSE, ch, 0, victim, TO_VICT);
    act("$N calms down.", FALSE, ch, 0, victim, TO_NOTVICT);
    act("$N calms down.", FALSE, ch, 0, victim, TO_CHAR);

}

ASPELL(spell_forget)
{
    memory_rec *mob_mem, *tmp;

    if (!ch || !victim)
        return;

    if (!IS_NPC(victim) || !IS_SET_AR(MOB_FLAGS(victim), MOB_MEMORY) || !(mob_mem = MEMORY(victim)))
    {
        send_to_char("You failed.\r\n", ch);
        return;
    }

    if (magic_savingthrow(ch, victim, SAVING_SPELL))
    {
        send_to_char("You failed.\r\n", ch);
        if (number(0,5) == 0)
            hit(victim, ch, TYPE_UNDEFINED);
        return;
    }

    while (mob_mem)
    {
        tmp = mob_mem;
        free(mob_mem);
        mob_mem = tmp->next;
    }
    act("$N looks forgetful.", FALSE, ch, 0, victim, TO_ROOM);
    act("$N looks forgetful.", FALSE, ch, 0, victim, TO_CHAR);
    act("You wonder what it was you were thinking.", FALSE, ch, 0, victim, TO_VICT);
}

ASPELL(spell_banish)
{
    int i;
    ObjData *temp_obj;

    if (!ch || !victim) return;

    if (!IS_NPC(victim) || !IS_SET_AR(MOB_FLAGS(victim), MOB_CONJURED)) {
        sendChar( ch, "Your banishment failed.\r\n" );
        if (IS_NPC(victim) && number(0,5) == 0)
            hit(victim, ch, TYPE_UNDEFINED);
        return;
    }

    if (magic_savingthrow(ch, victim, SAVING_SPELL)) {
        sendChar( ch, "Your banishment failed.\r\n" );
        hit(victim, ch, TYPE_UNDEFINED);
        return;
    }

    act("Your magical energies weave around $N, banishing $M!", FALSE, ch, 0, victim, TO_CHAR);
    act("$n's magical energies weave around $N, banishing $M!", FALSE, ch, 0, victim, TO_NOTVICT);
    act("You are banished by $n...sorry.", FALSE, ch, 0, victim, TO_VICT);
    for (i = 0; i < NUM_WEARS; i++)
    {
        if (victim->equipment[NUM_WEARS])
            extract_obj(victim->equipment[NUM_WEARS]);
    }

    // transfer objects to room, if any
    while( victim->carrying )
    {
        temp_obj = victim->carrying;
        obj_from_char(temp_obj);
        obj_to_room(temp_obj, victim->in_room);
    }

    if(!affected_by_spell( victim, SPELL_CHARM_CORPSE))
       extract_char(victim);
    else {
        char_from_room(victim);
        char_to_room(victim, 1);
	affect_from_char(victim, SPELL_CHARM_CORPSE);
    }

}

ASPELL(spell_sand_storm)
{

    char sand_buf[240];
    int dir_to_go, pos;

    if (!victim)
        return;

    if (!FIGHTING(victim))
    set_fighting(victim, ch);
    /* This prevents sand storm causing any damage upon saving throw
     * if (magic_savingthrow(ch, victim, SAVING_SPELL))
     * {
     * act("You fail.", FALSE, ch, 0, victim, TO_CHAR);
     * return;
     * }
     */
    act("The winds pick up and a great wall of sand assaults $N!", FALSE, ch, 0, victim, TO_CHAR);
    act("The winds pick up and a great wall of sand assaults you!", FALSE, ch, 0, victim, TO_VICT);
    act("The winds pick up and a great wall of sand assaults $N!", FALSE, ch, 0, victim, TO_NOTVICT);


    damage(ch, victim, number(level, 2), SPELL_SAND_STORM);

    /* Mortius: If the player dies then we try and check to throw
		it out of the room, we crash. */

    if (GET_HIT(victim) <= 0)
        return;

    if (magic_savingthrow(ch, victim, SAVING_SPELL) ||
       (IS_NPC(victim) && IS_SET_AR(MOB_FLAGS(victim), MOB_SENTINEL)))
    {
        act("You fail.", FALSE, ch, 0, victim, TO_CHAR);
        return;
     }

    dir_to_go = number(0,5);
    if (!CAN_GO(victim, dir_to_go))
        return;
    memset(sand_buf, 0, 240);
    sprintf(sand_buf,"You are picked up, and hurled %swards!", dirs[dir_to_go]);
    act(sand_buf, FALSE, victim, 0, 0, TO_CHAR);
    sprintf(sand_buf, "$n is picked up, and hurled %swards!", dirs[dir_to_go]);
    act(sand_buf, FALSE, victim, 0, 0, TO_ROOM);

    pos = GET_POS(victim);
    GET_POS(victim) = POS_STANDING;
    do_simple_move(victim, dir_to_go, 0);
    GET_POS(victim) = pos;

}

ASPELL(spell_hands_of_wind)
{
  char wind_buf[240];
  int dir_to_go;

  if( !victim ) return;

  /* Hands of wind will now be based on the relative size of ch and victim.
  if( magic_savingthrow(ch, victim, SAVING_SPELL) )
  {
    act("You fail.", FALSE, ch, 0, victim, TO_CHAR);
    return;
  }
  */

  if ( GET_MAX_HIT(ch) == 0 )
  {
	  mudlog(NRM, LVL_IMMORT, TRUE, "Hands of wind casted by %s, who has GET_MAX_HIT == 0.  This will crash the mud.", GET_NAME(ch) );
	  return;
  }

  // Fail if your opponent is big.  Players get a little bonus resistance so they aren't
  // wupped 
  if( number( 1, ((GET_MAX_HIT(victim)/GET_MAX_HIT(ch)) + 2*IS_PVP(ch, victim))) == 1 ? 0 : 1  )
  {
	  act("The winds howl, but $N's feet stand firm.", FALSE, ch, 0, victim, TO_CHAR);
	  act("The winds howl, but you lose no ground.", FALSE, ch, 0, victim, TO_VICT);
	  act("The winds howl, but $N's loses no ground.", FALSE, ch, 0, victim, TO_NOTVICT);
	  set_fighting(victim, ch);
	  return;
  }

  act("The winds pick up and a wall of air buffets $N!", FALSE, ch, 0, victim, TO_CHAR);
  act("The winds pick up and a wall of air buffets you!", FALSE, ch, 0, victim, TO_VICT);
  act("The winds pick up and a wall of air buffets $N!", FALSE, ch, 0, victim, TO_NOTVICT);

  if( IS_NPC(victim) && IS_SET_AR(MOB_FLAGS(victim), MOB_SENTINEL) ) return;

  dir_to_go = number(0,5);
  if( !CAN_GO(victim, dir_to_go) )
  {
    act("You are slammed into the wall!", FALSE, victim, 0, 0, TO_CHAR);
    act("$n is picked up, and slammed into the wall!", FALSE, victim, 0, 0, TO_ROOM);
    damage(ch, victim, number(GET_LEVEL(ch), 2), SPELL_HANDS_OF_WIND);
    WAIT_STATE( victim, SET_STUN(number( 2, 3 )));
  }
  else
  {
    sprintf(wind_buf,"You are picked up, and hurled %swards!", dirs[dir_to_go]);
    act(wind_buf, FALSE, victim, 0, 0, TO_CHAR);
    sprintf(wind_buf, "$n is picked up, and hurled %swards!", dirs[dir_to_go]);
    act(wind_buf, FALSE, victim, 0, 0, TO_ROOM);
    do_simple_move(victim, dir_to_go, 0);
  }
}


/*
 *Old shriek which wasn't used anyway.
 *
ASPELL(spell_shriek)
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
    CharData *temp_ch, *temp2;

    act("A piercing shriek fills the air!", FALSE, ch, 0, 0, TO_CHAR);
    act("A piercing shriek fills the air!", FALSE, ch, 0, 0, TO_ROOM);

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
                    sprintf(buf,"You hear a distant shriek from %s.", rev_dirs[i]);
                    act(buf,FALSE, ch, 0, 0, TO_ROOM);
                    for (temp2 = world[ch->in_room].people; temp2; temp2 = temp2->next_in_room)
                    {
                        temp_ch = temp2;
                        if (temp_ch == ch)
                            continue;
                        if (!IS_NPC(temp_ch) || IS_SET_AR(MOB_FLAGS(temp_ch), MOB_SENTINEL))
                            continue;
                        if (!magic_savingthrow(ch, temp_ch, SAVING_SPELL && number(0,1) == 0))
                            do_simple_move(temp_ch, rev_dir[i], 1);
                    }
                }
                else
                {
                    sprintf(buf,"You hear a distant shriek.");
                    act(buf,FALSE, ch, 0, 0, TO_ROOM);
                }
            }
        }
    }
    ch->in_room = orig_room;
    
    if (IS_NPC(ch))
        for (temp_ch = world[ch->in_room].people; temp_ch; temp_ch = temp_ch->next_in_room)
        {
            if (temp_ch != ch && IS_NPC(temp_ch))
                if (!IS_SET_AR(MOB_FLAGS(temp_ch), MOB_AGGRESSIVE))
                {
                    SET_BIT_AR(MOB_FLAGS(temp_ch), MOB_AGGRESSIVE);
                    act("$n is irritated by the high pitched noise.", FALSE, temp_ch, 0, 0, TO_ROOM);
                    act("You are irritated by the high pitched noise.", FALSE, temp_ch, 0, 0, TO_CHAR);
                }
        }
     
}
*/

ASPELL(spell_shriek)
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

    act("A piercing shriek fills the air!", FALSE, ch, 0, 0, TO_CHAR);
    act("A piercing shriek fills the air!", FALSE, ch, 0, 0, TO_ROOM);

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
                    sprintf(buf,"You hear a distant shriek from %s.", rev_dirs[i]);
                    act(buf,FALSE, ch, 0, 0, TO_ROOM);
                }
                else
                    act("You hear a distant shriek.",FALSE, ch, 0, 0, TO_ROOM);
            }
        }
    }
}

ASPELL(spell_life_drain)
{
    int dam;
    int amount = 10;

    if (!ch || !victim) return;

    dam = (GET_LEVEL(victim) * 2) + number(0, (GET_LEVEL(victim) * (GET_LEVEL(ch)/16)));
    /* Adjust for ch's alignment. */
    if (IS_EVIL(ch)) {
	if (IS_EVIL(victim)) {
	    GET_ALIGNMENT(ch) += amount;
	    dam >>= 1;
	}
	else if (IS_GOOD(victim))
	    GET_ALIGNMENT(ch) = MAX(-1000, GET_ALIGNMENT(ch) - amount);
    }
    else if (IS_GOOD(ch)) {
	if (IS_GOOD(victim)) {
	    GET_ALIGNMENT(ch) -= amount;
	    dam >>= 1;
	}
	else if (IS_EVIL(victim))
	    GET_ALIGNMENT(ch) = MIN(1000, GET_ALIGNMENT(ch) + amount);
    }
    else { /* ch is neutral */
	if (IS_NEUTRAL(victim)) { /* Move ch away from neutrality */
	    if (GET_ALIGNMENT(ch) > 0)
		GET_ALIGNMENT(ch) += amount;
	    else
		GET_ALIGNMENT(ch) -= amount;
	    dam >>= 1;
	}
	else { /* Move ch closer to neutrality */
	    if (GET_ALIGNMENT(ch) > 0)
		GET_ALIGNMENT(ch) = MAX(0, GET_ALIGNMENT(ch) - amount);
	    else
		GET_ALIGNMENT(ch) = MIN(0, GET_ALIGNMENT(ch) + amount);
	}
    } /* end of alignment adjustments */

    if (magic_savingthrow(ch, victim, SAVING_PARA)) {
        sendChar( ch, "%s resists your power!\r\n", GET_NAME(victim) );
	dam >>= 1;
    }
    else
        sendChar( ch, "%s succumbs to your power!\r\n", GET_NAME(victim) );

    damage(ch, victim, dam, SPELL_LIFE_DRAIN);
    if (AFF_FLAGGED(victim, AFF_SHIELD))
	sendChar(ch, "A force field around %s prevents the transfer of life force to you!\r\n", GET_NAME(victim));
    else {
        GET_HIT(ch) += (dam/2);
    }
}

ASPELL(spell_minor_creation)
{

}
ASPELL(spell_cleanse)
{
    if( affected_by_spell( victim, SPELL_PARALYZE ))
    {
      affect_from_char( victim, SPELL_PARALYZE );
      act("The stiffness fades from your limbs.", FALSE, victim, 0, 0, TO_CHAR);
    }
    if( affected_by_spell( victim, SPELL_BLINDNESS ))
    {
      affect_from_char( victim, SPELL_BLINDNESS );
      act("The curtain of darkness falls from your eyes.", FALSE, victim, 0, 0, TO_CHAR);
    }
    if( affected_by_spell( victim, SKILL_DUST ))
    {
      affect_from_char( victim, SKILL_DUST );
      act("The dust is flushed out of your eyes.", FALSE, victim, 0, 0, TO_CHAR);
    }
    if( affected_by_spell( victim, SPELL_PLAGUE ))
    {
      affect_from_char( victim, SPELL_PLAGUE );
      act("You feel a bit more healthy.", FALSE, victim, 0, 0, TO_CHAR);
    }
    if(affected_by_spell(victim, SPELL_POISON)) {
        affect_from_char(victim, SPELL_POISON);
        act("You feel a bit better.", FALSE, victim, 0, 0, TO_CHAR);
    }
    if(affected_by_spell(victim, SPELL_DISEASE)) {
        affect_from_char(victim, SPELL_DISEASE);
        act("You feel a lot better.", FALSE, victim, 0, 0, TO_CHAR);
    }
}

ASPELL(spell_sacrifice)
{
  if (!ch || !victim) return;

  GET_HIT(victim) = GET_MAX_HIT(victim);

  act("You sacrifice yourself for $N!", TRUE, ch, 0, victim, TO_CHAR);
  act("$n sacrifices $mself for $N.", TRUE, ch, 0, victim, TO_NOTVICT);
  act("$n sacrifices $mself for you.", TRUE, ch, 0, victim, TO_VICT);

  add_affect(ch, ch, SPELL_SACRIFICE, 0, APPLY_AC, 20, 1 TICKS, AFF_PARALYZE, FALSE,
      FALSE, FALSE, FALSE);

  GET_HIT(ch) -= GET_MAX_HIT(ch) / 2;
  update_pos(ch);
  if (GET_POS(ch) < POS_SLEEPING) {
    send_to_char("Alas, your sacrifice has left you in a bad way.\r\n", ch);
    act("$n's sacrifice has left them nearly dead.", FALSE, ch, 0, 0, TO_ROOM);
  } else if (GET_POS(ch) == POS_DEAD) {
    send_to_char("Alas, your sacrifice has killed you.\r\n", ch);
    die(ch, ch, 0);
  }
}

int read_max_hit(ObjData *corpse) {
    int max_hit = 0;
    CharData *mob = NULL;

    mob = read_mobile(corpse->obj_flags.cost_per_day, VIRTUAL);

    if (!mob)
        return -1;
    
    char_to_room(mob, 1);
    max_hit =  GET_MAX_HIT(mob);
    extract_char(mob);

    return max_hit;

}

ASPELL(spell_consume_corpse)
{
    int maxgain, gain;
    CharData *mob;

    if (!ch || !obj) return;

    if (GET_OBJ_TYPE(obj) != ITEM_CONTAINER || GET_OBJ_VAL(obj, 3) != 1) {
        send_to_char("Maybe you should just eat that.\r\n", ch);
        return;
    }

    if (!CAN_WEAR(obj, ITEM_WEAR_TAKE)) {
        send_to_char("That's not a very nice thing to try!\r\n", ch);
        return;
    }

    act("$n is revitalised by $p!", TRUE, ch, obj, 0, TO_ROOM);
    act("You are revitalised by $p.", TRUE, ch, obj, 0, TO_CHAR);

    maxgain = GET_MAX_HIT(ch) - GET_HIT(ch);

    if(read_max_hit(obj) == -1) {
        send_to_char("The life has completely left this creature.\r\n", ch);
        return;
    }
    
    /* see below for a discussion of this expression. */
    gain = GET_LEVEL(ch) * log(MAX(1, read_max_hit(obj))) / 1.8;

    GET_HIT(ch) += MIN(maxgain, gain);
     
    // If the player is a Revenant, they can consume the corpse while
    // leaving some behind for "seconds".

    if(IS_REVENANT(ch) && GET_ADVANCE_LEVEL(ch) >= SECOND_ADVANCE_SKILL) {
        // 33% chance of leaving seconds.  33% chance of increasing max hit points
        if(percentSuccess(66)) {
            add_affect(ch, ch, SPELL_CONSUME_CORPSE, 0, APPLY_HIT, MIN(gain/number(5, 15), 100), 2 TICKS, 0, FALSE, FALSE, FALSE, FALSE);
        }
        if(percentSuccess(66)) {
            obj->obj_flags.cost = number(gain, obj->obj_flags.cost);
            return;
        }
    }


    extract_obj(obj);
}

ASPELL(spell_charm_corpse)
{
    struct affected_type af;
    FollowType *f;
    CharData *mob;
    int size;

    if (!ch || !obj) return;

    if (GET_OBJ_TYPE(obj) != ITEM_CONTAINER || GET_OBJ_VAL(obj, 3) != 1) {
        send_to_char("You don't know how to charm that.\r\n", ch);
        return;
    }

    if (obj->contains) {
        send_to_char("The corpse must be empty first.\r\n", ch);
        return;
    }

    if (!CAN_WEAR(obj, ITEM_WEAR_TAKE)) {
        send_to_char("The corpse resists your attempted charm.\r\n", ch);
        return;
    }

    for (f = ch->followers; f; f = f->next) {
        if (IS_AFFECTED(f->follower, AFF_CHARM) && (f->follower->master == ch)) {
            if(affected_by_spell(f->follower, SPELL_CHARM_CORPSE))
               affect_from_char(f->follower, SPELL_CHARM_CORPSE);
            else {
                sendChar(ch, "You cannot control more than one creature.\r\n");
                return;
            }
        }
    }

    mob = read_mobile(obj->obj_flags.cost_per_day, VIRTUAL);
    if (!mob) {
        send_to_char("The life has completely left this creature.\r\n", ch);
        return;
    }

    char_to_room(mob, ch->in_room);

    if (GET_LEVEL(ch) < GET_LEVEL(mob) ||
	IS_SET_AR(MOB_FLAGS(mob), MOB_NOCHARM)) {
        send_to_char("This creature cannot be resurrected.\r\n", ch);
        extract_char(mob);
        return;
    }

    if (GET_MAX_HIT(mob) > GET_LEVEL(ch) * 100) {
        sendChar(ch, "You cannot control such a powerful creature!\r\n");
        extract_char(mob);
        return;
    }
    size = GET_MAX_HIT(mob);
    /* Bigger it is, harder it is.  We return the corpse to the floor when this fails.*/
    if (percentSuccess(GET_MAX_HIT(mob)/(GET_LEVEL(ch)*4))) {
        act("$N struggles to stand, but cannot hold $Mself together!",
                TRUE, ch, NULL, mob, TO_CHAR);
        act("$N struggles to stand, but cannot hold $Mself together!",
                TRUE, ch, NULL, mob, TO_ROOM);
        extract_char(mob);
        return;
    }

    
// Normalize the damage a mob does
    #define norm_nodice GET_LEVEL(ch)/5
    if(mob->mob_specials.damnodice > norm_nodice)
        mob->mob_specials.damnodice = number(norm_nodice, mob->mob_specials.damnodice);
    else
        mob->mob_specials.damnodice = number(mob->mob_specials.damnodice, norm_nodice);
    #undef norm_nodice

    #define norm_dicesize GET_LEVEL(ch)/10
    if(mob->mob_specials.damsizedice > norm_dicesize)
        mob->mob_specials.damsizedice = number(norm_dicesize, mob->mob_specials.damsizedice);
    else
        mob->mob_specials.damsizedice = number(mob->mob_specials.damsizedice, norm_dicesize);
    #undef norm_dicesize

    int num_attacks, avg_dam, raw_damage;
    #define NOMINAL_AC -7
    num_attacks = calculate_attack_count(mob);
    avg_dam = mob->mob_specials.damnodice * (mob->mob_specials.damsizedice + 1)/2 + GET_DAMROLL(mob);
    raw_damage = num_attacks*avg_dam + spellAttackStrength(mob)/100;

    mob->decay = obj->obj_flags.cost;
    // Mobs decay more readily if they're bigger.  Sanc or ward make them decay slightly faster.
    mob->decay += number(GET_MAX_HIT(mob)/1000, GET_MAX_HIT(mob)/400) *
        (affected_by_spell(mob, SPELL_SANCTUARY)?6:affected_by_spell(mob, SPELL_WARD)?5:4)/4;
    // Stronger mobs decay even more readily
    mob->decay += number(raw_damage/100, raw_damage/33);

    /* Enter the Mob */
    af.type      = SPELL_CHARM_CORPSE;
    af.duration  = (1 + (GET_LEVEL(ch) * 100 - GET_MAX_HIT(mob))/2000 ) TICKS;
    af.modifier  = 0;
    af.location  = 0;
    af.bitvector = AFF_CHARM;
    af.level     = level;
    affect_to_char(mob, &af);
    add_affect(ch, mob, SKILL_DELUSION, 4, 0, 0, -1, 0, FALSE, FALSE, FALSE, FALSE);

    /* Clean up the mob a bit */
    REMOVE_BIT_AR(MOB_FLAGS(mob), MOB_AGGRESSIVE);
    REMOVE_BIT_AR(MOB_FLAGS(mob), MOB_AGGR_EVIL);
    REMOVE_BIT_AR(MOB_FLAGS(mob), MOB_AGGR_GOOD);
    REMOVE_BIT_AR(MOB_FLAGS(mob), MOB_AGGR_NEUTRAL);
    REMOVE_BIT_AR(MOB_FLAGS(mob), MOB_PREDATOR);
    REMOVE_BIT_AR(MOB_FLAGS(mob), MOB_HELPER);
    REMOVE_BIT_AR(MOB_FLAGS(mob), MOB_SUPERAGG);
    REMOVE_BIT_AR(MOB_FLAGS(mob), MOB_GUARD_CLASS);
    REMOVE_BIT_AR(MOB_FLAGS(mob), MOB_GUARD_RACE);
    REMOVE_BIT_AR(MOB_FLAGS(mob), MOB_GUARD_BOTH);
    REMOVE_BIT_AR(MOB_FLAGS(mob), MOB_QUESTMASTER);
    REMOVE_BIT_AR(MOB_FLAGS(mob), MOB_SPEC);
    REMOVE_BIT_AR(MOB_FLAGS(mob), MOB_GRENADER);
    REMOVE_BIT_AR(MOB_FLAGS(mob), MOB_NOSUMMON);

    if(IS_WITCH_DOCTOR(ch)) {
        GET_MAX_HIT(mob) += 2* GET_ADVANCE_LEVEL(ch) * GET_MAX_HIT(mob) / 100;
        GET_HIT(mob) = GET_MAX_HIT(mob);
    }
    if(affected_by_spell(ch, SKILL_METAMORPHOSIS))
        affect_from_char(ch, SKILL_METAMORPHOSIS);

    WAIT_STATE(mob, SET_STUN(4));
    
    /* Cut it down to size */
    GET_GOLD(mob)     = 0;
    GET_EXP(mob)      = 1;

    /* the corpse goes away */
    act("$p twitches and stirs!", TRUE, ch, obj, NULL, TO_ROOM);
    act("$p twitches and stirs!", TRUE, ch, obj, NULL, TO_CHAR);
    extract_obj(obj);

    add_follower(mob, ch);
}

ASPELL(spell_showdam) {
    
}

/*Maestro - added entomb corpse and transmigrate corpse */
/*Craklyn - combined those two spells under entomb, since we're low on slots. */
ASPELL(spell_entomb_corpse)
{
	if(obj) {
		// If object chosen
		if (GET_OBJ_TYPE(obj) != ITEM_CONTAINER || GET_OBJ_VAL(obj, 3) != 1) {
			send_to_char("You can only entomb a corpse.\r\n", ch);
			return;
		}
		if (!CAN_WEAR(obj, ITEM_WEAR_TAKE)) {
			send_to_char("The corpse resists your attempt to entomb it.\r\n", ch);
			return;
		}
                
                if (obj->contains) {
                    send_to_char("The corpse must be empty first.\r\n", ch);
                    return;
                }

		act("$p sinks deep into the ground.", TRUE, ch, obj, NULL, TO_ROOM);
		act("$p sinks deep into the ground.", TRUE, ch, obj, NULL, TO_CHAR);

		ch->necro_corpse_recall_room = real_room(world[ch->in_room].number);

		extract_obj(obj);
	}
	else {
		// If no object selected
		if (ZONE_FLAGGED(world[ch->in_room].zone, ZONE_NORECALL)) {
			send_to_char("The magic unravels before you can complete the spell!\r\n", ch);
			return;
		}
		if (ch->necro_corpse_recall_room > 0) {
			act("You begin to shrivel up and decay, horrible pain wracks your body.", FALSE, ch, 0, NULL, TO_CHAR);
			act("$n begins to shrivel up and decay before your eyes, until only dust is left.", FALSE, ch, 0, NULL, TO_ROOM);
			char_from_room(ch);

			char_to_room(ch, ch->necro_corpse_recall_room);
			/*...after move...*/
			ch->necro_corpse_recall_room = 0;
			act("The ground quivers, and $n crawls up from the grave.", FALSE, ch, 0, NULL, TO_ROOM);
			look_at_room(ch, 0);
			act("You fade into the dead body entombed, and in desperation crawl your way to the surface.", FALSE, ch, 0, NULL, TO_CHAR);
			entry_memory_mtrigger(ch);
			greet_mtrigger(ch, -1);
			greet_memory_mtrigger(ch);
		} else {
			sendChar(ch, "How do you intend to go to a corpse you have never entombed?\r\n");
		}

	} //Else
}

ASPELL(spell_explode_corpse)
{
    int damage;

    if (!ch || !obj) return;

    if (GET_OBJ_TYPE(obj) != ITEM_CONTAINER || GET_OBJ_VAL(obj, 3) != 1) {
        send_to_char("You don't know how to explode that.\r\n", ch);
        return;
    }

    if (!CAN_WEAR(obj, ITEM_WEAR_TAKE)) {
        send_to_char("The corpse resists your attempted explosion.\r\n", ch);
        return;
    }

    if (obj->contains) {
      send_to_char("The corpse must be empty first.\r\n", ch);
      return;
    }
    
    if(read_max_hit(obj) == -1) {
        send_to_char("The life has completely left this creature.\r\n", ch);
        return;
    }

    act("$p explodes violently!", TRUE, ch, obj, NULL, TO_ROOM);
    act("$p explodes violently!", TRUE, ch, obj, NULL, TO_CHAR);

    /* By using a logarithmic function, the damage from exploding a
     * corpse scales up slower and slower the more hp a corpse had. So,
     * exploding a 1000hp corpse is much better than a 10hp corpse, but a
     * 30000hp corpse isn't ridiculously explosive.
     *   dice(4, damage) will return an average of, at char level 50:
     *      hitpoints       value
     *          10            48
     *         100            94
     *        1000           140
     *        5000           172
     *       10000           186
     *       30000           208
     */
    damage = (GET_LEVEL(ch) / 2) * log(MAX(1, read_max_hit(obj)));

    /* this damage is then passed over to mag_areas, masquerading as
     * the spell level. */
    mag_areas(damage, ch, SPELL_EXPLODE_CORPSE, SAVING_SPELL);

    extract_obj(obj);
}

ASPELL(spell_embalm)
{
//    #define	WEAR_N_TEAR		number(91, 100) / 100
    #define	WEAR_N_TEAR 		1
	
    if (!ch || !obj) return;

    if (GET_OBJ_TYPE(obj) != ITEM_CONTAINER || GET_OBJ_VAL(obj, 3) != 1) {
        send_to_char("You don't know how to embalm that.\r\n", ch);
        return;
    }

    if (!CAN_WEAR(obj, ITEM_WEAR_TAKE)) {
        send_to_char("You cannot preserve this corpse.\r\n", ch);
        return;
    }

    if (obj->contains) {
        send_to_char("The corpse must be empty first.\r\n", ch);
        return;
    }

    // Corpses get run down with extended use.  Max HP can't go below 1!!
    //obj->obj_flags.cost = MAX(1, obj->obj_flags.cost * WEAR_N_TEAR);

    GET_OBJ_TIMER(obj) += 50;
    GET_OBJ_WEIGHT(obj) = 1;
    obj->obj_flags.cost += 5;

    act("$p dries and shrivels up.", TRUE, ch, obj, 0, TO_ROOM);
    act("$p dries and shrivels up.", TRUE, ch, obj, 0, TO_CHAR);
}

ASPELL(spell_bone_wall)
{
    ObjData *wall = NULL;
    int i, dir = 0;

    if (!ch) return;

    for (i = 0; i < NUM_OF_DIRS; i++)
        if (cast_directions + i == obj) dir = i;

    if (!(wall = read_perfect_object(BONE_WALL_OBJ_VNUM, VIRTUAL)))
    {
        send_to_char("Oops! report this bug.\n\r", ch);
        mlog("SYSERR : no wall object when creating bone wall (#%d)",
                BONE_WALL_OBJ_VNUM);
        return;
    }

    act("You raise the bones of long dead warriors in a barricade.",
            FALSE, ch, 0, 0, TO_CHAR);
    act("$n raises a barricade of ancient bones!", FALSE, ch, 0, 0, TO_ROOM);

    /* load up the wall on this side */
    GET_OBJ_TIMER(wall) = 1;
    SET_BIT_AR(wall->obj_flags.extra_flags, ITEM_TIMED);
    sprintf(buf, "A wall of bones blocks passage %s.", dirs[dir]);
    wall->description = strdup(buf);
    wall->obj_flags.value[0] = dir;
    obj_to_room(wall, ch->in_room);

    /* if there's an exit, and it leads someplace ... */
    if (EXIT(ch, dir) && EXIT(ch, dir)->to_room != NOWHERE) {
        struct room_direction_data *back;
        back = world[EXIT(ch, dir)->to_room].dir_option[rev_dir[dir]];
        /* ... and it leads back here, put up a wall there too */
        if (back && back->to_room == ch->in_room) {
            wall = read_perfect_object(BONE_WALL_OBJ_VNUM, VIRTUAL);
            GET_OBJ_TIMER(wall) = 1;
            SET_BIT_AR(wall->obj_flags.extra_flags, ITEM_TIMED);
            sprintf(buf, "A wall of bones blocks passage %s.",
                    dirs[rev_dir[dir]]);
            wall->description = strdup(buf);
            wall->obj_flags.value[0] = rev_dir[dir];
            obj_to_room(wall, EXIT(ch, dir)->to_room);
            sprintf(buf, "A wall of ancient bones springs up, blocking passage"
                         " %s\r\n", dirs[rev_dir[dir]]);
            send_to_room(buf, -EXIT(ch, dir)->to_room);
        }
    }
}

ASPELL(spell_summon_corpse)
{
    ObjData *corpse;

    if (!victim) victim = ch;

	if(IS_SET_AR(PLR_FLAGS(victim), PLR_HUNTED) || IS_SET_AR(PLR_FLAGS(ch), PLR_HUNTED) ) {
		send_to_char("Wait until the hunt is over.\r\n", ch);
		if(ch != victim)
			send_to_char("The corpse summon fails.\r\n", victim);
		return;
	}

    if (IS_NPC(victim) || !(corpse = find_player_corpse(victim))) {
        send_to_char("That target has no corpse to summon!\r\n", ch);
        return;
    }

    act("You contact the realms of death to draw forth $p!", TRUE, ch, corpse,
            victim, TO_CHAR);
    act("$n contacts the realms of death to draw forth $p!", TRUE, ch, corpse,
            victim, TO_ROOM);

    obj_from_room(corpse);
    obj_to_room(corpse, victim->in_room);
}

ASPELL(spell_fletch)
{
    ObjData *quiver;
    ObjData *arrow;
    int c;

    quiver = read_perfect_object(892, VIRTUAL);
    if (!quiver) {
        sendChar(ch, "Tell an imm that this spell is broken!\r\n");
        return;
    }

    obj_to_char(quiver, ch);

    for (c = number(30, 50); c > 0; c--) {
        arrow = read_perfect_object(893, VIRTUAL);
        if (!arrow) {
            sendChar(ch, "Tell an imm that this spell is broken!\r\n");
            return;
        }
        obj_to_obj(arrow, quiver);
    }
    act("$n creates $p.", FALSE, ch, quiver, 0, TO_ROOM);
    act("You create $p.", FALSE, ch, quiver, 0, TO_CHAR);
}

static int called_steeds[NUM_CLASSES] = {
 /* Mu    Cl    Th    Wa    Ra    As    Sl    Sk    Dk    Sd    Nm */
     0,    0,    0,    0,    0,    0,    0,  290,  295,    0,    0
};

ASPELL(spell_call_steed)
{
    int mobnum = called_steeds[GET_CLASS(ch)];
    CharData *steed;
    char buf[40] = "";
    char *tmp = buf;
    CharData *i;

    if (IS_NPC(ch)) return;

    if (mobnum == 0) {
        sendChar(ch, "Your class cannot call a steed.\r\n");
        return;
    }

    // If the player already has a steed, we need not create another.  We simply
    // summon the steed!
    sprintf(buf, "steed%s", GET_NAME(ch));

    for(i = character_list; i; i = i->next) {
        if(isname(tmp, i->player.name) && IS_NPC(i) && IS_SET_AR(MOB_FLAGS(i), MOB_MOUNT) && i->in_room != NOWHERE) {
            if(i->rider && i->in_room == i->rider->in_room) {
                sendChar( ch, "You don't have the power to summon %s and its rider.\r\n", GET_NAME(i));
                return;
            }
            
            act("$n charges off into the distance.", TRUE, i, 0, 0, TO_ROOM);
            char_from_room(i);
            char_to_room(i, ch->in_room);
            act("$n charges into view!", TRUE, i, 0, 0, TO_ROOM);
            act("$n has summoned you!", FALSE, ch, 0, i, TO_VICT);
            look_at_room(i, 0);
            
            // Let's reimburse them something which is about their mana cost.
            sendChar(ch, "You regain some spent mana.\r\n");
            GET_MANA(ch) = MIN(GET_MAX_MANA(ch), GET_MANA(ch) + mag_manacost(ch, SPELL_CALL_STEED, 0));

            return;
        }
    }
    
    /* The more advanced you are, the better a steed you can call */
    if (GET_SKILL(ch, SPELL_CALL_STEED) > 60) {
        mobnum += (GET_SKILL(ch, SPELL_CALL_STEED) - 60) / 10;
    }

    steed = read_mobile(mobnum, VIRTUAL);

    if (!steed) {
        sendChar(ch, "This is broken!  Tell an immortal!\r\n");
        return;
    }

    GET_EXP(steed) = 1;
    GET_GOLD(steed) = 0;
    char_to_room(steed, ch->in_room);
    IS_CARRYING_W(steed) = 0;
    IS_CARRYING_N(steed) = 0;
    
    sprintf(buf, "%s steed%s", steed->player.name, GET_NAME(ch));
    steed->player.name = str_dup(buf);

    act("$N paws the earth beside you, eager for battle.", FALSE, ch, 0, steed,
            TO_CHAR);
    act("$N paws the earth beside $n, eager for battle.", FALSE, ch, 0, steed,
            TO_ROOM);
}

char *spells[TOP_SPELL_DEFINE + 2] =
{
  "!NO SPELL!",			/* 0 - reserved */

  /* SPELLS */

  "armor",			/* 1 */
  "teleport",
  "bless",
  "blindness",
  "burning hands",
  "call lightning",
  "charm person",
  "chill touch",
  "clone",
  "color spray",		/* 10 */
  "control weather",
  "create food",
  "create water",
  "cure blind",
  "cure critic",
  "cure light",
  "curse",
  "detect alignment",
  "detect invisibility",
  "detect magic",		/* 20 */
  "detect poison",
  "dispel evil",
  "earthquake",
  "enchant weapon",
  "life drain",
  "fireball",
  "harm",
  "heal",
  "invisibility",
  "lightning bolt",		/* 30 */
  "locate object",
  "magic missile",
  "poison",
  "protection from evil",
  "remove curse",
  "sanctuary",
  "shocking grasp",
  "sleep",
  "strength",
  "summon",			/* 40 */
  "ventriloquate",
  "word of recall",
  "remove poison",
  "sense life",
  "animate dead",
  "dispel good",
  "group armor",
  "group heal",
  "group recall",
  "infravision",		/* 50 */
  "shadow walk",
  "fly",
  "briarwood skin",
  "awaken",
  "regenerate",
  "ice storm",
  "meteor swarm",
  "portal",
  "cause wound",
  "maledict",			/* 60 */
  "cause critic",
  "cure serious",
  "calm",
  "chain lightning",
  "banish",					/* 65 */
  "demon fire",
  "flame strike",
  "protection from good",
  "shield",
  "chaos armor",			/* 70 */
  "holy armor",
  "create spring",
  "minor creation",
  "sand storm",
  "silence",				/* 75 */
  "paralyze",
  "haste",
  "slow",
  "shriek",
  "monster summon",			/* 80 */
  "remove paralysis",
  "conjure elemental",
  "gate",              /* Changed name from gate minor to gate - Vex. */
  "old gate major",    /* Changed name from gate major to old gate major. */
  "forget",	               /* 85 */
  "group sanctuary",
  "refresh",
  "true sight",
  "ball of light",
  "fear",					/* 90 */
  "fleet foot",
  "hands of wind",
  "plague",
  "cure plague",
  "relocate",					/* 95 */
  "group haste",
  "blur",
  "revive",
  "change alignment",
  "call of the wild",               			/* 100 */
  "doom bolt",
  "wrath of the ancients",
  "prayer of life",
  "holy word",
  "unholy word",	/* 105 */
  "black dart",
  "black breath",
  "death touch",
  "eyes of the dead",
  "righteous vision",	/* 110 */
  "ward",
  "group ward",
  "shadow vision",
  "shadow blades",
  "shadow sphere",      /* 115 */
  "group shadow vision",
  "identify",
  "group invisibility",
  "group fly",
  "airsphere",		/* 120 */
  "immunity to fire",
  "immunity to dryness",
  "immunity to cold",
  "web",
  "blink",	/* 125 */
  "pestilence",
  "entangle",
  "feeblemind",
  "flame blade",
  "nexus",	/* 130 */
  "assistant",
  "sagacity",
  "brilliance",
  "cleanse",
  "fortify",	/* 135 */
  "dishearten",
  "sacrifice",
  "pulse heal",
  "pulse gain",
  "dance of shadows",	/* 140 */
  "dance of dreams",
  "dance of mists",
  "crusade",
  "!UNUSED!",
  "aura of apocalypse",	/* 145 */
  "divine mission",
  "forest lore",
  "swarm",
  "wall of fire",
  "tremor",	/* 150 */
  "tsunami",
  "typhoon",
  "terror",
  "fast learning",
  "dispel magic",	/* 155 */
  "consume corpse",
  "explode corpse",
  "bone wall",
  "create warding",
  "wraithform",         /* 160 */
  "noxious skin",
  "disease",
  "summon corpse",
  "embalm",
  "charm corpse",      /* 165 */
  "resist poison",
  "age",
  "energy drain",
  "soul pierce",
  "debilitate",         /* 170 */
  "fountain of youth",
  "call steed",
  "reflection",
  "fletch",
  "entomb corpse",	/* 175 */
  "call to corpse",
  "quicken",
  "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 180 */

  /* SKILLS */

  "backstab",			/* 181 */
  "bash",
  "hide",
  "kick",
  "pick lock",
  "punch",
  "rescue",
  "sneak",
  "steal",
  "track",			/* 190 */
  "disarm",
  "second attack",
  "third attack",
  "scan",
  "lay on hands",			/* 195 */
  "fists",
  "throw",
  "shoot",
  "knock",
  "trip",			/* 200 */
  "blinding strike",
  "hamstring",
  "enhanced damage",
  "retreat",
  "turn undead",	/* 205 */
  "butcher",
  "dodge",
  "trap",
  "search for traps",
  "palm",			/* 210 */
  "find weakness",
  "skin*",
  "feign death",
  "art of the dragon",
  "aspect of the snake",	/* 215 */
  "art of the tiger",
  "aspect of the crane",
  "circle",
  "dust",
  "stalk",				/* 220 */
  "aspect of the wind",
  "envenom",
  "parry",
  "sweep",
  "doorbash",           /* 225 */
  "penumbrae",
  "pickpocket*",
  "appraise*",
  "delusion",
  "cutpurse*",	/* 230 */
  "cower",
  "danger sense*",
  "thief sense*",
  "assassination",
  "dirty tactics",
  "aggressive",
  "shadow dance",
  "gore",
  "breathe",
  "berserk",	/* 240 */
  "aspect of the monkey",
  "aspect of the flower",
  "guard",
  "heightened senses",
  "riposte",	/* 245 */
  "blackjack",
  "ambush",
  "shadow step",
  "shadow mist",
  "gut",	/* 250 */
  "brain",
  "cut throat", /* 252 */
  "convert",
  "familiarize",
  "escape",	/* 255 */
  "dance of death",
  "shadowbox",
  "fence",
  "mug",
  "spy",	/* 260 */
  "distract",
  "retarget",
  "devour",
  "block",
  "shield bash",	/* 265 */
  "weapon mastery",
  "redoubt",
  "invigorate",
  "steadfastness",
  "scout",	/* 270 */
  "bullseye",
  "expose",
  "camp",
  "poultice",
  "feed",	/* 275 */
  "calm",
  "mist",
  "sting",
  "battlecry",
  "warcry",	/* 280 */
  "stance",
  "powerstrike",
  "focus",
  "devotion",
  "fervor",	/* 285 */
  "false trail",
  "enhanced stealth",
  "shadow jump",
  "evasion",
  "critical hit",	/* 290 */
  "adrenaline",
  "befriend",
  "charge",
  "flashbang",
  "instant poison",	/* 295 */
  "!UNUSED!",
  "!UNUSED!", 
  "!UNUSED!", 
  "!UNUSED!",
  "!UNUSED!",           /* 300 */

  /* OBJECT SPELLS AND NPC SPELLS/SKILLS */

  "!UNUSED",			/* 301 */
  "fire breath",
  "gas breath",
  "frost breath",
  "acid breath",
  "lightning breath",
  "youthen",
  "knowledge",
  "recharge", "hippocratic oath",		/* 310 */
  "iniquity", "guardian angel", "shadow form", "favored soul", "dire blows",	/* 315 */
  "mend", "potency", "commanding shout", "deterrence", "paranoia",	/* 320 */
  "confusion", "sacrifice", "METAMORPHOSIS", "crusade", "adumbration",	/* 325 */
  "phaseshift", "combat buff !ERROR!", "sagacity2", "emaciated", "emaciated hit",	/* 330 */
  "emaciated mana", "maledict2", "show damage", "vivify", "rage",	/* 335 */
  "unimpeded asault", "defender health", "zombie reward", "!UNUSED!", "!UNUSED!",	/* 340 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 345 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 350 */

  /* SONGS */

  "minor refreshment",
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 355 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 360 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 365 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 370 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 375 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 380 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 385 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 390 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 395 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",		/* 399 */
   "\n"   /* The end */
};

const char *spell_wear_off_msg[NUM_SPELLS] = {
  "RESERVED SPELLS.C",		/* 0 */
  "You feel less protected.",	/* 1 */
  "!Teleport!",
  "You feel less righteous.",
  "You feel a cloak of blindness disolve.",
  "!Burning Hands!",		/* 5 */
  "!Call Lightning",
  "You feel more self-confident.",
  "You feel your strength return.",
  "!Clone!",
  "!Color Spray!",		/* 10 */
  "!Control Weather!",
  "!Create Food!",
  "!Create Water!",
  "!Cure Blind!",
  "You feel a lot better!!",		/* 15 */
  "!Cure Light!",
  "You feel more optimistic.",
  "You feel less aware.",
  "Your eyes stop tingling.",
  "The detect magic wears off.",/* 20 */
  "The detect poison wears off.",
  "!Dispel Evil!",
  "!Earthquake!",
  "!Enchant Weapon!",
  "!Life Drain!",		/* 25 */
  "!Fireball!",
  "",
  "A warm feeling floods your body as your heal prayer reaches fruition.",
  "You feel yourself exposed.",
  "!Lightning Bolt!",		/* 30 */
  "!Locate object!",
  "!Magic Missile!",
  "You feel less sick.",
  "You feel less protected.",
  "!Remove Curse!",		/* 35 */
  "The white aura around your body fades.",
  "!Shocking Grasp!",
  "You feel less tired.",
  "You feel weaker.",
  "!Summon!",			/* 40 */
  "!Ventriloquate!",
  "!Word of Recall!",
  "!Remove Poison!",
  "You feel less aware of your surroundings.",
  "!Animate Dead!",		/* 45 */
  "!Dispel Good!",
  "!Group Armor!",
  "!Group Heal!",
  "!Group Recall!",
  "Your night vision seems to fade.",	/* 50 */
  "Your feet seem less boyant.",
  "You float gently back to earth.",
  "Your skin loses its woody texture and you shed your thorns.",
  "You lose your spectral awareness.",
  "Your body is no longer regenerating.",   /* 55 */
  "!Ice Storm!",
  "!Meteor Swarm!",
  "!Portal!",
  "!Cause Light!",
  "",                        /* 60 */
  "!Cause Critic!",
  "!Cure Serious!",
  "!Calm!",
  "!Chain Lightning!",
  "",
  "!Demon Fire!",
  "!Flame Strike!",
  "You feel less protected.",
  "Your protective shield dissolves.",
  "The red glow around your body dissipates.", /* 70 */
  "The holy glow around your body dissipates.",
  "!Create Spring!",
  "!Minor Creation!",
  "!Sand Storm!",
  "You feel more talkative.",
  "You are no longer paralyzed.",
  "Your movements slow down to normal.",
  "Your movements speed up to normal.",
  "Your ears POP!  Ahh.. much better.",
  "!Monster Summon!",                           /* 80 */
  "!Remove Paralysis!",
  "!Conjure Elemental!",
  "!Gate Minor Demon!",
  "!Gate Major Demon!",
  "!Forget!",
  "!Group Sanctuary!",
  "!Refresh!",
  "Your vision returns to normal.",
  "!Ball of Light!",
  "!Fear!",                                      /* 90 */
  "Your feet don't feel as light any more.",
  "!Hands of wind!",
  "!Plague!",
  "You feel cleansed.",
  "!Relocate!",					/* 95 */
  "!Group Haste!",
  "You feel much more in focus now.",
  "!Revive!",
  "!Change Align!",
  "!Call of the Wild!",				/* 100 */
  "!Doom Bolt!",
  "!Wrath of the Ancients!",
  "!Prayer of Life!",
  "!Holy Word!",
  "!Unholy Word!",				/* 105 */
  "!Black Dart!",
  "!Black Breath!",
  "You don't feel so old.",
  "!Eyes of the Dead!",
  "!Righteous Vision!",				/* 110 */
  "The ward protecting you fades.",
  "!Group Ward!",
  "!Shadow Vision!",
  "!Shadow Blades!",
  "The globe of darkness around you dissipates.", /* 115 */
  "!Group shadow vision!",
  "!Identify!",
  "!Group invisibility!",
  "!Group fly!",
  "Your air bubble bursts.",                       /* 120 */
  "You feel unpleasantly warm.",
  "You feel your skin dry out.",
  "You feel a chill in the air.",
  "The webs around your body crumble to dust, and blow away.",
  "You feel a bit more stable.",
  "!Pestilence!",
  "You are less trapped by roots and vines now.",
  "Suddenly a flash of understanding comes over you.",
  "Your fires go out.",
  "!Nexus!",                       /* 130 */
  "You feel very alone again.",
  "Your mind starts to slip.",
  "You don't feel nearly as smart.",
  "!Cleanse!",
  "You are no longer fortified.",       /* 135 */
  "You feel better about the world.",
  "Your sacrifice no longer cripples you.",
  "You feel colder as your body's healing slows.",
  "You feel colder as your mind's healing slows.",
  "Your dance of shadows has ended.",   /* 140 */
  "You manage to forget the hypnotic dance you saw.",
  "You no longer feel so confused by the strange dance you saw.",
  "Your crusade has ended.",
  "",
  "The aura of terror surrounding you recedes.", /* 145 */
  "The glory of your divine mission fades away.",
  "Your forest lore no longer gives you insight into nature.",
  "Your insect bites hurt much less now.",
  "!wall of fire!",
  "!tremor!",                            /* 150 */
  "!tsunami!",
  "!typhoon!",
  "!terror!",
  "Your experience no longer teaches you so well.",
  "!dispel magic!",     /* 155 */
  "",
  "!explode corpse!",
  "!bone wall!",
  "!create warding!",
  "Your body becomes fully corporeal once more.",  /* 160 */
  "The blisters on your skin heal up and vanish.",
  "The disease has run its course.",
  "!summon corpse!",
  "!embalm!",
  "!charm corpse!",                    /* 165 */
  "You are once again vulnerable to poisons.",
  "You feel the strength of youth once more.",
  "You are more able to focus your mind.",
  "!soul pierce!",
  "A great weight lifts from your bones.",
  "!fountain of youth!",
  "!call steed!",
  "The air around you stops shimmering.",
  "!fletch!",
  "!Entomb Corpse!",
  "Fate's icy maw loosens its grip on you.",
};

/* Weapon attack texts */
const AttackHitType attack_hit_text[NUM_WEAPON_TYPES] =
{
  {"hit", "hits"},		/* 0 */
  {"sting", "stings"},
  {"whip", "whips"},
  {"slash", "slashes"},
  {"bite", "bites"},
  {"bludgeon", "bludgeons"},	/* 5 */
  {"crush", "crushes"},
  {"pound", "pounds"},
  {"claw", "claws"},
  {"maul", "mauls"},
  {"thrash", "thrashes"},	/* 10 */
  {"pierce", "pierces"},
  {"blast", "blasts"},
  {"punch", "punches"},
  {"stab", "stabs"},
  {"strangle", "strangles"},    /* 15 */
  {"tear", "tears"},
  {"squeeze", "squeezes"},
  {"stomp", "stomps"},
  {"drain", "drains"},
  {"bite", "bites"},		/* 20 */
  {"burn", "burns"},
  {"impale", "impales"},
  {"kick", "kicks"}
};
