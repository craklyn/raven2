 
/**************************************************************************
 * Routines to handle aliasing                                             *
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
#include "actions/act.alias.h"

#define  MIN_ALIAS_SAVE     15
#define  MAX_ALIAS_SZ       132


void
save_aliases(CharData *ch )
{
    struct alias *an_alias;
    int alias_count = 0;
    FILE *fd;

    if( IS_NPC( ch )) return;

    if( GET_LEVEL( ch ) < MIN_ALIAS_SAVE ) return;

    if( !get_filename( GET_NAME(ch), buf, ETEXT_FILE )) return;

    if( !(fd = fopen( buf, "wb" ))) return;

    /* Ok, if we've made it this far let's dump out those aliases. */

    an_alias = GET_ALIASES( ch );

    while(( an_alias != NULL ) && ( alias_count++ < GET_LEVEL(ch) )){
        fputs( an_alias->alias, fd );
        fputs( an_alias->replacement, fd );
        fputs( "\n", fd );
        an_alias = an_alias->next;
    }

    fclose( fd );

}/* save_aliases */

void
load_aliases( CharData *ch )
{
    int alias_count = 0;
    FILE *fd;
    char alias_str[MAX_ALIAS_SZ];

    ACMD( do_alias );

    if( IS_NPC( ch )) return;

    if( GET_LEVEL( ch ) < MIN_ALIAS_SAVE ) return;

    if( !get_filename( GET_NAME(ch), buf, ETEXT_FILE )) return;

    if( !(fd = fopen( buf, "r" ))) return;

    while( !feof(fd) && ( alias_count++ < GET_LEVEL(ch) )){
        int alias_end = 0;
        if( fgets( alias_str, sizeof( alias_str ), fd ) == NULL ) break;
        alias_end = strlen( alias_str ) - 1;

        while(( alias_end > 0 ) && ( alias_str[ alias_end ] == '\n' ))
            alias_str[ alias_end-- ] = 0;

        do_alias( ch, alias_str, 0, 0 );
    }

    fclose( fd );
//    command_interpreter(ch,"say hello");
}/* load_aliases */


struct alias *find_alias(struct alias * alias_list, char *str)
{
  while (alias_list != NULL) {
    if (*str == *alias_list->alias)	/* hey, every little bit counts :-) */
      if (!strcmp(str, alias_list->alias))
	return alias_list;

    alias_list = alias_list->next;
  }

  return NULL;
}


void free_alias(struct alias * a)
{
  if (a->alias)
    free(a->alias);
  if (a->replacement)
    free(a->replacement);
  free(a);
}


/* The interface: do_alias */
ACMD(do_alias)
{
  char *repl;
  struct alias *a, *temp;

  if (IS_NPC(ch))
    return;

  repl = any_one_arg(argument, arg);

  if (!*arg) {
    send_to_char("Currently defined aliases:\r\n", ch);
    if ((a = GET_ALIASES(ch)) == NULL)
      send_to_char(" None.\r\n", ch);
    else {
      while (a != NULL) {
	sprintf(buf, "%-15s %s\r\n", a->alias, a->replacement);
	send_to_char(buf, ch);
	a = a->next;
      }
    }
  } else {
    if ((a = find_alias(GET_ALIASES(ch), arg)) != NULL) {
      REMOVE_FROM_LIST(a, GET_ALIASES(ch), next);
      free_alias(a);
    }
    if (!*repl) {
      if (a == NULL)
	send_to_char("No such alias.\r\n", ch);
      else
	send_to_char("Alias deleted.\r\n", ch);
    } else {
      if (!str_cmp(arg, "alias")) {
	send_to_char("You can't alias 'alias'.\r\n", ch);
	return;
      }
      CREATE(a, struct alias, 1);
      a->alias = str_dup(arg);
      delete_doubledollar(repl);
      a->replacement = str_dup(repl);
      if (strchr(repl, ALIAS_SEP_CHAR) || strchr(repl, ALIAS_VAR_CHAR))
	a->type = ALIAS_COMPLEX;
      else
	a->type = ALIAS_SIMPLE;
      a->next = GET_ALIASES(ch);
      GET_ALIASES(ch) = a;
      sprintf( buf, "Alias added: %s\r\n", a->alias );
      send_to_char( buf, ch );
    }
  }
}

