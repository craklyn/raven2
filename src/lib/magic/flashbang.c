/*
**++
**  RCSID:     $Id: flashbang.c,v 1.6 2001/08/04 06:59:06 raven Exp $
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
**      Craklyn from RavenMUD (this file)
**
**  NOTES:
**
**      Use 132 column editing in here.
**
**--
*/

/*
**  MODIFICATION HISTORY:
**
**  Vision 1.0 07/20/2005  Craklyn
**  Srarted work on flashbangs.  Based on old throw code.
**  
**  Version 1.1 08/11/2005  Craklyn
**  Implimented the first version of flashbang.c.  I really screwed with
**  our gen_missile function, and ended up making a clone function so I
**  didn't have to create a physical object to fly between rooms.  But it
**  looks indecypherable to the mortals.  Maybe I'll clean it up some day.
**  (probably not)
**
*/


/*
** STANDARD U*IX INCLUDES
*/
#include "general/conf.h"
#include "general/sysdep.h"

#define THIS_SKILL                SKILL_FLASHBANG

#define SKILL_ADVANCES_WITH_USE   TRUE
#define SKILL_ADVANCE_STRING      "You've become notably more proficient."
#define SKILL_MAX_LEARN           90
#define DEX_AFFECTS               FALSE
#define INT_AFFECTS               TRUE
#define WIS_AFFECTS               TRUE

#define STUN_MIN                  2
#define STUN_MAX                  3
#define MOVE_PER_FLASHBANG        35

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
#include "magic/flashbang.h"


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      SKILL_NAME - SKILL_THROW
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
**      Skill for throwing all sorts of different stuff. 
**
*/
ACMD(do_flashbang)
{
  static const char *flashbangDirs[] = { "north", "east",
                               "south", "west",
                               "up", "down", "\n"};
 
  char dir[255], target[255]; 
  int dir_num = 0;
  char buf[MAX_INPUT_LENGTH];
  CHAR_DATA *victim = 0;

  IF_UNLEARNED_SKILL( "You wouldn't know where to begin.\r\n" );

  two_arguments(argument, target, dir);

  if( !*target )
  {
    sendChar( ch, "Usage : flashbang <target> [direction] \n\r" );
    return;
  }

  if( !*dir && !(victim = get_char_room_vis( ch, target )))
  {
    sendChar( ch, "Throw a flashbang at what?\n\r" );
    return;
  }

  if(*dir && (dir_num = search_block( dir, flashbangDirs, FALSE )) < 0)
  {
    sendChar( ch, "What direction???\n\r" );
    return;
  }

  if( !victim && !(victim = find_vict_dir( ch, target, DEFAULT_MEDIUM_RANGE, dir_num )))
  {
    sendChar( ch, "You cannot see that in range.\n\r" );
    return;
  }

  if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_PEACEFUL)) 
  {
      send_to_char("No matter how you pull, the safety pin just doesn't want to come out...", ch);
      return;
  }

  //  Not sure where the following line came from.  I didn't write it.  -Craklyn
  //  IF_CH_CANT_SEE_VICTIM( "You cannot see the pin to pull it.\n\r");

  // You can flashbang yourself to hit the room  you're in. :)  
  //IF_CH_CANT_BE_VICTIM("Why would you want to throw things at yourself?\n\r");

    IF_CANT_HURT_VICTIM;
	  
    if (GET_MOVE(ch) < MOVE_PER_FLASHBANG )
    {
        send_to_char("You are too tired to throw straight!\r\n", ch);
        return;
    }
    
    if( skillSuccess( ch, THIS_SKILL ))
    {
        advanceSkill( ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING, DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS );

        gen_grenade(ch, victim, dir_num, MISSILE_TYPE_GRENADE);
        if( ch->in_room == victim->in_room )
        {
            
            act("You throw a flashbang grenade towards $n!", FALSE, victim, 0, ch, TO_VICT);
            act("$N throws a flashbang grenade toward you!", FALSE, victim, 0, ch, TO_CHAR);
            act("$n throws a flashbang grenade toward $N!", FALSE, ch, 0, victim, TO_NOTVICT);

            set_fighting(ch, victim);
            flashbang_explode(ch, victim);
        }
        else
        {
            sprintf(buf, "You throw a flashbang grenade %swards towards $N.", flashbangDirs[dir_num]);
            act(buf, FALSE, ch, NULL, victim, TO_CHAR);
            sprintf(buf, "$n throws a flashbang grenade %swards.", flashbangDirs[dir_num]);
            act(buf, FALSE, ch, NULL, victim, TO_ROOM);
            
            flashbang_explode(ch, victim);
        }
		

    }/* if */
    else
    {
        sendChar( ch, "You fumble with a flashbang grenade, and it goes off in your hands!\n\r");
        flashbang_explode(ch, ch);
    }

    if (GET_LEVEL(ch) <= LVL_IMMORT) STUN_USER_MAX;
    GET_MOVE(ch) -= MOVE_PER_FLASHBANG;
    GET_MOVE(ch) = MAX(0, GET_MOVE(ch));

}/* do_SKILL */


void flashbang_explode(CHAR_DATA *ch, CHAR_DATA *victim)
{
	#define NO_APPLY APPLY_NONE

	int blindfactor;

	if (ROOM_FLAGGED(IN_ROOM(victim), ROOM_PEACEFUL))
	{
            act("&03The flashbang grenade fizzles playfully at your feet.&00", FALSE, victim, NULL, victim, TO_ROOM);
            act("&03The flashbang grenade fizzles playfully at your feet.&00", FALSE, victim, NULL, victim, TO_CHAR);
            return;
        }

	victim = world[victim->in_room].people;

	act("&10The flashbang grenade explodes in a violent flash!&00", FALSE, victim, NULL, victim, TO_ROOM);
	act("&10The flashbang grenade explodes in a violent flash!&00", FALSE, victim, NULL, victim, TO_CHAR);

	while (victim) {
            blindfactor = GET_CON(victim) + GET_WIS(victim)/3 + GET_DEX(victim)/2;
            
            if(GET_LEVEL(victim) < LVL_IMMORT) {
                if(number(1, 30) > GET_CON(victim)) {
                    STUN_VICTIM_RANGE;
                    act("The concussion report sends you reeling!", TRUE, victim, 0, victim, TO_VICT);
                    act("The concussive report sends $N reeling!", FALSE, victim, NULL, victim, TO_ROOM);
                    act("The concussive report sends $N reeling!", FALSE, victim, NULL, victim, TO_CHAR);
                }
            }
            
            if (GET_LEVEL(victim) < LVL_IMMORT  &&  !IS_SET_AR(MOB_FLAGS(victim), MOB_NOBLIND)) {
                if(number(30, 45) >  blindfactor) {
                    add_affect(ch, victim, THIS_SKILL, GET_LEVEL(ch),
                            NO_APPLY, 0, number(13, 18), AFF_BLIND, FALSE, TRUE, FALSE, FALSE);
                    act("A bright flash burns your eyes!", TRUE, victim, 0, victim, TO_VICT);
                    act("$N staggers around blindly!", FALSE, victim, NULL, victim, TO_ROOM);
                }
            }
            victim = victim->next_in_room;
	} //end while

}
        
