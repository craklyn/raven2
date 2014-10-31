/* ************************************************************************
*   File: mobact.c                                      Part of CircleMUD *
*  Usage: Functions for generating intelligent (?) behavior in mobiles    *
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
#include "actions/interpreter.h"
#include "general/handler.h"
#include "general/class.h"
#include "magic/spells.h"
#include "specials/seek.h"
#include "util/weather.h"
#include "magic/backstab.h"
#include "specials/mobact.h"
#include "magic/sing.h"
#include "magic/missile.h"
#include "magic/skills.h"
#include "magic/cut_throat.h"
#include "actions/fight.h"
#include "actions/act.h"          /* ACMDs located within the act*.c files */
#include "magic/stun.h"           /* For do_hamstring function */
#include "magic/flashbang.h"      /* For flashbang_explode function */
#include "actions/combat.h"

#define LOG_HEAL_ENABLED 0

#if LOG_HEAL_ENABLED
#    define LOG_HEAL(ch) mudlog(NRM, LVL_IMMORT, TRUE, "HEAL: %s is healing self", GET_NAME(ch));
#else
#    define LOG_HEAL(ch)
#endif

/* % chance a mob will love a victim instead of attacking them */
static int mob_love = 0;


void set_love(int love)
{
    mob_love = love;
}

ACMD(do_valentine)
{
    int love;

    one_argument(argument, buf);
    love = atoi(buf);

    if (love > 50) {
        sendChar(ch, "Don't you think that's a bit TOO much love?\r\n");
        return;
    }

    if (love < 0) {
        sendChar(ch, "Oh, you cold, cold person you!\r\n");
        return;
    }

    set_love(love);
    sendChar(ch, "You have set the world's love to %d%%.\r\n", love);
}

void mob_attack(CharData *mob, CharData *vict) {
	int can_backstab = 0;
	int percent_HPs = (100 * GET_HIT(vict))/GET_MAX_HIT(vict);

	//Sanity check.
	if ( mobCanBackstab(mob) )
		can_backstab = 1;

	
	if (percentSuccess(mob_love)) {
		act("$N gives you a really big hug!", TRUE, vict, 0, mob, TO_CHAR);
		act("$N gives $n a really big hug!", TRUE, vict, 0, mob, TO_ROOM);
	} else if ( mobCanBackstab(mob) )
	{
		// We know the mob has a weapon and can backstab.  If they're below
		// these levels, they have nothing to do except stab, so do it.
		if ((GET_CLASS(mob) == CLASS_SHADOW_DANCER)                     ||
		    (GET_CLASS(mob) == CLASS_ASSASSIN && (GET_LEVEL(mob) < 30)) || 
		    (GET_CLASS(mob) == CLASS_THIEF    && (GET_LEVEL(mob) < 30)) )
		{
			do_backstab(mob, vict->player.name, 0, 0);
			return;
		}
		// 25% chance to cutthroat some targets, 50% to cutthroat a caster.
		if ( GET_CLASS(mob) == CLASS_THIEF && 
			((GET_CLASS(vict) == CLASS_MAGIC_USER ||
			 GET_CLASS(vict) == CLASS_CLERIC     ||
			 GET_CLASS(vict) == CLASS_NECROMANCER ) + number(0, 3)) > 2 ) {
			 do_cut_throat(mob, vict->player.name, 0, 0);
			 return;
		}
		// more likely to hamstring hurt targets.
		if ( GET_CLASS(mob) == CLASS_ASSASSIN && 
			((percent_HPs < 80) + (percent_HPs > 20) + number(0, 6)) > 5 ) {
			 do_hamstring(mob, vict->player.name, 0, 0);
			 return;
		}
		do_backstab(mob, vict->player.name, 0, 0);
		return;
	} else {
	mob_talk(mob, vict, 4);
	hit(mob, vict, TYPE_UNDEFINED );
    }
}

