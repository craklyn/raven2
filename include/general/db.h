/* ************************************************************************
 *   File: db.h                                          Part of CircleMUD *
 *  Usage: header file for database handling                               *
 *                                                                         *
 *  All rights reserved.  See license.doc for complete information.        *
 *                                                                         *
 *  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
 *  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
 ************************************************************************ */
#ifndef __DB_H__
#define __DB_H__

#include <stdio.h>
#include <sys/types.h>


/* Variables for the output buffering system */
#define MAX_SOCK_BUF       (16 * 1024) /* Size of kernel's sock buf   */
#define MAX_PROMPT_LENGTH  104          /* Max length of prompt        */
#define GARBAGE_SPACE      32          /* Space for **OVERFLOW** etc  */
#define SMALL_BUFSIZE      1024        /* Static output buffer size   */
/** Max amount of output that can be buffered */
#define LARGE_BUFSIZE      (MAX_SOCK_BUF - GARBAGE_SPACE)

 /* Constants used in creating muds data structures. */
#define MAX_STRING_LENGTH     8192  /* Max length of string, as defined */
#define MAX_INPUT_LENGTH        256    /* Max length per *line* of input */
#define MAX_RAW_INPUT_LENGTH    512   /* Max size of *raw* input */
#define MAX_MESSAGES            150  /* maximum length of fight messages. */
#define MAX_NAME_LENGTH         20   /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_PWD_LENGTH          10   /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_TITLE_LENGTH        80   /* Used in char_file_u *DO*NOT*CHANGE* */
#define HOST_LENGTH             30   /* Used in char_file_u *DO*NOT*CHANGE* */
#define PLR_DESC_LENGTH           240  /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_SKILLS              300  /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_AFFECT              64   /* (Changed to 64 at purge from 32)    */
#define MAX_OBJ_AFFECT          6    /* Used in obj_file_elem *DO*NOT*CHANGE* */
#define MAX_NOTE_LENGTH         1000   /* Max length of text on a note obj */
#define MAX_LAST_ENTRIES        6000   /* Max log entries?? */
#define MAX_HELP_KEYWORDS       256    /* Max length of help keyword string */
#define MAX_HELP_ENTRY          MAX_STRING_LENGTH /* Max size of help entry */
#define MAX_QST_PREREQ          10   /* this should be hierarchical anyway */
#define MAX_QST_REWARDS         5    /* don't spoil the mortals */
#define MAX_QST_TASKS           10   /* don't make it too easy, either */
#define MAX_QST_GIVERS          15   /* quest may be too generic if >15! */
#define MAX_CHORES              15   /* number of tasks a player must do */
#define MAX_ROOM_NAME	        75
#define MAX_MOB_NAME	        50
#define MAX_OBJ_NAME	        50
#define MAX_ROOM_DESC	        1024
#define MAX_EXIT_DESC	        1024
#define MAX_EXTRA_DESC          1024
#define MAX_MOB_DESC	        1024
#define MAX_OBJ_DESC	        1024

/*
** Arbitrary constants used by index_boot() (must be unique)
*/
#define DB_BOOT_WLD 0
#define DB_BOOT_MOB 1
#define DB_BOOT_OBJ 2
#define DB_BOOT_ZON 3
#define DB_BOOT_SHP 4
#define DB_BOOT_RND 5
#define DB_BOOT_TRG 6
#define DB_BOOT_QST 7
#define DB_BOOT_HLP 8

#define OBJEOF "#99999"

#define SUF_OBJS	"objs"
#define SUF_TEXT	"text"
#define SUF_MEM	        "mem"
#define SUF_PLR		"plr"
#define SUF_QSTS        "qsts"

#define SYS_WORLD	"world/"
#define SYS_TEXT	"text/"
#define SYS_TEXT_HELP   "text/help/"
#define SYS_MISC	"misc/"
#define SYS_ETC		"etc/"
#define SYS_PLRTEXT	"plrtext/"
#define SYS_PLROBJS	"plrobjs/"
#define SYS_PLRVARS	"plrvars/"
#define SYS_HOUSE	"house/"
#define SLASH		"/"

#define EXE_FILE        "bin/moon" /* maybe use argv[0] but it's not reliable */
#define FASTBOOT_FILE   "../.fastboot"  /* autorun: boot without sleep  */
#define KILLSCRIPT_FILE "../.killscript"/* autorun: shut mud down       */
#define PAUSE_FILE      "../pause"      /* autorun: don't restart mud   */


/* names of various files and directories */
#define INDEX_FILE	"index"		/* index of world files		*/
#define MINDEX_FILE	"index.mini"	/* ... and for mini-mud-mode	*/
#define WLD_PREFIX  SYS_WORLD"wld"SLASH	/* room definitions	*/
#define MOB_PREFIX  SYS_WORLD"mob"SLASH	/* monster prototypes	*/
#define OBJ_PREFIX  SYS_WORLD"obj"SLASH	/* object prototypes	*/
#define ZON_PREFIX  SYS_WORLD"zon"SLASH	/* zon defs & command tables */
#define SHP_PREFIX  SYS_WORLD"shp"SLASH	/* shop definitions	*/
#define RND_PREFIX  SYS_WORLD"rnd"SLASH  /* random item table defs    */
#define TRG_PREFIX  SYS_WORLD"trg"SLASH	/* trigger files	*/
#define HLP_PREFIX  SYS_TEXT"help"SLASH /* Help files           */
#define QST_PREFIX  SYS_WORLD"qst"SLASH /* quest files          */

