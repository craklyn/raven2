/* ************************************************************************
*   File: db.c                                          Part of CircleMUD *
*  Usage: Loading/saving chars, booting/resetting world, internal funcs   *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#define __DB_C__

#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "actions/act.clan.h"
#include "general/class.h"
#include "general/comm.h"
#include "general/chores.h"
#include "general/handler.h"
#include "specials/house.h"
#include "actions/interpreter.h"
#include "olc/olc.h"
#include "specials/mail.h"
#include "magic/skills.h"
#include "magic/spells.h"
#include "util/utils.h"
#include "actions/ban.h"
#include "general/mudconfig.h"    /* for the default config values. */
#include "olc/oedit.h"
#include "olc/medit.h"
#include "olc/redit.h"
#include "specials/shop.h"
#include "util/weather.h"
#include "specials/combspec.h"
#include "scripts/dg_scripts.h"
#include "actions/quest.h"
#include "specials/scatter.h"     /* for scatter_init */
#include "general/objsave.h"

/*  declarations of most of the 'global' variables */

/* Please make your own buffs and don't use the nasty buf, buf1, buf2, and arg */

char buf[MAX_STRING_LENGTH];
char buf1[MAX_STRING_LENGTH];
char buf2[MAX_STRING_LENGTH];
char arg[MAX_STRING_LENGTH];

struct config_data config_info; /* Game configuration list.	 */

CharData *character_list;  /* global linked list of chars */
IndexData **trig_index;     /* index table for triggers    */
int top_of_trigt = 0;           /* top of trigger index table    */
long max_id = MOBOBJ_ID_BASE;   /* for unique mob/obj id's       */

QuestData *qst_list = NULL;     /* quests */
int top_of_qstt = 0;            /* top of quest table */

RoomData *world;           /* array of rooms              */
int       top_of_world;    /* ref to top element of world */

IndexData *mob_index;      /* index table for mobile file  */
CharData  *mob_proto;      /* prototypes for mobs          */
int        top_of_mobt;    /* top of mobile index table    */

ObjData   *object_list;    /* global linked list of objs   */
IndexData *obj_index;      /* index table for object file  */
ObjData   *obj_proto;      /* prototypes for objs          */
int        top_of_objt;    /* top of object index table    */

ZoneData  *zone_table;     /* zone table                   */
int top_of_zone_table;     /* top element of zone tab      */

RandData   *rand_table;
int top_of_rand_table;

PlayerIndexElement *player_table; /* index to plr file        */
FILE *player_fl;                  /* file desc of player file */
int   top_of_p_table;             /* ref to top of table      */
int   top_of_p_file;              /* ref of size of p file    */
long  top_idnum;                  /* highest idnum in use     */

int no_mail;       /* mail disabled?            */
int mini_mud = 0;               /* mini-mud mode?		 */
int no_rent_check = 0;          /* skip rent check on boot?	 */
long boot_time;                 /* time of mud boot          */
int circle_restrict = 0;        /* level of game restriction	 */

char *credits = NULL;     /* game credits            */
char *news = NULL;        /* mud news                */
char *motd = NULL;        /* mort message of the day */
char *imotd = NULL;       /* imm message of the day  */
char *help = NULL;        /* help screen             */
char *ihelp = NULL;       /* god help screen         */
char *info = NULL;        /* info page               */
char *wizlist = NULL;     /* list of higher gods     */
char *immlist = NULL;     /* list of peon gods       */
char *background = NULL;  /* background story        */
char *handbook = NULL;		/* handbook for new immortals	 */
char *policies = NULL;    /* policies page           */

int top_of_helpt = 0;
struct help_index_element *help_table = NULL;

 time_t newsmod; /* Time news file was last modified. */
 time_t motdmod; /* Time motd file was last modified. */
 
TimeInfoData      time_info; /* the infomation about the time */
PlayerSpecialData dummy_mob; /* dummy spec area for mobs      */
ResetQType        reset_q;   /* queue of zones to be reset    */

LockerList       *lockers = NULL;

ClanFileEntry    *clan_list = NULL;     /* list of known clans */
int               clan_count = 0;       /* number of entries in clan_list */

Rename            renames[MAX_RENAMES];
int               top_of_rename_t = 0;

/* ============================================================================
Global variable initilisations.
============================================================================ */
void
initialize_global_variables(void)
{
top_of_world = 0;		/* ref to top element of world	 */

world = NULL;	/* array of rooms		 */
character_list = NULL;	/* global linked list of
						 * chars	 */
top_of_mobt = 0;		/* top of mobile index table	 */

object_list = NULL;	/* global linked list of objs	 */
top_of_objt = 0;		/* top of object index table	 */

top_of_zone_table = 0;	/* top element of zone tab	 */

top_of_rand_table = 0;

player_table = NULL;	/* index to plr file	 */
player_fl = NULL;		/* file desc of player file	 */
top_of_p_table = 0;		/* ref to top of table		 */
top_of_p_file = 0;		/* ref of size of p file	 */
top_idnum = 0;		/* highest idnum in use		 */

no_mail = 0;		/* mail disabled?		 */
boot_time = 0;		/* time of mud boot		 */
} /* initialize_global_variables */

/* local functions */
void setup_dir(FILE * fl, int room, int dir);
void discrete_load(FILE * fl, int mode);
void parse_trigger(FILE *fl, int virtual_nr);
void parse_room(FILE * fl, int virtual_nr);
void parse_mobile(FILE * mob_f, int nr);
char *parse_object(FILE * obj_f, int nr);
static int count_alias_records(FILE *fl);
static void get_one_line(FILE *fl, char *buf);
static int hsort(const void *a, const void *b);
char *parse_quest(FILE * qst_f, int nr);
void load_zones(FILE * fl, char *zonename);
void load_randoms(FILE *fl);
void assign_mobiles(void);
void assign_objects(void);
void assign_rooms(void);
void assign_the_shopkeepers(void);
void char_to_store(CharData * ch, struct char_file_u * st);
void store_to_char(struct char_file_u * st, CharData * ch);
int is_empty(int zone_nr);
void resetGoldAndXps(void);
int file_to_string(char *name, char *buf);
int file_to_string_alloc(char *name, char **buf);
void renum_world(void);
void renum_zone_table(void);
static void free_extra_descriptions(struct extra_descr_data *edesc);
static void load_default_config( void );
void clear_char(CharData * ch);

/* external functions */
void load_messages(void);
void mag_assign_spells(void);
void boot_social_messages(void);
void update_obj_file(void);	/* In objsave.c */
void sort_commands(void);
void load_banned(void);
void boot_the_shops(FILE * shop_f, char *filename, int rec_count);
extern void auction_reset();
void save_char_vars(struct char_data *ch);


/*************************************************************************
*  routines for booting the system                                       *
*********************************************************************** */

/* this is necessary for the autowiz system */
void reboot_wizlists(void)
{
  file_to_string_alloc(WIZLIST_FILE, &wizlist);
  file_to_string_alloc(IMMLIST_FILE, &immlist);
}

/* Wipe out all the loaded text files, for shutting down. */
void free_text_files(void)
{
  char **textfiles[] = {
	&wizlist, &immlist, &news, &credits, &motd, &imotd, &help, &ihelp, &info,
	&policies, &handbook, &background, NULL
  };
  int rf;

  for (rf = 0; textfiles[rf]; rf++)
    if (*textfiles[rf]) {
      free(*textfiles[rf]);
      *textfiles[rf] = NULL;
    }
}

/* Too bad it doesn't check the return values to let the user know about -1
 * values.  This will result in an 'Okay.' to a 'reload' command even when the
 * string was not replaced. To fix later. */
ACMD(do_reload)
{
    char arg[MAX_INPUT_LENGTH];

    one_argument(argument, arg);

    if (!str_cmp(arg, "all") || *arg == '*') {
        if (file_to_string_alloc(WIZLIST_FILE, &wizlist) < 0)
            sendChar(ch, "Cannot read wizlist\r\n");
        if (file_to_string_alloc(IMMLIST_FILE, &immlist) < 0)
            sendChar(ch, "Cannot read immlist\r\n");
        if (file_to_string_alloc(NEWS_FILE, &news) < 0)
            sendChar(ch, "Cannot read news\r\n");
        if (file_to_string_alloc(CREDITS_FILE, &credits) < 0)
            sendChar(ch, "Cannot read credits\r\n");
        if (file_to_string_alloc(MOTD_FILE, &motd) < 0)
            sendChar(ch, "Cannot read motd\r\n");
        if (file_to_string_alloc(IMOTD_FILE, &imotd) < 0)
            sendChar(ch, "Cannot read imotd\r\n");
        if (file_to_string_alloc(HELP_PAGE_FILE, &help) < 0)
            sendChar(ch, "Cannot read help front page\r\n");
        if (file_to_string_alloc(IHELP_PAGE_FILE, &ihelp) < 0)
            sendChar(ch, "Cannot read ihelp front page\r\n");
        if (file_to_string_alloc(INFO_FILE, &info) < 0)
            sendChar(ch, "Cannot read info file\r\n");
        if (file_to_string_alloc(POLICIES_FILE, &policies) < 0)
            sendChar(ch, "Cannot read policies\r\n");
        if (file_to_string_alloc(HANDBOOK_FILE, &handbook) < 0)
            sendChar(ch, "Cannot read handbook\r\n");
        if (file_to_string_alloc(BACKGROUND_FILE, &background) < 0)
            sendChar(ch, "Cannot read background\r\n");
        if (help_table) {
            free_help_table();
        index_boot(DB_BOOT_HLP);
        }
        load_messages();
    } else if (!str_cmp(arg, "wizlist")) {
        if (file_to_string_alloc(WIZLIST_FILE, &wizlist) < 0)
            sendChar(ch, "Cannot read wizlist\r\n");
    } else if (!str_cmp(arg, "immlist")) {
        if (file_to_string_alloc(IMMLIST_FILE, &immlist) < 0)
            sendChar(ch, "Cannot read immlist\r\n");
    } else if (!str_cmp(arg, "news")) {
        if (file_to_string_alloc(NEWS_FILE, &news) < 0)
            sendChar(ch, "Cannot read news\r\n");
    } else if (!str_cmp(arg, "credits")) {
        if (file_to_string_alloc(CREDITS_FILE, &credits) < 0)
            sendChar(ch, "Cannot read credits\r\n");
    } else if (!str_cmp(arg, "motd")) {
        if (file_to_string_alloc(MOTD_FILE, &motd) < 0)
            sendChar(ch, "Cannot read motd\r\n");
    } else if (!str_cmp(arg, "imotd")) {
        if (file_to_string_alloc(IMOTD_FILE, &imotd) < 0)
            sendChar(ch, "Cannot read imotd\r\n");
    } else if (!str_cmp(arg, "help")) {
        if (file_to_string_alloc(HELP_PAGE_FILE, &help) < 0)
            sendChar(ch, "Cannot read help front page\r\n");
    } else if (!str_cmp(arg, "ihelp")) {
        if (file_to_string_alloc(IHELP_PAGE_FILE, &ihelp) < 0)
            sendChar(ch, "Cannot read help front page\r\n");
    } else if (!str_cmp(arg, "info")) {
        if (file_to_string_alloc(INFO_FILE, &info) < 0)
            sendChar(ch, "Cannot read info\r\n");
    } else if (!str_cmp(arg, "policy")) {
        if (file_to_string_alloc(POLICIES_FILE, &policies) < 0)
            sendChar(ch, "Cannot read policy\r\n");
    } else if (!str_cmp(arg, "handbook")) {
        if (file_to_string_alloc(HANDBOOK_FILE, &handbook) < 0)
            sendChar(ch, "Cannot read handbook\r\n");
    } else if (!str_cmp(arg, "background")) {
        if (file_to_string_alloc(BACKGROUND_FILE, &background) < 0)
            sendChar(ch, "Cannot read background\r\n");
    } else if (!str_cmp(arg, "xhelp")) {
    if (help_table) {
      free_help_table();
    index_boot(DB_BOOT_HLP);
        }
    } else {
        sendChar(ch, "Unknown reload option.\r\n");
        return;
    }

    sendChar(ch, "%s", CONFIG_OK);
}

void clans_boot(void)
{
    FILE *f = fopen(CLANS_FILE, "rb");

    if (!f) return;

    /* get the count of clans -- and verify the file is a safe size */
    fseek(f, 0L, SEEK_END);
    clan_count = ftell(f) / sizeof(ClanFileEntry);
    if (clan_count * sizeof(ClanFileEntry) != ftell(f)) {
        mlog("Clan file is corrupted!");
        clan_count = 0;
        fclose(f);
        return;
    }

    /* bulk read the clans into *clan_list */
    rewind(f);
    clan_list = calloc(clan_count, sizeof(ClanFileEntry));
    fread(clan_list, sizeof(ClanFileEntry), clan_count, f);
    fclose(f);
}

void save_clans(void)
{
    FILE *f = fopen(CLANS_FILE, "wb");

    if (!f) {
        mlog("Can't open clan file for writing!");
        return;
    }

    /* blat out the clan list */
    fwrite(clan_list, sizeof(ClanFileEntry), clan_count, f);
    fclose(f);
    mlog("Clan list saved");
}

static void free_extra_descriptions(struct extra_descr_data *edesc)
{
  struct extra_descr_data *enext;

  for (; edesc; edesc = enext) {
    enext = edesc->next;

    free(edesc->keyword);
    free(edesc->description);
    free(edesc);
  }
}

/* Free the world, in a memory allocation sense. */
void destroy_db(void)
{
  ssize_t cnt, itr;
  struct char_data *chtmp;
  struct obj_data *objtmp;

    /* Active Mobiles & Players */
  while (character_list) {
    chtmp = character_list;
    character_list = character_list->next;
    if (chtmp->master)
      stop_follower(chtmp);
    free_char(chtmp);
  }

    /* Active Objects */
  while (object_list) {
    objtmp = object_list;
    object_list = object_list->next;
    free_obj(objtmp);
  }

    /* Rooms */
  for (cnt = 0; cnt <= top_of_world; cnt++) {
    if (world[cnt].name)
      free(world[cnt].name);
    if (world[cnt].description)
      free(world[cnt].description);
    free_extra_descriptions(world[cnt].ex_description);

    /* free any assigned scripts */
    if (SCRIPT(&world[cnt]))
    extract_script(&world[cnt], WLD_TRIGGER);
    /* free script proto list */
    free_proto_script(&world[cnt], WLD_TRIGGER);

    for (itr = 0; itr < NUM_OF_DIRS; itr++) {
      if (!world[cnt].dir_option[itr])
        continue;

      if (world[cnt].dir_option[itr]->general_description)
        free(world[cnt].dir_option[itr]->general_description);
      if (world[cnt].dir_option[itr]->keyword)
        free(world[cnt].dir_option[itr]->keyword);
      free(world[cnt].dir_option[itr]);
    }
  }
  free(world);
  top_of_world = 0;

  /* Objects */
  for (cnt = 0; cnt <= top_of_objt; cnt++) {
    if (obj_proto[cnt].name)
      free(obj_proto[cnt].name);
    if (obj_proto[cnt].description)
      free(obj_proto[cnt].description);
    if (obj_proto[cnt].short_description)
      free(obj_proto[cnt].short_description);
    if (obj_proto[cnt].action_description)
      free(obj_proto[cnt].action_description);
    free_extra_descriptions(obj_proto[cnt].ex_description);

    /* free script proto list */
    free_proto_script(&obj_proto[cnt], OBJ_TRIGGER);
  }
  free(obj_proto);
  free(obj_index);

  /* Mobiles */
  for (cnt = 0; cnt <= top_of_mobt; cnt++) {
    if (mob_proto[cnt].player.name)
      free(mob_proto[cnt].player.name);
    if (mob_proto[cnt].player.title)
      free(mob_proto[cnt].player.title);
    if (mob_proto[cnt].player.short_descr)
      free(mob_proto[cnt].player.short_descr);
    if (mob_proto[cnt].player.long_descr)
      free(mob_proto[cnt].player.long_descr);
    if (mob_proto[cnt].player.description)
      free(mob_proto[cnt].player.description);

    /* free script proto list */
    free_proto_script(&mob_proto[cnt], MOB_TRIGGER);

    while (mob_proto[cnt].affected)
      affect_remove(&mob_proto[cnt], mob_proto[cnt].affected, TRUE);
  }
  free(mob_proto);
  free(mob_index);

  /* Shops */
  destroy_shops();

  /* Quests */
  //destroy_quests();

  /* Zones */
#define THIS_CMD zone_table[cnt].cmd[itr]

  for (cnt = 0; cnt <= top_of_zone_table; cnt++) {
    if (zone_table[cnt].name)
      free(zone_table[cnt].name);
   // if (zone_table[cnt].builders)
   //    free(zone_table[cnt].builders);
    if (zone_table[cnt].cmd) {
      /* first see if any vars were defined in this zone */
      for (itr = 0;THIS_CMD.command != 'S';itr++)
        if (THIS_CMD.command == 'V') {
          if (THIS_CMD.sarg1)
            free(THIS_CMD.sarg1);
          if (THIS_CMD.sarg2)
            free(THIS_CMD.sarg2);
        }
      /* then free the command list */
      free(zone_table[cnt].cmd);
    }
  }
  free(zone_table);

#undef THIS_CMD

  /* zone table reset queue */
  if (reset_q.head) {
    struct reset_q_element *ftemp=reset_q.head, *temp;
    while (ftemp) {
      temp = ftemp->next;
      free(ftemp);
      ftemp = temp;
    }
  }

  /* Triggers */
  for (cnt=0; cnt < top_of_trigt; cnt++) {
    if (trig_index[cnt]->proto) {
      /* make sure to nuke the command list (memory leak) */
      /* free_trigger() doesn't free the command list */
      if (trig_index[cnt]->proto->cmdlist) {
        struct cmdlist_element *i, *j;
        i = trig_index[cnt]->proto->cmdlist;
        while (i) {
          j = i->next;
          if (i->cmd)
            free(i->cmd);
          free(i);
          i = j;
        }
      }
      free_trigger(trig_index[cnt]->proto);
    }
    free(trig_index[cnt]);
  }
  free(trig_index);
}
/* body of the booting system */
void boot_db(void)
{
  int i;
  extern int no_specials;

  mlog("Boot db -- BEGIN.");

  mlog("Initializing global variables.");
  initialize_global_variables();

  mlog("Resetting the game time:");
  reset_time();

  mlog("Reading news, credits, help, ihelp, bground, info & motds.");
  file_to_string_alloc(NEWS_FILE, &news);
  file_to_string_alloc(CREDITS_FILE, &credits);
  file_to_string_alloc(MOTD_FILE, &motd);
  file_to_string_alloc(IMOTD_FILE, &imotd);
  file_to_string_alloc(HELP_PAGE_FILE, &help);
  file_to_string_alloc(IHELP_PAGE_FILE, &ihelp);
  file_to_string_alloc(INFO_FILE, &info);
  file_to_string_alloc(WIZLIST_FILE, &wizlist);
  file_to_string_alloc(IMMLIST_FILE, &immlist);
  file_to_string_alloc(POLICIES_FILE, &policies);
  file_to_string_alloc(HANDBOOK_FILE, &handbook);
  file_to_string_alloc(BACKGROUND_FILE, &background);

    mlog("Loading help entries.");
  index_boot(DB_BOOT_HLP);

  mlog("Loading renamed objects table.");
  load_renames();

  mlog("Loading zone table.");
  index_boot(DB_BOOT_ZON);

  mlog("Loading triggers and generating index.");
  index_boot(DB_BOOT_TRG);

  mlog("Loading rooms.");
  index_boot(DB_BOOT_WLD);

  mlog("Renumbering rooms.");
  renum_world();

  mlog("Checking start rooms.");
  getStartRoom( NULL );

  mlog("Loading mobs and generating index.");
  index_boot(DB_BOOT_MOB);

  mlog("Loading objs and generating index.");
  index_boot(DB_BOOT_OBJ);

  mlog("Loading random item tables.");
  index_boot(DB_BOOT_RND);

  mlog("Renumbering zone table.");
  renum_zone_table();

  mlog("Loading clan details.");
  clans_boot();

  mlog("Generating player index.");
  build_player_index();

  mlog("Loading fight messages.");
  load_messages();

  mlog("Loading social messages.");
  boot_social_messages();

  //if (!no_specials) {
    mlog("Loading shops.");
    index_boot(DB_BOOT_SHP);
  //}
  mlog("Assigning function pointers:");

  if (!no_specials) {
    mlog("   Mobiles.");
    assign_mobiles();
    mlog("   Shopkeepers.");
    assign_the_shopkeepers();
    mlog("   Objects.");
    assign_objects();
    mlog("   Rooms.");
    assign_rooms();
    mlog("   Combat Specials.");
    assignCombatSpecials();
  }
  mlog("   Spells.");
  mag_assign_spells();
  mlog("   Recalculating mob xps and gold.");
  resetGoldAndXps();

  mlog("Auction system reset.");
  auction_reset();

  mlog("Sorting command list.");
  sort_commands();

  mlog("Loading quests.");
  index_boot(DB_BOOT_QST);

  mlog("Booting mail system.");
  if (!scan_file()) {
    mlog("    Mail boot failed -- Mail system disabled");
    no_mail = 1;
  }
  mlog("Reading banned site and invalid-name list.");
  load_banned();
  Read_Invalid_List();

  if( !no_rent_check )
  {
    mlog( "Deleting timed-out crash and rent files:" );
    update_obj_file();
    mlog( "Done." );
  }

  mudlog(NRM, LVL_IMMORT, TRUE, "Resetting the Zones." );
  for( i = 0; i <= top_of_zone_table; i++ )
  {
#if VERBOSE_BOOT
    mlog("Resetting %s (rooms %d-%d).",
            zone_table[i].name, (i ? (zone_table[i - 1].top + 1) : 0),
            zone_table[i].top);
#endif
    reset_zone(i);
  }

  reset_q.head = reset_q.tail = NULL;

  bootMortuary();
  bootLockers();

  boot_time = time(0);

  mlog("Loading up scatter/tally data.");
  scatter_init();

  if(0)
  {
  // Used when Status has been compromised.  REASSIGNS INCOMPLETE PLAYER STATUS.
  //  mlog("Resetting all chores for all players.");
  }

  mlog("Boot db -- DONE.");
}

