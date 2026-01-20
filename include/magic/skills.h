/*
**++
**  RCSID:     $Id: skills.h,v 1.1.1.1 2000/10/10 04:15:17 raven Exp $
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
**
**  NOTES:
**
**      Use 132 column editing in here.
**
**--
*/


/*
**
**  MODIFICATION HISTORY:
**
**  $Log: skills.h,v $
**  Revision 1.1.1.1  2000/10/10 04:15:17  raven
**  RavenMUD 2.0
**
**  Revision 1.12  1997/12/07 15:54:27  vex
**  Cleanup crew came through.
**
**  Revision 1.11  1997/10/19 09:23:01  vex
**  Added art of the monkey.
**
**  Revision 1.10  1997/02/27 19:25:33  digger
**  Changed the rollToBeat into a new function called percentSuccess.
**
**  Revision 1.9  1997/01/03 12:34:54  digger
**  Renamed several of the functions from skills.c and added skill
**  avoidance to fist and hamstring. Vex has put in MAJOR changes
**  to the summoning code and many checks for Book of Blood signatures
**  were added.
**
** Revision 1.8  1996/03/14  11:46:25  digger
** Added a skill check that was preventing NPCs from using any
** skills or spells during combat.
**
** Revision 1.1  1994/12/16  14:23:52  jbp
** Initial revision
**
**
*/

#ifndef _SKILLS_H_
#define _SKILLS_H_

/*
** STANDARD Skill Entry Macros
*/

#define IF_CH_CANT_SEE_VICTIM( s )                          \
            if( !(victim = get_char_room_vis( ch, arg )) ){ \
                if( FIGHTING(ch) && ( !*arg )) {            \
                    victim = FIGHTING(ch);                  \
                }                                           \
                else {                                      \
                    sendChar( ch, s );                      \
                    return;                                 \
                }/* if */                                   \
            }/* if */

#define IF_CH_CANT_BE_VICTIM( s )                             \
            if( victim == ch ){                               \
                sendChar( ch, s );                            \
                return;                                       \
            }/* if */

#define IF_CANT_INITIATE_PKILL_WITH_THIS( s )                     \
            if( !IS_NPC( victim ) && ( victim != FIGHTING(ch)) ){ \
                sendChar( ch, s );                                \
                return;                                           \
            }/* if */

#define IF_CH_IS_WIELDING( s )                 \
            if( WIELDING(ch) ){ \
                sendChar( ch, s );             \
                return;                        \
            }/* if */

#define IF_CH_NOT_WIELDING( s )                 \
            if( !WIELDING(ch)){ \
                sendChar( ch, s );              \
                return;                         \
            }/* if */

#define IF_VICTIM_NOT_STANDING( s )               \
            if( GET_POS(victim) < POS_FIGHTING ){ \
                sendChar( ch, s );                \
                return;                           \
            }/* if */

#define IF_VICTIM_NOT_WIELDING( s )                 \
            if( !WIELDING(victim) ){ \
                sendChar( ch, s );                  \
                return;                             \
            }/* if */

#define IF_VICTIM_CANT_BE_FIGHTING( s ) \
            if FIGHTING( victim ){      \
                sendChar( ch, s );      \
                return;                 \
            }/* if */

#define IF_ROOM_IS_PEACEFUL( s )   \
            if( IS_SET_AR( ROOM_FLAGS( ch->in_room ), ROOM_PEACEFUL )){ \
                sendChar( ch, s ); \
                return;            \
            }

#define IF_UNLEARNED_SKILL( s )                               \
            if(!IS_NPC(ch) && !GET_SKILL( ch, THIS_SKILL )){  \
                sendChar( ch, s );                            \
                return;                                       \
            }

#define IF_CANT_HURT_VICTIM \
	if (chIsNonCombatant(victim)) { \
		sendChar(ch, "%s is a non-combatant, you can't attack them.\r\n", GET_NAME(victim)); \
		return; \
	} \

