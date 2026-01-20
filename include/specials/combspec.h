#ifndef __COMBSPEC_H__
#define __COMBSPEC_H__
/* ============================================================================ 
combspec.h
Header file for special procedures on mobs and objects that are only invoked
during combat. Written by Vex of RavenMUD for RavenMUD.
============================================================================ */

/* ============================================================================ 
Public functions.
============================================================================ */
int   attackerSpecial(CharData *attacker, CharData *victim, int *damage, int *attacktype);
int   victimSpecial(CharData *attacker, CharData *victim, int *damage, int *attacktype);
void  assignCombatSpecials(void);
void  combSpecName(int (*combSpec)(void *, CharData *, int *, int *), char *theName);
int   isUsingLochaber(CharData *ch);

#endif  /* __COMBSPEC_H__ */
