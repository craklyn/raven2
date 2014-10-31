#ifndef __SPELLS_H__
#define __SPELLS_H__
/* ************************************************************************
*   File: spells.h                                      Part of CircleMUD *
*  Usage: header file: constants and fn prototypes for spell system       *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 199, 1991.               *
************************************************************************ */

#include "general/class.h"

#define DEFAULT_STAFF_LVL	12
#define DEFAULT_WAND_LVL	12

#define CAST_UNDEFINED	-1
#define CAST_SPELL	0
#define CAST_POTION	1
#define CAST_WAND	2
#define CAST_STAFF	3
#define CAST_SCROLL	4
#define CAST_FOOD       5

#define MAG_DAMAGE      (1 << 0)
#define MAG_AFFECTS     (1 << 1)
#define MAG_UNAFFECTS	(1 << 2)
#define MAG_POINTS      (1 << 3)
#define MAG_ALTER_OBJS	(1 << 4)
#define MAG_GROUPS      (1 << 5)
#define MAG_MASSES      (1 << 6)
#define MAG_AREAS       (1 << 7)
#define MAG_SUMMONS     (1 << 8)
#define MAG_CREATIONS	(1 << 9)
#define MAG_MANUAL      (1 << 10)

#define SST_FIGHTING      (1 << 0)        // stop if in combat
#define SST_MOVEMENT      (1 << 1)        // stop if you move

/* magically created objects */
#define PORTAL_OBJ_VNUM 1299
#define BONE_WALL_OBJ_VNUM 3

/*
** JBP - Additions made for auto skill advancements.
*/
#define MAX_PRACTICE_LEVEL 60

#define TYPE_UNDEFINED               -1
#define SPELL_RESERVED_DBC            0  /* SKILL NUMBER ZERO -- RESERVED */

// A tick is 72 seconds. Changing code to make spells length have 
// more possible lengths. - Craklyn
#define TICKS  * 72


/* PLAYER SPELLS -- Numbered from 1 to MAX_SPELLS */

