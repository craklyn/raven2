/* ============================================================================ 
commact.h
Header file for communciation functions. Written by Vex of RavenMUD for
RavenMUD. This used to be all in "act.comm.c"
============================================================================ */

/* ============================================================================ 
Public functions.
============================================================================ */
extern void do_say(CharData *ch, char *argument, int cmd, int subcmd);
extern void do_gen_comm(CharData *ch, char *argument, int cmd, int subcmd);
char *makedrunk(char *string ,struct char_data *ch);
