#ifndef __OFFENSIVE_H__
#define __OFFENSIVE_H__
/* ============================================================================ 
offensive.h
Header file for offensive commands. Written by Vex of RavenMUD for RavenMUD.
============================================================================ */

/* ============================================================================ 
Public functions.
============================================================================ */
void do_flee(CharData *ch, char *argument, int cmd, int subcmd);
void do_assist(CharData *ch, char *argument, int cmd, int subcmd);

/* ============================================================================ 
Public variables.
============================================================================ */
#ifndef __OFFENSIVE_C__

extern CharData guard_group;

#endif /* __OFFENSIVE_C__ */

#endif /* _OFFENSIVE_H_*/
