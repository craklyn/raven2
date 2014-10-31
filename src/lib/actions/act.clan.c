/*
** This is the Clan specific code!
**
*/

#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "util/utils.h"
#include "general/comm.h"
#include "actions/interpreter.h"
#include "general/class.h"
#include "actions/act.clan.h"
#include "general/color.h"
#include "general/handler.h"
#include "magic/skills.h"
#include "magic/spells.h"
#include "util/weather.h"
#include "magic/sing.h"

ACMD(do_gen_comm);

#define MAX_CLAN_ID 255
int clan_member_counts[MAX_CLAN_ID];

char *clanLvls[] =
       { "None",
         "Plebe",   "Member", "Guildsman",
         "Officer", "Lord",   "Leader"
       };

#define MAX_CLAN_RANK (( sizeof( clanLvls ) / sizeof( clanLvls[0] )) - 1)

void
inc_clan_member_count( int clan_id )
{
  if( clan_id > 0 && clan_id < MAX_CLAN_ID)
    clan_member_counts[clan_id]++;
}

void
dec_clan_member_count( int clan_id )
{
  if( clan_id > 0 && clan_id < MAX_CLAN_ID)
    clan_member_counts[clan_id]--;
}

char *
get_clan_name( int clan_id )
{
  if( clan_id >= 0 && clan_id < MAX_CLAN_ID )
    return(clan_list[clan_id].name);
  else
  {
    static char *badClan = "*INVALID*";
    return( badClan );
    clan_id = 0;
  }
}

char *
get_clan_rank( int clan_level )
{
  if(( clan_level > 0 ) && ( clan_level <= MAX_CLAN_RANK ))
    return( clanLvls[ clan_level ] );
  else
  {
    return( "No Clan" );
    clan_level = 0;
  }
}


ACMD(do_clanadv)
{
  char       clanbuf[80];
  CHAR_DATA *victim;

  IF_NO_CLAN_THEN_EXIT;

  IF_NOT_MIN_CLAN_RANK(ch, CLAN_OFFICER);

  one_argument( argument, clanbuf );

  if( !(victim = get_char_room_vis( ch, clanbuf )))
  {
    send_to_char( "Clan advance who ?\n\r", ch );
    return;
  }

  if( IS_MOB(victim) || IS_NPC(victim))
  {
    send_to_char( "You cannot have mobs in a PC clan.", ch );
    return;
  }

  if( GET_CLAN_RANK(ch) <= GET_CLAN_RANK(victim)+1 )
  {
    send_to_char( "\r\nYou need a senior clansman to do that.\r\n", ch );
    return;
  }

  if( GET_CLAN(victim) == GET_CLAN(ch))
  {
    REMOVE_BIT_AR( PRF_FLAGS(victim), PRF_CONSENT );
    GET_CLAN_RANK(victim) += 1;
    sprintf( clanbuf, "%s has been advanced to the rank of %s !",
            GET_NAME(victim), get_clan_rank( GET_CLAN_RANK(victim)));
    do_gen_comm( ch, clanbuf, 0, SCMD_CLAN );
    return;
  }

  else if( PRF_FLAGGED( victim, PRF_CONSENT ))
  {
    REMOVE_BIT_AR( PRF_FLAGS(victim), PRF_CONSENT );
    GET_CLAN_RANK(victim) = 1;
    GET_CLAN(victim) = GET_CLAN(ch);
    inc_clan_member_count( GET_CLAN(victim) );
    sprintf( clanbuf, "%s has joined our ranks as a %s !",
             GET_NAME(victim),  get_clan_rank( GET_CLAN_RANK(victim)));
    do_gen_comm( ch, clanbuf, 0, SCMD_CLAN );
  }

  else
  {
    sendChar( ch, "Your recruit hasn't given consent for this action.\r\n" );
    return;
  }
}

