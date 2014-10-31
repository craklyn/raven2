/*
**++
**
**  FACILITY:  RavenMUD
**
**  LEGAL MUMBO JUMBO:
**
**      This is based on code developed for DIKU and Circle MUDs.
**
**  MODULE DESCRIPTION:
**      The six skills, bash, kick, trip, hamstring, fists of fury and sweep
**	where virtually identical, with only minor cosmetic differences. It
**	seemed silly to have the same code in 6 different files to do basically
**	the same thing. If all a skill does is basically stun the victim and
**	cause some damage, it belongs here. The knock skill is an example of
**	skill that stuns, but doesn't really fit here, as I would of had to
**	copy the entire knock file for knock and knock only. Backstab was 
**	another skill that does much of what is done here, but is still just
**	different enough that I felt it warranted its own file.
**
**
**  AUTHORS:
**
**      Vex from RavenMUD, based largely on skills by Digger.
**
**  NOTES:
**
**      Use 132 column editing in here.
**
**--
*/

#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/class.h"
#include "general/comm.h"
#include "general/handler.h"
#include "actions/interpreter.h"
#include "magic/skills.h"
#include "magic/spells.h"
#include "util/utils.h"
#include "magic/stun.h"
#include "magic/knock.h"
#include "specials/special.h"
#include "magic/sing.h"
#include "magic/magic.h"
#include "actions/fight.h"
#include "actions/act.h"          /* ACMDs located within the act*.c files */

/* ============================================================================ 
The sub commands the do_stun function knows how to handle.
============================================================================ */
#define SUBCMD_BASH       0
#define SUBCMD_KICK       1
#define SUBCMD_FIST       2
#define SUBCMD_TRIP       3 
#define SUBCMD_HAMSTRING  4 
#define SUBCMD_SWEEP      5
#define SUBCMD_SHIELD     6
#define NUM_STUN_SUBCMDS  7

/* ============================================================================ 
Stun skills are all assumed to advance through use. The messages received
when the skill advances are defined below.
============================================================================ */

#define SKILL_ADVANCES_WITH_USE   TRUE

char *stun_advance_strings[NUM_STUN_SUBCMDS + 1] = {
"Doors seem to quake in your prescence.",			/* 0 SUBCMD_BASH */
"You've become notably more proficient.",			/* 1 SUBCMD_KICK */
"Your fists momentarily glow blue.",				/* 2 SUBCMD_FIST */
"You've become notably more proficient.",			/* 3 SUBCMD_TRIP */
"You've become notably more proficient.",			/* 4 SUBCMD_HAMSTRING */
"The janitors look at you with envy.",				/* 5 SUBCMD_SWEEP */
"You've become notably more proficient.",                       /* 6 SUBCMD_SHIELD */
"\n"
};
#define SKILL_ADVANCE_STRING      stun_advance_strings[subcmd]

/* ============================================================================ 
The maximum skill level the classes can have for each stun skill.
============================================================================ */
static int max_stun_skill[NUM_STUN_SUBCMDS][NUM_CLASSES + 2] = { 
/* Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  XX  XX */
 { 00, 00, 00, 94, 85, 00, 00, 85, 85, 00, 00, 00}, /* 0 SUBCMD_BASH */
 { 00, 00, 90, 90, 90, 90, 90, 90, 90, 00, 00, 00}, /* 1 SUBCMD_KICK */
 { 00, 00, 00, 00, 00, 00, 90, 00, 00, 00, 00, 00}, /* 2 SUBCMD_FIST */
 { 00, 00, 90, 00, 90, 90, 00, 00, 00, 90, 00, 00}, /* 3 SUBCMD_KICK */
 { 00, 00, 00, 00, 00, 90, 00, 00, 00, 00, 00, 00}, /* 4 SUBCMD_HAMSTRING */
 { 00, 00, 00, 00, 00, 00, 90, 00, 00, 00, 00, 00}, /* 5 SUBCMD_SWEEP */
 { 00, 00, 00, 90, 00, 00, 00, 00, 00, 00, 00, 00}, /* 6 SUBCMD_SHIELDBASH */
};
#define SKILL_MAX_LEARN max_stun_skill[subcmd][(int)GET_CLASS(ch)]

/* ============================================================================ 
The attributes that affect advancement of the stun skills using do_stun.
============================================================================ */
#define DEX_AFFECTS               TRUE
#define INT_AFFECTS               TRUE
#define WIS_AFFECTS               FALSE

