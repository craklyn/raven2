
/*
** File (132 column edit): $Id: scan.c,v 1.6 2003/08/29 00:00:56 raven Exp $
**
** $Log: scan.c,v $
**
** Revision 1.5  2003/08/28 19:47:45  raven
** forgot a newline
**
** Revision 1.4  2003/08/28 17:05:39  raven
** changes to sunblind, and to drow in general
**
** Revision 1.3  2002/04/13 08:12:04  raven
** demote command, wa/ra revisions
**
** Revision 1.2  2000/10/10 13:47:04  raven
**
** Transitioned over to the new include structures.
**
** Revision 1.1.1.1  2000/10/10 04:15:17  raven
** RavenMUD 2.0
**
** Revision 1.1  2000/09/09 19:11:27  mortius
** Initial revision
**
** Revision 1.11  2000/04/28 14:19:19  mortius
** Added a GET_SKILL check to make sure the player can access the skill rather
** then display 4 blank lines.  Also added a blind check.
**
** Revision 1.10  1998/07/20 01:02:08  vex
** Fixed scan w/o arguments so it would show scan data in all valid
** directions.
**
** Revision 1.9  1998/07/20 00:32:29  vex
** Added () around aggro mobs on scan so people w/o colour could see...
**
** Revision 1.8  1998/05/23 03:11:03  vex
** Colored aggro mobs red on scan if heightened senses skill succceeds.
**
** Revision 1.7  1998/03/25 12:34:50  digger
** Added FOG.
**
** Revision 1.6  1997/09/26 05:32:30  vex
** Code clean up.
**
** Revision 1.5  1997/09/18 11:03:01  vex
** Replaced all obj_data, room_data, mob_special_data, char_data,
** descriptor_data structs with appropriate typedef.
**
** Revision 1.4  1997/01/03 12:32:45  digger
** Renamed several of the functions from skills.c and added skill
** avoidance to fist and hamstring. Vex has put in MAJOR changes
** to the summoning code and many checks for Book of Blood signatures
** were added.
**
** Revision 1.3  1996/02/21  12:34:30  digger
** Added the IF_UNLEARNED_SKILL check.
**
** Revision 1.2  1995/08/01  12:51:27  digger
** Fixed a type mismatch with the NULL pointer.
**
** Revision 1.1  1994/12/07  20:40:44  digger
** Initial revision
**
*/

/*
** STANDARD U*IX INCLUDES
*/
#include "general/conf.h"
#include "general/sysdep.h"

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
#include "magic/sing.h"

/*
** EXTERNAL DEFINITIONS OF SKILLS/SPELLS CALLED FROM THIS FILE
**
*/