void free_player_index(void)
{
  int tp;

  if (!player_table)
    return;

  for (tp = 0; tp <= top_of_p_table; tp++)
    if (player_table[tp].name)
      free(player_table[tp].name);

  free(player_table);
  player_table = NULL;
  top_of_p_table = 0;
}

/*
** generate index table for the player file
*/
void build_player_index(void)
{
  int nr = -1, i;
  long size, recs;
  struct char_file_u dummy;

  if (!(player_fl = fopen(SYS_PLRFILES, "r+b"))) {
    perror("Error opening playerfile");
    exit(1);
  }
  fseek(player_fl, 0L, SEEK_END);
  size = ftell(player_fl);
  rewind(player_fl);
  if (size % sizeof(struct char_file_u))
    fprintf(stderr, "WARNING:  PLAYERFILE IS PROBABLY CORRUPT!\n");
  recs = size / sizeof(struct char_file_u);
  if (recs) {
    mlog("   %ld players in database.", recs);
    CREATE(player_table, struct player_index_element, recs);
  } else {
    player_table = 0;
    top_of_p_file = top_of_p_table = -1;
    return;
  }

  for (; !feof(player_fl);) {
    fread(&dummy, sizeof(struct char_file_u), 1, player_fl);
    if (!feof(player_fl)) {	/* new record */
      /* update their chore count now */
      chore_update(&dummy);
      fseek(player_fl, -sizeof(struct char_file_u), SEEK_CUR);
      fwrite(&dummy, sizeof(struct char_file_u), 1, player_fl);
      nr++;
      CREATE(player_table[nr].name, char, strlen(dummy.name) + 1);
      for (i = 0;
	   (*(player_table[nr].name + i) = LOWER(*(dummy.name + i))); i++);
      player_table[nr].id = dummy.char_specials_saved.idnum;
      top_idnum = MAX(top_idnum, dummy.char_specials_saved.idnum);

      if( dummy.player_specials_saved.clan_id > 0 ){
          inc_clan_member_count( dummy.player_specials_saved.clan_id );
      }
    }
  }

  top_of_p_file = top_of_p_table = nr;
}

/* Thanks to Andrey (andrey@alex-ua.com) for this bit of code, although I did
 * add the 'goto' and changed some "while()" into "do { } while()". -gg */
static int count_alias_records(FILE *fl)
{
  char key[READ_SIZE], next_key[READ_SIZE];
  char line[READ_SIZE], *scan;
  int total_keywords = 0;

  /* get the first keyword line */
  get_one_line(fl, key);

  while (*key != '$') {
    /* skip the text */
    do {
      get_one_line(fl, line);
      if (feof(fl))
	goto ackeof;
    } while (*line != '#');

    /* now count keywords */
    scan = key;
    do {
      scan = one_word(scan, next_key);
      if (*next_key)
        ++total_keywords;
    } while (*next_key);

    /* get next keyword line (or $) */
    get_one_line(fl, key);

    if (feof(fl))
      goto ackeof;
  }

  return (total_keywords);

  /* No, they are not evil. -gg 6/24/98 */
ackeof:
  mlog("SYSERR: Unexpected end of help file.");
  exit(1);	/* Some day we hope to handle these things better... */
}

/* function to count how many hash-mark delimited records exist in a file */
int count_hash_records(FILE *fl)
{
  char buf[128];
  int count = 0;

  while (fgets(buf, 128, fl))
    if (*buf == '#')
      count++;

  return (count);
}

void index_boot(int mode)
{
  char *prefix;
  FILE *index, *db_file;
  int rec_count = 0;

  if( mode < DB_BOOT_WLD || mode > DB_BOOT_HLP )
  {
    mudlog(NRM, LVL_IMMORT, TRUE, "SYSERR: Unknown mode passed to index_boot." );
    exit(1);
  }
  else
  {
    static char *bootPrefix[] =
    {
      WLD_PREFIX, /* 0: DB_BOOT_WLD */
      MOB_PREFIX, /* 1: DB_BOOT_MOB */
      OBJ_PREFIX, /* 2: DB_BOOT_OBJ */
      ZON_PREFIX, /* 3: DB_BOOT_ZON */
      SHP_PREFIX, /* 4: DB_BOOT_SHP */
      RND_PREFIX, /* 5: DB_BOOT_RND */
      TRG_PREFIX, /* 6: DB_BOOT_TRG */
      QST_PREFIX, /* 7: DB_BOOT_QST */
      HLP_PREFIX
    };

    prefix = bootPrefix[ mode ];
  }

  sprintf( buf2, "%s/%s", prefix, (mini_mud ? MINDEX_FILE : INDEX_FILE));

  if( !(index = fopen(buf2, "r")))
  {
    sprintf(buf1, "Error opening index file '%s'", buf2);
    perror(buf1);
    exit(1);
  }

  /*
  ** first, count the number of records in the file so we can malloc
  */
  fscanf(index, "%s\n", buf1);
  while (*buf1 != '$')
  {
    sprintf(buf2, "%s/%s", prefix, buf1);
    if (!(db_file = fopen(buf2, "r")))
    {
      perror(buf2);
      exit(1);
    }
    else
    {
      if (mode == DB_BOOT_ZON || mode == DB_BOOT_RND)
	rec_count++;
      else if (mode == DB_BOOT_HLP)
	rec_count += count_alias_records(db_file);
      else
	rec_count += count_hash_records(db_file);
    }

    fclose(db_file);
    fscanf(index, "%s\n", buf1);
  }

  if (!rec_count) {
    mlog("SYSERR: boot error - 0 records counted");
    exit(1);
  }
  rec_count++;

  switch (mode) {
  case DB_BOOT_TRG:
    CREATE(trig_index, struct index_data *, rec_count);
    break;
  case DB_BOOT_WLD:
    CREATE(world, RoomData, rec_count);
    break;
  case DB_BOOT_MOB:
    CREATE(mob_proto, CharData, rec_count);
    CREATE(mob_index, struct index_data, rec_count);
    break;
  case DB_BOOT_OBJ:
    CREATE(obj_proto, ObjData, rec_count);
    CREATE(obj_index, struct index_data, rec_count);
    break;
  case DB_BOOT_QST:
    CREATE(qst_list, QuestData, rec_count);
    break;
  case DB_BOOT_ZON:
    CREATE(zone_table, struct zone_data, rec_count);
    break;
  case DB_BOOT_RND:
    CREATE(rand_table, struct rand_data, rec_count);
    break;
  case DB_BOOT_HLP:
    CREATE(help_table, struct help_index_element, rec_count);
    break;
  }

  rewind(index);
  fscanf(index, "%s\n", buf1);
  while (*buf1 != '$') {
    sprintf(buf2, "%s/%s", prefix, buf1);
    if (!(db_file = fopen(buf2, "r"))) {
      perror(buf2);
      exit(1);
    }
    switch (mode) {
    case DB_BOOT_TRG:
    case DB_BOOT_WLD:
    case DB_BOOT_OBJ:
    case DB_BOOT_MOB:
    case DB_BOOT_QST:
      discrete_load(db_file, mode);
      break;
    case DB_BOOT_ZON:
      load_zones(db_file, buf2);
      break;
    case DB_BOOT_HLP:
      load_help(db_file, buf2);
      break;
    case DB_BOOT_SHP:
      boot_the_shops(db_file, buf2, rec_count);
      break;
    case DB_BOOT_RND:
      load_randoms(db_file);
      break;
    }

    fclose(db_file);
    fscanf(index, "%s\n", buf1);
  }
  fclose(index);

  /* Sort the help index. */
  if (mode == DB_BOOT_HLP) {
      qsort(help_table, top_of_helpt, sizeof(struct help_index_element), hsort);
      top_of_helpt--;
  }
}


void
discrete_load(FILE * fl, int mode)
{
  int nr = -1, last = 0;
  char line[256];

  char *modes[] = {"world", "mob", "obj", "ZON", "SHP", "rnd", "trg", "qst" "hlp"};

  for (;;) {
    /*
     * we have to do special processing with the obj files because they have
     * no end-of-record marker :(
     */
    if ((mode != DB_BOOT_OBJ && mode != DB_BOOT_QST) || nr < 0)
      if (!get_line(fl, line)) {
	fprintf(stderr, "Format error after %s #%d\n", modes[mode], nr);
	exit(1);
      }
    if (*line == '$')
      return;

    if (*line == '#') {
      last = nr;
      if (sscanf(line, "#%d", &nr) != 1) {
	fprintf(stderr, "Format error after %s #%d\n", modes[mode], last);
	exit(1);
      }
      if (nr >= 99999)
	return;
      else
	switch (mode) {
        case DB_BOOT_TRG:
          parse_trigger(fl, nr);
          break;
	case DB_BOOT_WLD:
	  parse_room(fl, nr);
	  break;
	case DB_BOOT_MOB:
	  parse_mobile(fl, nr);
	  break;
	case DB_BOOT_OBJ:
	  strcpy(line, parse_object(fl, nr));
	  break;
        case DB_BOOT_QST:
          strcpy(line, parse_quest(fl, nr));
          break;
	}
    } else {
      fprintf(stderr, "Format error in %s file near %s #%d\n",
	      modes[mode], modes[mode], nr);
      fprintf(stderr, "Offending line: '%s'\n", line);
      exit(1);
    }
  }
}

u_int
asciiflag_conv( char *flag )
{
  u_int flags = 0;
  int is_number = 1;
  register char *p;

  for (p = flag; *p; p++) {
    if (islower(*p))
      flags |= 1 << (*p - 'a');
    else if (isupper(*p))
      flags |= 1 << (26 + (*p - 'A'));

    if (!isdigit(*p) && (*p != '-' || p != flag))
      is_number = 0;
  }

  if (is_number)
    flags = atol(flag);

  return flags;
}

char fread_letter(FILE *fp)
{
  char c;
  do {
    c = getc(fp);
  } while (isspace(c));
  return c;
}

void parse_room( FILE *fl, int virtual_nr)
{
  static int zone = 0, room_nr = 0;
  int t[10], i;
  char line[256], flags[128], flags2[128], flags3[128], flags4[128];
  struct extra_descr_data *new_descr;
  char letter;

  sprintf(buf2, "room #%d", virtual_nr);

  if (virtual_nr <= (zone ? zone_table[zone - 1].top : -1)) {
    fprintf(stderr, "Room #%d is below zone %d.\n", virtual_nr, zone);
    exit(1);
  }
  while (virtual_nr > zone_table[zone].top)
    if (++zone > top_of_zone_table) {
      fprintf(stderr, "Room %d is outside of any zone.\n", virtual_nr);
      exit(1);
    }
  world[room_nr].zone = zone;
  world[room_nr].number = virtual_nr;
  world[room_nr].name = fread_string(fl, buf2);
  world[room_nr].description = fread_string(fl, buf2);

  if (!get_line(fl, line) ||
      sscanf(line, " %d %s %s %s %s %d ", t, flags, flags2, flags3,
             flags4, t + 2) != 6) {
      fprintf(stderr, "Format error in room #%d\n", virtual_nr);
      exit(1);
  }

  /* t[0] is the zone number; ignored with the zone-file system */

  world[room_nr].room_flags[0] = asciiflag_conv(flags);
  world[room_nr].room_flags[1] = asciiflag_conv(flags2);
  world[room_nr].room_flags[2] = asciiflag_conv(flags3);
  world[room_nr].room_flags[3] = asciiflag_conv(flags4);

  rCheckRoomFlags(&world[room_nr]);
  world[room_nr].sector_type = t[2];
  world[room_nr].clan_id     = -1;


  if( IS_SET_AR( world[room_nr].room_flags, ROOM_CLAN ) ){
      sscanf(line, " %*d %*s %*s %*s %*s %*d %d %d ", &t[3], &t[4]);
      world[room_nr].clan_id = t[3];
      world[room_nr].clan_recept_sz = t[4];
  }

  world[room_nr].contents = NULL;
  world[room_nr].func     = NULL;
  world[room_nr].light    = 0;	/* Zero light sources */
  world[room_nr].people   = NULL;

  for (i = 0; i < NUM_OF_DIRS; i++)
    world[room_nr].dir_option[i] = NULL;

  world[room_nr].ex_description = NULL;

  sprintf(buf, "Format error in room #%d (expecting D/E/S)", virtual_nr);

  for (;;) {
    if (!get_line(fl, line)) {
      fprintf(stderr, "%s\n", buf);
      exit(1);
    }
    switch (*line) {
    case 'D':
      setup_dir(fl, room_nr, atoi(line + 1));
      break;
    case 'E':
      CREATE(new_descr, struct extra_descr_data, 1);
      new_descr->keyword = fread_string(fl, buf2);
      new_descr->description = fread_string(fl, buf2);
      new_descr->next = world[room_nr].ex_description;
      world[room_nr].ex_description = new_descr;
      break;
    case 'S':			/* end of room */
      /* DG triggers -- script is defined after the end of the room */
      letter = fread_letter(fl);
      ungetc(letter, fl);
      while (letter=='Z') {
        dg_read_trigger(fl, &world[room_nr], WLD_TRIGGER);
        letter = fread_letter(fl);
        ungetc(letter, fl);
      }
      top_of_world = room_nr++;
      return;
      break;
    default:
      fprintf(stderr, "%s\n", buf);
      exit(1);
      break;
    }
  }
}



