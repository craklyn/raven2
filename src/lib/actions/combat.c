/*
** This is the class specific combat code for non-spec_proc mobs. These
** functions will be called as a direct result of the class settings in
** the mob's prototype.
*/

#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/class.h"
#include "general/handler.h"
#include "actions/interpreter.h"
#include "magic/skills.h"
#include "magic/spells.h"
#include "util/utils.h"
#include "general/comm.h"
#include "util/weather.h"
#include "magic/stun.h"
#include "magic/knock.h"
#include "magic/gore.h"
#include "magic/feed.h"
#include "magic/sing.h"
#include "actions/fight.h"
#include "actions/act.h"          /* ACMDs located within the act*.c files */
#include "actions/combat.h"

typedef struct {
    int  class_id;
    void (* attack_function )();
} combat_vector_t;

typedef struct {
    int  skill_id;
    void (* skill_function )(CharData *, char *, int , int );
} skill_vector_t;

static skill_vector_t skill_vector[] = {
    { SKILL_NULL, SKILL_NULL },
    { SKILL_BACKSTAB, do_backstab }, 	/* 1 */
    { SKILL_BASH, do_bash },
    { SKILL_HIDE, do_hide },
    { SKILL_KICK, do_kick },
    { SKILL_PICK_LOCK, do_pick },	/* 5 */
    { SKILL_PUNCH, SKILL_NULL },
    { SKILL_RESCUE, do_rescue },
    { SKILL_SNEAK, do_sneak },
    { SKILL_STEAL, do_steal },
    { SKILL_TRACK, do_track },		/* 10 */
    { SKILL_DISARM, do_disarm },
    { SKILL_SECOND_ATTACK, SKILL_NULL },
    { SKILL_THIRD_ATTACK, SKILL_NULL },
    { SKILL_SCAN, do_scan },
    { SKILL_LAY_HANDS, do_hands },	/* 15 */
    { SKILL_FISTS_OF_FURY, do_fists },
    { SKILL_THROW, do_throw },
    { SKILL_SHOOT, do_shoot },
    { SKILL_KNOCK, do_knock },
    { SKILL_TRIP, do_trip },		/* 20 */
    { SKILL_BLINDING_STRIKE, do_blind_strike },
    { SKILL_HAMSTRING, do_hamstring },
    { SKILL_ENHANCED_DAMAGE, SKILL_NULL },
    { SKILL_RETREAT, do_retreat },
    { SKILL_TURN, SKILL_NULL },		/* 25 */
    { SKILL_BUTCHER, do_butcher },
    { SKILL_DODGE, SKILL_NULL },
    { SKILL_TRAP, SKILL_NULL },
    { SKILL_SEARCH_TRAP, SKILL_NULL },
    { SKILL_PALM, do_palm },		/* 30 */
    { SKILL_FIND_WEAKNESS, SKILL_NULL },
    { SKILL_SKIN, SKILL_NULL },
    { SKILL_FEIGN_DEATH, do_feign_death },
    { SKILL_ART_DRAGON, do_aspect },
    { SKILL_ART_SNAKE, do_aspect },	/* 35 */
    { SKILL_ART_TIGER, do_aspect },
    { SKILL_ART_CRANE, do_aspect },
    { SKILL_CIRCLE, do_circle },
    { SKILL_DUST, do_dust },
    { SKILL_STALK, do_stalk },		/* 40 */
    { SKILL_ART_WIND, SKILL_NULL },
    { SKILL_ENVENOM, do_envenom },
    { SKILL_PARRY, SKILL_NULL },
    { SKILL_SWEEP, do_sweep },	
    { SKILL_DOORBASH, do_doorbash },	/* 45 */
    { SKILL_PENUMBRAE, SKILL_NULL },
    { SKILL_PICKPOCKET, SKILL_NULL },
    { SKILL_APPRAISE, SKILL_NULL },
    { SKILL_DELUSION, SKILL_NULL },
    { SKILL_CUTPURSE, SKILL_NULL },	/* 50 */
    { SKILL_COWER, SKILL_NULL },
    { SKILL_DANGER_SENSE, SKILL_NULL },
    { SKILL_THIEF_SENSE, SKILL_NULL },
    { SKILL_ASSASSINATION, SKILL_NULL },
    { SKILL_DIRTY_TACTICS, SKILL_NULL },	/* 55 */
    { SKILL_AGGRESSIVE, SKILL_NULL },
    { SKILL_SHADOW_DANCE, do_shadowdance },
    { SKILL_GORE, do_gore },
    { SKILL_BREATHE, do_breathe },
    { SKILL_BERSERK, do_berserk },		/* 60 */
    { SKILL_ART_MONKEY, SKILL_NULL },
    { SKILL_ART_FLOWER, SKILL_NULL },
    { SKILL_GUARD,		SKILL_NULL },
    { SKILL_HEIGHTENED_SENSES, SKILL_NULL },
    { SKILL_RIPOSTE,     SKILL_NULL },   /* 65 */
    { SKILL_BLACKJACK, SKILL_NULL},
    { SKILL_AMBUSH,     SKILL_NULL },
    { SKILL_SHADOW_STEP, SKILL_NULL},
    { SKILL_SHADOW_MIST, SKILL_NULL},
    { SKILL_GUT,        SKILL_NULL },  /* 70 */
    { SKILL_BRAIN,      SKILL_NULL },
    { SKILL_CUT_THROAT, do_cut_throat },
    { SKILL_CONVERT,    SKILL_NULL },
    { SKILL_FAMILIARIZE, SKILL_NULL},
    { SKILL_ESCAPE,     SKILL_NULL },   /* 75 */
    { SKILL_DANCE_DEATH, SKILL_NULL },
    { SKILL_SHADOWBOX,  SKILL_NULL },
    { SKILL_FENCE,      SKILL_NULL },
    { SKILL_MUG,        SKILL_NULL },
    { SKILL_SPY,        SKILL_NULL },   /* 80 */
    { SKILL_DISTRACT,   SKILL_NULL },
    { SKILL_RETARGET,   do_retarget },
    { SKILL_DEVOUR,     do_devour  },
    { SKILL_BLOCK,      do_block   },
    { SKILL_SHIELD_BASH, SKILL_NULL},
};/* 85 */

/*
#define SKILL_EDGE_MASTERY          266
#define SKILL_POINT_MASTERY         267
#define SKILL_BLUNT_MASTERY         268
#define SKILL_EXOTIC_MASTERY        269
#define SKILL_SCOUT                 270
#define SKILL_BULLSEYE              271
#define SKILL_EXPOSE                272
#define SKILL_CAMP                  273
#define SKILL_POULTICE              274
#define SKILL_FEED                  275
#define SKILL_CALM                  276
#define SKILL_MIST                  277
#define SKILL_STING                 278
#define SKILL_BATTLECRY             279
#define SKILL_WARCRY                280
#define SKILL_STANCE                281
#define SKILL_POWERSTRIKE           282
#define SKILL_FOCUS                 283
#define SKILL_DEVOTION              284
#define SKILL_FERVOR                285
#define SKILL_FALSE_TRAIL           286
#define SKILL_ENHANCED_STEALTH      287
#define SKILL_SHADOW_JUMP           288
#define SKILL_EVASION               289
#define SKILL_CRITICAL_HIT          290
#define SKILL_ADRENALINE            291
#define SKILL_BEFRIEND              292
#define SKILL_CHARGE                293
#define SKILL_FLASHBANG		    294
#define SKILL_INSTANT_POISON	    295
#define SKILL_REIKI                 296
*/

/*
** Forward function definitions.
*/
static CharData * find_mob_victim( CharData * );
static void attack_as_XX( CharData *, CharData * );
static void attack_as_Mu( CharData *, CharData * );
static void attack_as_Cl( CharData *, CharData * );
static void attack_as_Th( CharData *, CharData * );
static void attack_as_Wa( CharData *, CharData * );
static void attack_as_Ra( CharData *, CharData * );
static void attack_as_As( CharData *, CharData * );
static void attack_as_Sl( CharData *, CharData * );
static void attack_as_Kn( CharData *, CharData * );
static void attack_as_De( CharData *, CharData * );
static void attack_as_Sd( CharData *, CharData * );
static void attack_as_Nm( CharData *, CharData * );

static void attack_as_Human (CharData *, CharData * );
static void attack_as_Plant (CharData *, CharData * );
static void attack_as_Animal (CharData *, CharData * );
static void attack_as_Dragon (CharData *, CharData * );
static void attack_as_Undead (CharData *, CharData * );
static void attack_as_Vampire (CharData *, CharData * );
static void attack_as_Halfling (CharData *, CharData * );
static void attack_as_Elf (CharData *, CharData * );
static void attack_as_Drow (CharData *, CharData * );
static void attack_as_Dwarf (CharData *, CharData * );
static void attack_as_Giant (CharData *, CharData * );
static void attack_as_Minotaur (CharData *, CharData * );
static void attack_as_Demon (CharData *, CharData * );
static void attack_as_Ogre (CharData *, CharData * );
static void attack_as_Troll (CharData *, CharData * );
static void attack_as_Werewolf (CharData *, CharData * );
static void attack_as_Elemental (CharData *, CharData * );
static void attack_as_Orc (CharData *, CharData * );
static void attack_as_Gnome (CharData *, CharData * );
static void attack_as_Draconian (CharData *, CharData * );
static void attack_as_Faerie (CharData *, CharData * );
static void attack_as_Amara (CharData *, CharData * );
static void attack_as_Izarti (CharData *, CharData * );
static void feign_death_attack(CharData *the_mob, CharData *the_pc);

