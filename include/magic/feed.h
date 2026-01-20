#ifndef __FEED_H__
#define __FEED_H__
/* ============================================================================ 
feed.h
The feed skill header.
============================================================================ */

/* ============================================================================ 
Public functions.
============================================================================ */
extern void feed_internal(CharData *ch, CharData *victim);
extern void do_feed(CharData *ch, char *argument, int cmd, int subcmd);

#endif
