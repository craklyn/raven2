#ifndef __AGGRESSIVE_H__
#define __AGGRESSIVE_H__
/* ============================================================================  
aggressive.h
Header file for the aggressive skill.
Copyright 1997 Jeremy Wright(a.k.a Vex)
============================================================================ */

#define AGGR_OFF 0
#define AGGR_MONSTERS 1
#define AGGR_PLAYERS 2
#define AGGR_BOTH 3
#define NUM_AGGR_PREF 4

#define GET_AGGR_PREF(ch) ( (ch)->player_specials->saved.aggr_pref)
/* ============================================================================ 
Public functions.
============================================================================ */
extern int pcAttackVictim(CharData *attacker, CharData *victim);

#endif
