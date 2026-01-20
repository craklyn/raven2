/*
**++
**  RCSID:     $Id: familiarize.c,v 1.1 2001/05/21 04:49:13 raven Exp $
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
**      Memnoch from RavenMUD
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
**  $Log: familiarize.c,v $
**  Revision 1.1  2001/05/21 04:49:13  raven
**  familliarize -> familiarize
**
**  Revision 1.1  2001/05/20 12:47:26  raven
**  new files
**
**  Revision 1.2  2000/10/10 13:47:04  raven
**
**  Transitioned over to the new include structures.
**
**  Revision 1.1.1.1  2000/10/10 04:15:17  raven
**  RavenMUD 2.0
**
**  Revision 1.6  1998/01/29 03:38:48  digger
**  Removed all references to BLOOD_ENABLED
**
**  Revision 1.5  1997/09/26 05:32:30  vex
**  Code clean up.
**
**  Revision 1.4  1997/09/18 11:00:44  vex
**  Replaced all obj_data, room_data, mob_special_data, char_data,
**  descriptor_data structs with appropriate typedef.
**
**  Revision 1.3  1997/01/03 12:32:45  digger
**  Renamed several of the functions from skills.c and added skill
**  avoidance to fist and hamstring. Vex has put in MAJOR changes
**  to the summoning code and many checks for Book of Blood signatures
**  were added.
**
**
*/


/*
** STANDARD U*IX INCLUDES
*/
#include "general/conf.h"
#include "general/sysdep.h"

#define THIS_SKILL                SKILL_FAMILIARIZE

#define SKILL_ADVANCES_WITH_USE   TRUE
#define SKILL_ADVANCE_STRING      "You feel more at home."

/*
** SKILL_MAX_LEARN 
**         Define as either a hard number or use the skill/class array
**         scheme to allow certain classes to learn particular skills
**         better than others.
**
** define SKILL_MAX_LEARN         90
**
*/

/*                                Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  XX  XX  XX  XX  XX  XX  XX  XX  XX  XX */
static int max_skill_lvls[] =   { 00, 90, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 };
#define SKILL_MAX_LEARN           max_skill_lvls[ (int)GET_CLASS(ch) ]

/*
** If dex, int, or wis affects the speed at which this skill is learned then
** set the following macros to TRUE. This means the user will learn the skill
** a little faster based on those attributes.
*/
#define DEX_AFFECTS               FALSE
#define INT_AFFECTS               TRUE
#define WIS_AFFECTS               TRUE
/*
** These are the two macros that define the range of the stun duration for
** an executed skill. These values are used for both the char and the victim.
*/
#define STUN_MIN                  1
#define STUN_MAX                  3

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

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      SKILL_NAME - [Familiarize]
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
**  NOTES:
**      The following standard macros can be used from skills.h:
**
**          IF_CH_CANT_SEE_VICTIM( string );    IF_VICTIM_NOT_WIELDING( string );
**          IF_CH_CANT_BE_VICTIM( string );     IF_VICTIM_CANT_BE_FIGHTING( string );
**          IF_CH_NOT_WIELDING( string );       IF_VICTIM_NOT_WIELDING( string );
**          IF_ROOM_IS_PEACEFUL( string );      IF_VICTIM_NOT_STANDING( string );
**          IF_UNLEARNED_SKILL( string );
**
**      The parameter `string' is the string that is sent to the user when the
**      error condition is met and the skill function exits. For example when an
**      attempt to disarm a weaponless victim is made:
**
**          IF_VICTIM_NOT_WIELDING( "That person isn't even wielding a weapon!\r\n" );
**
*/
ACMD(do_familiarize) {
    if( skillSuccess( ch, THIS_SKILL )){
      if(IS_SET_AR( ROOM_FLAGS( IN_ROOM(ch) ), ROOM_FAMILIAR)){
        advanceSkill( ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING, DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS );      
        STUN_USER_MIN;
        GET_RECALL(ch) = world[ch->in_room].number;
        send_to_char("You glance around, familiarizing yourself with this place.\r\n", ch);
        act("$n looks around carefully.", TRUE, ch, NULL, NULL, TO_ROOM);
        }
      else {
	    send_to_char("You glance around carefully, but can't seem to find any landmarks.\r\n", ch);
        STUN_USER_MAX;
      }       
  }


    else {
	  send_to_char("You glance around carefully, but can't seem to find any landmarks.\r\n", ch);
      GET_MANA(ch) -= number(5, 10);
      STUN_USER_MAX;
    }
    
}/* do_SKILL */