/*
** read direction data
*/
void
setup_dir(FILE * fl, int room, int dir)
{
  int  t[5];
  char line[256];

  sprintf( buf2, "room #%d, direction D%d", world[room].number, dir );

  CREATE( world[room].dir_option[dir], struct room_direction_data, 1 );
  world[room].dir_option[dir]->general_description = fread_string( fl, buf2 );
  world[room].dir_option[dir]->keyword             = fread_string( fl, buf2 );

  world[room].dir_option[dir]->exit_info = 0;
  world[room].dir_option[dir]->to_room   = -1;
  world[room].dir_option[dir]->key       = -1;

  if( !get_line( fl, line ))
  {
    mudlog(NRM, LVL_IMMORT, TRUE, "Format error, %s\n", buf2 );
    return;
  }

  if( sscanf( line, " %d %d %d", t, t+1, t+2 ) != 3 )
  {
    mudlog(NRM, LVL_IMMORT, TRUE, "Format error, %s\n", buf2 );
    return;
  }

  if( t[0] == 1 )
    world[room].dir_option[dir]->exit_info = EX_ISDOOR;
  else if( t[0] == 2 )
    world[room].dir_option[dir]->exit_info = EX_ISDOOR | EX_PICKPROOF;
  else
    world[room].dir_option[dir]->exit_info = 0;

  world[room].dir_option[dir]->to_room = t[2];
  world[room].dir_option[dir]->key     = t[1];

  if( t[2] == 0 )
    mudlog(NRM, LVL_IMMORT, TRUE, "(db.c) %s leads to the void.", buf2);
}


/*
** Let's make the mud a little more spread out but giving different
** range level different rooms to start in.
*/
int
getStartRoom( CharData *ch )
{
  typedef struct {
    int rNum;
    int vNum;
    int cLev;
  } StartRooms;

  static StartRooms roomList[] = {
//    {  0, 1280, 15 }, OLD ROOM
    { 0, 3003, 15 },
//    {  0, 1281, 50 }, OLD ROOM
    { 0, 18102, 50 },
    {  0, 1204, 60 },
    { -1, 5000, -1 }
  };

  StartRooms *rms = roomList;

  if( ch != NULL && PLR_FLAGGED( ch, PLR_FROZEN) )
    return( real_room( 1202 ));

  while( rms->cLev > 0 )
  {
    if( ch == NULL )
      rms->rNum = real_room( rms->vNum );
    else if( GET_LEVEL(ch) <= rms->cLev )
    {
#if 0
      mudlog(NRM, LVL_IMMORT, TRUE, "Rooms for %s level %d rnum %d vnum %d",
        GET_NAME(ch), GET_LEVEL(ch), rms->rNum, rms->vNum );
#endif
     return(rms->rNum);
    }
    rms++;
  }
  /*
  ** If we get here there is a problem so just return
  ** the default room number to drop the char in.
  */
  return( real_room( 1205 ));
}

/*
** resolve all vnums into rnums in the world
*/
void
renum_world(void)
{
  register int room, door;

  for (room = 0; room <= top_of_world; room++)
    for (door = 0; door < NUM_OF_DIRS; door++)
      if (world[room].dir_option[door])
	if (world[room].dir_option[door]->to_room != NOWHERE)
	  world[room].dir_option[door]->to_room =
	    real_room(world[room].dir_option[door]->to_room);
}


#define ZCMD zone_table[zone].cmd[cmd_no]

/* resulve vnums into rnums in the zone reset tables */
void renum_zone_table(void)
{
  int zone, cmd_no, a, b, bug;

  for (zone = 0; zone <= top_of_zone_table; zone++)
    for (cmd_no = 0; ZCMD.command != 'S'; cmd_no++) {
      a = b = 0;
      switch (ZCMD.command) {
      case 'M':
	bug = ZCMD.arg1;
	a = ZCMD.arg1 = real_mobile(ZCMD.arg1);
	if (a < 0)
	    mudlog(NRM, LVL_IMMORT, TRUE, "(INVALID): M command: Zone %d, arg1 %d", zone, bug);
	bug = ZCMD.arg3;
	b = ZCMD.arg3 = real_room(ZCMD.arg3);
	if (b < 0)
	    mudlog(NRM, LVL_IMMORT, TRUE, "(INVALID): M command: Zone %d, arg3 %d", zone, bug);
	break;
      case 'O':
	bug = ZCMD.arg1;
	a = ZCMD.arg1 = real_object(ZCMD.arg1);
	if (a < 0)
	    mudlog(NRM, LVL_IMMORT, TRUE, "(INVALID): O command: Zone %d, arg1 %d", zone, bug);
	bug = ZCMD.arg3;
	if (ZCMD.arg3 != NOWHERE)
	  b = ZCMD.arg3 = real_room(ZCMD.arg3);
	if (b < 0)
	    mudlog(NRM, LVL_IMMORT, TRUE, "(INVALID): O command: Zone %d, arg3 %d", zone, bug);
	break;
      case 'G':
	bug = ZCMD.arg1;
	a = ZCMD.arg1 = real_object(ZCMD.arg1);
	if (a < 0)
		mudlog(NRM, LVL_IMMORT, TRUE, "(INVALID): G command arg1 = %d", bug);
	break;
      case 'E':
	bug = ZCMD.arg1;
	a = ZCMD.arg1 = real_object(ZCMD.arg1);
	if (a < 0)
		mudlog(NRM, LVL_IMMORT, TRUE, "(INVALID): E command arg1 = %d", bug);
	break;
      case 'P':
	bug = ZCMD.arg1;
	a = ZCMD.arg1 = real_object(ZCMD.arg1);
	if (a < 0)
		mudlog(NRM, LVL_IMMORT, TRUE, "(INVALID): P command arg1 = %d", bug);
	bug = ZCMD.arg3;
	b = ZCMD.arg3 = real_object(ZCMD.arg3);
	if (b < 0)
		mudlog(NRM, LVL_IMMORT, TRUE, "(INVALID): P command arg3 = %d", bug);
	break;
      case 'D':
	bug = ZCMD.arg1;
	a = ZCMD.arg1 = real_room(ZCMD.arg1);
	if (a < 0)
		mudlog(NRM, LVL_IMMORT, TRUE, "(INVALID): D command arg1 = %d", bug);
	break;
      case 'R':
      if (ZCMD.arg2 != NOWHERE)
      	b = ZCMD.arg2 = real_room(ZCMD.arg2);
      break;
      case 'Q':
      case 'N':
		break;
      case 'T':
        b = ZCMD.arg2 = real_object(ZCMD.arg2);
        break;
      case 'Z':
        b = real_trigger(ZCMD.arg2); /* leave this in for validation */
        break;
      case 'V': /* trigger variable assignment */
        if (ZCMD.arg1 == WLD_TRIGGER)
          b = ZCMD.arg2 = real_room(ZCMD.arg2);
        break;
      }
      if (a < 0 || b < 0) {
    if (!mini_mud)
        fprintf(stderr, "Invalid vnum in zone reset cmd: zone #%d, cmd %d .. command disabled.\n",
	  zone_table[zone].number, cmd_no + 1);
	ZCMD.command = '*';
      }
    }
}

void
parse_mobile( FILE * mob_f, int nr )
{
    static int i = 0;
    int j, t[10];
    char line[256], *tmpptr; /* letter was here for reading 'S' and 'E' */
    char f1[128], f2[128], f3[128], f4[128], f5[128], f6[128], f7[128], f8[128];
    char *mob_race;
    char mob_race_array[80], first_arg[80], let_array[80];
    char class, race;
    int subrace;
    char letter;

    /* initialise variables */
    f1[0] = f2[0] = f3[0] = f4[0] = f5[0] = f6[0] = f7[0] = f8[0] = '\0';

    mob_index[i].combatSpec = NULL;
    mob_index[i].virtual    = nr;
    mob_index[i].number     = 0;
    mob_index[i].func       = NULL;

    clear_char(mob_proto + i);

    mob_proto[i].player_specials = &dummy_mob;
    sprintf(buf2, "mob vnum %d", nr);

    mob_race = fread_string (mob_f, buf2);

    mob_proto[i].player.name = fread_string(mob_f, buf2);
    tmpptr = mob_proto[i].player.short_descr = fread_string(mob_f, buf2);
    if( tmpptr && *tmpptr )
        if( !str_cmp( fname(tmpptr), "a"  )
         || !str_cmp( fname(tmpptr), "an" )
         || !str_cmp( fname(tmpptr), "the"))
            *tmpptr = LOWER(*tmpptr);

    mob_proto[i].player.long_descr = fread_string(mob_f, buf2);
    mob_proto[i].player.description = fread_string(mob_f, buf2);
    mob_proto[i].player.title = NULL;

    get_line(mob_f, line);
    sscanf(line, "%s %s %s %s %s %s %s %s %d", f1, f2, f3, f4, f5, f6, f7, f8, t + 2 );  /* would have read 'S'/'E' here. */
    MOB_FLAGS(mob_proto + i)[0] = asciiflag_conv(f1);
    MOB_FLAGS(mob_proto + i)[1] = asciiflag_conv(f2);
    MOB_FLAGS(mob_proto + i)[2] = asciiflag_conv(f3);
    MOB_FLAGS(mob_proto + i)[3] = asciiflag_conv(f4);

    SET_BIT_AR(MOB_FLAGS(mob_proto + i), MOB_ISNPC);

    AFF_FLAGS(mob_proto + i)[0] = asciiflag_conv(f5);
    AFF_FLAGS(mob_proto + i)[1] = asciiflag_conv(f6);
    AFF_FLAGS(mob_proto + i)[2] = asciiflag_conv(f7);
    AFF_FLAGS(mob_proto + i)[3] = asciiflag_conv(f8);

    /*
    ** Poke around in here to see what's set
    */
    GET_ALIGNMENT(mob_proto + i) = t[2];

    mob_proto[i].real_abils.str   = 11;
    mob_proto[i].real_abils.intel = 11;
    mob_proto[i].real_abils.wis   = 11;
    mob_proto[i].real_abils.dex   = 11;
    mob_proto[i].real_abils.con   = 11;
    /*
    ** The following parse statement is SO ugly someone
    ** should be slapped for it.
    */
    get_line(mob_f, line);
    if( sscanf(line, " %d %d %d %dd%d+%d %dd%d+%d ",
        t, t + 1, t + 2, t + 3, t + 4, t + 5, t + 6, t + 7, t + 8) != 9) {
              fprintf(stderr, "Format error in mob #%d, first line after S flag\n"
	              "...expecting line of form '# # # #d#+# #d#+#'\n", nr);
              exit(1);
    }
    GET_LEVEL(mob_proto + i)     = t[0];
    mob_proto[i].points.max_hit  = 0;
    mob_proto[i].points.max_mana = 10;
    mob_proto[i].points.max_move = 50;

    mob_proto[i].points.hitroll  = 20 - t[1];
    mob_proto[i].points.armor    = 10 * t[2];
    mob_proto[i].points.hit      = t[3];
    mob_proto[i].points.mana     = t[4];
    mob_proto[i].points.move     = t[5];
    if (t[6] <= 0)
    {
      t[6] = 1;
      mudlog(NRM, LVL_IMMORT, TRUE, "0 ERROR: mob #%d has zero damage dice", nr);
    }
    mob_proto[i].mob_specials.damnodice   = t[6];
    if (t[7] <= 0) { /* This was annoying for a while. Vex. */
	t[7] = 1;
	mudlog(NRM, LVL_IMMORT, TRUE, "0 ERROR: mob #%d has zero damage size", nr);
    }
    mob_proto[i].mob_specials.damsizedice = t[7];
    mob_proto[i].points.damroll           = t[8];

    get_line(mob_f, line);
    sscanf(line, " %d %d ", t, t + 1);

	GET_MOB_SKILL_SUC(mob_proto + i) = t[0];
    GET_MOB_SPELL_DAM(mob_proto + i) = t[1];

    /* Set up mob attack types. Vex. */
    get_line(mob_f, line);
    /* Initialize. */
    mob_proto[i].mob_specials.attack_type = 0;
    mob_proto[i].mob_specials.attack_type2 = -1;
    mob_proto[i].mob_specials.attack_type3 = -1;
    /* Error report. */
    if (sscanf(line, " %d %d %d %d %d %d", t, t + 1, t + 2, t + 3, t + 4, t + 5) < 3)
	mudlog(NRM, LVL_IMMORT, TRUE, "(PARSE): Not enough entries for mob %d. Expected at least <pos> <pos> <sex>.", nr);
    if (sscanf(line, " %d %d %d %d %d %d", t, t + 1, t + 2, t + 3, t + 4, t + 5) > 6)
	mudlog(NRM, LVL_IMMORT, TRUE, "(PARSE): Too many entries following sex in mob %d", nr);
    /* Assign the types, if they are there. */
    if (sscanf(line, " %d %d %d %d %d %d", t, t + 1, t + 2, t + 3, t + 4, t + 5) >= 4)
      mob_proto[i].mob_specials.attack_type = t[3];
    if (sscanf(line, " %d %d %d %d %d %d", t, t + 1, t + 2, t + 3, t + 4, t + 5) >= 5)
      mob_proto[i].mob_specials.attack_type2 = t[4];
    if (sscanf(line, " %d %d %d %d %d %d", t, t + 1, t + 2, t + 3, t + 4, t + 5) == 6)
      mob_proto[i].mob_specials.attack_type3 = t[5];

    /* Remember load_pos  for later use - Vex */
    if (t[0] > POS_STANDING) { /* This can cause problems, fix it and complain. */
	mudlog(NRM, LVL_IMMORT, TRUE, "(PARSE): Invalid load position(%d) for mob %d.", t[0], nr);
	t[0] = POS_STANDING;
    }

    // THE FOLLOWING THREE '+1' LINES SHOULD BE MANUALLY EDITED INTO THE MOB
    // FILES AND THEN REMOVED FROM THE CODE!!!!! -ARBACES
    mob_proto[i].char_specials.position = t[0];
    mob_proto[i].mob_specials.load_pos = t[0];
    //if(t[0] > POS_SLEEPING) {
    //    mob_proto[i].char_specials.position = t[0]+1;
    //    mob_proto[i].mob_specials.load_pos = t[0]+1;
    //}

    if (t[1] > POS_STANDING) { /* This can cause problems, fix it and complain. */
	mudlog(NRM, LVL_IMMORT, TRUE, "(PARSE): Invalid load position(%d) for mob %d.", t[1], nr);
	t[1] = POS_STANDING;
    }
    mob_proto[i].mob_specials.default_pos = t[1];
    //if(t[1] > POS_SLEEPING)
    //    mob_proto[i].mob_specials.default_pos = t[1]+1;

    if (t[2] > SEX_FEMALE) { /* This can cause problems, fix it and complain. */
	mudlog(NRM, LVL_IMMORT, TRUE, "(PARSE): Invalid sex(%d) for mob %d.", t[2], nr);
	t[2] = SEX_MALE;
    }
    mob_proto[i].player.sex = t[2];

    mob_proto[i].player.class  = 0;
    mob_proto[i].player.weight = 200;
    mob_proto[i].player.height = 198;

    for (j = 0; j < 3; j++)
      GET_COND(mob_proto + i, j) = -1;

    /*
     * these are now save applies; base save numbers for MOBs are now from
     * the warrior save table.
     */
    for (j = 0; j < 5; j++)
      GET_SAVE(mob_proto + i, j) = 0;

    for (j = 0; j < NUM_WEARS; j++) mob_proto[i].equipment[j] = NULL;
    /*  Liam
    **  Setup the class and race here
    **  Vex: made these checks a bit brighter.
    */
    strcpy(mob_race_array, mob_race);
    if (*mob_race_array) {
	case_chop(mob_race_array, first_arg, mob_race_array);
	strxfrm(let_array, first_arg, strlen(first_arg));
	for (j = 0; j < strlen(first_arg); j++)
	    if (isalpha(let_array[j])) break; /* found race. */
	if (j >= strlen(first_arg)) { /* No race found. */
	    mudlog(NRM, LVL_IMMORT, TRUE, "(DB.C) Invalid race entry %s for mob[%d]", first_arg, nr);
	    race = 'H'; /* default to human */
	}
	else
            race = let_array[j];
    }
    else {
	mudlog(NRM, LVL_IMMORT, TRUE, "(DB.C) No race was entered for mob[%d]", nr);
	race = 'H'; /* default to human */
    }
    if (*mob_race_array) {
	case_chop(mob_race_array, first_arg, mob_race_array);
	strxfrm(let_array, first_arg, strlen(first_arg));
	for (j = 0; j < strlen(first_arg); j++)
	    if (isalpha(let_array[j])) break; /* found class. */
	if (j >= strlen(first_arg)) { /* No class found. */
	    mudlog(NRM, LVL_IMMORT, TRUE, "(DB.C) Invalid class entry %s for mob[%d]", first_arg, nr);
	    class = 'W'; /* default to warrior */
	}
	else
            class = let_array[j];
    }
    else {
	mudlog(NRM, LVL_IMMORT, TRUE, "(DB.C) No class was entered for mob[%d]", nr);
	class = 'W'; /* default to warrior */
    }
    if (*mob_race_array) { /* We have either a sub_race or a ~ */
	case_chop(mob_race_array, first_arg, mob_race_array);
	if (is_integer(first_arg))
	    subrace = atoi(first_arg);
	else
	    subrace = 0;
    }
    else { /* We did'nt get a sub_race or a tilde, but it don't really matter. */
	subrace = 0; /* default to no sub race */
    }

    GET_CLASS(mob_proto + i ) = parse_class(class);
    SET_RACE(mob_proto + i, all_parse_race(race)); /* PC and NPC races are stored in different files.. */
    SET_SUBRACE(mob_proto + i, subrace);
    free(mob_race);

    /* not sure if this is the right place for this.  triggers at end
     * of mob def, anyway. */
  /* DG triggers -- script info follows mob S/E section */
  letter = fread_letter(mob_f);
  ungetc(letter, mob_f);
  while (letter=='Z') {
    dg_read_trigger(mob_f, &mob_proto[i], MOB_TRIGGER);
    letter = fread_letter(mob_f);
    ungetc(letter, mob_f);
  }

    mob_proto[i].aff_abils = mob_proto[i].real_abils;

    mob_proto[i].nr = i;
    mob_proto[i].desc = NULL;

    roll_real_abils(&mob_proto[i]);

	top_of_mobt = i++;
}

/* redo all mob proto-type xps and gps */
void
resetGoldAndXps(void)
{
    int i;
    for (i = 0; i < top_of_mobt; i++)
    {
        GET_GOLD(mob_proto + i) = 99999999;
        calculateXpAndGp(mob_proto+i);
    }
}

