/*
**++
**  RCSID:     $Id: shoot.c,v 1.4 2001/02/12 13:14:22 raven Exp $
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
**      Digger from RavenMUD (Hastily thrown together template :)
**        Fleee from RavenMUD
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
**  $Log: shoot.c,v $
**  Revision 1.4  2001/02/12 13:14:22  raven
**  scripting
**
**  Revision 1.2  2000/10/10 13:47:04  raven
**
**  Transitioned over to the new include structures.
**
**  Revision 1.1.1.1  2000/10/10 04:15:17  raven
**  RavenMUD 2.0
**
**  Revision 1.14  2000/04/05 23:19:20  mortius
**  fixed the problem with peacerooms for shoot, can't shoot in or out of them
**  now.
**
**  Revision 1.13  1998/01/29 03:38:48  digger
**  Removed all references to BLOOD_ENABLED
**
**  Revision 1.12  1997/12/07 15:54:27  vex
**  Cleanup crew came through.
**
**  Revision 1.11  1997/09/26 05:32:30  vex
**  I couldn't see anything wrong with this skill, so I've removed the
**  check that only allowed LVL_IMMORT to use it.
**
**  Revision 1.10  1997/09/18 12:52:36  vex
**  world was declared above the inclusion of structs.h, which caused a compiler
**  error after I switched everything to typdefs.
**
**  Revision 1.9  1997/09/18 11:04:15  vex
**  Replaced all obj_data, room_data, mob_special_data, char_data,
**  descriptor_data structs with appropriate typedef.
**
**  Revision 1.8  1997/05/08 02:09:35  liam
**  Removed the BLOODCHK
**
**  Revision 1.7  1997/01/03 12:32:45  digger
**  Renamed several of the functions from skills.c and added skill
**  avoidance to fist and hamstring. Vex has put in MAJOR changes
**  to the summoning code and many checks for Book of Blood signatures
**  were added.
**
**  Revision 1.6  1996/02/21 12:33:15  digger
**  Added the IF_UNLEARNED_SKILL check.
**
** Revision 1.5  1996/01/03  23:07:19  digger
** Checkpointing.
**
**  Revision 1.4  1995/07/26 20:48:25  digger
**  Removed the unused range var.
**
**  Revision 1.3  1994/12/19 22:18:34  moon
**  Added a check so that you couldn't shoot yourself.
**
*/


/*
** STANDARD U*IX INCLUDES
*/
#include "general/conf.h"
#include "general/sysdep.h"

#define THIS_SKILL                SKILL_SHOOT

#define SKILL_ADVANCES_WITH_USE   TRUE
#define SKILL_ADVANCE_STRING      "You've become notably more proficient."


/*                                Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  XX  XX  XX  XX  XX  XX  XX  XX  XX  XX */
static int max_skill_lvls[] =   { 00, 00, 75, 84, 94, 75, 00, 00, 00, 85, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 };
#define SKILL_MAX_LEARN           max_skill_lvls[ (int)GET_CLASS(ch) ]

#define DEX_AFFECTS               TRUE
#define INT_AFFECTS               FALSE
#define WIS_AFFECTS               FALSE