#define IF_CANT_HIT_FLYING \
	if (SINGING(ch) == SPELL_FLY) { \
		sendChar(ch, "%s is flying out of your range.\r\n", GET_NAME(victim)); \
		return; \
	} \

#define IS_RACIAL(ch) (IS_NPC((ch)) && percentSuccess(GET_MOB_SKILL_SUC((ch))))

/*
** STUNNING Macros
*/
#define SET_STUN(s)          ((PULSE_VIOLENCE * s))

#define STUN_USER_MIN        WAIT_STATE( ch,     SET_STUN( STUN_MIN ))
#define STUN_USER_MAX        WAIT_STATE( ch,     SET_STUN( STUN_MAX ))
#define STUN_USER_RANGE      WAIT_STATE( ch,     SET_STUN(number( STUN_MIN, STUN_MAX )))

#define STUN_VICTIM_MIN      WAIT_STATE( victim, SET_STUN( STUN_MIN ))
#define STUN_VICTIM_MAX      WAIT_STATE( victim, SET_STUN( STUN_MAX ))
#define STUN_VICTIM_RANGE    WAIT_STATE( victim, SET_STUN(number( STUN_MIN, STUN_MAX )))



/*
** Liam Feb 1, 1995
**
** Maximum percentage chance a mob can have for skills.  Ie. the maximum
** percent to which a mob can "learn" a skill... 
*/
#define	MAX_MOB_SKILL_PERCENT	95

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      advanceSkill -
**
**  FORMAL PARAMETERS:
**
**      ch:
**          A pointer to the character structure that is trying to use the skill/spell.
**
**      skill_id:
**          The skill that is being used (ie, SKILL_BASH, SKILL_DISARM).
**
**  RETURN VALUE:
**
**      None
**
**  DESIGN:
**
**      Rather than look at the same definition of this code OVER and OVER again it's
** 
*/
extern void
advanceSkill( struct char_data *ch,
              int skill_id,
              int max_learn,
              char *advancement_string,
              int dex_affect, int int_affect, int wis_affect );

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      competenceDescription -
**
**  FORMAL PARAMETERS:
**
**      learned:
**          The percentage that a skill has been learned.
**
**  RETURN VALUE:
**
**      Character pointer to a static string to display.
**
**  DESIGN:
**
**      This
** 
*/
extern char
*competenceDescription( int );

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      skillSuccess -
**
**  FORMAL PARAMETERS:
**
**      ch:
**          A pointer to the character structure that is trying to use the skill/spell.
**
**      skill_id:
**          The skill that is being used (ie, SKILL_BASH, SKILL_DISARM).
**
**  RETURN VALUE:
**
**      Boolean
**
**  DESIGN:
**
**      Rather than look at the same definition of this code OVER and OVER again it's
**      been made into a function that will make the skill code more readable and generic.
**      Basically, this function is called with the character record and the skill id
**      and then the roll is made and compared. If the random roll is higher than the
**      char's learned ability then the skill failed and a false is returned.
** 
*/
extern int skillSuccess( struct char_data *ch, int skill_id );

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      victimAvoidsSkill -
**
**  FORMAL PARAMETERS:
**
**      ch:
**          A pointer to the character structure that is trying to use the skill/spell.
**
**      victim:
**          A pointer to the character structure that is the target of the skill.
**
**      skill_id:
**          The skill that is being used (ie, SKILL_BASH, SKILL_DISARM).
**
**  RETURN VALUE:
**
**      Boolean
**
**  DESIGN:
**
**      In order to add an enhanced version of combat we must check for other parameters
**	during skill usage. A victim's level, strength, wisdom, dexterity, intelligence,
**	may all play a role in avoiding a bash or a trip.
** 
*/
extern int
victimAvoidsSkill( struct char_data *ch, struct char_data *victim, int skill_id );

extern int
raceSkillAvoid( CharData *ch, CharData *vict, int the_skill);

extern int victimIsAngry( int chanceToAnger );
extern int artMonkey( CharData *ch, CharData *victim, int skill_id );
extern int equipmentSkillSuccess(CharData *ch);

#endif