#define CREDITS_FILE    SYS_TEXT"credits" /* for the 'credits' command	*/
#define NEWS_FILE	SYS_TEXT"news"	/* for the 'news' command	*/
#define MOTD_FILE	SYS_TEXT"motd"	/* messages of the day / mortal	*/
#define IMOTD_FILE	SYS_TEXT"imotd"	/* messages of the day / immort	*/
#define HELP_PAGE_FILE	SYS_TEXT_HELP"help"	/* for HELP <CR>		*/
#define IHELP_PAGE_FILE SYS_TEXT_HELP"ihelp"	/* for GHELP <CR>		*/
#define INFO_FILE	SYS_TEXT"info"	/* for INFO			*/
#define WIZLIST_FILE	SYS_TEXT"wizlist"	/* for WIZLIST			*/
#define IMMLIST_FILE	SYS_TEXT"immlist"	/* for IMMLIST			*/
#define BACKGROUND_FILE	SYS_TEXT"background" /* for the background story	*/
#define POLICIES_FILE	SYS_TEXT"policies"	/* player policies/rules	*/
#define HANDBOOK_FILE	SYS_TEXT"handbook"	/* handbook for new immorts	*/
#define HELP_FILE       "help.hlp"

#define IDEA_FILE	SYS_MISC"ideas"	/* for the 'idea'-command	*/
#define TYPO_FILE	SYS_MISC"typos"	/*         'typo'		*/
#define BUG_FILE	SYS_MISC"bugs"	/*         'bug'		*/
#define MESS_FILE	SYS_MISC"messages"	/* damage messages		*/
#define SOCMESS_FILE	SYS_MISC"socials"	/* messgs for social acts	*/
#define XNAME_FILE	SYS_MISC"xnames"	/* invalid name substrings	*/

/* BEGIN: Assumed default locations for logfiles, mainly used in do_file. */
/**/
#define SYSLOG_LOGFILE           "../syslog"
#define CRASH_LOGFILE            "../syslog.CRASH"
#define PREFIX_LOGFILE           "../log/"
#define BADPWS_LOGFILE           PREFIX_LOGFILE"badpws"
#define DEATHS_LOGFILE           PREFIX_LOGFILE"deaths"
#define DELETE_LOGFILE           PREFIX_LOGFILE"delete"
#define DTRAPS_LOGFILE           PREFIX_LOGFILE"dtraps"
#define ERRORS_LOGFILE           PREFIX_LOGFILE"errors"
#define GODCMDS_LOGFILE          PREFIX_LOGFILE"godcmds"
#define HELP_LOGFILE             PREFIX_LOGFILE"help"
#define LEVELS_LOGFILE           PREFIX_LOGFILE"levels"
#define METAPHYS_LOGFILE         PREFIX_LOGFILE"metaphys"
#define NEWPLAYERS_LOGFILE       PREFIX_LOGFILE"newplayers"
#define OBJECTERRORS_LOGFILE     PREFIX_LOGFILE"objecterrors"
#define OLC_LOGFILE              PREFIX_LOGFILE"olc"
#define PRAYERS_LOGFILE          PREFIX_LOGFILE"prayers"
#define REBOOTS_LOGFILE          PREFIX_LOGFILE"reboots"
#define RENTGONE_LOGFILE         PREFIX_LOGFILE"rentgone"
#define TRANSCEND_LOGFILE        PREFIX_LOGFILE"transcend"
#define TRIGGER_LOGFILE          PREFIX_LOGFILE"trigger"
#define USAGE_LOGFILE            PREFIX_LOGFILE"usage"
/**/
/* END: Assumed default locations for logfiles, mainly used in do_file. */

#define SYS_PLRFILES    SYS_ETC"players"
#define MAIL_FILE	SYS_ETC"plrmail"   /* for the mudmail system	*/
#define MAIL_FILE_TMP	SYS_ETC"plrmail_tmp"   /* for the mudmail system */
#define BAN_FILE	SYS_ETC"badsites"  /* for the siteban system	*/
#define HCONTROL_FILE	SYS_ETC"hcontrol"  /* for the house system	*/
#define CLANS_FILE      SYS_ETC"clans"     /* the clan database          */
#define RENAMES_FILE    SYS_ETC"renames"   /* the renames database       */
#define CONFIG_FILE	SYS_ETC"config"    /* Setup Config Info. */

/* 128 bits */

/* Settings for Bit Vectors */
#define RF_ARRAY_MAX    4  /* # Bytes in Bit vector - Room flags */
#define PM_ARRAY_MAX    4  /* # Bytes in Bit vector - Act and Player flags */
#define PR_ARRAY_MAX    4  /* # Bytes in Bit vector - Player Pref Flags */
#define AF_ARRAY_MAX    4  /* # Bytes in Bit vector - Affect flags */
#define TW_ARRAY_MAX    4  /* # Bytes in Bit vector - Obj Wear Locations */
#define EF_ARRAY_MAX    4  /* # Bytes in Bit vector - Obj Extra Flags */

/* preamble */
/** As of bpl20, it should be safe to use unsigned data types for the various
 * virtual and real number data types.  There really isn't a reason to use
 * signed anymore so use the unsigned types and get 65,535 objects instead of
 * 32,768. NOTE: This will likely be unconditionally unsigned later.
 * 0 = use signed indexes; 1 = use unsigned indexes */
#define CIRCLE_UNSIGNED_INDEX	0

#if CIRCLE_UNSIGNED_INDEX
# define IDXTYPE	ush_int          /**< Index types are unsigned short ints */
# define IDXTYPE_MAX USHRT_MAX     /**< Used for compatibility checks. */
# define IDXTYPE_MIN 0             /**< Used for compatibility checks. */
# define NOWHERE	((IDXTYPE)~0)    /**< Sets to ush_int_MAX, or 65,535 */
# define NOTHING	((IDXTYPE)~0)    /**< Sets to ush_int_MAX, or 65,535 */
# define NOBODY		((IDXTYPE)~0)    /**< Sets to ush_int_MAX, or 65,535 */
# define NOFLAG         ((IDXTYPE)~0)    /**< Sets to ush_int_MAX, or 65,535 */
#else
# define IDXTYPE	sh_int           /**< Index types are unsigned short ints */
# define IDXTYPE_MAX SHRT_MAX      /**< Used for compatibility checks. */
# define IDXTYPE_MIN SHRT_MIN      /**< Used for compatibility checks. */
# define NOWHERE	((IDXTYPE)-1)    /**< nil reference for rooms */
# define NOTHING	((IDXTYPE)-1)    /**< nil reference for objects */
# define NOBODY		((IDXTYPE)-1)	   /**< nil reference for mobiles  */
# define NOFLAG         ((IDXTYPE)-1)    /**< nil reference for flags   */
#endif