/*
** attack_lists on a class by class def.
*/
#   define MIN_LEVEL_SPREAD  10

static short attack_list_Cl[] = { SPELL_NULL,
    /* 01 */ SPELL_CAUSE_WOUND, SPELL_CAUSE_WOUND, SPELL_CAUSE_WOUND, SPELL_CAUSE_WOUND,
    /* 05 */ SPELL_CAUSE_WOUND, SPELL_CAUSE_WOUND, SPELL_CAUSE_WOUND, SPELL_CAUSE_WOUND, SPELL_CAUSE_WOUND,
    /* 10 */ SPELL_CAUSE_WOUND, SPELL_BLINDNESS, SPELL_CAUSE_WOUND, SPELL_BLINDNESS, SPELL_CAUSE_WOUND,
    /* 15 */ SPELL_BLINDNESS, SPELL_CALL_LIGHTNING, SPELL_CALL_LIGHTNING, SPELL_BLINDNESS, SPELL_CALL_LIGHTNING,
    /* 20 */ SPELL_EARTHQUAKE, SPELL_EARTHQUAKE, SPELL_DISPEL_EVIL, SPELL_DISPEL_EVIL, SPELL_BLINDNESS,
    /* 25 */ SPELL_HARM, SPELL_HARM, SPELL_EARTHQUAKE, SPELL_DISPEL_EVIL, SPELL_EARTHQUAKE,
    /* 30 */ SPELL_BANISH, SPELL_BANISH, SPELL_HARM, SPELL_CHAIN_LIGHTNING, SPELL_CHAIN_LIGHTNING,
    /* 35 */ SPELL_BANISH, SPELL_CHAIN_LIGHTNING, SPELL_BLINDNESS, SPELL_DISPEL_EVIL, SPELL_HARM,
    /* 40 */ SPELL_FLAME_STRIKE, SPELL_FLAME_STRIKE, SPELL_CHAIN_LIGHTNING, SPELL_HOLY_WORD, SPELL_HOLY_WORD,
    /* 45 */ SPELL_CHAIN_LIGHTNING, SPELL_FLAME_STRIKE, SPELL_HOLY_WORD, SPELL_FLAME_STRIKE, SPELL_DISHEARTEN,
    /* 50 */ SPELL_HOLY_WORD
};

static short attack_list_Mu[] = { SPELL_NULL,
    /* 01 */ SPELL_MAGIC_MISSILE, SPELL_MAGIC_MISSILE, SPELL_MAGIC_MISSILE, SPELL_MAGIC_MISSILE,
    /* 05 */ SPELL_CHILL_TOUCH, SPELL_MONSTER_SUMMON, SPELL_CHILL_TOUCH, SPELL_BLINDNESS, SPELL_BLINDNESS,
    /* 10 */ SPELL_SHOCKING_GRASP, SPELL_MONSTER_SUMMON, SPELL_SHOCKING_GRASP, SPELL_BLINDNESS, SPELL_SHOCKING_GRASP,
    /* 15 */ SPELL_SHOCKING_GRASP, SPELL_MONSTER_SUMMON, SPELL_SHOCKING_GRASP, SPELL_SHOCKING_GRASP, SPELL_BLINDNESS,
    /* 20 */ SPELL_COLOR_SPRAY, SPELL_BLINDNESS, SPELL_COLOR_SPRAY, SPELL_LIFE_DRAIN, SPELL_MONSTER_SUMMON,
    /* 25 */ SPELL_ICE_STORM, SPELL_COLOR_SPRAY, SPELL_ICE_STORM, SPELL_COLOR_SPRAY, SPELL_SAND_STORM,
    /* 30 */ SPELL_CONJURE_ELEMENTAL, SPELL_SHRIEK, SPELL_ICE_STORM, SPELL_SHRIEK, SPELL_MONSTER_SUMMON,
    /* 35 */ SPELL_LIGHTNING_BOLT, SPELL_LIGHTNING_BOLT, SPELL_ICE_STORM, SPELL_LIGHTNING_BOLT, SPELL_SAND_STORM,
    /* 40 */ SPELL_GATE, SPELL_MONSTER_SUMMON, SPELL_METEOR_SWARM, SPELL_SAND_STORM, SPELL_FIREBALL,
    /* 45 */ SPELL_METEOR_SWARM, SPELL_FIREBALL, SPELL_GATE, SPELL_WRATH_OF_THE_ANCIENTS, SPELL_WRATH_OF_THE_ANCIENTS,
    /* 50 */ SPELL_DOOM_BOLT
};

static short attack_list_Sl2[] = { SKILL_NULL,
    /* 01 */ SKILL_NULL, SKILL_NULL, SKILL_NULL, SKILL_NULL,
    /* 05 */ SKILL_DISARM, SKILL_DISARM, SKILL_DISARM, SKILL_DISARM, SKILL_KICK,
    /* 10 */ SKILL_KICK, SKILL_KICK, SKILL_DISARM, SKILL_KICK, SKILL_KICK,
    /* 15 */ SKILL_SWEEP, SKILL_DISARM, SKILL_KICK, SKILL_SWEEP, SKILL_SWEEP,
    /* 20 */ SKILL_BLINDING_STRIKE, SKILL_BLINDING_STRIKE, SKILL_SWEEP, SKILL_KICK, SKILL_DISARM,
    /* 25 */ SKILL_KNOCK, SKILL_KNOCK, SKILL_BLINDING_STRIKE, SKILL_SWEEP, SKILL_SWEEP,
    /* 30 */ SKILL_FISTS_OF_FURY, SKILL_BLINDING_STRIKE, SKILL_SWEEP, SKILL_FISTS_OF_FURY, SKILL_KICK,
    /* 35 */ SKILL_FISTS_OF_FURY, SKILL_DISARM, SKILL_SWEEP, SKILL_FISTS_OF_FURY, SKILL_KICK,
    /* 40 */ SKILL_FISTS_OF_FURY, SKILL_DISARM, SKILL_SWEEP, SKILL_FISTS_OF_FURY, SKILL_KICK,
    /* 45 */ SKILL_FISTS_OF_FURY, SKILL_BLINDING_STRIKE, SKILL_SWEEP, SKILL_FISTS_OF_FURY, SKILL_KICK,
    /* 50 */ SKILL_SWEEP
};

static short attack_list_Sl[] = { SKILL_NULL,
    /* 01 */ SKILL_KICK, SKILL_KICK, SKILL_KICK, SKILL_KICK,
    /* 05 */ SKILL_KICK, SKILL_KICK, SKILL_KICK, SKILL_KICK, SKILL_KICK,
    /* 10 */ SKILL_KICK, SKILL_KICK, SKILL_SWEEP, SKILL_SWEEP, SKILL_KICK,
    /* 15 */ SKILL_DISARM, SKILL_DISARM, SKILL_KICK, SKILL_BLINDING_STRIKE, SKILL_BLINDING_STRIKE,
    /* 20 */ SKILL_SWEEP, SKILL_DISARM, SKILL_KICK, SKILL_KNOCK, SKILL_KNOCK,
    /* 25 */ SKILL_DISARM, SKILL_SWEEP, SKILL_FEIGN_DEATH, SKILL_KNOCK, SKILL_FEIGN_DEATH,
    /* 30 */ SKILL_DISARM, SKILL_KICK, SKILL_BLINDING_STRIKE, SKILL_FISTS_OF_FURY, SKILL_FISTS_OF_FURY,
    /* 35 */ SKILL_SWEEP, SKILL_DISARM, SKILL_KICK, SKILL_FEIGN_DEATH, SKILL_KICK,
    /* 40 */ SKILL_FISTS_OF_FURY, SKILL_FISTS_OF_FURY, SKILL_KNOCK, SKILL_DISARM, SKILL_KICK,
    /* 45 */ SKILL_FEIGN_DEATH, SKILL_BLINDING_STRIKE, SKILL_DISARM, SKILL_SWEEP, SKILL_FISTS_OF_FURY,
    /* 50 */ SKILL_FEIGN_DEATH
};

static short attack_list_Kn[] = { SKILL_NULL,
    /* 01 */ SKILL_KICK, SKILL_KICK, SKILL_KICK, SKILL_KICK,
    /* 05 */ SKILL_KICK, SKILL_KICK, SKILL_KICK, SKILL_KICK, SKILL_BASH,
    /* 10 */ SKILL_BASH, SKILL_BASH, SKILL_KICK, SKILL_BASH, SKILL_KICK,
    /* 15 */ SKILL_BASH, SKILL_KICK, SKILL_BASH, SKILL_KICK, SKILL_BASH,
    /* 20 */ SKILL_DISARM, SKILL_KICK, SKILL_BASH, SKILL_DISARM, SKILL_KICK,
    /* 25 */ SKILL_BASH, SKILL_KICK, SKILL_DISARM, SKILL_KICK, SKILL_BASH,
    /* 30 */ SPELL_DISPEL_EVIL, SKILL_DISARM, SKILL_KICK, SKILL_BASH, SPELL_BANISH,
    /* 35 */ SKILL_BLOCK, SKILL_BLOCK, SKILL_DISARM, SKILL_BASH, SPELL_BANISH,
    /* 40 */ SPELL_DISPEL_EVIL, SKILL_DISARM, SKILL_KICK, SKILL_BASH, SKILL_BLOCK,
    /* 45 */ SPELL_HOLY_WORD, SKILL_DISARM, SKILL_RETARGET, SKILL_RETARGET, SKILL_BLOCK,
    /* 50 */ SPELL_HOLY_WORD
};