/* ============================================================================ 
Stun duration for all stun skills using do_stun.
============================================================================ */
#define STUN_MIN                  2
static int max_stun_duration[NUM_STUN_SUBCMDS] =
/* bash kick fists trip ham sweep shield*/
{  4,   3,   4,    3,   4,  4,    4 };
// static int min_stun_duration[NUM_STUN_SUBCMDS] =
//{  2,   2,   2,    2,   2,  2,    3 };
// #define STUN_MIN                  min_stun_duration[subcmd]
#define STUN_MAX                  max_stun_duration[subcmd]

/* a subfunction to send target staggering out of the room */
static void knock_target_out(CharData *ch, CharData *victim)
{
    char msgbuf[200];
    int dir_to_go = -1;
    int dirs_tested = 0;
    int i;

    /* find a random exit to leave by */
    for (i = 0; i < 6; i++) {
        if (CAN_GO(victim, i)) {
            int to_room = EXIT(victim, i)->to_room;
            if (!IS_SET_AR(world[to_room].room_flags, ROOM_DEATH )) {
                dirs_tested++;
                if (number(1,dirs_tested) == 1)
                    dir_to_go = i;
            }
        }
    }

    if (dir_to_go == -1) return;
    sprintf(msgbuf, "$N's bash sends you staggering %swards!", dirs[dir_to_go]);
    act(msgbuf, FALSE, victim, 0, ch, TO_CHAR);
    sprintf(msgbuf, "$n is sent staggering %swards!", dirs[dir_to_go]);
    act(msgbuf, FALSE, victim, 0, ch, TO_ROOM);
    do_simple_move(victim, dir_to_go, 0);
}

