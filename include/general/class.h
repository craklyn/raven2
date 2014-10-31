/* ============================================================================
Header files for classes.
* I've put every constant related to race or class(which includes a
* few immortal bits and pieces) in here. If your adding a new race or
* class you will have to adjust all the relevant constants in this
* file(defined in class.c)
* Vex.
============================================================================ */
#ifndef __CLASS_H__
#define __CLASS_H__
/* ============================================================================
Public functions found in class.c
============================================================================ */
char get_class_char(int class);
int  parse_class(char arg);
int  invalid_class(CharData *ch, ObjData *obj);
int  objAntiShouLin( ObjData *obj );
bitvector_t find_class_bitvector(const char *arg);

/* Race specific functions */
char get_race_char(int race);
int  all_parse_race(char arg);
int  pc_parse_race(char arg);
int  has_native_aff(CharData *ch, int skill);
int  invalid_race(CharData *ch, ObjData *obj);
bitvector_t find_race_bitvector(const char *arg);

/* Character Initializations */
void give_initial_skills(CharData *ch);
void do_initialise(CharData *ch);
void do_start(CharData * ch);
void roll_real_abils(CharData * ch);
int  parse_pc_race_class(int race, char arg);

/* Player Exp & Leveling Functions */
void level_up(CharData * ch);
void retreat_level(CharData *ch);
int  expfactor(CharData *ch);

int GET_RACE(CharData *ch);
void SET_RACE(CharData *ch, int race);

int GET_SUBRACE(CharData *ch);
void SET_SUBRACE(CharData *ch, int srace);

/* ============================================================================
Class Definitions
============================================================================ */
#define CLASS_UNDEFINED	     -1
#define CLASS_MAGIC_USER      0
#define CLASS_CLERIC          1
#define CLASS_THIEF           2
#define CLASS_WARRIOR         3
#define CLASS_RANGER          4
#define CLASS_ASSASSIN        5
#define CLASS_SHOU_LIN        6
#define CLASS_SOLAMNIC_KNIGHT 7
#define CLASS_DEATH_KNIGHT    8
#define CLASS_SHADOW_DANCER   9
#define CLASS_NECROMANCER     10
#define NUM_CLASSES           11

/* ============================================================================
Race Definitions
============================================================================ */
#define RACE_UNDEFINED        -1  /* Unknown race */
#define RACE_HUMAN            0   /* 'H'          */
#define RACE_PLANT            1   /* 'P' NPC only */
#define RACE_ANIMAL           2   /* 'K' NPC only */
#define RACE_DRAGON           3   /* 'D' NPC only */
#define RACE_UNDEAD           4   /* 'U' NPC only */
#define RACE_VAMPIRE          5   /* 'V' NPC only */
#define RACE_HALFLING         6   /* 'L'          */
#define RACE_ELF              7   /* 'E'          */
#define RACE_DWARF            8   /* 'W'          */
#define RACE_GIANT            9   /* 'G'          */
#define RACE_MINOTAUR        10   /* 'M' NPC only */
#define RACE_DEMON           11   /* 'N' NPC only */
#define RACE_OGRE            12   /* 'O'          */
#define RACE_TROLL           13   /* 'T' NPC only */
#define RACE_WEREWOLF        14   /* 'R' NPC only */
#define RACE_ELEMENTAL       15   /* 'F' NPC only */
#define RACE_ORC             16   /* 'C'	  */
#define RACE_GNOME           17   /* 'B'      */
#define RACE_DRACONIAN	     18   /* 'I'	  */
#define RACE_FAERIE          19   /* 'J' NPC only */
#define RACE_AMARA           20   /* 'A' NPC only */
#define RACE_IZARTI          21   /* 'Z' NPC only */
#define RACE_DROW            22   /* 'X'          */
#define RACE_SHUMAN          23
#define RACE_SHALFLING       24
#define RACE_SELF            25
#define RACE_SDROW           26
#define RACE_SDWARF          27
#define RACE_SMINOTAUR       28
#define RACE_SOGRE           29
#define RACE_STROLL          30
#define RACE_SDRACONIAN      31
#define RACE_SGNOME          32
#define RACE_SORC            33
#define RACE_TERRAN          34
#define RACE_ZERG            35
#define RACE_PROTOSS         36
#define NUM_RACES            37   /* Always update this! */

/* ============================================================================
SubRace Definitions
============================================================================ */
#define SUBRACE_UNDEFINED     0
#define NUM_SUBRACES         13

/* Dragon(and currently draconian) subraces */
#define RED_DRAGON            1
#define GREEN_DRAGON          2
#define WHITE_DRAGON          3
#define BLACK_DRAGON          4
#define BLUE_DRAGON           5
#define CHROMATIC_DRAGON      6
#define GOLD_DRAGON           7
#define SILVER_DRAGON         8
#define BRONZE_DRAGON         9
#define COPPER_DRAGON        10
#define BRASS_DRAGON         11
#define SHADOW_DRAGON        12
#define MIST_DRAGON          13

