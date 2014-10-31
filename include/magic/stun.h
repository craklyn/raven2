/* ============================================================================ 
Header file for all stun skills. Written by Vex of RavenMUD for RavenMUD 
============================================================================ */
#ifndef __STUN_H__
#define __STUN_H__


/* ============================================================================ 
Public functions for performing stun skills.
============================================================================ */
void do_bash(CharData *ch, char *argument, int cmd, int subcmd);
void do_kick(CharData *ch, char *argument, int cmd, int subcmd);
void do_fists(CharData *ch, char *argument, int cmd, int subcmd);
void do_sweep(CharData *ch, char *argument, int cmd, int subcmd);
void do_trip(CharData *ch, char *argument, int cmd, int subcmd);
void do_hamstring(CharData *ch, char *argument, int cmd, int subcmd);
void do_shieldbash(CharData *ch, char *argument, int cmd, int subcmd);

#endif