//-------------------------------------------------------------------------
//
// This function determines if mob, by virtue of its behaviour should hit
// victim. It returns 1 if it should, 0 if it should not.
//
int
mobAttackVictim( CharData* mob,
                 CharData* victim )
{
  if( !IS_NPC(mob) )
  {
    mudlog(NRM, LVL_IMMORT, TRUE, "SYSERR: Non-npc %s passed as mob to mobAttackVictim",
                                                    GET_NAME(mob));
    return 0;
  }

  /* Befriended mobs aren't feeling aggressive right now */
  if (affected_by_spell(mob, SKILL_BEFRIEND)) return 0;

  // Quick validation of the rooms.
  //
  if( mob->in_room < 0 || victim->in_room < 0 )
  {
    mudlog(NRM, LVL_IMMORT, TRUE, "SYSERR: mobAttackVict %s in room %d, %s in room %d",
              GET_NAME(mob),    mob->in_room,
              GET_NAME(victim), victim->in_room );
    return 0;
  }

  if (IS_NPC(victim)) return 0;

  if (FIGHTING(mob)) return 0;

  if (!AWAKE(mob)) return 0;

  if (!IS_NPC(victim) && PRF_FLAGGED(victim, PRF_NOHASSLE)) return 0;

  if (!CAN_SEE(mob, victim)) return 0;

  if (IS_AFFECTED(mob, AFF_PARALYZE)) return 0;

#ifdef WHY_IS_THIS_HERE
  if (MOB_FLAGGED(mob, MOB_WIMPY) && AWAKE(victim)) return 0;
#endif

  if (IS_SET_AR(ROOM_FLAGS(victim->in_room), ROOM_PEACEFUL) ) return 0;

  if (MOB_FLAGGED(mob,MOB_PREDATOR)) return 1;

  if ((MOB_IS_AGGR_EVIL(mob) && IS_EVIL(victim) )   ||
      (MOB_IS_AGGR_NEUT(mob) && IS_NEUTRAL(victim)) ||
      (MOB_IS_AGGR_GOOD(mob) && IS_GOOD(victim)) )
    return 1;

  // The following return statement will return true ONLY if the mob is
  // flagged as a generally aggressive mob without any tendencies towards
  // a particular alignment.
  //
  return (MOB_IS_AGGR(mob) && 
        !(MOB_IS_AGGR_EVIL(mob) ||
          MOB_IS_AGGR_NEUT(mob) ||
          MOB_IS_AGGR_GOOD(mob) ));

}


//-------------------------------------------------------------------------
//
// This functions checks if ch should smack something in the room. If they
// should, then it starts the fight.
//
int aggroCheck( CharData* ch )
{
    bool found= FALSE;
    CharData *vict;
    int aggrod = 0;


    // Disable attacks of switched mobs.
    if (ch->desc != NULL) 
        return FALSE;

    // Befriended mobs are just feeling too darn good.
    if (affected_by_spell(ch, SKILL_BEFRIEND)) 
        return FALSE;

    // Predators are smart enough to know when to attack.
    if (MOB_FLAGGED( ch, MOB_PREDATOR)) {
        CharData* targetCh = NULL;
        double    minLevel = 9999.0;

        strcpy( buf, "PREDATOR: " );

        for (vict = world[ch->in_room].people; vict ; vict=vict->next_in_room) {
            double test = (vict->player.level) * 10.0 + vict->points.hit;
            test *= 0.5 + (-100.0 + vict->points.armor) / (-200.0);
            if (mobAttackVictim( ch, vict)) {
                if (test < minLevel) {
                    minLevel = test;
                    targetCh = vict;
                }
            }   
        }

        if (minLevel < 9999.0) {
            aggrod = 1;
            mob_attack(ch, targetCh);
        }
    }

  // Do the standard AGGR tests.
  //
  else if(( MOB_FLAGGED(ch, MOB_AGGRESSIVE)   ||
            MOB_FLAGGED(ch, MOB_AGGR_EVIL)    ||
            MOB_FLAGGED(ch, MOB_AGGR_NEUTRAL) ||
            MOB_FLAGGED(ch, MOB_AGGR_GOOD)    ))
  {
    for( vict = world[ch->in_room].people; vict && !found;
         vict = vict->next_in_room)
    {
      if( !mobAttackVictim(ch, vict) ) continue;

      else
      {
        aggrod = 1;
        mob_attack(ch, vict);
        found = TRUE;
      }
    }
  }
  return aggrod;
}

