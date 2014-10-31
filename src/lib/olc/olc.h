/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*  _TwyliteMud_ by Rv.                          Based on CircleMud3.0bpl9 *
*    				                                          *
*  OasisOLC - olc.h 		                                          *
*    				                                          *
*  Copyright 1996 Harvey Gilpin.                                          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* Extended, and largely re-written by Vex for RavenMUD */

#ifndef _OLC_H_
#define _OLC_H_

#include "util/utils.h" /* for ACMD macro */

#define HEDIT_PERMISSION  888  /* arbitrary number higher then max zone vnum*/
#define ALL_PERMISSION    666  /* arbitrary number higher then max zone vnum*/

/* The data types for miscellaneous functions. */
#define OASIS_WLD	0
#define OASIS_MOB	1
#define OASIS_OBJ	2
#define OASIS_ZON	3
#define OASIS_EXI	4
#define OASIS_CFG	5

/*. Macros, defines, structs and globals for the OLC suite .*/

/* public functions */
void  strip_string(char *);
void  olc_add_to_save_list(int zone, byte type);
void  olc_remove_from_save_list(int zone, byte type);
int   getType(DescriptorData *d, char *value_arg, char *names[], const int num_names, const char *this_type);
int   toggleBit(DescriptorData *d, char *value_arg, char *the_bits[], const int num_bits, const char *this_flag, u_int *bitvector);
void  olcTable(DescriptorData *d, char *names[], const int num_names, bool pbit);
void  intTable(DescriptorData *d, char *names[], const int num_names, const char *intName, const int current);
void  bitTable(DescriptorData *d, char *names[], const int num_names, const char *bitName, const u_int current);
void  attackTable(DescriptorData *d);
void  weightTable(DescriptorData *d);
/* Utilities exported from oasis.c. */
void  cleanup_olc(struct descriptor_data *d, byte cleanup_type);
void  get_char_colors(struct char_data *ch);
void  split_argument(char *argument, char *tag);
void  clear_screen(struct descriptor_data *d);

/*. OLC structs .*/
struct olc_data {
  int mode;
  int zone_num;
  int vnum;			/* Room/obj vnum. */
  int value;
  bool tableDisp;         	/* To control display of tables. Vex. */
  CharData *mob;
  RoomData *room;
  ObjData *obj;
  struct config_data *config;    /* used for 'cedit'         */
  struct zone_data *zone;
  struct shop_data *shop;
  struct extra_descr_data *desc;
  struct trig_data *trig;
  QuestData *quest;
  int script_mode;
  int trigger_position;
  int item_type;
  struct trig_proto_list *script;
  char *storage; /* for holding commands etc.. */
  struct help_index_element*help;   /* Hedit uses this */
};

struct olc_save_info {
  int zone;
  char type;
  struct olc_save_info *next;
};


/*. Exported globals .*/
#ifdef _RV_OLC_
char *nrm, *grn, *cyn, *yel;
struct olc_save_info *olc_save_list = NULL;
#else
extern char *nrm, *grn, *cyn, *yel;
extern struct olc_save_info *olc_save_list;
#endif


/*. Descriptor access macros .*/
#define OLC(d)          ((d)->olc)
#define OLC_MODE(d) 	((d)->olc->mode) 	/*. Parse input mode	.*/
#define OLC_NUM(d) 	((d)->olc->vnum)	/*. Room/Obj VNUM 	.*/
#define OLC_VAL(d) 	((d)->olc->value)  	/*. Scratch variable	.*/
#define OLC_ZNUM(d) 	((d)->olc->zone_num) 	/*. Real zone number	.*/
#define OLC_ROOM(d) 	((d)->olc->room)	/*. Room structure	.*/
#define OLC_OBJ(d) 	((d)->olc->obj)	  	/*. Object structure	.*/
#define OLC_ZONE(d)     ((d)->olc->zone)	/*. Zone structure	.*/
#define OLC_MOB(d)	((d)->olc->mob)	  	/*. Mob structure	.*/
#define OLC_SHOP(d) 	((d)->olc->shop)	/*. Shop structure	.*/
#define OLC_CONFIG(d)   (OLC(d)->config)   /**< Config structure.	*/
#define OLC_DESC(d) 	((d)->olc->desc)	/*. Extra description	.*/
#define OLC_TRIG(d)     ((d)->olc->trig)        /* Trigger structure.   */
#define OLC_STORAGE(d)  ((d)->olc->storage)     /* For command storage  */
#define OLC_QUEST(d)    ((d)->olc->quest)       /* Quest structure      */

