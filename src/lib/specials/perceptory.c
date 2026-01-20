/***************************************************************************
*                                                                          *
*  perceptory.c written by Yun of RavenMUD to be used in concordance with  *
*  the Knight Templar Perceptory area.                                     *
*                                                                          *
***************************************************************************/


#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/handler.h"
#include "actions/act.h"          /* ACMDs located within the act*.c files */
#include "general/comm.h"
#include "actions/interpreter.h"
#include "general/class.h"
#include "util/utils.h"
#include "specials/special.h"

#define STJRING 23009

ACMD(do_say);

SPECIAL(stjohn)
{
	CharData *stj = me;
	ObjData *obj1 = NULL;
	ObjData *obj2 = NULL;

	// the 'thing' triggering the special process MUST
	// be a CONNECTED PLAYER
	if (!ch->desc || IS_NPC(ch) ) 
		return FALSE;

	// Hunted/Thief/Killer players should NOT be rewarded
	// I make the checks separatly as I am considering if
	// S:t John should do something depending of what flags
	// the player has set

	if (IS_SET_AR(PLR_FLAGS(ch), PLR_HUNTED))
		return FALSE;
	if (IS_SET_AR(PLR_FLAGS(ch), PLR_KILLER))
		return FALSE;
	if (IS_SET_AR(PLR_FLAGS(ch), PLR_THIEF))
		return FALSE;


	// S:t John likes players who pray
	if (!CMD_IS("ask")) 
		return FALSE;

	// Check if he has the ring and/or the staff
	obj1=get_obj_in_list_vis(stj, "stjring", stj->carrying);
	obj2=get_obj_in_list_vis(stj, "stjstaff", stj->carrying);
    if ( (obj1==NULL) && (obj2==NULL) )
		return FALSE;


	// Hand over the items he got
	if (obj1)
		perform_give(stj, ch, obj1);
	if (obj2)
		perform_give(stj, ch, obj2);

	return TRUE;
}

SPECIAL(ibm)
{
	CharData *ibm=me;
	CharData *the_mob=NULL;
	ObjData *obj1 = NULL;
	ObjData *obj2 = NULL;
	char *rem_arg;
	char obj_arg[MAX_INPUT_LENGTH];
	char mob_arg[MAX_INPUT_LENGTH];
	int dotmode;
	
	// the 'thing' triggering the special process MUST
	// be a CONNECTED PLAYER
	if (!ch->desc || IS_NPC(ch) ) 
		return FALSE;	

	// Hunted/Thief/Killer players should NOT be rewarded
	if (IS_SET_AR(PLR_FLAGS(ch), PLR_HUNTED))
		return FALSE;
	if (IS_SET_AR(PLR_FLAGS(ch), PLR_KILLER))
		return FALSE;
	if (IS_SET_AR(PLR_FLAGS(ch), PLR_THIEF))
		return FALSE;

		// The key command here is give
	if (!CMD_IS("give"))
		return FALSE;

	// The player MUST actually give something
    rem_arg = one_argument(argument, obj_arg);
	if (!*obj_arg)
		return FALSE;

	// No money accepted, only goods
	if (is_number(obj_arg))
		return FALSE;

	// And the player must give it to I.B.M
	one_argument(rem_arg, mob_arg);
	if ( !(the_mob = give_find_vict(ch, mob_arg)) )
		return FALSE;
	if ( !(the_mob == me))
		return FALSE;

	// Does the player have the item he is trying to give away?
	dotmode = find_all_dots(obj_arg);
	if (dotmode != FIND_INDIV)
		return FALSE;
	if (!(obj1= get_obj_in_list_vis(ch, obj_arg, ch->carrying)))
		return FALSE;

	// Perform the give, I.B.M accepts all donations to his cause!
	perform_give(ch, ibm, obj1);
	do_say(the_mob, "I thank thee for thine donation!",0 ,0); 

	// OK now, is it what I.B.M want to have?
	if ( !(GET_OBJ_VNUM(obj1) == STJRING) )
		return FALSE;
	
	//Does he have the reward?
	obj2=get_obj_in_list_vis(ibm, "ibmgrail", ibm->carrying);	
	if (obj2) {
		// Yes he does! Give it and a speech!
		perform_give(ibm, ch, obj2);
		do_say(the_mob, "I see that chivalry and goodness still pervail in the world, despite the assaults of the forces of darkness.",0,0);
		do_say(the_mob, "Take this artifact, the Holy Grail and use it in the upcoming struggle against opression.",0,0);
		do_say(the_mob, "Use it to defend your country, the Order, and the defenseless, but never use it to spill the blood of the innocent.",0,0);
    } else {
	// No he don't... Tough luck!! return the ring :(
		do_say(the_mob, "I see that chivalry and goodness still pervail in the world, despite the assaults of the forces of darkness.",0,0);
		do_say(the_mob, "Keep the ring and use it in the upcoming struggle against opression.",0,0);
		do_say(the_mob, "Use it to defend your country, the Order, and the defenseless, but never use it to spill the blood of the innocent.",0,0);
		if (obj1)
			perform_give(ibm, ch, obj1);

	}

	return TRUE;
}

