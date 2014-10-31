
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
#include "specials/lockers.h"
#include "olc/olc.h"
#include "olc/oedit.h"

#define LOCKER_PURCHASE_10      0
#define LOCKER_PURCHASE_20      1
#define LOCKER_UPGRADE_20       2

#define LOCKER_MAX        3

#define VNUM_START      41301
#define VNUM_MAX        41397
#define LOCKER_ZONE     413

typedef struct {
  int  option;             
  char *name;             /* name the option is purchased under   */
  char *filler;           /* filler to make all the names line up */
  char *description;      /* description for list                 */
  int  cost;              /* cost of option                       */
} LockerStruct;


static LockerStruct lockercosts[LOCKER_MAX] =
{
  {LOCKER_PURCHASE_10, "small", "  ", "Buy a 10 item locker       ", 10000000},
  {LOCKER_PURCHASE_20, "large", "  ", "Buy a 20 item locker       ", 25000000},
  {LOCKER_UPGRADE_20,  "upgrade", "", "Upgrade from 10 to 20 items", 16000000},
};

/* find a locker by its rnum */
static ObjData *find_locker(int vnum)
{
  LockerList *llitem = lockers;

  while (llitem && obj_index[llitem->locker->item_number].virtual != vnum)
    llitem = llitem->next;
  return (llitem ? llitem->locker : NULL);
}

static int create_new_locker(CharData *ch, int size)
{
  int vnum = VNUM_START;
  ObjData *obj, *proto;
  char buf[200];

  // find a locker that both loads and is unused
  while (vnum <= VNUM_MAX) {
    obj = find_locker(vnum);
    if (obj) proto = obj_proto + obj->item_number;
    if (obj && GET_OBJ_VAL(proto, 0) == 0) break;       // 0 items = unused
    vnum++;
  }
  if (vnum > VNUM_MAX) return -1;

  // update the locker size
  GET_OBJ_VAL(obj, 0) = size; GET_OBJ_VAL(proto, 0) = size;

  // update the locker descriptions
  sprintf(buf, "locker l%d l%s %s", vnum, GET_NAME(ch), GET_NAME(ch));
  if (proto->name) free(proto->name);
  obj->name = proto->name = strdup(buf);
  sprintf(buf, "A locker labelled '%s' is here.", GET_NAME(ch));
  if (proto->description) free(proto->description);
  obj->description = proto->description = strdup(buf);

  // save the lockers to disk
  CREATE(ch->desc->olc, struct olc_data, 1);
  OLC_ZNUM(ch->desc) = real_zone(LOCKER_ZONE);
  oSaveToDisk(ch->desc);
  free(ch->desc->olc);

  GET_LOCKER(ch) = vnum;

  return vnum;
};

static int update_locker_size(CharData *ch, int size)
{
  ObjData *obj, *proto;
  int oldsize;

  obj = find_locker(GET_LOCKER(ch));
  if (!obj) return -1;
  proto = obj_proto + obj->item_number;

  oldsize = GET_OBJ_VAL(obj, 0);
  if (oldsize == size) return 0;

  // update the locker size
  GET_OBJ_VAL(obj, 0) = size; GET_OBJ_VAL(proto, 0) = size;

  // save the lockers to disk
  CREATE(ch->desc->olc, struct olc_data, 1);
  OLC_ZNUM(ch->desc) = real_zone(LOCKER_ZONE);
  oSaveToDisk(ch->desc);
  free(ch->desc->olc);

  return size;
}
  
SPECIAL( locker_manager )
{
    int cost = 0, i, size = 20;
    char buf[100];
    LockerStruct *ptr;

    struct char_data *manager = me;

    if( !ch->desc || IS_NPC(ch)) return FALSE;

    if( !CMD_IS("list") && !CMD_IS("buy") ) return FALSE;

    if ( CMD_IS("list"))
      {
        ptr = lockercosts;

	act ("$n tells you, 'I offer the following services:'", FALSE, manager, 0, ch, TO_VICT);
	for (i=0; i < LOCKER_MAX; i++, ptr++ )
	  {

	    sprintf (buf, "$n tells you, '%s%s %s %8d coins.'", ptr->name,ptr->filler, 
		     ptr->description, ptr->cost);
	    act (buf, FALSE, manager, 0, ch, TO_VICT);
	  }
	return TRUE;
      }

    if ( CMD_IS("buy"))
      {
	int choice = -1, newlocker;
	char item_name[MAX_INPUT_LENGTH];
	one_argument (argument, item_name );
	ptr = lockercosts;
        if (item_name[0] == '\0') strcpy(item_name, "!!NOTHING!!");
	for (i=0; i < LOCKER_MAX; i++, ptr++ )
	  {
	    if ( !strncmp(ptr->name, item_name, strlen(item_name) ) )
	      {
		choice = ptr->option;
		cost   = ptr->cost;
		break;
	      }
	  }
	
	if ( choice >= 0 )
	  {
	    if (cost > GET_GOLD(ch)) 
	      {
		act("$n tells you, 'You can't afford it!'", FALSE, manager, 0, ch, TO_VICT);
		return TRUE;
	      }

	    switch ( choice )
	      {
	      case LOCKER_PURCHASE_10:
                size = 10;
                // fall through to be handled the same way
	      case LOCKER_PURCHASE_20:
                if (GET_LOCKER(ch) != 0) {
                  act("$n tells you, 'You already own a locker!'", FALSE, manager, 0, ch, TO_VICT);
                  return TRUE;
                }
                newlocker = create_new_locker(ch, size);
                if (newlocker == -1) {
                  act("$n tells you, 'Sorry, we've sold out!'", FALSE, manager, 0, ch, TO_VICT);
                  return TRUE;
                }
                GET_GOLD(ch) -= cost;
                act("$n tells you, 'Your locker is ready for use!'", FALSE, manager, 0, ch, TO_VICT);
                break;
	      case LOCKER_UPGRADE_20:
                if (GET_LOCKER(ch) == 0) {
                  act("$n tells you, 'You don't own a locker!'", FALSE, manager, 0, ch, TO_VICT);
                  return TRUE;
                }
                switch (update_locker_size(ch, 20)) {
                  case -1:
                    act("$n tells you, 'Your locker has been revoked!'", FALSE, manager, 0, ch, TO_VICT);
                    break;
                  case 0:
                    act("$n tells you, 'Your locker was already upgraded!'", FALSE, manager, 0, ch, TO_VICT);
                    break;
                  default:
                    act("$n tells you, 'Your locker has been upgraded.'", FALSE, manager, 0, ch, TO_VICT);
                    GET_GOLD(ch) -= cost;
                    break;
                }
                break;
	      }
	  } else {
            act("$n tells you, 'I don't offer that service.'", FALSE, manager, 0, ch, TO_VICT);
          }
	
	return TRUE;
      }


    act("$n says, 'I'm afraid I am not ready for business quite yet.'", 
	FALSE, manager, 0, ch, TO_VICT);
    return TRUE;
}

