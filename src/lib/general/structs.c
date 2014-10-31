/* ************************************************************************
*  File: structs.c                                      Part of  RavenMUD *
*  Author: Vex of RavenMUD						  *
*  Usage: Numeric and string contants used by the MUD                     *
*                                                                         *
*  RavenMUD is derived from CircleMUD, so the CircleMUD license applies.  *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"

/* RavenMUD version */
cpp_extern const char *circlemud_version = "RavenMUD, Version 2.3";

/* strings corresponding to ordinals/bitvectors in structs.h ***********/

/* cardinal directions */
const char *dirs[] =
{
  "north",
  "east",
  "south",
  "west",
  "up",
  "down",
  "\n"
};

const int rev_dir[NUM_OF_DIRS] = { 2, 3, 0, 1, 5, 4 };

/* ROOM_x */
char *none_room_bits[NUM_ROOM_FLAGS + 2] = {
  "NONE",
  "DARK",
  "DEATH",
  "!MOB",
  "INDOORS",
  "PEACEFUL",
  "SOUNDPROOF",
  "!TRACK",
  "!MAGIC",
  "TUNNEL",
  "PRIVATE",
  "GODROOM",
  "HOUSE",
  "HCRSH",
  "ATRIUM",
  "OLC",
  "BFS_MRK",				/* BFS MARK */
  "FALL",
  "MANA_REGEN",
  "HP_REGEN",
  "SUFFER",
  "CLAN_ROOM",
  "!RECALL",
  "FOG",
  "ONE_PERSON",
  "!DISASTER",
  "DISASTER_FIREBALL",
  "DISASTER_LIGHTNING",
  "DISASTER_EARTHQUAKE",
  "DISASTER_WIND",
  "SMALL_RACE",
  "HOT",
  "COLD",
  "DRY",
  "FAMILIAR",
  "!RELOCATE",
  "SALT_FISH",
  "FRESH_FISH",
  "DISRUPTIVE",
  "POISONED",
  "\n"
};

char **room_bits = none_room_bits + 1;

/* SECT_ */
const char *sector_types[] = {
  "Inside",
  "City",
  "Field",
  "Forest",
  "Hills",
  "Mountains",
  "Water (Swim)",
  "Water (No Swim)",
  "Underwater",
  "In Flight",
  "Underwater River",
  "Corpse Room",
  "Road",
  "Plain",
  "Rocky",
  "Muddy",
  "Sand",
  "Light Forest",
  "Thick Forest",
  "\n"
};

  char *player_bits[NUM_PLAYER_FLAGS + 1] = {
    "KILLER", "THIEF", "FROZEN", "DONTSET", "WRITING",
    "MAILING", "CSH", "SITEOK", "NOSHOUT", "NOTITLE",
    "DELETED", "LOADRM", "!WIZL", "!DEL", "INVST",
    "CRYO", "SHUNNED", "BUILDING", "RMC", "HUNTED",
    "ATTACKER", "JAILED", "R", "REIMBURSED",
    "CNJ_TMR", "UNUSED (was NORECALL)", "RETIRED", "NOCOMM", "UNUSED (was POULTICED)",
    "UBERCHEAT", "GRIEF", "FISHING", "FISH_ON", "\n"
  };

  char *preference_bits[NUM_PRF_FLAGS + 1] = {
    "BRIEF",  "COMPACT", "DEAF",    "!TELL",    "D_HP",
    "D_MANA", "D_MOVE",  "AUTOEX",  "!HASS",    "QUEST",
    "SUMN",   "!REP",    "LIGHT",   "C1",       "C2",
    "!WIZ",   "L1",      "L2",      "!AUC",     "!RPLAY",
    "!GTZ",   "RMFLG",   "!ANON",   "CONSENT",  "!SPAM",
    "D_EXP",  "OLCV",    "GOLD_TM", "BLACK_TM", "!OOC",
    "!QUERY", "!GUILD", "AUTOSPLIT","AUTOLOOT", "ROGUE_TM",
    "RED_TM", "!CLAN",   "AUTOGOLD","PRF_NORECALL", "TANK_HP",
    "PRF_NOWHO", "SHOWDAM", "CLS", "NOWIZ", "AFK", "\n"
  };

/* SEX_x */
char *genders[NUM_GENDERS + 1] =
{
  "Neutral",
  "Male",
  "Female",
  "\n"
};

