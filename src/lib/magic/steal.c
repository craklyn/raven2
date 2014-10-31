/*
**++
**  RCSID:     $Id: steal.c,v 1.14 2003/10/23 01:30:54 raven Exp $
**
**  FACILITY:  RavenMUD
**
**  LEGAL MUMBO JUMBO:
**
**      This is based on code developed for DIKU and Circle MUDs.
**
**  MODULE DESCRIPTION:
**
**  AUTHORS:
**
**      Digger from RavenMUD
**      Vex from RavenMud
**  NOTES:
**
**      Use 132 column editing in here.
**
**--
*/

/*
** STANDARD U*IX INCLUDES
*/
#include "general/conf.h"
#include "general/sysdep.h"

#define THIS_SKILL                SKILL_STEAL

#define SKILL_ADVANCES_WITH_USE   TRUE
#define SKILL_ADVANCE_STRING      "You feel as if you could steal the Eye of the Serpent itself."

/*                                Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  XX  XX  XX  XX  XX  XX  XX  XX  XX  XX */
static int max_skill_lvls[] =   { 00, 00, 90, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 };
#define SKILL_MAX_LEARN           max_skill_lvls[ (int)GET_CLASS(ch) ]

#define DEX_AFFECTS               FALSE
#define INT_AFFECTS               TRUE
#define WIS_AFFECTS               FALSE

#define STUN_MIN                  1
#define STUN_MAX                  2

/*
** MUD SPECIFIC INCLUDES
*/
#include "general/db.h"
#include "general/structs.h"
#include "general/class.h"
#include "general/comm.h"
#include "general/handler.h"
#include "actions/interpreter.h"
#include "magic/skills.h"
#include "magic/spells.h"
#include "util/utils.h"
#include "util/weather.h"
#include "specials/special.h"
#include "actions/outlaw.h"
#include "specials/mobact.h"
#include "actions/fight.h"
#include "actions/act.h"          /* ACMDs located within the act*.c files */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      SKILL_NAME -
**
**  FORMAL PARAMETERS:
**
**      ch:
**          A pointer to the character structure that is trying to use the skill/spell.
**
**  RETURN VALUE:
**
**      None
**
**  DESIGN:
**
**      What's it do ?
**
*/
SPECIAL(shop_keeper);
ACMD(do_stand);
ACMD(do_wake);

/* Uncomment this if you want steals to be logged. */
/* #define LOG_STEALS */