#define OLC_HELP(d)     (OLC(d)->help)          /**< Hedit structure      */
/*. Other macros .*/

#define OLC_EXIT(d)	(OLC_ROOM(d)->dir_option[OLC_VAL(d)])
#define GET_OLC_ZONE(c)	((c)->player_specials->saved.olc_zone)

/*. Cleanup types .*/
#define CLEANUP_ALL			1	/*. Free the whole lot  .*/
#define CLEANUP_STRUCTS 		2	/*. Don't free strings  .*/
#define CLEANUP_CONFIG                  3       /* Used just to send proper message. 	*/

/*. Add/Remove save list types	.*/
#define OLC_SAVE_ROOM			0
#define OLC_SAVE_OBJ			1
#define OLC_SAVE_ZONE			2
#define OLC_SAVE_MOB			3
#define OLC_SAVE_SHOP			4
#define OLC_SAVE_QUEST                  5

/* Submodes of OEDIT connectedness */
#define OEDIT_MAIN_MENU              	1
#define OEDIT_EDIT_NAMELIST          	2
#define OEDIT_SHORTDESC              	3
#define OEDIT_LONGDESC               	4
#define OEDIT_ACTDESC                	5
#define OEDIT_TYPE                   	6
#define OEDIT_EXTRAS                 	7
#define OEDIT_WEAR                  	8
#define OEDIT_WEIGHT                	9
#define OEDIT_COST                  	10
#define OEDIT_COSTPERDAY            	11
#define OEDIT_TIMER                 	12
#define OEDIT_VALUE_1               	13
#define OEDIT_VALUE_2               	14
#define OEDIT_VALUE_3               	15
#define OEDIT_VALUE_4               	16
#define OEDIT_APPLY                 	17
#define OEDIT_APPLYMOD              	18
#define OEDIT_EXTRADESC_KEY         	19
#define OEDIT_CONFIRM_SAVEDB        	20
#define OEDIT_CONFIRM_SAVESTRING    	21
#define OEDIT_PROMPT_APPLY          	22
#define OEDIT_EXTRADESC_DESCRIPTION 	23
#define OEDIT_EXTRADESC_MENU        	24
#define OEDIT_AFF_FLAGS			25


/* Submodes of REDIT connectedness */
#define REDIT_MAIN_MENU 		1
#define REDIT_NAME 			2
#define REDIT_DESC 			3
#define REDIT_FLAGS 			4
#define REDIT_SECTOR 			5
#define REDIT_EXIT_MENU 		6
#define REDIT_CONFIRM_SAVEDB 		7
#define REDIT_CONFIRM_SAVESTRING 	8
#define REDIT_EXIT_NUMBER 		9
#define REDIT_EXIT_DESCRIPTION 		10
#define REDIT_EXIT_KEYWORD 		11
#define REDIT_EXIT_KEY 			12
#define REDIT_EXIT_DOORFLAGS 		13
#define REDIT_EXTRADESC_MENU 		14
#define REDIT_EXTRADESC_KEY 		15
#define REDIT_EXTRADESC_DESCRIPTION 	16
#define REDIT_CLAN                      17

/*. Submodes of ZEDIT connectedness 	.*/
#define ZEDIT_MAIN_MENU              	0
#define ZEDIT_DELETE_ENTRY		1
#define ZEDIT_NEW_ENTRY			2
#define ZEDIT_CHANGE_ENTRY		3
#define ZEDIT_COMMAND_TYPE		4
#define ZEDIT_IF_FLAG			5
#define ZEDIT_ARG1			6
#define ZEDIT_ARG2			7
#define ZEDIT_ARG3			8
#define ZEDIT_ZONE_NAME			9
#define ZEDIT_ZONE_LIFE			10
#define ZEDIT_ZONE_TOP			11
#define ZEDIT_ZONE_RESET		12
#define ZEDIT_CONFIRM_SAVESTRING	13
#define ZEDIT_ZONE_FLAGS		14