/*
 * Valid numeric replacements are only &1 .. &9 (makes parsing a little
 * easier, and it's not that much of a limitation anyway.)  Also valid
 * is "&*", which stands for the entire original line after the alias.
 * ";" is used to delimit commands.
 */
#define NUM_TOKENS       9

void perform_complex_alias(struct txt_q *input_q, char *orig, struct alias *a, DescriptorData * d)
{
#define MAX_SPECIALS_BEFORE_AUTO_DC 20
  struct txt_q temp_queue;
  char *tokens[NUM_TOKENS], *temp, *write_point;
  int num_of_tokens = 0, num;
  int total_alias_specials = 0;

  /* First, parse the original string */
  temp = strtok(strcpy(buf2, orig), " ");
  while (temp != NULL && num_of_tokens < NUM_TOKENS) {
    tokens[num_of_tokens++] = temp;
    temp = strtok(NULL, " ");
  }

  /* initialize */
  write_point = buf;
  temp_queue.head = temp_queue.tail = NULL;

  /* now parse the alias */
  for (temp = a->replacement; *temp; temp++) {
    if (*temp == ALIAS_SEP_CHAR) {
      *write_point = '\0';
      buf[MAX_INPUT_LENGTH - 1] = '\0';
      write_to_q(buf, &temp_queue, 1);
      write_point = buf;
    } else if (*temp == ALIAS_VAR_CHAR) {
      temp++;
      if ((num = *temp - '1') < num_of_tokens && num >= 0) {
	strcpy(write_point, tokens[num]);
	write_point += strlen(tokens[num]);
        total_alias_specials += 1;
        if( total_alias_specials >= MAX_SPECIALS_BEFORE_AUTO_DC ){
            mudlog(BRF, MAX(LVL_LRGOD, GET_INVIS_LEV(d->character)), TRUE,
            "(**)Faked a crash to %s - possible crash attempted - SHUN?", GET_NAME( d->character ));
            SET_DCPENDING(d);
            return;
        }
      } else if (*temp == ALIAS_GLOB_CHAR) {
	strcpy(write_point, orig);
	write_point += strlen(orig);
        total_alias_specials += 1;
        if( total_alias_specials >= MAX_SPECIALS_BEFORE_AUTO_DC ){
            mudlog(BRF, MAX(LVL_LRGOD, GET_INVIS_LEV(d->character)), TRUE,
                   "(**)Faked a crash to %s - possible crash attempted - SHUN?", GET_NAME( d->character ));
            SET_DCPENDING(d);
            return;
        }
      } else
	if ((*(write_point++) = *temp) == '$') /* redouble $ for act safety */
	  *(write_point++) = '$';
    } else
      *(write_point++) = *temp;
  }

  *write_point = '\0';
  buf[MAX_INPUT_LENGTH - 1] = '\0';
  write_to_q(buf, &temp_queue, 1);

  /* push our temp_queue on to the _front_ of the input queue */
  if (input_q->head == NULL)
    *input_q = temp_queue;
  else {
    temp_queue.tail->next = input_q->head;
    input_q->head = temp_queue.head;
  }
}


/*
 * Given a character and a string, perform alias replacement on it.
 *
 * Return values:
 *   0: String was modified in place; call command_interpreter immediately.
 *   1: String was _not_ modified in place; rather, the expanded aliases
 *      have been placed at the front of the character's input queue.
 */
int perform_alias(DescriptorData * d, char *orig)
{
  char first_arg[MAX_INPUT_LENGTH], *ptr;
  struct alias *a, *tmp;

  /* Mobs don't have alaises. */
  if (IS_NPC(d->character))
    return (0);

  /* bail out immediately if the guy doesn't have any aliases */
  if ((tmp = GET_ALIASES(d->character)) == NULL)
    return (0);

  /* find the alias we're supposed to match */
  ptr = any_one_arg(orig, first_arg);

  /* bail out if it's null */
  if (!*first_arg)
    return (0);

  /* if the first arg is not an alias, return without doing anything */
  if ((a = find_alias(tmp, first_arg)) == NULL)
    return (0);

  if (a->type == ALIAS_SIMPLE) {
    strcpy(orig, a->replacement);
    return (0);
  } else {
    perform_complex_alias(&d->input, ptr, a, d);
    return (1);
  }
}