static short attack_list_De[] = { SKILL_NULL,
    /* 01 */ SKILL_KICK, SPELL_CAUSE_WOUND, SKILL_KICK, SPELL_CAUSE_WOUND,
    /* 05 */ SKILL_KICK, SPELL_CAUSE_WOUND, SKILL_KICK, SPELL_CAUSE_WOUND, SKILL_BASH,
    /* 10 */ SPELL_BLACK_DART, SKILL_BASH, SPELL_BLACK_DART, SKILL_KICK, SPELL_BLACK_DART,
    /* 15 */ SKILL_BASH, SPELL_BLACK_DART, SKILL_KICK, SPELL_BLACK_DART, SKILL_BASH,
    /* 20 */ SKILL_DISARM, SPELL_BLACK_DART, SKILL_KICK, SPELL_CURSE, SKILL_BASH,
    /* 25 */ SPELL_CAUSE_CRITIC, SKILL_DISARM, SPELL_CAUSE_CRITIC, SKILL_KICK, SPELL_CAUSE_CRITIC,
    /* 30 */ SPELL_DISPEL_GOOD, SPELL_DISPEL_GOOD, SPELL_BLINDNESS, SKILL_BASH, SKILL_DISARM,
    /* 35 */ SPELL_PESTILENCE, SKILL_BLOCK, SPELL_FEAR, SPELL_CURSE, SPELL_BLACK_BREATH,
    /* 40 */ SPELL_BLACK_BREATH, SPELL_FEAR, SPELL_PESTILENCE, SKILL_BLOCK, SKILL_BLOCK,
    /* 45 */ SPELL_DEATH_TOUCH, SPELL_UNHOLY_WORD, SPELL_DEATH_TOUCH, SPELL_UNHOLY_WORD, SKILL_DEVOUR,
    /* 50 */ SPELL_DEATH_TOUCH
};

static short attack_list_Th[] = { SKILL_NULL,
    /* 01 */ SKILL_NULL, SKILL_NULL, SKILL_TRIP, SKILL_TRIP,
    /* 05 */ SKILL_TRIP, SKILL_TRIP, SKILL_TRIP, SKILL_TRIP, SKILL_TRIP,
    /* 10 */ SKILL_DISARM, SKILL_TRIP, SKILL_DISARM, SKILL_TRIP, SKILL_DISARM,
    /* 15 */ SKILL_KICK, SKILL_TRIP, SKILL_DISARM, SKILL_KICK, SKILL_TRIP,
    /* 20 */ SKILL_DISARM, SKILL_KICK, SKILL_TRIP, SKILL_DISARM, SKILL_KICK,
    /* 25 */ SKILL_DUST, SKILL_KICK, SKILL_TRIP, SKILL_DISARM, SKILL_DUST,
    /* 30 */ SKILL_KICK, SKILL_TRIP, SKILL_DISARM, SKILL_DUST, SKILL_KICK,
    /* 35 */ SKILL_TRIP, SKILL_DISARM, SKILL_DUST, SKILL_KICK, SKILL_TRIP,
    /* 40 */ SKILL_FEIGN_DEATH, SKILL_FEIGN_DEATH, SKILL_TRIP, SKILL_FEIGN_DEATH, SKILL_DISARM,
    /* 45 */ SKILL_FEIGN_DEATH, SKILL_DUST, SKILL_FEIGN_DEATH, SKILL_KICK, SKILL_DISARM,
    /* 50 */ SKILL_FEIGN_DEATH
};

static short attack_list_Ra[] = { SKILL_NULL,
    /* 01 */ SKILL_NULL, SKILL_NULL, SKILL_NULL, SKILL_NULL,
    /* 05 */ SKILL_NULL, SKILL_NULL, SKILL_NULL, SKILL_NULL,
    /* 10 */ SKILL_NULL, SKILL_NULL, SKILL_NULL, SKILL_KICK,
    /* 15 */ SKILL_TRIP, SKILL_TRIP, SKILL_KICK, SKILL_KICK,
    /* 20 */ SKILL_TRIP, SKILL_TRIP, SKILL_KICK, SKILL_KICK,
    /* 25 */ SKILL_DISARM, SKILL_KICK, SKILL_TRIP, SKILL_TRIP, SPELL_CALL_OF_THE_WILD,
    /* 30 */ SKILL_TRIP, SPELL_CALL_OF_THE_WILD, SKILL_DISARM, SPELL_ENTANGLE, SPELL_ENTANGLE,
    /* 35 */ SKILL_TRIP, SKILL_KICK, SKILL_DISARM, SPELL_CALL_OF_THE_WILD, SPELL_SWARM,
    /* 40 */ SKILL_TRIP, SKILL_KICK, SPELL_CALL_OF_THE_WILD, SPELL_ENTANGLE, SPELL_SWARM,
    /* 45 */ SKILL_TRIP, SPELL_HANDS_OF_WIND, SKILL_DISARM, SPELL_ENTANGLE, SPELL_CALL_OF_THE_WILD,
    /* 50 */ SPELL_SWARM
};

static short attack_list_As[] = { SKILL_NULL,
    /* 01 */ SKILL_NULL, SKILL_NULL, SKILL_NULL, SKILL_NULL,
    /* 05 */ SKILL_NULL, SKILL_NULL, SKILL_NULL, SKILL_NULL,
    /* 10 */ SKILL_KICK, SKILL_KICK, SKILL_KICK, SKILL_KICK,
    /* 15 */ SKILL_KICK, SKILL_KICK, SKILL_TRIP, SKILL_TRIP,
    /* 20 */ SKILL_KICK, SKILL_KICK, SKILL_TRIP, SKILL_TRIP,
    /* 25 */ SKILL_KICK, SKILL_KICK, SKILL_TRIP, SKILL_TRIP,
    /* 30 */ SKILL_KICK, SKILL_KICK, SKILL_TRIP, SKILL_DUST,
    /* 35 */ SKILL_KICK, SKILL_KICK, SKILL_TRIP, SKILL_DUST,
    /* 40 */ SKILL_KICK, SKILL_KICK, SKILL_TRIP, SKILL_DUST,
    /* 45 */ SKILL_KICK, SKILL_DUST, SKILL_TRIP, SKILL_DUST,
    /* 50 */ SKILL_DUST
};

static short attack_list_Sd[] = { SKILL_NULL,
    /* 01 */ SKILL_NULL, SKILL_NULL, SKILL_NULL, SKILL_NULL,
    /* 05 */ SKILL_NULL, SKILL_NULL, SKILL_NULL, SKILL_NULL, SKILL_NULL,
    /* 10 */ SKILL_NULL, SKILL_NULL, SPELL_SHADOW_BLADES, SPELL_SHADOW_BLADES, SPELL_SHADOW_BLADES,
    /* 15 */ SPELL_BLINDNESS, SKILL_TRIP, SPELL_SHADOW_BLADES, SPELL_BLINDNESS, SKILL_TRIP,
    /* 20 */ SPELL_SHADOW_BLADES, SKILL_TRIP, SKILL_DISARM, SPELL_SHADOW_BLADES, SPELL_BLINDNESS,
    /* 25 */ SKILL_TRIP, SKILL_DISARM, SPELL_SHADOW_BLADES, SPELL_SHADOW_BLADES, SPELL_BLINDNESS,
    /* 30 */ SKILL_TRIP, SKILL_DISARM, SPELL_DANCE_DREAMS, SPELL_SHADOW_BLADES, SPELL_SHADOW_BLADES,
    /* 35 */ SKILL_TRIP, SKILL_DISARM, SPELL_BLINDNESS, SPELL_SHADOW_BLADES, SPELL_DANCE_DREAMS,
    /* 40 */ SKILL_SHADOW_DANCE, SKILL_TRIP, SKILL_SHADOW_DANCE, SPELL_SHADOW_BLADES, SPELL_DANCE_MISTS,
    /* 45 */ SKILL_SHADOW_DANCE, SKILL_DISARM, SPELL_BLINDNESS, SPELL_SHADOW_BLADES, SPELL_DANCE_DREAMS,
    /* 50 */ SKILL_SHADOW_DANCE
};

static short attack_list_Nm[] = { SPELL_NULL,
    /* 01 */ SPELL_LIFE_DRAIN, SPELL_LIFE_DRAIN, SPELL_LIFE_DRAIN, SPELL_LIFE_DRAIN,
    /* 05 */ SPELL_SOUL_PIERCE, SPELL_SOUL_PIERCE, SPELL_LIFE_DRAIN, SPELL_LIFE_DRAIN, SPELL_LIFE_DRAIN,
    /* 10 */ SPELL_LIFE_DRAIN, SPELL_LIFE_DRAIN, SPELL_SOUL_PIERCE, SPELL_SOUL_PIERCE, SPELL_BLINDNESS,
    /* 15 */ SPELL_SOUL_PIERCE, SPELL_LIFE_DRAIN, SPELL_SOUL_PIERCE, SPELL_SOUL_PIERCE, SPELL_BLINDNESS,
    /* 20 */ SPELL_LIFE_DRAIN, SPELL_LIFE_DRAIN, SPELL_SOUL_PIERCE, SPELL_SOUL_PIERCE, SPELL_BLINDNESS,
    /* 25 */ SPELL_SOUL_PIERCE, SPELL_LIFE_DRAIN, SPELL_SOUL_PIERCE, SPELL_SOUL_PIERCE, SPELL_BLINDNESS,
    /* 30 */ SPELL_LIFE_DRAIN, SPELL_LIFE_DRAIN, SPELL_SOUL_PIERCE, SPELL_SOUL_PIERCE, SPELL_BLINDNESS,
    /* 35 */ SPELL_SOUL_PIERCE, SPELL_LIFE_DRAIN, SPELL_SOUL_PIERCE, SPELL_SOUL_PIERCE, SPELL_BLINDNESS,
    /* 40 */ SPELL_LIFE_DRAIN, SPELL_LIFE_DRAIN, SPELL_SOUL_PIERCE, SPELL_SOUL_PIERCE, SPELL_BLINDNESS,
    /* 45 */ SPELL_SOUL_PIERCE, SPELL_LIFE_DRAIN, SPELL_SOUL_PIERCE, SPELL_SOUL_PIERCE, SPELL_BLINDNESS,
    /* 50 */ SPELL_SOUL_PIERCE
};