/* POS_x */
const char *position_types[] = {
  "Dead",
  "Mortally wounded",
  "Incapacitated",
  "Stunned",
  "Sleeping",
  "Meditating",
  "Resting",
  "Sitting",
  "Fighting",
  "Standing",
  "\n"
};

/* MOB_x */
char *action_bits[NUM_MOB_FLAGS + 1] = {
  "SPEC",
  "SENTINEL",
  "SCVNGR",
  "ISNPC",
  "AWARE",
  "AGGR",
  "STAYZ",
  "WIMP",
  "AGG_E",
  "AGG_G",
  "AGG_N",
  "SEEKR",
  "HELPER",
  "!CHRM",
  "!SUMN",
  "!SLEP",
  "!BASH",
  "!BLND",
  "CNJRD",
  "MEMORY",
  "SUPAGG",
  "MOUNT",
  "CLONE",
  "PREDATOR",
  "GUARD_CLASS",
  "GUARD_RACE",
  "GUARD_BOTH",
  "QUESTMASTER",
  "TELEPORTS",
  "NPC_KILLER",
  "!NOX",
  "GRENADER",
  "EVASIVE",
  "\n"
};

/* AFF_x */
char *affected_bits[] =
{
  "DONTSET",
  "INVIS",
  "DET-ALIGN",
  "DET-INVIS",
  "DET-MAGIC",
  "SENSE-LIFE",
  "SHAWALK",
  "SANCT",
  "GROUP",
  "CURSE",
  "INFRA",
  "POISON",
  "PROT-EVIL",
  "PROT-GOOD",
  "SLEEP",
  "!TRACK",
  "FLY",
  "REGEN",
  "SNEAK",
  "HIDE",
  "HASTE",
  "CHARM",
  "SHIELD",
  "PARA",
  "AIRSPHERE",
  "PLAGUE",
  "DONTSET",
  "SILENCE",
  "MOUNTED",
  "WARD",
  "SHAD_SPH",
  "BERSERK",
  "!HOT",
  "!COLD",
  "!DRY",
  "BLIND",
  "WEBBED",
  "BLINK",
  "FEEBLEMIND",
  "DONTSET",
  "DONTSET",
  "HAMSTRUNG",
  "PULSE-HIT",
  "PULSE-MANA",
  "DISTRACT",
  "CRUSADE",
  "APOCALYPSE",
  "DIVINE-MISSION",
  "DONTSET",
  "FOREST-LORE",
  "LEARNING",
  "FLAMING",
  "UNUSED",
  "WRAITHFORM",
  "DISEASE",
  "\n"
};

char *zone_bits[] = {
  "OPEN",
  "CLOSED",
  "!RECALL",
  "!SUMMON",
  "!PORTAL",
  "!RELOCATE",
  "!MORTAL",
  "REMORTS",
  "LIGHTNING",
  "FIREBALL",
  "WIND",
  "EARTHQUAKE",
  "LAVA",
  "FLOOD",
  "PEACEFUL",
  "SLEEP_TAG",
  "ARENA"
};

/** Connection type descriptions.
 * @pre Must be in the same order as the defines.
 * Must end array with a single newline. */
const char *connected_types[] = {
  "Playing",  "Disconn",  "GetName",
  "ConfName", "GetPW",    "GetNewPW",
  "ConfPW",   "SelSex",   "SelClass",
  "SelRace",  "ReadMOTD", "MainMenu",
  "GetDscrp", "ChgPW1",   "ChgPW2",
  "ChgPW3",   "SelfDel1", "SelfDel2",
  "ShowPlrs", "GenAbils", "SelBrth",
  "ObjEdit",  "RoomEdit", "ZoneEdit",
  "MobEdit",  "ShopEdit", "TextEdit",
  "TrigEdit", "QuestEdit", "ConfEdit",
  "HelpEdit", "\n"
};

/** Describes the position in the equipment listing.
 * @pre Must be in the same order as the defines.
 * Not used in sprinttype() so no \n. */
const char *wear_where[] = {
  "<used as light>      ",
  "<worn on finger>     ",
  "<worn on finger>     ",
  "<worn as cloak>      ",
  "<worn around neck>   ",
  "<worn on body>       ",
  "<worn on head>       ",
  "<worn on ears>       ",
  "<worn on face>       ",
  "<worn on legs>       ",
  "<worn on ankles>     ",
  "<worn on feet>       ",
  "<worn on hands>      ",
  "<worn on arms>       ",
  "<worn as shield>     ",
  "<worn about body>    ",
  "<worn about waist>   ",
  "<worn around wrist>  ",
  "<worn around wrist>  ",
  "<wielded>            ",
  "<held>               ",
  "<orbitting head>     ",
};