#define SKILL_NULL                    0
#define SPELL_NULL                    0
#define SPELL_ARMOR                   1 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_TELEPORT                2 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BLESS                   3 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BLINDNESS               4 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BURNING_HANDS           5 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CALL_LIGHTNING          6 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CHARM                   7 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CHILL_TOUCH             8 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CLONE                   9 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_COLOR_SPRAY            10 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CONTROL_WEATHER        11 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CREATE_FOOD            12 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CREATE_WATER           13 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_BLIND             14 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_CRITIC            15 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_LIGHT             16 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURSE                  17 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_ALIGN           18 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_INVIS           19 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_MAGIC           20 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_POISON          21 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DISPEL_EVIL            22 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_EARTHQUAKE             23 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_ENCHANT_WEAPON         24 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_LIFE_DRAIN             25 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_FIREBALL               26 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_HARM                   27 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_HEAL                   28 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_INVISIBLE              29 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_LIGHTNING_BOLT         30 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_LOCATE_OBJECT          31 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_MAGIC_MISSILE          32 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_POISON                 33 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_PROT_FROM_EVIL         34 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_REMOVE_CURSE           35 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SANCTUARY              36 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SHOCKING_GRASP         37 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SLEEP                  38 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_STRENGTH               39 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SUMMON                 40 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_VENTRILOQUATE          41 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_WORD_OF_RECALL         42 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_REMOVE_POISON          43 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SENSE_LIFE             44 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_ANIMATE_DEAD	   	 	 45 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DISPEL_GOOD	     	 46 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_GROUP_ARMOR	     	 47 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_GROUP_HEAL	     	 48 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_GROUP_RECALL	     	 49 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_INFRAVISION	     	 50 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SHADOW_WALK	     	 51 /* Reserved Skill[] DO NOT CHANGE */
/* Insert new spells here, up to MAX_SPELLS */
#define SPELL_FLY		     	 	 52 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BARKSKIN		     	 53 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_AWAKEN		     	 54 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_REGENERATE	     	 55 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_ICE_STORM		     	 56
#define SPELL_METEOR_SWARM	     	 57
#define SPELL_PORTAL		     	 58
#define SPELL_CAUSE_WOUND	     	 59
#define SPELL_MALEDICT  	     	 60
#define SPELL_CAUSE_CRITIC	     	 61
#define SPELL_CURE_SERIOUS	     	 62
#define SPELL_CALM                   63
#define SPELL_CHAIN_LIGHTNING	 	 64
#define SPELL_BANISH		     	 65
#define SPELL_DEMON_FIRE	     	 66
#define SPELL_FLAME_STRIKE	     	 67
#define SPELL_PROT_FROM_GOOD	 	 68
#define SPELL_SHIELD		     	 69
#define SPELL_ARMOR_OF_CHAOS	 	 70
#define SPELL_HOLY_ARMOR	     	 71
#define SPELL_CREATE_SPRING	     	 72
#define SPELL_MINOR_CREATION	 	 73
#define SPELL_SAND_STORM	     	 74
#define SPELL_SILENCE		     	 75
#define SPELL_PARALYZE		     	 76
#define SPELL_HASTE                  77
#define SPELL_SLOW                   78
#define SPELL_SHRIEK		     	 79
#define SPELL_MONSTER_SUMMON	 	 80
#define SPELL_REMOVE_PARALYSIS	 	 81
#define SPELL_CONJURE_ELEMENTAL	 	 82
#define SPELL_GATE                       83
#define SPELL_GATE_MAJOR_DEMON	 	 84
#define SPELL_FORGET		     	 85
#define SPELL_GROUP_SANCTUARY	 	 86
#define SPELL_REFRESH                87
#define SPELL_TRUE_SIGHT             88
#define SPELL_BALL_OF_LIGHT          89
#define SPELL_FEAR                   90
#define SPELL_FLEET_FOOT             91
#define SPELL_HANDS_OF_WIND          92
#define SPELL_PLAGUE                 93
#define SPELL_CURE_PLAGUE            94
#define SPELL_RELOCATE               95
#define SPELL_GROUP_HASTE            96
#define SPELL_BLUR                   97
#define SPELL_REVIVE                 98
#define SPELL_CHANGE_ALIGN           99
#define SPELL_CALL_OF_THE_WILD       100
#define SPELL_DOOM_BOLT              101
#define SPELL_WRATH_OF_THE_ANCIENTS  102
#define SPELL_PRAYER_OF_LIFE         103
#define SPELL_HOLY_WORD              104
#define SPELL_UNHOLY_WORD            105
#define SPELL_BLACK_DART             106
#define SPELL_BLACK_BREATH           107
#define SPELL_DEATH_TOUCH            108
#define SPELL_EYES_OF_THE_DEAD       109
#define SPELL_RIGHTEOUS_VISION       110
#define SPELL_WARD                   111
#define SPELL_GROUP_WARD             112
#define SPELL_SHADOW_VISION          113
#define SPELL_SHADOW_BLADES          114
#define SPELL_SHADOW_SPHERE          115
#define SPELL_GROUP_SHADOW_VISION    116
#define SPELL_IDENTIFY               117
#define SPELL_GROUP_INVISIBILITY     118
#define SPELL_GROUP_FLY              119
#define SPELL_AIRSPHERE		         120
#define SPELL_NO_HOT		         121
#define SPELL_NO_DRY		         122
#define SPELL_NO_COLD		         123
#define SPELL_WEB                    124
#define SPELL_BLINK                  125
// New Spells -Memnoch 01-21-01
#define SPELL_PESTILENCE             126
#define SPELL_ENTANGLE               127
// New Spells -Memnoch 01-22-01
#define SPELL_FEEBLEMIND             128
#define SPELL_FLAME_BLADE            129
#define SPELL_NEXUS                  130
// New Spells -Imhotep
#define SPELL_ASSISTANT              131
#define SPELL_SAGACITY               132
#define SPELL_BRILLIANCE             133
#define SPELL_CLEANSE                134
#define SPELL_FORTIFY                135
#define SPELL_DISHEARTEN             136
#define SPELL_SACRIFICE              137
#define SPELL_PULSE_HEAL             138
#define SPELL_PULSE_GAIN             139
#define SPELL_DANCE_SHADOWS          140
#define SPELL_DANCE_DREAMS           141
#define SPELL_DANCE_MISTS            142
#define SPELL_CRUSADE                143
#define SPELL_UNUSED                 144
#define SPELL_APOCALYPSE             145
#define SPELL_MISSION                146
#define SPELL_FOREST_LORE            147
#define SPELL_SWARM                  148
#define SPELL_WALL_OF_FIRE           149
#define SPELL_TREMOR                 150
#define SPELL_TSUNAMI                151
#define SPELL_TYPHOON                152
#define SPELL_TERROR                 153
#define SPELL_FAST_LEARNING          154
#define SPELL_DISPEL_MAGIC           155
#define SPELL_CONSUME_CORPSE         156
#define SPELL_EXPLODE_CORPSE         157
#define SPELL_BONE_WALL              158
#define SPELL_CREATE_WARDING         159
#define SPELL_WRAITHFORM             160
#define SPELL_NOXIOUS_SKIN           161
#define SPELL_DISEASE                162
#define SPELL_SUMMON_CORPSE          163
#define SPELL_EMBALM                 164
#define SPELL_CHARM_CORPSE           165
#define SPELL_RESIST_POISON          166
#define SPELL_AGE	             167
#define SPELL_ENERGY_DRAIN           168
#define SPELL_SOUL_PIERCE            169
#define SPELL_DEBILITATE             170