/*
** read all objects from obj file; generate index and prototypes
*/
char *
parse_object( FILE * obj_f, int nr )
{
  static unsigned int i = 0;
  static char line[256];
  int t[10], j, retval;
  char *tmpptr;
  char f1[128], f2[128], f3[128], f4[128], f5[128], f6[128], f7[128], f8[128];
  struct extra_descr_data *new_descr;
  int wpn_avg;

  clear_object( obj_proto + i );

  obj_index[i].virtual     = nr;
  obj_index[i].number      = 0;
  obj_index[i].func        = NULL;
  obj_index[i].combatSpec  = NULL;
  obj_proto[i].in_room     = NOWHERE;
  obj_proto[i].item_number = i;
  obj_proto[i].traplevel = 0;

  sprintf( buf2, "object #%d", nr );

  /*
  ** string data
  */
  if(( obj_proto[i].name = fread_string( obj_f, buf2 )) == NULL )
  {
    mudlog(NRM, LVL_IMMORT, TRUE, "Null obj name or format error at or near %s\n", buf2 );
    return( "#99999" );
  }

  tmpptr = obj_proto[i].short_description = fread_string(obj_f, buf2);
  if (*tmpptr)
    if (!str_cmp(fname(tmpptr), "a") || !str_cmp(fname(tmpptr), "an") ||
	!str_cmp(fname(tmpptr), "the"))
      *tmpptr = LOWER(*tmpptr);

  tmpptr = obj_proto[i].description = fread_string(obj_f, buf2);
  if (tmpptr && *tmpptr)
    *tmpptr = UPPER(*tmpptr);
  obj_proto[i].action_description = fread_string(obj_f, buf2);

  /* *** numeric data *** */
  if (!get_line(obj_f, line) ||
      (retval = sscanf(line, " %d %s %s %s %s %s %s %s %s",
       		       t, f1, f2, f3, f4, f5, f6, f7, f8)) != 9) {
      fprintf(stderr, "Format error in first numeric line (expecting 9 args, got %d), %s\n", retval, buf2);
    exit(1);
  }

  obj_proto[i].obj_flags.type_flag   = t[0];
  obj_proto[i].obj_flags.extra_flags[0] = asciiflag_conv(f1);
  obj_proto[i].obj_flags.extra_flags[1] = asciiflag_conv(f2);
  obj_proto[i].obj_flags.extra_flags[2] = asciiflag_conv(f3);
  obj_proto[i].obj_flags.extra_flags[3] = asciiflag_conv(f4);
  obj_proto[i].obj_flags.wear_flags[0] = asciiflag_conv(f5);
  obj_proto[i].obj_flags.wear_flags[1] = asciiflag_conv(f6);
  obj_proto[i].obj_flags.wear_flags[2] = asciiflag_conv(f7);
  obj_proto[i].obj_flags.wear_flags[3] = asciiflag_conv(f8);


  /*
  ** ShouLin Equipment Restrictions
  */
  if( objAntiShouLin( &obj_proto[i] ) )
  {
   SET_BIT_AR(obj_proto[i].obj_flags.extra_flags, ITEM_ANTI_SHOU_LIN);
  }

  if (!get_line(obj_f, line) ||
      (retval = sscanf(line, "%d %d %d %d", t, t + 1, t + 2, t + 3)) != 4) {
    fprintf(stderr, "Format error in second numeric line(expecting 4 args, got %d), %s\n", retval, buf2);
    exit(1);
  }

   /* Limit item values. */
  t[0] = objValueLimit(obj_proto[i].obj_flags.type_flag, 0, t[0]);
  t[1] = objValueLimit(obj_proto[i].obj_flags.type_flag, 1, t[1]);
  if (!((obj_proto[i].obj_flags.type_flag == ITEM_SCROLL) && (t[1] == SPELL_WORD_OF_RECALL)) ) {
      t[2] = objValueLimit(obj_proto[i].obj_flags.type_flag, 2, t[2]);
      t[3] = objValueLimit(obj_proto[i].obj_flags.type_flag, 3, t[3]);
  }
  obj_proto[i].obj_flags.value[0] = t[0];
  obj_proto[i].obj_flags.value[1] = t[1];
  obj_proto[i].obj_flags.value[2] = t[2];
  obj_proto[i].obj_flags.value[3] = t[3];

  /* Check for boosted weapons. Vex. */
  if (obj_proto[i].obj_flags.type_flag == ITEM_WEAPON) {
	wpn_avg = t[1] * ((t[2] + 1)/2);
	if (wpn_avg <= 0)
	    mudlog(NRM, LVL_IMMORT, TRUE, "OBJ[%d] has invalid weapon average %d", nr, wpn_avg);
	else if (wpn_avg >= 35)
	    mudlog(NRM, LVL_IMMORT, TRUE, "OBJ[%d] has weapon average %d, bit high?", nr, wpn_avg);
  }

  /* Inspect projectiles. */
  if (obj_proto[i].obj_flags.type_flag == ITEM_MISSILE) {
	wpn_avg = t[1] * ((t[2] + 1)/2);
	if (wpn_avg <= 0)
	    mudlog(NRM, LVL_IMMORT, TRUE, "Arrow OBJ[%d] has invalid weapon average %d", nr, wpn_avg);
        else if (wpn_avg >= 10) //FIXME: Needs proper value set, sick of syslog messages. -Xiuh
	    mudlog(NRM, LVL_IMMORT, TRUE, "Arrow [%d] has average %d.", nr, wpn_avg);
  }

  if (!get_line(obj_f, line) || sscanf(line, "%d %d %d", t, t + 1, t + 2) != 3) {
    fprintf(stderr, "Format error in third numeric line, %s\n", buf2);
    exit(1);
  }
  obj_proto[i].obj_flags.weight = t[0];
  obj_proto[i].obj_flags.cost = t[1];
  obj_proto[i].obj_flags.cost_per_day = abs(t[2]);
  /* *** extra descriptions and affect fields *** */

  for (j = 0; j < MAX_OBJ_AFFECT; j++) {
    obj_proto[i].affected[j].location = APPLY_NONE;
    obj_proto[i].affected[j].modifier = 0;
  }

  strcat(buf2, ", after numeric constants (expecting E/A/#xxx)");
  j = 0;

  for(;;)
  {
    if( !get_line( obj_f, line ))
    {
      mudlog(NRM, LVL_IMMORT, TRUE, "IGN1: Format error in %s", buf2 );
      return( OBJEOF );
    }

    switch( *line )
    {
    case 'E': /* extra description */
      CREATE(new_descr, struct extra_descr_data, 1);
      new_descr->keyword = fread_string(obj_f, buf2);
      new_descr->description = fread_string(obj_f, buf2);
      new_descr->next = obj_proto[i].ex_description;
      obj_proto[i].ex_description = new_descr;
      break;

    case 'B': /* bitvectors */
      if( !get_line(obj_f, line) ||
        ( retval = sscanf(line, "%s %s %s %s",
		          f1, f2, f3, f4) != 4))
      {
//128 PROBLEM    	mudlog(NRM, LVL_IMMORT, TRUE, "IGN2:Format error after B(expecting 1 args, got %d), %s", retval, buf2 );
// 128 PROBLEM        return( OBJEOF );
      }
  obj_proto[i].obj_flags.bitvector[0] = asciiflag_conv(f1);
  obj_proto[i].obj_flags.bitvector[1] = asciiflag_conv(f2);
  obj_proto[i].obj_flags.bitvector[2] = asciiflag_conv(f3);
  obj_proto[i].obj_flags.bitvector[3] = asciiflag_conv(f4);

      oCheckAffectFlags(&obj_proto[i]);
      break;

    case 'A': /* affect */
      if( j >= MAX_OBJ_AFFECT )
      {
	mudlog(NRM, LVL_IMMORT, TRUE, "Too many A fields (%d max), %s\n", MAX_OBJ_AFFECT, buf2 );
	return( OBJEOF );
      }
      get_line(obj_f, line);
      sscanf(line, " %d %d ", t, t + 1);
      if ( (t[0] < 0) || (t[0] > (NUM_APPLIES - 1)) ) t[0] = APPLY_NONE;
      obj_proto[i].affected[j].location = t[0];
      t[1] = applyModLimit(t[0], t[1]); /* this function from oedit.h */
      obj_proto[i].affected[j].modifier = t[1];
      j++;
      break;

    case 'T':                   /* new TRAP flag */
      obj_proto[i].traplevel = MAX(1, MIN(LVL_IMMORT, atoi(line+1)));
      break;
    case 'Z':
      dg_obj_trigger(line, &obj_proto[i]);
      break;
    case 'D': /* TIMED object */
      obj_proto[i].obj_flags.timer = MAX(0, atoi(line+1));
      break;
    case '$':
    case '#':
      calculateRent(obj_proto+i);
      top_of_objt = i++;
      return line;
      break;

    default:
      mudlog(NRM, LVL_IMMORT, TRUE, "IGN3:Format error in %s", buf2 );
      return( OBJEOF );
      break;
    }
  }
}

/*
** Load all quests from a quest file
*/
char *
parse_quest( FILE * qst_f, int nr )
{
  int p = 0, r = 0, o = 0, g = 0;
  static unsigned int i = 0;
  static char line[256];
  char f1[128], f2[128];
  int retval;
  int t[10];
  int spl;

  // clear the quest data
  memset(qst_list + i, 0, sizeof(QuestData));

  qst_list[i].virtual = nr;
  qst_list[i].number = i;

  sprintf(buf2, "quest #%d", nr);

  /* string data */
  if(( qst_list[i].name = fread_string( qst_f, buf2 )) == NULL )
  {
    mudlog(NRM, LVL_IMMORT, TRUE, "Null quest name or format error at or near %s\n", buf2 );
    return( "#99999" );
  }

  qst_list[i].speech = fread_string( qst_f, buf2 );
  qst_list[i].description = fread_string( qst_f, buf2 );

  /* numeric data */
  if (!get_line(qst_f, line) ||
      (retval = sscanf(line, "%d %d %s %d %d %d", t, t+1, f1, t+2, t+3, t+4) != 6)
     ) {
    fprintf(stderr, "Format error in numeric line (expecting 6 args, got %d), %s\n", retval, buf2);
    exit(1);
  }

  qst_list[i].minlvl = t[0];
  qst_list[i].maxlvl = t[1];
  qst_list[i].flags = asciiflag_conv(f1);
  qst_list[i].timelimit = t[2];
  qst_list[i].waitcomplete = t[3];
  qst_list[i].waitrefuse = t[4];

  /* tasks, prerequisites, and rewards */
  for (;;) {
    if( !get_line( qst_f, line ))
    {
      mudlog(NRM, LVL_IMMORT, TRUE, "IGN1: Format error in %s", buf2 );
      return( OBJEOF );
    }

    switch (*line) {
      case 'R':  /* reward */
        if (r > MAX_QST_REWARDS) {
          mudlog(NRM, LVL_IMMORT, TRUE, "Too many rewards (%d max), %s\n", MAX_QST_REWARDS, buf2);
          return (OBJEOF);
        }
        if ((retval = sscanf(line, "%*c %s %s", f1, f2)) != 2) {
          mudlog(NRM, LVL_IMMORT, TRUE, "Broken reward line (expecting 2 args, got %d), %s\n",
              retval, buf2);
          return (OBJEOF);
        }
        if (strcmp(f1, "QP") == 0) {
          qst_list[i].rewards[r].type = QST_REWARD_QP;
          qst_list[i].rewards[r].amount = atoi(f2);
        } else if (strcmp(f1, "EXP") == 0) {
          qst_list[i].rewards[r].type = QST_REWARD_EXP;
          qst_list[i].rewards[r].amount = atoi(f2);
        } else if (strcmp(f1, "OBJ") == 0) {
          qst_list[i].rewards[r].type = QST_REWARD_OBJ;
          qst_list[i].rewards[r].amount = atoi(f2);
        } else if (strcmp(f1, "GOLD") == 0) {
          qst_list[i].rewards[r].type = QST_REWARD_GOLD;
          qst_list[i].rewards[r].amount = atoi(f2);
        } else if (strcmp(f1, "PRAC") == 0) {
          qst_list[i].rewards[r].type = QST_REWARD_PRAC;
          qst_list[i].rewards[r].amount = atoi(f2);
        } else if (strcmp(f1, "SPELL") == 0) {
          qst_list[i].rewards[r].type = QST_REWARD_SPELL;
          spl = find_skill_num(f2);
          if (spl < 1 || spl > MAX_SPELLS) spl = atoi(f2);
          if (spl < 1 || spl > MAX_SPELLS) spl = 0;
          if (spl == 0) mudlog(NRM, LVL_IMMORT, TRUE, "Unknown spell %s\n", f2);
          qst_list[i].rewards[r].amount = spl;
        } else if (strcmp(f1, "REMORT") == 0) {
            qst_list[i].rewards[r].type = QST_REWARD_REMORT;
            qst_list[i].rewards[r].amount = atoi(f2);
        } else {
          mudlog(NRM, LVL_IMMORT, TRUE, "Unknown reward type, %s\n", buf2);
          return (OBJEOF);
        }
        qst_list[i].rewards[r++].speech = fread_string(qst_f, buf2);
        break;
      case 'P':
        if (p > MAX_QST_PREREQ) {
          mudlog(NRM, LVL_IMMORT, TRUE, "Too many prereqs (%d max), %s\n", MAX_QST_PREREQ, buf2);
          return (OBJEOF);
        }
        if ((retval = sscanf(line, "%*c %d", t)) != 1) {
          mudlog(NRM, LVL_IMMORT, TRUE, "Broken prereq line (expecting 1 args, got %d), %s\n",
              retval, buf2);
          return (OBJEOF);
        }
        qst_list[i].prereqs[p++] = t[0];
        break;
      case 'G':
        if (g > MAX_QST_GIVERS) {
          mudlog(NRM, LVL_IMMORT, TRUE, "Too many givers (%d max), %s\n", MAX_QST_GIVERS, buf2);
          return (OBJEOF);
        }
        if ((retval = sscanf(line, "%*c %d", t)) != 1) {
          mudlog(NRM, LVL_IMMORT, TRUE, "Broken givers line (expecting 1 args, got %d), %s\n",
              retval, buf2);
          return (OBJEOF);
        }
        qst_list[i].givers[g++] = t[0];
        break;
      case 'O':
        if (o > MAX_QST_TASKS) {
          mudlog(NRM, LVL_IMMORT, TRUE, "Too many tasks (%d max), %s\n", MAX_QST_TASKS, buf2);
          return (OBJEOF);
        }
        if ((retval = sscanf(line, "%*c %s %d", f1, t)) != 2) {
          mudlog(NRM, LVL_IMMORT, TRUE, "Broken task line (expecting 2 args, got %d), %s\n",
              retval, buf2);
          return (OBJEOF);
        }
        if (strcmp(f1, "OBJ") == 0) {
          qst_list[i].tasks[o].type = QST_TASK_OBJ;
        } else if (strcmp(f1, "MOB") == 0) {
          qst_list[i].tasks[o].type = QST_TASK_MOB;
        } else {
          mudlog(NRM, LVL_IMMORT, TRUE, "Unknown task type, %s\n", buf2);
          return (OBJEOF);
        }
        qst_list[i].tasks[o++].identifier = t[0];
        break;
      case '$':
      case '#':
        top_of_qstt = i++;
        return line;
        break;
      default:
        mudlog(NRM, LVL_IMMORT, TRUE, "Format error in %s", buf2 );
        return( OBJEOF );
        break;
    }
  }
}

static void get_one_line(FILE *fl, char *buf)
{
  if (fgets(buf, READ_SIZE, fl) == NULL) {
    mlog("SYSERR: error reading help file: not terminated with $?");
    exit(1);
  }

  buf[strlen(buf) - 1] = '\0'; /* take off the trailing \n */
}

void free_help(struct help_index_element *hentry)
{
  if (hentry->keywords)
    free(hentry->keywords);
  if (hentry->entry && !hentry->duplicate)
    free(hentry->entry);

  free(hentry);
}

void free_help_table(void)
{
  if (help_table) {
    int hp;
    for (hp = 0; hp < top_of_helpt; hp++) {
      if (help_table[hp].keywords)
        free(help_table[hp].keywords);
      if (help_table[hp].entry && !help_table[hp].duplicate)
        free(help_table[hp].entry);
    }
    free(help_table);
    help_table = NULL;
  }
  top_of_helpt = 0;
}

void load_help(FILE * fl, char *name)
{
  char key[READ_SIZE + 1], next_key[READ_SIZE + 1], entry[32384];
  size_t entrylen;
  char line[READ_SIZE + 1], hname[READ_SIZE + 1], *scan;
  struct help_index_element el;

  strlcpy(hname, name, sizeof(hname));

  get_one_line(fl, key);
  while (*key != '$') {
    strcat(key, "\r\n"); /* strcat: OK (READ_SIZE - "\n"  "\r\n" == READ_SIZE  1) */
    entrylen = strlcpy(entry, key, sizeof(entry));

    /* Read in the corresponding help entry. */
    get_one_line(fl, line);
    while (*line != '#' && entrylen < sizeof(entry) - 1) {
      entrylen += strlcpy(entry + entrylen, line, sizeof(entry) - entrylen);

      if (entrylen + 2 < sizeof(entry) - 1) {
        strcpy(entry + entrylen, "\r\n"); /* strcpy: OK (size checked above) */
        entrylen += 2;
      }
      get_one_line(fl, line);
    }

    if (entrylen >= sizeof(entry) - 1) {
      int keysize;
      const char *truncmsg = "\r\n*TRUNCATED*\r\n";

      strcpy(entry + sizeof(entry) - strlen(truncmsg) - 1, truncmsg); /* strcpy: OK (assuming sane 'entry' size) */

      keysize = strlen(key) - 2;
      mlog("SYSERR: Help entry exceeded buffer space: %.*s", keysize, key);

      /* If we ran out of buffer space, eat the rest of the entry. */
      while (*line != '#')
      get_one_line(fl, line);
    }

    if (*line == '#') {
      if (sscanf(line, "#%d", &el.min_level) != 1) {
        mlog("SYSERR: Help entry does not have a min level. %s", key);
        el.min_level = 0;
      }
    }

    el.duplicate = 0;
    el.entry = strdup(entry);
    scan = one_word(key, next_key);

    while (*next_key) {
      el.keywords = strdup(next_key);
      help_table[top_of_helpt++] = el;
      el.duplicate++;
      scan = one_word(scan, next_key);
    }
  get_one_line(fl, key);
  }
}