#define SPECIAL(name) \
        int (name)(struct char_data *ch, void *me, int cmd, char *argument)

/*
 ** These typedefs should not be used but are already in place.
 */
typedef signed char sbyte;          /* 1 byte; vals = -127 to 127 */
typedef unsigned char ubyte;        /* 1 byte; vals = 0 to 255 */
typedef signed short int sh_int;    /* 2 bytes; vals = -32,768 to 32,767 */
typedef unsigned short int ush_int; /* 2 bytes; vals = 0 to 65,535 */
#if !defined(__cplusplus)
typedef char bool; /* Technically 1 signed byte; vals should only = TRUE or FALSE. */
#endif

#if !defined(CIRCLE_WINDOWS) || defined(LCC_WIN32)	/* Hm, sysdep.h? */
typedef signed char byte; /* Technically 1 signed byte; vals should only = TRUE or FALSE. */
#endif

/* Various virtual (human-reference) number types. */
typedef int     room_vnum;  /**< vnum specifically for room */
typedef int     obj_vnum;   /**< vnum specifically for object */
typedef int     mob_vnum;   /**< NOT USED vnum specifically for mob (NPC) */
typedef ush_int zone_vnum;  /**< NOT USED vnum specifically for zone */
typedef int     shop_vnum;  /**< NOT USED vnum specifically for shop */
typedef int     trig_vnum;  /**< NOT USED vnum specifically for triggers */
typedef int     qst_vnum;       /**< NOT USED vnum specifically for quests */

/* Various real (array-reference) number types. */
typedef int     room_rnum;  /**< references an instance of a room */
typedef int     obj_rnum;   /**< references an instance of a obj */
typedef int     mob_rnum;   /**< NOT USED references an instance of a mob (NPC) */
typedef ush_int zone_rnum;  /**< references an instance of a zone */
typedef int     shop_rnum;  /**< NOT USED references an instance of a shop */
typedef int     trig_rnum;      /**< NOT USED references an instance of a trigger */
typedef int     qst_rnum;   /**< references an instance of a quest */

/** Bitvector type for 32 bit unsigned long bitvectors. 'unsigned long long'
 * will give you at least 64 bits if you have GCC. You'll have to search
 * throughout the code for "bitvector_t" and change them yourself if you'd
 * like this extra flexibility. */
typedef unsigned long int bitvector_t;

/*
 ** ================== Memory Structure for Quests ==================
 */
typedef struct quest_reward_data {
    int type;
    int amount;
    char *speech;
} QuestRewardData;

typedef struct quest_task_data {
    int type;
    int identifier;
} QuestTaskData;

typedef struct quest_data {
    int virtual; /* virtual number of this quest */
    int number; /* real number of this quest */
    char *name; /* short name */
    char *speech; /* speech given by mob */
    char *description; /* "quest info" description */
    int minlvl, maxlvl; /* min/max level of player */
    int flags; /* quest flags */
    int timelimit; /* # of ticks to complete quest */
    int waitcomplete; /* minutes to wait after completion */
    int waitrefuse; /* minutes to wait after refusal */
    int prereqs[MAX_QST_PREREQ]; /* vnums of prereq quests */
    int givers[MAX_QST_GIVERS]; /* vnums of mobs that can give quest */
    QuestRewardData rewards[MAX_QST_REWARDS];
    QuestTaskData tasks[MAX_QST_TASKS];
} QuestData;

/* data saved to disk about what quests the player has done */
typedef struct player_quest_data_saved {
    time_t next_quest; /* time the next quest can be done */
    int quest_count; /* # of quests the player has done */
    int *quests; /* array of quest vnums */
} PlayerQuestDataSaved;

/* in-memory data about the player's current quest */
typedef struct player_quest_data {
    QuestData *quest; /* the quest the player is doing */
    int time; /* # ticks left, -1 = still on offer */
    int tasks; /* bitvector of completed tasks */
    int mobrnum; /* rnum of the giving mob */
    PlayerQuestDataSaved saved;
} PlayerQuestData;

/*
 ** Extra description: used in objects, mobiles, and rooms
 */
typedef struct extra_descr_data {
    char *keyword; /* Keyword in look/examine          */
    char *description; /* What to see                      */
    struct extra_descr_data *next; /* Next in list                     */
} ExtraDescrData;


/*
 ** Object Flags; used in obj_data
 */
#define NUM_OBJ_VAL_POSITIONS 4
/* object flags; used in obj_data */
typedef struct obj_flag_data {
    int value[NUM_OBJ_VAL_POSITIONS]; /* Values of the item (see list) */
    byte type_flag;                   /* Type of item                  */
    int wear_flags[TW_ARRAY_MAX];     /* Where you can wear it         */
    int extra_flags[EF_ARRAY_MAX];    /* If it hums, glows, etc.       */
    int weight;                       /* Weight what else              */
    int cost;                         /* Value when sold (gp.)         */
    int cost_per_day;                 /* Cost to keep pr. real day     */
    int timer;                        /* Timer for object              */
    int bitvector[AF_ARRAY_MAX];      /* To set chars bits             */
} ObjFlagData;

/*
 ** Used in obj_file_elem *DO*NOT*CHANGE*
 */
typedef struct obj_affected_type {
    char location; /* Which ability to change (APPLY_XXX) */
    char modifier; /* How much it changes by              */
} ObjAffectedType;

/*
 ** ================== Memory Structure for Objects ==================
 */