/* WEAR_x - for stat */
const char *equipment_types[] = {
  "Used as light",
  "Worn on right finger",
  "Worn on left finger",
  "Worn as cloak",
  "Worn around Neck",
  "Worn on body",
  "Worn on head",
  "Worn on ears",
  "Worn on face",
  "Worn on legs",
  "Worn on ankles",
  "Worn on feet",
  "Worn on hands",
  "Worn on arms",
  "Worn as shield",
  "Worn about body",
  "Worn around waist",
  "Worn around right wrist",
  "Worn around left wrist",
  "Wielded",
  "Held",
  "Orbitting Head",
  "\n"
};

/* ITEM_x (ordinal object types) */
const char *item_types[] = {
  "UNDEFINED",
  "LIGHT",
  "SCROLL",
  "WAND",
  "STAFF",
  "WEAPON",
  "FIRE WEAPON",
  "MISSILE",
  "TREASURE",
  "ARMOR",
  "POTION",
  "WORN",
  "OTHER",
  "TRASH",
  "UNUSED",
  "CONTAINER",
  "NOTE",
  "LIQ CONTAINER",
  "KEY",
  "FOOD",
  "MONEY",
  "PEN",
  "BOAT",
  "FOUNTAIN",
  "PORTAL",
  "SCRIBE",
  "AFFECT",
  "DUST",
  "POLE",
  "\n"
};

/* ITEM_WEAR_ (wear bitvector) */
char *wear_bits[NUM_ITEM_WEARS + 1] = {
  "TAKE",
  "FINGER",
  "CLOAK",
  "BODY",
  "HEAD",
  "LEGS",
  "FEET",
  "HANDS",
  "ARMS",
  "SHIELD",
  "ABOUT",
  "WAIST",
  "WRIST",
  "WIELD",
  "HOLD",
  "NECK",
  "ORBIT",
  "ANKLES",
  "EARS",
  "FACE",
  "\n"
};

/* ITEM_x (extra bits) */
char *extra_bits[NUM_ITEM_FLAGS + 1] = {
  "GLOW",        /*  0 */
  "HUM",
  "!RENT",
  "!DONATE",
  "!INVIS",
  "INVISIBLE",   /*  5 */
  "MAGIC",
  "CURSED",
  "BLESS",
  "!GOOD",
  "!EVIL",        /* 10 */
  "!NEUTRAL",
  "!MAGE",
  "!CLERIC",
  "!THIEF",
  "!WARRIOR",     /* 15 */
  "!SELL",
  "!RANGER",
  "!ASSASSIN",
  "!SHOU_LIN",
  "!SOLAMN_KN",   /* 20 */
  "!DEATH_KN",
  "!SHAD_DANC",
  "TIMED",
  "EXPLODES",
  "ARTIFACT",   /* 25 */
  "NO_LOCATE",
  "!MINOTAUR",
  "!GNOME",
  "!ORC",
  "!ELF",	/* 30 */
  "!DRACONIAN",
  "!HALFLING",
  "!OGRE",
  "!TROLL",
  "!DWARF",	/* 35 */
  "!HUMAN",
  "!NECRO",
  "MAIN_PART",
  "INSERT_PART",
  "ARENA",     /* 40 */
  "!DEMON",
  "!IZARTI",
  "!VAMPIRE",
  "!WEREWOLF",
  "!ELEMENTAL", /* 45 */
  "!GIANT",
  "!FAERIE",
  "!AMARA",
  "!UNDEAD",
  "!DROW",      /* 50 */
  "LOOTED",
  "IDENTIFIED",
  "RANDOMIZED",
  "SOULBOUND",
  "!REMORT",    /* 55 */
  "!PREMORT",
  "TROPHY",
  "\n"
};

/* APPLY_x */
const char *apply_types[] = {
  "NONE",
  "STR",
  "DEX",
  "INT",
  "WIS",
  "CON",
  "CHA",
  "CLASS",
  "LEVEL",
  "AGE",
  "CHAR_WEIGHT",
  "CHAR_HEIGHT",
  "MAXMANA",
  "MAXHIT",
  "MAXMOVE",
  "GOLD",
  "EXP",
  "ARMOR",
  "HITROLL",
  "DAMROLL",
  "SAVING_PARA",
  "SAVING_ROD",
  "SAVING_PETRI",
  "SAVING_BREATH",
  "SAVING_SPELL",
  "POISON",
  "PLAGUE",
  "MANA_COST",
  "SPELL_SAVES",
  "SPELL_DAMAGE",
  "SPELL_DURATION",
  "SKILL_SUCCESS",
  "SKILL_SUCCESS(DEPRECATED)",
  "USELEVEL",
  "\n"
};