/*
**
*/
void do_scan(CharData *ch, char *argument, int cmd, int subcmd)
{
    static const char *scan_directions[] = { "north",        "east",        "south",        "west",        "up",        "down", "\n"};
    static char *modifier[]        = { "to the north", "to the east", "to the south", "to the west", "above you", "below you" };
    static char *horizontal[]      = { "Slightly", "Nearby",   "Off",  "Far off" };
    static char *vertical[]        = { "Directly", "Slightly", "Well", "Far" };

    int   scan_dir                 = NOWHERE;
    char  arg[100];
    char  dir_arg[100];
    int i;
#   define SCAN_MAX                  (sizeof( horizontal ) / sizeof( horizontal[0]))
#   define CAN_SCAN( room, door ) \
        ((world[room].dir_option[door]) &&                     \
         (world[room].dir_option[door]->to_room != NOWHERE) && \
         (!IS_SET(world[room].dir_option[door]->exit_info, EX_CLOSED)))

    one_argument(argument, arg);

/* Mortius : If your blind or don't have scan you don't get to see anything */

    if (!GET_SKILL(ch, SKILL_SCAN)) {
        send_to_char("You try to scan the area but see nothing.\r\n", ch);
        return;
    }

    if (IS_AFFECTED(ch, AFF_BLIND)) {
        send_to_char("You are blind and unable to see anything.\r\n", ch);
        return;
    }

/* Sanji : If you're sunblind, you can't use scan */

    if (IS_SUNBLIND(ch)) {
        send_to_char("The sunlight is too bright for you to see anything.\r\n", ch);
        return;
    }
    
   if (ZONE_FLAGGED(world[ch->in_room].zone, ZONE_SLEEPTAG)) {
       send_to_char("Naw, not in here.\r\n", ch);
       return;
   }

    if( !*arg ){
#if 0
        send_to_char( "You scan about the room for its exits.\r\n", ch );
        return;
#else
	// Show all valid directions.
	for(i = 0; i < 6; i++) {
		if ( CAN_SCAN( (ch)->in_room, i) ) {
			strcpy(dir_arg, scan_directions[i]);
			do_scan(ch, dir_arg, cmd, subcmd);
			sendChar(ch, "\r\n");
		}
	}
	return;
#endif
    }

    if(( scan_dir = search_block(arg, scan_directions, FALSE)) >= 0 ){

        char   scan_buf[256] = "";
        int    scan_distance = 0;
        int    scan_max      = (GET_SKILL(ch, SKILL_SCAN)/20 < SCAN_MAX ? GET_SKILL(ch, SKILL_SCAN)/20 : SCAN_MAX );
        int    scan_room     = (ch)->in_room;
        char **scan_strings  = ( scan_dir <= 3 ? horizontal : vertical );
        char  *scan_modifier = modifier[ scan_dir ];

        /* if you have scout at promising or above, you can scan further */
        if (GET_SKILL(ch, SKILL_SCOUT) >= 60 && scan_max < SCAN_MAX)
            scan_max++;

        while( CAN_SCAN( scan_room, scan_dir ) && ( scan_distance < scan_max ))
        {
          CharData *scan_mob = NULL;
          int scanned_mobs = 0, hidden_mobs = 0;

          scan_room = world[ scan_room ].dir_option[ scan_dir ]->to_room;
          scan_mob  = world[ scan_room ].people;
          sendChar( ch, "%s %s: ", scan_strings[ scan_distance ], scan_modifier );
          scanned_mobs = 0;

          if( GET_RACE(ch) == RACE_DROW && IS_SUNLIGHT(scan_room) && !PRF_FLAGGED(ch, PRF_HOLYLIGHT)) {
              sendChar( ch, "Nothing but brilliant sunlight\r\n" );
              return;
          }
            
          if( ROOM_FLAGGED( scan_room, ROOM_FOG ))
          {
            sendChar( ch, "Dense fog" );
            return;
          }

          while( scan_mob != NULL ){
            if( CAN_SEE( ch, scan_mob ) && !IS_AFFECTED( scan_mob, AFF_HIDE )){
              if( scanned_mobs > 0 ) send_to_char( ", ", ch );

	      /* Let rangers see aggro mobs on scan. */
	      if ( IS_NPC(scan_mob) &&
		   IS_SET_AR(MOB_FLAGS(scan_mob), MOB_AGGRESSIVE) &&
		   skillSuccess(ch, SKILL_HEIGHTENED_SENSES)
		 )
              	sprintf( scan_buf, "&08(%s)&00", scan_mob->player.short_descr);
	      else
                sprintf( scan_buf, "%s", ( IS_MOB( scan_mob ) ? scan_mob->player.short_descr : scan_mob->player.name ));

              send_to_char( scan_buf, ch );
              scanned_mobs += 1;
            } else if (CAN_SEE(ch, scan_mob)) {
                hidden_mobs++;
            }
            scan_mob = scan_mob->next_in_room;
          }/* while */
          if (skillSuccess(ch, SKILL_SCOUT) && hidden_mobs > 0) {
              if (scanned_mobs > 0) send_to_char(", ", ch);
              send_to_char((hidden_mobs > 1 ?
                      "hidden lifeforms" :
                      "a hidden lifeform"), ch);
              advanceSkill(ch, SKILL_SCOUT,
                      (GET_LEVEL(ch) - 35) * 2 + 60,
                      NULL, FALSE, FALSE, FALSE);
          }
          send_to_char( "\n\r", ch );
          scan_distance += 1;
        }/* while */
    }/* if */
    else
        send_to_char("Which direction do you want to scan ?\r\n", ch );

}/* do_scan */