/*                                Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  XX  XX  XX  XX  XX  XX  XX  XX  XX */
static int max_attacks[]  =     {  3,  3,  3,  4,  3,  3,  4,  3,  3,  3,  3,  0,  0,  0,  0,  0,  0,  0,  0,  0 };
static int max_2nd_lvls[] =     { 00, 00, 75, 94, 85, 80, 90, 85, 85, 70, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 };
static int max_3rd_lvls[] =     { 00, 00, 00, 90, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 };
#define SKILL_2ND_LEARN           max_2nd_lvls[ (int)GET_CLASS(ch) ]
#define SKILL_3RD_LEARN           max_3rd_lvls[ (int)GET_CLASS(ch) ]

/*
** calculate_attack_count:
**
*/
int
calculate_attack_count( CharData * ch )
{
    int number_of_attacks = 1;

    // If affected by commanding shout, no attacks are allowed
    if(affected_by_spell(ch, SKILL_COMMANDING_SHOUT))
        return 0;

    if( IS_MOB(ch) ){
        /* if someone's riding it, it cannot attack anymore */
        if (ch->rider) return 0;

        switch (GET_CLASS(ch)) {
            case CLASS_RANGER:
            case CLASS_SOLAMNIC_KNIGHT:
            case CLASS_DEATH_KNIGHT:
                number_of_attacks = ( GET_LEVEL(ch) < 13 ? 1 : GET_LEVEL(ch) / 13 );
                break;
            case CLASS_WARRIOR:
            case CLASS_SHOU_LIN:
                number_of_attacks = ( GET_LEVEL(ch) < 11 ? 1 : GET_LEVEL(ch) / 11 );
                break;
            default:
                number_of_attacks = ( GET_LEVEL(ch) < 15 ? 1 : GET_LEVEL(ch) / 15 );
                break;
        }
    }/* if */

    else if( skillSuccess( ch, SKILL_SECOND_ATTACK ) || IS_AFFECTED(ch, AFF_BERSERK)){
        number_of_attacks += 1;
        advanceSkill( ch, SKILL_SECOND_ATTACK, SKILL_2ND_LEARN, NULL, TRUE, TRUE, TRUE );

        if( (skillSuccess( ch, SKILL_THIRD_ATTACK ) || IS_AFFECTED(ch, AFF_BERSERK))
            && !affected_by_spell(ch, SKILL_STEADFASTNESS)){
            number_of_attacks += 1;
            advanceSkill( ch, SKILL_THIRD_ATTACK, SKILL_3RD_LEARN, NULL, TRUE, TRUE, TRUE );
        }
    }

        if(affected_by_spell(ch, SKILL_REDOUBT)) number_of_attacks -= 1;

	if(affected_by_spell(ch, SPELL_HASTE)) number_of_attacks += 1;
	else if(affected_by_spell(ch, SPELL_SLOW)) number_of_attacks -= 1;
	else if(IS_AFFECTED(ch, AFF_HASTE)) number_of_attacks += 1;
   	
        /*
	if( affected_by_spell(ch, SPELL_SLOW  )) number_of_attacks -= 1;

	if( affected_by_spell(ch, SPELL_HASTE )) number_of_attacks += 1;
	else if(IS_AFFECTED(ch, AFF_HASTE)) number_of_attacks += 1;
        */	
 
    number_of_attacks = ( number_of_attacks > max_attacks[ (int)GET_CLASS(ch) ] ?
                          max_attacks[ (int)GET_CLASS(ch) ] : number_of_attacks ); 

    if( number_of_attacks < 1 ) number_of_attacks  = 1;

    return( number_of_attacks );
}

// should_use_spell exists to check if the mob really wants to use the spell
// or skill.  For example, if a mortal already has a gate, he can't summon
// another, so this function will say "Don't do that."
static int
should_use_spell( CharData *the_mob, CharData *the_pc, int the_spell )
{
	struct follow_type *f;
	int count = 0;

	if (the_spell == SPELL_ANIMATE_DEAD                  ||
			the_spell == SPELL_CALL_OF_THE_WILD  ||
			the_spell == SPELL_MONSTER_SUMMON    ||
			the_spell == SPELL_CHARM             ||
			the_spell == SPELL_GATE_MAJOR_DEMON  ||
			the_spell == SPELL_CONJURE_ELEMENTAL ||
			the_spell == SPELL_GATE) 
        {
            // Charmies can't use charmies.
            if (IS_SET_AR(AFF_FLAGS(the_mob), AFF_CHARM))
                return FALSE;
            
            // Only one pet allowed, don't try to make another.
            // UPDATE: Now level 51's can summon more pets, chance reduces if mob is in room and fighting.
            f = the_mob->followers;
            while( f )
            {
                if( IS_AFFECTED( f->follower, AFF_CHARM ) && ( f->follower->master == the_mob )){
                    if( GET_LEVEL(the_mob) < LVL_IMMORT)
                        return FALSE;
                    else if( (IN_ROOM(the_mob) == IN_ROOM( f->follower )) &&
                            (GET_POS(f->follower) != POS_SLEEPING) )
                        count++;
                }
                f = f->next;
            }
            // Chance of summoning another pet decreases as more pets are around.
            /*if ( !number(0, count) )
             * 			return FALSE;
             * 			*/
        }

        if(IS_AFFECTED(the_pc, AFF_BLIND) && (the_spell == SPELL_BLINDNESS ||
                the_spell == SKILL_DUST ||
                the_spell == SKILL_BLINDING_STRIKE))
                return FALSE;

        // Can't see why not..
        return TRUE;
}

static void
use_move( CharData *the_mob, CharData *the_pc, int the_spell)
{
	if( !should_use_spell(the_mob, the_pc, the_spell) )
		return;

        // If you chose to heal, target self!
        if(the_spell == SKILL_LAY_HANDS)
            the_pc = the_mob;

	// If it's a spell cast it, otherwise DO it.
	if( the_spell <= MAX_SPELLS && the_spell > 0 )
        {
            // If the mob calls the spell, cast it.  If not, stun for failing the cast.
            if( skillSuccess( the_mob, the_spell ))
            {
                /* If the_mob successfully casts the spell and it's a summon type of
                 * spell then we should order our new mob to join the fight. */
                if( cast_spell( the_mob, the_pc, NULL, the_spell ) &&
                        (the_spell == SPELL_CALL_OF_THE_WILD ||
                        the_spell == SPELL_ANIMATE_DEAD      ||
                        the_spell == SPELL_MONSTER_SUMMON    ||
                        the_spell == SPELL_CHARM             ||
                        the_spell == SPELL_GATE_MAJOR_DEMON  ||
                        the_spell == SPELL_CONJURE_ELEMENTAL ||
                        the_spell == SPELL_GATE) )

                    do_order(the_mob, "follower assist", 0, 0 );
            }
            else
                WAIT_STATE(the_mob, PULSE_VIOLENCE);
        }
        else
            skill_vector[ the_spell - MAX_SPELLS ].skill_function( the_mob, "", 0, 0 );

}

static void
attempt_circle( CharData *the_mob, CharData *the_pc)
{
    int level       = GET_LEVEL(the_mob);
    CharData *tmp_ch;
    int can_circle  = FALSE;

    if ((level >= 25) && WIELDING(the_mob) &&
        (GET_OBJ_VAL(the_mob->equipment[WEAR_WIELD],3) == (TYPE_PIERCE - TYPE_HIT)))
    {
        can_circle = TRUE;
        for (tmp_ch = world[the_mob->in_room].people; tmp_ch; tmp_ch = tmp_ch->next_in_room)
        {
            if (FIGHTING(tmp_ch) == the_mob)
                can_circle = FALSE;
        }
    }
    
    if (can_circle)
    {
        do_circle(the_mob, "", 0, 0);
        return;
    }
}