ACMD(do_clankick) {
    char name[80];
    CharData *victim;
    DescriptorData *d;
    CharData *cbuf;
    int player_i;
    struct char_file_u tmp_store;
    int is_file = 0;

    one_argument( argument, name );

    IF_NO_CLAN_THEN_EXIT;
    IF_NOT_MIN_CLAN_RANK(ch, CLAN_LEADER);

    /* check the room first */
    if (!(victim = get_char_room_vis(ch, name))) {
        /* Check player list next. */
        for (d = descriptor_list; d; d = d->next) {
            if (d->connected != CON_PLAYING)
                continue;
            // If its their EXACT name.
            if (strcasecmp(name, d->character->player.name) == 0) {
                victim = d->character;
                break;
            }
        }

        // If still nobody found, we check the pfile
        if(!victim)
        {
            CREATE(cbuf, CharData, 1);
            clear_char(cbuf);
            if ((player_i = load_char(name, &tmp_store)) > -1) {
                store_to_char(&tmp_store, cbuf);
                victim = cbuf;
                is_file = 1;
            } else {
                free(cbuf);
                send_to_char("There is no such player.\r\n", ch);
                return;
            }
        }
    }

    if(!victim) {
        sendChar(ch, "Clankick who?\r\n");
        return;
    }

    if(GET_CLAN(ch) != GET_CLAN(victim)) {
        sendChar(ch, "The victim is not a member of your clan.\r\n");
    }
    else if(GET_CLAN_RANK(ch) <= GET_CLAN_RANK(victim)) {
        sendChar(ch, "\r\nYou need a senior clansman to do that.\r\n");
    }
    else {
        dec_clan_member_count( GET_CLAN(victim) );
        sprintf(name, "%s has been discharged from the clan.", GET_NAME(victim));
        do_gen_comm(ch, name, 0, SCMD_CLAN);
        GET_CLAN(victim) = 0;
        GET_CLAN_RANK(victim) = 0;
    }
    
    if (!is_file && !IS_NPC(victim))
        save_char(victim, NOWHERE);
    
    if (is_file) {
        char_to_store(victim, &tmp_store);
        fseek(player_fl, (player_i) * sizeof(struct char_file_u), SEEK_SET);
        fwrite(&tmp_store, sizeof(struct char_file_u), 1, player_fl);
        free_char(cbuf);
    }
    
}


ACMD(do_clandem)
{
  char       clanbuf[80];
  CHAR_DATA *victim;

  IF_NO_CLAN_THEN_EXIT;
  IF_NOT_MIN_CLAN_RANK(ch, CLAN_OFFICER);

  one_argument( argument, clanbuf );

  if( !(victim = get_char_room_vis( ch, clanbuf )))
  {
    sendChar( ch, "Clan demote who ?\n\r" );
    return;
  }

  if( IS_MOB(victim) ||
      IS_NPC(victim) ||
    ( GET_CLAN(victim) != GET_CLAN(ch)))
  {
    sendChar( ch, "You can only demote clan members.\n\r" );
    return;
  }

  if( GET_CLAN_RANK(ch) <= GET_CLAN_RANK(victim) )
  {
    sendChar( ch, "\r\nYou need a senior clansman to do that.\r\n" );
    return;
  }

  GET_CLAN_RANK(victim) -= 1;

  if( GET_CLAN_RANK(victim) <= 0 )
  {
    dec_clan_member_count( GET_CLAN(victim) );
    GET_CLAN_RANK(victim) = 0;
    GET_CLAN(victim) = 0;
    sprintf( clanbuf, "%s has been discharged from the clan.", GET_NAME(victim));
    do_gen_comm( ch, clanbuf, 0, SCMD_CLAN );
    return;
  }

  else
  {
    sprintf( clanbuf, "%s has been demoted to the rank of %s.",
                      GET_NAME(victim),  get_clan_rank( GET_CLAN_RANK(victim)));
    do_gen_comm( ch, clanbuf, 0, SCMD_CLAN );
  }
}

ACMD(do_clanlist)
{
    int i, j, l;
    sendChar( ch, "\r\n Clan Name            Leader(s)        Formed      "
            "Members \r\n" );
    sendChar( ch, "-------------------------------------------------------"
            "----\r\n" );

    /* skip zero'th clan! */
    for (i = 1; i < clan_count; i++) {
        if (*clan_list[i].name == '\0') continue;
        sendChar(ch, " &%02d%-20s&00 ", clan_list[i].color, clan_list[i].name);
        for (j = l = 0; j < 5; j++) {
            char *name = get_name_by_id(clan_list[i].leaders[j]);
            if (clan_list[i].leaders[j] && name) {
                if (l++) sendChar(ch, "\r\n                      ");
                sendChar(ch, "%c%-14s", toupper(name[0]), name + 1);
            }
        }
        if (!l) sendChar(ch, "               ");
        sendChar(ch, " %-10s    %-3d\r\n",
                clan_list[i].formed, clan_member_counts[i]);
    }
    sendChar(ch, "-----------------------------------------------------------"
                 "\r\n");
}

