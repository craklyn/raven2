/* Mortius : Spec proc created for Diamond of RavenMUD drop the dice to see
	     what they roll */

#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/handler.h"
#include "actions/act.h"          /* ACMDs located within the act*.c files */
#include "general/comm.h"
#include "actions/interpreter.h"
#include "util/utils.h"
#include "specials/special.h"

SPECIAL(random_dice)
{
  int dice1 = 0, dice2 = 0;
  ObjData *obj = NULL;

  if (!ch->desc || IS_NPC(ch))
      return FALSE;

  if (!CMD_IS("drop"))
      return FALSE;

  if (!(obj = get_obj_in_list_vis(ch, "dice", ch->carrying))) 
      return FALSE;

  if (obj == NULL)
      return FALSE;

  dice1 = number(1, 6);
  dice2 = number(1, 6);


  sprintf(buf, "You roll the dice, clenches your teeth and await the outcome!");
  sprintf(buf2,"%s rolls the dice, clenches $s teeth and awaits the outcome!",
          GET_NAME(ch));

  act(buf,  FALSE, ch, 0, 0, TO_CHAR);
  act(buf2, FALSE, ch, 0, 0, TO_ROOM);

  sprintf(buf, "The first number rolled is a %d and the second is %d.",
          dice1, dice2);

  act(buf, FALSE, ch, 0, 0, TO_CHAR);
  act(buf, FALSE, ch, 0, 0, TO_ROOM);

  obj_from_char(obj);
  obj_to_room(obj, ch->in_room);
  
  return TRUE;
}