static void
assist_master( CharData *the_mob, CharData *the_pc)
{
    if (IS_SET_AR(AFF_FLAGS(the_mob), AFF_CHARM)
            && the_mob->master
            && (the_mob->master->in_room == the_mob->in_room)
            && FIGHTING(the_mob->master))
    {
        /* consider helping our master */
        if( !FIGHTING(the_mob) && (number(1, 100) > 90) )
        {
            do_assist(the_mob, GET_NAME(the_mob->master), 0, 0);
            return;
        }

        /* Our master could be in trouble!  If we're fighting, consider rescuing him.
         * First, bail if you're not a rescuing class.  Rangers ignored on principle.
         */
        if( !FIGHTING(the_mob) ||
                !( GET_CLASS(the_mob) == CLASS_WARRIOR
                || GET_CLASS(the_mob) == CLASS_DEATH_KNIGHT
                || GET_CLASS(the_mob) == CLASS_SOLAMNIC_KNIGHT) )
            return;

        if(!GET_SKILL(the_mob->master, SKILL_RESCUE )){

        }

        
        // If the master is a player and her hitpoints are greater than 25%, leave her alone.
        if( !IS_NPC(the_mob->master) )
            if ( ((100 * GET_HIT(the_mob->master)) / GET_MAX_HIT(the_mob->master)) > 25 )
                return;
        
        CharData *tmp_ch;
        for (tmp_ch = world[the_mob->in_room].people; tmp_ch && (FIGHTING(tmp_ch) != the_mob->master); tmp_ch = tmp_ch->next_in_room);
        if (tmp_ch && (number(1, 100) > 97)) /* lets help our master out */
        {
            int the_action = number(1, 10);

            // Death knights have something to say:
            if( GET_CLASS(the_mob) == CLASS_DEATH_KNIGHT )
                switch (the_action) {
                    case 1:
                        act("$n says, 'Prepare to meet thine maker $N!'", FALSE, the_mob, 0, tmp_ch, TO_ROOM);
                        break;
                    case 2:
                        act("$n screams, '&08MORE BLOOD!!!&00'", FALSE, the_mob, 0, tmp_ch, TO_ROOM);
                        break;
                    case 3:
                        act("$n says, 'This is another fine mess you have gotten me into $N.'", FALSE, the_mob, 0, the_mob->master, TO_ROOM);
                        break;
                    case 4:
                        act("$n says, '...and now, $N, you must deal with me!!'", FALSE, the_mob, 0, tmp_ch, TO_ROOM);
                        break;
                    case 5:
                        act("$n says, 'Your heart will make a nice after dinner treat, $N!'", FALSE, the_mob, 0, tmp_ch, TO_ROOM);
                        break;
                    default: /* Don't bother doing anything. */
                        break;
                }
                
                do_rescue(the_mob, the_mob->master->player.name, 0, 0);
                return;
        }
    }
}

/*
** smart_NPC_combat:
**
**        This function allows us to more easily write custom code to control
**    our mobs. The only hokey part to it is that the combat_vector array MUST
**    be in acsending order or the class index won't work. This may result in
**    a mage trying to bash.
**
**        It should be noted that races will eventually affect these funcs!
**
*/
void
smart_NPC_combat( CharData *the_mob )
{
    ACMD(do_stand);

    static combat_vector_t class_combat_vectors[] = {
        { CLASS_MAGIC_USER,      attack_as_Mu },
        { CLASS_CLERIC,          attack_as_Cl },
        { CLASS_THIEF,           attack_as_Th },
        { CLASS_WARRIOR,         attack_as_Wa },
        { CLASS_RANGER,          attack_as_Ra },
        { CLASS_ASSASSIN,        attack_as_As },
        { CLASS_SHOU_LIN,        attack_as_Sl },
        { CLASS_SOLAMNIC_KNIGHT, attack_as_Kn },
        { CLASS_DEATH_KNIGHT,    attack_as_De },
        { CLASS_SHADOW_DANCER,   attack_as_Sd },
        { CLASS_NECROMANCER,     attack_as_Nm },
        { CLASS_UNDEFINED,       attack_as_XX },
        { CLASS_UNDEFINED,       attack_as_XX },
        { CLASS_UNDEFINED,       attack_as_XX },
        { CLASS_UNDEFINED,       attack_as_XX },
        { CLASS_UNDEFINED,       0            }
    };/* class_combat_vectors */

    static combat_vector_t race_combat_vectors[] = {
	{ RACE_HUMAN,		attack_as_Human },
	{ RACE_PLANT,		attack_as_Plant },
	{ RACE_ANIMAL,		attack_as_Animal },
	{ RACE_DRAGON,		attack_as_Dragon },
	{ RACE_UNDEAD,		attack_as_Undead },
	{ RACE_VAMPIRE,		attack_as_Vampire },
	{ RACE_HALFLING,	attack_as_Halfling },
	{ RACE_ELF,		attack_as_Elf },
	{ RACE_DWARF,		attack_as_Dwarf },
	{ RACE_GIANT,		attack_as_Giant },
	{ RACE_MINOTAUR,	attack_as_Minotaur },
	{ RACE_DEMON,		attack_as_Demon },
	{ RACE_OGRE,		attack_as_Ogre },
	{ RACE_TROLL,		attack_as_Troll },
	{ RACE_WEREWOLF,	attack_as_Werewolf },
	{ RACE_ELEMENTAL,	attack_as_Elemental },
	{ RACE_ORC,		attack_as_Orc },
	{ RACE_GNOME,		attack_as_Gnome },
	{ RACE_DRACONIAN,	attack_as_Draconian },
	{ RACE_FAERIE,		attack_as_Faerie },
	{ RACE_AMARA,		attack_as_Amara },
	{ RACE_IZARTI,		attack_as_Izarti },
	{ RACE_DROW,		attack_as_Drow },
	{ RACE_SHUMAN,   	attack_as_Human },
	{ RACE_SHALFLING,	attack_as_Halfling },
	{ RACE_SELF,	        attack_as_Elf },
        { RACE_SDROW,	        attack_as_Drow },
        { RACE_SDWARF,	        attack_as_Dwarf },
        { RACE_SMINOTAUR,       attack_as_Minotaur },
        { RACE_SOGRE,	        attack_as_Ogre },
        { RACE_STROLL,	        attack_as_Troll },
        { RACE_SDRACONIAN,	attack_as_Draconian },
        { RACE_SGNOME,	        attack_as_Gnome },
        { RACE_SORC,	        attack_as_Orc },

	{ RACE_UNDEFINED,	attack_as_XX },
	{ RACE_UNDEFINED,	0 }
    };/* race_combat_vectors */

    CharData *the_pc;

    /* Bail out of here on any of the following: */
    if( !IS_NPC( the_mob ))                   return;

    if( STUNNED( the_mob ))                   return;
    else OFF_BALANCE(the_mob) = 0;

    if( IS_AFFECTED( the_mob, AFF_PARALYZE )) return;
    if( !FIGHTING( the_mob))                  return;

    // Pets will not take action if the master wants it to be passive.
    if(IS_AFFECTED(the_mob, AFF_CHARM) && !IS_NPC(the_mob->master)
            && PRF_FLAGGED(the_mob->master, PRF_PASSIVE_PET))
        return;

    if( GET_POS( the_mob ) <= POS_SITTING ) {
        do_stand( the_mob, "", 0, 0);
        return;
    }

    if( GET_CLASS(the_mob) < CLASS_MAGIC_USER ||
        GET_CLASS(the_mob) > CLASS_NECROMANCER )
        return;

    if( !(the_pc = find_mob_victim( the_mob ))) return; /* Bail if we don't have a victim */
    /* First, lets do any race specific attacks. */
    race_combat_vectors[ (int)GET_RACE(the_mob) ].attack_function( the_mob, the_pc );

    if (STUNNED(the_mob)) return; /* this would be the result of racial attack. */
    /* pc might be dead by now. */
    if( !(the_pc = find_mob_victim( the_mob ))) return; /* Bail if we don't have a victim */
	/* If we're an assistant, let's see if we can help our master */
	assist_master(the_mob, the_pc);

    if (STUNNED(the_mob)) return; /* this would be the result of rescuing. */
    /* pc might be dead by now. */
    if( !(the_pc = find_mob_victim( the_mob ))) return; /* Bail if we don't have a victim */

    /* Ok, we've found a victim, let's vector to a special proc */
    class_combat_vectors[ (int)GET_CLASS(the_mob) ].attack_function( the_mob, the_pc );

}/* smart_NPC_combat */



/*
** find_mob_victim
**
*/
static CharData *
find_mob_victim( CharData *the_mob )
{
#   define MOB_RESET_PERCENTAGE 35

    CharData *caster = the_mob->mob_specials.high_spellcaster;
    CharData *tanker = the_mob->mob_specials.high_attacker;
    CharData *victim;

    if(( caster && caster->in_room != the_mob->in_room ) || number(1, 100) < MOB_RESET_PERCENTAGE ) {
        the_mob->mob_specials.high_spellcaster = NULL;
        the_mob->mob_specials.max_spell_damage = 0;
    }

    if(( tanker && tanker->in_room != the_mob->in_room ) || number(1, 100) < MOB_RESET_PERCENTAGE ) {
        the_mob->mob_specials.high_attacker     = NULL;
        the_mob->mob_specials.max_attack_damage = 0;
    }

    victim = ( the_mob->mob_specials.max_spell_damage > the_mob->mob_specials.max_attack_damage ? caster : tanker );

    return(( victim ? victim : FIGHTING(the_mob)));

}/* find_mob_victim */

/*
** attack_as_XX
**
*/
static void
attack_as_XX( CharData *the_mob,
              struct char_data *the_pc )
{
#if 0
    /* IMPORTANT! - Vex */
    if (the_skill == SKILL_NULL)
      return;
#endif
}/* attack_as_XX */