ACMD(do_steal)
{
  ObjData  *obj;
  CharData *victim;

  char arg[100]; /* This is the victims name. */
  char obj_name[100];

  int percent, gold, eq_pos, pcsteal = 0;
  bool ohoh = FALSE;
  bool notice = FALSE;
  int steal_cost = 5;
  int skill = subcmd == SCMD_STEAL ? SKILL_STEAL : SKILL_MUG;

  IF_UNLEARNED_SKILL( "You have no talent in the art of theft.\r\n" );

  argument = one_argument( argument, obj_name );
  one_argument( argument, arg );

  IF_CH_CANT_SEE_VICTIM( "Steal from who?\r\n"); /* victim is assigned here. */
  IF_CH_CANT_BE_VICTIM( "Are'nt we funny today.\r\n" );
  // Let em steal from a fighting victim but make them join the fight if they fail
  //  IF_VICTIM_CANT_BE_FIGHTING( "They are to alert!\r\n" );
  IF_ROOM_IS_PEACEFUL( "You don't feel like stealing right now.\r\n" );

  if( chIsNonCombatant(victim) )
  {
    sendChar(ch, "You can't steal from %s, they are a non-combatant.\r\n", GET_NAME(victim));
    return;
  }

  if( !IS_NPC(victim) && GET_LEVEL(ch) < 20 )
  {
    sendChar(ch, "You must be at least 20th lvl to steal from a player.\r\n");
    return;
  }

  if( GET_MOVE(ch) < steal_cost )
  {
    send_to_char("You're too exhausted to effectively steal anything.\r\n", ch);
    return;
  }

  GET_MOVE(ch) -= steal_cost;

  if (!IS_NPC(victim) && !CONFIG_PT_ALLOWED) {
    sendChar(ch, "No.\r\n");
    return;
  }

  percent = number(1, 101) - dex_app_skill[GET_DEX(ch)].p_pocket;

  /* Easier if victim is asleep. */
  if( !AWAKE(victim) ) percent -= 20;

  /* Harder to steal from higher level, easier to steal from lower level. */
  percent += ( GET_LEVEL( victim ) - GET_LEVEL( ch )) * 2;

  /* Intelligent beings are a bit harder to steal from */
  percent += (GET_INT(victim) - 12) * 2;

  /* Very powerful creatures are harder to steal from, it gets easier if they are wounded. */
  if (GET_HIT(victim) > (GET_LEVEL(ch) * 200))
    percent += (GET_HIT(victim) - (GET_LEVEL(ch) * 200))/500;

  if( notice && !IS_NPC(victim) &&
                !IS_NPC(ch) &&
                !PLR_FLAGGED(victim, PLR_THIEF) &&
                !PLR_FLAGGED(victim, PLR_KILLER) &&
                !PLR_FLAGGED(ch, PLR_THIEF))
  {
  /*
  ** SET_BIT(ch->specials.act, PLR_THIEF); send_to_char("Okay, you're the
  ** boss... you're now a THIEF!\r\n",ch); mlog("PC Thief bit set
  ** on %s", GET_NAME(ch));
  */
    pcsteal = 1;
  }

  /* Easier if subcmd == SCMD_MUG, but you'll get caught! */
  if (subcmd == SCMD_MUG && percent < 101) percent -= 20, notice = TRUE;
  
  if (IS_TRICKSTER(ch)) {
      // Tricksters are slightly better at stealing to begin with.
      percent -= GET_ADVANCE_LEVEL(ch);

      // High level tricksters also get a bonus against group members.
      if(chGroupedWithTarget(ch, victim) && GET_ADVANCE_LEVEL(ch) >= 4)
          percent -= 15;
  }

  if(!IS_NPC(victim) && victim->desc == NULL )
  {
    percent = 101;
    sendChar(ch, "Ok... let the hunt begin!\r\n");
    SET_BIT_AR(PLR_FLAGS(ch), PLR_HUNTED);
    (ch)->player_specials->saved.phunt_countdown = HUNTED_TIME;
  }

  /* NO NO With Imm's, Clones and Shopkeepers! */
  if((GET_LEVEL( victim ) >= LVL_IMMORT) || GET_MOB_SPEC( victim ) == shop_keeper || IS_CLONE(ch))
    percent = 101;		/* Failure */        

  if (str_cmp(obj_name, "coins") && str_cmp(obj_name, "gold"))
  {
    if (!(obj = get_obj_in_list_vis(ch, obj_name, victim->carrying)))
    {
      for (eq_pos = 0; eq_pos < NUM_WEARS; eq_pos++)
        if( victim->equipment[eq_pos] &&
          ( isname(obj_name, victim->equipment[eq_pos]->name)) &&
            CAN_SEE_OBJ(ch, victim->equipment[eq_pos]))
        {
          obj = victim->equipment[eq_pos];
          break;
        }

      if( !obj )
      {
        act("$E hasn't got that item.", FALSE, ch, 0, victim, TO_CHAR);
        return;
      }
      else
      {
        /* It is equipment */
        /* Difficult, but not neccessarily impossible to steal equipment. */

          if( (IN_ARENA(ch) || ZONE_FLAGGED(world[ch->in_room].zone, ZONE_ARENA) ||
                  ZONE_FLAGGED(world[ch->in_room].zone, ZONE_SLEEPTAG)) && !IS_FLAG_ITEM(obj) )
              percent = 101;
          
          switch( eq_pos )
          {
          case WEAR_WIELD:
              percent = affected_by_spell(victim, SKILL_DISARM)? 50 : 101; break; /* You MUST disarm them. */
          case WEAR_BODY:     percent += 100; break;
          case WEAR_FEET:     percent += (GET_POS(victim) == POS_STANDING ? 100 : 70); break;
          case WEAR_SHIELD:   percent += 90; break;
          case WEAR_ABOUT:    percent += 85; break;
          case WEAR_ARMS:
          case WEAR_LEGS:
          case WEAR_HANDS:    percent += 80; break;
          case WEAR_HEAD:     
          case WEAR_CLOAK:    percent += 75; break;
          case WEAR_FINGER_R:
          case WEAR_FINGER_L: percent += 70; break;
          case WEAR_WRIST_R:
          case WEAR_WRIST_L:
          case WEAR_NECK:
          case WEAR_WAIST:    percent += 65; break;
          case WEAR_LIGHT:
          case WEAR_HOLD:     percent += 50 + str_app[STRENGTH_APPLY_INDEX(victim)].wield_w; break;
          case WEAR_ANKLES:   percent += 50; break;
          case WEAR_FACE:     percent += 101; break;
          case WEAR_EARS:     percent += 80; break;          
          default:
            mudlog(NRM, LVL_IMMORT, TRUE, "(STEAL): unknown eq position %d passed to steal.c", eq_pos);
            percent = 101;
            break;
        }

        percent += GET_OBJ_WEIGHT(obj);	/* Make heavy harder */

        if (percent <= GET_SKILL(ch, skill)) {
            if ((percent + (3 * GET_INT(victim))) > GET_SKILL(ch, skill))
                notice = TRUE;
            switch (eq_pos) { /* show appropriate message */
                case WEAR_BODY:
                case WEAR_ABOUT:
                    act("You steal $p right off $N's body!", FALSE, ch, obj, victim, TO_CHAR);
                    if (notice) {
                        act("$n steals $p right off your body!", FALSE, ch, obj, victim, TO_VICT);
                        act("$n steals $p right off $N's body!", FALSE, ch, obj, victim, TO_NOTVICT);
                    }
                    break;
                case WEAR_FEET:
                    act("You slip $p off $N's feet!", FALSE, ch, obj, victim, TO_CHAR);
                    if (notice) {
                        act("$n slips $p off your feet!", FALSE, ch, obj, victim, TO_VICT);
                        act("$n slips $p off $N's feet!", FALSE, ch, obj, victim, TO_NOTVICT);
                    }
                    break;
                case WEAR_SHIELD:
                    act("You prize $p off $N's arm!", FALSE, ch, obj, victim, TO_CHAR);
                    if (notice)
                    {
                        act("$n prizes $p off your arm!", FALSE, ch, obj, victim, TO_VICT);
                        act("$n prizes $p off $N's arm!", FALSE, ch, obj, victim, TO_NOTVICT);
                    }
                    break;
                case WEAR_ARMS:
                    act("You steal $p right off $N's arms!", FALSE, ch, obj, victim, TO_CHAR);
                    if (notice)
                    {
                        act("$n steals $p right off your arms!", FALSE, ch, obj, victim, TO_VICT);
                        act("$n steals $p right off $N's arms!", FALSE, ch, obj, victim, TO_NOTVICT);
                    }
                    break;
                case WEAR_LEGS:
                    act("You steal $p right off $N's legs!", FALSE, ch, obj, victim, TO_CHAR);
                    if (notice)
                    {
                        act("$n steals $p right off your legs!", FALSE, ch, obj, victim, TO_VICT);
                        act("$n steals $p right off $N's legs!", FALSE, ch, obj, victim, TO_NOTVICT);
                    }
                    break;
                case WEAR_HEAD:
                    act("You snatch $p off $N's head!", FALSE, ch, obj, victim, TO_CHAR);
                    if (notice)
                    {
                        act("$n snatches $p off your head!", FALSE, ch, obj, victim, TO_VICT);
                        act("$n snatches $p off $N's head!", FALSE, ch, obj, victim, TO_NOTVICT);
                    }
                    break;
                case WEAR_CLOAK:
                    act("You steal $p from $N's back!", FALSE, ch, obj, victim, TO_CHAR);
                    if (notice)
                    {
                        act("$n steals $p right off your back!", FALSE, ch, obj, victim, TO_VICT);
                        act("$n steals $p right off $N's back!", FALSE, ch, obj, victim, TO_NOTVICT);
                    }
                    break;
                case WEAR_FINGER_R:
                case WEAR_FINGER_L:
                    act("You deftly slide $p from $N's finger!", FALSE, ch, obj, victim, TO_CHAR);
                    if (notice)
                    {
                        act("$n deftly slides $p from your finger!", FALSE, ch, obj, victim, TO_VICT);
                        act("$n deftly slides $p from $N's finger!", FALSE, ch, obj, victim, TO_NOTVICT);
                    }
                    break;
                case WEAR_WRIST_R:
                case WEAR_WRIST_L:
                    act("You prize $p from $N's wrist!", FALSE, ch, obj, victim, TO_CHAR);
                    if (notice)
                    {
                        act("$n prizes $p from your wrist!", FALSE, ch, obj, victim, TO_VICT);
                        act("$n prizes $p from $N's wrist!", FALSE, ch, obj, victim, TO_NOTVICT);
                    }
                    break;
                case WEAR_NECK:
                    act("You snatch $p from $N's neck!", FALSE, ch, obj, victim, TO_CHAR);
                    if (notice)
                    {
                        act("$n snatches $p from your neck!", FALSE, ch, obj, victim, TO_VICT);
                        act("$n snatches $p from $N's neck!", FALSE, ch, obj, victim, TO_NOTVICT);
                    }
                    break;
                case WEAR_LIGHT:
                case WEAR_HOLD:
                    act("You prize $p from $N's grasp!", FALSE, ch, obj, victim, TO_CHAR);
                    if (notice)
                    {
                        act("$n prizes $p from your hand!", FALSE, ch, obj, victim, TO_VICT);
                        act("$n prizes $p from $N's hand!", FALSE, ch, obj, victim, TO_NOTVICT);
                    }
                    break;
                case WEAR_WAIST:
                    act("You steal $p from $N's waist!", FALSE, ch, obj, victim, TO_CHAR);
                    if (notice)
                    {
                        act("$n steals $p from your waist!", FALSE, ch, obj, victim, TO_VICT);
                        act("$n steals $p from $N's waist!", FALSE, ch, obj, victim, TO_NOTVICT);
                    }
                    break;
                case WEAR_HANDS:
                    act("You slip $p off $N's hands!", FALSE, ch, obj, victim, TO_CHAR);
                    if (notice)
                    {
                        act("$n slips $p off your hands!", FALSE, ch, obj, victim, TO_VICT);
                        act("$n slips $p from $N's hands!", FALSE, ch, obj, victim, TO_NOTVICT);
                    }
                    break;
                case WEAR_WIELD:
                    act("You steal $p from under $N's fingertips.", FALSE, ch, obj, victim, TO_CHAR);
                    if (notice)
                    {
                        act("$n steals $p from under your fingertips!", FALSE, ch, obj, victim, TO_VICT);
                        act("$n steals $p from under $N's fingertips", FALSE, ch, obj, victim, TO_NOTVICT);
                    }
                    //mudlog(NRM, LVL_IMMORT, TRUE, "(STEAL): %s stole a weapon! BUG! BUG!", GET_NAME(ch));
                    break;
                case WEAR_FACE:
                    act("You slip $p off $N's face!", FALSE, ch, obj, victim, TO_CHAR);
                    if (notice)
                    {
                        act("$n slips $p off your face!", FALSE, ch, obj, victim, TO_VICT);
                        act("$n slips $p from $N's face!", FALSE, ch, obj, victim, TO_NOTVICT);
                    }
                    break;
                case WEAR_ANKLES:
                    act("You slip $p off $N's ankles!", FALSE, ch, obj, victim, TO_CHAR);
                    if (notice)
                    {
                        act("$n slips $p off your ankles!", FALSE, ch, obj, victim, TO_VICT);
                        act("$n slips $p from $N's ankles!", FALSE, ch, obj, victim, TO_NOTVICT);
                    }
                    break;
                case WEAR_EARS:
                    act("You slip $p off $N's ears!", FALSE, ch, obj, victim, TO_CHAR);
                    if (notice)
                    {
                        act("$n slips $p off your ears!", FALSE, ch, obj, victim, TO_VICT);
                        act("$n slips $p from $N's ears!", FALSE, ch, obj, victim, TO_NOTVICT);
                    }
                    break;
                default:
                    sendChar(ch, "Unknown equipment position to steal, please report!\r\n");
                    mudlog(NRM, LVL_IMMORT, TRUE, "(STEAL): unknown equipment position(%d) passed to steal.c", eq_pos);
                    break;
            }
            if( SKILL_ADVANCES_WITH_USE && !IS_NPC(ch) && IS_NPC(victim) )
            {
                advanceSkill( ch, skill, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING, DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS );

            }
            obj_to_char(unequip_char(victim, eq_pos), ch);
#ifdef LOG_STEALS
            mudlog(NRM, LVL_IMMORT, TRUE, "(STEAL): %s stole equiped item %s from %s.(percent = %d)",
                    GET_NAME(ch), obj->short_description, GET_NAME(victim), percent);
#endif
        }
        else {
            if(GET_RACE(ch) == RACE_SHALFLING && percentSuccess(15)) {
                sendChar(ch, "You realize you can't pull off this heist and abort.\r\n");
                return;
            }

            ohoh = TRUE;
            sendChar(ch, "Ooops...\r\n");
            sprintf(buf, "%s just tried to steal %s from you!", GET_NAME(ch), obj->short_description);
            sendChar(victim, buf);
        }
      } /* equipment else */
    } /* if */
    else {
		  if( (IN_ARENA(ch) || ZONE_FLAGGED(world[ch->in_room].zone, ZONE_ARENA) ||
			  ZONE_FLAGGED(world[ch->in_room].zone, ZONE_SLEEPTAG)) && !IS_FLAG_ITEM(obj))
			  percent = 101;    

		  /* obj found in inventory */
          percent += GET_OBJ_WEIGHT(obj);	/* Make heavy harder */
	  if ((percent + (GET_INT(victim) * 2)) > GET_SKILL(ch, skill))
		notice = TRUE;
          if (percent > GET_SKILL(ch, skill)) {
	      ohoh = TRUE;
  	      act("Oops..", FALSE, ch, 0, 0, TO_CHAR);
	      act("$n tried to steal something from you!", FALSE, ch, 0, victim, TO_VICT);
	      act("$n tries to steal something from $N.", TRUE, ch, 0, victim, TO_NOTVICT);
      	  }
	  else {			/* Steal the item */
		if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch))) {
	  	    if ((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) < CAN_CARRY_W(ch)) {
                        if(IS_SET_AR(GET_OBJ_EXTRA(obj), ITEM_SOULBOUND) || contains_soulbound(obj)) {
                            act("You cannot steal a bound item.", FALSE, ch, 0, victim, TO_CHAR);
                            return;
                        }

	    		obj_from_char(obj);
	    		obj_to_char(obj, ch);
	    		send_to_char("Got it!\r\n", ch);
        		if( SKILL_ADVANCES_WITH_USE && !IS_NPC(ch) && IS_NPC(victim) && (GET_LEVEL(ch) >= (GET_LEVEL(victim) - 10)))
			{
            			advanceSkill( ch, skill, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING, DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS );
#ifdef LOG_STEALS
				mudlog(NRM, LVL_IMMORT, TRUE, "STEAL:advanceSkill called for %s for stealing from %s", GET_NAME(ch), GET_NAME(victim));
#endif
			}
			if (notice) {
			    sprintf(buf, "%s just stole %s from you!\r\n", GET_NAME(ch), obj->short_description);
			    sendChar(victim, buf);
			}
#ifdef LOG_STEALS
			mudlog(NRM, LVL_IMMORT, TRUE, "(STEAL): %s stole %s from %s's inventory.(percent = %d)",
                                          GET_NAME(ch), obj->short_description, GET_NAME(victim), percent);
#endif
	  	     }
		}
		else
	  		send_to_char("You cannot carry that much.\r\n", ch);
          }
      }
    }
    else {
		if( IN_ARENA(ch) || ZONE_FLAGGED(world[ch->in_room].zone, ZONE_ARENA) ||
			ZONE_FLAGGED(world[ch->in_room].zone, ZONE_SLEEPTAG))
			percent = 101;    
		
		/* Steal some coins */
		/* stealing gold is a bit easier */
	percent += 20;
	if ((percent + GET_INT(victim)) > GET_SKILL(ch, skill))
	    notice = TRUE;
    	if (percent > GET_SKILL(ch, skill)) {
            ohoh = TRUE;
            act("Oops..", FALSE, ch, 0, 0, TO_CHAR);
            act("You discover that $n has $s hands in your wallet.", FALSE, ch, 0, victim, TO_VICT);
      	    act("$n tries to steal gold from $N.", TRUE, ch, 0, victim, TO_NOTVICT);
    	}
	else {
      	    /* Steal some gold coins */
            gold = (int) ((GET_GOLD(victim) * number(1, GET_LEVEL(ch))) / 500);
            if (gold > 0) {
		GET_GOLD(ch) += gold;
		GET_GOLD(victim) -= gold;
		sprintf(buf, "Bingo!  You got %d gold coins.\r\n", gold);
		send_to_char(buf, ch);
		if (notice) {
		    sprintf(buf, "%s stole some gold from you!\r\n", GET_NAME(ch));
		    sendChar(victim, buf);
		}
      	    }
	    else {
		send_to_char("You couldn't get any gold...\r\n", ch);
		if (notice) {
		    sprintf(buf, "%s tried to steal gold from you, but could not find any!\r\n", GET_NAME(ch));
		    sendChar(victim, buf);
		}
      	    }
        }
    }

  if (notice) ohoh = TRUE;

  if (notice && IS_NPC(victim) && MOB_FLAGGED(victim, MOB_MEMORY) && !IS_NPC(ch) && (GET_LEVEL(ch) < LVL_IMMORT))
	  remember(victim, ch);

  if (!AWAKE(victim) && ohoh)
  {
	  affect_from_char(victim, SPELL_SLEEP);
	  affect_from_char(victim, SPELL_DANCE_DREAMS);
          affect_from_char(victim, SKILL_BRAIN);
	  do_wake(victim, "", 0, 0);
  }

  if (ohoh && IS_NPC(victim) && AWAKE(victim)) {
    if ((GET_POS(victim) < POS_STANDING) && (GET_POS(victim) > POS_STUNNED))
	do_stand(victim, "", 0, 0);
    hit(victim, ch, TYPE_UNDEFINED);
  }

  if(notice && !IS_NPC(victim) && !IS_NPC(ch)) {
	player_steal_victim(ch, victim);
  }

  /*if(notice && !IS_NPC(victim) && !IS_NPC(ch) && !PLR_FLAGGED(victim, PLR_THIEF) && !PLR_FLAGGED(victim, PLR_KILLER) && !PLR_FLAGGED(ch, PLR_THIEF)) {
	player_steal_victim(ch, victim);
  } if */

  STUN_USER_MAX; 
} /* steal */