typedef struct obj_data {
    obj_rnum item_number; /* Where in data-base			*/
    room_rnum in_room; /* In what room -1 when conta/carr	*/

    struct obj_flag_data obj_flags; /* Object information               */
    struct obj_affected_type affected[MAX_OBJ_AFFECT]; /* affects */

    char *name; /* Title of object :get etc.        */
    char *description; /* When in room                     */
    char *short_description; /* when worn/carry/in cont.         */
    char *action_description; /* What to write when used          */
    ExtraDescrData *ex_description; /* extra descriptions     */
    struct char_data *carried_by; /* Carried by :NULL in room/conta   */
    struct char_data *worn_by; /* Worn by?                         */
    int worn_at; /* Where's it being worn?           */
    int container; /* Used to keep track during saves  */
    u_char traplevel; /* 0 for untrapped, containers only */

    struct obj_data *in_obj; /* In what object NULL when none    */
    struct obj_data *contains; /* Contains objects                 */

    int rename_slot; /* renamed to which name?           */

    long id;
    struct trig_proto_list *proto_script;
    struct script_data *script;

    struct obj_data *next_content; /* For 'contains' lists             */
    struct obj_data *next; /* For the object list              */
} ObjData;

typedef struct obj_data OBJ_DATA;

/* ======================================================================= */

/*
 ** ====================== File Element for Objects =======================
 **                 BEWARE: Changing it will ruin rent files
 */
typedef struct obj_file_elem {
    obj_vnum item_number;
    int value[NUM_OBJ_VAL_POSITIONS];
    int extra_flags[EF_ARRAY_MAX];
    int weight;
    int timer;
    int bitvector[AF_ARRAY_MAX];
    int rename_slot;
    int spare2;
    int spare3;
    int spare4;
    int spare5;
    byte position;
    byte traplevel;
    short container;
    struct obj_affected_type affected[MAX_OBJ_AFFECT];
} ObjFileElem;

/*
 ** header block for rent files.
 ** BEWARE: Changing it will ruin rent files
 */
typedef struct rent_info {
    int time;
    int rentcode;
    int net_cost_per_diem;
    int gold;
    int account;
    int nitems;
    int spare0;
    int spare1;
    int spare2;
    int spare3;
    int spare4;
    int spare5;
    int spare6;
    int spare7;
} RentInfo;

/*
 ** room-related structures
 */

typedef struct room_direction_data {
    char *general_description; /* When looking in DIR.			*/
    char *keyword; /* for open/close           */
    ush_int exit_info; /* Exit info                */
    obj_vnum key; /* Key's number (-1 for no key)     */
    room_rnum to_room; /* Where direction leads (NOWHERE)  */
    u_char traplevel; /* trap level, 0 means none       */
} RoomDirectionData;

#define NUM_OF_DIRS    6

typedef struct room_data {
    room_vnum number; /* Rooms number (vnum)            */
    zone_rnum zone; /* Room zone (for resetting)      */
    int sector_type; /* sector type (move/hide)        */
    char *name; /* Rooms name 'You are ...'       */
    char *description; /* Shown when entered             */
    ExtraDescrData *ex_description; /* for examine/look   */
    RoomDirectionData * dir_option[NUM_OF_DIRS]; /* Directions         */
    int room_flags[RF_ARRAY_MAX]; /* DEATH,DARK ... etc             */
    u_char clan_id; /* Which clan owns this room ?    */
    u_char clan_recept_sz; /* Max items a clan can store     */
    u_char light; /* Number of lightsources in room */
    SPECIAL(*func); /* Points to special function attached to room */
    struct trig_proto_list *proto_script; /* list of default triggers */
    struct script_data *script; /* script info for the room */
    ObjData *contents; /* List of items in room          */
    struct char_data *people; /* List of NPC / PC in room       */
} RoomData;

/*
 ** char-related structures
 */

/*
 ** memory structure for characters
 */
typedef struct memory_rec_struct {
    long id;
    struct memory_rec_struct *next;
} MemoryRec;

typedef struct memory_rec_struct memory_rec;

typedef struct pardon_list {
    char *name; /* who was it */
    struct pardon_list *next; /* next in list */
} PardonList;

/* This structure is purely intended to be an easy way to transfer */

/* and return information about time (real or mudwise).            */
typedef struct time_info_data {
    byte hours;
    byte day;
    byte month;
    sh_int year;
} TimeInfoData;

/* These data contain information about a players time data */
typedef struct time_data {
    time_t birth; /* This represents the characters age                */
    time_t logon; /* Time of the last logon (used to calculate played) */
    int played; /* This is the total accumulated time played in secs */
} TimeData;

/*
 ** general player-related info, usually PC's and NPC's
 */
typedef struct char_player_data {
    char *name; /* PC / NPC s name (kill ...  ) */
    char *short_descr; /* for NPC 'actions'            */
    char *long_descr; /* for 'look'                   */
    char *description; /* Extra descriptions           */
    char *title; /* PC / NPC's title             */
    byte sex; /* PC / NPC's sex               */
    byte class; /* PC / NPC's class             */
    byte race; /* (ADDED) PC / NPC's race      */
    byte level; /* PC / NPC's level             */
    byte lostlevels; /* Number of levels lost by -xp */
    byte orcs; /* Number of orcs who hate you  */
    TimeData time; /* PC's AGE in days             */
    u_char weight; /* PC / NPC's weight            */
    u_char height; /* PC / NPC's height            */
    u_char stunned; /* Number of rounds on stun     */
    u_char arenaDly; /* Number of rounds on arena delay */
} CharPlayerData;

/*
 ** Char's abilities.  Used in char_file_u *DO*NOT*CHANGE*
 */
typedef struct char_ability_data {
    char str;
    char str_add;
    char intel;
    char wis;
    char dex;
    char con;
    char cha;
} CharAbilityData;

/*
 ** Char's points.  Used in char_file_u *DO*NOT*CHANGE*
 */