void mobile_activity(void) {
    register struct char_data *ch, *next_ch, *vict;
    struct obj_data *obj, *best_obj;
    int door, found, max, percent_HPs, i, healed;
    DescriptorData *desc;
    /* This next variable is used to make the conditional test for smart */
    /* helper mobs a bit less confusing - Vex.                           */
    int fighting_enemy_conjured;

    memory_rec *names;

    extern int no_specials;

    for (ch = character_list; ch; ch = next_ch) {
        next_ch = ch->next;

        if (!IS_MOB(ch))
            continue; /* Just incase */

        if (IS_AFFECTED(ch, AFF_PARALYZE))
            continue;

        /* Befriended mobs will assist you, but nothing else! */
        if (affected_by_spell(ch, SKILL_BEFRIEND)) {
            if (!FIGHTING(ch)) {
                do_stand(ch, "", 0, 0); // Make sure they're always upright
                if (FIGHTING(ch->master) && ch->master->in_room == ch->in_room) {
                    act("$n assists you!", TRUE, ch, 0, ch->master, TO_VICT);
                    act("$n assists $N!", TRUE, ch, 0, ch->master, TO_NOTVICT);
                    set_fighting(ch, FIGHTING(ch->master));
                }
            }
            continue;
        }

        if (ch->desc != NULL)
            continue; /* Switched mob only do what the imm wants */

        if (STUNNED(ch)) {
            if (!FIGHTING(ch) && !ch->master) {
                ch->player.stunned = 0;
            }
            else
                continue;
        }

        if (OFF_BALANCE(ch)) {
            if (!FIGHTING(ch))
                OFF_BALANCE(ch) = 0;
        }

        if (FIGHTING(ch)) {
            if (FIGHTING(ch) == SEEKING(ch))
                SEEK_STATUS(ch) = SEEK_FIGHTING;
            else
                SEEK_STATUS(ch) = SEEK_BUSY;
        } else {
            if (SEEK_STATUS(ch) == SEEK_FIGHTING || SEEK_STATUS(ch) == SEEK_BUSY)
                SEEK_STATUS(ch) = SEEK_IDLE;
        }

        if (IS_NPC(ch) && !IS_AFFECTED(ch, AFF_CHARM) && !FIGHTING(ch)) {
            if (GET_POS(ch) != GET_LOAD_POS(ch) && GET_POS(ch) != GET_DEFAULT_POS(ch)) {
                if (!AWAKE(ch))
                    do_wake(ch, "", 0, 0);
                switch (GET_DEFAULT_POS(ch)) {
                    case POS_STANDING:
                        do_stand(ch, "", 0, 0);
                        break;
                    case POS_SITTING:
                        do_sit(ch, "", 0, 0);
                        break;
                    case POS_RESTING:
                        do_rest(ch, "", 0, 0);
                        break;
                    case POS_SLEEPING:
                        do_sleep(ch, "", 0, 0);
                        break;
                    default:
                        break;
                }/* switch */
            }/* if */
        }/* not charmed */

        if (!IS_MOB(ch) || FIGHTING(ch) || !AWAKE(ch)) /* we can perhaps make pcs do stuff then... */
            continue;

        /* If seeking someone, go get em... */
        if (SEEKING(ch)) {
            seek_seekprey(ch);
        } else {
            seek_newtarget(ch);
        }

        if (SEEKING(ch)) continue;

        /* Examine call for special procedure */
        if (MOB_FLAGGED(ch, MOB_SPEC) && !no_specials) {
            if ((GET_MOB_RNUM(ch) >= 0) && mob_index[GET_MOB_RNUM(ch)].func == NULL) {
                mlog("%s (#%d): Attempting to call non-existing mob func",
                        GET_NAME(ch), GET_MOB_VNUM(ch));
                //128	REMOVE_BIT(MOB_FLAGS(ch), MOB_SPEC);
                REMOVE_BIT_AR(MOB_FLAGS(ch), MOB_SPEC);
            } else {
                if ((mob_index[GET_MOB_RNUM(ch)].func) (ch, ch, 0, ""))
                    continue; /* go to next char */
            }
        }
        /* Scavenger (picking up objects) */
        if (MOB_FLAGGED(ch, MOB_SCAVENGER) && !FIGHTING(ch) && AWAKE(ch))
            if (world[ch->in_room].contents && !number(0, 10)) {
                max = 1;
                best_obj = NULL;
                for (obj = world[ch->in_room].contents; obj; obj = obj->next_content)
                    if (CAN_GET_OBJ(ch, obj) && GET_OBJ_COST(obj) > max) {
                        best_obj = obj;
                        max = GET_OBJ_COST(obj);
                    }
                if (best_obj != NULL) {
                    obj_from_room(best_obj);
                    obj_to_char(best_obj, ch);
                    act("$n gets $p.", FALSE, ch, best_obj, 0, TO_ROOM);
                }
            }

        door = number(0, 18);

        if (GET_POS(ch) == POS_STANDING && door < NUM_OF_DIRS)
            if (CAN_GO(ch, door)) {
                if (!IS_SET_AR(MOB_FLAGS(ch), MOB_SENTINEL))
                    if (!IS_SET_AR(ROOM_FLAGS(EXIT(ch, door)->to_room), ROOM_NOMOB))
                        if (!IS_SET_AR(ROOM_FLAGS(EXIT(ch, door)->to_room), ROOM_DEATH)) {
                            if (IS_SET_AR(MOB_FLAGS(ch), MOB_STAY_ZONE)) {
                                if (world[EXIT(ch, door)->to_room].zone == world[ch->in_room].zone)
                                    if (!perform_move(ch, door, 0))
                                        continue;
                                    else
                                        door = 0; /* Do nothing DEBUGGING */
                            } else if (!perform_move(ch, door, 0))
                                continue;
                        }
            } else if (IS_SET_AR(MOB_FLAGS(ch), MOB_TELEPORTS)) {
                if (IS_SET_AR(MOB_FLAGS(ch), MOB_STAY_ZONE)) {
                } else {
                    /* XXX todo */
                }
            }

        /* Aggressive Mobs */
        if (aggroCheck(ch)) return;

        /* Mob Super Agressive */
        /*    if( MOB_FLAGGED(ch, MOB_SUPERAGG) ){
        }
         */
        /* Mob Memory Bug BugID0001 */

        if (affected_by_spell(ch, SPELL_BLINDNESS))
            if ((GET_CLASS(ch) == CLASS_CLERIC && GET_LEVEL(ch) >= 11) ||
                    (GET_CLASS(ch) == CLASS_SOLAMNIC_KNIGHT && GET_LEVEL(ch) >= 33)) {
                cast_spell(ch, ch, NULL, SPELL_CURE_BLIND);
                continue;
            }

        if (affected_by_spell(ch, SKILL_DUST) &&
                GET_CLASS(ch) == CLASS_CLERIC &&
                GET_LEVEL(ch) >= 41) {
            cast_spell(ch, ch, NULL, SPELL_CLEANSE);
            continue;
        }

        if( IS_SHOU_LIN(ch) &&
            (GET_ASPECT(ch) == ASPECT_NONE || !number(0, 30)) ) {
            choose_aspect(ch);
        }

        if (MOB_FLAGGED(ch, MOB_MEMORY) && MEMORY(ch)) {
            found = FALSE;
            for (vict = world[ch->in_room].people; vict && !found; vict = vict->next_in_room) {
                if (IS_NPC(vict) || !CAN_SEE(ch, vict) || PRF_FLAGGED(vict, PRF_NOHASSLE) || GET_POS(vict) != POS_STANDING || ROOM_FLAGGED(IN_ROOM(ch), ROOM_PEACEFUL))
                    continue;
                for (names = MEMORY(ch); names && !found; names = names->next)
                    if (names->id == GET_IDNUM(vict)) {
                        found = TRUE;
                        act("'Hey!  You're the fiend that attacked me!!!', exclaims $n.",
                                FALSE, ch, 0, 0, TO_ROOM);
                        hit(ch, vict, TYPE_UNDEFINED);
                    }
            }
            if (found) continue;
        }
        /* Helper Mobs */
        if (MOB_FLAGGED(ch, MOB_HELPER)) {
            found = healed = FALSE;
            fighting_enemy_conjured = FALSE;
            for (vict = world[ch->in_room].people; vict && !found; vict = vict->next_in_room) {
                /* a CLERIC mob will look for someone to heal about now */
                if (GET_CLASS(ch) == CLASS_CLERIC && IS_NPC(vict) &&
                        !IS_SET_AR(MOB_FLAGS(vict), MOB_CONJURED) && !healed &&
                        GET_HIT(vict) < GET_MAX_HIT(vict) && vict != ch) {
                    healed = TRUE;
                    /* select the best healing spell available */
                    if (GET_LEVEL(ch) >= 45) {
                        cast_spell(ch, vict, NULL, SPELL_REVIVE);
                    } else if (GET_LEVEL(ch) >= 20) {
                        cast_spell(ch, vict, NULL, SPELL_HEAL);
                    } else if (GET_LEVEL(ch) >= 10) {
                        cast_spell(ch, vict, NULL, SPELL_CURE_CRITIC);
                    }
                }
                /* Vex - made mobs smarter in choosing who they help out.            */
                /* This statement translates too : See if what we are thinking about */
                /* helping is a mob that has'nt been summoned. If so, then if they   */
                /* are fighting a mob that has been summoned then treat the          */
                /* conjured mob like a player and hit them.                          */
                if (IS_NPC(vict) && (!IS_SET_AR(MOB_FLAGS(vict), MOB_CONJURED)) &&
                        FIGHTING(vict) && IS_NPC(FIGHTING(vict)) &&
                        IS_SET_AR(MOB_FLAGS(FIGHTING(vict)), MOB_CONJURED))
                    fighting_enemy_conjured = TRUE;

                if (IS_NPC(vict) && vict != ch && FIGHTING(vict) && CAN_SEE(ch, FIGHTING(vict)) &&
                        ((!IS_NPC(FIGHTING(vict))) || fighting_enemy_conjured)) {
                    /* Vex - Febuary 18 1997
                     ** Made mobs backstab if they are able too
                     */

                    int class = GET_CLASS(ch);
                    int level = GET_LEVEL(ch);
                    int has_back_wpn;

                    act("$n jumps to the aid of $N!", FALSE, ch, 0, vict, TO_ROOM);

                    if (WIELDING(ch) &&
                            (GET_OBJ_VAL(ch->equipment[WEAR_WIELD], 3) == (TYPE_PIERCE - TYPE_HIT)))
                        has_back_wpn = TRUE;
                    else
                        has_back_wpn = FALSE;

                    if ((class == CLASS_ASSASSIN) && has_back_wpn)
                        do_circle(ch, FIGHTING(vict)->player.name, 0, 0);
                    else if ((class == CLASS_THIEF) && (level >= 5) && has_back_wpn)
                        do_circle(ch, FIGHTING(vict)->player.name, 0, 0);
                    else
                        hit(ch, FIGHTING(vict), TYPE_UNDEFINED);
                    found = TRUE;
                }
            }
            if (found) continue;
        }
        /* Vex Febuary 17, 1997
         ** Just because nobody is around does'nt mean there is nothing for us
         ** to do.
         */

        percent_HPs = (100 * GET_HIT(ch)) / GET_MAX_HIT(ch);
        /* If we are hurt, lets see about healing our selves. */
        if (percent_HPs < 100) {
            int level = GET_LEVEL(ch);
            if (GET_CLASS(ch) == CLASS_CLERIC && !healed) {
                LOG_HEAL(ch);
                if (level >= 45)
                    cast_spell(ch, ch, NULL, SPELL_REVIVE);
                else if (level >= 20)
                    cast_spell(ch, ch, NULL, SPELL_HEAL);
                else if (level >= 10)
                    cast_spell(ch, ch, NULL, SPELL_CURE_CRITIC);
                else
                    cast_spell(ch, ch, NULL, SPELL_CURE_LIGHT);
                continue;
            } else if (GET_CLASS(ch) == CLASS_SOLAMNIC_KNIGHT) {
                LOG_HEAL(ch);
                if (level >= 45)
                    cast_spell(ch, ch, NULL, SPELL_PRAYER_OF_LIFE);
                else if (level >= 35)
                    cast_spell(ch, ch, NULL, SPELL_CURE_CRITIC);
                else if (level >= 25)
                    cast_spell(ch, ch, NULL, SPELL_CURE_SERIOUS);
                else if (level >= 10)
                    cast_spell(ch, ch, NULL, SPELL_CURE_LIGHT);
                continue;
            } else if (GET_CLASS(ch) == CLASS_SHOU_LIN) {
                LOG_HEAL(ch);
                do_hands(ch, ch->player.name, 0, 0);
                continue;
            }
        }
        /* Add new mobile actions here */

        // Grenader mob, called half the time or else too crazy.
        if (MOB_FLAGGED(ch, MOB_GRENADER) && number(0, 1)) {
            // Go through all people in the game, see if the people are in range to
            // have grenades thrown at them.
            for (desc = descriptor_list; desc; desc = desc->next) {
                if (!desc->connected && desc->character && desc->character != ch) {
                    vict = desc->character;
                    for (door = 0; door < NUM_OF_DIRS; door++)
                        if (EXIT(ch, door))
                            if (EXIT(ch, door)->to_room != NOWHERE)
                                if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) &&
                                        (EXIT(ch, door)->to_room == IN_ROOM(vict)) && CAN_SEE(ch, vict)
                                        && GET_LEVEL(vict) < LVL_IMMORT && percentSuccess(25)) {
                                    if (STUNNED(ch))
                                        break;
                                    act("$n mixes chemicals together in a pouch and lobs it overhead.", FALSE, ch, 0, 0, TO_ROOM);
                                    gen_grenade(ch, vict, door, MISSILE_TYPE_GRENADE);
                                    flashbang_explode(ch, vict);
                                    WAIT_STATE(ch, SET_STUN(1));
                                    continue;
                                }
                }
            }
        }

        if (IS_SET_AR(AFF_FLAGS(&mob_proto[ch->nr]), AFF_HIDE) &&
                !IS_SET_AR(AFF_FLAGS(ch), AFF_HIDE)) {
            do_hide(ch, "", 0, 0);
            continue;
        }

        /* 35+ assassins should envenom */
        if (WIELDING(ch) && GET_CLASS(ch) == CLASS_ASSASSIN && GET_LEVEL(ch) > 35 &&
                CAN_SEE_OBJ(ch, WIELDING(ch))) {
            /* but only if the weapon has a free slot */
            for (i = 0; i < MAX_OBJ_AFFECT; i++)
                if (WIELDING(ch)->affected[i].location == APPLY_NONE) break;
            if (i < MAX_OBJ_AFFECT) {
                ObjData *weapon = unequip_char(ch, WEAR_WIELD);
                obj_to_char(weapon, ch);
                do_envenom(ch, weapon->name, 0, 0);
                obj_from_char(weapon);
                equip_char(ch, weapon, WEAR_WIELD);
            }
        }

        /* Send charmed mobs with masters not present out to find their master */
        if (IS_NPC(ch) && IS_SET_AR(AFF_FLAGS(ch), AFF_CHARM)
                && GET_POS(ch) == POS_STANDING
                && ch->master && ch->master->in_room != ch->in_room)
            SEEKING(ch) = ch->master;

    } /* end for() */
} /* mobile_activity */



