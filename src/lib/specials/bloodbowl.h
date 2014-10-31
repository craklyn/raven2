/* ============================================================================
*Header file for special routines of bloodbowl zone.
*Unknown author - 10/18/99
============================================================================ */
#ifndef __BLOODBOWL_H__
#define __BLOODBOWL_H__

/* Constants */
#define BLOODBOWL_TOP   30000
#define BLOODBOWL_BOT   30314
#define BLOODBOWL_START 30310
#define BLOODBALL       30000

/* ============================================================================
Public functions in bloodbowl.c.
============================================================================ */

int   inBloodBowl( CharData* );
void  dieInBloodBowl( CharData* );
int   bloodref( CharData *ch, void *me, int cmd, char *argument );
int   blood_ball(CharData *ch, void *me, int cmd, char *argument);

#endif /* __BLOODBOWL_H__ */