typedef struct char_point_data {
    sh_int mana;
    sh_int max_mana; /* Max move for PC/NPC                     */
    sh_int hit;
    sh_int max_hit; /* Max hit for PC/NPC                      */
    sh_int move;
    sh_int max_move; /* Max move for PC/NPC                     */

    sh_int armor; /* Internal -100..100, external -10..10 AC */
    int gold; /* Money carried                           */
    int bank_gold; /* Gold the char has in a bank account     */
    int exp; /* The experience of the player            */
    char hitroll; /* Any bonus or penalty to the hit roll    */
    char damroll; /* Any bonus or penalty to the damage roll */
} CharPointData;

/* 
 ** char_special_data_saved: specials which both a PC and an NPC have in
 ** common, but which must be saved to the playerfile for PC's.
 **
 ** WARNING:  Do not change this structure.  Doing so will ruin the
 ** playerfile.  If you want to add to the playerfile, use the spares
 ** in player_special_data.
 */
typedef struct char_special_data_saved {
    int alignment; /* +-1000 for alignments                    */
    long idnum; /* player's idnum; -1 for mobiles           */
    int act[PM_ARRAY_MAX]; /* act flag for NPC's; player flag for PC's */
    int affected_by[AF_ARRAY_MAX]; /* Bitvector for spells/skills affected by */
    sh_int apply_saving_throw[5]; /* Saving throw (Bonuses)       */
} CharSpecialDataSaved;

/*
 ** KDM(Ynnek) - Fields pertaining to the seeking code in stalk.c
 */
typedef struct char_special_data_seeking {
    struct char_data *hunting; /* Char hunted by this char (moved from char_special_data) */
    byte status; /* Status of seeker */
    byte lastDir; /* Answer of the last seek command */
    room_rnum startRoom; /* The original location of this mob */
    char *targetstr; /* Dynamic string for targetspec - nil if none */
} CharSpecialDataSeeking;

/*
 ** Special playing constants shared by PCs and NPCs which aren't in pfile
 */
typedef struct char_special_data {
    struct char_data *fighting; /* Opponent                           */
    CharSpecialDataSeeking seeking; /* vars related to  stalking.         */
    byte position; /* Standing, fighting, sleeping, etc. */
    int carry_weight; /* Carried weight                     */
    byte carry_items; /* Number of items carried            */
    int timer; /* Timer for update                   */
    int feign_flag; /* Has character just feigned death?  */
    int water_counter; /* Time till death while underwater   */
    CharSpecialDataSaved saved; /* constants saved in plrfile         */
} CharSpecialData;


/*
 *  If you want to add new values to the playerfile, do it here.  DO NOT
 * ADD, DELETE OR MOVE ANY OF THE VARIABLES - doing so will change the
 * size of the structure and ruin the playerfile.  However, you can change
 * the names of the spares to something more meaningful, and then use them
 * in your new code.  They will automatically be transferred from the
 * playerfile into memory when players log in.
 */

typedef struct player_special_data_saved
{
  u_char skills[MAX_SKILLS+1]; /* array of skills plus skill 0  */
  u_char skill_usage[MAX_SKILLS+1];
  u_char spare01;              /* Used to be race, moved it after purge */
  u_char spells_to_learn;      /* How many can you learn yet this level. */
  int    wimp_level;           /* Below this # of hit points, flee!    */
  u_char freeze_level;         /* Level of god who froze char, if any */
  short  invis_level;          /* level of invisibility  */
  short  load_room;            /* Which room to place char in  */
  int  pref[PR_ARRAY_MAX];     /* preference flags for PC's.  */
  u_char bad_pws;              /* number of bad password attemps   */
  char   conditions[3];        /* Drunk, full, thirsty         */
  u_char clan_id;
  u_char clan_rank;
  u_char sub_race;             /* Players sub_race */
  u_char pthief_countdown;
  u_char combat_delay;
  u_char race_modifier;
  char poofin[80];	       /* Poofin */
  char poofout[80];	       /* Poofout */
  int    pkill_countdown;
  u_char conj_countdown[4];       /* Time before character can use a conj spell */
  int    ac_remain;            /* Remainder from ac calcs in handler.c - stored in 8ths */
  int    jail_timer;           /* Flag Game jail control variable.         */
  int    quest_pts;            /* The number of quest points a player has  */
  char unused1;
  char unused2;
  char unused3;
  char mastery;                /* Mastery, currently only used for warrior weapons */
  int    olc_zone;             /* zone this immortal can use olc on.       */
  int    prayer_time;	       /* Time in ticks before can pray again */
  int    locker_num;           /* vnum of this player's locker, if any */
  int    assistant;            /* hp of divine assistant */
  u_char notell_level;
  u_char nogoss_level;
  u_char phunt_countdown;
  u_char chore_count;
  int    aggr_pref;
  int   recall;
  int    chores[MAX_CHORES];
  /*
  ** spares below for future expansion.  You can change the names from
  ** 'sparen' to something meaningful, but don't change the order.
  */
  int    arenarank;           /* Elo rating for arena kills */
  int    advance_level;
  int    specialization;
  char   cooldown[4];
  int    spares[10];
} PlayerSpecialDataSaved;

/*
 ** Specials needed only by PCs, not NPCs.  Space for this structure is
 ** not allocated in memory for NPCs, but it is for PCs and the portion
 ** of it labelled 'saved' is saved in the playerfile.  This structure can
 ** be changed freely; beware, though, that changing the contents of
 ** player_special_data_saved will corrupt the playerfile.
 */
typedef struct player_special_data {
    PlayerSpecialDataSaved saved;
    char *poofin; /* Description on arrival of a god.    */
    char *poofout; /* Description upon a god's exit.      */
    struct alias *aliases; /* Character's aliases                 */
    long *last_tell; /* last tell from                      */
    void *last_olc_targ; /* If oasis does'nt use this, nuke it. */
    int last_olc_mode; /* If oasis does'nt use this, nuke it. */
    PlayerQuestData quest; /* vars related to questing            */
    PardonList *pardons; /* list of PCs looted by this player   */
} PlayerSpecialData;