/* epic spells */
#define SPELL_FOUNT_OF_YOUTH         171
#define SPELL_CALL_STEED             172
#define SPELL_REFLECTION             173
#define SPELL_FLETCH                 174

#define SPELL_ENTOMB_CORPSE          175
#define SPELL_CALL_TO_CORPSE         176
#define SPELL_QUICKEN                177

#define IS_REMORT_SPELL(s)           ((s) >= SPELL_WALL_OF_FIRE && \
                                      (s) <= SPELL_TERROR)

/* Keep this up to date. Should be one larger than the highest spell that */
/* is currently defined(to take into account "0". */
#define NUM_SPELLS          177
#define MAX_SPELLS          180

/* PLAYER SKILLS - Numbered from MAX_SPELLS+1 to MAX_SKILLS */
#define SKILL_BACKSTAB              181 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_BASH                  182 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_HIDE                  183 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_KICK                  184 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_PICK_LOCK             185 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_PUNCH                 186 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_RESCUE                187 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_SNEAK                 188 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_STEAL                 189 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_TRACK                 190 /* Reserved Skill[] DO NOT CHANGE */

/* New skills may be added here up to MAX_SKILLS (300) */
#define SKILL_DISARM                191
#define SKILL_SECOND_ATTACK         192
#define SKILL_THIRD_ATTACK          193
#define SKILL_SCAN                  194
#define SKILL_LAY_HANDS             195
#define SKILL_FISTS_OF_FURY         196
#define SKILL_THROW                 197
#define SKILL_SHOOT                 198
#define SKILL_KNOCK                 199
#define SKILL_TRIP                  200
#define SKILL_BLINDING_STRIKE       201
#define SKILL_HAMSTRING             202
#define SKILL_ENHANCED_DAMAGE       203
#define SKILL_RETREAT               204
#define SKILL_TURN                  205
#define SKILL_BUTCHER               206
#define SKILL_DODGE                 207
#define SKILL_TRAP                  208
#define SKILL_SEARCH_TRAP           209
#define SKILL_PALM                  210
#define SKILL_FIND_WEAKNESS         211
#define SKILL_SKIN                  212
#define SKILL_FEIGN_DEATH           213
#define SKILL_ART_DRAGON            214
#define SKILL_ART_SNAKE             215
#define SKILL_ART_TIGER             216
#define SKILL_ART_CRANE             217
#define SKILL_CIRCLE                218
#define SKILL_DUST                  219
#define SKILL_STALK                 220
#define SKILL_ART_WIND              221
#define SKILL_ENVENOM               222
#define SKILL_PARRY                 223
#define SKILL_SWEEP                 224
#define SKILL_DOORBASH              225
#define SKILL_PENUMBRAE             226
#define SKILL_PICKPOCKET            227
#define SKILL_APPRAISE              228
#define SKILL_DELUSION              229
#define SKILL_CUTPURSE              230
#define SKILL_COWER                 231
#define SKILL_DANGER_SENSE          232
#define SKILL_THIEF_SENSE           233
#define SKILL_ASSASSINATION         234
#define SKILL_DIRTY_TACTICS         235
#define SKILL_AGGRESSIVE            236
#define SKILL_SHADOW_DANCE          237
#define SKILL_GORE                  238
#define SKILL_BREATHE               239
#define SKILL_BERSERK               240
#define SKILL_ART_MONKEY            241
#define SKILL_ART_FLOWER            242
#define SKILL_GUARD                 243
#define SKILL_HEIGHTENED_SENSES	    244
#define SKILL_RIPOSTE               245
#define SKILL_BLACKJACK             246
#define SKILL_AMBUSH                247
#define SKILL_SHADOW_STEP           248
#define SKILL_SHADOW_MIST           249
#define SKILL_GUT                   250
#define SKILL_BRAIN                 251
#define SKILL_CUT_THROAT            252
#define SKILL_CONVERT               253
#define SKILL_FAMILIARIZE           254
#define SKILL_ESCAPE                255
#define SKILL_DANCE_DEATH           256
#define SKILL_SHADOWBOX             257
#define SKILL_FENCE                 258
#define SKILL_MUG                   259
#define SKILL_SPY                   260
#define SKILL_DISTRACT              261
#define SKILL_RETARGET              262
#define SKILL_DEVOUR                263
#define SKILL_BLOCK                 264
#define SKILL_SHIELD_BASH           265
#define SKILL_WEAPON_MASTERY        266
#define SKILL_REDOUBT               267
#define SKILL_INVIGORATE            268
#define SKILL_STEADFASTNESS         269
#define SKILL_SCOUT                 270
#define SKILL_BULLSEYE              271
#define SKILL_EXPOSE                272
#define SKILL_CAMP                  273
#define SKILL_POULTICE              274
#define SKILL_FEED                  275
#define SKILL_CALM                  276
#define SKILL_MIST                  277
#define SKILL_STING                 278

