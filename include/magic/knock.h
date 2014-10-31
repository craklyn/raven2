#ifndef __KNOCK_H__
#define __KNOCK_H__
/* ============================================================================ 
Header file for the knock skill. Vex.
============================================================================ */

/* The knock skill. */
void do_knock(CharData *ch, char *argument, int cmd, int subcmd);
void knock_internal(CharData *ch, CharData *victim);

#endif
