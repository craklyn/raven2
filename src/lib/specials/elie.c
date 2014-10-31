
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
 * Digger Dec 7, 1996
 *
 * That boy did such a bang up job I'm gonna use his healer as
 * the basis for the Metaphys. Thanks Li ... you're the best man.
 * *wipe tear from eye*sniff*
 *
 * Craklyn 16-May-2000
 *
 * Those boys did such a bang up job of healer/metaphysician
 * that I'm using their code as a base for Elie, the automatic
 * identifying mob..
 *
 */

#define identifyRate 4000
#define discernRate  15000


int withinIdentifyBounds(CharData *ch, CharData *identifier, ObjData *obj) 
{   
    if ( IS_OBJ_STAT(obj, ITEM_ARTIFACT) ) {
        act("$n tells you, 'You brought $p here for me to identify?!  The gods themselves "
                "could not perceive that item's character.'\r\n.", FALSE, identifier, obj, ch, TO_VICT);
        return FALSE;
    }

    if( IS_OBJ_STAT(obj, ITEM_IDENTIFIED) )
    {
        act("$n tells you, 'Even you should be able to discern that item's affects...\r\n"
                "but here you go:",  FALSE, identifier, 0, ch, TO_VICT);
    }

    return TRUE;
}

int identifyItem( CharData *ch, CharData *identifier, ObjData *obj )
{
	if (!withinIdentifyBounds(ch, identifier, obj))
		return FALSE;

        call_magic(ch, ch, obj, SPELL_IDENTIFY, GET_LEVEL(ch), NULL, CAST_SPELL);
		
	// success.  It costs to get the item identified
	return TRUE; 
}

int discernItem(CharData *ch, CharData *identifier, ObjData *obj)
{
    ObjData *identify_obj = NULL;

    identify_obj = read_perfect_object(GET_OBJ_VNUM(obj), VIRTUAL);
    if (!identify_obj) {
        send_to_char("Oh no!  Oh no!  No object!  Oh no!\r\n", ch);
        mlog("Can't create virtual object for Elie, object doesn't exist");
        return FALSE;
    }

    if (!withinIdentifyBounds(ch, identifier, identify_obj))
        return FALSE;

    SET_BIT_AR(identify_obj->obj_flags.extra_flags, ITEM_TIMED);
    GET_OBJ_TIMER(identify_obj) = 1;

    obj_to_char(identify_obj, identifier);

    call_magic(ch, ch, identify_obj, SPELL_IDENTIFY, GET_LEVEL(ch), NULL, CAST_SPELL);

    return TRUE;
}

typedef struct {
  int  (*identifyFunc)(CharData *, CharData *, ObjData *);
  char  *name;             /* name the attr is purchased under     */
  int   cost;              /* cost PER LEVEL of purchaser          */
} identifyEntry;


identifyEntry identifyActs[] = {
  { identifyItem, "identify", identifyRate},
  { discernItem, "discern", discernRate},
  { NULL,    "",       0}
};   


SPECIAL( identifyshop ) {
	CharData *identifier = me;

    if( !ch->desc || IS_NPC(ch) ) return FALSE;

    if( !CMD_IS("list" ) &&
        !CMD_IS("buy"  )) return FALSE;
	
    if( CMD_IS( "list" )){
        act( "$n shows you a menu, 'I offer the following services:'", FALSE, identifier, 0, ch, TO_VICT);
        sendChar(ch,
"+---------------+----------------------------------------+-----------------+\r\n"
"|   Service     |               Description              |       Rates     |\r\n"
"|               |                                        |       (Gold)    |\r\n"
"| Identify      | Identify an item's affects             |                 |\r\n"
"|               |                                        |                 |\r\n"
"|               |                                        |      %d       |\r\n"
"|               |                                        |                 |\r\n"
"| Discern       | See the stats an ideal the item could  |                 |\r\n"
"|               | ideally carry                          |                 |\r\n"
"|               |                                        |      %d      |\r\n"
"|               |                                        |                 |\r\n"
"+--------------------------------------------------------------------------+\r\n", identifyRate, discernRate);
        sendChar( ch, "\r\n" );
        return( TRUE );
    }

    if( CMD_IS( "buy" )){
        identifyEntry *ptr  = identifyActs;
        struct     obj_data *identifyObj;
        char identifyCmd[80], objName[80];

        argument = one_argument( argument, identifyCmd );

		
        while( ptr->identifyFunc != NULL && strcmp(ptr->name, identifyCmd )){
            ptr++;
        }

        if( ptr->identifyFunc == NULL ){
            act("$n tells you, 'What, do you keep the funk alive by talking like idiots?'", FALSE, identifier, 0, ch, TO_VICT);
            return( TRUE ); 
        }

        if( identifyRate > GET_GOLD(ch) ){
            act("$n tells you, 'You can't afford it!'", FALSE, identifier, 0, ch, TO_VICT);
            return( TRUE );
        }

        one_argument( argument, objName );

		// if no object named, or if object not found
        if( (objName != NULL)  && 
			!(identifyObj = get_obj_in_list_vis( ch, objName, ch->carrying)) ) 
		{
			
			act("$n tells you, 'What do you want me to identify?'", FALSE, identifier, 0, ch, TO_VICT);
            return( TRUE ); 
		}
		
		// If the function returns true, then charge.
		// If it returns false, don't.
		if(( ptr->identifyFunc( ch, identifier, identifyObj ))){  
			if( GET_LEVEL(ch) < LVL_IMMORT ) GET_GOLD(ch) -= identifyRate;
            // For me and my good buddies.
			save_char( ch, NOWHERE );
        }
        return( TRUE );
    }
    else
	{
		return( FALSE );  
		//if it returns false, I'm guessing it goes on checking the command 
		//against other things in the room like shopkeepers.
	}
}





