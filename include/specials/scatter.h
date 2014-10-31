#ifndef __SCATTER_H__
#define __SCATTER_H__
/* ============================================================================
 * scatter.h
 *
 * Header file system for scatter.c - Random item drops across the MUD world.
 * Author: Unknown (and hopefully he didn't do any more coding on Raven)
============================================================================ */

/* scatter Public functions */
void  scatter_pulse(void);
void  scatter_init(void);
int   check_scatter_give(CharData *ch, CharData *vict, ObjData *obj);

#endif /* End of _SCATTER_H_ */
