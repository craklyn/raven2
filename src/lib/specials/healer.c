
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
#include "actions/act.h"          /* ACMDs located within the act*.c files */
#include "specials/healer.h"

/*
 * Liam Nov 3, 1996
 * 
 * First time I've written a special for a shopkeeper...
 * This is probably all wrong.
 */

#define HEALER_HP_100     0
#define HEALER_HP_200     1
#define HEALER_MANA_100   2
#define HEALER_MANA_200   3
#define HEALER_MV_50      4
#define HEALER_MV_100     5
#define HEALER_RESTORE    6

#define HEALER_MAX        7

typedef struct {
  int  option;             
  char *name;             /* name the spell is purchased under    */
  char *filler;           /* filler to make all the names line up */
  char *description;      /* description for list                 */
  int  cost;              /* cost PER LEVEL of purchaser          */
} HealerStruct;


HealerStruct healerspells[HEALER_MAX] =
{
  {HEALER_HP_100   ,"heal", "     ", "100 hit  points      ",   1250},
  {HEALER_HP_200   ,"revive", "   ", "200 hit  points      ",   2000},
  {HEALER_MANA_100 ,"infuse", "   ", "100 mana points      ",   1250},
  {HEALER_MANA_200 ,"recharge", " ", "200 mana points      ",   2000},
  {HEALER_MV_50    ,"refresh", "  ", " 50 move points      ",   1250},
  {HEALER_MV_100   ,"recover", "  ", "100 move points      ",   2000},
  {HEALER_RESTORE  ,"restore", "  ", "all hp,mana,mv points",   5000}
};
  
SPECIAL( healer )
{
    int cost = 0, i;
    char buf[100];
    HealerStruct *ptr;


    struct char_data *healer = me;

    if( !ch->desc || IS_NPC(ch)) return FALSE;

    if( !CMD_IS("list") && !CMD_IS("buy") ) return FALSE;

    if(IS_SET_AR( ROOM_FLAGS( IN_ROOM(ch)), ROOM_CLAN ) &&
        ( world[IN_ROOM(ch)].clan_id != GET_CLAN(ch))) {
        sendChar(ch, "I will never serve you, infidel!\r\n");
        return;
    }

    if ( CMD_IS("list") )
      {
        ptr = healerspells;

	act ("$n tells you, 'I offer the following services:'", FALSE, healer, 0, ch, TO_VICT);
	for (i=0; i < HEALER_MAX; i++, ptr++ )
	  {

	    sprintf (buf, "$n tells you, '%s%s %s %8d coins.'", ptr->name,ptr->filler, 
		     ptr->description, ptr->cost*GET_LEVEL(ch) );
	    act (buf, FALSE, healer, 0, ch, TO_VICT);
	  }
	return TRUE;
      }

    if ( CMD_IS("buy") )
      {
	int choice = -1;
	char item_name[MAX_INPUT_LENGTH];
	one_argument (argument, item_name );
#if 0
	mudlog( NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE, "HEALER: BUY : [%s]", item_name);
#endif
	ptr = healerspells;
	for (i=0; i < HEALER_MAX; i++, ptr++ )
        {
            if ( !strcmp(ptr->name, item_name ) )
            {
                choice = ptr->option;
                cost   = ptr->cost*GET_LEVEL(ch);
                break;
            }
        }

	if ( choice >= 0 )
	  {
	    if (cost > GET_GOLD(ch)) 
	      {
		act("$n tells you, 'You can't afford it!'", FALSE, healer, 0, ch, TO_VICT);
		return TRUE;
	      }
	    else
	      {
                if(IS_SET_AR(ROOM_FLAGS( IN_ROOM(ch)), ROOM_CLAN) && (world[IN_ROOM(ch)].clan_id == GET_CLAN(ch))) {
                    cost = cost * 95 / 100;
                    sprintf(buf, "$n tells you, 'And since you're a fellow clans%s, I'll give you a small discount.", MANWOMAN(ch));
                    act(buf, FALSE, healer, 0, ch, TO_VICT);
                }
		act("$n tells you, 'Thanks for your business.'", FALSE, healer, 0, ch, TO_VICT);
		act("$n waves his hands over you.", FALSE, healer, 0, ch, TO_VICT);
		act("You feel better.", FALSE, ch, 0, 0, TO_CHAR);
		act("$n waves his hands over $N.", TRUE, healer, 0, ch, TO_NOTVICT);

                GET_GOLD(ch) -= cost;
	      }

	    switch ( choice )
	      {
	      case HEALER_HP_100:
		GET_HIT(ch) = MIN(GET_MAX_HIT(ch), GET_HIT(ch) + 100);
		break;

	      case HEALER_HP_200:
		GET_HIT(ch) = MIN(GET_MAX_HIT(ch), GET_HIT(ch) + 200);
		break;
		
	      case HEALER_MANA_100:
		GET_MANA(ch) = MIN(GET_MAX_MANA(ch), GET_MANA(ch) + 100);
		break;

	      case HEALER_MANA_200:
		GET_MANA(ch) = MIN(GET_MAX_MANA(ch), GET_MANA(ch) + 200);
		break;
	      
	      case HEALER_MV_50:
		GET_MOVE(ch) = MIN(GET_MAX_MOVE(ch), GET_MOVE(ch) + 50 );
		break;

	      case HEALER_MV_100:
		GET_MOVE(ch) = MIN(GET_MAX_MOVE(ch), GET_MOVE(ch) + 100 );
		break;

	      case HEALER_RESTORE:
                  restore(ch, ch);
		break;
	      }
	  }
	
	return TRUE;
      }


    act("$n says, 'I'm afraid I am not ready for business quite yet.'", 
	FALSE, healer, 0, ch, TO_VICT);

    mudlog(NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE, "HEALER: Reached end of function with no result.");
    return TRUE;
}