/* epic skills */
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

/*
 *  NON-PLAYER AND OBJECT SPELLS AND SKILLS
 *  The practice levels for the spells and skills below are _not_ recorded
 *  in the playerfile; therefore, the intended use is for spells and skills
 *  associated with npcs or objects.
 */

/* 301 is unused(so its free), cause I moved identify into player spells. */
#define SPELL_FIRE_BREATH            302
#define SPELL_GAS_BREATH             303
#define SPELL_FROST_BREATH           304
#define SPELL_ACID_BREATH            305
#define SPELL_LIGHTNING_BREATH       306
#define SPELL_YOUTHEN                307
#define SPELL_KNOWLEDGE              308
#define SPELL_RECHARGE               309
#define SPELL_HIPPOCRATIC_OATH       310
#define SKILL_INIQUITY               311
#define SKILL_GUARDIAN_ANGEL         312
#define SKILL_SHADOWFORM             313
#define SPELL_FAVORED_SOUL           314
#define SKILL_DIRE_BLOWS             315
#define SKILL_PET_MEND               316
#define SKILL_POTENCY                317
#define SKILL_COMMANDING_SHOUT       318
#define SKILL_DETERRENCE             319
#define SPELL_PARANOIA               320
#define SPELL_CONFUSION              321
#define SKILL_NM_SACRIFICE           322
#define SKILL_METAMORPHOSIS          323
#define SPELL_CRUSADE2               324
#define SKILL_ADUMBRATION            325
#define SKILL_PHASESHIFT             326
#define TMP_COMBAT_BUFF              327
#define SPELL_SAGACITY2              328
#define SKILL_EMACIATED              329
#define SKILL_EMACIATED_HIT          330
#define SKILL_EMACIATED_MANA         331
#define SPELL_MALEDICT2              332
#define SKILL_SHOWDAM                333
#define SPELL_VIVIFY                 334
#define SKILL_RAGE                   335
#define SKILL_UNIMPEDED_ASSAULT      336
#define SKILL_DEFENDER_HEALTH        337
#define SPELL_ZOMBIE_REWARD          338
#define NPC_SPELL_TOP                SPELL_ZOMBIE_REWARD /* Track the spells currently defined */

/* Player songs, numbered from SONG_FIRST to MAX_SONGS */
#define SONG_FIRST                   351
#define SONG_MINOR_REFRESHMENT       351

#define MAX_SONGS                    399

#define TOP_SPELL_DEFINE         399
const char *spell_wear_off_msg[NUM_SPELLS];
char *spells[TOP_SPELL_DEFINE + 2]; /* all pc/npc skills & spells */

/* NEW NPC/OBJECT SPELLS can be inserted here up to 499 */