/*
** attack_as_Cl
**
*/
static void
attack_as_Cl( CharData *the_mob,
              CharData *the_pc )
{
	int min_index   = ( GET_LEVEL(the_mob) <= MIN_LEVEL_SPREAD ? 1 : GET_LEVEL(the_mob) - MIN_LEVEL_SPREAD );
	int max_index   = ( GET_LEVEL(the_mob) >  MAX_MORTAL ? MAX_MORTAL : GET_LEVEL(the_mob));
	int spell_index = number( min_index, max_index );
	int the_spell   = attack_list_Cl[ spell_index ];
	int percent_HPs = (100 * GET_HIT(the_mob)) / GET_MAX_HIT(the_mob);

	/*  Vex, Febuary 17, 1997
	**  Improved cleric spell selection.
	*/
	if ((the_spell == SPELL_DISPEL_EVIL) && !IS_GOOD(the_mob))
		if IS_EVIL(the_mob)
			the_spell = SPELL_DISPEL_GOOD;
		else
			the_spell = SPELL_HARM;
	else if ((the_spell == SPELL_HOLY_WORD) && !IS_GOOD(the_mob))
		if IS_EVIL(the_mob)
			the_spell = SPELL_UNHOLY_WORD;
		else
			the_spell = SPELL_CHAIN_LIGHTNING;
	else if ((the_spell == SPELL_FLAME_STRIKE) && !IS_GOOD(the_mob))
		if IS_EVIL(the_mob)
			the_spell = SPELL_DEMON_FIRE;
		else
			the_spell = SPELL_HARM;
	else if ((the_spell == SPELL_BANISH) 
		&& !(IS_NPC(the_pc) && IS_SET_AR(MOB_FLAGS(the_pc), MOB_CONJURED)))
		the_spell = SPELL_HARM;

	/* Vex, Febuary 17, 1997
	** Made the way mobs heal them selves more random.
	*/
    if( number(1,70) >= percent_HPs){
		int level  = GET_LEVEL(the_mob);
		if( level >= 45 ){
			cast_spell( the_mob, the_mob, NULL, SPELL_REVIVE );
			return;
		}
		else if( level >= 20 ){
			cast_spell( the_mob, the_mob, NULL, SPELL_HEAL );
			return;
		}
		else if (level >= 10){
			cast_spell(the_mob, the_mob, NULL, SPELL_CURE_CRITIC );
			return;
		}
		else {
			cast_spell(the_mob, the_mob, NULL, SPELL_CURE_LIGHT );
			return;
		}
	}

    /* IMPORTANT! - Vex */
    if (the_spell == SKILL_NULL)
      return;

	use_move( the_mob, the_pc, the_spell);
}/* attack_as_Cl */


/*
** attack_as_Th
**
*/
static void
attack_as_Th( CharData *the_mob, CharData *the_pc )
{
    ACMD(do_stand);
    int min_index   = ( GET_LEVEL(the_mob) <= MIN_LEVEL_SPREAD ? 1 : GET_LEVEL(the_mob) - MIN_LEVEL_SPREAD );
    int max_index   = ( GET_LEVEL(the_mob) >  MAX_MORTAL ? MAX_MORTAL : GET_LEVEL(the_mob));
    int skill_index = number( min_index, max_index );
    int the_skill   = attack_list_Th[ skill_index ];

	/* IMPORTANT! - Vex */
    if (the_skill == SKILL_NULL)
      return;
	
	attempt_circle( the_mob, the_pc );
	// Stun would be due to the mob circling.
	if(STUNNED(the_mob))
		return;

    if (the_skill == SKILL_FEIGN_DEATH)
        feign_death_attack(the_mob, the_pc);
    else
        use_move( the_mob, the_pc, the_skill );
        
}/* attack_as_Th */

static void
attack_as_Ra( CharData *the_mob,
              CharData *the_pc )
{
    int min_index   = ( GET_LEVEL(the_mob) <= MIN_LEVEL_SPREAD ? 1 : GET_LEVEL(the_mob) - MIN_LEVEL_SPREAD );
    int max_index   = ( GET_LEVEL(the_mob) >  MAX_MORTAL ? MAX_MORTAL : GET_LEVEL(the_mob));
    int spell_index = number( min_index, max_index );
    int the_skill   = attack_list_Ra[ spell_index ];

	/* IMPORTANT! - Vex */
    if (the_skill == SKILL_NULL)
      return;

	use_move(the_mob, the_pc, the_skill);
}/* attack_as_Ra */



static void
attack_as_As( CharData *the_mob,
              CharData *the_pc )
{
    int min_index   = ( GET_LEVEL(the_mob) <= MIN_LEVEL_SPREAD ? 1 : GET_LEVEL(the_mob) - MIN_LEVEL_SPREAD );
    int max_index   = ( GET_LEVEL(the_mob) >  MAX_MORTAL ? MAX_MORTAL : GET_LEVEL(the_mob));
    int skill_index = number( min_index, max_index );
    int the_skill   = attack_list_As[ skill_index ];

    /* IMPORTANT! - Vex */
    if (the_skill == SKILL_NULL)
      return;

	attempt_circle( the_mob, the_pc );
	// Stun would be due to mob circling.
	if( STUNNED( the_mob )) 
		return;

	use_move( the_mob, the_pc, the_skill );
}/* attack_as_As */


/*
** attack_as_Sl
**
*/
static void
attack_as_Sl( CharData *the_mob,
              CharData *the_pc )
{
    int min_index   = ( GET_LEVEL(the_mob) <= MIN_LEVEL_SPREAD ? 1 : GET_LEVEL(the_mob) - MIN_LEVEL_SPREAD );
    int max_index   = ( GET_LEVEL(the_mob) >  MAX_MORTAL ? MAX_MORTAL : GET_LEVEL(the_mob));
    int skill_index = number( min_index, max_index );
    int the_skill   = attack_list_Sl[ skill_index ];
    int percent_HPs;

    if(GET_MAX_HIT(the_mob) != 0)
         percent_HPs = (100 * GET_HIT(the_mob)) / GET_MAX_HIT(the_mob);
    else
        percent_HPs = 100;

    if( number(1,70) >= percent_HPs ) {
        do_hands( the_mob, the_mob->player.name, 0, 0 );
        return;
    }

    // Occassionally, let's check our stance.
    if(!number(0, 15)) {
        choose_aspect(the_mob);
        return;
    }

    /* IMPORTANT! - Vex */
    if (the_skill == SKILL_NULL)
      return;

    if (the_skill == SKILL_FEIGN_DEATH)
        feign_death_attack(the_mob, the_pc);
    else
        use_move( the_mob, the_pc, the_skill );
}/* attack_as_Sl */


/*
** attack_as_Kn
**
*/

static void
attack_as_Kn( CharData *the_mob, CharData *the_pc )
{
	int min_index   = ( GET_LEVEL(the_mob) <= MIN_LEVEL_SPREAD ? 1 : GET_LEVEL(the_mob) - MIN_LEVEL_SPREAD );
	int max_index   = ( GET_LEVEL(the_mob) >  MAX_MORTAL ? MAX_MORTAL : GET_LEVEL(the_mob));
	int spell_index = number( min_index, max_index );
	int the_skill   = attack_list_Kn[ spell_index ];
	int percent_HPs = (100 * GET_HIT(the_mob)) / GET_MAX_HIT(the_mob);

	if (number(1, 70) >= percent_HPs)
	{
		int level = GET_LEVEL(the_mob);
		if (level >= 45)
		{
			cast_spell(the_mob, the_mob, NULL, SPELL_PRAYER_OF_LIFE);
			return;
		}
		else if (level >= 35)
		{
			cast_spell(the_mob, the_mob, NULL, SPELL_CURE_CRITIC);
			return;
		}
		else if (level >= 25)
		{
			cast_spell(the_mob, the_mob, NULL, SPELL_CURE_SERIOUS);
			return;
		}
		else if (level >= 10)
		{
			cast_spell(the_mob, the_mob, NULL, SPELL_CURE_LIGHT);
			return;
		}
	}

	/* IMPORTANT! - Vex */
	if (the_skill == SKILL_NULL)
		return;

	/* find who to retarget to */
	if (the_skill == SKILL_RETARGET) {
		CharData *ch = world[the_mob->in_room].people;
		CharData *infidel = NULL;

		if(number(0, 2)) {
			while (ch) {
				if (FIGHTING(ch) == the_mob && IS_EVIL(ch) && !IS_NPC(ch) &&
					(!infidel || GET_ALIGNMENT(ch) < GET_ALIGNMENT(infidel)) &&
					FIGHTING(the_mob) != ch)
					infidel = ch;
				ch = ch->next_in_room;
			}
			if (infidel) {
				sprintf(buf2, "Prepare to face my holy wrath, %s!",	GET_NAME(infidel));
				do_say(the_mob, buf2, 0, 0);
				do_retarget(the_mob, GET_NAME(infidel), 0, 0);
				return;
			}
		}  

		while (ch) {
			if (FIGHTING(ch) == the_mob && !IS_NPC(ch) &&
				(!infidel || GET_HIT(ch) < GET_HIT(infidel)) &&
				FIGHTING(the_mob) != ch)
				infidel = ch;
			ch = ch->next_in_room;
		}
		if (infidel) {
			if(number(0, 1))
				sprintf(buf2, "Prepare to be met an untimely demise, %s!", GET_NAME(infidel));
			else
				sprintf(buf2, "Prepare to meet our maker, %s!",	GET_NAME(infidel));
			do_say(the_mob, buf2, 0, 0);
			do_retarget(the_mob, GET_NAME(infidel), 0, 0);
			return;
		}
		return;
	} //the_skill == retarget

	use_move( the_mob, the_pc, the_skill );
}/* attack_as_Kn */


