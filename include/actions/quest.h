/* ***************************************************************************
*    File:   quest.h                                  Part of CircleMUD      *
* Version:   2.1 (December 2005) Written and Modified for CircleMUD/RavenMUD *
* Purpose:   To provide special quest-related code.                          *
******************************************************************************/
#ifndef __QUEST_H__
#define __QUEST_H__

/* Quest Functions ***********************************************************/
ACMD(do_quest);
int  check_quest_give(CharData *ch, CharData *to, ObjData *obj);
void check_quest_kill(CharData *ch, CharData *vict);
void give_quest(CharData *ch, QuestData *qst, CharData *mob);
int  can_take_quest(CharData *ch, QuestData *qst, CharData *mob);

#define QUEST_KILL_TOKEN        1

/* ============================================================================
global buffering system - allow access to global variables within quest.c
============================================================================ */
#ifndef __QUEST_C__

extern char *questflag_bits[];

#endif /* __QUEST_C__ */

#endif /* __QUEST_H__ */
