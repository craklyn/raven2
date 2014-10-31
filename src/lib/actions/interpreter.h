/* ************************************************************************
*   File: interpreter.h                                 Part of CircleMUD *
*  Usage: header file: public procs, macro defs, subcommand defines       *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#ifndef _INTERPRETER_H_
#define _INTERPRETER_H_

#define CMD_NAME (cmd_info[cmd].command)
#define CMD_IS(cmd_name) (!strcmp(cmd_name, cmd_info[cmd].command))
#define IS_MOVE(cmdnum) (cmdnum >= 1 && cmdnum <= 6)

void	command_interpreter(CharData *ch, char *argument);
int	search_block(char *arg, const char **list, int exact);
char	*one_argument(char *argument, char *first_arg);
char	*one_word(char *argument, char *first_arg);
char	*any_one_arg(char *argument, char *first_arg);
extern char	lower( char c );
extern char	*case_one_arg(char *argument, char *first_arg);
extern char	*two_arguments(char *argument, char *first_arg, char *second_arg);
extern int	fill_word(char *argument);
extern void	half_chop(char *string, char *arg1, char *arg2);
extern void	case_chop(char *string, char *arg1, char *arg2);
extern void	nanny(DescriptorData *d, char *arg);
extern int	is_abbrev(const char *arg1, const char *arg2);
extern int	is_number(char *str);
extern int	find_command(char *command);
extern void	skip_spaces(char **string);
extern char	*delete_doubledollar(char *string);

/* From act.movement.c */
int     has_key(CharData *, int, ObjData ** );

/* From act.obj.c */
void get_check_money(CharData * ch, ObjData * obj);

/* Begin Functions and defines for act.informative.c */
void space_to_minus(char *str);
int search_help(const char *argument, int level);


/* for compatibility with 2.20: */
#define argument_interpreter(a, b, c) two_arguments(a, b, c)


struct command_info {
   char *command;
   byte minimum_position;
   void	(*command_pointer)
   (CharData *ch, char * argument, int cmd, int subcmd);
   sh_int minimum_level;
   int	subcmd;
   int  unaffFlags;;
};

struct alias {
  char *alias;
  char *replacement;
  int type;
  struct alias *next;
};

#define ALIAS_SIMPLE	0
#define ALIAS_COMPLEX	1

#define ALIAS_SEP_CHAR	';'
#define ALIAS_VAR_CHAR	'$'
#define ALIAS_GLOB_CHAR	'*'

#define ALIAS_SAVE_MAX  1000

/*
 * SUBCOMMANDS
 *   You can define these however you want to, and the definitions of the
 *   subcommands are independent from function to function.
 */

/* directions */
#define SCMD_NORTH	1
#define SCMD_EAST	2
#define SCMD_SOUTH	3
#define SCMD_WEST	4
#define SCMD_UP		5
#define SCMD_DOWN	6

/* do_seek */
#define SCMD_SET        0
#define SCMD_UNSET      1
#define SCMD_QUERY      2

/* do_gen_ps */
#define SCMD_INFO       0
#define SCMD_HANDBOOK   1 
#define SCMD_CREDITS    2
#define SCMD_NEWS       3
#define SCMD_WIZLIST    4
#define SCMD_POLICIES   5
#define SCMD_VERSION    6
#define SCMD_IMMLIST    7
#define SCMD_MOTD	8
#define SCMD_IMOTD	9
#define SCMD_CLEAR	10
#define SCMD_WHOAMI	11

/* do_gen_tog */
#define SCMD_NOSUMMON   0
#define SCMD_NOHASSLE   1
#define SCMD_BRIEF      2
#define SCMD_COMPACT    3
#define SCMD_NOTELL	4
#define SCMD_NOAUCTION	5
#define SCMD_DEAF	6
#define SCMD_NORPLAY 	7
#define SCMD_NOGRATZ	8
#define SCMD_NOARENA	9
#define SCMD_QUEST	10
#define SCMD_ROOMFLAGS	11
#define SCMD_NOREPEAT	12
#define SCMD_HOLYLIGHT	13
#define SCMD_SLOWNS	14
#define SCMD_AUTOEXIT	15
#define SCMD_ANON       16
#define SCMD_CONSENT    17
#define SCMD_PLAGUE     18
#define SCMD_RAWLOG     19
#define SCMD_SPAM       20
#define SCMD_OLCVERBOSE 21
#define SCMD_NOOOC      22
#define SCMD_NOQUERY    23
#define SCMD_NOGUILD    24
#define SCMD_NORECALL   25
#define SCMD_AUTOSPLIT  26
#define SCMD_AUTOLOOT	27
#define SCMD_NOCLAN	28
#define SCMD_AUTOGOLD	29
#define SCMD_PKILL      30
#define SCMD_PSTEAL     31
#define SCMD_NOWHO      32
#define SCMD_SHOWDAM    33
#define SCMD_CLS        34
#define SCMD_NOWIZ      35
#define SCMD_AFK        36
#define SCMD_COLOR      37
#define SCMD_SYSLOG     38
#define SCMD_WIMPY      39