/* Specials used by NPCs, not PCs */
typedef struct mob_special_data {
    u_char last_direction; /* The last direction the monster went          */
    short attack_type; /* The Attack Type Bitvector for NPC's          */
    short attack_type2; /* Secoundary Attack Type                       */
    short attack_type3; /* Tertiary  Attack Type                        */
    u_char default_pos; /* Default position for NPC                     */
    u_char load_pos; /* Position NPC was loaded in(used in mobact.c) */
    memory_rec *memory; /* List of attackers to remember                */
    u_char damnodice; /* The number of damage dice's                  */
    u_char damsizedice; /* The size of the damage dice's                */
    u_char sub_race; /* NPC's sub race                               */

    /* Liam Feb 14, 1995 */
    int max_attack_damage; /* how much dam we taking?          */
    struct char_data *high_attacker; /* this is the dude thats doing it! */
    int max_spell_damage; /* how much spell dam?              */
    struct char_data *high_spellcaster; /* who is doing it?                 */
} MobSpecialData;

/*
 ** An affect structure.  (Not) Used in char_file_u *DO*NOT*CHANGE*
 */
typedef struct affected_type {
    short type; /* The type of spell that caused this       */
    short duration; /* For how long its effects will last       */
    char modifier; /* This is added to apropriate ability      */
    u_char location; /* Tells which ability to change(APPLY_XXX) */
    u_int bitvector; /* Tells which bits to set (AFF_XXX)        */
    char level;
    struct affected_type *next;
} AffectedType;

/*
 ** An affect structure.  Used in char_file_u *DO*NOT*CHANGE*
 */
typedef struct affected_type_saved {
    short type; /* The type of spell that caused this       */
    short duration; /* For how long its effects will last       */
    char modifier; /* This is added to apropriate ability      */
    u_char location; /* Tells which ability to change(APPLY_XXX) */
    u_int bitvector; /* Tells which bits to set (AFF_XXX)        */
    char level;
    char unused1;
    short unused2;
} AffectedTypeSaved;

/*
 ** Structure used for chars following other chars
 */
typedef struct follow_type {
    struct char_data *follower;
    struct follow_type *next;
} FollowType;


/*
 ** ================== Structure for player/non-player =====================
 */
#define NUM_WEARS 22   /* Changing this corrupts pfile. */

typedef struct char_data {
    short nr; /* Mob's rnum                     */
    int in_room;
    int was_in_room;

    CharPlayerData player; /* Normal data                    */
    CharAbilityData real_abils; /* Abilities without modifiers    */
    CharAbilityData aff_abils; /* Abils with spells/stones/etc   */
    CharPointData points; /* Points                         */
    CharSpecialData char_specials; /* PC/NPC specials                */
    PlayerSpecialData *player_specials; /* PC specials                    */
    MobSpecialData mob_specials; /* NPC specials                   */
    AffectedType *affected; /* affected by what spells        */
    ObjData * equipment[NUM_WEARS]; /* Equipment array                */
    ObjData *carrying; /* Head of list                   */
    struct descriptor_data *desc; /* NULL for mobiles               */

    long id; /* used by DG triggers */
    struct trig_proto_list *proto_script;
    struct script_data *script;
    struct script_memory *memory;

    struct char_data *next_in_room; /* For room->people - list        */
    struct char_data *next; /* For either monster or ppl-list */
    struct char_data *next_fighting; /* For fighting list              */

  FollowType *followers;              /* List of chars followers        */
  struct char_data *master;           /* Who is char following?         */
  struct char_data *guarding;         /* Who is char guarding ?         */
  struct char_data *mount;            /* What is char riding ?          */
  struct char_data *rider;            /* What is riding char ?          */
  struct char_data *fortifiee;        /* Who is char fortifying ?       */
  struct char_data *fortifier;        /* Who is fortifying char ?       */
  u_char tickstate;                   /* Once per tick states           */
  short singing;                        /* song number of any song        */
  struct char_data *target;           /* who their song or spell is targeting */
  char *ambushing;                    /* keyword to ambush              */
  u_char powerstrike;                 /* powerstrike level, if any      */
  u_char stance;                      /* STANCE_XXX                     */
  room_rnum false_trail;               /* False trail room               */
  room_rnum necro_corpse_recall_room;  /* Maestro - holds necro recall pt*/
  int state;			      /* Craklyn- State of chant/prayer */
  char offbalance;		      /* Craklyn- Char thrown off balance, deserves spanked.*/
  char call_to_corpse;                 /* Chars running from death       */
  char vote;			      /* One man, one vote              */
  char flee_timer;		      /* Fleeing makes player vulnerable*/
  short mobskill_suc;                   /* How good at ability is the mob */
  short mobspell_dam;                   /* How much damage do spells do?  */
  short sacrifice;                      /* Protection for necromancers    */
  char herbs;                          /* How many poultices left?       */
  char aspect;                         /* ASPECT_XXX                     */
  char timer;                          /* Keeps track of conj. pet type  */
  short decay;                          /*How much has this corpse rotted?*/
  u_short muckleTime;                  /* How long as this character been the muckle? */
  short playerDamroll;
} CharData;
typedef struct char_data CHAR_DATA;

/* ====================================================================== */


/* ==================== File Structure for Player ======================= */
/*             BEWARE: Changing it will ruin the playerfile    */

typedef struct char_file_u { /* char_player_data */
    u_char name[MAX_NAME_LENGTH + 1];
    u_char description[PLR_DESC_LENGTH];
    u_char title[MAX_TITLE_LENGTH + 1];
    u_char sex;
    u_char class;
    u_char race;
    u_char level;
    u_char lostlevels;
    u_char orcs;
    time_t birth;
    int played; /* Number of secs played in total */
    u_char weight;
    u_char height;
    u_char pwd[MAX_PWD_LENGTH + 1]; /* password */
    CharSpecialDataSaved char_specials_saved;
    PlayerSpecialDataSaved player_specials_saved;
    CharAbilityData abilities;
    CharPointData points;
    AffectedTypeSaved affected[MAX_AFFECT];
    time_t last_logon; /* Time (in secs) of last logon */
    u_char host[HOST_LENGTH + 1]; /* host of last logon */
} CharFileU;
typedef struct char_file_u CHAR_FILE_U;
/* ====================================================================== */

