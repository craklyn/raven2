
/**************************************************************************
 * Routines to handle ignoring                                             *
  **************************************************************************/

#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/comm.h"
#include "actions/interpreter.h"
#include "util/utils.h"
#include "general/class.h"
#include "magic/spells.h"
#include "general/handler.h"
#include "specials/mail.h"
#include "general/color.h"
#include "actions/act.ignore.h"

#define  MIN_IGNORE_SAVE     15
#define  MAX_IGNORE_SZ       132


void
save_ignores(CharData *ch )
{
    struct ignore *an_ignore;
    int ignore_count = 0;
    FILE *fd;

    if( IS_NPC( ch )) return;

    if( GET_LEVEL( ch ) < MIN_IGNORE_SAVE ) return;

    if( !get_filename( GET_NAME(ch), buf, IGNORE_FILE )) {
        return;
    }

    if( !(fd = fopen( buf, "wb" ))) {
        return;
    }

    /* Ok, if we've made it this far let's dump out those ignores. */

    an_ignore = GET_IGNORES(ch);

    while(( an_ignore != NULL ) && ( ignore_count++ < GET_LEVEL(ch) )){
        fputs( an_ignore->ignore, fd );
        fputs( "\n", fd );
        an_ignore = an_ignore->next;
    }

    fclose( fd );

}/* save_ignores */

void
load_ignores( CharData *ch )
{
    int ignore_count = 0;
    FILE *fd;
    char ignore_str[MAX_IGNORE_SZ];

    ACMD(do_ignore);

    if(IS_NPC(ch)) return;

    if(GET_LEVEL(ch) < MIN_IGNORE_SAVE ) return;

    if(!get_filename(GET_NAME(ch), buf, IGNORE_FILE )) {
        return;
    }

    if(!(fd = fopen( buf, "r" ))) {
        return;
    }

    while(!feof(fd) && (ignore_count++ < GET_LEVEL(ch) )){
        int ignore_end = 0;
        if( fgets( ignore_str, sizeof( ignore_str ), fd ) == NULL ) break;
        ignore_end = strlen( ignore_str ) - 1;

        while(( ignore_end > 0 ) && ( ignore_str[ ignore_end ] == '\n' ))
            ignore_str[ ignore_end-- ] = 0;

        do_ignore( ch, ignore_str, 0, 0 );
    }

    fclose( fd );

}/* load_ignores */

struct ignore *find_ignore(struct ignore * ignore_list, char *str)
{
  while (ignore_list != NULL) {
    if (*str == *ignore_list->ignore)	/* hey, every little bit counts :-) */
      if (!strcmp(str, ignore_list->ignore))
	return ignore_list;

    ignore_list = ignore_list->next;
  }

  return NULL;
}


void free_ignore(struct ignore * a)
{
  if (a->ignore)
    free(a->ignore);
  
  free(a);
}


/* The interface: do_ignore */
ACMD(do_ignore)
{
  char *repl;
  struct ignore *a, *temp;

  if (IS_NPC(ch))
    return;

  repl = any_one_arg(argument, arg);

  if (!*arg) {
    send_to_char("Currently defined ignores:\r\n", ch);
    if ((a = GET_IGNORES(ch)) == NULL)
      send_to_char(" None.\r\n", ch);
    else {
      while (a != NULL) {
	sprintf(buf, "%-15s\r\n", a->ignore);
	send_to_char(buf, ch);
	a = a->next;
      }
    }
  } else {
    if ((a = find_ignore(GET_IGNORES(ch), arg)) != NULL) {
      REMOVE_FROM_LIST(a, GET_IGNORES(ch), next);
      free_ignore(a);
    }

    if (a != NULL) {
      send_to_char("Ignore deleted.\r\n", ch);
    }
    else {
      CREATE(a, struct ignore, 1);
      a->ignore = str_dup(arg);

      a->next = GET_IGNORES(ch);
      GET_IGNORES(ch) = a;
      sprintf( buf, "Ignore added: %s\r\n", a->ignore );
      send_to_char( buf, ch );
    }
  }
}



/*
 * Determine whether ch wants to ignore a talker.
 *
 * Return values:
 *   0: ch does not want to ignore talker
 *   1: ch wants to ignore talker
 */
int prefers_ignore(CharData *ch, CharData *talker)
{
    struct ignore *a;
    char talkerName[500];
    int i;

    if(IS_NPC(ch) || IS_NPC(talker))
        return 0;

    sprintf(talkerName, "%s", GET_NAME(talker));
    any_one_arg(talkerName, talkerName);

    if((a = find_ignore(GET_IGNORES(ch), talkerName)) != NULL)
        return 1;
    else
        return 0;
}