SPECIAL(heavens_door) 
{
	char *rem_arg;
	char *dir = NULL;
	char *type = "door";
	char obj_arg[MAX_INPUT_LENGTH];
  	int door, other_room;
  	struct room_direction_data *back;


	// the 'thing' triggering the special process MUST
	// be a CONNECTED PLAYER
	if (!ch->desc || IS_NPC(ch) ) 
		return FALSE;	

	// Hunted/Thief/Killer players should NOT be rewarded
	if (IS_SET_AR(PLR_FLAGS(ch), PLR_HUNTED))
		return FALSE;
	if (IS_SET_AR(PLR_FLAGS(ch), PLR_KILLER))
		return FALSE;
	if (IS_SET_AR(PLR_FLAGS(ch), PLR_THIEF))
		return FALSE;

		// The key command here is knock
	if (!CMD_IS("knock"))
		return FALSE;

	// The player MUST actually knock on something
    rem_arg = one_argument(argument, obj_arg);
	if (!*obj_arg)
		return FALSE;

	// Is it a door the player is knocking?!?
	if ( strcmp(obj_arg,"door")!= 0   ) 
		return FALSE;


	// Unlock and Open the door going north from 23003
	// Don't know how to do that :/
  if ((door = find_door(ch, type, dir)) >= 0) {

    if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR)) {
	mudlog(NRM, LVL_IMMORT, TRUE, "heavens_door: problem, exit isn't a door.");
	return FALSE;
    }
    else if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED)) {
	// All is cool and fruity.
	return FALSE;
    }
    else {
      REMOVE_BIT(EXIT(ch, door)->exit_info, EX_CLOSED);
      if (EXIT(ch, door)->keyword) {
	act("The $F opens!", FALSE, ch, 0, EXIT(ch, door)->keyword, TO_ROOM);
	act("The $F opens!", FALSE, ch, 0, EXIT(ch, door)->keyword, TO_CHAR);
      }
      else {
	act("The door opens!", FALSE, ch, 0, 0, TO_ROOM);
	act("The door opens!", FALSE, ch, 0, 0, TO_CHAR);
      }
      /* now for opening the OTHER side of the door! */
      if ((other_room = EXIT(ch, door)->to_room) != NOWHERE)
	if ((back = world[other_room].dir_option[rev_dir[door]]))
	  if (back->to_room == ch->in_room) {
	    REMOVE_BIT(back->exit_info, EX_CLOSED);
	    if (back->keyword) {
	      sprintf(buf, "The %s is opened from the other side.\r\n",
		      fname(back->keyword));
	      send_to_room(buf, EXIT(ch, door)->to_room);
	    } else
	      send_to_room("The door is opened from the other side.\r\n",
			   EXIT(ch, door)->to_room);
	  }
    }
    return TRUE;
    }
    return FALSE;	
}







