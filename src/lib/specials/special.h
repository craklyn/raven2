/* ============================================================================ 
special.h
Header file for standard special procedures.
============================================================================ */
#ifndef __SPECIAL_H__
#define __SPECIAL_H__

/* ============================================================================ 
Public functions.
============================================================================ */
void  specialName(int (*theFunc)(CharData *ch, void *me, int cmd, char *argument), char *theName);
int   chIsNonCombatant(CharData *ch);

/* ============================================================================   
The special procedures them selves.
============================================================================ */
int  null_proc(CharData *ch, void *me, int cmd, char *argument);
int  remort_questmaster(CharData *ch, void *me, int cmd, char *argument);

int  guild(CharData *ch, void *me, int cmd, char *argument);
#ifdef DO_DUMPS
int  dump(CharData *ch, void *me, int cmd, char *argument);
#endif
int  snake(CharData *ch, void *me, int cmd, char *argument);
int  thief(CharData *ch, void *me, int cmd, char *argument);
int  fido(CharData *ch, void *me, int cmd, char *argument);
int  janitor(CharData *ch, void *me, int cmd, char *argument);
int  cityguard(CharData *ch, void *me, int cmd, char *argument);
int  roomAggro(CharData *ch, void *me, int cmd, char *argument);
int  roomFall(CharData *ch, void *me, int cmd, char *argument);
int  clan_bank(CharData *ch, void *me, int cmd, char *argument);
int  bank(CharData *ch, void *me, int cmd, char *argument);
SPECIAL(pet_shops);

/* ============================================================================ 
Other stuff.
============================================================================ */
#define CMD_NORTH 1
#define CMD_EAST  2
#define CMD_SOUTH 3
#define CMD_WEST  4
#define CMD_UP    5
#define CMD_DOWN  6

#define GUARD_CLASS 1
#define GUARD_LEVEL 2
#define GUARD_CLAN  3

typedef struct {
    sh_int  guarded_room;
    sh_int  guarded_exit;
    byte    allowed_passage;
    char   *guard_name;
} guarded_exit_t;

int   exitIsGuarded(guarded_exit_t *guard_list, int guard_type, int cmd, CharData *ch);
char  *exitIsClanGuarded(int cmd, CharData *ch);

#endif /* __SPECIAL_H__ */
