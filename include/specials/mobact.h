#ifndef __MOBACT_H__
#define __MOBACT_H__
/* ============================================================================ 
mobact.h
Header file for mob activities.
Written by Vex of RavenMud for RavenMUD.
============================================================================ */

/* ============================================================================ 
Public functions.
============================================================================ */
int   mobAttackVictim(CharData *mob, CharData *victim);
void  mob_attack(CharData *mob, CharData *vict);
void  clearMemory(CharData * ch);
void  mobile_activity(void);
void  forget(struct char_data * ch, struct char_data * victim);
void  remember(CharData *ch, CharData *victim);

#endif /* __MOBACT_H__ */