static int hsort(const void *a, const void *b)
{
  const struct help_index_element *a1, *b1;

  a1 = (const struct help_index_element *) a;
  b1 = (const struct help_index_element *) b;

  return (str_cmp(a1->keywords, b1->keywords));
}


/*
** load the zone table and command tables
*/
void
load_zones( FILE* fl, char* zonename )
{
# define Z zone_table[zone]

  static int zone = 0;
  int cmd_no = 0, numCmds = 0, line_num = 0, tmp;
  char *ptr, buf[256], zname[256];
  char t1[80], t2[80];

  strcpy( zname, zonename );

  // Get a quick line count on the file
  //
  while( get_line( fl, buf )) numCmds++;
  rewind(fl);

  if( numCmds == 0 )
  {
    mudlog(NRM, LVL_IMMORT, TRUE, "SYSERR: %s is empty - skipping the load", zname );
    return;
  }

  CREATE(Z.cmd, ResetCom, numCmds);
  Z.tournament_room = 0;
  Z.kill_count      = 0;
  Z.pkill_room      = 0;

  line_num += get_line( fl, buf );

  if( sscanf(buf, "#%d", &Z.number) != 1 )
  {
    mudlog(NRM, LVL_IMMORT, TRUE, "SYSERR: Format error in %s, line %d", zname, line_num );
    return;
  }

  // The Quest fields are in 200 and 201.
  //
  Z.pkill_room = (( Z.number == 200 ) || ( Z.number == 201 ) || (Z.number == 204) || (Z.number == 248));

  line_num += get_line(fl, buf);
  if ((ptr = strchr(buf, '~')) != NULL)	/* take off the '~' if it's there */
    *ptr = '\0';
  Z.name = str_dup(buf);

  line_num += get_line(fl, buf);

  if( sscanf(buf, " %d %d %d %d",
             &Z.top, &Z.lifespan, &Z.reset_mode, &Z.zone_flags) != 4)
  {
    mudlog(NRM, LVL_IMMORT, TRUE, "SYSERR: Format error in 4-constant line of %s", zname );
    return;
  }

  cmd_no = 0;

  for(;;)
  {
    int error = 0;
    if(( tmp = get_line(fl, buf)) == 0 )
    {
      mudlog(NRM, LVL_IMMORT, TRUE, "SYSERR: Format error in %s - premature end of file", zname );
      return;
    }

    line_num += tmp;
    ptr = buf;
    skip_spaces(&ptr);

    // Skip comments.
    //
    if((ZCMD.command = *ptr) == '*') continue;

    ptr++;

    if (ZCMD.command == 'S' || ZCMD.command == '$')
    {
      ZCMD.command = 'S';
      break;
    }

    // Check for arena safety room.
    //
    if( ZCMD.command == 'T' )
    {
      sscanf(ptr, " %d ", &Z.tournament_room );
    }
    else if( strchr( "MOEPDNRZV", ZCMD.command) == NULL )
    {
      // These are all 3-arg commands.
      //
      error = (sscanf( ptr, " %d %d %d ",
                             &tmp, &ZCMD.arg1, &ZCMD.arg2) != 3);
      if (error && ZCMD.command=='V') {
        error = (sscanf(ptr, " %d %d %d %d %s %s", &tmp, &ZCMD.arg1,
              &ZCMD.arg2, &ZCMD.arg3, t1, t2) != 6);
        if (!error) {
          ZCMD.sarg1 = str_dup(t1);
          ZCMD.sarg2 = str_dup(t2);
        }
      }
    }
    else
    {
      error = (sscanf( ptr, " %d %d %d %d ",
                             &tmp, &ZCMD.arg1, &ZCMD.arg2, &ZCMD.arg3) != 4);
    }

    ZCMD.if_flag = tmp;

    if( error )
    {
      mudlog(NRM, LVL_IMMORT, TRUE, "SYSERR: Format error in %s, line %d: '%s'",
                zname, line_num, buf);
      return;
    }

    ZCMD.line = line_num;
    cmd_no++;
  }

  top_of_zone_table = zone++;
# undef Z
}



/*************************************************************************
*  procedures for resetting, both play-time and boot-time	 	 *
*********************************************************************** */



int vnum_mobile(char *searchname, CharData * ch)
{
  int nr, found = 0;

  for (nr = 0; nr <= top_of_mobt; nr++) {
    if (isname(searchname, mob_proto[nr].player.name)) {
      sprintf(buf, "%3d. [%5d] %s\r\n", ++found,
	      mob_index[nr].virtual,
	      mob_proto[nr].player.short_descr);
      send_to_char(buf, ch);
    }
  }

  return (found);
}



int vnum_object(char *searchname, CharData * ch)
{
  int nr, found = 0;

  for (nr = 0; nr <= top_of_objt; nr++) {
    if (isname(searchname, obj_proto[nr].name)) {
      sprintf(buf, "%3d. [%5d] %s\r\n", ++found,
	      obj_index[nr].virtual,
	      obj_proto[nr].short_description);
      send_to_char(buf, ch);
    }
  }
  return (found);
}


/* create a character, and add it to the char list */
CharData *create_char(void)
{
  CharData *ch;

  CREATE(ch, CharData, 1);
  clear_char(ch);
  ch->next = character_list;
  character_list = ch;
  GET_ID(ch) = max_id++;

  return ch;
}


/* create a new mobile from a prototype */
CharData *read_mobile(int nr, int type)
{
  int manacap;
  int i, class;
  CharData *mob;

  if (type == VIRTUAL) {
    if ((i = real_mobile(nr)) < 0) {
      sprintf(buf, "Mobile (V) %d does not exist in database.", nr);
      return (0);
    }
  } else
    i = nr;

  CREATE(mob, CharData, 1);
  clear_char(mob);
  *mob = mob_proto[i];
  mob->next = character_list;
  character_list = mob;

/* Vex, Febuary 17, 1997
** Previously, mobs mana was simply set to 10. I've changed that
** so that the mobs mana is determined in the same way as the mobs hps.
*/
  /* Had this problem showup when converting areas - Vex. */
  if ((mob->points.hit <= 0) || (mob->points.mana <=0))
  	mudlog(NRM, LVL_IMMORT, TRUE, "BUGGY MOB: mob->points.hit = %d, mob->points.mana = %d, for mob %s.", mob->points.hit, mob->points.mana, mob->player.name);

  if (!mob->points.max_hit)
  {
    mob->points.max_hit = dice(mob->points.hit, mob->points.mana) +
      mob->points.move;
    mob->points.max_mana = dice(mob->points.hit, mob->points.mana) +
      mob->points.move;
  }
  else
  {
    mob->points.max_hit = number(mob->points.hit, mob->points.mana);
    mob->points.max_mana = number(mob->points.hit, mob->points.mana);
  }

/* Vex, Febuary 17, 1997
** Ok, now lets modify the mobs mana according to there class.
*/
  class = GET_CLASS(mob);
  switch(class) {
    case CLASS_DEATH_KNIGHT:
    case CLASS_SOLAMNIC_KNIGHT:
	mob->points.max_mana = MAX(1, ((mob->points.max_mana << 1)/6)) ; /* One Third */
	manacap = 20;
	break;
    case CLASS_SHOU_LIN:
    case CLASS_SHADOW_DANCER:
	mob->points.max_mana = MAX(1, (mob->points.max_mana >> 1)); /* One Half */
	manacap = 30;
	break;
    case CLASS_ASSASSIN:
    case CLASS_RANGER:
	mob->points.max_mana >>=2; /* One Quarter */
	manacap = 15;
	break;
    case CLASS_THIEF:
    case CLASS_WARRIOR:
	mob->points.max_mana = MAX(1, (mob->points.max_mana  >> 3)); /* One Eighth */
	manacap = 0;
	break;
    case CLASS_MAGIC_USER:
    case CLASS_NECROMANCER:
    case CLASS_CLERIC:
	manacap = 70;
	break;			/* Unmodified */
    default:
	/* New class added, let em know they have more to do. */
	mudlog(NRM, LVL_IMMORT, TRUE, "INFO: Mob %s class is unknown to read_mobile.", mob->player.name);
	manacap = 0;
	break;
  } /* End switch */

  // Try to make sure mana isn't to outrageous.
  mob->points.max_mana = MIN( (100 + (manacap * GET_LEVEL(mob))), mob->points.max_mana);

  mob->points.hit = mob->points.max_hit;
  mob->points.mana = mob->points.max_mana;
  mob->points.move = mob->points.max_move;

  mob->player.time.birth = time(0);
  mob->player.time.played = 0;
  mob->player.time.logon = time(0);

  mob_index[i].number++;
  GET_ID(mob) = max_id++;
  assign_triggers(mob, MOB_TRIGGER);

  return mob;
}


/* create an object, and add it to the object list */
ObjData *create_obj(void)
{
  ObjData *obj;

  CREATE(obj, ObjData, 1);
  clear_object(obj);
  obj->next = object_list;
  object_list = obj;
  GET_ID(obj) = max_id++;
  assign_triggers(obj, OBJ_TRIGGER);

  return obj;
}


/* create a new object that's a perfect copy
   of the prototype object                  */
ObjData *read_perfect_object(int nr, int type)
{
  ObjData *obj;
  int i;

  if (nr < 0) {
    mlog("SYSERR: trying to create obj with negative num!");
    return NULL;
  }
  if (type == VIRTUAL) {
    if ((i = real_object(nr)) < 0) {
      sprintf(buf, "Object (V) %d does not exist in database.", nr);
      return NULL;
    }
  } else
    i = nr;

  if (IS_LOCKER(obj_proto + i) && obj_index[i].number != 0) {
    sprintf(buf, "Can't create more than one locker.");
    return NULL;
  }

  CREATE(obj, ObjData, 1);
  clear_object(obj);
  *obj = obj_proto[i];
  obj->next = object_list;
  object_list = obj;

  obj_index[i].number++;
  GET_ID(obj) = max_id++;

  // If the object is a trophy, we want to set it up so it automatically deletes itself
  // at the appropriate time in the future.  The object's cost/day for rent is the
  // number of months it will exist.
  if(IS_OBJ_STAT(obj, ITEM_TROPHY)) {
      // For player/builder clarity, an object will have a lifetime equal
      // to what the builder sets the timer to
      GET_OBJ_TIMER(obj) += TICKS_SO_FAR;
  }

  assign_triggers(obj, OBJ_TRIGGER);

  if (IS_LOCKER(obj)) {
    LockerList *listelem = malloc(sizeof(LockerList));
    listelem->locker = obj;
    listelem->next = lockers;
    lockers = listelem;
    load_locker(obj);
  }

  return obj;
}

/* create a new, bad object from a prototype.  This is a
   temporary function to keep really good equipment from
   appearing, and to keep decently good equipment from
   being scrambled non-obviously */
ObjData *read_failed_object(int nr, int type)
{
  ObjData *obj;
  int aff_val, i;

  if (nr < 0) {
    mlog("SYSERR: trying to create obj with negative num!");
    return NULL;
  }
  if (type == VIRTUAL) {
    if ((i = real_object(nr)) < 0) {
      sprintf(buf, "Object (V) %d does not exist in database.", nr);
      return NULL;
    }
  } else
    i = nr;

  if (IS_LOCKER(obj_proto + i) && obj_index[i].number != 0) {
    sprintf(buf, "Can't create more than one locker.");
    return NULL;
  }

  CREATE(obj, ObjData, 1);
  clear_object(obj);
  *obj = obj_proto[i];

  // Here is where we Make Terrible the equipment stats:

  // 1. Go through every affects slot.
  for (aff_val = 0; aff_val < MAX_OBJ_AFFECT; aff_val++){
  // Lets have Artifacts load Max! - Bean
	  if (obj->affected[aff_val].location == APPLY_NONE || ( IS_OBJ_STAT(obj, ITEM_ARTIFACT ) ) )
		  break;

	  // Set the stat to zero - object load fails
	  obj->affected[aff_val].modifier = 0;
  }

  if( GET_OBJ_TYPE(obj) == ITEM_ARMOR )
	  GET_OBJ_VAL( obj, 0 ) = 0;

  obj->next = object_list;
  object_list = obj;

  obj_index[i].number++;
  GET_ID(obj) = max_id++;
  assign_triggers(obj, OBJ_TRIGGER);

  if (IS_LOCKER(obj)) {
    LockerList *listelem = malloc(sizeof(LockerList));
    listelem->locker = obj;
    listelem->next = lockers;
    lockers = listelem;
    load_locker(obj);
  }

  return obj;
}


/* create a new object from a prototype */
ObjData *read_object(int nr, int type)
{
  ObjData *obj;
  int i, aff_val, negative_val;

  if (nr < 0) {
    mlog("SYSERR: trying to create obj with negative num!");
    return NULL;
  }
  if (type == VIRTUAL) {
    if ((i = real_object(nr)) < 0) {
      sprintf(buf, "Object (V) %d does not exist in database.", nr);
      return NULL;
    }
  } else
    i = nr;

  if (IS_LOCKER(obj_proto + i) && obj_index[i].number != 0) {
    sprintf(buf, "Can't create more than one locker.");
    return NULL;
  }

  CREATE(obj, ObjData, 1);
  clear_object(obj);
  *obj = obj_proto[i];

  // Here is where we randomize equipment stats:
  // If the flag ITEM_RANDOMIZED is set, go through every
  // affect slot and randomize it.
  if (IS_OBJ_STAT(obj, ITEM_RANDOMIZED))
  {
      for (aff_val = 0; aff_val < MAX_OBJ_AFFECT; aff_val++)
      {
          if (obj->affected[aff_val].location == APPLY_NONE )
              break;

          negative_val = (obj->affected[aff_val].modifier < 0);
          // We evaluate them all as positive, and then flip them back negative at the end.
          if (negative_val)
              obj->affected[aff_val].modifier *= -1;
          
          // Don't Randomize apply_uselevel, class, or level.
          if (obj->affected[aff_val].location == APPLY_USELEVEL ||
                  obj->affected[aff_val].location == APPLY_CLASS    ||
                  obj->affected[aff_val].location == APPLY_LEVEL      )
              continue;

          if (obj->affected[aff_val].modifier != 0)
			  obj->affected[aff_val].modifier =
			  number( number(1, obj->affected[aff_val].modifier), obj->affected[aff_val].modifier);

		  if (negative_val)
			  obj->affected[aff_val].modifier *= -1;
	  }

	  // Two rolls made for ACAP, choose the higher value.
	  if( GET_OBJ_TYPE(obj) == ITEM_ARMOR )
		  GET_OBJ_VAL( obj, 0 ) = MAX( number( 0, GET_OBJ_VAL(obj, 0)), number( 0, GET_OBJ_VAL(obj, 0)) );
  }

  // If the object is a trophy, we want to set it up so it automatically deletes itself
  // at the appropriate time in the future.  The object's cost/day for rent is the
  // number of months it will exist.
  if(IS_OBJ_STAT(obj, ITEM_TROPHY)) {
      // For player/builder clarity, an object will have a lifetime equal
      // to what the builder sets the timer to
      GET_OBJ_TIMER(obj) += TICKS_SO_FAR;
  }

  obj->next = object_list;
  object_list = obj;

  obj_index[i].number++;
  GET_ID(obj) = max_id++;
  assign_triggers(obj, OBJ_TRIGGER);

  if (IS_LOCKER(obj)) {
	  LockerList *listelem = malloc(sizeof(LockerList));
	  listelem->locker = obj;
	  listelem->next = lockers;
	  lockers = listelem;
	  load_locker(obj);
  }

  return obj;
}

#define ZO_DEAD  999

/*
** update zone ages, queue for reset if necessary,
** and dequeue when possible
*/
void
zone_update(void)
{
  int i;
  ResetQElement *update_u;
  static int timer = 0;

  if( ((++timer * PULSE_ZONE) / PASSES_PER_SEC) >= 60 )
  {
    /*
    ** NOT accurate unless PULSE_ZONE is a multiple of PASSES_PER_SEC
    ** or a factor of 60
    */
    timer = 0;

    /* since one minute has passed, increment zone ages */
    for( i = 0; i <= top_of_zone_table; i++ )
    {
      if( zone_table[i].age < zone_table[i].lifespan &&
          zone_table[i].reset_mode )
        zone_table[i].age += 1;

      if( zone_table[i].age >= zone_table[i].lifespan &&
          zone_table[i].age < ZO_DEAD &&
          zone_table[i].reset_mode)
      { /* enqueue zone */
        CREATE(update_u, struct reset_q_element, 1);
        update_u->zone_to_reset = i;
        update_u->next = 0;
        if( !reset_q.head )
          reset_q.head = reset_q.tail = update_u;
        else
        {
          reset_q.tail->next = update_u;
          reset_q.tail = update_u;
        }
        zone_table[i].age = ZO_DEAD;
      }
    }
  }
  /*
  ** dequeue zones (if possible) and reset this code is
  ** executed every 10 seconds (i.e. PULSE_ZONE)
  */
  for( update_u = reset_q.head; update_u; update_u = update_u->next )
  {
    if( zone_table[ update_u->zone_to_reset ].reset_mode >= 2 ||
        is_empty( update_u->zone_to_reset ))
    {
      reset_zone(update_u->zone_to_reset);
      // Disabling mudlog of this because we never use it and it makes the logs spammy - Arbaces
      //mudlog( CMP, LVL_LRGOD, FALSE, "Auto zone reset: %s", zone_table[update_u->zone_to_reset].name);

      /*
      ** dequeue
      */
      if( update_u == reset_q.head )
        reset_q.head = reset_q.head->next;
      else
      {
        ResetQElement *temp;
        for( temp = reset_q.head; temp->next != update_u; temp = temp->next);

        if( !update_u->next ) reset_q.tail = temp;

        temp->next = update_u->next;
      }
      free(update_u);
      break;
    }
  }
}

void
log_zone_error(int zone, int cmd_no, char *message)
{
  mudlog( NRM, LVL_LRGOD, TRUE, "SYSERR: error in zone file: %s", message);

  mudlog( NRM, LVL_LRGOD, TRUE, "SYSERR: ...offending cmd: '%c' cmd in zone #%d, line %d",
	  ZCMD.command, zone_table[zone].number, ZCMD.line);
}