/*. Submodes of MEDIT connectedness 	.*/
#define MEDIT_MAIN_MENU              	0
#define MEDIT_ALIAS			1
#define MEDIT_S_DESC			2
#define MEDIT_L_DESC			3
#define MEDIT_D_DESC			4
#define MEDIT_NPC_FLAGS			5
#define MEDIT_AFF_FLAGS			6
#define MEDIT_CONFIRM_SAVESTRING	7
/*. Numerical responses .*/
#define MEDIT_NUMERICAL_RESPONSE	10
#define MEDIT_SEX			11
#define MEDIT_HITROLL			12
#define MEDIT_DAMROLL			13
#define MEDIT_NDD			14
#define MEDIT_SDD			15
#define MEDIT_NUM_HP_DICE		16
#define MEDIT_SIZE_HP_DICE		17
#define MEDIT_ADD_HP			18
#define MEDIT_AC			19
#define MEDIT_EXP			20
#define MEDIT_GOLD			21
#define MEDIT_POS			22
#define MEDIT_DEFAULT_POS		23
#define MEDIT_ATTACK			24
#define MEDIT_LEVEL			25
#define MEDIT_ALIGNMENT			26
#define MEDIT_ATTACK2			27
#define MEDIT_ATTACK3			28
#define MEDIT_RACE			29
#define MEDIT_CLASS			30
#define MEDIT_SUBRACE			31
#define MEDIT_SKILL_SUC         32
#define MEDIT_SPELL_DAM         33

/*. Submodes of SEDIT connectedness 	.*/
#define SEDIT_MAIN_MENU              	0
#define SEDIT_CONFIRM_SAVESTRING	1
#define SEDIT_NOITEM1			2
#define SEDIT_NOITEM2			3
#define SEDIT_NOCASH1			4
#define SEDIT_NOCASH2			5
#define SEDIT_NOBUY			6
#define SEDIT_BUY			7
#define SEDIT_SELL			8
#define SEDIT_PRODUCTS_MENU		11
#define SEDIT_ROOMS_MENU		12
#define SEDIT_NAMELIST_MENU		13
#define SEDIT_NAMELIST			14
/*. Numerical responses .*/
#define SEDIT_NUMERICAL_RESPONSE	20
#define SEDIT_OPEN1			21
#define SEDIT_OPEN2			22
#define SEDIT_CLOSE1			23
#define SEDIT_CLOSE2			24
#define SEDIT_KEEPER			25
#define SEDIT_BUY_PROFIT		26
#define SEDIT_SELL_PROFIT		27
#define SEDIT_TYPE_MENU			29
#define SEDIT_DELETE_TYPE		30
#define SEDIT_DELETE_PRODUCT		31
#define SEDIT_NEW_PRODUCT		32
#define SEDIT_DELETE_ROOM		33
#define SEDIT_NEW_ROOM			34
#define SEDIT_SHOP_FLAGS		35
#define SEDIT_NOTRADE			36

/*. Submodes of QEDIT connectedness 	.*/
#define QEDIT_MAIN_MENU              	0
#define QEDIT_CONFIRM_SAVESTRING        1
#define QEDIT_NAME                      2
#define QEDIT_SPEECH                    3
#define QEDIT_INFO                      4
#define QEDIT_MINLVL                    5
#define QEDIT_MAXLVL                    6
#define QEDIT_TIMELIMIT                 7
#define QEDIT_ACCEPT_WAIT               8
#define QEDIT_REFUSE_WAIT               9
#define QEDIT_FLAGS                     10
#define QEDIT_PREREQUISITES             11
#define QEDIT_GIVERS                    12
#define QEDIT_TASK_MENU                 13
#define QEDIT_TASK_NEW                  14
#define QEDIT_TASK_CHANGE               15
#define QEDIT_TASK_DELETE               16
#define QEDIT_TASK_TYPE                 17
#define QEDIT_TASK_ARG                  18
#define QEDIT_REWARD_MENU               19
#define QEDIT_REWARD_NEW                20
#define QEDIT_REWARD_CHANGE             21
#define QEDIT_REWARD_DELETE             22
#define QEDIT_REWARD_TYPE               23
#define QEDIT_REWARD_ARG                24
#define QEDIT_REWARD_ACT                25

/* Used to strip bad bits off objects and mobs. */
# define STRIPBITS (AFF_GROUP      | AFF_CURSE    | \
                    AFF_POISON     | AFF_SLEEP    | \
                    AFF_CHARM      | AFF_PARALYZE | \
                    AFF_MOUNTED    | AFF_PLAGUE   | \
                    AFF_SILENCE    | AFF_BERSERK)

