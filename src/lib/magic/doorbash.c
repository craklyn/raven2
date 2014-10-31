/*
**++
**  RCSID:     $Id: doorbash.c,v 1.3 2001/08/04 06:59:06 raven Exp $
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
**  $Log: doorbash.c,v $
**  Revision 1.3  2001/08/04 06:59:06  raven
**  timed scrolls
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
**  Revision 1.4  1997/09/18 10:49:53  vex
**  Replaced all obj_data, room_data, mob_special_data, char_data,
**  descriptor_data structs with appropriate typedef.
**
**  Revision 1.3  1997/05/08 02:14:27  liam
**  Prevented doorbash working on unpickable doors.
**  Vex.
**
**  Revision 1.2  1997/01/03 12:32:45  digger
**  Renamed several of the functions from skills.c and added skill
**  avoidance to fist and hamstring. Vex has put in MAJOR changes
**  to the summoning code and many checks for Book of Blood signatures
**  were added.
**
**  Revision 1.1  1996/09/17 17:59:21  digger
**  Initial revision
**
**
*/



/*
** STANDARD U*IX INCLUDES
*/
#include "general/conf.h"
#include "general/sysdep.h"

#define THIS_SKILL                SKILL_DOORBASH

#define SKILL_ADVANCES_WITH_USE   TRUE
#define SKILL_ADVANCE_STRING      "Doors seem to quake in your presence."

/*                                Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  XX  XX  XX  XX  XX  XX  XX  XX  XX  XX */
static int max_skill_bash[] =   { 00, 00, 00, 94, 30, 00, 00, 50, 50, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 };
#define SKILL_MAX_LEARN           max_skill_bash[ (int)GET_CLASS(ch) ]

#define DEX_AFFECTS               TRUE
#define INT_AFFECTS               FALSE
#define WIS_AFFECTS               FALSE

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

/*
** EXTERNAL DEFINITIONS OF SKILLS/SPELLS CALLED FROM THIS FILE
**
**
*/
extern int find_door(CharData * ch, char *type, char *dir);


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
ACMD(do_doorbash)
{
    char   dir[100];
    int    door;
    int    other_room;
    struct room_direction_data *back;

    IF_UNLEARNED_SKILL( "You wouldn't know where to begin.\r\n" );

    one_argument( argument, dir );

    if( !*dir ){
        send_to_char( "Which direction ?\n\r", ch );
        return;
    }

    if(( door = find_door(ch, "door", dir )) < 0 ) return;

    if( !IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR )){
        send_to_char("You bounce off the wall and fall flat on your back.\r\n", ch);
        act( "$n bouces off a wall and crashes to the ground.\n\r", FALSE, ch, 0, 0, TO_ROOM);
        GET_POS(ch) = POS_SITTING;
        STUN_USER_MAX;
    }

    else if( !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED )){
        send_to_char("It's already open!\r\n", ch);
        return;
    }
    /*
    ** Stopped people bashing unpickable doors open... Vex.
    */
    else if( IS_SET(EXIT(ch, door)->exit_info, EX_PICKPROOF )){
        sendChar( ch, "That door is far to strong for you to bash open.\r\n" );
        return;
    }
    /*
    ** Stop them from taking it to TM in a day.
    */
    else if( IS_SET(EXIT(ch, door)->exit_info, EX_DOORBASHED )){
        sendChar( ch, "That door is far to rickety to bash open again.\r\n" );
        return;
    }

    else if( GET_MOVE(ch) < 3 ){
        sendChar( ch, "You are too tired to bash down a door.\r\n" );
        return;
    }

    GET_MOVE(ch) -= 3;

    if( skillSuccess( ch, THIS_SKILL )){
        if( SKILL_ADVANCES_WITH_USE ){
            advanceSkill( ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING, DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS );
        }
        REMOVE_BIT(EXIT(ch, door)->exit_info, EX_CLOSED);
        SET_BIT(EXIT(ch, door)->exit_info, EX_DOORBASHED);
        send_to_char( "You crash into a door sending it flying open on its hinges.\r\n", ch );
        act( "$n crashes into the door sending it flying open!\r\n", FALSE, ch, 0, 0, TO_ROOM);
        STUN_USER_MIN;

        /* open the door on the other side */
        if ((other_room = EXIT(ch, door)->to_room) != NOWHERE)
          if ((back = world[other_room].dir_option[rev_dir[door]]))
            if (back->to_room == ch->in_room) {
              REMOVE_BIT(back->exit_info, EX_CLOSED);
              if (back->keyword) {
                sprintf(buf, "The %s flies open suddenly!\r\n",
                    fname(back->keyword));
                send_to_room(buf, -EXIT(ch, door)->to_room);
              } else
                send_to_room("The door flies open suddenly.\r\n",
                    -EXIT(ch, door)->to_room);
            }
    }/* if */

    else {
        send_to_char( "You bounce off the door and go crashing to the floor.\n\r", ch );
        act( "$n bounces off of a closed door and lands flat on $s back.\n\r", FALSE, ch, 0, 0, TO_ROOM);
        GET_POS(ch) = POS_SITTING;
        STUN_USER_MAX;
    }/* else */

}/* do_doorbash */

