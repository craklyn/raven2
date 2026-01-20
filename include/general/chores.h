/* ************************************************************************
*  File: chores.h                                       Part of  RavenMUD *
*  Author: Imhotep of RavenMUD						  *
*  Usage: Random tasks for characters to achieve                          *
*                                                                         *
*  RavenMUD is derived from CircleMUD, so the CircleMUD license applies.  *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
#ifndef _CHORES_H_
#define _CHORES_H_

/* List of global function prototypes found in chores.c. */
void chore_initialise(CharData *ch);
void chore_check_kill(CharData *ch, int vnum);
void chore_check_quest(CharData *ch, int vnum);
void chore_update(CharFileU *ch);

#endif /* _CHORES_H_ */
