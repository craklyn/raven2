/* ************************************************************************
*   File: stalk.h                                       Part of CircleMUD *
*  Usage: header file for stalking code                                   *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
#ifndef __SEEK_H__
#define __SEEK_H__

/* Stalking Constants */
#define SEEK_IDLE       0      /* Not stalking                       */
#define SEEK_OK         1      /* Merrily Stalking Along             */
#define SEEK_BLOCKED    2      /* Something in the way he moves....  */
#define SEEK_FIGHTING   3      /* Fighting his prey                  */
#define SEEK_LOSTPREY   4      /* Prey rented, quit or was killed    */
#define SEEK_BFSERROR   5      /* TRACK returned an error            */
#define SEEK_NOPATH     6      /* No path found                      */
#define SEEK_DOOR       7      /* Door in way                        */
#define SEEK_LOCKED     8      /* Locked door in way                 */
#define SEEK_BUSY       9      /* Fighting someone else              */
#define SEEK_WATER      10     /* Water in the way                   */
#define SEEK_AIR        11     /* Needs to fly                       */
#define SEEK_POOFED     12     /* Just Teleported                    */

/* Constants returned from *_find_dots */
/* result of >=1 is the number preceding the dot */
#define YDOTS_ZERO        0     /* 0.fred    */
#define YDOTS_NONE       -1     /* fred      */
#define YDOTS_NODOTS     -1     /* fred      */
#define YDOTS_ALLDOT     -2     /* all.fred  */
#define YDOTS_UNKNOWN    -3     /* hi.fred   */
#define YDOTS_ALL        -4     /* all       */
#define YDOTS_ALLDOTALL  -5     /* all.all   */

/* dotmodes used elsewhere */
#define YDOTS_QUEST     -10
#define YDOTS_PC        -11
#define YDOTS_NPC       -12
#define YDOTS_MOB       -13

/* All procedures located in stalk.c  */

/* Moves mob ch one step closer to prey     */
void seek_seekprey(struct char_data *ch);
/* Pick a new target for the mob */
void seek_newtarget(struct char_data *ch);

#endif /* __SEEK_H__ */