/* Mob Memory Routines */

/* make ch remember victim */
void remember(struct char_data * ch, struct char_data * victim)
{
  memory_rec *tmp;
  bool present = FALSE;

  if(pvpHoliday(victim))
      return;
  
  if (!IS_NPC(ch) || IS_NPC(victim))
    return;

  for (tmp = MEMORY(ch); tmp && !present; tmp = tmp->next)
    if (tmp->id == GET_IDNUM(victim))
      present = TRUE;

  if (!present) {
    CREATE(tmp, memory_rec, 1);
    tmp->next = MEMORY(ch);
    tmp->id = GET_IDNUM(victim);
    MEMORY(ch) = tmp;
  }
}


/* make ch forget victim */
void forget(struct char_data * ch, struct char_data * victim)
{
  memory_rec *curr, *prev;

  /* Let's see if we can get them to stop seeking too... Vex. */
  if (SEEKING(ch) && (SEEKING(ch) == victim))
  {
	mudlog(NRM, LVL_IMMORT, TRUE, "SEEK: %s forgetting about %s.", GET_NAME(ch), GET_NAME(SEEKING(ch)));
	SEEKING(ch) = NULL;
	SEEK_STATUS(ch) = SEEK_IDLE;
	SEEK_TARGETSTR(ch) = NULL;
  }
  if (!(curr = MEMORY(ch)))
    return;

  while (curr && curr->id != GET_IDNUM(victim)) {
    prev = curr;
    curr = curr->next;
  }

  if (!curr)
    return;			/* person wasn't there at all. */

  if (curr == MEMORY(ch))
    MEMORY(ch) = curr->next;
  else
    prev->next = curr->next;

  free(curr);
}


