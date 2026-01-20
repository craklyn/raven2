
/*
**++
**  RCSID:     $Id: envenom.c,v 1.5 2004/07/21 23:15:38 raven Exp $
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
**  $Log: envenom.c,v $
**  Revision 1.5  2004/07/21 23:15:38  raven
**  current code state
**
**  Revision 1.4  2001/08/16 02:40:06  raven
**  duration para fix, added escape to parser list
**
**  Revision 1.3  2001/08/16 00:19:10  raven
**  assassin branch merged
**
**  Revision 1.2.2.1  2001/08/15 05:02:17  raven
**  Corrections to level usage, stun and movement cost
**
**  Revision 1.2  2000/10/10 13:47:04  raven
**
**  Transitioned over to the new include structures.
**
**  Revision 1.1.1.1  2000/10/10 04:15:17  raven
**  RavenMUD 2.0
**
**  Revision 1.6  1997/09/26 05:32:30  vex
**  Code clean up.
**
**  Revision 1.5  1997/09/18 12:52:36  vex
**  world was declared above the inclusion of structs.h, which caused a compiler
**  error after I switched everything to typdefs.
**
**  Revision 1.4  1997/09/18 10:51:02  vex
**  Replaced all obj_data, room_data, mob_special_data, char_data,
**  descriptor_data structs with appropriate typedef.
**
**  Revision 1.3  1997/01/03 12:32:45  digger
**  Renamed several of the functions from skills.c and added skill
**  avoidance to fist and hamstring. Vex has put in MAJOR changes
**  to the summoning code and many checks for Book of Blood signatures
**  were added.
**
**  Revision 1.2  1996/02/21 12:33:57  digger
**  Added the IF_UNLEARNED_SKILL check.
**
 * Revision 1.1  1995/03/22  14:16:24  raven
 * Initial revision
 *
 * Revision 1.1  1995/02/14  04:39:57  raven
 * Initial revision
 *
 * Revision 1.1  1994/12/16  14:23:52  jbp
 * Initial revision
 *
**
*/


/*
** STANDARD U*IX INCLUDES
*/
#include "general/conf.h"
#include "general/sysdep.h"

#define THIS_SKILL                SKILL_ENVENOM

#define SKILL_ADVANCES_WITH_USE   FALSE
#define SKILL_ADVANCE_STRING      "You've become notably more proficient."
#define SKILL_MAX_LEARN           90
#define DEX_AFFECTS               FALSE
#define INT_AFFECTS               FALSE
#define WIS_AFFECTS               FALSE

#define MOVE_PER_MISSILE            20
#define MOVE_PER_WEAPON             15
#define STUN_MIN                  2
#define STUN_MAX                  4

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
**      SKILL_NAME - envenom
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
**      This skill enables the user to envenom a weapon that he/she carries,
**      so that next time it strikes a victim, that victim may be poisoned.
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
ACMD(do_envenom)
{
    char   arg[100];
    ObjData *blade;
    int aff_val;
#define NO_APPLY APPLY_NONE
#define NO_BIT   0
#define NO_MOD   0

    IF_UNLEARNED_SKILL( "You wouldn't know where to begin.\r\n" );

    one_argument( argument, arg );
    if (GET_MOVE(ch) < MOVE_PER_MISSILE)
    {
        send_to_char("You are too tired to think about handling poison.\n\r", ch);
        return;
    }

    if (!(blade = get_obj_in_list_vis(ch, arg, ch->carrying)))
    {
        send_to_char("What do you wish to envenom?\n\r", ch);
        return;
    }
    
    if (GET_OBJ_TYPE(blade) != ITEM_WEAPON && GET_OBJ_TYPE(blade) != ITEM_MISSILE)
    {
        send_to_char("Only weapons or missiles can be envenomed.\n\r", ch);
        return;
    }

    for (aff_val = 0; aff_val < MAX_OBJ_AFFECT; aff_val++)
        if (blade->affected[aff_val].location == APPLY_NONE)
            break;
    
    if (aff_val == (MAX_OBJ_AFFECT) && blade->affected[aff_val-1].location != APPLY_NONE)
    {
        send_to_char("This weapon is not suitable for envenoming.\n\r", ch);
        return;
    }

    // If the blade is not owned by this player, ch may not envenom it.
    // If this blade is not owned, ch now owns it.
    if(has_owner(blade) && !owns_item(ch, blade))
    {
        send_to_char("You may not envenom another's blade.\r\n", ch);
        return;
    }
    else if (!IS_NPC(ch) && !has_owner(blade))
        set_owner(ch, blade);

    if( skillSuccess( ch, THIS_SKILL )){
        advanceSkill( ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING, DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS );
        
        act("You coat $p with a vile, green poison!\n\r", FALSE, ch, blade, 0, TO_CHAR);
        blade->affected[aff_val].location = APPLY_POISON;
        blade->affected[aff_val].modifier = GET_LEVEL(ch);
    }/* if */
    else {

        if (mag_savingthrow(ch, SAVING_PARA))
        {
            send_to_char("You narrowly avoid poisoning yourself.\n\r", ch);
            return;
        }
        else
        {
            if( GET_LEVEL(ch) >= 35){
                add_affect( NULL, ch, SPELL_POISON, 0, APPLY_STR, -2, 4 TICKS, AFF_POISON,
                        FALSE, FALSE, FALSE, FALSE);
                act("You suddenly feel ill!", FALSE, ch, 0, 0, TO_CHAR);
                act("$n turns pale and looks deathly sick!", FALSE, ch, 0, 0, TO_ROOM);
            }
            if( GET_LEVEL(ch) >= 43){
                add_affect( ch, ch, SPELL_BLINDNESS, 0, NO_APPLY, NO_MOD, 2 TICKS, AFF_BLIND,
                        FALSE, FALSE, FALSE, FALSE);
                act("Your vision fades to black.", FALSE, ch, 0, 0, TO_CHAR);
                act("$n seems to be blinded!", FALSE, ch, 0, 0, TO_ROOM);
            }
            if( GET_LEVEL(ch) >=48){
                add_affect( ch, ch, SPELL_PARALYZE, 0, APPLY_AC, 20, 1 TICKS, AFF_PARALYZE,
                        FALSE, FALSE, FALSE, FALSE);
                act("Your limbs stiffen, immobilizing you.", FALSE, ch, 0, 0, TO_CHAR);
                act("$n's limbs stiffen, immobilizing them.", FALSE, ch, 0, 0, TO_ROOM);
            }
            
        }
    }/* else */
    if (GET_OBJ_TYPE(blade) == ITEM_WEAPON){
        STUN_USER_MIN;
        GET_MOVE(ch) -= MOVE_PER_WEAPON;
    }
    if (GET_OBJ_TYPE(blade) == ITEM_MISSILE){
        STUN_USER_MAX;
        GET_MOVE(ch) -= MOVE_PER_MISSILE;
    }
}/* do_SKILL */