/* descriptor-related structures ******************************************/


typedef struct txt_block {
    char *text;
    int aliased;
    struct txt_block *next;
} TxtBlock;

typedef struct txt_q {
    TxtBlock *head;
    TxtBlock *tail;
} TxtQ;

typedef struct descriptor_data {
    int descriptor; /* file descriptor for socket          */
    char host[HOST_LENGTH + 1]; /* hostname                            */
    char pwd[MAX_PWD_LENGTH + 1]; /* password                            */
    char bad_pws; /* number of bad pw attemps this login */
    int pos; /* position in player-file             */
    int connected; /* mode of 'connectedness'             */
    int dcPending; /* a disconnect is pending             */
    int dcTimer; /* ticks until an automatic dc occurs  */
    int wait; /* wait for how many loops             */
    int desc_num; /* unique num assigned to desc         */
    long login_time; /* when the person connected           */
    char *showstr_head; /* for paging through texts            */
    char *showstr_point; /*      -                              */
    char **str; /* for the modify-str system           */
    char *backstr; /* added for handling abort buffers    */
    int max_str; /*      -                              */
    long mail_to; /* name for mail system                */
    int prompt_mode; /* control of prompt-printing          */
    char inbuf[MAX_RAW_INPUT_LENGTH]; /* buffer for raw input         */
    char last_input[MAX_INPUT_LENGTH]; /* the last input               */
    char small_outbuf[SMALL_BUFSIZE]; /* standard output buffer       */
    char *output; /* ptr to the current output buffer    */
    int bufptr; /* ptr to end of current output        */
    int bufspace; /* space left in the output buffer     */
    TxtBlock *large_outbuf; /* ptr to large buffer, if we need it  */
    TxtQ input; /* q of unprocessed input              */
    CharData *character; /* linked to char                      */
    CharData *original; /* original char if switched           */
    struct descriptor_data *snooping; /* Who is this char snooping     */
    struct descriptor_data *snoop_by; /* And who is snooping this char */
    struct olc_data *olc; /* OLC info - defined in olc.h         */
    char *storage; /* ReUse, was for text wiz_comm. Now uses olc_data structure */
    char noisy; /* counter to prevent spamming         */
    char spammy; /* and another to prevent tell spams   */
    struct descriptor_data *next; /* link to next descriptor             */
} DescriptorData;


/* other miscellaneous structures ***************************************/

/*
 ** element in monster and object index-tables
 */
typedef struct index_data {
    int virtual; /* virtual number of this mob/obj           */
    int number; /* number of existing units of this mob/obj */
    SPECIAL(*func);
    /* Special procedures invoked in the "damage" function in fight.c */
    int (*combatSpec)(void *me, CharData *victim, int *damage, int *attacktype);

    char *farg;
    struct trig_data *proto;
} IndexData;

/* linked list for mob/object prototype trigger lists */
struct trig_proto_list {
    int vnum; /* vnum of the trigger */
    struct trig_proto_list *next; /* next trigger */
};

#define REAL 0
#define VIRTUAL 1

/* structure for the reset commands */
typedef struct reset_com {
   char	command;   /* current command                      */

   bool if_flag;	/* if TRUE: exe only if preceding exe'd */
   int	arg1;		/*                                      */
   int	arg2;		/* Arguments to the command             */
   int	arg3;		/*                                      */
   int line;		/* line number this command appears on  */
   char *sarg1;		/* string argument                      */
   char *sarg2;		/* string argument                      */

   /* Commands:
    *  'M': Read a mobile
    *  'O': Read an object
    *  'G': Give obj to mob
    *  'P': Put obj in obj
    *  'G': Obj to char
    *  'E': Obj to char equip
    *  'D': Set state of door
    *  'Z': Trigger command
    *  'V': Assign a variable */
} ResetCom;

/*
 ** zone definition structure. for the 'zone-table'
 **
 **  Reset mode:
 **  0: Don't reset, and don't update age.
 **  1: Reset if no PC's are located in zone.
 **  2: Just reset.
 **  3: Zone is OFFLIMITS to mortals but reset like a 2.
 */
typedef struct zone_data {
   char	*name;		    /* name of this zone                  */
   char *builders;          /* namelist of builders allowed to    */
                            /* modify this zone.		  */
   int	lifespan;           /* how long between resets (minutes)  */
   int	age;                /* current age of this zone (minutes) */
   room_vnum bot;           /* starting room number for this zone */
   room_vnum top;           /* upper limit for rooms in this zone */

    int zone_flags; /* Zone Flags */

    int number; /* virtual number of this zone     */
    int tournament_room;
   int min_level;                 /* Minimum level a player must be to enter this zone */
   int max_level;                 /* Maximum level a player must be to enter this zone */
    int kill_count;
    int pkill_room; /* Full player killing permitted */
    int reset_mode; /* conditions for reset (see below)      */
    ResetCom *cmd;
} ZoneData;

/*
 ** for queueing zones for update
 */
typedef struct reset_q_element {
    int zone_to_reset; /* ref to zone_data */
    struct reset_q_element *next;
} ResetQElement;

/* structure for the update queue     */
typedef struct reset_q_type {
    ResetQElement *head;
    ResetQElement *tail;
} ResetQType;

/* ============================================================================
Clan file structure - if you change this, clan.db will be broken!
============================================================================ */
typedef struct clan_guard {
    int room;
    int dir;
    char name[MAX_NAME_LENGTH + 1];
} ClanGuard;

typedef struct clan_file_entry {
    char name[23];
    long leaders[5];
    int home;
    char formed[12];
    int color;
    ClanGuard guards[3];
} ClanFileEntry;

extern ClanFileEntry *clan_list;
extern int clan_count;