/* LIQ_x */
char *drinks[NUM_LIQ_TYPES + 1] =
{
  "water",
  "beer",
  "wine",
  "ale",
  "dark ale",
  "whisky",
  "lemonade",
  "firebreather",
  "local speciality",
  "slime mold juice",
  "milk",
  "tea",
  "coffee",
  "blood",
  "salt water",
  "clear water",
  "\n"
};

/* Immortal Names */
char *god_labels[NUM_GOD_LABELS + 1] = {
      " HERO ",
      " SAINT",
      " ANGEL",
      " DEITY",
      " CREAT",
      " DEMI ",
      " LGOD ",
      " GOD  ",
      " GRGOD",
      " IMP  ",
      "\n"
};

/* STANCE_x */
char *stances[NUM_STANCES + 1] = 
{
    "neutral",
    "offensive",
    "defensive",
    "\n"
};

/* STANCE_x */
char *aspects[NUM_ASPECTS + 1] =
{
    "none",
    "wind",
    "tiger",
    "snake",
    "monkey",
    "crane",
    "flower",
    "dragon",
    "\n"
};

char *mastery_types[] =
{
    "None",
    "Edge",
    "Point",
    "Blunt",
    "Exotic",
    "\n"
};

/** Describes the color of the various drinks.
 * @pre Must be in the same order as the defines.
 * Must end array with a single newline. */
  const char *color_liquid[] = {
    "clear", "brown", "clear", "brown", "dark",
    "golden", "red", "green", "clear", "light green",
    "white", "brown", "black", "red", "clear",
    "crystal clear",
    "\n"
  };

 /** Used to describe the level of fullness of a drink container. Not used in
   * sprinttype() so no \n. */
  const char *fullness[] = {
    "less than half ",
    "about half ",
    "more than half ",
    ""
  };

/* Stuff below this point jsut here temporarily. */
/* str, int, wis, dex, con applies **************************************/


/* Strength Apply Table:
**
**    Fld1 = tohit
**    Fld2 = todam
**    Fld3 = carry_w
**    Fld4 = wield_w
*/

const StrAppType str_app[32] = {
/*    hit  dam   car  wld */
    {  -5,  -4,    0,   0 }, /* 00 */
    {  -5,  -4,    3,   1 }, /* 01 */
    {  -3,  -2,    5,   1 }, /* 02 */
    {  -3,  -1,   10,   5 }, /* 03 */
    {  -2,  -1,   25,   4 }, /* 04 */
    {  -2,  -1,   55,   5 }, /* 05 */
    {  -1,   0,   80,   6 }, /* 06 */
    {  -1,   0,   90,   7 }, /* 07 */
    {   0,   0,  100,   8 }, /* 08 */
    {   0,   0,  100,   9 }, /* 09 */
    {   0,   0,  115,  10 }, /* 10 */
    {   0,   0,  115,  11 }, /* 11 */
    {   0,   0,  140,  12 }, /* 12 */
    {   0,   0,  150,  13 }, /* 13 */
    {   0,   0,  170,  14 }, /* 14 */
    {   0,   0,  180,  15 }, /* 15 */
    {   0,   1,  195,  16 }, /* 16 */
    {   1,   1,  220,  18 }, /* 17 */
    {   1,   2,  255,  20 }, /* 18 */
    {   1,   2,  280,  22 }, /* 18/01-40 */
    {   2,   3,  305,  24 }, /* 18/41-60 */
    {   2,   4,  330,  26 }, /* 18/61-80 */
    {   2,   5,  380,  28 }, /* 18/81-99 */
    {   3,   6,  480,  30 }, /* 18/100   */

    {   3,   7,  640,  32 }, /* 19 */
    {   3,   8,  700,  34 }, /* 20 */
    {   4,   9,  810,  36 }, /* 21 */
    {   4,  10,  970,  38 }, /* 22 */
    {   5,  11, 1130,  40},  /* 23 */
    {   6,  12, 1440,  45 }, /* 24 */
    {   7,  14, 1750,  50 }, /* 25 */
    {   0,   0,    0,   0 } /* -- */
};

