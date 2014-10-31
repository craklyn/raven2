/* Mortius : Spec proc created for Jamar of RavenMUD to unlock and open
	     a door in a certain room if keyword is said */

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

/* Room to say in 9084, door to open 9082, direction is north */

SPECIAL(xactuachak)
{
  if (!ch->desc || IS_NPC(ch))
      return FALSE;

  if (!CMD_IS("say"))
      return FALSE;

  one_argument(argument, arg);

  /* Right keyword? */
  if (strcmp(arg, "xactuachak") == 0) {

      if (!IS_SET(EXIT(ch, NORTH)->exit_info, EX_ISDOOR))
          return FALSE;
      else if (!IS_SET(EXIT(ch, NORTH)->exit_info, EX_CLOSED))
               return FALSE;
      else if (!IS_SET(EXIT(ch, NORTH)->exit_info, EX_LOCKED))
               return FALSE;

      /* We got this far so lets unlock and open the door */
      REMOVE_BIT(EXIT(ch, NORTH)->exit_info, EX_LOCKED);
      REMOVE_BIT(EXIT(ch, NORTH)->exit_info, EX_CLOSED);

      /* Send the message to the room */
      act("After you say Xactuachak you hear a click and a door swings open.",
          FALSE, ch, 0, 0, TO_CHAR);
      act("You hear a click and a door swings open.",
          FALSE, ch, 0, 0, TO_ROOM);

      /* And we are done */
      return (TRUE);
  } else 
        return (FALSE);
}

