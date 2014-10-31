/* Mortius: Ten Trials spec proc writen for Diamond of RavenMUD */

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

/* Going to write this based on Legend.  No real point writing it any
   other way. I hope the formatting for this looks ok, using telnet and
   its a major pain in the ass. */

ACMD(do_say);
ACMD(do_remove);

SPECIAL(ten_trials)
{
  CharData *nereus = me;
  ObjData  *obj = NULL;

  /* Is it a mob? */
  if (!ch->desc || IS_NPC(ch))
      return FALSE;

  /* Nereus wants you to shake his hand */
  if (!CMD_IS("shake"))
      return FALSE;

  /* First lets remove the key if he is wearing it */
  do_remove(nereus, "19918", 0, 0);
    
  /* Lets use the name of the key (which Diamond also put in vnum) */
  if (!(obj = get_obj_in_list_vis(nereus, "19918", nereus->carrying)))
      return FALSE;

  /* If we got this far its time to hand the object over */
  do_say(nereus, "Hello my friends.  Take this key to help you on your journey and "
	             "may the gods smile upon you.", 0, 0);
  do_remove(nereus, "19918", 0, 0);
  perform_give(nereus, ch, obj);

  return TRUE;

} /* The end */