/* WEAPON ATTACK TYPES */

#define TYPE_HIT                     400
#define TYPE_STING                   401
#define TYPE_WHIP                    402
#define TYPE_SLASH                   403
#define TYPE_BITE                    404
#define TYPE_BLUDGEON                405
#define TYPE_CRUSH                   406
#define TYPE_POUND                   407
#define TYPE_CLAW                    408
#define TYPE_MAUL                    409
#define TYPE_THRASH                  410
#define TYPE_PIERCE                  411
#define TYPE_BLAST                   412
#define TYPE_PUNCH                   413
#define TYPE_STAB                    414
#define TYPE_STRANGLE                415
#define TYPE_TEAR                    416
#define TYPE_SQUEEZE                 417
#define TYPE_STOMP                   418
#define TYPE_DRAIN                   419
#define TYPE_VAMP_BITE               420
#define TYPE_BURN                    421
#define TYPE_IMPALE                  422
#define TYPE_KICK                    423

#define NUM_WEAPON_TYPES             24
/* new attack types can be added here - up to TYPE_SUFFERING */
#define TYPE_SPECIAL                 490
#define TYPE_ASSISTANT               491
#define TYPE_UNDEAD_DECAY            496
#define TYPE_ROOM_SUFFER             497
#define TYPE_FALLEN                  498
#define TYPE_SUFFERING               499

/* Attacktypes with grammar */
typedef struct attack_hit_type {
   char *singular;
   char *plural;
} AttackHitType;

const AttackHitType attack_hit_text[NUM_WEAPON_TYPES];

#define TAR_IGNORE        1
#define TAR_CHAR_ROOM     2
#define TAR_CHAR_WORLD    4
#define TAR_FIGHT_SELF    8
#define TAR_FIGHT_VICT   16
#define TAR_SELF_ONLY    32 /* Only a check, use with i.e. TAR_CHAR_ROOM */
#define TAR_NOT_SELF     64 /* Only a check, use with i.e. TAR_CHAR_ROOM */
#define TAR_OBJ_INV     128
#define TAR_OBJ_ROOM    256
#define TAR_OBJ_WORLD   512
#define TAR_OBJ_EQUIP  1024
#define TAR_CHAR_GROUP 2048 /* Spell can ALSO be cast on group. */
#define TAR_CHAR_DIR   4096 /* Spell can be cast in a direction */
#define TAR_DIRECTION  8192 /* Spell is cast in a direction, no target */
#define TAR_CHAR_ZONE 16384

/* TAR_DIRECTION spells will actually point to one of these objects */
/* WARNING: do not treat these objects as real objects!  They are
 * merely pointer locations for reference! */
#define TAR_DIR_NORTH   (cast_directions + 0)
#define TAR_DIR_EAST    (cast_directions + 1)
#define TAR_DIR_SOUTH   (cast_directions + 2)
#define TAR_DIR_WEST    (cast_directions + 3)
#define TAR_DIR_UP      (cast_directions + 4)
#define TAR_DIR_DOWN    (cast_directions + 5)
extern ObjData cast_directions[NUM_OF_DIRS];

typedef struct spell_info_type {
   byte min_position;	/* Position for caster	 */
   int  mana_min;	/* Min amount of mana used by a spell (highest lev) */
   int  mana_max;	/* Max amount of mana used by a spell (lowest lev) */
   byte mana_change;	/* Change in mana used by spell from lev to lev */

   byte min_level[NUM_CLASSES];
   int routines;
   byte violent;
   sh_int targets;         /* See below for use with TAR_XXX  */
   byte grouplvl;          /* added to min_level, becomes lvl of group version, if applicable. */
} SpellInfoType;

extern SpellInfoType spell_info[TOP_SPELL_DEFINE + 1];

/* Possible Targets: 

   bit 0 : IGNORE TARGET
   bit 1 : PC/NPC in room
   bit 2 : PC/NPC in world
   bit 3 : Object held
   bit 4 : Object in inventory
   bit 5 : Object in room
   bit 6 : Object in world
   bit 7 : If fighting, and no argument, select tar_char as self
   bit 8 : If fighting, and no argument, select tar_char as victim (fighting)
   bit 9 : If no argument, select self, if argument check that it IS self.

*/

