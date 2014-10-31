
#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "util/utils.h"
#include "scripts/dg_scripts.h"
#include "util/weather.h"
#include "general/comm.h"
#include "general/class.h"
#include "general/handler.h"
#include "magic/spells.h"
#include "magic/sing.h"

extern struct room_data *world;

/* same as any_one_arg except that it stops at punctuation */
char *any_one_name(char *argument, char *first_arg)
{
    char* arg;

    /* Find first non blank */
    while(isspace(*argument))
        argument++; 
 
    /* Find length of first word */
    for(arg = first_arg ;
        *argument && !isspace(*argument) &&
          (!ispunct(*argument) || *argument == '#' || *argument == '-') ;
        arg++, argument++)
        *arg = LOWER(*argument);
    *arg = '\0';

    return argument;
}


void sub_write_to_char(char_data *ch, char *tokens[],
		       void *otokens[], char type[])
{
    char sb[MAX_STRING_LENGTH];
    int i;

    strcpy(sb,"");

    for (i = 0; tokens[i + 1]; i++)
    {
	strcat(sb,tokens[i]);

	switch (type[i])
	{
	case '~':
	    if (!otokens[i])
		strcat(sb,"someone");
	    else if ((char_data *)otokens[i] == ch)
		strcat(sb,"you");
	    else
		strcat(sb, PERS((char_data *)otokens[i], ch));
	    break;

	case '@':
	    if (!otokens[i])
		strcat(sb,"someone's");
	    else if ((char_data *)otokens[i] == ch)
		strcat(sb,"your");
	    else
	    {
		strcat(sb,PERS((char_data *) otokens[i], ch));
		strcat(sb,"'s");
	    }
	    break;

	case '^':
	    if (!otokens[i] || !CAN_SEE(ch, (char_data *) otokens[i]))
		strcat(sb,"its");
	    else if (otokens[i] == ch)
		strcat(sb,"your");
	    else
		strcat(sb,HSHR((char_data *) otokens[i]));
	    break;
		
	case '#':
	    if (!otokens[i] || !CAN_SEE(ch, (char_data *) otokens[i]))
		strcat(sb,"it");
	    else if (otokens[i] == ch)
		strcat(sb,"you");
	    else
		strcat(sb,HSSH((char_data *) otokens[i]));
	    break;
	    
	case '*':
	    if (!otokens[i] || !CAN_SEE(ch, (char_data *) otokens[i]))
		strcat(sb,"it");
	    else if (otokens[i] == ch)
		strcat(sb,"you");
	    else
		strcat(sb,HMHR((char_data *) otokens[i]));
	    break;

	case '`':
	    if (!otokens[i])
		strcat(sb,"something");
	    else
		strcat(sb,OBJS(((obj_data *) otokens[i]), ch));
	    break;
	}
    }

    strcat(sb,tokens[i]);
    strcat(sb,"\n\r");
    sb[0] = toupper(sb[0]);
    send_to_char(sb,ch);
}

/* Xiuh - 02.09.09
 * sub_write is used by %echo% and %send% to ensure that the correct info is sent
 * to the correct people. I've updated this function from tbamud-3.58 to fix
 * compile errors on gcc 4.3.2*/
void sub_write(char *arg, char_data *ch, byte find_invis, int targets)
{
    char str[MAX_INPUT_LENGTH * 2];
    char type[MAX_INPUT_LENGTH], name[MAX_INPUT_LENGTH];
    char *tokens[MAX_INPUT_LENGTH], *s, *p;
    void *otokens[MAX_INPUT_LENGTH];
    char_data *to;
    obj_data *obj;
    int i, tmp;
  int to_sleeping = 1; /* mainly for windows compiles */

    if (!arg)
	return;
    
    tokens[0] = str;
    
  for (i = 0, p = arg, s = str; *p;) {
	switch (*p) {
	case '~':
	case '@':
	case '^':
	case '#':
	case '*':
	    /* get char_data, move to next token */
	    type[i] = *p;
	    *s = '\0';
	    p = any_one_name(++p, name);
	    (char_data *)otokens[i] =
		find_invis ? get_char(name) : get_char_room_vis(ch, name);
	    tokens[++i] = ++s;
	    break;
	    
	case '`':
	    /* get obj_data, move to next token */
	    type[i] = *p;
	    *s = '\0';
	    p = any_one_name(++p, name);

            if (find_invis) obj = get_obj(name);
            else if (!(obj = get_obj_in_list_vis(ch, name, world[IN_ROOM(ch)].contents))) ;
            else if (!(obj = get_object_in_equip_vis(ch, name, ch->equipment, &tmp))) ;
            else obj = get_obj_in_list_vis(ch, name, ch->carrying);

	    otokens[i] = (void *)obj;
	    tokens[++i] = ++s;
	    break;

	case '\\':
	    p++;
	    *s++ = *p++;
	    break;
	    
	default:
	    *s++ = *p++;
	}
    }

    *s = '\0';
    tokens[++i] = NULL;

    if (IS_SET(targets, TO_CHAR) && SENDOK(ch))
	sub_write_to_char(ch, tokens, otokens, type);

    if (IS_SET(targets, TO_ROOM))
	for (to = world[ch->in_room].people;
	     to; to = to->next_in_room)
	    if (to != ch && SENDOK(to))
		sub_write_to_char(to, tokens, otokens, type);
}

void send_to_zone(char *messg, int zone_rnum)
{
  struct descriptor_data *i;

  if (!messg || !*messg)
    return;

  for (i = descriptor_list; i; i = i->next)
    if (!i->connected && i->character && AWAKE(i->character) &&
        (IN_ROOM(i->character) != NOWHERE) &&
        (world[IN_ROOM(i->character)].zone == zone_rnum))
      write_to_output(i, "%s", messg);
}

