/* ============================================================================
Header file for ban.
Header set up by Xiuh of RavenMUD for RavenMUD. 04.11.09
============================================================================ */
#ifndef _BAN_H_
#define _BAN_H_

/* ============================================================================
Stuff for banned site list.
============================================================================ */
/* don't change these */
#define BAN_NOT      0
#define BAN_SHUNNED  1
#define BAN_RAWLOG   2
#define BAN_NEW      3
#define BAN_SELECT   4
#define BAN_ALL      5

#define BANNED_SITE_LENGTH    50

/* ============================================================================
ban_List structures.
============================================================================ */
struct ban_list_element {
    char site[BANNED_SITE_LENGTH + 1];
    int type;
    time_t date;
    char name[MAX_NAME_LENGTH + 1];
    struct ban_list_element *next;
};

/* ============================================================================
Global functions.
============================================================================ */
extern void load_banned();
extern int isbanned(char *hostname);
void Read_Invalid_List(void);
void free_invalid_list(void);
/* Command functions without subcommands */
extern ACMD(do_ban);
extern ACMD(do_unban);

#ifndef __BAN_C__

extern struct ban_list_element *ban_list;
extern int num_invalid;

#endif /*__BAN_C__ */

#endif /* _BAN_H_*/