/*
** attack_as_De
**
*/
static void
attack_as_De( CharData *the_mob,
              CharData *the_pc )
{
    int min_index   = ( GET_LEVEL(the_mob) <= MIN_LEVEL_SPREAD ? 1 : GET_LEVEL(the_mob) - MIN_LEVEL_SPREAD );
    int max_index   = ( GET_LEVEL(the_mob) >  MAX_MORTAL ? MAX_MORTAL : GET_LEVEL(the_mob));
    int spell_index = number( min_index, max_index );
    int the_skill   = attack_list_De[ spell_index ];

    /* IMPORTANT! - Vex */
    if (the_skill == SKILL_NULL)
      return;

    /* see if a devour is viable on one of the attackers in the room */
    if (the_skill == SKILL_DEVOUR) {
		int dam = 2 * (calculateSimpleDamage(the_mob) - 20);
		CharData *ch = world[the_mob->in_room].people, *lunch = NULL;

		while (ch) {
			if (FIGHTING(ch) == the_mob && GET_HIT(ch) < dam && !IS_NPC(ch) &&
				(!lunch || GET_HIT(ch) < GET_HIT(lunch)))
				lunch = ch;
			ch = ch->next_in_room;
		}

		if (lunch) {
			sprintf(buf2, "Your soul will feed my rage, %s!", GET_NAME(lunch));
			do_say(the_mob, buf2, 0, 0);
			do_devour(the_mob, GET_NAME(lunch), 0, 0);
			return;
		}
		
		return;
    }

	/*
	if( the_skill == SKILL_BLOCK) {
		do_block( the_mob, 0, 0, 0);
		return;
	}
	*/

	use_move( the_mob, the_pc, the_skill );
} /* attack_as_De */

/*
** attack_as_Sd
**
*/
static void
attack_as_Sd( CharData *the_mob,
              CharData *the_pc )
{
    int min_index   = ( GET_LEVEL(the_mob) <= MIN_LEVEL_SPREAD ? 1 : GET_LEVEL(the_mob) - MIN_LEVEL_SPREAD );
    int max_index   = ( GET_LEVEL(the_mob) >  MAX_MORTAL ? MAX_MORTAL : GET_LEVEL(the_mob));
    int spell_index = number( min_index, max_index );
    int the_skill   = attack_list_Sd[ spell_index ];

    /* IMPORTANT! - Vex */
    if (the_skill == SKILL_NULL)
      return;
	
	use_move( the_mob, the_pc, the_skill );
}/* attack_as_Sd */


/*
** attack_as_Wa
** Since warriors have so few moves available, I kept this function its 
** classical version.  Work could be done here.  Craklyn
*/
static void
attack_as_Wa( CharData *the_mob,
              CharData *the_pc )
{
    int rand  = number(1, 5);

    if ( (rand == 5) && (GET_LEVEL(the_mob) >= 32) )
    {
        do_shieldbash(the_mob, "", 0, 0);
        return;
    }
    else if ((GET_LEVEL(the_mob) >= 45) && (rand == 4) && !IS_AFFECTED(the_mob, AFF_BERSERK)) {
        do_berserk(the_mob, "", 0, 0);
        return;
    }
    else if ( (GET_LEVEL(the_mob) >= 45) && (rand == 4) && CAN_SEE(the_mob, the_pc) && !number(0,2) ) {
        do_berserk(the_mob, "", 0, 0);
        return;
    }
    else if( rand == 3 && GET_LEVEL(the_mob) >= 9 )	{
        do_disarm( the_mob, "", 0, 0 );
    }
    else if( rand == 2 )
        do_bash( the_mob, "", 0, 0 );
    else if( rand == 1 )
        do_kick( the_mob, "", 0, 0 );
    
}/* attack_as_Wa */


/*
** attack_as_Mu
**
**      This function is designed to let mobs act like players by providing
**   a range of spells to select from rather than one specific spell based on
**   the mob's level. The static array, attack_list, contains a list of spells
**   that a mob has access to and is indexed into via the mob's level. Radomicity
**   is achieved by calling the random function with a range of the mob's level
**   minus the spread, upto the mob's level. This will give the mobs a range of
**   spells to work with without limitting the high level mobs to such things as
**   magic missile, or chill touch.
**
*/
static void
attack_as_Mu( CharData *the_mob,
              CharData *the_pc )
{
    int min_index   = ( GET_LEVEL(the_mob) <= MIN_LEVEL_SPREAD ? 1 : GET_LEVEL(the_mob) - MIN_LEVEL_SPREAD );
    int max_index   = ( GET_LEVEL(the_mob) >  MAX_MORTAL ? MAX_MORTAL : GET_LEVEL(the_mob));
    int spell_index = number( min_index, max_index );
    int the_spell   = attack_list_Mu[ spell_index ];

	/* IMPORTANT! - Vex */
	if (the_spell == SKILL_NULL)
		return;
	
	use_move( the_mob, the_pc, the_spell);
}/* attack_as_Mu */

/*
** attack_as_Nm
**
*/
static void
attack_as_Nm(CharData *the_mob,
             CharData *the_pc)
			 
{
    int min_index   = ( GET_LEVEL(the_mob) <= MIN_LEVEL_SPREAD ? 1 : GET_LEVEL(the_mob) - MIN_LEVEL_SPREAD );
    int max_index   = ( GET_LEVEL(the_mob) >  MAX_MORTAL ? MAX_MORTAL : GET_LEVEL(the_mob));
    int spell_index = number( min_index, max_index );
    int the_spell   = attack_list_Nm[ spell_index ];

    /* IMPORTANT! - Vex */
    if (the_spell == SKILL_NULL)
        return;
    
    use_move( the_mob, the_pc, the_spell);
}/* attack_as_Nm */

static void
attack_as_Human (CharData *the_mob, CharData *the_pc)
{
}

static void
attack_as_Plant (CharData *the_mob, CharData *the_pc)
{
}

static void
attack_as_Animal (CharData *the_mob, CharData *the_pc)
{
}

/* This function picks a chromatic dragons breath weapon. */
/* assumes that not all of chromatic_breath are TRUE... */
int select_breath(bool chromatic_breath[5])
{
	int the_breath;

	do 	{
		the_breath = number(0, 4);
	} while (chromatic_breath[the_breath]);
	chromatic_breath[the_breath] = TRUE;
	switch(the_breath) {
		case 0: return SPELL_FIRE_BREATH;
		case 1: return SPELL_GAS_BREATH;
		case 2: return SPELL_FROST_BREATH;
		case 3: return SPELL_ACID_BREATH;
		case 4: return SPELL_LIGHTNING_BREATH;
		default: /* this should never happen. */
			return SPELL_FIRE_BREATH;
	}
}

static void
attack_as_Dragon (CharData *the_mob, CharData *the_pc)
{
#define BREATH_CHANCE 30
    bool chromatic_breath[5];
    int i, num_breaths;

	if (number(1,100) >= BREATH_CHANCE) 
	{
		WAIT_STATE(the_mob, 2 * PULSE_VIOLENCE);
		/* breath on them. */
		switch (GET_SUBRACE(the_mob)) 
		{
		case RED_DRAGON:
		case GOLD_DRAGON:	    
			cast_spell(the_mob, the_pc, NULL, SPELL_FIRE_BREATH);
			break;
		case GREEN_DRAGON:
		case SHADOW_DRAGON:
		case MIST_DRAGON:
		case BRASS_DRAGON:
			cast_spell(the_mob, the_pc, NULL, SPELL_GAS_BREATH);
			break;
		case WHITE_DRAGON:
		case SILVER_DRAGON:
			cast_spell(the_mob, the_pc, NULL, SPELL_FROST_BREATH);
			break;
		case BLACK_DRAGON:
		case COPPER_DRAGON:
			cast_spell(the_mob, the_pc, NULL, SPELL_ACID_BREATH);
			break;
		case BLUE_DRAGON:
		case BRONZE_DRAGON:
			cast_spell(the_mob, the_pc, NULL, SPELL_LIGHTNING_BREATH);
			break;
		case CHROMATIC_DRAGON:
			for (i = 0; i < 5; i++) chromatic_breath[i] = FALSE;
			num_breaths = number(1, 3);
			for (i = 1; i <= num_breaths; i++) {
				cast_spell(the_mob, the_pc, NULL, select_breath(chromatic_breath));
				if (!(the_pc = find_mob_victim(the_mob))) return; /* they are all dead. */
			}
			break;
		case 0: break;
		default:
			mudlog(NRM, LVL_IMMORT, TRUE, "mob[%d] has unknown Dragon subrace %d.",
				GET_MOB_VNUM(the_mob), GET_SUBRACE(the_mob));
		} /* switch */
		WAIT_STATE(the_mob, 2 * PULSE_VIOLENCE);
	} /* if */
#undef BREATH_CHANCE
} /* attack_as_Dragon */

static void
attack_as_Undead (CharData *the_mob, CharData *the_pc)
{
	if (number(1, 100) > 90)
	{
		if(IS_RACIAL(the_mob))
		{
			say_spell(the_mob, SPELL_TERROR, the_pc, NULL);
			call_magic(the_mob, the_pc, NULL, SPELL_TERROR, GET_LEVEL(the_mob), NULL, CAST_SPELL);
		}
		WAIT_STATE(the_mob, PULSE_VIOLENCE);
	}
}