/* do_wizutil */
#define SCMD_REROLL	0
#define SCMD_PARDON     1
#define SCMD_NOTITLE    2
#define SCMD_SQUELCH    3
#define SCMD_FREEZE	4
#define SCMD_THAW	5
#define SCMD_UNAFFECT	6

/* do_spec_com */
#define SCMD_WHISPER	0
#define SCMD_ASK	1

/* do_gen_com */
#define SCMD_HOLLER	0
#define SCMD_SHOUT	1
#define SCMD_ROLEPLAY   2
#define SCMD_AUCTION	3
#define SCMD_GRATZ	4
#define SCMD_CLAN       5
#define SCMD_GUILD      6
#define SCMD_OOC        7
#define SCMD_QUERYSAY   8

/* do_shutdown */
#define SCMD_SHUTDOW	0
#define SCMD_SHUTDOWN   1
#define SCMD_REBOOT     2

/* do_quit */
#define SCMD_QUI	0
#define SCMD_QUIT	1

/* do_date */
#define SCMD_DATE	0
#define SCMD_UPTIME	1

/* do_commands */
#define SCMD_COMMANDS	0
#define SCMD_SOCIALS	1
#define SCMD_WIZHELP	2

/* do_drop */
#define SCMD_DROP	0
#define SCMD_JUNK	1
#define SCMD_DONATE	2
#define SCMD_DEST	3
#define SCMD_CAPT   4

/* do_gen_write */
#define SCMD_BUG	0
#define SCMD_TYPO	1
#define SCMD_IDEA	2

/* do_look */
#define SCMD_LOOK	0
#define SCMD_READ	1

/* do_gecho */
#define SCMD_GECHO	0
#define SCMD_QECHO	1

/* do_pour */
#define SCMD_POUR	0
#define SCMD_FILL	1

/* do_advance_tog */
#define SCMD_POTENCY     0
#define SCMD_DETERRENCE  1
#define SCMD_SHADOWFORM  2
#define SCMD_PYRE        3
#define SCMD_COM_SHOUT   4
#define SCMD_SACRIFICE   5
#define SCMD_METAMORPH   6
#define SCMD_MEND_PET    7
#define SCMD_PHASESHIFT  8
#define SCMD_HIPP        9

/* do_poof */
#define SCMD_POOF	0
#define SCMD_POOFIN	1
#define SCMD_POOFOUT	2

/* do_hit */
#define SCMD_HIT	0
#define SCMD_MURDER	1

/* do_eat */
#define SCMD_EAT	0
#define SCMD_TASTE	1
#define SCMD_DRINK	2
#define SCMD_SIP	3

/* do_use */
#define SCMD_USE	0
#define SCMD_QUAFF	1
#define SCMD_RECITE	2
#define SCMD_TOSS       3

/* do_echo */
#define SCMD_ECHO	0
#define SCMD_EMOTE	1

/* do_olc */
#define SCMD_OLC_REDIT 0
#define SCMD_OLC_OEDIT 1
#define SCMD_OLC_ZEDIT 2
#define SCMD_OLC_MEDIT 3
#define SCMD_OLC_SEDIT 4
#define SCMD_OLC_TRIGEDIT 5
#define SCMD_OLC_QEDIT 6
#define SCMD_OLC_SAVEINFO 7

/* do_questPnts */
#define SCMD_QPADD	0
#define SCMD_QPDEC	1
#define SCMD_QPSHOW	2
#define SCMD_QPSET	3
#define SCMD_QPCLR	4

/* do_liblist */
#define SCMD_RLIST	0
#define SCMD_OLIST	1
#define SCMD_MLIST	2
#define SCMD_ZLIST	3

/* do_steal */
#define SCMD_STEAL      0
#define SCMD_MUG        1

/* Necessary for CMD_IS macro.  Borland needs the structure defined first
 * so it has been moved down here. */
/* Global buffering system */
#ifndef __INTERPRETER_C__

//extern int *cmd_sort_info;
//extern struct command_info *complete_cmd_info;
extern struct command_info cmd_info[];

#endif /* __INTERPRETER_C__ */

#endif