/* Intelligence apply index, first field mana bonus, 2nd field chance of
** 1 extra point(1-3) */
const IntAppType int_app[26] = {
{-6, 0},	/* 0 */
{-5, 0},	/* 1  */
{-4, 1},
{-4, 2},
{-3, 0},
{-2, 1},	/* 5 */
{-2, 2},
{-1, 0},
{-1, 1},
{-1, 2},
{0, 0},		/* 10 */
{0, 1},
{0, 2},
{1, 0},
{1, 1},
{1, 2},		/* 15 */
{2, 0},
{2, 2},
{3, 1},
{4, 0},
{4, 2},		/* 20 */
{5, 1},
{6, 0},
{6, 2},
{7, 1},
{8, 0}		/* 25 */
};

/* Wisdom apply index, first field bonus, 2nd field chance of extra(1-5)*/
const WisAppType wis_app[26] = {
{0, 0},		/* 0 */
{0, 0},		/* 1  */
{0, 1},
{0, 2},
{0, 3},
{0, 4},		/* 5  */
{1, 0},
{1, 1},
{1, 2},
{1, 3},
{1, 4},		/* 10 */
{2, 0},
{2, 1},
{2, 2},
{2, 3},
{2, 4},		/* 15 */
{3, 0},
{3, 1},
{3, 2},
{3, 3},
{3, 4},		/* 20 */
{4, 0},
{4, 1},
{4, 2},
{4, 3},
{5, 0}		/* 25 */
};



/* [dex] skill apply (thieves only) */
const DexSkillType dex_app_skill[26] = {
/* pp  pl  trap  sn   hi */
{-99, -99, -90, -99, -60},	/* 0  */
{-90, -90, -60, -90, -50},
{-80, -80, -40, -80, -45},
{-70, -70, -30, -70, -40},
{-60, -60, -30, -60, -35},
{-50, -50, -20, -50, -30},	/* 5  */
{-40, -40, -20, -40, -25},
{-30, -30, -15, -30, -20},
{-20, -20, -15, -20, -15},
{-15, -10, -10, -20, -10},
{-10, -5, -10, -15, -5},	/* 10 */
{-5, 0, -5, -10, 0},
{0, 0, 0, -5, 0},
{0, 0, 0, 0, 0},
{0, 0, 0, 0, 0},
{0, 0, 0, 0, 0},		/* 15 */
{0, 5, 0, 0, 0},
{5, 10, 0, 5, 5},
{10, 15, 5, 10, 10},
{15, 20, 10, 12, 12},
{20, 25, 15, 15, 15},		/* 20 */
{25, 30, 20, 18, 18},
{30, 35, 25, 20, 20},
{35, 40, 30, 23, 23},
{40, 45, 35, 25, 25},
{45, 50, 40, 30, 30}		/* 25 */
};

/* [dex] apply (all) */
const DexAppType dex_app[26] = {
  {-7, -7, 50},			/* 0  */
  {-6, -6, 45},
  {-4, -4, 40},
  {-3, -3, 35},
  {-2, -2, 30},
  {-1, -1, 25},			/* 5  */
  {0, 0, 20},
  {0, 0, 15},
  {0, 0, 10},
  {0, 0, 5},
  {0, 0, 0},			/* 10 */
  {0, 0, -5},
  {0, 0, -10},
  {0, 0, -15},
  {0, 0, -20},
  {0, 0, -25},			/* 15 */
  {1, 1, -30},
  {2, 2, -35},
  {2, 2, -40},
  {3, 3, -45},
  {3, 3, -50},			/* 20 */
  {4, 4, -55},
  {4, 4, -60},
  {4, 4, -65},
  {5, 5, -70},
  {5, 5, -75}			/* 25 */
};



/* [con] apply (all) */
const ConAppType con_app[26] = {
  {-6, 20, 0},	/* 0 */
  {-5, 25, 0},
  {-4, 30, 1},
  {-4, 35, 2},
  {-3, 40, 0},
  {-2, 45, 0},	/* 5*/
  {-2, 50, 2},
  {-1, 55, 0},
  {-1, 60, 1},
  {-1, 65, 2},
  {0, 70, 0},	/* 10*/
  {0, 75, 1},
  {0, 80, 2},
  {1, 85, 0},
  {1, 88, 1},
  {1, 90, 2},	/* 15 */
  {2, 95, 0},
  {2, 97, 2},
  {3, 99, 1},
  {4, 99, 0},
  {4, 99, 2},	/* 20*/
  {5, 99, 1},
  {6, 99, 0},
  {6, 99, 2},
  {7, 99, 1},
  {8, 99, 0}	/* 25 */
};