/* ============================================================================ 
Random item structures.
============================================================================ */
typedef struct rand_item {
    int number; /* the current number of the item that has been loaded randomly */
    int max; /* the maximun number that will be loaded randomly */
    int virtual; /* the virtual number of the object */
    int min_val; /* the minimum reset value of the object */
} RandItem;

typedef struct rand_data {
    char *name; /* name for the table */
    int number; /* number of the table */
    int total; /* total amount of items in the table */
    RandItem *item;
} RandData;

/*
 ** Index structures.
 */
typedef struct player_index_element {
    char *name;
    long id;
} PlayerIndexElement;

struct help_index_element {
   char *index;      /*Future Use */
   char *keywords;   /*Keyword Place holder and sorter */
   char *entry;      /*Entries for help files with Keywords at very top*/
   int duplicate;    /*Duplicate entries for multple keywords*/
   int min_level;    /*Min Level to read help entry*/
};

typedef struct locker_list {
    ObjData *locker;
    struct locker_list *next;
} LockerList;

/* ============================================================================ 
Stuff for renamed items.
============================================================================ */

#define MAX_RENAMES 10000

typedef struct rename {
    int renamer_id;
    char name[MAX_OBJ_NAME];
} Rename;

extern Rename renames[MAX_RENAMES];
extern int top_of_rename_t;
extern void save_renames(void);
extern void load_renames(void);

/*
 ** The generic buffers that are used all OVER the place.
 */
extern char buf[MAX_STRING_LENGTH];
extern char buf1[MAX_STRING_LENGTH];
extern char buf2[MAX_STRING_LENGTH];
extern char arg[MAX_STRING_LENGTH];

/*
 ** Exported Function Prototypes
 */
extern void boot_db(void);
extern void  destroy_db(void);
extern int create_entry(char *name);
extern void zone_update(void);
extern int real_room(int virtual);
extern char *fread_string(FILE *fl, char *error);
extern long get_id_by_name(char *name);
extern char *get_name_by_id(long id);
extern int getStartRoom(CharData *ch);

void  free_text_files(void);
void  free_help_table(void);
void  free_player_index(void);
void  load_help(FILE *fl, char *name);

extern void char_to_store(CharData *ch, struct char_file_u *st);
extern void store_to_char(struct char_file_u *st, CharData *ch);
extern int load_char(char *name, struct char_file_u *char_element);
extern int load_char_quests(CharData *ch);
//digger changed ush_int
extern void save_char(CharData *ch, ush_int load_room);
extern void init_char(CharData *ch);
extern CharData *create_char(void);
extern void build_player_index(void);
extern CharData *read_mobile(int nr, int type);
extern int real_zone(int virtual);
extern int real_mobile(int virtual);
extern int real_quest(int virtual);
extern int vnum_mobile(char *searchname, CharData *ch);
extern void clear_char(CharData *ch);
extern void reset_char(CharData *ch);
extern void free_char(CharData *ch);
extern void clear_object(ObjData *obj);
extern void free_obj(ObjData *obj);
extern int real_object(int virtual);
extern int vnum_object(char *searchname, CharData *ch);
void reset_zone(int zone);
extern ObjData *create_obj(void);
extern ObjData *read_object(int nr, int type);
extern ObjData *read_perfect_object(int nr, int type);
extern ObjData *read_failed_object(int nr, int type);
extern ObjData *random_object(int table, int min_val);
void index_boot(int mode);
void reboot_wizlists(void);
extern void save_clans(void);
void load_config( void );

#ifndef __DB_C__

/* Various Files */
extern char *credits;    /* game credits            */
extern char *news;       /* mud news                */
extern char *motd;       /* mort message of the day */
extern char *imotd;      /* imm message of the day  */
extern char *help;       /* help screen             */
extern char *ihelp;
extern char *info;       /* info page               */
extern char *wizlist;    /* list of higher gods     */
extern char *immlist;    /* list of peon gods       */
extern char *background; /* background story        */
extern char *policies;   /* policies page           */
extern char *handbook;   /* immortal handbook       */

/* The ingame helpfile */
extern int top_of_helpt;
extern struct help_index_element *help_table;

/* Mud configurable variables */
extern int no_mail;                  /* mail disabled?            */
extern int mini_mud;                 /* mini-mud mode?            */
extern int no_rent_check;            /* skip rent check on boot?  */
extern long boot_time;               /* time of mud boot          */
extern int circle_restrict;          /* level of game restriction */

extern struct config_data config_info;

extern TimeInfoData time_info;       /* the infomation about the time */
extern CharData *character_list;     /* global linked list of chars */
extern PlayerSpecialData dummy_mob;  /* dummy spec area for mobs      */
extern ResetQType reset_q;           /* queue of zones to be reset    */

extern RoomData *world;              /* array of rooms              */
extern int top_of_world;             /* ref to top element of world */

extern ZoneData *zone_table;         /* zone table                   */
extern int top_of_zone_table;        /* top element of zone tab  */

extern IndexData *mob_index;         /* index table for mobile file  */
extern CharData *mob_proto;          /* prototypes for mobs          */
extern int top_of_mobt;              /* top of mobile index table    */

extern IndexData *obj_index;         /* index table for object file  */
extern ObjData *object_list;         /* global linked list of objs   */
extern ObjData *obj_proto;           /* prototypes for objs          */
extern int top_of_objt;              /* top of object index table    */

extern struct index_data **trig_index;
extern struct trig_data *trigger_list;
extern int top_of_trigt;
extern int dg_owner_purged;

extern QuestData *qst_list;
extern int top_of_qstt;

extern RandData *rand_table;
extern int top_of_rand_table;

extern LockerList *lockers;              /* list of lockers in the game */

extern PlayerIndexElement *player_table; /* index to plr file        */
extern FILE *player_fl;                  /* file desc of player file */
extern int top_of_p_table;               /* ref to top of table      */
extern int top_of_p_file;                /* ref of size of p file    */
extern long top_idnum;                   /* highest idnum in use     */
/* end previously located in players.c */

#endif /* __DB_C__ */

#endif /* _DB_H_ */