/* erase ch's memory */
void clearMemory(struct char_data * ch)
{
  memory_rec *curr, *next;

  curr = MEMORY(ch);

  while (curr) {
    next = curr->next;
    free(curr);
    curr = next;
  }

  MEMORY(ch) = NULL;
}


// Generic blocking mobs ala Mortius tweaked by Digger 11/09/00.
//
int
mob_block( CharData* ch, CharData* mob, int dir )
{
  char* desc = NULL;
  char  block_msg_char[256] = "BLOCK PROBLEM: REPORT THIS TO AN IMM";
  char  block_msg_room[256] = "BLOCK PROBLEM: REPORT THIS TOP AN IMM";
  char  block_dir[10] = "\0";
  char  direction[10] = "\0";

  bool  classMismatch = (GET_CLASS(ch) != GET_CLASS(mob));
  bool  raceMismatch  = (GET_RACE(ch)  != GET_RACE(mob));
  int   block = FALSE;

  sprintf( direction, "%s\r\n", dirs[dir] );

  // Get all the info required out of the extra desc for the room.
  // 
  if(( desc = find_exdesc( "block_msg_char",
                            world[ch->in_room].ex_description)) != NULL)
  {
    strcpy(block_msg_char, desc);
  }

  if(( desc = find_exdesc( "block_msg_room",
                            world[ch->in_room].ex_description)) != NULL)
  {
    strcpy(block_msg_room, desc);
  }

  if(( desc = find_exdesc( "block_dir",
                            world[ch->in_room].ex_description)) != NULL)
  {
    strcpy(block_dir, desc);
  }

  // If its not the same direction as blocking let them pass
  //
  if( !strcmp(block_dir, direction) == 0 ) return FALSE;

  if( MOB_FLAGGED(mob, MOB_GUARD_CLASS ) && classMismatch )
  {
    block = TRUE; 
  }

  if( MOB_FLAGGED( mob, MOB_GUARD_RACE ) && raceMismatch )
  {
    block = TRUE;
  }

  if( MOB_FLAGGED(mob, MOB_GUARD_BOTH) && ( classMismatch || raceMismatch ))
  {
    block = TRUE;
  }

  /* Imhotep: hackityhackityhackity */
  if (world[ch->in_room].number == 35702 && GET_MOB_VNUM(mob) == 35740) {
      if (IS_AFFECTED(ch, AFF_AIRSPHERE))
          block = FALSE;
      else
          block = TRUE;
  }

  if( block == TRUE )
  {
    act(block_msg_char, TRUE, ch, 0, mob, TO_CHAR);
    act(block_msg_room, TRUE, ch, 0, mob, TO_ROOM);
    return TRUE;
  }
  else
    return FALSE;
}