ACMD(do_clanmem)
{

}

ACMD(do_clanquit)
{
	char       clanbuf[80];
	one_argument( argument, clanbuf );
    
	IF_NO_CLAN_THEN_EXIT;

    /* Make sure they really do mean it */
    if (strncmp(clanbuf, "yes", 3) != 0) {
        sendChar(ch, "If you really really want to quit your clan, "
                     "type 'clanquit yes'.\r\n");
        return;
    }

    /* Brag about it */
    sprintf(clanbuf, "%s has removed themselves from the clan.", GET_NAME(ch));
    do_gen_comm(ch, clanbuf, 0, SCMD_CLAN);

    /* Then do it -- must be done /after/ so the do_gen_comm() will work */
    dec_clan_member_count(GET_CLAN(ch));
    GET_CLAN_RANK(ch) = 0;
    GET_CLAN(ch) = 0;
}


ACMD(do_clanwho)
{
  DescriptorData *d;
  CharData *tch;
  char   buf[132];
  int    clansmen = 0;

  IF_NO_CLAN_THEN_EXIT;

  send_to_char("Fellow Clansmen\r\n---------------\r\n", ch);

  for( d = descriptor_list; d; d = d->next )
  {
    if( d->connected != CON_PLAYING )   continue;

    if( d->original )          tch = d->original;

    else if( !(tch = d->character ))    continue; /* Bail on yourself */

    if( GET_CLAN(ch) != GET_CLAN(tch) ) continue; /* Bail on non-clansmen */

    /* Finally, bail on the unseen. */
    if( !CAN_SEE(ch, tch) ) continue;

    sendChar(ch, "%s[%2d %-8s %2s %3s%s] %s %s%s%s",
        (GET_LEVEL(tch) >= LVL_IMMORT ? CCYEL(ch, C_SPR) : ""),
        GET_LEVEL(tch),
        get_clan_rank(GET_CLAN_RANK(tch)),
        CLASS_ABBR(tch),
        RACE_ABBR(tch),
        (GET_LEVEL(tch) >= LVL_IMMORT ? CCYEL(ch, C_SPR) : ""),
        GET_NAME(tch),
        GET_TITLE(tch),
        (GET_LEVEL(tch) >= LVL_IMMORT ? CCNRM(ch, C_SPR) : ""),
        "\r\n");

    clansmen += 1;
  }

  sprintf(buf, "\r\n%d clan members displayed.\r\n", clansmen);
  sendChar(ch, buf);
}


ACMD(do_clanroster) {
    CharFileU victim, index;
    CharData *temp;
    extern FILE *player_fl;

    if(!GET_CLAN(ch) || GET_CLAN_RANK(ch) < 5) {
        sendChar(ch, "You must be at least a clan lord to see the roster.\r\n");
        return;
    }

    sendChar(ch, "Showing clan roster for [%s]\r\n\r\n", get_clan_name(GET_CLAN(ch)));

    fseek(player_fl, 0L, SEEK_SET);
    while (!feof(player_fl)) {
        fread(&index, sizeof(CharFileU), 1, player_fl);
        if (!feof(player_fl)) {
            if(index.player_specials_saved.clan_id == GET_CLAN(ch)) {
                sprintf(buf, "%s, %s\n", index.name, get_clan_rank(index.player_specials_saved.clan_rank));
                send_to_char(buf, ch);
            }
        }
    }

}

SPECIAL(clan_bank)
{
  if( !ROOM_FLAGGED(ch->in_room, ROOM_CLAN )) return 0;

  if( CMD_IS("balance") )
  {
    if( GET_CLAN_RANK(ch) < CLAN_MEMBER )
    {
      sendChar( ch, "\r\nYou need a more senior clansman to do this.\r\n" );
      return 1;
    }

    sendChar( ch, "Clan banking has not yet been enabled.\r\n" );
    return 1;
  }

  else if( CMD_IS("withdraw") )
  {
    if( GET_CLAN_RANK(ch) < CLAN_OFFICER )
    {
      sendChar( ch, "\r\nYou need a more senior clansman to do this.\r\n" );
      return 1;
    }

    sendChar( ch, "Clan banking has not yet been enabled.\r\n" );
    return 1;
  }

  else if( CMD_IS("deposit") )
  {
    sendChar( ch, "Clan banking has not yet been enabled.\r\n" );
    return 1;
  }

  return 0;
}