/* ============================================================================ 
The do_stun function. Most of the muds stun oriented skills are effectively
done here. This function COULD be used externally, but, its better to call
the seperate skill functions themselves IMHO. Vex.
============================================================================ */
void do_stun(CharData *ch, char *argument, int cmd, int subcmd)
{
char   arg[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
CharData *victim;
int the_skill, stun_damage;
#define THIS_SKILL                the_skill
CharData *tch, *next_tch;


    one_argument( argument, arg );

    /* Setup the_skill and stun_damage */
    switch(subcmd) {
    case SUBCMD_BASH:
	the_skill = SKILL_BASH;
	stun_damage = GET_STR(ch);
	break;
    case SUBCMD_KICK:
	the_skill = SKILL_KICK;
	stun_damage = GET_LEVEL(ch) >> 1;
	if (IS_MINOTAUR(ch)) stun_damage += 15;
	break;
    case SUBCMD_FIST:
	the_skill = SKILL_FISTS_OF_FURY;
	stun_damage = (number(1, (GET_LEVEL(ch) >> 1)) * 8);
	// The following line was added to make fists not lose damage, when
	// knock was removed from their arsenel.  Craklyn
	// stun_damage += ( number( 1, (GET_LEVEL(ch)>>1)) * 4 );
	break;
    case SUBCMD_TRIP:
	the_skill = SKILL_TRIP;
	stun_damage = 10;
	break;
    case SUBCMD_HAMSTRING:
	the_skill = SKILL_HAMSTRING;
	stun_damage = GET_LEVEL(ch) + GET_DEX(ch);
	break;
    case SUBCMD_SWEEP:
	the_skill = SKILL_SWEEP;
	stun_damage = 20;
	break;
    case SUBCMD_SHIELD:
	the_skill = SKILL_SHIELD_BASH;
	stun_damage = GET_STR(ch) * 3;
	break;
    default:
	mudlog(NRM, LVL_IMMORT, TRUE, "SYSERR: Unknown subcmd %d passed to do_stun!", subcmd);
	return;
    }
    IF_UNLEARNED_SKILL   ( "You wouldn't know where to begin.\r\n" );

    sprintf(buf, "%s who?\r\n", spells[the_skill]);
    IF_CH_CANT_SEE_VICTIM(CAP(buf));

    IF_CH_CANT_BE_VICTIM ( "Aren't we funny today...\r\n" );

    IF_ROOM_IS_PEACEFUL  ( "You're overcome by a feeling of serenity.\r\n" );

    IF_CANT_HURT_VICTIM;
	IF_CANT_HIT_FLYING;

    if (!IS_NPC(victim) && victim->mount) {
        if (victim->mount->in_room == victim->in_room) {
            sendChar(ch, "You cannot stun a mounted person!\r\n");
            return;
        }
        else {
            sendChar(victim, "You have become separated from your mount!\r\n");
	    if(victim->mount->rider == victim)
                victim->mount->rider = NULL;
            victim->mount = NULL;
        }
    }

    if (the_skill == SKILL_BASH || the_skill == SKILL_HAMSTRING)
        IF_CH_NOT_WIELDING   ( "You need to wield a weapon to make it a success.\r\n" );

    if (the_skill == SKILL_SHIELD_BASH && !ch->equipment[WEAR_SHIELD]) {
        send_to_char("You must be wearing a shield to use this skill.\r\n", ch);
        return;
    }

    if (the_skill == SKILL_BASH || the_skill == SKILL_TRIP || the_skill == SKILL_SWEEP || the_skill == SKILL_SHIELD_BASH)
        IF_VICTIM_NOT_STANDING( "Your victim is not standing.\r\n" );

    if (the_skill == SKILL_FISTS_OF_FURY)
        IF_CH_IS_WIELDING( "You shame yourself by attempting the art of fury with weapon in hand.\r\n" );

    if( IS_NPC( victim ) && IS_SET_AR( MOB_FLAGS( victim ), MOB_NOBASH )) {
	sendChar(ch, "%s is too tough for you to be able to %s effectively!\r\n", GET_NAME(victim), spells[the_skill]);
        return;
    }

    if( skillSuccess( ch, the_skill )){
        if( SKILL_ADVANCES_WITH_USE )
            advanceSkill( ch, the_skill, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING, DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS );

        if( victimAvoidsSkill( ch, victim, the_skill )) {
            if( victimIsAngry( 40 )) 
		set_fighting(victim, ch); 
        } else if (raceSkillAvoid(ch, victim, the_skill)) {
            if( victimIsAngry( 40 )) 
		set_fighting(victim, ch);
        } else if ( the_skill == SKILL_KICK &&
                (IS_HALFLING(victim) && percentSuccess(15)) ||
                (GET_RACE(victim) == RACE_HALFLING && percentSuccess(15)) ) {
            send_to_char("You spot a coin and bend over to pick it up.\r\n", victim);
            GET_GOLD(victim) += 1;
            act("You go to kick $n but $e suddenly ducks!\r\n", FALSE,
                    victim, 0, ch, TO_VICT);
            act("$n suddenly bends over, causing $N to miss a kick!\r\n",
                    FALSE, victim, 0, ch, TO_NOTVICT);
	} else {
            if(IS_DARK_PRIEST(victim) && GET_ADVANCE_LEVEL(victim) >= 5 && percentSuccess(15)) {
                send_to_char("Your god protects you from being stunned.\r\n", victim);
                act("$n's god protects $n from your stun!\r\n", FALSE,
                        victim, 0, ch, TO_VICT);
                act("$n's god intervenes, protecting $n from $N's stun!\r\n",
                        FALSE, victim, 0, ch, TO_NOTVICT);
            }
            else if(GET_RACE(victim) == RACE_SORC && !affected_by_spell(victim, SKILL_UNIMPEDED_ASSAULT)) {
                add_affect(victim, victim, SKILL_UNIMPEDED_ASSAULT, 0, APPLY_NONE, 0, number(2, 2 TICKS),
                        0, FALSE, FALSE, FALSE, FALSE);
                send_to_char("Your blood boils and you continue unimpeded.\r\n", victim);
                act("$n's eyes narrow down at you and turn red!\r\n", FALSE,
                        victim, 0, ch, TO_VICT);
                act("$n's eyes narrow at $N and turn red!\r\n",
                        FALSE, victim, 0, ch, TO_NOTVICT);
                if(GET_POS(victim) == POS_SITTING)
                    do_stand(victim, "", 0, 0);
            }
            else
                STUN_VICTIM_RANGE;

            damage(ch, victim, stun_damage, the_skill);
            /* 1 in 30 chance a shield bash will knock target out of room */
            if (subcmd == SUBCMD_SHIELD) {
                if (number(1,30) == 1 && GET_POS(victim) != POS_DEAD)
                    knock_target_out(ch, victim);
            }

            if ( (the_skill == SKILL_KICK || subcmd == SUBCMD_SHIELD)
                    && (SINGING(victim) != SKILL_BERSERK) )
            {
                if (SINGING(victim)) stop_singing(victim);
            }
            
            if (the_skill == SKILL_FISTS_OF_FURY && GET_POS(victim) != POS_DEAD)
            {
                knock_internal(ch, victim);
            }
	    else if (the_skill == SKILL_BASH || the_skill == SKILL_TRIP ||
		     the_skill == SKILL_SWEEP || the_skill == SKILL_HAMSTRING ||
                     the_skill == SKILL_SHIELD_BASH)
                GET_POS(victim) = POS_SITTING;
            if (the_skill == SKILL_HAMSTRING) {
                affect_from_char(ch, SKILL_HAMSTRING);
                add_affect(ch, victim, SKILL_HAMSTRING, 0, APPLY_NONE, 0, 1 TICKS,
                        AFF_HAMSTRUNG, FALSE, FALSE, FALSE, FALSE);
            }
            if (the_skill == SKILL_SWEEP && IS_ANCIENT_DRAGON(ch) && 
                    GET_ADVANCE_LEVEL(ch) >= 5 && percentSuccess(66)){
                if(debuff_protections(ch, victim)) {
                    act("You strike at $N's throat, dark Ki coiling around your fingers!", FALSE, ch, 0, victim, TO_CHAR);
                    act("$n strikes at your throat, dark energy coiling around $s fingers!", FALSE, ch, 0, victim, TO_VICT);
                    act("$n strikes at $N's throat, dark energy coiling around $s fingers.", FALSE, ch, 0, victim, TO_NOTVICT);
                }
            }

        }
        STUN_USER_MIN;

        /* If the steed is knocked over, the victim is thrown off! */
        if (IS_NPC(victim) && victim->rider && GET_POS(victim) < POS_STANDING) {
            awkward_dismount(victim->rider);
        }
    }/* if */
    else {
	if (the_skill == SKILL_BASH
                || the_skill == SKILL_TRIP
                || the_skill == SKILL_SWEEP
                || the_skill == SKILL_HAMSTRING
                || the_skill == SKILL_SHIELD_BASH) {
            GET_POS(ch) = POS_SITTING;
            if (ch->rider) awkward_dismount(ch->rider);
            if (ch->mount) awkward_dismount(ch);
        }

        STUN_USER_MAX;
        damage(ch, victim, 0, the_skill);
        
        // They just failed their stun.  Let's teach them a lesson for trying to
        // mess up another player's freedom
        if (the_skill != SKILL_SWEEP &&
                the_skill != SKILL_KICK)
        {
            OFF_BALANCE(ch) = 1;
            send_to_char("You find yourself off balance and vulnerable!\r\n", ch);
            act("$n is off balance and vulnerable!\r\n", FALSE,	ch, 0, victim, TO_VICT);
            act("$n is off balance and vulnerable!\r\n", FALSE, ch, 0, victim, TO_NOTVICT);

            for( tch = world[ch->in_room].people; tch; tch = next_tch ){
                next_tch = tch->next_in_room;

                if(FIGHTING(tch) == ch && GET_ASPECT(tch) == ASPECT_MONKEY
                        && skillSuccess(tch, SKILL_ART_MONKEY))
                    do_knock(tch, ch->player.name, 0, 0);
            }
        }
    }/* else */

}/* do_stun */

/* **************************************************************************** 
Skills using the do_stun function.
**************************************************************************** */

/* ============================================================================ 
The bash skill.
============================================================================ */
ACMD(do_bash) { do_stun(ch, argument, cmd, SUBCMD_BASH); }

/* ============================================================================ 
The shield bash skill.
============================================================================ */
ACMD(do_shieldbash) { do_stun(ch, argument, cmd, SUBCMD_SHIELD); }

/* ============================================================================ 
The kick skill.
============================================================================ */
ACMD(do_kick) { do_stun(ch, argument, cmd, SUBCMD_KICK); }

/* ============================================================================ 
The fists of fury skill.
============================================================================ */
ACMD(do_fists) { do_stun(ch, argument, cmd, SUBCMD_FIST); }

/* ============================================================================ 
The hamstring skill.
============================================================================ */
ACMD(do_hamstring) { do_stun(ch, argument, cmd, SUBCMD_HAMSTRING); }

/* ============================================================================ 
The sweep skill.
============================================================================ */
ACMD(do_sweep) { do_stun(ch, argument, cmd, SUBCMD_SWEEP); }

/* ============================================================================ 
The trip skill.
============================================================================ */
ACMD(do_trip) { do_stun(ch, argument, cmd, SUBCMD_TRIP); }
