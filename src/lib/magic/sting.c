/*
 **++
 **  RCSID:     $Id: sting.c,v 1.2 2003/09/30 04:51:50 raven Exp $
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
 **      Elendil from RavenMUD
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
 **  $Log: sting.c,v $
 **  Revision 1.2  2003/09/30 04:51:50  raven
 **  more necromancerly goodness
 **
 **  Revision 1.1  2002/07/12 05:22:31  raven
 **  remort code; bugfixes to curse and movement guards
 **
 **
 */


/*
 ** STANDARD U*IX INCLUDES
 */
#include "general/conf.h"
#include "general/sysdep.h"

#define THIS_SKILL                SKILL_STING

#define SKILL_ADVANCES_WITH_USE   TRUE
#define SKILL_ADVANCE_STRING      "You've become notably more proficient."
#define SKILL_MAX_LEARN           90
#define DEX_AFFECTS               TRUE
#define INT_AFFECTS               FALSE
#define WIS_AFFECTS               FALSE

#define STUN_MIN                  2
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
#include "specials/special.h"
#include "actions/fight.h"
#include "actions/act.h"          /* ACMDs located within the act*.c files */

/*
 ** MUD SPECIFIC GLOBAL VARS
 */

/*
 ** EXTERNAL DEFINITIONS OF SKILLS/SPELLS CALLED FROM THIS FILE
 **
 **
 */

/*
 **++
 **  FUNCTIONAL DESCRIPTION:
 **
 **      SKILL_NAME - sting
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
 **      The skill enables the user to sting their opponent, causing damage and
 **      attempting to land a poison.
 **
 **
 **  NOTES:
 **      The following standard macros can be used from skills.h:
 **
 **          IF_CH_CANT_SEE_VICTIM( string );             IF_VICTIM_NOT_WIELDING( string );
 **          IF_CH_CANT_BE_VICTIM( string );              IF_VICTIM_CANT_BE_FIGHTING( string );
 **          IF_CH_NOT_WIELDING( string );
 **
 **      The parameter `string' is the string that is sent to the user when the
 **      error condition is met and the skill function exits. For example when an
 **      attempt to disarm a weaponless victim is made:
 **
 **          IF_VICTIM_NOT_WIELDING( "That person isn't even wielding a weapon!\r\n" );
 **
 */
ACMD(do_sting) {
    char arg[100];
    CharData *victim;

    one_argument(argument, arg);

    IF_UNLEARNED_SKILL("You pretend to be a rock musician.\r\n");
    IF_CH_CANT_SEE_VICTIM("Sting who?\r\n");
    IF_CH_CANT_BE_VICTIM("You don't want to do that.\r\n");
    IF_ROOM_IS_PEACEFUL("You viciously flick their ear.\r\n");
    IF_CANT_HURT_VICTIM;
    
    if (skillSuccess(ch, THIS_SKILL) || IS_RACIAL(ch)) {
        advanceSkill(ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING, DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS);

        // This skill was way underpowered for amaras, lets give it some spice! - Bean
        if (mag_savingthrow(victim, SAVING_SPELL)) {
            add_affect(ch, victim, SPELL_POISON, GET_LEVEL(ch), APPLY_NONE, 10 TICKS,
                    GET_LEVEL(ch), AFF_POISON, FALSE, FALSE, FALSE, FALSE);
            act("$n turns pale and looks deathly sick!",
                    FALSE, victim, 0, 0, TO_ROOM);
            act("You suddenly feel ill!", FALSE, victim, 0, 0, TO_CHAR);
        } 
        else if ((GET_LEVEL(ch) >= 45) && dice(1, 100) <= 10) {
                add_affect(ch, victim, SPELL_PARALYZE, 0, APPLY_AC, 20, 1 TICKS, AFF_PARALYZE,
                        FALSE, FALSE, FALSE, FALSE);
                act("$n's limbs stiffen, immobilizing them.", FALSE, victim, 0, 0, TO_ROOM);
                act("Your limbs stiffen as a burning deep within your veins takes hold.", FALSE, victim, 0, 0, TO_CHAR);
        }
        damage(ch, victim, dice(2, GET_LEVEL(ch)), THIS_SKILL);
    } else {
        damage(ch, victim, 0, THIS_SKILL);
    }/* else */

    STUN_USER_MIN;

}/* do_SKILL */