#define ZONE_ERROR(message) \
	{ log_zone_error(zone, cmd_no, message); last_cmd = 0; }

/*
** execute the reset command table of a given zone
*/
void
reset_zone(int zone)
{
  CharData *mob = NULL;
  ObjData  *obj = NULL;
  ObjData  *obj_to = NULL;
  int cmd_no, last_cmd = 0;
  int room_vnum, room_rnum;
  struct char_data *tmob=NULL; /* for trigger assignment */
  struct obj_data *tobj=NULL;  /* for trigger assignment */

  for( cmd_no = 0; ZCMD.command != 'S'; cmd_no++ )
  {
    if( ZCMD.if_flag && !last_cmd ) continue;

    switch( ZCMD.command )
    {
    case '*': break; /* Just a comment. */

    case 'M': /* Mobs */
      if( mob_index[ZCMD.arg1].number < ZCMD.arg2 )
      {
        mob = read_mobile(ZCMD.arg1, REAL);
        char_to_room(mob, ZCMD.arg3);
        load_mtrigger(mob);
        tmob = mob;
        last_cmd = 1;
      }
      else last_cmd = 0;
      tobj = NULL;
      break;

    case 'O': /* Objects */
      if(( obj_index[ZCMD.arg1].number < ZCMD.arg2 ) || percentSuccess( 10 ))
      {
        if( ZCMD.arg3 >= 0 )
        {
          if( !get_obj_in_list_num(ZCMD.arg1, world[ZCMD.arg3].contents ))
          {
            obj = read_object( ZCMD.arg1, REAL );
            obj_to_room( obj, ZCMD.arg3 );
            load_otrigger(obj);
            tobj = obj;
            last_cmd = 1;
          }
          else last_cmd = 0;
        }
        else
        {
          obj = read_object(ZCMD.arg1, REAL);
          load_otrigger(obj);
          obj->in_room = NOWHERE;
          last_cmd = 1;
          tobj = obj;
        }
      }
      else last_cmd = 0;
      tmob = NULL;
      break;

    case 'R': /* Random Object */
      if( ZCMD.arg2 >= 0 )
      {
        if(( obj = random_object( ZCMD.arg1, ZCMD.arg3 )))
        {
          obj_to_room( obj, ZCMD.arg2 );
          load_otrigger(obj);
          tobj = obj;
          last_cmd = 1;
        }
        else last_cmd = 0;
      }
      else
      {
        if(( obj = random_object( ZCMD.arg1, ZCMD.arg3 )))
        {
          obj->in_room = NOWHERE;
          load_otrigger(obj);
          tobj = obj;
          last_cmd = 1;
        }
        else last_cmd = 0;
      }
      tmob = NULL;
      break;

    case 'A':                   /* set trap */
      last_cmd = 0;
      if (ZCMD.arg2 < 0 || ZCMD.arg2 >= NUM_OF_DIRS ||
	  (world[ZCMD.arg1].dir_option[ZCMD.arg2] == NULL)) {
        sprintf(buf, "exit %d does not exist in room %d",
                ZCMD.arg2, world[ZCMD.arg1].number );
	ZONE_ERROR(buf);
      } else {
        GET_TRAP(world[ZCMD.arg1].dir_option[ZCMD.arg2]) =
          MAX(1, MIN(LVL_IMMORT, ZCMD.arg3));
        last_cmd = 1;
      }
      break;

    case 'P': /* Place Object in Object */
      if( obj_index[ZCMD.arg1].number < ZCMD.arg2 )
      {
        obj = read_object(ZCMD.arg1, REAL);
        if (!(obj_to = get_obj_num(ZCMD.arg3)))
        {
          ZONE_ERROR("target obj not found");
          break;
        }
        obj_to_obj(obj, obj_to);
        load_otrigger(obj);
        tobj = obj;
        last_cmd = 1;
      }
      else last_cmd = 0;
      tmob = NULL;
      break;

    case 'T': /* random obj to obj */
      last_cmd = 0;
      if(( obj = random_object( ZCMD.arg1, ZCMD.arg3 )))
      {
        if(( obj_to = get_obj_num(ZCMD.arg2) ))
        {
          obj_to_obj( obj, obj_to );
          load_otrigger(obj);
          tobj = obj;
          last_cmd = 1;
        }
        else
          ZONE_ERROR("target obj not found for rnd item");
      }
      tmob = NULL;
      break;

    case 'G':			/* obj_to_char */
      last_cmd = 0;
      if( mob )
      {
        if (obj_index[ZCMD.arg1].number < ZCMD.arg2)
        {
	  obj = read_object(ZCMD.arg1, REAL);
          obj->in_room = NOWHERE;
	  obj_to_char(obj, mob);
          load_otrigger(obj);
          tobj = obj;
	  last_cmd = 1;
        }
      }
      else
	ZONE_ERROR( "attempt to give obj to non-existant mob" );
      tmob = NULL;
      break;

    case 'Q':
      last_cmd = 0;
      if( mob )
      {
        obj = random_object(ZCMD.arg1, ZCMD.arg2);
        if( obj )
        {
          obj->in_room = NOWHERE;
          obj_to_char(obj, mob);
          load_otrigger(obj);
          tobj = obj;
          last_cmd = 1;
        }
      }
      else
        ZONE_ERROR("attempt to give rand obj to null mob");
      tmob = NULL;
      break;

/* DIGGER */
    case 'E':			/* object to equipment list */
      if (!mob) {
	ZONE_ERROR("trying to equip non-existant mob");
	break;
      }
      if (obj_index[ZCMD.arg1].number < ZCMD.arg2) {
	if (ZCMD.arg3 < 0 || ZCMD.arg3 >= NUM_WEARS) {
	  ZONE_ERROR("invalid equipment pos number");
	} else {
	  obj = read_object(ZCMD.arg1, REAL);
	  if (obj && (ZCMD.arg3 == 19) && (GET_OBJ_TYPE(obj) != ITEM_WEAPON))
	  {
		ZONE_ERROR("trying to make mob wield non-weapon");
		obj_to_char(obj, mob);
	  }
	  else {
            IN_ROOM(obj) = IN_ROOM(mob);
            load_otrigger(obj);
            IN_ROOM(obj) = NOWHERE;
            if (wear_otrigger(obj, mob, ZCMD.arg3))
              equip_char(mob, obj, ZCMD.arg3);
            else
              obj_to_char(obj, mob);
            tobj = obj;
          }
	  last_cmd = 1;
	}
      } else
	last_cmd = 0;
      tmob = NULL;
      break;
    case 'N':   /* random obj to eq list */
      if (!mob) {
        ZONE_ERROR("trying to equip rand item to null mob");
        break;
      }
      if (ZCMD.arg2 < 0 || ZCMD.arg2 >= NUM_WEARS) {
        ZONE_ERROR("invalid eq pos num for rand obj");
      } else {
        obj = random_object(ZCMD.arg1, ZCMD.arg3);
        if (obj)
        {
          IN_ROOM(obj) = IN_ROOM(mob);
          load_otrigger(obj);
          IN_ROOM(obj) = NOWHERE;
          if (wear_otrigger(obj, mob, ZCMD.arg3))
            equip_char(mob, obj, ZCMD.arg3);
          else
            obj_to_char(obj, mob);
          tobj = obj;
          last_cmd = 1;
        } else last_cmd = 0;
        tmob = NULL;
      }
      break;

    case 'D':			/* set state of door */
      if (ZCMD.arg2 < 0 || ZCMD.arg2 >= NUM_OF_DIRS ||
	  (world[ZCMD.arg1].dir_option[ZCMD.arg2] == NULL)) {
/* DIGGER */
        sprintf(buf, "door %d does not exist in room %d",
                ZCMD.arg2, world[ZCMD.arg1].number );
	ZONE_ERROR(buf);
      } else
	switch (ZCMD.arg3) {
	case 0:
	  REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info, EX_LOCKED);
	  REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info, EX_CLOSED);
	  REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info, EX_DOORBASHED);
	  break;
	case 1:
	  SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info, EX_CLOSED);
	  REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info, EX_LOCKED);
	  REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info, EX_DOORBASHED);
	  break;
	case 2:
	  SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info, EX_LOCKED);
	  SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info, EX_CLOSED);
	  REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info, EX_DOORBASHED);
	  break;
	}
      last_cmd = 1;
      break;

    case 'Z': /* trigger command; details to be filled in later */
      if (ZCMD.arg1==MOB_TRIGGER && tmob) {
        if (!SCRIPT(tmob))
          CREATE(SCRIPT(tmob), struct script_data, 1);
        add_trigger(SCRIPT(tmob), read_trigger(real_trigger(ZCMD.arg2)), -1);
        last_cmd = 1;
      } else if (ZCMD.arg1==OBJ_TRIGGER && tobj) {
        if (!SCRIPT(tobj))
          CREATE(SCRIPT(tobj), struct script_data, 1);
        add_trigger(SCRIPT(tobj), read_trigger(real_trigger(ZCMD.arg2)), -1);
        last_cmd = 1;
      }
      break;
    case 'V':
      if (ZCMD.arg1==MOB_TRIGGER && tmob) {
        if (!SCRIPT(tmob)) {
          ZONE_ERROR("Attempt to give variable to scriptless mobile");
        } else
          add_var(&(SCRIPT(tmob)->global_vars), ZCMD.sarg1, ZCMD.sarg2,
                  ZCMD.arg3);
        last_cmd = 1;
      } else if (ZCMD.arg1==OBJ_TRIGGER && tobj) {
        if (!SCRIPT(tobj)) {
          ZONE_ERROR("Attempt to give variable to scriptless object");
        } else
          add_var(&(SCRIPT(tobj)->global_vars), ZCMD.sarg1, ZCMD.sarg2,
                  ZCMD.arg3);
        last_cmd = 1;
      } else if (ZCMD.arg1==WLD_TRIGGER) {
        if (ZCMD.arg2<0 || ZCMD.arg2>top_of_world) {
          ZONE_ERROR("Invalid room number in variable assignment");
        } else {
          if (!(world[ZCMD.arg2].script)) {
            ZONE_ERROR("Attempt to give variable to scriptless object");
          } else
            add_var(&(world[ZCMD.arg2].script->global_vars),
                   ZCMD.sarg1, ZCMD.sarg2, ZCMD.arg3);
         last_cmd = 1;
        }
      }
      break;

    default:
      ZONE_ERROR("unknown cmd in reset table; cmd disabled");
      ZCMD.command = '*'; /* Used to do nothing here. */
      break;
    }
  }

  zone_table[zone].age = 0;

  /* handle reset_wtrigger's */
  room_vnum = zone_table[zone].number * 100;
  while (room_vnum <= zone_table[zone].top) {
    room_rnum = real_room(room_vnum);
    if (room_rnum != NOWHERE) reset_wtrigger(&world[room_rnum]);
    room_vnum++;
  }
}



/* for use in reset_zone; return TRUE if zone 'nr' is free of PC's  */
int is_empty(int zone_nr)
{
  DescriptorData *i;

  for (i = descriptor_list; i; i = i->next)
    if (!i->connected)
      if (world[i->character->in_room].zone == zone_nr)
	return (0);

  return (1);
}





/*************************************************************************
*  stuff related to the save/load player system				 *
*********************************************************************** */


long get_id_by_name(char *name)
{
  int i;

  one_argument(name, arg);
  for (i = 0; i <= top_of_p_table; i++)
    if (!strcmp((player_table + i)->name, arg))
      return ((player_table + i)->id);

  return -1;
}


char *get_name_by_id(long id)
{
  int i;

  for (i = 0; i <= top_of_p_table; i++)
    if ((player_table + i)->id == id)
      return ((player_table + i)->name);

  return NULL;
}


/* Load a char, TRUE if loaded, FALSE if not */
int
load_char( char *name, CharFileU *chu )
{
  int player_i;

  int find_name(char *name);

  if ((player_i = find_name(name)) >= 0) {
    fseek(player_fl, (long) (player_i * sizeof(struct char_file_u)), SEEK_SET);
    fread( chu, sizeof(*chu), 1, player_fl);
    return (player_i);
  } else
    return (-1);
}


int
load_char_quests(CharData *ch)
{
  char fname[100];
  FILE *f;
  int i;

  // set up some defaults
  GET_QUEST_WAIT(ch) = 0;
  GET_QUEST_COUNT(ch) = 0;
  ch->player_specials->quest.saved.quests = NULL;

  // attempt to read the real values
  if (!get_filename(GET_NAME(ch), fname, QUESTS_FILE)) return 0;
  if ((f = fopen(fname, "rb")) == NULL) return 0;
  fread(&GET_QUEST_WAIT(ch), sizeof(time_t), 1, f);
  fread(&GET_QUEST_COUNT(ch), sizeof(int), 1, f);
  CREATE(ch->player_specials->quest.saved.quests, int, GET_QUEST_COUNT(ch));
  fread(ch->player_specials->quest.saved.quests, sizeof(int),
      GET_QUEST_COUNT(ch), f);
  fclose(f);

  for (i = 0; i < GET_QUEST_COUNT(ch); i++) {
      chore_check_quest(ch, GET_QUEST_DONE(ch, i));
  }

  return 1;
}

void
save_char_quests(CharData *ch)
{
  char fname[100];
  FILE *f;

  if (!get_filename(GET_NAME(ch), fname, QUESTS_FILE)) return;
  if ((f = fopen(fname, "wb")) == NULL) return;
  fwrite(&GET_QUEST_WAIT(ch), sizeof(time_t), 1, f);
  fwrite(&GET_QUEST_COUNT(ch), sizeof(int), 1, f);
  fwrite(ch->player_specials->quest.saved.quests, sizeof(int),
      GET_QUEST_COUNT(ch), f);
  fclose(f);
}

/* write the vital data of a player to the player file */
void
save_char(CharData * ch, ush_int load_room)
{
    struct char_file_u st;

    if( IS_NPC(ch) || !ch->desc ) return;

    char_to_store(ch, &st);

    strncpy( st.host, ch->desc->host, HOST_LENGTH );
    st.host[HOST_LENGTH] = '\0';

    if( !PLR_FLAGGED( ch, PLR_LOADROOM ))
        st.player_specials_saved.load_room = load_room;

    strcpy(st.pwd, ch->desc->pwd);

    fseek( player_fl, ch->desc->pos * sizeof(struct char_file_u), SEEK_SET );
    fwrite( &st, sizeof(struct char_file_u), 1, player_fl );
    save_char_vars(ch);
    save_char_quests(ch);
}



/* copy data from the file structure to a char struct */
void store_to_char(struct char_file_u * st, CharData * ch)
{
  int i;

  /* to save memory, only PC's -- not MOB's -- have player_specials */
  if (ch->player_specials == NULL)
    CREATE(ch->player_specials, struct player_special_data, 1);

  ch->player_specials->pardons = NULL;

  GET_SEX(ch) = st->sex;
  GET_CLASS(ch) = st->class;
  SET_RACE(ch, st->race);
  GET_LEVEL(ch) = st->level;

  ch->player.short_descr = NULL;
  ch->player.long_descr = NULL;
  ch->player.title = str_dup(st->title);
  ch->player.description = str_dup(st->description);

  ch->player.lostlevels  = st->lostlevels;
  ch->player.orcs        = st->orcs;
  ch->player.time.birth  = st->birth;
  ch->player.time.played = st->played;
  ch->player.time.logon  = time(0);

  ch->player.weight = st->weight;
  ch->player.height = st->height;

  /* make sure abilities aren't above race max */
  st->abilities.str = MIN(st->abilities.str,
          race_stat_limits[st->race][STRENGTH_INDEX]);
  if (st->abilities.str < 18) st->abilities.str_add = 0;
  else if (st->abilities.str == 18)
    st->abilities.str_add = MIN(st->abilities.str_add,
            race_stat_limits[st->race][STRENGTH_ADD_INDEX]);
  else st->abilities.str_add = race_stat_limits[st->race][STRENGTH_ADD_INDEX];
  st->abilities.intel = MIN(st->abilities.intel,
          race_stat_limits[st->race][INTELLIGENCE_INDEX]);
  st->abilities.wis = MIN(st->abilities.wis,
          race_stat_limits[st->race][WISDOM_INDEX]);
  st->abilities.con = MIN(st->abilities.con,
          race_stat_limits[st->race][CONSTITUTION_INDEX]);
  st->abilities.dex = MIN(st->abilities.dex,
          race_stat_limits[st->race][DEXTERITY_INDEX]);
  st->abilities.cha = MIN(st->abilities.cha,
          race_stat_limits[st->race][CHARISMA_INDEX]);

  ch->real_abils = st->abilities;
  ch->aff_abils = st->abilities;
  ch->points = st->points;
  ch->char_specials.saved = st->char_specials_saved;
  ch->player_specials->saved = st->player_specials_saved;

#ifdef OLDWAY
  POOFIN(ch) = NULL;
  POOFOUT(ch) = NULL;
#else
  POOFIN(ch) = str_dup(st->player_specials_saved.poofin);
  POOFOUT(ch) = str_dup(st->player_specials_saved.poofout);
#endif
  if (ch->points.max_mana < 100)
    ch->points.max_mana = 100;

  ch->char_specials.carry_weight = 0;
  ch->char_specials.carry_items = 0;
  ch->points.armor = 100;
  ch->player_specials->saved.ac_remain = 0; /* stop remainders accumulating. */
  ch->points.hitroll = 0;
  SET_DAMROLL(ch, 0);

  CREATE(ch->player.name, char, strlen(st->name) + 1);
  strcpy(ch->player.name, st->name);

  /* Add all spell effects */
  for (i = 0; i < MAX_AFFECT; i++) {
    AffectedType af;

    *(AffectedTypeSaved *)&af = st->affected[i];
    if (st->affected[i].type)
      affect_to_char(ch, &af);
  }

  ch->in_room = GET_LOADROOM(ch);
  ch->necro_corpse_recall_room = 0;

  ch->state = 0;  //  You enter not singing.
  ch->offbalance = 0; // You enter not off balance.
  ch->call_to_corpse = 0; // Newborns fear not the end.
  GET_VOTE(ch) = 0;
  ch->flee_timer = 0;

/*   affect_total(ch); also - unnecessary?? */

  /*
   * If you're not poisioned and you've been away for more than an hour,
   * we'll set your HMV back to full
   */

  if( !IS_AFFECTED(ch, AFF_PLAGUE) && (((long) (time(0) - st->last_logon)) >= SECS_PER_REAL_HOUR)) {
    affect_from_char(ch, SPELL_PLAGUE);
  }

  if (!IS_AFFECTED(ch, AFF_POISON) && (((long) (time(0) - st->last_logon)) >= SECS_PER_REAL_HOUR)) {
    GET_HIT(ch) = GET_MAX_HIT(ch);
    GET_MOVE(ch) = GET_MAX_MOVE(ch);
    GET_MANA(ch) = GET_MAX_MANA(ch);
  }
}				/* store_to_char */