#define SPELL_TYPE_SPELL   0
#define SPELL_TYPE_POTION  1
#define SPELL_TYPE_WAND    2
#define SPELL_TYPE_STAFF   3
#define SPELL_TYPE_SCROLL  4

#define ASPELL(spellname) \
void	spellname(byte      level,   \
                  CharData *ch,      \
		  CharData *victim,  \
                  ObjData  *obj,     \
                  ObjData  *castobj, \
                  int       casttype )

/* Player Spell Declartions */
ASPELL(spell_create_water);
ASPELL(spell_recall);
ASPELL(spell_teleport);
ASPELL(spell_relocate);
ASPELL(spell_summon);
ASPELL(spell_locate_object);
ASPELL(spell_charm);
ASPELL(spell_information);
ASPELL(spell_identify);
ASPELL(spell_enchant_weapon);
ASPELL(spell_portal);
ASPELL(spell_calm);
ASPELL(spell_forget);
ASPELL(spell_shriek);
ASPELL(spell_banish);
ASPELL(spell_sand_storm);
ASPELL(spell_hands_of_wind);
ASPELL(spell_life_drain);
ASPELL(spell_assistant);
ASPELL(spell_fortify);
ASPELL(spell_sacrifice);
ASPELL(spell_nexus);
ASPELL(spell_cleanse);
ASPELL(spell_consume_corpse);
ASPELL(spell_explode_corpse);
ASPELL(spell_bone_wall);
ASPELL(spell_summon_corpse);
ASPELL(spell_embalm);
ASPELL(spell_charm_corpse);
ASPELL(spell_fletch);
ASPELL(spell_entomb_corpse);

/* Epic Spell Declarations */
ASPELL(spell_call_steed);

/* NPC & Object Spells Declartion */
ASPELL(spell_fire_breath);
ASPELL(spell_frost_breath);
ASPELL(spell_acid_breath);
ASPELL(spell_gas_breath);
ASPELL(spell_lightning_breath);
ASPELL(spell_youthen);
ASPELL(spell_knowledge);
ASPELL(spell_recharge);

/* basic magic calling functions */

/*
** add_affect is one UGLY function prototype but the
** alternative is even uglier. An example of how to use
** it to add the affects from poison follows:
**
**     add_affect( c, v, SPELL_POISON, <- Specify the caster, victim and spell.
**                 GET_LEVEL(c),    <- The spell's level
**                 APPLY_STR, -2,   <- What and how much does it affect ?
**                 GET_LEVEL(ch),   <- How many ticks will it last ?
**                 AFF_POISON,      <- The affect bit to set on victim.
**                 FALSE, FALSE,
**                 FALSE, FALSE);
*/
void add_affect( CharData * caster,
		 CharData * target,
                 int  spell_num,
                 int  level,
                 int  location,
                 int  modifier,
                 int  duration,
                 int  bitvector,
                 bool add_dur,
                 bool avg_dur,
                 bool add_mod,
                 bool avg_mod );

int  call_magic( CharData *caster,
                 CharData *cvict,
                 ObjData *ovict,
                 int spellnum, int level,
                 ObjData *castobj, int casttype);

int  cast_spell( CharData *ch, CharData *tch, ObjData *tobj, int spellnum);
int  find_skill_num( char *name );
int  mag_savingthrow( CharData *ch, int type);

void mag_affects(     int level, CharData *ch, CharData *victim, int spellnum, int savetype);
void mag_alter_objs(  int level, CharData *ch, ObjData  *obj, int spellnum, int type);
void mag_areas(       int level, CharData *ch, int spellnum, int savetype);
void mag_creations(   int level, CharData *ch, int spellnum);
void mag_damage(      int level, CharData *ch, CharData *victim, int spellnum, int savetype);
void mag_group_switch(int level, CharData *ch, CharData *tch, int spellnum, int savetype);
void mag_groups(      int level, CharData *ch, int spellnum, int savetype);
void mag_masses(      int level, CharData *ch, int spellnum, int savetype);
void mag_objectmagic( CharData *ch, ObjData *obj, char *arg );
void mag_points(      int level, CharData *ch, CharData *victim, int spellnum, int savetype);
void mag_summons(     int level, CharData *ch, ObjData *obj, int spellnum, int savetype);
void mag_unaffects(   int level, CharData *ch, CharData *victim, int spellnum, int type);

#endif
