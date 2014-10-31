
#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/comm.h"
#include "general/handler.h"
#include "actions/interpreter.h"
#include "util/utils.h"
#include "specials/special.h"
#include "general/class.h"
#include "magic/spells.h"

/*
 * Liam Nov 3, 1996
 * 
 * First time I've written a special for a shopkeeper...
 * This is probably all wrong.
 *
 * Digger Dec 7, 1996
 *
 * That boy did such a bang up job I'm gonna use his healer as
 * the basis for the Metaphys. Thanks Li ... you're the best man.
 * *wipe tear from eye*sniff*
 *
 * Craklyn 16-May-2000
 *
 * Those boys did such a bang up job of healer/metaphysician
 * that I'm using their code to do a dwarven forger.
 *
 */

int withinForgeBounds(CharData *ch, CharData *forger, ObjData *obj) {

    // It's in the forge code, might as well not open a work-around
    // if retirement is ever in.
    if ( IS_SET_AR(PLR_FLAGS(ch), PLR_RETIRED) ) {
        send_to_char("Just relax and enjoy being retired.\r\n", ch);
        return FALSE;
    }
    
    if ((GET_OBJ_TYPE(obj) != ITEM_WEAPON)) {
        act("$n tells you, 'It doesn't look like $p would make a good weapon...'\r\n.", FALSE, forger, obj, ch, TO_VICT);
        return FALSE;
    }

    // Its forge level, aka already forged?
    if (GET_OBJ_VAL(obj, 0) > 0) { 
        act("$n tells you, 'I am not powerful enough to forge $p...'\r\n.", FALSE, forger, obj, ch, TO_VICT);
        return FALSE;
    }
    
    if (IS_SET_AR(GET_OBJ_EXTRA(obj), ITEM_MAGIC)) {
        act("$n tells you, 'This weapon is magical and I cannot forge it.'\r\n.", FALSE, forger, obj, ch, TO_VICT);
        return FALSE;
    }
    
    return TRUE;
}


int forgeWea( CharData *ch, CharData *forger, ObjData *obj )
{
	/* PLEASE NOTE!!!  This command alters the object_values of the target
	   weapon, and this will save to the rent files.  It should not cause
	   a problem with stock Circle, but if your weapons use the first 
	   position [ GET_OBJ_VAL(weapon, 0); ], then you WILL have a problem.
	   This command stores the character's level in the first value to 
	   prevent the weapon from being "forged" more than once by mortals.
	   Install at your own risk.  You have been warned...

	*/ // Oh, he's doing it.  He's installing the forbidden code.
	
	int prob = 0, dam = 0; 

	if ( !withinForgeBounds(ch, forger, obj) )
		return FALSE;  
	

	/* determine success probability */
	
	prob += ((GET_LEVEL(forger)/8) * 10) + 35;
	//prob += ( ((GET_LEVEL(forger)/8) * 10) + GET_DEX(forger) + GET_WIS(forger));
	// I auto-set it to have the max dex and wis of dwarves
	// since not all forgers are dwarves.

	if ((number(60, 100) > prob) && (GET_LEVEL(forger) < LVL_IMMORT)) {
		send_to_char("As you pound out the dents in the weapon,"
			     " you hit a weak spot and it explodes!\r\n", forger);
		act("$n tries to forge a weapon, but it explodes!",
		    FALSE, forger, 0,0, TO_ROOM);
		extract_obj(obj);
		dam = number(20, 60);
		GET_HIT(forger) -= dam;
		update_pos(forger);
		act("$n says, 'Ow.  I hate it when that happens.",
		    FALSE, forger, 0,0, TO_ROOM);
		return TRUE; // Weapon exploded, still costs money
	}

        int initialWeapon1 = GET_OBJ_VAL(obj, 1);
        int initialWeapon2 = GET_OBJ_VAL(obj, 2);

	GET_OBJ_VAL(obj, 0) = GET_LEVEL(forger);
	GET_OBJ_VAL(obj, 1) += number(-1, 2); 
	GET_OBJ_VAL(obj, 2) += number(-1, 1);
	GET_OBJ_RENT(obj) += (GET_LEVEL(forger) << 3);
        obj->affected[5].location = APPLY_USELEVEL;
        obj->affected[5].modifier = GET_LEVEL(forger);
	send_to_char("You have forged new life into the weapon!\r\n", forger);
	act("$n vigorously pounds on a weapon!",
		    FALSE, forger, 0, 0, TO_ROOM);

        if(initialWeapon1 + 2 == GET_OBJ_VAL(obj, 1) &&
               initialWeapon2 + 1 == GET_OBJ_VAL(obj, 2)) {
            act("You have forged $p into a very fine weapon!", FALSE, forger, obj, 0, TO_CHAR);
            act("$n has forged $p into a very fine weapon!", FALSE, forger, obj, 0, TO_ROOM);
        }

	return TRUE; //scha-wing.  It costs to get a forged weapon.
}