/* Submodes of CEDIT connectedness. */
#define CEDIT_MAIN_MENU			0
#define CEDIT_CONFIRM_SAVESTRING	1
#define CEDIT_GAME_OPTIONS_MENU		2
#define CEDIT_CRASHSAVE_OPTIONS_MENU	3
#define CEDIT_OPERATION_OPTIONS_MENU	4
#define CEDIT_DISP_EXPERIENCE_MENU	5
#define CEDIT_ROOM_NUMBERS_MENU		6
#define CEDIT_AUTOWIZ_OPTIONS_MENU	7
#define CEDIT_OK			8
#define CEDIT_NOPERSON			9
#define CEDIT_NOEFFECT			10
#define CEDIT_DFLT_IP			11
#define CEDIT_DFLT_DIR			12
#define CEDIT_LOGNAME			13
#define CEDIT_MENU			14
#define CEDIT_WELC_MESSG		15
#define CEDIT_START_MESSG		16

/* Numerical responses. */
#define CEDIT_NUMERICAL_RESPONSE	20
#define CEDIT_LEVEL_CAN_SHOUT		21
#define CEDIT_HOLLER_MOVE_COST		22
#define CEDIT_TUNNEL_SIZE		23
#define CEDIT_MAX_EXP_GAIN		24
#define CEDIT_MAX_EXP_LOSS		25
#define CEDIT_MAX_NPC_CORPSE_TIME	26
#define CEDIT_MAX_PC_CORPSE_TIME	27
#define CEDIT_IDLE_VOID			28
#define CEDIT_IDLE_RENT_TIME		29
#define CEDIT_IDLE_MAX_LEVEL		30
#define CEDIT_DTS_ARE_DUMPS		31
#define CEDIT_LOAD_INTO_INVENTORY	32
#define CEDIT_TRACK_THROUGH_DOORS	33
#define CEDIT_NO_MORT_TO_IMMORT		34
#define CEDIT_MAX_OBJ_SAVE		35
#define CEDIT_MIN_RENT_COST		36
#define CEDIT_AUTOSAVE_TIME		37
#define CEDIT_CRASH_FILE_TIMEOUT	38
#define CEDIT_RENT_FILE_TIMEOUT		39
#define CEDIT_MORTAL_START_ROOM		40
#define CEDIT_IMMORT_START_ROOM		41
#define CEDIT_FROZEN_START_ROOM		42
#define CEDIT_DONATION_ROOM_1		43
#define CEDIT_DONATION_ROOM_2		44
#define CEDIT_DONATION_ROOM_3		45
#define CEDIT_DFLT_PORT			46
#define CEDIT_MAX_PLAYING		47
#define CEDIT_MAX_FILESIZE		48
#define CEDIT_MAX_BAD_PWS		49
#define CEDIT_SITEOK_EVERYONE		50
#define CEDIT_NAMESERVER_IS_SLOW	51
#define CEDIT_USE_AUTOWIZ		52
#define CEDIT_MIN_WIZLIST_LEV		53

/* Hedit Submodes of connectedness. */
#define HEDIT_CONFIRM_SAVESTRING        0
#define HEDIT_CONFIRM_EDIT              1
#define HEDIT_CONFIRM_ADD               2
#define HEDIT_MAIN_MENU                 3
#define HEDIT_ENTRY                     4
#define HEDIT_KEYWORDS                  5
#define HEDIT_MIN_LEVEL                 6

/* public functions from cedit.c */
int  save_config( IDXTYPE nowhere );

/* public functions from dg_olc.c */
void trigedit_parse(struct descriptor_data *d, char *arg);

/* public functions from medit.c */
void medit_string_cleanup(struct descriptor_data *d, int terminator);

/* public functions from oedit.c */
void oedit_string_cleanup(struct descriptor_data *d, int terminator);

/* public functions from redit.c */
void redit_string_cleanup(struct descriptor_data *d, int terminator);

/* public functions from cedit.c */
void cedit_save_to_disk( void );
void cedit_parse(struct descriptor_data *d, char *arg);
void cedit_string_cleanup(struct descriptor_data *d, int terminator);
ACMD(do_oasis_cedit);

/* public functions from hedit.c */
void hedit_parse(struct descriptor_data *d, char *arg);
void hedit_string_cleanup(struct descriptor_data *d, int terminator);
void free_help(struct help_index_element *help);
ACMD(do_oasis_hedit);

/* public functions from tedit.c */
void tedit_string_cleanup(struct descriptor_data *d, int terminator);
ACMD(do_tedit);

/* public functions from oasis_delete.c */
int free_strings(void *data, int type);

#endif /* _OLC_H_ */