/* Elemental subraces */
#define FIRE_ELEMENTAL        1
#define AIR_ELEMENTAL         2
#define WATER_ELEMENTAL       3
#define EARTH_ELEMENTAL       4

/* ============================================================================
Class & Race Macros
============================================================================ */

#define RACE_ABBR(ch)       (IS_ELEMENTAL(ch) || GET_RACE(ch) == RACE_SDRACONIAN ? SUBRACE_ABBR(ch) : race_abbrevs[(int)GET_RACE(ch)])
#define SUBRACE_ABBR(ch)    (IS_ELEMENTAL(ch) ? ele_subrace_abbrevs[(int)GET_SUBRACE(ch)] : drc_subrace_abbrevs[(int)GET_SUBRACE(ch)])

#define IS_REMORT(ch)      ((ch)->player.race == RACE_UNDEAD    || \
                            (ch)->player.race == RACE_VAMPIRE   || \
                            (ch)->player.race == RACE_GIANT     || \
                            (ch)->player.race == RACE_DEMON     || \
                            (ch)->player.race == RACE_WEREWOLF  || \
                            (ch)->player.race == RACE_ELEMENTAL || \
                            (ch)->player.race == RACE_FAERIE    || \
                            (ch)->player.race == RACE_AMARA     || \
                            (ch)->player.race == RACE_SHUMAN    || \
                            (ch)->player.race == RACE_SHALFLING || \
                            (ch)->player.race == RACE_SELF      || \
                            (ch)->player.race == RACE_SDROW     || \
                            (ch)->player.race == RACE_SDWARF    || \
                            (ch)->player.race == RACE_SMINOTAUR || \
                            (ch)->player.race == RACE_SOGRE     || \
                            (ch)->player.race == RACE_STROLL    || \
                            (ch)->player.race == RACE_SDRACONIAN || \
                            (ch)->player.race == RACE_SGNOME    || \
                            (ch)->player.race == RACE_SORC      || \
                            (ch)->player.race == RACE_IZARTI)

#define GET_CLASS(ch)       ((ch)->player.class)
#define CLASS_ABBR(ch)      (class_abbrevs[(int)GET_CLASS(ch)])


#define IS_MAGIC_USER(ch)      (GET_CLASS(ch) == CLASS_MAGIC_USER)
#define IS_CLERIC(ch)          (GET_CLASS(ch) == CLASS_CLERIC)
#define IS_THIEF(ch)           (GET_CLASS(ch) == CLASS_THIEF)
#define IS_WARRIOR(ch)         (GET_CLASS(ch) == CLASS_WARRIOR)
#define IS_RANGER(ch)          (GET_CLASS(ch) == CLASS_RANGER)
#define IS_ASSASSIN(ch)        (GET_CLASS(ch) == CLASS_ASSASSIN)
#define IS_SHOU_LIN(ch)        (GET_CLASS(ch) == CLASS_SHOU_LIN)
#define IS_SOLAMNIC_KNIGHT(ch) (GET_CLASS(ch) == CLASS_SOLAMNIC_KNIGHT)
#define IS_DEATH_KNIGHT(ch)    (GET_CLASS(ch) == CLASS_DEATH_KNIGHT)
#define IS_SHADOW_DANCER(ch)   (GET_CLASS(ch) == CLASS_SHADOW_DANCER)
#define IS_NECROMANCER(ch)     (GET_CLASS(ch) == CLASS_NECROMANCER)

/* ============================================================================
Misc. class/race related stuff
============================================================================ */
typedef struct title_type
{
  char *title_m;
  char *title_f;
  int  exp;
} TitleType;

#define SAVING_PARA           0
#define SAVING_ROD            1
#define SAVING_PETRI          2
#define SAVING_BREATH         3
#define SAVING_SPELL          4
#define SAVING_MAX            5

/* constants used to index race_stat_limits */
#define STRENGTH_INDEX        0
#define STRENGTH_ADD_INDEX    1
#define INTELLIGENCE_INDEX    2
#define WISDOM_INDEX 	      3
#define DEXTERITY_INDEX       4
#define CONSTITUTION_INDEX    5
#define CHARISMA_INDEX 	      6

/* ============================================================================
global buffering system - allow access to global variables within class.c
============================================================================ */
#ifndef __CLASS_C__

extern const TitleType titles[][ LVL_IMPL + 2 ];
extern const int   thaco[][ LVL_IMPL + 2 ];
extern const int   saving_throws[][SAVING_MAX][LVL_IMPL+2];
extern const char  *pc_class_types[];
extern const char  *class_abbrevs[];
extern const char  *class_menu[];
extern       int   prac_params[][NUM_CLASSES];
/* Race global variables */
extern const char *race_menu;
extern const char *breath_menu;
extern const char *race_types[];
extern const char *race_abbrevs[];
extern const char *ele_subrace_abbrevs[];
extern const char *drc_subrace_abbrevs[];
/* This array determines the stat limits for every race on the mud. */
extern const char race_stat_limits[][7];
/* This array determines which classes a pc of a particular race can be. */
extern const sh_int classes_allowed[][NUM_CLASSES];

#endif /* __CLASS_C__ */

#endif /* _CLASS_H_*/