typedef struct {
  int  (*forgeFunc)(CharData *, CharData *, ObjData *);
  char  *name;             /* name the attr is purchased under     */
  int   cost;              /* cost PER LEVEL of purchaser          */
} forgeEntry;


forgeEntry forgeActs[] = {
  { forgeWea, "forge", 0},
  { NULL,    "",       0}
};   // 1) This is a copy of Digger's system, which I spent a long time
     //    figuring out and didn't want to make my own.
     // 2) if some other player time-sink, like armor forging is eventually
     //    added, you can let armorsmiths do that also, by adding it to this
     //    generalized function.



SPECIAL( forgeshop ) {
    CharData *forger = me;
    // Doing it this way so you can see the cost
    // is different on each forge board.  This is one of those
    // bad things we talked about that gets screwed up when you
    // change some but not all code.  Any suggestions?
    int	forgeRate = 200000 + 800000 * (GET_LEVEL(forger) - 30)/20 ;  

    if( !ch->desc || IS_NPC(ch) ) return FALSE;

    if( !CMD_IS("list" ) &&
        !CMD_IS("buy"  ) &&
        !CMD_IS("terms") ) return FALSE;

	
    if( CMD_IS( "list" )){
        act( "$n shows you a menu, 'I offer the following services:'", FALSE, forger, 0, ch, TO_VICT);
sendChar(ch,
             "+---------------+----------------------------------------+-----------------+\r\n"
             "|   Service     |               Description              |       Rates     |\r\n"
             "|               |                                        |                 |\r\n"
             "| Forge a weapon| Attempts to improve the weapon's       |       Gold      |\r\n"
             "|               |   ability to deal damage.              |                 |\r\n");
sendChar(ch, "|               |                                        |      %d\r\n", forgeRate);	
sendChar(ch, "|               |  Forge Level:  %d                      |                 |\r\n", GET_LEVEL(forger));
sendChar(ch,
             "|               |                                                          |\r\n"
             "+---------------+----------------------------------------+------+-----+----+\r\n"
             "| Make certain you read the '&08terms&00' of doing business in this shop         |\r\n"
             "| before initiating a transaction. Also, try '&09help forge&00' for a brief      |\r\n"
             "| explanation of how things work around here.                              |\r\n"
             "+--------------------------------------------------------------------------+\r\n");
        sendChar( ch, "\r\n" );
        return( TRUE );
    }

    if( CMD_IS( "terms" )){
        act( "$n shows you a contract that reads:", FALSE, forger, 0, ch, TO_VICT);
        sendChar( ch, "----------------------------------------------------------\r\n" );
        sendChar( ch, "* * *NO REFUNDS* * * ALL  SALES FINAL * * *NO REFUNDS* * *\r\n" );
        sendChar( ch, "----------------------------------------------------------\r\n" );
        sendChar( ch, "    When you purchase  an item upgrade from this shop you \r\n" );
        sendChar( ch, "will be charged  the going rate for the desired work.  My \r\n" );
        sendChar( ch, "prices fluctuate  based on  the mood the gods are in.  If \r\n" );
        sendChar( ch, "my  prices vary,  and trust me, they will, and you feel   \r\n" );
        sendChar( ch, "the need to  hassle me for the difference, I will be      \r\n" );
        sendChar( ch, "forced to contact the  High Assassin to put a contract on \r\n" );
        sendChar( ch, "your life for being an insolent fool who needs to be      \r\n" );
        sendChar( ch, "taught a lesson.                                          \r\n" );
        sendChar( ch, "                                                          \r\n" );
        sendChar( ch, "By purchasing an item upgrade you AGREE to the FULL TERMS \r\n" );
        sendChar( ch, "of this contract which means you will not be suprised when\r\n" );
        sendChar( ch, "I deal with you in a severe fashion. I &09DO&00 hope I'm making\r\n" );
        sendChar( ch, "myself perfectly clear. Have a pleasant day. Drive through\r\n" );
        sendChar( ch, "please.                                                   \r\n" );
        sendChar( ch, "----------------------------------------------------------\r\n" );
        return( TRUE );
    }

	/*
    if( GET_LEVEL(ch) < 30 ){
        sendChar( ch, "You must be at least level 30 to make a purchase.\r\n" );
        return( TRUE );  
    }
	*/  //This is the sort of code I hate as a player.  And I almost put it in.

	
	if( GET_LEVEL(forger) < 30 ){
        sendChar( ch, "This mob is incorrectly set.  Please alert an immortal.\r\n" );
		mudlog(NRM, LVL_IMMORT, TRUE, "Forger incorrectly set; level too low. Vnum: %d", GET_MOB_VNUM(forger));
        return( TRUE );  // Important check
    }

    if( CMD_IS( "buy" )){
        forgeEntry *ptr  = forgeActs;
        struct     obj_data *forgeObj;
        char forgeCmd[80], objName[80];

        argument = one_argument( argument, forgeCmd );
        /*
        ** First, let's locate a valid entry based on what the customer is
        ** attempting to buy.  Shouldn't be hard right now, with only one thing for sale.
        */
        while( ptr->forgeFunc != NULL && strcmp(ptr->name, forgeCmd )){
            ptr++;
        }  // This leaves room to grow, if armor or other stuff is one day forgable.
		   // Also, easier for me to copy Digger this way. -Craklyn

        if( ptr->forgeFunc == NULL ){
            act("$n tells you, 'What, do you keep the funk alive by talking like idiots?'", FALSE, forger, 0, ch, TO_VICT);
            return( TRUE ); 
        }

        if( forgeRate > GET_GOLD(ch) ){
            act("$n tells you, 'You can't afford it!'", FALSE, forger, 0, ch, TO_VICT);
            return( TRUE );
        }

        one_argument( argument, objName );

        // if no object named, or if object not found
        if((objName != NULL) && !(forgeObj = get_obj_in_list_vis( ch, objName, ch->carrying)) )
        {
            act("$n tells you, 'What do you want me to forge?'", FALSE, forger, 0, ch, TO_VICT);
            return( TRUE );
        }

        // If the function either blows it up or forges it
        // then charge.  If it's outside of bounds, don't.
        if(( ptr->forgeFunc( ch, forger, forgeObj ))){
            char metaLog[132];
            sprintf( metaLog, "Forger: %s paid %d for %s", GET_NAME(ch), forgeRate, forgeCmd );
            // Gotta figure out how logs work, to give it its own log.
            // Or now log it.  Depending.
            if( GET_LEVEL(ch) < LVL_IMMORT ) GET_GOLD(ch) -= forgeRate;
            // For me and my good buddies.
            save_char( ch, NOWHERE );
            // Not as necessary as with meta, but still a good idea'r.
        }
        return(TRUE);
    }
    else return(FALSE);
}





