#ifndef __SECRETS_H__
#define __SECRETS_H__

/* Thresholds for Status System */
#define STATUS_EXPLORER_THRESHOLD 5
#define STATUS_VETERAN_THRESHOLD  10
#define STATUS_LEGEND_THRESHOLD   15

/* Chore System Secrets */
#define CHORE_VISIT     0
#define CHORE_KILL      1
#define CHORE_QUEST     2

typedef struct player_chore {
    unsigned char type;
    int vnum;
} PlayerChore;

extern PlayerChore chores[];
extern int num_chores;

#endif