const char *chaString[27] = {
    "NONE!",
    "Nightmare",  "Horrifying",  "Monstrous",   /* 01 - 03 */
    "Sickening",  "Hideous",     "Deformed",    /* 04 - 06 */
    "Ugly",       "Ugly",        "Unattractive",   /* 07 - 09 */
    "Below Avg",  "Plain",       "Average",     /* 10 - 12 */
    "Appealing",  "Attractive",  "Comely",      /* 13 - 15 */
    "Charming",   "Stunning",    "Gorgeous",    /* 16 - 18 */
    "Breathtaking","Majestic",   "Perfect",     /* 19 - 21 */
    "Unearthly",  "Godlike",     "Godlike",	/* 22 - 24 */
    "Divine",
    "-*END*-"
};

const char *conString[27] = {
    "NONE!",
    "Simp",       "Sickly",      "Anemic",      /* 01 - 03 */
    "Anemic",     "Fragile",     "Frail",       /* 04 - 06 */
    "Feeble",     "Feeble",      "Below Avg",   /* 07 - 09 */
    "Below Avg",  "Average",     "Average",     /* 10 - 12 */
    "Above Avg",  "Healthy",     "Husky",       /* 13 - 15 */
    "Hardy",      "Sturdy",      "Rugged",      /* 16 - 18 */
    "Tough",      "Tireless",    "Primordial",  /* 19 - 21 */
    "Primordial", "Godlike",     "Godlike",	/* 22 - 24 */
    "Divine",
    "-*END*-"
};

const char *dexString[27] = {
    "NONE!",
    "Immobile",   "Slugish",     "Arthritic",   /* 01 - 03 */
    "Brick",      "Clutz",       "Clutz",       /* 04 - 06 */
    "Clumsy",     "Clumsy",      "Below Avg",   /* 07 - 09 */
    "Below Avg",  "Average",     "Average",     /* 10 - 12 */
    "Above Avg",  "Deft",        "Adroit",      /* 13 - 15 */
    "Smooth",     "Nimble",      "Agile",       /* 16 - 18 */
    "Graceful",   "Cat-like",    "Incredible",  /* 19 - 21 */
    "Incredible", "Godlike",	 "Godlike",	/* 22 - 24 */
    "Divine",					/* 25	   */
    "-*END*-"
};

const char *strString[32] = {
    "NONE!",
    "Child",      "Child",       "Wimpy",       /* 01 - 03 */
    "Wimpy",      "Pencil-neck", "Weak",        /* 04 - 06 */
    "Weak",       "Weak",        "Pipsqueak",   /* 07 - 09 */
    "Piwi",       "Below Avg",   "Average",     /* 10 - 12 */
    "Average",    "Above Avg",   "Vigorous",    /* 13 - 15 */
    "Wiry",       "Sinewy",      "Muscular",    /* 16 - 18 */
    "Potent",     "Strong",      "Powerful",    /* 18/0 , 18/41,  18/61 */
    "Heroic",     "Atlantian",   "Herculean",   /* 18/81, 18/100, 19    */
    "Gigantic",   "Colossal",    "Gargantuan",  /* 20 - 22 */
    "Titanic",    "Godlike",	 "Divine",	/* 23 - 25 */
    "-*END*-"
};

const char *intString[27] = {
    "NONE!",
    "Vegetable",  "Animal",      "Idiot",
    "Imbecile",   "Moron",       "Dolt",
    "Dumb",       "Below Avg",   "Average",
    "Average",    "Average",     "Apt",
    "Smart",      "Clever",      "Intellignt",
    "Exceptnl",   "Brilliant",   "Gifted",
    "Genius",     "Genius",      "Sup-Genius",
    "Sup-Genius", "Godlike",	 "Godlike",
    "Divine",
    "-*END*-"
};

const char *wisString[27] = {
    "NONE!",
    "Ninkempoop",        "Fool",              "Fool",
    "Fool",              "Twit",              "Gullible",
    "Gullible",          "Gullible",          "Naive",
    "Naive",             "Average",           "Average",
    "Above Avg",         "Keen",              "Crafty",
    "Shrewd",            "Astute",            "Wise",
    "Sagacious",         "Transcendent",      "Infinite",
    "Omniscient",	 "Godlike",	      "Godlike",
    "Divine",
    "-*END*-"
};
