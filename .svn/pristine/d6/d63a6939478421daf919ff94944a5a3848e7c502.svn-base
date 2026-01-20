/*
** This is the Clan specific code!
*/

#ifndef _ACT_CLAN_H_
#define _ACT_CLAN_H_

#include "util/utils.h"

#ifdef  __cplusplus
extern "C" {
#endif

#define  CLAN_LEADER      6
#define  CLAN_LORD        5
#define  CLAN_OFFICER     4
#define  CLAN_GUILDSMAN   3
#define  CLAN_MEMBER      2
#define  CLAN_PLEBE       1
#define  CLAN_NONE        0

#define IF_NO_CLAN_THEN_EXIT \
    if( GET_CLAN(ch) == 0 ){ \
        send_to_char( "\r\nYou are not in a clan.\r\n", ch ); \
        return; \
    }

#define IF_NOT_MIN_CLAN_RANK(ch, lvl) \
    if( GET_CLAN_RANK(ch) < lvl ){ \
        send_to_char( "\r\nYou need a more senior clansman to do this.\r\n", ch ); \
        return; \
    }

void inc_clan_member_count( int clan_id );
void dec_clan_member_count( int clan_id );
char * get_clan_name( int clan_id );
char * get_clan_rank( int clan_id );

ACMD(do_clanadv);
ACMD(do_clandem);
ACMD(do_clanlist);
ACMD(do_clanmem);
ACMD(do_clanwho);
ACMD(do_gen_comm);
ACMD(do_clanquit);
ACMD(do_clanroster);
ACMD(do_clankick);

#ifdef  __cplusplus
}
#endif

#endif

