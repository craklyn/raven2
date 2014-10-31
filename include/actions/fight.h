/* ============================================================================ 
Header file for fight.c
Written by Vex of RavenMUD for RavenMUD.
============================================================================ */
#ifndef _FIGHT_H_
#define _FIGHT_H_

/* ============================================================================ 
Public functions in fight.c.
============================================================================ */
int      realAC(CharData *ch);
int      thacoCalc(CharData *ch);
int      calculateDamage(CharData * ch, CharData * victim, int attackType, int attackNumber );
int      calculateSimpleDamage(CharData *ch);
void     damage(CharData * ch, CharData * victim, int dam, int attacktype);
void     end_fight(CharData *ch);
void	 stop_fighting(CharData *ch);
void     protect_unlinked_victim(CharData *victim);
void     mobile_combat_moves(void);
ObjData *raw_kill(CharData * ch, CharData * killer);
ObjData *die( CharData *ch, CharData *killer, int pkill);
ObjData *make_corpse( CharData *ch );
void     free_messages(void);

void	set_fighting(CharData *ch, CharData *victim);
void	hit(CharData *ch, CharData *victim, int type);
int	skill_message(int dam, CharData *ch, CharData *vict, int attacktype);

void    ch_kill_victim( CharData *ch, CharData *victim);
void    death_cry(CharData * ch);
int     pvpFactor(void);
/* utility functions for causing damage */
int      damage_affects(CharData *ch, CharData *victim, int dam, int type);
int      damage_limit(CharData *ch, CharData *victim, int dam, int type);
void     damage_display(CharData *ch, CharData *victim, int dam, int type);
int      spell_damage_gear(CharData *ch);
void     show_kills(CharData *ch, char *name);


/* ============================================================================
Structures.
============================================================================ */
typedef struct msg_type
{
  char *attacker_msg;  /* message to attacker */
  char *victim_msg;    /* message to victim   */
  char *room_msg;      /* message to room     */
} MsgType;


typedef struct message_type
{
  MsgType die_msg;               /* messages when death      */
  MsgType miss_msg;              /* messages when miss       */
  MsgType hit_msg;               /* messages when hit        */
  MsgType god_msg;               /* messages when hit on god */
  struct  message_type *next;    /* next group of messages   */
} MessageType;

typedef struct message_list
{
  int  a_type;            /* Attack type                      */
  int  number_of_attacks; /* How many messages to chose from. */
  MessageType *msg;       /* List of messages.                */
} MessageList;

/* Hard coded bonuses */
typedef struct class_ModT
{
  int pc_level;         /* The inclusive level to apply the bonus to */
  int number_of_dice;   /* The number of dice to toss for this level */
  int size_of_dice;     /* The number of sides on the die            */
  int base_ac;          /* The base AC to use for the class          */
  int to_hit;           /* The hitroll bonus                         */
  int to_damage;        /* And, finally, the damage bonus            */
} ClassModT;

// Data on one melee attack
typedef struct melee_damage_data {
  int damage;		// How much damage was inflicted.
  int type;		// What skill/spell or other attack form?
  bool valid;		// Is this data part of a 'real' attack?
} MeleeDamageData;

/* ============================================================================
Constants.
============================================================================ */
extern const ClassModT shouLinMods[13];
extern MessageList fight_messages[MAX_MESSAGES];

#define  MAX_DAMAGE   250	// damage cap limit.
#define  MAX_NUM_ATTACKS 5	// maximum number of attacks/round

#define  pvpHoliday(ch)  (!IS_NPC(ch) && pvpFactor() > 1)
// Chance in 100 that a character can flee in PvP
#define  fleeFactor(ch)	 (fleeValue * attackers(ch))
// Set arbitrarily for balance by Craklyn
#define  fleeValue       100

#define BLUNT_MASTERY  1
#define POINT_MASTERY  2
#define EDGE_MASTERY   3
#define EXOTIC_MASTERY 4

#endif /* _FIGHT_H_*/