/* copy vital data from a players char-structure to the file structure */
void char_to_store(CharData * ch, struct char_file_u * st)
{
  int i, fishing, fish_on;
  struct affected_type *af;
  ObjData *char_eq[NUM_WEARS];

  /* Unaffect everything a character can be affected by */

  /*  Note that we should save and restore the FISHING and FISH_ON  states */
  fishing = PLR_FLAGGED(ch, PLR_FISHING);
  fish_on = PLR_FLAGGED(ch, PLR_FISH_ON);

  for (i = 0; i < NUM_WEARS; i++) {
    if (ch->equipment[i])
    {
      char_eq[i] = unequip_char(ch, i);
#ifndef NO_EXTRANEOUS_TRIGGERS
      remove_otrigger(char_eq[i], ch);
#endif
    }
    else
      char_eq[i] = NULL;
  }

  for (af = ch->affected, i = 0; i < MAX_AFFECT; i++) {
    if (af) {
      st->affected[i] = *(AffectedTypeSaved *)af;
      af = af->next;
    } else {
      st->affected[i].type = 0;	/* Zero signifies not used */
      st->affected[i].duration = 0;
      st->affected[i].modifier = 0;
      st->affected[i].location = 0;
      st->affected[i].bitvector = 0;
      st->affected[i].level = 0;
    }
  }


  /*
   * remove the affections so that the raw values are stored; otherwise the
   * effects are doubled when the char logs back in.
   */

  /* Vex. I've hacked this while loop so that berserk does'nt give funny
  ** results. Messing with the duration is ok because its destined to be
  ** nuked anyway(original has already been stored).
  **
  ** Mort.  added TRUE to affect_remove, this for for fall rooms, when the
  ** player types save it unaffects him removes fall and causes the player
  ** to fall if in a fall room
  **
  ** Imhotep.  It seems to be a tradition to do ugly hacks here.  FORTIFY
  ** must not have its special remove done here, so I copied Vex's neat
  ** little horror hack.
  **
  */
  while (ch->affected) {
    if (ch->affected->bitvector == AFF_BERSERK ||
        ch->affected->bitvector == AFF_FORTIFY) /* hack hack hack! */
	ch->affected->duration = -2; /* When affect_remove sees this, it does'nt go nuts. */
      affect_remove(ch, ch->affected, TRUE);
  } /* end while loop */

  if ((i >= MAX_AFFECT) && af && af->next)
    mlog("SYSERR: WARNING: OUT OF STORE ROOM FOR AFFECTED TYPES!!!");

  ch->aff_abils = ch->real_abils;

  st->birth = ch->player.time.birth;
  st->played = ch->player.time.played;
  st->played += (long) (time(0) - ch->player.time.logon);
  st->last_logon = time(0);

  ch->player.time.played = st->played;
  ch->player.time.logon = time(0);

  st->lostlevels = ch->player.lostlevels;
  st->orcs = ch->player.orcs;
  st->weight = GET_WEIGHT(ch);
  st->height = GET_HEIGHT(ch);
  st->sex = GET_SEX(ch);
  st->class = GET_CLASS(ch);
  st->race = GET_RACE(ch);
  st->level = GET_LEVEL(ch);
  st->abilities = ch->real_abils;
  st->points = ch->points;
  st->char_specials_saved = ch->char_specials.saved;
  st->player_specials_saved = ch->player_specials->saved;

  sprintf(st->player_specials_saved.poofin,"%s", POOFIN(ch));
  sprintf(st->player_specials_saved.poofout, "%s", POOFOUT(ch));


  st->points.armor = 100;
  st->points.hitroll = 0;
  st->points.damroll = 0;
  st->player_specials_saved.ac_remain = 0; /* stop remainders accumulating. */

  if (GET_TITLE(ch))
    strcpy(st->title, GET_TITLE(ch));
  else
    *st->title = '\0';

  if (ch->player.description)
    strcpy(st->description, ch->player.description);
  else
    *st->description = '\0';

  strcpy(st->name, GET_NAME(ch));

  /* add spell and eq affections back in now */
  for (i = 0; i < MAX_AFFECT; i++) {
    AffectedType af;

    *(AffectedTypeSaved *)&af = st->affected[i];
    if (st->affected[i].type)
      affect_to_char(ch, &af);
  }

  for (i = 0; i < NUM_WEARS; i++) {
    if (char_eq[i]) {
#ifndef NO_EXTRANEOUS_TRIGGERS
      if (wear_otrigger(char_eq[i], ch, i))
#endif
        equip_char(ch, char_eq[i], i);
#ifndef NO_EXTRANEOUS_TRIGGERS
      else
        obj_to_char(char_eq[i], ch);
#endif
    }
  }

  /* restore fishing and fish_on flags */
  if (fishing) SET_BIT_AR(PLR_FLAGS(ch), PLR_FISHING);
  if (fish_on) SET_BIT_AR(PLR_FLAGS(ch), PLR_FISH_ON);
/*   affect_total(ch); unnecessary, I think !?! */
}				/* Char to store */



void save_etext(CharData * ch)
{


}


/* create a new entry in the in-memory index table for the player file */
int create_entry(char *name)
{
  int i;

  if (top_of_p_table == -1) {
    CREATE(player_table, struct player_index_element, 1);
    top_of_p_table = 0;
  } else if (!(player_table = (struct player_index_element *)
	       realloc(player_table, sizeof(struct player_index_element) *
		       (++top_of_p_table + 1)))) {
    perror("create entry");
    exit(1);
  }
  CREATE(player_table[top_of_p_table].name, char, strlen(name) + 1);

  /* copy lowercase equivalent of name to table field */
  for (i = 0; (*(player_table[top_of_p_table].name + i) = LOWER(*(name + i)));
       i++);

  return (top_of_p_table);
}



/************************************************************************
*  procs of a (more or less) general utility nature			*
********************************************************************** */


/* read and allocate space for a '~'-terminated string from a given file */
char *fread_string(FILE * fl, char *error)
{
  char buf[MAX_STRING_LENGTH], tmp[512], *rslt;
  register char *point;
  int done = 0, length = 0, templength = 0;

  *buf = '\0';

  do {
    if( !fgets(tmp, 512, fl))
    {
      mudlog(NRM, LVL_IMMORT, TRUE, "Well THIS sucks ... [%s]", error );
      return(NULL);
    }
    /* If there is a '~', end the string; else an "\r\n" over the '\n'. */
    if ((point = strchr(tmp, '~')) != NULL) {
      *point = '\0';
      done = 1;
    } else {
      point = tmp + strlen(tmp) - 1;
      *(point++) = '\r';
      *(point++) = '\n';
      *point = '\0';
    }

    templength = strlen(tmp);

    if (length + templength >= MAX_STRING_LENGTH) {
      mlog("SYSERR: fread_string: string too large (db.c)");
      exit(1);
    } else {
      strcat(buf + length, tmp);
      length += templength;
    }
  } while (!done);

  /* allocate space for the new string and copy it */
  if (strlen(buf) > 0) {
    CREATE(rslt, char, length + 1);
    strcpy(rslt, buf);
  } else
    rslt = NULL;

  return rslt;
}


/* release memory allocated for a char struct */
void free_char(CharData * ch)
{
  int i;
  struct alias *a;

  void free_alias(struct alias * a);

  if (!IS_NPC(ch) || (IS_NPC(ch) && GET_MOB_RNUM(ch) == -1)) {
    /* if this is a player, or a non-prototyped non-player, free all */
    if (GET_NAME(ch))
      free(GET_NAME(ch));
    if (ch->player.title)
      free(ch->player.title);
    if (ch->player.short_descr)
      free(ch->player.short_descr);
    if (ch->player.long_descr)
      free(ch->player.long_descr);
    if (ch->player.description)
      free(ch->player.description);
  } else if ((i = GET_MOB_RNUM(ch)) > -1) {
    /* otherwise, free strings only if the string is not pointing at proto */
    if (ch->player.name && ch->player.name != mob_proto[i].player.name)
      free(ch->player.name);
    if (ch->player.title && ch->player.title != mob_proto[i].player.title)
      free(ch->player.title);
    if (ch->player.short_descr && ch->player.short_descr != mob_proto[i].player.short_descr)
      free(ch->player.short_descr);
    if (ch->player.long_descr && ch->player.long_descr != mob_proto[i].player.long_descr)
      free(ch->player.long_descr);
    if (ch->player.description && ch->player.description != mob_proto[i].player.description)
      free(ch->player.description);
  }
  while (ch->affected)
    affect_remove(ch, ch->affected, TRUE);

/* Mortius : Well I had the above affect_remove set to FALSE so it would try
   and run the fall code which is pretty damn dumb when this is free'ing up
   the character.  24-04-2000 : FALSE changed to TRUE so it won't run the
   fall code */

  while ((a = GET_ALIASES(ch)) != NULL) {
    GET_ALIASES(ch) = (GET_ALIASES(ch))->next;
    free_alias(a);
  }

  if (SEEK_TARGETSTR(ch)!=NULL) free(SEEK_TARGETSTR(ch));

/* Imhotep: it's a bad idea to free the player_specials structure
 * before freeing the aliases, given the aliases live within the
 * player specials .. */

  if (ch->player_specials != NULL && ch->player_specials != &dummy_mob) {
    if (ch->player_specials->poofin)
      free(ch->player_specials->poofin);
    if (ch->player_specials->poofout)
      free(ch->player_specials->poofout);
    free(ch->player_specials);
    if( IS_NPC(ch) && !IS_CLONE(ch) )
      mlog("SYSERR: Mob had player_specials allocated!");
  }

  free(ch);
}




/* release memory allocated for an obj struct */
void free_obj(ObjData * obj)
{
  int nr;
  struct extra_descr_data *this, *next_one;

  if ((nr = GET_OBJ_RNUM(obj)) == -1) {
    if (obj->name)
      free(obj->name);
    if (obj->description)
      free(obj->description);
    if (obj->short_description)
      free(obj->short_description);
    if (obj->action_description)
      free(obj->action_description);
    if (obj->ex_description)
      for (this = obj->ex_description; this; this = next_one) {
	next_one = this->next;
	if (this->keyword)
	  free(this->keyword);
	if (this->description)
	  free(this->description);
	free(this);
      }
  } else {
    if (obj->name && obj->name != obj_proto[nr].name)
      free(obj->name);
    if (obj->description && obj->description != obj_proto[nr].description)
      free(obj->description);
    if (obj->short_description && obj->short_description != obj_proto[nr].short_description)
      free(obj->short_description);
    if (obj->action_description && obj->action_description != obj_proto[nr].action_description)
      free(obj->action_description);
    if (obj->ex_description && obj->ex_description != obj_proto[nr].ex_description)
      for (this = obj->ex_description; this; this = next_one) {
	next_one = this->next;
	if (this->keyword)
	  free(this->keyword);
	if (this->description)
	  free(this->description);
	free(this);
      }
  }

  free(obj);
}



/* read contets of a text file, alloc space, point buf to it */
int file_to_string_alloc(char *name, char **buf)
{
  char temp[MAX_STRING_LENGTH];

  if (file_to_string(name, temp) < 0)
    return -1;

  if (*buf)
    free(*buf);

  *buf = str_dup(temp);

  return 0;
}



/* read contents of a text file, and place in buf */
int file_to_string(char *name, char *buf)
{
  FILE *fl;
  char tmp[128];

  *buf = '\0';

  if (!(fl = fopen(name, "r"))) {
    sprintf(tmp, "Error reading %s", name);
    perror(tmp);
    return (-1);
  }
  do {
    fgets(tmp, 128, fl);
    tmp[strlen(tmp) - 1] = '\0';/* take off the trailing \n */
    strcat(tmp, "\r\n");

    if (!feof(fl)) {
      if (strlen(buf) + strlen(tmp) + 1 > MAX_STRING_LENGTH) {
	mlog("SYSERR: fl->strng: string too big (db.c, file_to_string)");
	*buf = '\0';
	return (-1);
      }
      strcat(buf, tmp);
    }
  } while (!feof(fl));

  fclose(fl);

  return (0);
}




/* clear some of the the working variables of a char */
void
reset_char( CharData * ch )
{
  int i;

  for (i = 0; i < NUM_WEARS; i++)
    ch->equipment[i] = NULL;

/*ch->in_room   = NOWHERE; */
  ch->followers = NULL;
  ch->master    = NULL;
  ch->carrying  = NULL;
  ch->next      = NULL;
  ch->next_fighting = NULL;
  ch->next_in_room  = NULL;
  FIGHTING(ch)      = NULL;
  ch->char_specials.position   = POS_STANDING;
  ch->mob_specials.default_pos = POS_STANDING;
  ch->mob_specials.load_pos    = POS_STANDING; /* I think this will be ok.. */
  ch->char_specials.carry_weight = 0;
  ch->char_specials.carry_items  = 0;

  if( GET_HIT(ch)  <= 0 ) GET_HIT(ch) = GET_MAX_HIT(ch)/4;
  if( GET_MOVE(ch) <= 0 ) GET_MOVE(ch) = GET_MAX_MOVE(ch)/4;
  if( GET_MANA(ch) <= 0 ) GET_MANA(ch) = GET_MAX_MANA(ch)/4;

  GET_LAST_TELL(ch) = NOBODY;
}



/*
** Clear ALL of the working variables of a char; do NOT free
** any space alloc'ed
*/
void
clear_char(CharData * ch)
{
  memset((char *) ch, 0, sizeof(CharData));

  ch->in_room    = NOWHERE;
  ch->followers  = NULL;
  ch->master     = NULL;
  ch->singing    = 0;
  ch->target = NULL;
  ch->offbalance = 0;
  GET_VOTE(ch)		 = 0;
  ch->flee_timer = 0;
  GET_WAS_IN(ch) = NOWHERE;
  GET_POS(ch)    = POS_STANDING;
  ch->mob_specials.default_pos = POS_STANDING;
  ch->mob_specials.load_pos = POS_STANDING;
  WAIT_STATE(ch, 0);

  GET_AC(ch) = 100;		/* Basic Armor */
  if (ch->points.max_mana < 100)
    ch->points.max_mana = 100;
}


void clear_object(ObjData * obj)
{
  memset((char *) obj, 0, sizeof(ObjData));

  obj->item_number = NOTHING;
  obj->in_room     = NOWHERE;
  obj->worn_at     = NOWHERE;
}




/* initialize a new character only if class is set */
void
init_char( CharData* ch )
{
  int i, taeller;
  char poofin_buf[80]  = "";
  char poofout_buf[80] = "";

  /* create a player_special structure */
  if( ch->player_specials == NULL )
  {
    CREATE(ch->player_specials, struct player_special_data, 1);
    ch->player_specials->pardons = NULL;
  }

  /* *** if this is our first player --- he be God *** */

  if( top_of_p_table == 0 )
  {
    GET_EXP(ch)   = 7000000;
    GET_LEVEL(ch) = LVL_IMPL;

    ch->points.max_hit  = 500;
    ch->points.max_mana = 100;
    ch->points.max_move = 82;
  }

  set_title( ch, NULL );

  ch->player.short_descr = NULL;
  ch->player.long_descr  = NULL;
  ch->player.description = NULL;

  ch->player.lostlevels = 0;
  ch->player.orcs = 0;

  ch->player.time.played = 0;
  ch->player.time.logon = time(0);

  sprintf(poofin_buf,  "%s arrives in a puff of smoke.", GET_NAME(ch));
  sprintf(poofout_buf, "%s departs in a puff of smoke.", GET_NAME(ch));

  POOFIN(ch)  = str_dup(poofin_buf);
  POOFOUT(ch) = str_dup(poofout_buf);

  /* make favors for sex */
  if( ch->player.sex == SEX_MALE )
  {
    ch->player.weight = number(120, 180);
    ch->player.height = number(160, 200);
  }
  else
  {
    ch->player.weight = number(100, 160);
    ch->player.height = number(150, 180);
  }

  ch->points.max_mana = 100;
  ch->points.mana     = GET_MAX_MANA(ch);
  ch->points.hit      = GET_MAX_HIT(ch);
  ch->points.max_move = 82;
  ch->points.move     = GET_MAX_MOVE(ch);
  ch->points.armor    = 100;

  player_table[top_of_p_table].id = GET_IDNUM(ch) = ++top_idnum;

  for( i = 1; i <= MAX_SKILLS; i++)
  {
    if( GET_LEVEL(ch) < LVL_IMPL )
      SET_SKILL(ch, i, 0)
    else
      SET_SKILL(ch, i, 100);
  }

  // ch->char_specials.saved.affected_by = 0;
  //
  for( taeller=0; taeller < AF_ARRAY_MAX; taeller++ )
  {
    ch->char_specials.saved.affected_by[taeller] = 0;
  }

  for( i = 0; i < 5; i++ )
  {
    GET_SAVE(ch, i) = 0;
  }

  for( i = 0; i < 3; i++ )
  {
    GET_COND(ch, i) = (GET_LEVEL(ch) == LVL_IMPL ? -1 : 24);
  }
}

/*
** returns the real number of the room with given virtual number
*/
int real_room(int virtual)
{
  int bot, top, mid;

  bot = 0;
  top = top_of_world;

  /* perform binary search on world-table */
  for (;;) {
    mid = (bot + top) >> 1;

    if ((world + mid)->number == virtual)
      return mid;
    if (bot >= top)
      return -1;
    if ((world + mid)->number > virtual)
      top = mid - 1;
    else
      bot = mid + 1;
  }
}