static void
attack_as_Vampire (CharData *the_mob, CharData *the_pc)
{
    if (number(1,100) > 90) do_feed( the_mob, "", 0, 0 );
}

static void
attack_as_Halfling (CharData *the_mob, CharData *the_pc)
{
}

static void
attack_as_Elf (CharData *the_mob, CharData *the_pc)
{
}

static void
attack_as_Drow (CharData *the_mob, CharData *the_pc)
{
}

static void
attack_as_Dwarf (CharData *the_mob, CharData *the_pc)
{
}

static void
attack_as_Giant (CharData *the_mob, CharData *the_pc)
{
}

static void
attack_as_Minotaur (CharData *the_mob, CharData *the_pc)
{
	if (number(1,100) > 90) do_gore( the_mob, "", 0, 0 );
}

static void
attack_as_Demon (CharData *the_mob, CharData *the_pc)
{
}

static void
attack_as_Ogre (CharData *the_mob, CharData *the_pc)
{
}

static void
attack_as_Troll (CharData *the_mob, CharData *the_pc)
{
}

static void
attack_as_Werewolf (CharData *the_mob, CharData *the_pc)
{
}

static void
attack_as_Elemental (CharData *the_mob, CharData *the_pc)
{
	if (number(1,100) > 70) 
	{
		// Whether they cast or lose concentration, they have to wait.
		WAIT_STATE(the_mob, PULSE_VIOLENCE);
		// IS_RACIAL includes the check for skillsuccess
		if (IS_RACIAL(the_mob) )
			switch (GET_SUBRACE(the_mob)) {
			case FIRE_ELEMENTAL:
				//cast_spell(the_mob, the_pc, NULL, SPELL_WALL_OF_FIRE);
				// Since the above code didn't work, I created the following
				// hack, which has faults, but gets the job done. -Craklyn
				say_spell(the_mob, SPELL_WALL_OF_FIRE, the_pc, NULL);
				call_magic(the_mob, the_pc, NULL, SPELL_WALL_OF_FIRE, GET_LEVEL(the_mob), NULL, CAST_SPELL);
				WAIT_STATE(the_mob, PULSE_VIOLENCE);
				break;
			case AIR_ELEMENTAL:
				say_spell(the_mob, SPELL_TYPHOON, the_pc, NULL);
				call_magic(the_mob, the_pc, NULL, SPELL_TYPHOON, GET_LEVEL(the_mob), NULL, CAST_SPELL);
				WAIT_STATE(the_mob, PULSE_VIOLENCE);
				break;
			case EARTH_ELEMENTAL:
				say_spell(the_mob, SPELL_TREMOR, the_pc, NULL);
				call_magic(the_mob, the_pc, NULL, SPELL_TREMOR, GET_LEVEL(the_mob), NULL, CAST_SPELL);
				WAIT_STATE(the_mob, PULSE_VIOLENCE);
				break;
			case WATER_ELEMENTAL:
				say_spell(the_mob, SPELL_TSUNAMI, the_pc, NULL);
				call_magic(the_mob, the_pc, NULL, SPELL_TSUNAMI, GET_LEVEL(the_mob), NULL, CAST_SPELL);
				WAIT_STATE(the_mob, PULSE_VIOLENCE);
				break;
		}
	}
}

static void
attack_as_Orc (CharData *the_mob, CharData *the_pc)
{
}

static void
attack_as_Gnome (CharData *the_mob, CharData *the_pc)
{
}

static void
attack_as_Draconian (CharData *the_mob, CharData *the_pc)
{
	if (number(1,100) > 90) do_breathe( the_mob, "", 0, 0 );
}

static void
attack_as_Faerie (CharData *the_mob, CharData *the_pc)
{
}

static void
attack_as_Amara (CharData *the_mob, CharData *the_pc)
{
	if (number(1,100) > 90) do_sting( the_mob, "", 0, 0 );
}

static void
attack_as_Izarti (CharData *the_mob, CharData *the_pc)
{
}

static void
feign_death_attack(CharData *the_mob, CharData *the_pc) {
    CharData *ch = world[the_mob->in_room].people;
    CharData *target = NULL;
    int r = 0;

    // A blind mob will somehow attack the player while blind.  To prevent this,
    // just stop them here.
    if(!CAN_SEE(the_mob, the_pc))
        return;

    // There are currently three ways for the mob to decide who to attack after the
    // feign.  Increase the number below as more are added.
    int choice_method = number(0, 2);

    while (ch) {
        if(choice_method == 0) {
            // Just have the mobile re-attack the tank
            target = FIGHTING(ch);
        }
        else if(choice_method == 1) {
            // Select someone fighting the mob, prefering those with
            // less hp's and near the bottom of the room list.
            if(FIGHTING(ch) == the_mob && ch != the_mob && !IS_NPC(ch) && CAN_SEE(the_mob, ch) &&
                    (!target ||  number(1, GET_HIT(ch)) < number(1, GET_HIT(target))) )
                target = ch;
        }
        else {
            // Select randomly among those attacking the_mob
            if(FIGHTING(ch) == the_mob && ch != the_mob && !IS_NPC(ch) && CAN_SEE(the_mob, ch) &&
                    (number(1, ++r) == 1 ) )
                target = ch;
        }

        ch = ch->next_in_room;
    }

    if(target) {
        if (!IS_SHOU_LIN(the_mob) && WIELDING(the_mob) &&
                (GET_OBJ_VAL(the_mob->equipment[WEAR_WIELD], 3) == (TYPE_PIERCE - TYPE_HIT)))
        {
            /* Play dead, then backstab 'em, this will get 'em hoppin! */
            do_feign_death(the_mob, "", 0, 0);
            if(!STUNNED(the_mob)) {
                do_stand(the_mob, "", 0, 0);
                do_backstab(the_mob, target->player.name, 0, 0);
            }
        }
        else {
            do_feign_death(the_mob, "", 0, 0);
            if(!STUNNED(the_mob)) {
                do_stand(the_mob, "", 0, 0);
                hit(the_mob, target, TYPE_UNDEFINED);
            }
        }
    }
}

void choose_aspect(CharData *the_mob) {
    
    int windCount = 0, snakeCount = 0, monkeyCount = 0, craneCount = 0, flowerCount = 0;
    int i;
    
    if(GET_LEVEL(the_mob) >= 20)
        windCount = 5;
    
    if(GET_LEVEL(the_mob) >= 30)
        snakeCount = windCount + attackers(the_mob) == 1? 35 : 5;
    
    if(GET_LEVEL(the_mob) >= 35)
        monkeyCount = snakeCount + 15;
    
    if(GET_LEVEL(the_mob) >= 40)
        craneCount = monkeyCount + MAX(35 - 5*attackers(the_mob), 0);
    
    if(GET_LEVEL(the_mob) >= 45)
        flowerCount = craneCount + 3*attackers(the_mob);
    
    i = number(0, flowerCount - 1);

    if(i < windCount && GET_ASPECT(the_mob) != ASPECT_WIND)
        do_aspect(the_mob, "wind", 0, 0);
    else if(i < snakeCount && GET_ASPECT(the_mob) != ASPECT_SNAKE)
        do_aspect(the_mob, "snake", 0, 0);
    else if(i < monkeyCount && GET_ASPECT(the_mob) != ASPECT_MONKEY)
        do_aspect(the_mob, "monkey", 0, 0);
    else if(i < craneCount && GET_ASPECT(the_mob) != ASPECT_CRANE)
        do_aspect(the_mob, "crane", 0, 0);
    else if(i < flowerCount && GET_ASPECT(the_mob) != ASPECT_FLOWER)
        do_aspect(the_mob, "flower", 0, 0);

}

#if 0
SPELL_CHARM: Mu8 Sd32
SPELL_CLONE:
SPELL_CURE_BLIND: Cl4  Kn33
SPELL_DETECT_INVIS: Mu2  Cl6  Kn13  Dk13  Sd8
SPELL_INFRAVISION: Mu11 Cl7 Ra9 As25 Sd9
SPELL_INVISIBLE: My2 Sd3
SPELL_PROT_FROM_EVIL: Cl11 Kn10
SPELL_SANCTUARY: Cl18
SPELL_STRENGTH: Mu9 Ra21
SPELL_SENSE_LIFE: Cl5  Ra18  
SPELL_FLY: Mu12 Cl8
SPELL_BARKSKIN: Ra12
SPELL_STONESKIN: Mu15 Cl22 Ra30
SPELL_REGENERATE: Cl15 Kn28 Dk28
SPELL_PORTAL: Mu30
SPELL_PROT_FROM_GOOD: Cl11 Dk10
SPELL_SHIELD: Mu10 Ra18
SPELL_ARMOR_OF_CHAOS: Dk30
SPELL_HOLY_ARMOR: Kn30
SPELL_HASTE: Mu11 Sd45
SPELL_ANIMATE_DEAD: Dk6
SPELL_GROUP_SANCTUARY: Cl40
SPELL_REFRESH: Ra35
SPELL_TRUE_SIGHT: Mu31
SPELL_RELOCATE: Cl45
SPELL_GROUP_HASTE: Mu40
SPELL_BLUR: Sd35
SKILL_BACKSTAB: Th5 As1 Sd10
SKILL_HIDE: Th4 Ra3 As2 Sd5
SKILL_RESCUE: Wa2 Ra20 Kn5 Dk5
SKILL_RETREAT: Wa15 Ra5 Sl1 Kn15 Dk15
SKILL_FLASHBANG: Th46
#endif