#define STUN_MIN                  2
#define STUN_MAX                  2
#define MOVE_PER_SHOOT            5
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
#include "magic/missile.h"
#include "specials/special.h"
/*
** EXTERNAL DEFINITIONS OF SKILLS/SPELLS CALLED FROM THIS FILE
**
**
*/


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      SKILL_NAME - SKILL_SHOOT
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
**      Skill for firing different types of missile weapons, such as bows, crossbows, slings, and so on.
**
*/
ACMD(do_shoot)
{
    char dir[255], target[255], weapon[255], temp[255];
    int dir_num = 0;
    CHAR_DATA *victim = 0;
    OBJ_DATA *wpn = 0;
    OBJ_DATA *ammo = 0;
    char buf[MAX_INPUT_LENGTH];

    static const char *throw_directions[] = { "north", "east", "south", "west", "up", "down", "\n"};
 
    IF_UNLEARNED_SKILL( "You wouldn't know where to begin.\r\n" );

    half_chop(argument, weapon, temp);

    two_arguments(temp, target, dir);

    if(FIGHTING(ch)) {
        if(GET_CLASS(ch) != CLASS_RANGER) {
            send_to_char("You are not trained to shoot while in combat.", ch);
            return;
        }
        else {
            victim = FIGHTING(ch);
        }
    }
    

    if( !*weapon || (!*target && !FIGHTING(ch))) {
        send_to_char( "Usage : shoot <ammo> <target> [direction] \n\r", ch );
        return;
    }

    if( !( wpn = ch->equipment[WEAR_HOLD] ) || wpn->obj_flags.type_flag != ITEM_FIREWEAPON) {
        send_to_char( "You must be holding something to shoot with!\n\r", ch );
        return;
    }
 
    if( !(ammo = get_obj_in_list_vis(ch, weapon, ch->carrying)) ){
        send_to_char( "What ammunition did you say you were using?\n\r", ch );
        return;
    }

    if( ammo->obj_flags.value[0] != wpn->obj_flags.value[0] ){
        send_to_char( "You cannot use that ammunition with your weapon.\n\r", ch );
        return;
    }

    // Enforce uselevel
    int modifier = 0;
    int j;
    for (j=0; j<MAX_OBJ_AFFECT; j++)
        if ((ammo->affected[j].location == APPLY_USELEVEL) && (modifier == 0) )
            modifier=ammo->affected[j].modifier;
    if ((modifier<0) && (GET_LEVEL(ch)>(-modifier))) { // Char too high level
        act("You are too powerful for $p and lets go of it.", FALSE, ch, ammo, 0, TO_CHAR);
        return;
    } else if ((modifier>0) && (GET_LEVEL(ch)<modifier)) { // Char is too low level
        act("You are not powerful enough for $p and let go of it.", FALSE, ch, ammo, 0, TO_CHAR);
        return;
    }

    if( !IS_NPC(ch) &&    /* Lets not have mobs zapped by their own stuff. */
            ((IS_OBJ_STAT(ammo, ITEM_ANTI_EVIL) && IS_EVIL(ch)) ||
            (IS_OBJ_STAT(ammo, ITEM_ANTI_GOOD) && IS_GOOD(ch)) ||
            (IS_OBJ_STAT(ammo, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch)) ||
            invalid_race(ch, ammo) ||
            invalid_class(ch, ammo))) {
        act("You are zapped by $p and instantly let go of it.", FALSE, ch, ammo, 0, TO_CHAR);
        return;
    }
    
    if( !*dir && !(victim = get_char_room_vis( ch, target )) && !(victim = FIGHTING(ch)) ){
        send_to_char( "Shoot at what?\n\r", ch );
        return;
    }

    if( *dir && ( dir_num = search_block( dir, throw_directions, FALSE )) < 0 ){
        send_to_char( "What direction???\n\r", ch );
        return;
    }

    if( !victim && !(victim = find_vict_dir( ch, target, wpn->obj_flags.value[1], dir_num )) ){
        send_to_char( "You cannot see that in range.\n\r", ch );
        return;
    }
    
    if (ROOM_FLAGGED(IN_ROOM(victim), ROOM_PEACEFUL) ||
            ROOM_FLAGGED(IN_ROOM(ch), ROOM_PEACEFUL)) {
        send_to_char("A flash of white light fills the room, dispelling your violent deed!\r\n", ch);
        return;
    }

    if(FIGHTING(ch) && FIGHTING(ch) != victim) {
        send_to_char("You cannot take focus from the fight at hand.\r\n", ch);
        return;
    }

    IF_CH_CANT_BE_VICTIM("You can't shoot yourself.\n\r");
    IF_CANT_HURT_VICTIM;

    if (GET_MOVE(ch) < MOVE_PER_SHOOT )
    {
        send_to_char("You are too tired to shoot straight!\r\n", ch);
        return;
    }

    if( skillSuccess( ch, THIS_SKILL )){
        if( ch->in_room == victim->in_room ){
            act("You fire $p towards $N.", FALSE, ch, wpn, victim, TO_CHAR);
            act("$n fires $p towards you.", FALSE, ch, wpn, victim, TO_VICT);
            act("$n fires $p towards $N.", FALSE, ch, wpn, victim, TO_NOTVICT);
            set_fighting(ch, victim);
        }

        else {
            sprintf(buf, "You fire $p %swards towards $N.", throw_directions[dir_num]);
            act(buf, FALSE, ch, wpn, victim, TO_CHAR);
            sprintf(buf, "$n fires $p %swards.", throw_directions[dir_num]);
            act(buf, FALSE, ch, wpn, victim, TO_ROOM);
        }

        advanceSkill( ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING, DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS );
        gen_missile(ch, victim, ammo, 0, dir_num, MISSILE_TYPE_MISSILE);

        if( ammo->obj_flags.value[3] > 0 ) ammo->obj_flags.value[3]--;
        if( ammo->obj_flags.value[3] == 0) extract_obj(ammo);

    }/* if */
    else {
        sprintf( buf, "You fumble with %s, trying to put it the right way up.\n\r", wpn->short_description );
        send_to_char( buf, ch );
    }/* else */

    if( wpn->obj_flags.value[2] )
        WAIT_STATE(ch, PULSE_VIOLENCE * wpn->obj_flags.value[2]);
    else
        STUN_USER_MAX;
    
    GET_MOVE(ch) -= MOVE_PER_SHOOT;
    GET_MOVE(ch) = MAX(0, GET_MOVE(ch));
    add_affect( ch, ch, SKILL_SHOOT, 0, APPLY_AC, 30,
            wpn->obj_flags.value[2], 0, FALSE, FALSE, FALSE, FALSE);
}/* do_SKILL */