/* Vex. Returns the real number of the zone with the given vnum */
int real_zone(int virtual)
{
  int bot, top, mid;

  bot = 0;
  top = top_of_zone_table;

  /* perform binary search on zone-table */
  for (;;) {
    mid = (bot + top) / 2;

    if ((zone_table + mid)->number == virtual)
      return (mid);
    if (bot >= top)
      return (-1);
    if ((zone_table + mid)->number > virtual)
      top = mid - 1;
    else
      bot = mid + 1;
  }
}

/* returns the real number of the monster with given virtual number */
int real_mobile(int virtual)
{
  int bot, top, mid;

  bot = 0;
  top = top_of_mobt;

  /* perform binary search on mob-table */
  for (;;) {
    mid = (bot + top) / 2;

    if ((mob_index + mid)->virtual == virtual)
      return (mid);
    if (bot >= top)
      return (-1);
    if ((mob_index + mid)->virtual > virtual)
      top = mid - 1;
    else
      bot = mid + 1;
  }
}



/* returns the real number of the object with given virtual number */
int real_object(int virtual)
{
  int bot, top, mid;

  bot = 0;
  top = top_of_objt;

  /* perform binary search on obj-table */
  for (;;) {
    mid = (bot + top) / 2;

    if ((obj_index + mid)->virtual == virtual)
      return (mid);
    if (bot >= top)
      return (-1);
    if ((obj_index + mid)->virtual > virtual)
      top = mid - 1;
    else
      bot = mid + 1;
  }
}

/* returns the real number of the quest with given virtual number */
int real_quest(int virtual)
{
  int bot, top, mid;

  bot = 0;
  top = top_of_qstt;

  /* perform binary search on obj-table */
  for (;;) {
    mid = (bot + top) / 2;

    if ((qst_list + mid)->virtual == virtual)
      return (mid);
    if (bot >= top)
      return (-1);
    if ((qst_list + mid)->virtual > virtual)
      top = mid - 1;
    else
      bot = mid + 1;
  }
}

void
load_randoms( FILE *fl )
{
  static int   rand = 0;
  int  rand_no = 0, expand, tmp;
  char *check, buf[81];

  for(;;)
  {
    fscanf( fl, " #%d\n", &tmp );
    check = fread_string( fl, buf2 );

    if( *check == '$' ) break; /* EOF */

    rand_table[rand].number = tmp;
    rand_table[rand].name = check;
    /*
    ** read the random table
    */
    rand_no = 0;

    for( expand = 1; ; )
    {
      if( expand ) {
        if( !rand_no )
          CREATE( rand_table[rand].item, RandItem, 1 );
        else if( !(rand_table[rand].item =
          (RandItem*) realloc( rand_table[rand].item,
                              (rand_no+1)*sizeof(RandItem))))
        {
          perror("reset command load random");
          exit(0);
        }
	}
        expand = 1;

        fscanf( fl, " " ); /* skip blanks */
        fscanf( fl, "%d", &rand_table[rand].item[rand_no].virtual );

        if (rand_table[rand].item[rand_no].virtual == -1) { break; }

        fscanf(fl, " %d %d",
          &rand_table[rand].item[rand_no].max,
          &rand_table[rand].item[rand_no].min_val);

        rand_table[rand].item[rand_no].number = 0;
        fgets(buf, 80, fl);    /* read comment */
        rand_no++;
   }
        rand_table[rand++].total = rand_no;
  }
   top_of_rand_table = rand - 1;
   free(check);
}

ObjData *random_object( int table, int min_pos )
{
	int num;

	num = number(0, rand_table[table].total -1);
	if (rand_table[table].item[num].min_val <= min_pos)
		return(read_object(rand_table[table].item[num].virtual, VIRTUAL));
	else
		return (0);
}

void save_renames(void)
{
    FILE *f = fopen(RENAMES_FILE, "wb");

    if (!f) {
        mlog("Failed to create renames file!");
        return;
    }
    fwrite(renames, sizeof(Rename), top_of_rename_t, f);
    fclose(f);
}

void load_renames(void)
{
    FILE *f = fopen(RENAMES_FILE, "rb");

    if (!f) {
        mlog("Failed to open renames file!");
        return;
    }
    fseek(f, 0, SEEK_END);
    top_of_rename_t = ftell(f) / sizeof(Rename);
    rewind(f);
    fread(renames, sizeof(Rename), top_of_rename_t, f);
    fclose(f);
}

  /* This function is called only once, at boot-time. We assume config_info is
   * empty. All customizations for this found in mudconfig.c */
static void load_default_config( void )
{
  /* Game play options. */
  CONFIG_PK_ALLOWED 	        = pk_allowed;
  CONFIG_PT_ALLOWED             = pt_allowed;
  CONFIG_LEVEL_CAN_SHOUT 	= level_can_shout;
  CONFIG_HOLLER_MOVE_COST 	= holler_move_cost;
  CONFIG_TUNNEL_SIZE 	        = tunnel_size;
  CONFIG_MAX_EXP_GAIN	        = max_exp_gain;
  CONFIG_MAX_EXP_LOSS 	        = max_exp_loss;
  CONFIG_MAX_NPC_CORPSE_TIME    = max_npc_corpse_time;
  CONFIG_MAX_PC_CORPSE_TIME	= max_pc_corpse_time;
  CONFIG_IDLE_VOID		= idle_void;
  CONFIG_IDLE_RENT_TIME	        = idle_rent_time;
  CONFIG_IDLE_MAX_LEVEL	        = idle_max_level;
  CONFIG_DTS_ARE_DUMPS	        = dts_are_dumps;
  CONFIG_LOAD_INVENTORY         = load_into_inventory;
  CONFIG_OK			= strdup(OK);
  CONFIG_NOPERSON		= strdup(NOPERSON);
  CONFIG_NOEFFECT		= strdup(NOEFFECT);
  CONFIG_TRACK_T_DOORS          = track_through_doors;
  CONFIG_NO_MORT_TO_IMMORT	= no_mort_to_immort;
  CONFIG_DISP_CLOSED_DOORS      = display_closed_doors;
  CONFIG_SCRIPT_PLAYERS         = script_players;
  CONFIG_PLAGUE_IS_CONTAGIOUS   = plague_is_contagious;
  CONFIG_QUEST_ACTIVE           = quest_active;


  /* Rent / crashsave options. */
  CONFIG_FREE_RENT              = free_rent;
  CONFIG_MAX_OBJ_SAVE           = max_obj_save;
  CONFIG_MIN_RENT_COST	        = min_rent_cost;
  CONFIG_AUTO_SAVE		= auto_save;
  CONFIG_AUTOSAVE_TIME	        = autosave_time;
  CONFIG_CRASH_TIMEOUT          = crash_file_timeout;
  CONFIG_RENT_TIMEOUT	        = rent_file_timeout;

  /* Room numbers. */
  CONFIG_MORTAL_START           = mortal_start_room;
  CONFIG_IMMORTAL_START         = immort_start_room;
  CONFIG_FROZEN_START           = frozen_start_room;
  CONFIG_DON_ROOM_1             = donation_room_1;
  CONFIG_DON_ROOM_2             = donation_room_2;
  CONFIG_DON_ROOM_3             = donation_room_3;

  /* Game operation options. */
  CONFIG_DFLT_PORT              = DFLT_PORT;

  if (DFLT_IP)
    CONFIG_DFLT_IP              = strdup(DFLT_IP);
  else
    CONFIG_DFLT_IP              = NULL;

  CONFIG_DFLT_DIR               = strdup(DFLT_DIR);

  if (LOGNAME)
    CONFIG_LOGNAME              = strdup(LOGNAME);
  else
    CONFIG_LOGNAME              = NULL;

  CONFIG_MAX_PLAYING            = max_playing;
  CONFIG_MAX_FILESIZE           = max_filesize;
  CONFIG_MAX_BAD_PWS            = max_bad_pws;
  CONFIG_SITEOK_ALL             = siteok_everyone;
  CONFIG_NS_IS_SLOW             = nameserver_is_slow;
  /* Socials below NOT used for future expansion. */
  CONFIG_NEW_SOCIALS            = use_new_socials;
  CONFIG_OLC_SAVE               = auto_save_olc;
  CONFIG_MENU                   = strdup(MENU);
  CONFIG_GREETINGS              = strdup(GREETINGS);
  CONFIG_WELC_MESSG             = strdup(WELC_MESSG);
  CONFIG_START_MESSG            = strdup(START_MESSG);

  /* Autowiz options. */
  CONFIG_USE_AUTOWIZ            = use_autowiz;
  CONFIG_MIN_WIZLIST_LEV        = min_wizlist_lev;
}

void load_config( void )
{
  FILE *fl;
  char line[MAX_STRING_LENGTH];
  char tag[MAX_INPUT_LENGTH];
  int  num;
  char buf[MAX_INPUT_LENGTH];

  load_default_config();

  snprintf(buf, sizeof(buf), "%s/%s", CONFIG_DFLT_DIR, CONFIG_CONFFILE);
  if ( !(fl = fopen(CONFIG_CONFFILE, "r")) && !(fl = fopen(buf, "r")) ) {
    snprintf(buf, sizeof(buf), "No %s file, using defaults", CONFIG_CONFFILE);
    perror(buf);
    return;
  }

  /* Load the game configuration file. */
  while (get_line(fl, line)) {
    split_argument(line, tag);
    num = atoi(line);

    switch (LOWER(*tag)) {
      case 'a':
        if (!str_cmp(tag, "auto_save"))
          CONFIG_AUTO_SAVE = num;
        else if (!str_cmp(tag, "autosave_time"))
          CONFIG_AUTOSAVE_TIME = num;
        else if (!str_cmp(tag, "auto_save_olc"))
          CONFIG_OLC_SAVE = num;
        break;

      case 'c':
        if (!str_cmp(tag, "crash_file_timeout"))
          CONFIG_CRASH_TIMEOUT = num;
        break;

      case 'd':
        if (!str_cmp(tag, "display_closed_doors"))
          CONFIG_DISP_CLOSED_DOORS = num;
        else if (!str_cmp(tag, "dts_are_dumps"))
          CONFIG_DTS_ARE_DUMPS = num;
        else if (!str_cmp(tag, "donation_room_1"))
          if (num == -1)
            CONFIG_DON_ROOM_1 = NOWHERE;
          else
            CONFIG_DON_ROOM_1 = num;
        else if (!str_cmp(tag, "donation_room_2"))
          if (num == -1)
            CONFIG_DON_ROOM_2 = NOWHERE;
          else
            CONFIG_DON_ROOM_2 = num;
        else if (!str_cmp(tag, "donation_room_3"))
          if (num == -1)
            CONFIG_DON_ROOM_3 = NOWHERE;
          else
            CONFIG_DON_ROOM_3 = num;
        else if (!str_cmp(tag, "dflt_dir")) {
          if (CONFIG_DFLT_DIR)
            free(CONFIG_DFLT_DIR);
          if (line != NULL && *line)
            CONFIG_DFLT_DIR = strdup(line);
          else
            CONFIG_DFLT_DIR = strdup(DFLT_DIR);
        } else if (!str_cmp(tag, "dflt_ip")) {
          if (CONFIG_DFLT_IP)
            free(CONFIG_DFLT_IP);
          if (line != NULL && *line)
            CONFIG_DFLT_IP = strdup(line);
          else
            CONFIG_DFLT_IP = NULL;
        } else if (!str_cmp(tag, "dflt_port"))
          CONFIG_DFLT_PORT = num;
        break;

      case 'f':
        if (!str_cmp(tag, "free_rent"))
          CONFIG_FREE_RENT = num;
        else if (!str_cmp(tag, "frozen_start_room"))
          CONFIG_FROZEN_START = num;
        break;

      case 'g':
        if (!str_cmp(tag, "greetings")) {
          strncpy(buf, "Reading greetings message in load_config()", sizeof(buf));
          if (CONFIG_GREETINGS)
            free(CONFIG_GREETINGS);
          CONFIG_GREETINGS= fread_string(fl, buf);
        }
        break;

      case 'h':
        if (!str_cmp(tag, "holler_move_cost"))
          CONFIG_HOLLER_MOVE_COST = num;
        break;

      case 'i':
        if (!str_cmp(tag, "idle_void"))
          CONFIG_IDLE_VOID = num;
        else if (!str_cmp(tag, "idle_rent_time"))
          CONFIG_IDLE_RENT_TIME = num;
        else if (!str_cmp(tag, "idle_max_level"))
          CONFIG_IDLE_MAX_LEVEL = num;
        else if (!str_cmp(tag, "no_mort_to_immort"))
          CONFIG_NO_MORT_TO_IMMORT = num;
        else if (!str_cmp(tag, "immort_start_room"))
          CONFIG_IMMORTAL_START = num;
        break;

      case 'l':
        if (!str_cmp(tag, "level_can_shout"))
          CONFIG_LEVEL_CAN_SHOUT = num;
        else if (!str_cmp(tag, "load_into_inventory"))
          CONFIG_LOAD_INVENTORY = num;
        else if (!str_cmp(tag, "logname")) {
          if (CONFIG_LOGNAME)
            free(CONFIG_LOGNAME);
          if (line != NULL && *line)
            CONFIG_LOGNAME = strdup(line);
          else
            CONFIG_LOGNAME = NULL;
        }
        break;

      case 'm':
        if (!str_cmp(tag, "max_bad_pws"))
          CONFIG_MAX_BAD_PWS = num;
        else if (!str_cmp(tag, "max_exp_gain"))
          CONFIG_MAX_EXP_GAIN = num;
        else if (!str_cmp(tag, "max_exp_loss"))
          CONFIG_MAX_EXP_LOSS = num;
        else if (!str_cmp(tag, "max_filesize"))
          CONFIG_MAX_FILESIZE = num;
        else if (!str_cmp(tag, "max_npc_corpse_time"))
          CONFIG_MAX_NPC_CORPSE_TIME = num;
        else if (!str_cmp(tag, "max_obj_save"))
          CONFIG_MAX_OBJ_SAVE = num;
        else if (!str_cmp(tag, "max_pc_corpse_time"))
          CONFIG_MAX_PC_CORPSE_TIME = num;
        else if (!str_cmp(tag, "max_playing"))
          CONFIG_MAX_PLAYING = num;
        else if (!str_cmp(tag, "menu")) {
          if (CONFIG_MENU)
            free(CONFIG_MENU);
          strncpy(buf, "Reading menu in load_config()", sizeof(buf));
          CONFIG_MENU = fread_string(fl, buf);
        } else if (!str_cmp(tag, "min_rent_cost"))
          CONFIG_MIN_RENT_COST = num;
        else if (!str_cmp(tag, "min_wizlist_lev"))
          CONFIG_MIN_WIZLIST_LEV = num;
        else if (!str_cmp(tag, "mortal_start_room"))
          CONFIG_MORTAL_START = num;
        break;

      case 'n':
        if (!str_cmp(tag, "nameserver_is_slow"))
          CONFIG_NS_IS_SLOW = num;
        else if (!str_cmp(tag, "noperson")) {
          char tmp[READ_SIZE];
          if (CONFIG_NOPERSON)
            free(CONFIG_NOPERSON);
          snprintf(tmp, sizeof(tmp), "%s\r\n", line);
          CONFIG_NOPERSON = strdup(tmp);
        } else if (!str_cmp(tag, "noeffect")) {
          char tmp[READ_SIZE];
          if (CONFIG_NOEFFECT)
            free(CONFIG_NOEFFECT);
          snprintf(tmp, sizeof(tmp), "%s\r\n", line);
          CONFIG_NOEFFECT = strdup(tmp);
        }
        break;

      case 'o':
        if (!str_cmp(tag, "ok")) {
          char tmp[READ_SIZE];
          if (CONFIG_OK)
            free(CONFIG_OK);
          snprintf(tmp, sizeof(tmp), "%s\r\n", line);
          CONFIG_OK = strdup(tmp);
        }
        break;

      case 'p':
        if (!str_cmp(tag, "pk_allowed"))
          CONFIG_PK_ALLOWED = num;
       else if (!str_cmp(tag, "plague_is_contagious"))
          CONFIG_PLAGUE_IS_CONTAGIOUS = num;
        else if (!str_cmp(tag, "pt_allowed"))
          CONFIG_PT_ALLOWED = num;
        break;

     case 'q':
        if (!str_cmp(tag, "quest_active"))
          CONFIG_QUEST_ACTIVE = num;
        break;

      case 'r':
        if (!str_cmp(tag, "rent_file_timeout"))
          CONFIG_RENT_TIMEOUT = num;
        break;

      case 's':
        if (!str_cmp(tag, "siteok_everyone"))
          CONFIG_SITEOK_ALL = num;
        else if (!str_cmp(tag, "script_players"))
          CONFIG_SCRIPT_PLAYERS = num;
        else if (!str_cmp(tag, "start_messg")) {
          strncpy(buf, "Reading start message in load_config()", sizeof(buf));
          if (CONFIG_START_MESSG)
            free(CONFIG_START_MESSG);
          CONFIG_START_MESSG = fread_string(fl, buf);
        }
        break;

      case 't':
        if (!str_cmp(tag, "tunnel_size"))
          CONFIG_TUNNEL_SIZE = num;
        else if (!str_cmp(tag, "track_through_doors"))
          CONFIG_TRACK_T_DOORS = num;
        break;

      case 'u':
        if (!str_cmp(tag, "use_autowiz"))
          CONFIG_USE_AUTOWIZ = num;
        else if (!str_cmp(tag, "use_new_socials"))
          CONFIG_NEW_SOCIALS = num;
        break;

      case 'w':
        if (!str_cmp(tag, "welc_messg")) {
          strncpy(buf, "Reading welcome message in load_config()", sizeof(buf));
          if (CONFIG_WELC_MESSG)
            free(CONFIG_WELC_MESSG);
          CONFIG_WELC_MESSG = fread_string(fl, buf);
        }
        break;

      default:
        break;
    }
  }

  fclose(fl);
}
